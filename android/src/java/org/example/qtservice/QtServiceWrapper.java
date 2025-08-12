package org.qtproject.qtservice;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.QtServiceDelegate;
import org.qtproject.qt.android.bindings.QtService;

/**
 * Generic Qt Android Service wrapper for QtAndroidService library.
 * This class can be extended by third-party applications to integrate
 * the Qt timer service functionality.
 */
public class QtServiceWrapper extends QtService {
    private static final String TAG = "QtServiceWrapper";
    private static final String DEFAULT_CHANNEL_ID = "qt_service_library_channel";
    private static final int NOTIFICATION_ID = 1;

    private QtServiceDelegate m_delegate;
    private static boolean qtInitialized = false;
    private static boolean qtStarted = false;
    private Handler mainHandler;

    // Configuration methods - can be overridden by subclass or consuming app
    protected String getLibraryName() {
        return "QtAndroidService"; // Default library name
    }

    protected String getServiceTitle() {
        return "Qt Service Library";
    }

    protected String getChannelName() {
        return "Qt Background Service";
    }

    protected String getChannelId() {
        return DEFAULT_CHANNEL_ID;
    }

    protected String getPackageName() {
        return super.getPackageName();
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "=== QtServiceWrapper onCreate (LIBRARY MODE) ===");
        Log.d(TAG, "Process ID: " + android.os.Process.myPid());
        Log.d(TAG, "Package: " + getPackageName());

        mainHandler = new Handler(Looper.getMainLooper());

        // Start foreground service IMMEDIATELY
        Log.d(TAG, "Starting foreground service immediately...");
        createNotificationChannel();

        Notification notification = createServiceNotification("Initializing Qt service library...");
        startForeground(NOTIFICATION_ID, notification);
        Log.d(TAG, "✓ Foreground service started");

        // Schedule Qt initialization to happen AFTER onCreate returns
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (!qtStarted) {
                    qtStarted = true;
                    startQtInitialization();
                }
            }
        });

        Log.d(TAG, "onCreate completed immediately (Qt will start shortly)");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "=== QtServiceWrapper onStartCommand (LIBRARY MODE) ===");

        // Ensure foreground service is active
        createNotificationChannel();

        String statusText = qtInitialized ?
                "Qt service library active - timer threads running" :
                qtStarted ? "Qt library initializing..." : "Starting Qt service library...";

        Notification notification = createServiceNotification(statusText);
        startForeground(NOTIFICATION_ID, notification);

        // Schedule Qt start if not already started
        if (!qtStarted) {
            qtStarted = true;
            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    startQtInitialization();
                }
            });
        }

        Log.d(TAG, "onStartCommand completed immediately");
        return START_STICKY; // Restart if killed by system
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "=== QtServiceWrapper onDestroy ===");

        try {
            // Stop foreground service
            stopForeground(true);

            // Clean up Qt if initialized
            if (m_delegate != null && qtInitialized) {
                Log.d(TAG, "Cleaning up Qt library runtime...");
                try {
                    QtNative.quitQtCoreApplication();
                    Thread.sleep(1000);
                } catch (Exception e) {
                    Log.w(TAG, "Exception during Qt cleanup: " + e.getMessage());
                }
                m_delegate = null;
            }

            qtInitialized = false;
            qtStarted = false;
            Log.d(TAG, "Qt service library cleanup completed");

        } catch (Exception e) {
            Log.e(TAG, "Error during Qt service library cleanup", e);
        } finally {
            // Don't call super.onDestroy() until Qt is cleaned up
            if (qtInitialized) {
                super.onDestroy();
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        // Return null for started services (not bound services)
        return null;
    }

    private void startQtInitialization() {
        Log.d(TAG, "Starting Qt library initialization in background thread...");

        // Run Qt initialization in a separate thread
        new Thread(new Runnable() {
            @Override
            public void run() {
                initializeQt();
            }
        }).start();
    }

    private void initializeQt() {
        Log.d(TAG, "Qt library initialization thread started");

        try {
            updateNotification("Loading Qt service library...");

            // Enable Qt logging
            System.setProperty("QT_LOGGING_RULES", "*=true");
            System.setProperty("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
            System.setProperty("QT_ANDROID_PLAIN_LOG", "1");

            // Debug library loading
            updateNotification("Attempting to load Qt library: " + getLibraryName());
            if (!debugLibraryLoading()) {
                return; // Exit if no library could be loaded
            }

            updateNotification("Initializing Qt service runtime...");

            // Initialize Qt service delegate
            m_delegate = new QtServiceDelegate();

            Bundle extras = new Bundle();
            extras.putString("android.app.lib_name", getLibraryName());
            extras.putString("android.app.arguments", "-v");
            extras.putBoolean("android.app.background_running", true);
            extras.putString("android.app.extract_android_style", "none");
            extras.putString("qt.qpa.platform", "offscreen");
            extras.putBoolean("android.app.qt_no_gui", true);

            Log.d(TAG, "Loading Qt service application...");
            m_delegate.loadApplication(this, getClassLoader(), extras);

            Log.d(TAG, "Setting Qt native service...");
            QtNative.setService(this, m_delegate);

            updateNotification("Starting Qt service application...");

            // Call super.onCreate() in background thread
            Log.d(TAG, "Calling super.onCreate() for Qt service initialization...");
            super.onCreate();

            qtInitialized = true;
            Log.d(TAG, "✓ Qt service library initialization completed successfully");

            // Update notification to show success
            updateNotification("Qt service library active - background service running");

        } catch (Exception e) {
            Log.e(TAG, "Qt service library initialization failed", e);
            updateNotification("ERROR: Qt service library initialization failed - " + e.getMessage());
        }
    }

    private boolean debugLibraryLoading() {
        Log.d(TAG, "=== DEBUGGING QT LIBRARY LOADING ===");

        // Debug application info
        Log.d(TAG, "Package: " + getPackageName());
        Log.d(TAG, "Native lib dir: " + getApplicationInfo().nativeLibraryDir);
        Log.d(TAG, "Library name: " + getLibraryName());

        // Try multiple library name variations for flexibility
        String[] libraryVariations = {
                getLibraryName() + "_arm64-v8a",  // Architecture-specific
                getLibraryName(),                 // Base name
                "lib" + getLibraryName() + "_arm64-v8a", // Full name variation
                "lib" + getLibraryName()          // Lib prefix variation
        };

        for (String libName : libraryVariations) {
            try {
                Log.d(TAG, "Attempting to load: " + libName);
                System.loadLibrary(libName);
                Log.d(TAG, "✓ SUCCESS: Loaded Qt library: " + libName);
                updateNotification("Qt library loaded: " + libName);
                return true;
            } catch (UnsatisfiedLinkError e) {
                Log.d(TAG, "✗ FAILED: " + libName + " - " + e.getMessage());
            }
        }

        Log.e(TAG, "CRITICAL: No Qt service libraries could be loaded!");
        Log.e(TAG, "Tried variations: " + java.util.Arrays.toString(libraryVariations));
        updateNotification("ERROR: Qt service library not found");
        return false;
    }

    private Notification createServiceNotification(String statusText) {
        Notification.Builder builder;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, getChannelId());
        } else {
            builder = new Notification.Builder(this);
        }

        return builder
                .setContentTitle(getServiceTitle())
                .setContentText(statusText)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setOngoing(true)
                .setPriority(Notification.PRIORITY_LOW)
                .build();
    }

    private void updateNotification(String statusText) {
        try {
            Notification notification = createServiceNotification(statusText);
            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (manager != null) {
                manager.notify(NOTIFICATION_ID, notification);
                Log.d(TAG, "✓ Notification updated: " + statusText);
            }
        } catch (Exception e) {
            Log.w(TAG, "Failed to update notification", e);
        }
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    getChannelId(),
                    getChannelName(),
                    NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Qt-based background service library");
            channel.setShowBadge(false);

            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (manager != null) {
                manager.createNotificationChannel(channel);
                Log.d(TAG, "✓ Notification channel created: " + getChannelName());
            }
        }
    }
}