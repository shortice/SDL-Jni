/*
Yes, this code not handling JNI errors and dont implement cache for
slow operations like GetMethodID.
If you really want to use this in producation scope, please
implement this.
Or just wait path from me (maybe in 1-2 weeks later, maybe...).

More info about JNI:
https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html
*/
#include "android_jni.hpp"
#include "jni.h"
#include "SDL3/SDL_system.h"

/*
Java signature:
void PushNotification(String title, String text) {};
JNI signature:
(Ljava/lang/String;Ljava/lang/String;)V

More info about JNI types:
https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/types.html
*/
void PushNotification(
    const char* title,
    const char* text
) {
    JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();

    // Get the current SDL Activity java object
    jobject activity = (jobject)SDL_GetAndroidActivity();
    // Get the class from object
    jclass activityClass(env->GetObjectClass(activity));

    // Get method id from class
    jmethodID method_id = env->GetMethodID(
        activityClass,
        "PushNotification", // method name
        "(Ljava/lang/String;Ljava/lang/String;)V" // method signature ^^^
    );

    // Create Java string from C-string
    jstring param1 = env->NewStringUTF(title);
    jstring param2 = env->NewStringUTF(text);

    // Call the method
    env->CallVoidMethod(
        activity,
        method_id,
        param1,
        param2
    );

    // Clean up memory
    env->DeleteLocalRef(param1);
    env->DeleteLocalRef(param2);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
}
