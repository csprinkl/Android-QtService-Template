VERSION = 1.0.0
BUILD = 1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_BUILD=$$BUILD
DEFINES += APP_BUILD_STR=\\\"$${BUILD}\\\"

# TARGET is the name of the built executable
TARGET = QtAndroidService

QT += core
CONFIG += console c++17 sdk_no_version_check

# Source files
SOURCES += main.cpp timerworker.cpp \
    qtservicemanager.cpp
HEADERS += timerworker.h \
    qtservice_global.h \
    qtservicemanager.h

android {
    DEFINES += BUILD_ANDROID ANDROID
    QTANDROID_DEPLOYMENT_SETTINGS += --install
    CONFIG += staticlib

    TEMPLATE = lib

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

    # Install headers for library users
    headers.files = \
        qtservicemanager.h \
        timerworker.h \
        qtservice_global.h
    headers.path = $$[QT_INSTALL_HEADERS]/QtAndroidService
    INSTALLS += headers

    # Install the library
    target.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target

    # Create pkg-config file for library distribution
    CONFIG += create_prl create_pc

    # Library metadata
    QMAKE_PKGCONFIG_NAME = QtAndroidService
    QMAKE_PKGCONFIG_DESCRIPTION = Qt Android Timer Service Library
    QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
    QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
    QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]/QtAndroidService
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
    android/src/org/example/qtservice/QtServiceWrapper.java

message("Platform DEFINES: $$DEFINES")
contains(DEFINES, BUILD_ANDROID): message("Android build detected")
