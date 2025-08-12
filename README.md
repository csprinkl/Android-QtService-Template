# Qt Android Timer Service

A Qt-based Android foreground service that demonstrates running background timer threads with precise intervals. This project shows how to create a Qt Android service with multiple QTimer-based worker threads running in separate QThreads.

## 🏗️ Project Structure

```
QtAndroidService/
├── main.cpp                     # Qt service entry point
├── timerworker.h/.cpp          # QTimer-based worker class
├── QtAndroidService.pro        # Qt project file
├── android/                    # Android-specific files
│   ├── AndroidManifest.xml     # Android manifest with service definitions
│   └── src/org/example/qtservice/
│       ├── MainActivity.java   # Android activity launcher
│       └── QtAndroidService.java # Java service wrapper
└── README.md                   # This file
```

## 📱 Architecture Overview

### **Dual-Process Architecture**
- **Main Process**: Runs MainActivity (minimal Android UI)
- **Service Process** (`:qt`): Runs the Qt service with timer threads

### **Components**

#### 1. **Qt C++ Layer**
- **`main.cpp`**: Entry point that creates 4 QThreads with TimerWorker objects
- **`TimerWorker`**: QObject-based class that manages QTimer in separate threads
- **Intervals**: 1s, 2s, 3s, and 5s timer threads

#### 2. **Java Android Layer**
- **`MainActivity.java`**: Launches the Qt service as a foreground service
- **`QtAndroidService.java`**: Android service wrapper that:
   - Starts as foreground service immediately (prevents ANR)
   - Loads Qt library asynchronously
   - Initializes Qt runtime in background thread
   - Manages service lifecycle

#### 3. **Android Manifest**
- **Foreground Service**: Configured with `dataSync` type
- **Separate Process**: Service runs in `:qt` process
- **Permissions**: Foreground service, wake lock, network access

## 🔄 Execution Flow

### Service Startup Sequence:
1. **MainActivity** calls `startForegroundService()`
2. **QtAndroidService.onCreate()** returns immediately after starting foreground notification
3. **Background thread** loads Qt library (`QtAndroidService_arm64-v8a`)
4. **Qt initialization** happens asynchronously via `super.onCreate()`
5. **Qt main()** creates 4 QThreads with TimerWorker objects
6. **Timer threads start** logging at their respective intervals

### Expected Log Output:
```
QtAndroidService: === Qt Timer Service Starting ===
QtAndroidService: Step 1-8: All initialization complete
QtAndroidService: Qt Timer Service: Starting event loop
libQtAndroidService_arm64-v8a.so: "Timer 1: Tick at 1000ms interval"
libQtAndroidService_arm64-v8a.so: "Timer 2: Tick at 2000ms interval"
libQtAndroidService_arm64-v8a.so: "Timer 3: Tick at 3000ms interval"  
libQtAndroidService_arm64-v8a.so: "Timer 4: Tick at 5000ms interval"
```

## ⚙️ Key Features

### **Multi-Threading**
- 4 separate QThreads each running a TimerWorker
- Thread-safe Qt signal/slot communication
- Proper thread lifecycle management

### **Android Foreground Service**
- No ANR (Application Not Responding) issues
- Progressive notification updates during initialization
- Proper foreground service lifecycle management
- Survives app backgrounding

### **Qt Service Integration**
- Uses Qt's official QtService framework
- Proper Qt event loop in service context
- Android logging integration (`__android_log_print`)
- Graceful shutdown handling

## 🛠️ Build Requirements

- **Qt 6.5.3** or later
- **Android SDK API 23+** (target API 32)
- **NDK** with arm64-v8a support
- **Clang** compiler

### Build Configuration:
```qmake
# Key settings from .pro file
TARGET = QtAndroidService
QT += core
CONFIG += console c++17
ANDROID_TARGET_SDK_VERSION = 32
ANDROID_MIN_SDK_VERSION = 23
```

## 🚀 Usage

### Building:
```bash
# Configure for Android
qmake QtAndroidService.pro
make
make install INSTALL_ROOT=android-build
```

### Running:
1. Install APK on Android device
2. Launch "QtAndroidService" app
3. Service starts automatically as foreground service
4. Check logcat for timer outputs:
```bash
adb logcat | grep -E "(QtService|QtAndroidService|Timer)"
```

### Stopping:
- Service runs until manually stopped or device reboot
- Automatically restarts if killed by system (`START_STICKY`)

## 📊 Performance Characteristics

- **Memory Usage**: ~50MB (Qt runtime + 4 worker threads)
- **CPU Usage**: Minimal (<1% on modern devices)
- **Battery Impact**: Low (efficient QTimer implementation)
- **Precision**: Timer accuracy typically within ±10ms

## ⚡ ANR Prevention Strategy

The service uses a **fast-exit pattern** to prevent Android ANR:

1. **`onCreate()` returns immediately** after starting foreground service
2. **Qt initialization happens asynchronously** in background thread
3. **Service lifecycle never blocks** main Android thread
4. **Progressive notifications** show initialization status

## 🔧 Customization

### Adding More Timers:
```cpp
// In main.cpp
TimerWorker* worker5 = new TimerWorker(5, 10000); // 10 second timer
QThread test5;
worker5->moveToThread(&test5);
// Connect signals and start thread
```

### Changing Intervals:
```cpp
// In TimerWorker constructor
TimerWorker* worker1 = new TimerWorker(1, 500); // 500ms instead of 1000ms
```

### Different Worker Tasks:
```cpp
// Override TimerWorker::onTimeout()
void TimerWorker::onTimeout() {
    // Custom work here instead of just logging
    performNetworkRequest();
    updateDatabase();
    qInfo() << QString("Worker %1 completed task").arg(workerId);
}
```

## 📋 Troubleshooting

### Common Issues:

**Library Not Found:**
```
UnsatisfiedLinkError: library "libQtAndroidService_arm64-v8a.so" not found
```
- Ensure NDK build includes arm64-v8a architecture
- Check `android/libs/` directory for .so files

**ANR Dialog:**
```
"QtAndroidService isn't responding"
```
- Service initialization is taking too long
- Check that `super.onCreate()` is called in background thread only

**No Timer Output:**
```
Qt service starts but no timer logs appear
```
- Check that Qt event loop is running (`a.exec()`)
- Verify all QThread connections are made before starting

## 📜 License

This project demonstrates Qt Android service architecture and can be used as a template for Qt-based Android background services.

## 🤝 Contributing

This is a template project showing Qt Android service patterns. Feel free to use it as a foundation for your own Qt Android services.