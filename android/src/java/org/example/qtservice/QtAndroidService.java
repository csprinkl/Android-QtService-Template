package org.example.qtservice;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.QtServiceDelegate;
import org.qtproject.qt.android.bindings.QtService;

public class QtAndroidService extends QtService {
    private static final String TAG = "QtAndroidService";
    private static final String CHANNEL_ID = "android_qtservice_channel";

    private QtServiceDelegate m_delegate;

    @Override
    public void onCreate() {
        Log.d(TAG, "Service created");

        createNotificationChannel();

        Notification notification = new Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("Android QtService Running")
                .setContentText("Your background service is active.")
                .setSmallIcon(android.R.drawable.ic_media_play)
                .build();

        startForeground(1, notification);

        try {
            Log.d(TAG, "Starting Qt runtime...");
            m_delegate = new QtServiceDelegate();

            // Pass optional command-line args to your Qt app here:
            Bundle extras = new Bundle();
            extras.putString("applicationArguments", "-v");  // Add more if needed

            m_delegate.loadApplication(this, getClassLoader(), extras);

            QtNative.setService(this, m_delegate);
            Log.d(TAG, "Qt runtime started successfully");
        } catch (Throwable t) {
            Log.e(TAG, "Failed to start Qt runtime", t);
        }
    }


    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID,
                    "Android Qt Background Service",
                    NotificationManager.IMPORTANCE_LOW
            );
            NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }
}
