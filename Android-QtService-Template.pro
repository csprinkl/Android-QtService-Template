VERSION = 1.0.0
BUILD = 1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_BUILD=$$BUILD
DEFINES += APP_BUILD_STR=\\\"$${BUILD}\\\"

# TARGET is the name of the built executable
TARGET = Android-QtService-Template

QT += positioning concurrent multimedia serialport sql httpserver widgets core gui core-private
CONFIG += console c++17 app_bundle sdk_no_version_check

android {
    DEFINES += BUILD_ANDROID
    QTANDROID_DEPLOYMENT_SETTINGS += --install

    LIBS += -L$$ANDROID_LIB_PATH \
        -lmediandk \
        -latomic \
        -llog

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    CONFIG(debug, debug|release) {
        message("Including ANDROID_EXTRA_LIBS for DEBUG")
    }

    CONFIG(release, debug|release) {
        message("Including ANDROID_EXTRA_LIBS for RELEASE")
    }

    # Android-specific permissions
    ANDROID_PERMISSIONS += \
        android.permission.ACCESS_NETWORK_STATE \
        android.permission.INTERNET \
        android.permission.ACCESS_FINE_LOCATION \
        android.permission.ACCESS_COARSE_LOCATION
}

QML_IMPORT_NAME = com.deft.example
QML_IMPORT_MAJOR_VERSION = 1
QML_IMPORT_MINOR_VERSION = 0

SOURCES += \
    main.cpp \
    timerworker.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/strings.xml

message("Platform DEFINES: $$DEFINES")
contains(DEFINES, BUILD_ANDROID): message("Android build detected")

HEADERS += \
    timerworker.h
