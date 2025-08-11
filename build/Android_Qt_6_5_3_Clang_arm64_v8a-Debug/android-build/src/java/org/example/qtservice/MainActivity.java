package org.example.qtservice;

import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import org.qtproject.qt.android.bindings.QtActivity;

public class MainActivity extends QtActivity {
    private static final String TAG = "MainActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        System.setProperty("debug.qt", "true");

        // Initialize Java-side services

        Log.i(TAG, "Starting QtAndroidService from MainActivity...");

        Intent serviceIntent = new Intent(this, QtAndroidService.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(serviceIntent);
        } else {
            startService(serviceIntent);
        }
    }
}
