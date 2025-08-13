VERSION = 1.0.0
BUILD = 1
DEFINES += APP_VERSION=\\\"$VERSION\\\"
DEFINES += APP_BUILD=$$BUILD
DEFINES += APP_BUILD_STR=\\\"${BUILD}\\\"

TARGET = QtAndroidService

# Qt Core only - no GUI, no Android integration
QT = core
QT -= gui widgets androidextras

CONFIG += console c++17 sdk_no_version_check
CONFIG -= app_bundle

# Source files
SOURCES += main.cpp timerworker.cpp qtservicemanager.cpp
HEADERS += timerworker.h qtservice_global.h qtservicemanager.h

android {
    DEFINES += BUILD_ANDROID ANDROID

    # STATIC LIBRARY with ALL Qt dependencies bundled
    TEMPLATE = lib
    CONFIG += staticlib
    CONFIG -= shared

    # Force static linking of ALL Qt libraries
    CONFIG += static
    QMAKE_LFLAGS += -static

    # Bundle Qt Core statically
    QT = core
    CONFIG += link_pkgconfig

    # Force static Qt linking
    QTPLUGIN +=
    CONFIG += qt_no_framework

    # No Qt Android integration or dynamic loading
    DEFINES += QT_NO_GUI QT_NO_WIDGETS QT_STATICPLUGIN QT_NO_DYNAMIC_LIBRARY_LOADING
    CONFIG -= android_deployment_settings qtquickcompiler

    # Android system libraries
    LIBS += -llog

    # Position-independent code
    QMAKE_CXXFLAGS += -fPIC
    QMAKE_CFLAGS += -fPIC

    # Android API levels
    ANDROID_TARGET_SDK_VERSION = 34
    ANDROID_MIN_SDK_VERSION = 23
}

# Desktop testing
!android {
    TEMPLATE = app
    CONFIG += console
}

message("Building self-contained QtAndroidService static library")
message("Qt modules: $QT")
message("Config: $CONFIG")
contains(DEFINES, BUILD_ANDROID): message("Android static library with Qt bundled inside")
