package org.example.qtservice;

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

public class QtAndroidService extends QtService {
    private static final String TAG = "QtAndroidService";
    private static final String CHANNEL_ID = "qt_timer_service_channel";
    private static final int NOTIFICATION_ID = 1;

    private QtServiceDelegate m_delegate;
    private static boolean qtInitialized = false;
    private static boolean qtStarted = false;
    private Handler mainHandler;

    @Override
    public void onCreate() {
        Log.d(TAG, "=== QtAndroidService onCreate (FAST EXIT) ===");
        Log.d(TAG, "Process ID: " + android.os.Process.myPid());

        mainHandler = new Handler(Looper.getMainLooper());

        // CRITICAL: Start foreground service IMMEDIATELY
        Log.d(TAG, "Starting foreground service immediately...");
        createNotificationChannel();

        Notification notification = createServiceNotification("Qt Timer Service starting...");
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
        // onCreate returns immediately - no blocking!
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "=== QtAndroidService onStartCommand (FAST EXIT) ===");

        // Ensure foreground service is active
        createNotificationChannel();

        String statusText = qtInitialized ?
                "Running 4 timer threads (1s, 2s, 3s, 5s intervals)" :
                qtStarted ? "Qt initializing in background..." : "Qt Timer Service starting...";

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
        Log.d(TAG, "=== QtAndroidService onDestroy ===");

        try {
            // Stop foreground service
            stopForeground(true);

            // Clean up Qt if initialized
            if (m_delegate != null && qtInitialized) {
                Log.d(TAG, "Cleaning up Qt runtime...");
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
            Log.d(TAG, "Service cleanup completed");

        } catch (Exception e) {
            Log.e(TAG, "Error during service cleanup", e);
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
        Log.d(TAG, "Starting Qt initialization in background thread...");

        // Run Qt initialization in a separate thread
        new Thread(new Runnable() {
            @Override
            public void run() {
                initializeQt();
            }
        }).start();
    }

    private void initializeQt() {
        Log.d(TAG, "Qt initialization thread started");

        try {
            updateNotification("Loading Qt libraries...");

            // Enable Qt logging
            System.setProperty("QT_LOGGING_RULES", "*=true");
            System.setProperty("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
            System.setProperty("QT_ANDROID_PLAIN_LOG", "1");

            // Debug library loading - try multiple variations
            updateNotification("Attempting to load Qt library...");
            if (!debugLibraryLoading()) {
                return; // Exit if no library could be loaded
            }

            updateNotification("Initializing Qt runtime...");

            // Initialize Qt service delegate
            m_delegate = new QtServiceDelegate();

            Bundle extras = new Bundle();
            extras.putString("android.app.lib_name", "QtAndroidService");
            extras.putString("android.app.arguments", "-v");
            extras.putBoolean("android.app.background_running", true);
            extras.putString("android.app.extract_android_style", "none");
            extras.putString("qt.qpa.platform", "offscreen");
            extras.putBoolean("android.app.qt_no_gui", true);

            Log.d(TAG, "Loading Qt application...");
            m_delegate.loadApplication(this, getClassLoader(), extras);

            Log.d(TAG, "Setting Qt native service...");
            QtNative.setService(this, m_delegate);

            updateNotification("Starting Qt application...");

            // CRITICAL: Call super.onCreate() in background thread
            Log.d(TAG, "Calling super.onCreate() for Qt initialization...");
            super.onCreate();

            qtInitialized = true;
            Log.d(TAG, "✓ Qt initialization completed successfully");

            // Update notification to show success
            updateNotification("Qt Timer Service active - 4 timer threads running");

        } catch (Exception e) {
            Log.e(TAG, "Qt initialization failed", e);
            updateNotification("ERROR: Qt initialization failed - " + e.getMessage());
        }
    }

    private boolean debugLibraryLoading() {
        Log.d(TAG, "=== DEBUGGING LIBRARY LOADING ===");

        // Debug application info
        Log.d(TAG, "Package: " + getPackageName());
        Log.d(TAG, "Native lib dir: " + getApplicationInfo().nativeLibraryDir);

        try {
            Log.d(TAG, "Attempting to load: " + "QtAndroidService_arm64-v8a");
            System.loadLibrary("QtAndroidService_arm64-v8a");
            Log.d(TAG, "✓ SUCCESS: Loaded library: " + "QtAndroidService_arm64-v8a");
            updateNotification("Qt library loaded: " + "QtAndroidService_arm64-v8a");
            return true;
        } catch (UnsatisfiedLinkError e) {
            Log.d(TAG, "✗ FAILED: " + "QtAndroidService_arm64-v8a" + " - " + e.getMessage());
        }

        Log.e(TAG, "CRITICAL: No Qt libraries could be loaded!");
        updateNotification("ERROR: No Qt libraries found");
        return false;
    }

    private Notification createServiceNotification(String statusText) {
        Notification.Builder builder;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID);
        } else {
            builder = new Notification.Builder(this);
        }

        return builder
                .setContentTitle("Qt Timer Service")
                .setContentText(statusText)
                .setSmallIcon(android.R.drawable.ic_media_play)
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
                    CHANNEL_ID,
                    "Qt Timer Service",
                    NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Background Qt timer service with multiple threads");
            channel.setShowBadge(false);

            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (manager != null) {
                manager.createNotificationChannel(channel);
                Log.d(TAG, "✓ Notification channel created");
            }
        }
    }
}