VERSION = 1.0.0
BUILD = 1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_BUILD=$$BUILD
DEFINES += APP_BUILD_STR=\\\"$${BUILD}\\\"

# TARGET is the name of the built executable
TARGET = QtAndroidService

QT += core gui
CONFIG += console c++17 sdk_no_version_check

# Source files
SOURCES += main.cpp timerworker.cpp
HEADERS += timerworker.h

android {
    DEFINES += BUILD_ANDROID ANDROID
    QTANDROID_DEPLOYMENT_SETTINGS += --install

    TEMPLATE = app

    LIBS += -llog

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    # Android API levels
    ANDROID_TARGET_SDK_VERSION = 32
    ANDROID_MIN_SDK_VERSION = 23

    # Android-specific permissions
    ANDROID_PERMISSIONS += \
        android.permission.ACCESS_NETWORK_STATE \
        android.permission.INTERNET \
        android.permission.FOREGROUND_SERVICE \
        android.permission.WAKE_LOCK
} else {
    # Desktop build
    TEMPLATE = app
    CONFIG += app_bundle
}

# Files to include in deployment
DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/src/org/example/qtservice/MainActivity.java \
    android/src/org/example/qtservice/QtAndroidService.java

message("Platform DEFINES: $$DEFINES")
contains(DEFINES, BUILD_ANDROID): message("Android build detected")
