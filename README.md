# SDL-Jni

Sample code how use JNI in SDL3.
This code implement the simple notification API for SDL (including Android API 26+
and new notification permission).

Its uses Dear ImGui for simple UI and SDL3 for multimedia API.

Check source code in there files if you need details:
* app/jni/android_jni.cpp <- JNI code
* app/src/main/java/<...>/SDLActivity.java [lines from 66 to 122 and line 332] <-
notification implementation
* app/src/main/AndroidManifest.xml <- line 45 need for notification on latest Android versions

# How build this app?

Just run:
```
./gradlew build
```

Be sure you installed Android SDK and NDK.

I tested this code on this Android devices:
* 9;
* 12;
* 14;
* 16;

# Screenshot

![Notification show](content/notification.jpg)
