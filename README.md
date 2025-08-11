# Android Qt Service with QThreads and QTimers

This project demonstrates a complete Android service implementation using Qt 6.5.3 with multiple QThread workers running QTimer-based periodic tasks.

## Project Structure

```
AndroidQtService/
├── main.cpp                           # Main service implementation
├── CMakeLists.txt                     # Build configuration (recommended)
├── AndroidQtService.pro              # Alternative qmake build file
├── README.md                         # This file
└── android/                          # Android-specific files
    ├── AndroidManifest.xml           # App manifest
    ├── build.gradle                  # Gradle build configuration
    └── src/org/qtproject/example/    # Java service wrapper
        └── QtService.java
```

## Features

- **Multi-threaded Architecture**: Three worker threads with different timer intervals:
    - Data Processing: 2-second intervals
    - Network Operations: 5-second intervals
    - Maintenance Tasks: 30-second intervals

- **Thread-Safe Communication**: Uses Qt's signal/slot mechanism and QMutex for safe inter-thread communication

- **Android Service Integration**: Proper Android service lifecycle management with JNI bridge

- **Qt 6.5.3 Compatible**: Uses modern Qt APIs without deprecated QtAndroidExtras

## Build Instructions

### Prerequisites
- Qt 6.5.3 or later with Android support
- Android SDK (API 23+)
- Android NDK
- CMake 3.16+ (if using CMake build)

### Option 1: CMake Build (Recommended)
```bash
mkdir build
cd build
cmake .. -DQT_HOST_PATH=/path/to/qt -DCMAKE_TOOLCHAIN_FILE=/path/to/qt/lib/cmake/Qt6/qt.toolchain.cmake
cmake --build .
```

### Option 2: qmake Build
```bash
qmake AndroidQtService.pro
make
```

## Deployment

1. **Configure Qt Creator**:
    - Open the project in Qt Creator
    - Configure Android kit with proper SDK/NDK paths
    - Set target device/emulator

2. **Build and Deploy**:
    - Build the project for Android
    - Deploy to device/emulator
    - The service will start automatically

## Usage

### Starting the Service Programmatically
```java
Intent serviceIntent = new Intent(this, QtService.class);
startService(serviceIntent);
```

### Stopping the Service
```java
Intent serviceIntent = new Intent(this, QtService.class);
stopService(serviceIntent);
```

## Code Architecture

### Main Components

1. **Worker Class**: Encapsulates timer-based work execution
2. **WorkerThread Class**: Manages Worker lifecycle in separate threads
3. **AndroidQtService Class**: Main service coordinating all workers
4. **JNI Bridge**: Connects Android service lifecycle to Qt code

### Thread Safety
- Each worker thread has its own event loop
- Cross-thread communication via Qt's queued connections
- Shared data protected by QMutex locks

### Customization

To add new worker threads:

1. Create a new WorkerThread instance in `setupService()`
2. Connect its signals to appropriate slots
3. Add cleanup in `stopAllWorkers()`

To modify worker behavior:
- Override `doWork()` in the Worker class
- Adjust timer intervals in WorkerThread constructor
- Add custom signals/slots for specific communication needs

## Troubleshooting

### Common Issues

1. **Service Not Starting**:
    - Check AndroidManifest.xml permissions
    - Verify package name matches in all files
    - Check logcat for JNI loading errors

2. **Native Library Not Found**:
    - Ensure CMakeLists.txt target name matches AndroidManifest.xml lib_name
    - Check architecture compatibility (arm64-v8a, armeabi-v7a)

3. **Permissions**:
    - Add required permissions to AndroidManifest.xml
    - Request runtime permissions for API 23+

### Debug Tips

- Use `adb logcat | grep QtService` to monitor service logs
- Enable Qt logging with `QT_LOGGING_RULES="*.debug=true"`
- Check thread IDs in debug output to verify multi-threading

## License

This example is provided under the same license terms as Qt itself.

## Requirements

- Qt 6.5.3+
- Android API 23+
- Android NDK r21+
- CMake 3.16+ (for CMake builds)