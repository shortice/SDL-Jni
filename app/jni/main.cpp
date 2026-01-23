// Using new SDL callbacks feature.
// More info: https://wiki.libsdl.org/SDL3/README-main-functions
#define SDL_MAIN_USE_CALLBACKS 1
#include "malloc.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_system.h"
#include "SDL3/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "jila-android.hpp"
#include <string>


struct AppState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    ImGuiIO* io;
};


static SDL_Rect rect {};

void handle_resize(SDL_Window* window) {
    SDL_GetWindowSafeArea(
        window,
        &rect
    );
}

void HandleRequest(void* userdata, const char* perm, bool granted) {}

void RequestNotificationPermission() {
    SDL_RequestAndroidPermission(
        "android.permission.POST_NOTIFICATIONS",
        &HandleRequest,
        NULL
    );
}

SDL_AppResult SDL_AppInit(void** state, int argc, char** argv) {
    // Init Dear ImGui and SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    };

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Application",
        -1,
        -1,
        SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    };

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        NULL
    );

    if (!renderer) {
        SDL_Log("SDL_CreateRenderer error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    };

    SDL_SetRenderVSync(renderer, 1);

    // Setup Dear ImGui context

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();

    float main_scale = SDL_GetDisplayContentScale(
        SDL_GetPrimaryDisplay()
    );
    style.FontScaleMain = 1.5f;
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.IniFilename = nullptr;

    *state = new AppState {window, renderer, &io};

    handle_resize(window);

    // Get SDL Android Activity Context
    JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
    jobject activity = (jobject)SDL_GetAndroidActivity();
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getApplicationContext = env->GetMethodID(
        activityClass, 
        "getApplicationContext", 
        "()Landroid/content/Context;"
    );
    jobject context = env->CallObjectMethod(activity, getApplicationContext);

    Jila_Android_InitContext(env, context);
    Jila_Android_CreateNotificationChannel(
        "default",
        "Default", 
        "Default notification channel"
    );

    // Clear jni resources
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return SDL_APP_CONTINUE;
}

static bool isError = false;
static char* notification_title = (char*)calloc(1, 50);
static char* notification_text = (char*)calloc(1, 50);

static std::string folderPath;
static std::string item;

void OpenFolderCallback(const char* path) {
    folderPath = path;
}

SDL_AppResult SDL_AppIterate(void* state) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(rect.x, rect.y)); // Safe area on Mobile devices
    ImGui::SetNextWindowSize(ImVec2(rect.w, rect.h));
    ImGui::Begin(
        "SDL, Dear ImGui and JNI!",
        NULL,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
    );

    static char* err = (char*)"";

    ImGui::TextWrapped("If notification doesnt work, click on this button:");
    if (ImGui::Button("Grant permission")) {
        RequestNotificationPermission();
    }

    ImGui::Separator();

    ImGui::Text("Title: ");
    ImGui::InputText("##title", notification_title, 50);

    ImGui::Text("Message: ");
    ImGui::InputText("##message", notification_text, 50);

    ImGui::Separator();

    ImGui::Text("%s\n%s", "Error:", err);

    if (ImGui::Button("GetError")) {
        err = (char*)Jila_Android_GetError();
    }

    if (ImGui::Button("Push Notification")) {
        Jila_Android_PushNotification(
            "default", 
            1, 
            notification_title, 
            notification_text, 
            // Note. This call maybe slow, need cache GetResID value
            Jila_Android_GetResID(
                "ic_launcher",
                "drawable"
            )
        );
    }

    ImGui::Separator();

    // File OpenFolder Dialog + FileIterator

    if (ImGui::Button("Open folder")) {
        Jila_Android_OpenFolder(&OpenFolderCallback);
    }

    if (ImGui::Button("Get first item from opened folder")) {
        if (!folderPath.empty()) {
            const char** files = Jila_Android_IterateFiles(
                folderPath.c_str(),
                false
            );

            if (files) {
                item = files[0];
                free(files);
            } else {
                item = "";
            }
        }
    }

    ImGui::TextWrapped("Current selected folder path: %s", folderPath.c_str());
    ImGui::TextWrapped("Current file item path: %s", item.c_str());

    ImGui::End();

    ImGui::Render();
    SDL_SetRenderScale(
        ((AppState*)state)->renderer,
        ((AppState*)state)->io -> DisplayFramebufferScale.x,
        ((AppState*)state)->io -> DisplayFramebufferScale.y
    );
    SDL_RenderClear(((AppState*)state)->renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(
        ImGui::GetDrawData(),
        ((AppState*)state)->renderer
    );
    SDL_RenderPresent(((AppState*)state)->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* state, SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (
        event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED |
        event->type == SDL_EVENT_WINDOW_RESIZED
    ) {
        handle_resize(
            SDL_GetWindowFromID(event->window.windowID)
        );
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* state, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    free(notification_text);
    free(notification_title);

    SDL_DestroyRenderer(
        ((AppState*)state)->renderer
    );
    SDL_DestroyWindow(
        ((AppState*)state)->window
    );
    SDL_Quit();

    delete (AppState*)state;
}
