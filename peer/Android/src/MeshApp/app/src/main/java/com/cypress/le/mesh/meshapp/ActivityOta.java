/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 * Corporation. All rights reserved. This software, including source code, documentation and
 * related materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.cypress.le.mesh.meshapp.leotaapp.FileChooser;
import com.cypress.le.mesh.meshframework.IMeshControllerCallback;
import com.cypress.le.mesh.meshframework.MeshController;

import java.io.File;
import java.util.UUID;

public class ActivityOta extends AppCompatActivity implements LightingService.IServiceCallback {
    private static final String TAG = "ActivityOta";

    private static final int MSG_CONNECTION      =  0;
    private static final int MSG_NODE_CONNECTION =  1;
    private static final int MSG_START_OTA       =  2;
    private static final int MSG_OTA_STATUS      =  3;

    private static final int OTA_STATE_IDLE            = 0;
    private static final int OTA_STATE_NODE_CONNECTING = 1;
    private static final int OTA_STATE_UPLOADING       = 2;

    private String          mName     = null;
    private Activity        mActivity = null;
    private Context         mContext  = null;
    private LightingService mService = null;

    private int     mOtaState         = OTA_STATE_IDLE;
    private boolean mConnected        = false;
    private int     mPercentComplete  = 0;
    private String  mFirmwareFileName = null;

    private TextView    mTextUploadingStatus;
    private TextView    mTextFirmwareFilePath;
    private ImageButton mBtnFirmwareFile;
    private ImageButton mBtnStartOta;
    private ImageButton mBtnStopOta;
    private View        mViewConnected;
    private Button      mBtnConnect;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_ota);
        Intent intent = new Intent(this, LightingService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            mName = extras.getString("name");
        }

        Log.i(TAG, "onCreate: name = " + mName);

        mContext  = this;
        mActivity = this;

        mTextUploadingStatus  = (TextView) findViewById(R.id.text_ota_uploading_status);
        mTextFirmwareFilePath = (TextView) findViewById(R.id.text_ota_firmware_path);

        mBtnFirmwareFile   = (ImageButton) findViewById(R.id.btn_ota_firmware_file);
        mBtnStartOta       = (ImageButton) findViewById(R.id.btn_ota_start);
        mBtnStopOta        = (ImageButton) findViewById(R.id.btn_ota_stop);

        mViewConnected = (View) findViewById(R.id.view_ota_connected);
        mViewConnected.setBackgroundColor(getResources().getColor(R.color.blue));

        mBtnConnect = (Button) findViewById(R.id.btn_ota_connect_to_proxy);

        mBtnFirmwareFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Choose OTA Image file from the local storage
                FileChooser filechooser = new FileChooser(ActivityOta.this);
                filechooser.setFileListener(new FileChooser.FileSelectedListener() {
                    @Override
                    public void fileSelected(final File file) {
                        if (file.exists()) {
                            mFirmwareFileName = file.getAbsolutePath();
                            mTextFirmwareFilePath.setText(mFirmwareFileName);
                        }
                    }
                });
                filechooser.showDialog();
            }
        });

        mBtnStartOta.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Start OTA");
                if (mFirmwareFileName == null)
                {
                    Constants.Show(mActivity, "Please select firmware file first!", Toast.LENGTH_LONG);
                    return;
                }
                if (mOtaState != OTA_STATE_IDLE) {
                    Constants.Show(mActivity, "OTA started already!", Toast.LENGTH_LONG);
                    return;
                }
                if (!mConnected) {
                    Constants.Show(mActivity, "Please connect network first!", Toast.LENGTH_LONG);
                    return;
                }

                AlertDialog alertDialog = new AlertDialog.Builder(ActivityOta.this).create();
                alertDialog.setTitle("Mesh Device OTA");
                alertDialog.setMessage("\nStart OTA?\n\nFirmware: " + mFirmwareFileName);
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mMsgHandler.sendEmptyMessage(MSG_START_OTA);
                            dialog.dismiss();
                        }
                    });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    });
                alertDialog.show();
            }
        });

        mBtnStopOta.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Stop OTA");

                AlertDialog alertDialog = new AlertDialog.Builder(ActivityOta.this).create();
                alertDialog.setTitle("Stop OTA!");
                alertDialog.setMessage("\nDo you want to stop OTA?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mService.stopOta();
                            dialog.dismiss();
                        }
                    });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    });
                alertDialog.show();
            }
        });

        mBtnConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Connect to Network");
                if (mConnected) {
                    Constants.Show(mActivity, "Network connected already!", Toast.LENGTH_LONG);
                    return;
                }
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityOta.this).create();
                alertDialog.setTitle(getString(R.string.label_connect_to_network));
                alertDialog.setMessage(getString(R.string.label_connect_to_network_msg));
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            if (!Constants.IsLocationServiceEnabled(mContext)) {
                                Log.i(TAG, "Connect: LocationService not enabled");
                                Constants.Show(mActivity, "Please turn on the location!!!", Toast.LENGTH_LONG);
                            }
                            else {
                                Log.i(TAG, "Connect: LocationService enabled");
                                if (mService.connectToNetwork(Constants.TRANSPORT_GATT) == MeshController.MESH_CLIENT_SUCCESS) {
                                    mViewConnected.setBackgroundColor(getResources().getColor(R.color.blue));
                                    Constants.Show(mActivity, "Connecting...", Toast.LENGTH_LONG);
                                }
                            }
                        }
                    });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    });
                alertDialog.show();
            }
        });
    }

    @Override
    protected void onDestroy() {
        unbindService(mConnection);
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_light, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        try {
            return super.dispatchTouchEvent(ev);
        } catch (Exception e) {
            return false;
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName compname, IBinder service) {
            Log.i(TAG, "Bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            mService = binder.getService();
            mService.registerCb(ActivityOta.this);

            mConnected = mService.isConnectedToNetwork();
            setViewConnected(mConnected);
            mBtnConnect.setEnabled(!mConnected);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mService = null;
            mConnected = false;
        }
    };

    private Handler mMsgHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "handleMessage: " + msg.what);
            switch (msg.what) {
            case MSG_CONNECTION:
                handleMsgConnection(msg.arg1);
                break;
            case MSG_NODE_CONNECTION:
                handleMsgNodeConnection(msg.arg1);
                break;
            case MSG_START_OTA:
                handleMsgStartOta();
                break;
            case MSG_OTA_STATUS:
                handleMsgOtaStatus(msg.arg1, msg.arg2);
                break;
            }
        }
    };

    private void handleMsgConnection(int status) {
        String text = null;
        if (status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED) {
            mConnected = true;
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.green));
            text = "Connected to device";
        }
        else if (status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED) {
            mConnected = false;
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.red));
            text = "Disconnected from device";
        }
        mBtnConnect.setEnabled(!mConnected);
        if (text != null) {
            Constants.Show(mActivity, text, Toast.LENGTH_LONG);
        }
    }

    private void handleMsgNodeConnection(int status) {
        if (mOtaState == OTA_STATE_NODE_CONNECTING) {
            if (status == IMeshControllerCallback.MESH_CLIENT_NODE_CONNECTED) {
                handleMsgConnection(IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED);
                mTextUploadingStatus.setText(R.string.label_ota_node_connected);
                mService.startOta(mFirmwareFileName);
                mPercentComplete = 0;
            }
            else {
                handleMsgConnection(IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED);
                mOtaState = OTA_STATE_IDLE;
                mTextUploadingStatus.setText(R.string.label_ota_node_connect_failed);
            }
        }
    }

    private void handleMsgStartOta() {
        if (mService.connectComponent(mName, (byte)100) == MeshController.MESH_CLIENT_SUCCESS) {
            mOtaState = OTA_STATE_NODE_CONNECTING;
            mTextUploadingStatus.setText(R.string.label_ota_connecting_node);
        }
        else {
            mOtaState = OTA_STATE_IDLE;
        }
    }

    private void handleMsgOtaStatus(int status, int percentage) {
        if (status == IMeshControllerCallback.OTA_UPGRADE_STATUS_IN_PROGRESS) {
            if (mOtaState == OTA_STATE_NODE_CONNECTING) {
                mOtaState = OTA_STATE_UPLOADING;
            }
            if (mOtaState == OTA_STATE_UPLOADING) {
                if (mPercentComplete != percentage) {
                    mPercentComplete = percentage;
                    mTextUploadingStatus.setText("Status: uploading " + percentage + "%");
                }
            }
        }
        else if (status == IMeshControllerCallback.OTA_UPGRADE_STATUS_COMPLETED) {
            if (mOtaState == OTA_STATE_UPLOADING) {
                mTextUploadingStatus.setText("Status: OTA upgrade completed");
            }
            if (mOtaState != OTA_STATE_IDLE) {
                mOtaState = OTA_STATE_IDLE;
            }
        }
        else {
            if (mOtaState == OTA_STATE_UPLOADING) {
                mTextUploadingStatus.setText("Status: OTA failed " + status);
            }
            if (mOtaState != OTA_STATE_IDLE) {
                mOtaState = OTA_STATE_IDLE;
            }
        }
    }

    private void setViewConnected(boolean connected) {
        if (connected) {
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.green));
        }
        else {
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.red));
        }
    }

    @Override
    public void onNetworkConnectionStatusChanged(final byte transport, final byte status) {
        Log.i(TAG, "onNetworkConnectionStatusChanged: transport = " + transport + ", status = " + status);
        Message msg = mMsgHandler.obtainMessage(MSG_CONNECTION, status);
        mMsgHandler.sendMessage(msg);
    }

    @Override
    public void onNodeConnStateChanged(final byte status, final String componentName) {
        Log.i(TAG, "onNodeConnStateChanged: status:" + status + " name = " + componentName);
        Message msg = mMsgHandler.obtainMessage(MSG_NODE_CONNECTION, status, 0);
        mMsgHandler.sendMessage(msg);
    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {
        Log.i(TAG, "onOtaStatus: status:" + status + " percentComplete:" + percentComplete);
        Message msg = mMsgHandler.obtainMessage(MSG_OTA_STATUS, status, percentComplete);
        mMsgHandler.sendMessage(msg);
    }

    @Override
    public void onMeshServiceStatusChangeCb(int status) {
    }

    @Override
    public void onDeviceFound(UUID uuid, String name) {
    }

    @Override
    public void onProvisionComplete(UUID device, byte status) {
    }

    @Override
    public void onHslStateChanged(String deviceName, final int lightness, int hue, int saturation, int remainingTime) {
    }

    @Override
    public void onOnOffStateChanged(String deviceName, byte targetOnOff, byte presentOnOff, int remainingTime) {
    }

    @Override
    public void onLevelStateChanged(String deviceName, short targetLevel, short presentLevel, int remainingTime) {
    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {
    }

    @Override
    public void onNetworkOpenedCallback(byte status) {
    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, final String componentInfoStr) {
    }

    @Override
    public void onDfuStatus(byte state, byte[] data) {
    }

    @Override
    public void onSensorStatusCb(String componentName, int propertyId, byte[] data) {
    }

    @Override
    public void onVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen) {
    }

    @Override
    public void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime) {
    }

    @Override
    public void onLightLcModeStatus(String componentName, int mode) {
    }

    @Override
    public void onLightLcOccupancyModeStatus(String componentName, int mode) {
    }

    @Override
    public void onLightLcPropertyStatus(String componentName, int propertyId, int value) {
    }
}
