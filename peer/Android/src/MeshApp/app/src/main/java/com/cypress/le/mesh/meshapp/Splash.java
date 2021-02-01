/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 * Corporation. All rights reserved. This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection
 * (United States and foreign), United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software ("EULA"). If no EULA applies, Cypress
 * hereby grants you a personal, nonexclusive, non-transferable license to  copy, modify, and
 * compile the Software source code solely for use in connection with Cypress's  integrated circuit
 * products. Any reproduction, modification, translation, compilation,  or representation of this
 * Software except as specified above is prohibited without the express written permission of
 * Cypress. Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO  WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING,  BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to make changes to
 * the Software without notice. Cypress does not assume any liability arising out of the application
 * or use of the Software or any product or circuit  described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or failure of the
 * Cypress product may reasonably be expected to result  in significant property damage, injury
 * or death ("High Risk Product"). By including Cypress's product in a High Risk Product, the
 * manufacturer of such system or application assumes  all risk of such use and in doing so agrees
 * to indemnify Cypress against all liability.
 */
package com.cypress.le.mesh.meshapp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import androidx.core.app.ActivityCompat;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.util.Log;

import java.util.ArrayList;


public class Splash extends AppCompatActivity {
    private static final String TAG = "Splash";

    private static final int BT_ENABLE_REQUEST_CODE = 10;
    private static final int PERM_REQUEST_CODE      = 11;

    private static int SPLASH_TIME_OUT = 1250;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");

        setContentView(R.layout.activity_splash);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().hide();

        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, BT_ENABLE_REQUEST_CODE);
        }
        else if (!requestPermissions()) {
            startMeshLighting();
        }
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == BT_ENABLE_REQUEST_CODE && resultCode == RESULT_OK) {
            if (!requestPermissions()) {
                startMeshLighting();
            }
        }
        else {
            // Request again
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, BT_ENABLE_REQUEST_CODE);
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        Log.d(TAG, "onRequestPermissionsResult: requestCode = " + requestCode +
              ", permissions.length = " + permissions.length +
              ", grantResults.length = " + grantResults.length);

        if (requestCode != PERM_REQUEST_CODE || permissions.length != grantResults.length || permissions.length == 0) {
            return;
        }

        boolean permissionGranted = true;
        for (int i = 0; i < permissions.length; i++) {
            Log.d(TAG, "    permissions[" + i + "] = " + permissions[i] + ", grant = "
                  + grantResults[i]);
            if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                permissionGranted = false;
            }
        }

        if (permissionGranted) {
            startMeshLighting();
        }
        else if (!requestPermissions()) {
            startMeshLighting();
        }
    }

    // Check application for the required permissions:
    //   Since Android 6.0, LE Scan need location permissions
    private boolean requestPermissions() {
        ArrayList<String> permissionList = new ArrayList<String>();
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP_MR1) {
            if (checkSelfPermission(Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.BLUETOOTH);
            }
            if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.CAMERA);
            }
            if (checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.ACCESS_COARSE_LOCATION);
            }

            if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
            }
            if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            }
            if (checkSelfPermission(Manifest.permission.GET_ACCOUNTS) != PackageManager.PERMISSION_GRANTED) {
                permissionList.add(Manifest.permission.GET_ACCOUNTS);
            }
        }

        if (permissionList.size() > 0) {
            Log.d(TAG, "requestPermissions: permissionList size = " + permissionList.size());

            String[] permissions = permissionList.toArray(new String[permissionList.size()]);
            Log.d(TAG, "requestPermissions: permissions length = " + permissions.length);
            for (int i = 0; i < permissions.length; i++) {
                Log.d(TAG, "requestPermissions: permissions[" + i + "] = " + permissions[i]);
            }

            ActivityCompat.requestPermissions(this, permissions, PERM_REQUEST_CODE);
            return true;
        }
        return false;
    }

    private void startMeshLighting() {
        Log.d(TAG, "startMeshLighting");

        Intent intent = new Intent(this, LightingService.class);
        startService(intent);

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Intent intent = new Intent(getApplicationContext(), MainActivity.class);
                startActivity(intent);
                finish();
            }

        },SPLASH_TIME_OUT);
    }
}
