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

# Dependencies To Be Added — Proof‑of‑Concept Checklist

A compact, **drag‑and‑drop** list of tasks to wire additional libraries into the Qt Android Timer Service template.  
Each section includes **Build System**, **Permissions/Runtime**, **Code Smoke Test**, **Tests**, and a **Definition of Done**.

> Tip: Check items off as you go. Replace placeholder paths and package names with your own.

---

## Qt: Concurrent

- [ ] **Build system**
  - [ ] **qmake**: add to `.pro`  
    ```pro
    QT += concurrent
    ```
  - [ ] **CMake**: add to `find_package` and link target  
    ```cmake
    find_package(Qt6 REQUIRED COMPONENTS Concurrent)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Concurrent)
    ```
- [ ] **Code (smoke test)**  
  Replace one `QTimer` worker with a `QtConcurrent::run` task and log start/finish.
  ```cpp
  #include <QtConcurrent>
  auto task = QtConcurrent::run([]{
      qInfo() << "Concurrent task running";
      QThread::sleep(1);
      qInfo() << "Concurrent task done";
  });
  ```
- [ ] **Tests**
  - [ ] Verify task runs and can be canceled/aborted cleanly.
  - [ ] Confirm log output appears without blocking the event loop.
- [ ] **Definition of Done**  
  Concurrent APIs are linked and used in at least one production code path.

---

## Qt: Multimedia

- [ ] **Build system**
  - [ ] **qmake**  
    ```pro
    QT += multimedia
    ```
  - [ ] **CMake**  
    ```cmake
    find_package(Qt6 REQUIRED COMPONENTS Multimedia)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Multimedia)
    ```
- [ ] **Permissions (AndroidManifest.xml)**
  - [ ] Add **only if needed**:
    ```xml
    <uses-permission android:name="android.permission.RECORD_AUDIO"/>
    <uses-permission android:name="android.permission.CAMERA"/>
    <!-- Android 13+ notifications (if you show playback notifications) -->
    <uses-permission android:name="android.permission.POST_NOTIFICATIONS"/>
    ```
- [ ] **Runtime**
  - [ ] Request mic/camera permissions before first use (Android 6+).
- [ ] **Code (smoke test)**  
  Play a short media file or generate a tone; log state changes.
- [ ] **Tests**
  - [ ] Verify playback/recording works on device and survives background.
- [ ] **Definition of Done**  
  Playback or record test runs without crashes; permissions requested correctly.

---

## Qt: Positioning

- [ ] **Build system**
  - [ ] **qmake**  
    ```pro
    QT += positioning
    ```
  - [ ] **CMake**  
    ```cmake
    find_package(Qt6 REQUIRED COMPONENTS Positioning)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Positioning)
    ```
- [ ] **Permissions (AndroidManifest.xml)**
  ```xml
  <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION"/>
  <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
  <!-- Optional if you need background location; follow Play policy carefully -->
  <!-- <uses-permission android:name="android.permission.ACCESS_BACKGROUND_LOCATION"/> -->
  ```
- [ ] **Runtime**
  - [ ] Request location permissions before first use.
- [ ] **Code (smoke test)**  
  Use `QGeoPositionInfoSource` to log periodic location updates.
- [ ] **Tests**
  - [ ] Confirm at least one location fix arrives with proper permission flow.
- [ ] **Definition of Done**  
  Location updates logged; graceful handling when permissions are denied.

---

## Qt: SerialPort

- [ ] **Build system**
  - [ ] **qmake**  
    ```pro
    QT += serialport
    ```
  - [ ] **CMake**  
    ```cmake
    find_package(Qt6 REQUIRED COMPONENTS SerialPort)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::SerialPort)
    ```
- [ ] **Manifest/Features**
  ```xml
  <uses-feature android:name="android.hardware.usb.host" android:required="false"/>
  ```
- [ ] **Runtime**
  - [ ] If using USB‑serial, implement the USB permission grant flow with `UsbManager` intent.
- [ ] **Code (smoke test)**
  - [ ] Open a serial device (loopback or test fixture), write a short frame, and log the echo/read.
- [ ] **Tests**
  - [ ] Exercise open/read/write and permission grant; verify hot‑plug/disconnect behavior.
- [ ] **Definition of Done**  
  Successful open/read/write with correct permission handling.

---

## Qt: Sql

- [ ] **Build system**
  - [ ] **qmake**  
    ```pro
    QT += sql
    ```
  - [ ] **CMake**  
    ```cmake
    find_package(Qt6 REQUIRED COMPONENTS Sql)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Sql)
    ```
- [ ] **Drivers/Plugins**
  - [ ] Ensure the SQLite driver is available. For static builds, link the plugin.
- [ ] **Code (smoke test)**
  ```cpp
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/test.db");
  if (db.open()) {
      QSqlQuery q; q.exec("create table if not exists t(x int)");
      q.exec("insert into t(x) values(1)");
  }
  ```
- [ ] **Tests**
  - [ ] CRUD succeeds; DB file visible under the app sandbox.
- [ ] **Definition of Done**  
  Basic CRUD works; errors logged with actionable messages.

---

## Qt: HttpServer

> Prefer CMake for this module.

- [ ] **Build system (CMake)**
  ```cmake
  find_package(Qt6 REQUIRED COMPONENTS HttpServer)
  target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::HttpServer)
  ```
- [ ] **Permissions**
  ```xml
  <uses-permission android:name="android.permission.INTERNET"/>
  ```
- [ ] **Code (smoke test)**
  Start a minimal HTTP server on `127.0.0.1:<port>` that responds `"OK"`.
- [ ] **Tests**
  ```bash
  adb shell curl http://127.0.0.1:<port>/
  ```
- [ ] **Definition of Done**  
  Local HTTP endpoint responds while the service runs.

---

## OpenCV

- [ ] **Choose path**
  - [ ] Native SDK (recommended; `opencv_java4.so` / module `.so`s)
  - [ ] Java AAR (use if Java‑side APIs are needed)
- [ ] **qmake (native)**
  ```bash
  # environment
  export OPENCV_SDK=/abs/path/OpenCV-android-sdk
  ```
  ```pro
  INCLUDEPATH += $$OPENCV_SDK/sdk/native/jni/include
  ANDROID_EXTRA_LIBS += $$OPENCV_SDK/sdk/native/libs/arm64-v8a/libopencv_java4.so
  ```
- [ ] **CMake (native)**
  ```cmake
  set(OpenCV_DIR "/abs/path/OpenCV-android-sdk/sdk/native/jni")
  find_package(OpenCV REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
  ```
- [ ] **Code (smoke test)**
  ```cpp
  #include <opencv2/core.hpp>
  cv::Mat m(2,2,CV_8UC1, cv::Scalar(7));
  qInfo() << "OpenCV sum =" << cv::sum(m)[0];
  ```
- [ ] **Definition of Done**  
  OpenCV symbols resolve at runtime; smoke test logs expected values.

---

## LicenseSpring

- [ ] Native C/C++ (link from C++)
- [ ] **Permissions**
  ```xml
  <uses-permission android:name="android.permission.INTERNET"/>
  ```
- [ ] **CMake (native path)**
  ```cmake
  add_library(licensespring STATIC IMPORTED)
  set_target_properties(licensespring PROPERTIES
      IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/main/cpp/libs/${ANDROID_ABI}/liblicensespring.a")
  target_link_libraries(${PROJECT_NAME} PRIVATE licensespring)
  ```
- [ ] **Code**
  - [ ] Initialize/activate at service startup (before worker threads).
  - [ ] Handle offline grace and error codes; log status.
- [ ] **Definition of Done**  
  Activation succeeds and status is logged; protected features remain blocked when unlicensed.

---

## Global Notes

- [ ] Ensure ABI coverage matches Gradle `abiFilters`.
- [ ] For **static Qt**: include required **plugins** (e.g., SQLite, multimedia backends).
- [ ] Test on a **physical device** for Multimedia/Serial/Positioning.
- [ ] Keep **log tags** consistent for easy `logcat` filtering.




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
