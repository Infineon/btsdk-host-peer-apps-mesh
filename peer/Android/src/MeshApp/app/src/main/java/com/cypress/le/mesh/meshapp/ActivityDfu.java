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
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.cypress.le.mesh.meshapp.leotaapp.FileChooser;
import com.cypress.le.mesh.meshframework.IMeshControllerCallback;
import com.cypress.le.mesh.meshframework.MeshController;

import java.io.File;
import java.util.UUID;

public class ActivityDfu extends AppCompatActivity implements LightingService.IServiceCallback {
    private static final String TAG = "ActivityDfu";

    private static final int MSG_CONNECTION      =  0;
    private static final int MSG_DFU_START       =  1;
    private static final int MSG_DFU_STOP        =  2;
    private static final int MSG_DFU_GET_STATUS  =  3;
    private static final int MSG_OTA_STATUS      =  4;
    private static final int MSG_DFU_STATUS      =  5;

    private static final int DFU_STATE_IDLE      = 0;
    private static final int DFU_STATE_SETUP_DFU = 1;
    private static final int DFU_STATE_UPLOADING = 2;  // Initiator -> Distributor
    private static final int DFU_STATE_UPDATING  = 3;  // Distributor -> Updating Node

    private static final int DFU_STATUS_INTERVAL = 10;


    private Activity        mActivity = null;
    private Context         mContext  = null;
    private LightingService mService = null;

    private byte    mDfuMethod = 0;
    private int     mDfuState = DFU_STATE_IDLE;
    private int     mDfuAction;

    private String  mJsonFileName = null;
//    private String  mFirmwareFileName = null;
//    private String  mMetadataFileName = null;
    private boolean mConnected        = false;

    private int     mPercentComplete  = 0;

    private TextView    mTextUploadingStatus;
    private TextView    mTextDfuStatus;

    private Spinner mDfuType;

    private TextView    mTextJsonFilePath;
//    private TextView    mTextFirmwareFilePath;

    //    private TextView    mTextMetadataFilePath;
    private ImageButton mBtnJsonFile;
//    private ImageButton mBtnFirmwareFile;
//    private ImageButton mBtnMetadataFile;
    private ImageButton mBtnDfuStart;
    private ImageButton mBtnDfuStop;
    private ImageButton mBtnDfuGetStatus;
    private Switch      mSwitchDfuStatus;
    private EditText    mEditDfuStatusInterval;
    private View        mViewConnected;
    private Button      mBtnConnect;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_dfu);
        Intent intent = new Intent(this, LightingService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        Log.i(TAG, "onCreate");

        mContext  = this;
        mActivity = this;

        mTextUploadingStatus   = (TextView) findViewById(R.id.text_dfu_uploading_status);
        mTextDfuStatus         = (TextView) findViewById(R.id.text_dfu_status);

        mDfuType = (Spinner) findViewById(R.id.dfu_type);

        mTextJsonFilePath  = (TextView) findViewById(R.id.text_dfu_json_path);
//        mTextFirmwareFilePath  = (TextView) findViewById(R.id.text_dfu_firmware_path);
//        mTextMetadataFilePath  = (TextView) findViewById(R.id.text_dfu_metadata_path);

        mBtnJsonFile    = (ImageButton) findViewById(R.id.btn_dfu_json_file);
//        mBtnFirmwareFile    = (ImageButton) findViewById(R.id.btn_dfu_firmware_file);
//        mBtnMetadataFile    = (ImageButton) findViewById(R.id.btn_dfu_metadata_file);
        mBtnDfuStart        = (ImageButton) findViewById(R.id.btn_dfu_start);
        mBtnDfuStop         = (ImageButton) findViewById(R.id.btn_dfu_stop);
        mBtnDfuGetStatus    = (ImageButton) findViewById(R.id.btn_dfu_get_status);

        mSwitchDfuStatus       = (Switch)   findViewById(R.id.switch_dfu_status);
        mEditDfuStatusInterval = (EditText) findViewById(R.id.edit_dfu_status_interval);
        mViewConnected = (View) findViewById(R.id.view_dfu_connected);
        mBtnConnect  = (Button) findViewById(R.id.btn_dfu_connect_to_proxy);

        mDfuType.setSelection(0);
        mDfuMethod = 0;

        mDfuType.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                mDfuMethod = (byte) i;
                Log.d(TAG, "DFU Method"+i);

            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {
            }
        });

        mBtnJsonFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Choose OTA Image file from the local storage
                FileChooser filechooser = new FileChooser(ActivityDfu.this);
                filechooser.setFileListener(new FileChooser.FileSelectedListener() {
                    @Override
                    public void fileSelected(final File file) {
                        if (file.exists()) {
                            mJsonFileName = file.getAbsolutePath();
                            mTextJsonFilePath.setText(mJsonFileName);
                        }
                    }
                });
                filechooser.showDialog();
            }
        });

//        mBtnFirmwareFile.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Choose OTA Image file from the local storage
//                FileChooser filechooser = new FileChooser(ActivityDfu.this);
//                filechooser.setFileListener(new FileChooser.FileSelectedListener() {
//                    @Override
//                    public void fileSelected(final File file) {
//                        if (file.exists()) {
//                            mFirmwareFileName = file.getAbsolutePath();
//                            mTextFirmwareFilePath.setText(mFirmwareFileName);
//                        }
//                    }
//                });
//                filechooser.showDialog();
//            }
//        });

//        mBtnMetadataFile.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Choose OTA Metadata file from the local storage
//                FileChooser filechooser = new FileChooser(ActivityDfu.this);
//                filechooser.setFileListener(new FileChooser.FileSelectedListener() {
//                    @Override
//                    public void fileSelected(final File file) {
//                        if (file.exists()) {
//                            mMetadataFileName = file.getAbsolutePath();
//                            mTextMetadataFilePath.setText(mMetadataFileName);
//                        }
//                    }
//                });
//                filechooser.showDialog();
//            }
//        });

        mSwitchDfuStatus.setChecked(true);
        mSwitchDfuStatus.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, final boolean isChecked) {
                mEditDfuStatusInterval.setEnabled(isChecked);
                Log.i(TAG, "DfuStatusEnabled = " + isChecked);
            }
        });

        if (mSwitchDfuStatus.isChecked()) {
            mEditDfuStatusInterval.setEnabled(true);
        }
        else {
            mEditDfuStatusInterval.setEnabled(false);
        }

        mBtnDfuStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Start DFU");
                if (mJsonFileName == null ) {
//                    if (mFirmwareFileName == null || mMetadataFileName == null) {
                    Constants.Show(mActivity, "Please select json file first!", Toast.LENGTH_LONG);
                    return;
                }
                if (mDfuState != DFU_STATE_IDLE) {
                    Constants.Show(mActivity, "DFU started already!", Toast.LENGTH_LONG);
                    return;
                }
                if (!mConnected) {
                    Constants.Show(mActivity, "Please connect network first!", Toast.LENGTH_LONG);
                    return;
                }

                AlertDialog alertDialog = new AlertDialog.Builder(ActivityDfu.this).create();
                alertDialog.setTitle("Device Firmware Upgrade");
                alertDialog.setMessage("\nStart DFU?\n\nJson File: " + mJsonFileName + "\n");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mMsgHandler.sendEmptyMessage(MSG_DFU_START);
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

        mBtnDfuStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Stop DFU");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityDfu.this).create();
                alertDialog.setTitle("Stop DFU!");
                alertDialog.setMessage("\nDo you want to stop DFU?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mMsgHandler.sendEmptyMessage(MSG_DFU_STOP);
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

        mBtnDfuGetStatus.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Get DFU Status");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityDfu.this).create();
                alertDialog.setTitle("Get DFU Status");
                alertDialog.setMessage("\nGet DFU status?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mMsgHandler.sendEmptyMessage(MSG_DFU_GET_STATUS);
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
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityDfu.this).create();
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
            mService.registerCb(ActivityDfu.this);

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
            case MSG_DFU_START:
                handleMsgStartDfu();
                break;
            case MSG_DFU_STOP:
                handleMsgStopDfu();
                break;
            case MSG_DFU_GET_STATUS:
                handleMsgGetDfuStatus();
                break;
            case MSG_OTA_STATUS:
                handleMsgOtaStatus(msg.arg1, msg.arg2);
                break;
            case MSG_DFU_STATUS:
                handleMsgDfuStatus(msg.arg1, (byte[])msg.obj);
                break;
            }
        }
    };

    private void handleMsgConnection(int status) {
        String text = null;
        if (status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED) {
            mConnected = true;
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.green));
            text = "Connected to network";
        }
        else if (status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED) {
            mConnected = false;
            mViewConnected.setBackgroundColor(getResources().getColor(R.color.red));
            text = "Disconnected from network";
        }
        mBtnConnect.setEnabled(!mConnected);
        if (text != null) {
            Constants.Show(mActivity, text, Toast.LENGTH_LONG);
        }
    }

    private void handleMsgStartDfu() {
        int res = mService.startDfu(mJsonFileName, mDfuMethod);
        if (res == MeshController.MESH_CLIENT_SUCCESS) {
            mDfuState = DFU_STATE_SETUP_DFU;
            mTextDfuStatus.setText("DFU status: starting DFU");
            handleMsgGetDfuStatus();
        }
        else {
            mTextDfuStatus.setText("DFU status: DFU failed, error " + res);
        }
    }

    private void handleMsgStopDfu() {
        if (mDfuState != DFU_STATE_IDLE) {
            mDfuState = DFU_STATE_IDLE;
            if (mConnected) {
                mService.stopDfu();
            }
            mTextDfuStatus.setText("DFU status: stoping DFU");
        }
    }

    private void handleMsgGetDfuStatus() {
        if (mConnected) {
            int statusInterval = 0;
            if (mEditDfuStatusInterval.isEnabled()) {
                String str = mEditDfuStatusInterval.getText().toString();
                if (str != null && str.length() > 0) {
                    statusInterval = Integer.valueOf(str);
                }
                Log.i(TAG, "Status interval = " + statusInterval);
            }
            if (statusInterval != 0) {
                mService.getDfuStatus(statusInterval);
            }
        }
    }

    private void handleMsgOtaStatus(int status, int percentage) {
        if (status == IMeshControllerCallback.OTA_UPGRADE_STATUS_IN_PROGRESS) {
            if (mDfuState == DFU_STATE_SETUP_DFU) {
                mDfuState = DFU_STATE_UPLOADING;
            }
            if (mDfuState == DFU_STATE_UPLOADING) {
                if (mPercentComplete != percentage) {
                    mPercentComplete = percentage;
                    mTextUploadingStatus.setText("Status: uploading " + percentage + "%");
                }
            }
        }
        else if (status == IMeshControllerCallback.OTA_UPGRADE_STATUS_COMPLETED) {
            if (mDfuState == DFU_STATE_UPLOADING) {
                mTextUploadingStatus.setText("Status: OTA upgrade completed");
            }
            if (mDfuState != DFU_STATE_UPDATING) {
                mDfuState = DFU_STATE_UPDATING;
            }
        }
        else {
            if (mDfuState == DFU_STATE_UPLOADING) {
                mTextUploadingStatus.setText("Status: OTA failed " + status);
            }
            if (mDfuState != DFU_STATE_IDLE) {
                mDfuState = DFU_STATE_IDLE;
            }
        }
    }

    private void handleMsgDfuStatus(int state, byte[] data) {
        Log.i(TAG, "handleMsgDfuStatus: state = " + state);
        if (data != null) {
            Log.i(TAG, "handleMsgDfuStatus: data[" + data.length + "] = " + Constants.toHexString(data));
        }

        String text = null;
        switch (state) {
        case Constants.WICED_BT_MESH_DFU_STATE_INIT:
            // Nothing to do
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_VALIDATE_NODES:
            text = "DFU status: Validating nodes";
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_GET_DISTRIBUTOR:
            text = "DFU status: Get distributor";
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_UPLOAD:
            if (data != null && data.length > 0) {
                text = "DFU status: Uploading " + data[0];
            }
            else {
                text = "DFU status: Uploading";
            }
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_DISTRIBUTE:
            if (data != null && data.length >= 2) {
                int node_num = (data[1] << 8) + data[0];
                text = "DFU status: Distributing, node_num:" + node_num;
            }
            else {
                text = "DFU status: Distributing";
            }
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_APPLY:
            text = "DFU status: Applying";
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_COMPLETE:
            if (data == null || data.length < 2) {
                text = "DFU status: Completed";
            }
            else {
                int node_num = (data[1] << 8) + data[0];
                text = "DFU status: Completed, node_num:" + node_num;
            }
            mDfuState = DFU_STATE_IDLE;
            break;
        case Constants.WICED_BT_MESH_DFU_STATE_FAILED:
            text = "DFU status: Failed";
            mDfuState = DFU_STATE_IDLE;
            break;
        default:
            text = "DFU status: Unknwon state" + state;
            mDfuState = DFU_STATE_IDLE;
            break;
        }
        if (text != null) {
            mTextDfuStatus.setText(text);
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
        Message msg = mMsgHandler.obtainMessage(MSG_CONNECTION, status, 0);
        mMsgHandler.sendMessage(msg);
    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {
        Log.i(TAG, "onOtaStatus: status:" + status + " percentComplete:" + percentComplete);
        Message msg = mMsgHandler.obtainMessage(MSG_OTA_STATUS, status, percentComplete);
        mMsgHandler.sendMessage(msg);
    }

    @Override
    public void onDfuStatus(byte state, byte[] data) {
        Log.i(TAG, "onDfuStatus: state = " + state);
        if (data != null) {
            Log.i(TAG, "onDfuStatus: data[" + data.length + "] = " + Constants.toHexString(data));
        }
        Message msg = mMsgHandler.obtainMessage(MSG_DFU_STATUS, state, 0, data);
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
    public void onNodeConnStateChanged(final byte status, final String componentName) {
    }

    @Override
    public void onNetworkOpenedCallback(byte status) {
    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, final String componentInfoStr) {
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
