/*
* Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 *  Corporation. All rights reserved. This software, including source code, documentation and  related
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
 *  subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection
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
 *  manufacturer of such system or application assumes  all risk of such use and in doing so agrees
 * to indemnify Cypress against all liability.
*/
package com.cypress.le.mesh.meshframework;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelUuid;
import android.util.Log;

import com.cypress.le.mesh.meshcore.MeshNativeHelper;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;


public class MeshGattClient {
    private static final String TAG = "MeshGattClient";

    private static final int MSG_GATT_CONNECTED         = 1;
    private static final int MSG_GATT_MTU_TIMEOUT       = 2;
    private static final int MSG_GATT_SERVICE_DISCOVERY = 3;
    private static final int MSG_DEVICE_CONNECTED       = 4;

    private static final long GATT_MTU_DELAY               = 100;
    private static final long GATT_MTU_TIMEOUT             = 5000;
    private static final long GATT_SERVICE_DISCOVERY_DELAY = 100;
    private static final long DEVICE_CONNECTED_DELAY       = 100;

    private Context                     mCtx              = null;
    private BluetoothAdapter            mBluetoothAdapter = null;
    private BluetoothLeScanner          mLeScanner        = null;
    private BluetoothGatt               mGatt             = null;
    private BluetoothGattService        mGattService      = null;
    private BluetoothGattCharacteristic mGattCharData     = null;
    private BluetoothGattCharacteristic mGattCharNotify   = null;
    private BluetoothDevice             mDevice           = null;

    private MeshNativeHelper        mMeshNativeHelper = null;
    private IMeshGattClientCallback mCallback         = null;
    private OtaUpgrade              mOtaUpgrade       = null;
    private GattUtils.RequestQueue  mRequestQueue     = GattUtils.createRequestQueue();

    private short   mMtuSize                = 0;
    private boolean mOtaSupported           = false; // Whether device support Cypress OTA service
    private boolean mSecureServiceSupported = false; // Whether support secure OTA service

    public MeshGattClient(Context ctx, MeshNativeHelper meshNativeHelper, IMeshGattClientCallback callback) {
        mCtx              = ctx;
        mMeshNativeHelper = meshNativeHelper;
        mCallback         = callback;
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        mCtx.registerReceiver(mBroadcastReceiver, filter);
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
            case BluetoothAdapter.ACTION_STATE_CHANGED:
                if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1) == BluetoothAdapter.STATE_OFF) {
                    mMeshNativeHelper.meshClientConnectionStateChanged((short)0, (short)0);
                }
                break;
            }
        }
    };

    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            byte[] scanData = result.getScanRecord().getBytes();
            Log.i(TAG, "onScanResult: (" + scanData.length + ") " + Constants.toHexString(scanData));
            BluetoothDevice device = result.getDevice();
            if (device == null) {
                Log.i(TAG, "onScanResult: device is null");
                return;
            }
            String address = device.getAddress();
            String name = result.getScanRecord().getDeviceName();
            Log.i(TAG, "onScanResult: device = " + address + ", rssi = " + result.getRssi() + ", name = " + name);

            byte[] identity = new byte[scanData.length];
            System.arraycopy(scanData, 0, identity, 0, scanData.length);

            String[] dev = address.split(":");
            byte[] bdAddr = new byte[6];        // mac.length == 6 bytes
            for (int j = 0; j < dev.length; j++) {
                bdAddr[j] = Integer.decode("0x" + dev[j]).byteValue();
            }
            mMeshNativeHelper.meshClientAdvertReport(bdAddr, (byte)0, (byte)result.getRssi(), identity, identity.length);
        }
    };

    public boolean meshAdvScan(boolean start) {
        Log.i(TAG, "meshAdvScan: start = " + start);
        if (start == false) {
            if (mLeScanner == null) {
                Log.i(TAG, "meshAdvScan: scan stopped already");
                return true;
            }
            mLeScanner.stopScan(mScanCallback);
            mLeScanner = null;
            return true;
        }

        if (mLeScanner != null) {
            Log.e(TAG, "meshAdvScan: scan is running already");
            return true;
        }
        if (mBluetoothAdapter == null) {
            mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            if (mBluetoothAdapter == null) {
                Log.e(TAG, "meshAdvScan: BluetoothAdapter is null");
                return false;
            }
        }
        mLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
        if (mLeScanner == null) {
            Log.e(TAG, "meshAdvScan: Failed to get scanner");
            return false;
        }

        List<ScanFilter> scanFilters = new ArrayList<ScanFilter>();
        ScanFilter scanFilterProv
            = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(Constants.UUID_SERVICE_MESH_PROVISIONING)).build();
        ScanFilter scanFilterProxy
            = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(Constants.UUID_SERVICE_MESH_PROXY)).build();
        scanFilters.add(scanFilterProv);
        scanFilters.add(scanFilterProxy);
        ScanSettings setting
            = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
        mLeScanner.startScan(scanFilters, setting, mScanCallback);
        return true;
    }


    private BluetoothGattCallback mGattCallbacks = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(final BluetoothGatt gatt, int status, int newState) {
            Log.i(TAG, "onConnectionStateChange: status = " + status + ", newState = " + newState);
            if (status != 0 && newState != BluetoothProfile.STATE_DISCONNECTED) {
                gatt.disconnect();
                mMeshNativeHelper.meshClientConnectionStateChanged((short)0, mMtuSize);
            }
            else if (newState == BluetoothProfile.STATE_CONNECTED) {
                mHandler.sendEmptyMessageDelayed(MSG_GATT_CONNECTED, GATT_MTU_DELAY);
                mDevice = gatt.getDevice();
            }
            else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                gatt.close();
                mGatt           = null;
                mGattService    = null;
                mGattCharNotify = null;
                mDevice         = null;
                mMeshNativeHelper.meshClientConnectionStateChanged((short)0, mMtuSize);
            }
        }

        @Override
        public void onMtuChanged(final BluetoothGatt gatt, int mtu, int status) {
            Log.i(TAG, "onMtuChanged: mtu = " + mtu + ", status = " + status);
            mHandler.removeMessages(MSG_GATT_MTU_TIMEOUT);
            mMeshNativeHelper.meshClientSetGattMtu(mtu);
            mMtuSize = (short)mtu;
            mHandler.sendEmptyMessageDelayed(MSG_GATT_SERVICE_DISCOVERY, GATT_SERVICE_DISCOVERY_DELAY);
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            if (descriptor.getCharacteristic().getUuid().equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT)) {
                mOtaUpgrade.onOtaDescriptorRead(gatt, descriptor, status);
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            if (descriptor.getCharacteristic().getUuid().equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT)) {
                mOtaUpgrade.onOtaDescriptorWrite(gatt, descriptor, status);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            Log.i(TAG, "onServicesDiscovered: status = " + status);
            if (status != 0) {
                gatt.disconnect();
                return;
            }

            /*
            // List all the services
            List<BluetoothGattService> serviceList = gatt.getServices();
            Log.i(TAG, "onServicesDiscovered: service num = " + serviceList.size());
            // Loops through available Characteristics.
            for (BluetoothGattService service : serviceList) {
                Log.i(TAG, "listServices: service " + service.getUuid());
                List<BluetoothGattCharacteristic> charList = service.getCharacteristics();
                Log.i(TAG, "listServices: Characteristic num = " + charList.size());
                for (BluetoothGattCharacteristic characteristic : charList) {
                    Log.i(TAG, "listServices:     Characteristic " + characteristic);
                }
            }
            */

            boolean connectProvisioning = mMeshNativeHelper.meshClientIsConnectingProvisioning();
            Log.i(TAG, "onServicesDiscovered: connectProvisioning = " + connectProvisioning);
            if (connectProvisioning) { // Connecting to provisioning
                mGattService = gatt.getService(Constants.UUID_SERVICE_MESH_PROVISIONING);
                if (mGattService == null) {
                    Log.e(TAG, "onServicesDiscovered: SIG Mesh Provisioning Service ("
                            + Constants.UUID_SERVICE_MESH_PROVISIONING + ") not found");
                    mGatt.disconnect();
                    mGatt = null;
                    return;
                }

                ArrayList<BluetoothGattCharacteristic> array
                    = (ArrayList<BluetoothGattCharacteristic>) mGattService.getCharacteristics();
                Log.i(TAG, "onServicesDiscovered: Provisioning Service Characteristics num = " + array.size());
                for (int i = 0; i < array.size(); i++) {
                    Log.i(TAG, " Characteristics[" + i + "]: " + array.get(i).getUuid());
                }

                mGattCharData = mGattService.getCharacteristic(
                        Constants.UUID_CHARACTERISTIC_MESH_PROVISIONING_DATA_IN);
                if (mGattCharData == null) {
                    Log.e(TAG, "onServicesDiscovered: Provisioning Service DataIn Characteristic (" +
                          Constants.UUID_CHARACTERISTIC_MESH_PROVISIONING_DATA_IN + ") not found");
                    mGatt.disconnect();
                    mGatt = null;
                    return;
                }

                mGattCharNotify = mGattService.getCharacteristic(
                        Constants.UUID_CHARACTERISTIC_MESH_PROVISIONING_DATA_OUT);
                if (mGattCharNotify == null) {
                    Log.e(TAG, "onServicesDiscovered: Provisioning Service Notify Characteristic (" +
                          Constants.UUID_CHARACTERISTIC_MESH_PROVISIONING_DATA_OUT + ") not found");
                    mGatt.disconnect();
                    mGatt = null;
                    return;
                }
            }
            else { // Connecting to proxy
                mOtaSupported           = false;
                mSecureServiceSupported = false;
                mGattService = gatt.getService(Constants.UUID_SERVICE_CYPRESS_OTA_FW_UPGRDE);
                if (mGattService != null) {
                    Log.i(TAG, "onServicesDiscovered: Proxy device supports Cypress OTA Service");
                    mOtaSupported = true;
                }
                mGattService = gatt.getService(Constants.UUID_SERVICE_CYPRESS_OTA_SEC_FW_UPGRDE);
                if (mGattService != null) {
                    Log.i(TAG, "onServicesDiscovered: Proxy device supports Cypress Secure OTA Service");
                    mOtaSupported           = true;
                    mSecureServiceSupported = true;
                }

                mGattService = gatt.getService(Constants.UUID_SERVICE_MESH_PROXY);
                if (mGattService == null) {
                    Log.e(TAG, "onServicesDiscovered: SIG Mesh Proxy Service ("
                            + Constants.UUID_SERVICE_MESH_PROXY + ") not found");
                    gatt.disconnect();
                    return;
                }

                ArrayList<BluetoothGattCharacteristic> array
                    = (ArrayList<BluetoothGattCharacteristic>) mGattService.getCharacteristics();
                Log.i(TAG, "onServicesDiscovered: Proxy Service Characteristics num = " + array.size());
                for (int i = 0; i < array.size(); i++) {
                    Log.i(TAG, " Characteristics[" + i + "]: " + array.get(i).getUuid());
                }

                mGattCharData = mGattService.getCharacteristic(
                        Constants.UUID_CHARACTERISTIC_MESH_PROXY_DATA_IN);
                if (mGattCharData == null) {
                    Log.e(TAG, "onServicesDiscovered: Proxy Service DataIn Characteristic (" +
                          Constants.UUID_CHARACTERISTIC_MESH_PROXY_DATA_IN + ") not found");
                    gatt.disconnect();
                    return;
                }

                mGattCharNotify = mGattService.getCharacteristic(
                        Constants.UUID_CHARACTERISTIC_MESH_PROXY_DATA_OUT);
                if (mGattCharNotify == null) {
                    Log.e(TAG, "onServicesDiscovered: Proxy Service Notify Characteristic (" +
                          Constants.UUID_CHARACTERISTIC_MESH_PROXY_DATA_OUT + ") not found");
                    gatt.disconnect();
                    return;
                }
            }
            Log.i(TAG, "onServicesDiscovered: Gatt connected");

            gatt.setCharacteristicNotification(mGattCharNotify, true);
            BluetoothGattDescriptor descriptor = mGattCharNotify.getDescriptor(
                    Constants.UUID_DESCRIPTOR_CHARACTERISTIC_UPDATE_NOTIFICATION);
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            boolean ret = gatt.writeDescriptor(descriptor);
            Log.i(TAG, "onServicesDiscovered: Update_Notify descriptor write: " + ret);

            mHandler.sendEmptyMessageDelayed(MSG_DEVICE_CONNECTED, DEVICE_CONNECTED_DELAY);
            /*
            // Sleep 100 ms
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            // Send a broadcast to MeshService_old that proxy is connected
            new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                @Override
                public void run() {
                    Log.i(TAG, "onServicesDiscovered: set GATT MTU size = " + mMtuSize);
                    mMeshNativeHelper.meshClientConnectionStateChanged((short)1, mMtuSize);
                    mMeshNativeHelper.meshClientSetGattMtu(mMtuSize);
                }
            }, 100);
            mDevice = gatt.getDevice();
            */
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            UUID uuid = characteristic.getUuid();
            Log.i(TAG, "onCharacteristicWrite: status = " + status + ", char = " + uuid);
            if (uuid.equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_DATA) ||
                uuid.equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT)) {
                mOtaUpgrade.onOtaCharacteristicWrite(gatt, characteristic, status);
            }
            else {
                mRequestQueue.next();
            }
        }

        /**
         * Callback invoked by Android framework when a characteristic
         * notification occurs
         */
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            UUID uuid = characteristic.getUuid();
            if (uuid.equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_DATA) ||
                uuid.equals(Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT)) {
                Log.i(TAG, "onCharacteristicChanged: OTA char changed");
                mOtaUpgrade.onOtaCharacteristicChanged(characteristic);
            }
            else {
                byte[] data = characteristic.getValue();
                int    len  = data.length;
                Log.i(TAG, "onCharacteristicChanged: " + uuid.toString());
                UUID serviceUuid = characteristic.getService().getUuid();
                if (serviceUuid.equals(Constants.UUID_SERVICE_MESH_PROVISIONING)) {
                    Log.i(TAG, "onCharacteristicChanged: provis packet: len = " + len + ", val = " + Constants.toHexString(data));
                    mMeshNativeHelper.SendRxProvisPktToCore(data, len);
                }
                else if (serviceUuid.equals(Constants.UUID_SERVICE_MESH_PROXY)) {
                    Log.e(TAG, "onCharacteristicChanged: proxy packet: len = " + len + ", val = " + Constants.toHexString(data));
                    mMeshNativeHelper.SendRxProxyPktToCore(data, len);
                }
            }
        }
    };


    public boolean connect(byte[] bdaddr) {
        Log.i(TAG, "connect: addr = " + Constants.toHexString(bdaddr, ':'));
        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(bdaddr);
        if (device == null) {
            Log.e(TAG, "connect: Device " + Constants.toHexString(bdaddr, ':') + " not found");
            return false;
        }
        return connect(device);
    }

    public boolean connect(BluetoothDevice device) {
        Log.i(TAG, "connect: device = " + device.getAddress());
        if (mGatt != null) {
            Log.e(TAG, "connect: GATT is busy...");
            mGatt.close();
        }
        mRequestQueue.clear();

        // API 23(Android-M) introduced connectGatt with Transport
        if (Build.VERSION.SDK_INT >= 23)
            mGatt = device.connectGatt(mCtx, false, mGattCallbacks, BluetoothDevice.TRANSPORT_LE);
        else
            mGatt = device.connectGatt(mCtx, false, mGattCallbacks);

        if (mGatt != null)
            return true;
        Log.e(TAG, "connect: Failed to connect device " + device.getAddress());
        return false;
    }

    public void disconnect(int connId) {
        Log.i(TAG, "disconnect: connId = " + connId);
        if (mGatt != null) {
            mGatt.disconnect();
            mGatt = null;
        }
        else {
            Log.e(TAG, "disconnect: mGatt is null");
        }
    }

    // Send provisioning packet through GATT to device
    public void sendProvisionPacket(byte[] packet, int len) {
        Log.i(TAG, "sendProvisionPacket: (" + len + ") " + Constants.toHexString(packet));
        BluetoothGattCharacteristic characteristic = GattUtils.getCharacteristic(mGatt,
                    Constants.UUID_SERVICE_MESH_PROVISIONING,
                    Constants.UUID_CHARACTERISTIC_MESH_PROVISIONING_DATA_IN);
        if (characteristic == null) {
            Log.e(TAG, "sendProvisionPacket: characteristic not found!");
            return;
        }
        mRequestQueue.addWriteCharacteristic(mGatt, characteristic, packet);
        mRequestQueue.execute();
    }

    // Send proxy packet through GATT to device
    public void sendProxyPacket(byte[] packet, int len) {
        Log.i(TAG, "sendProxyPacket: (" + len + ") " + Constants.toHexString(packet));
        BluetoothGattCharacteristic characteristic = GattUtils.getCharacteristic(mGatt,
                    Constants.UUID_SERVICE_MESH_PROXY,
                    Constants.UUID_CHARACTERISTIC_MESH_PROXY_DATA_IN);
        if (characteristic == null) {
            Log.e(TAG, "sendProxyPacket: characteristic not found!");
            return;
        }
        mRequestQueue.addWriteCharacteristic(mGatt, characteristic, packet);
        mRequestQueue.execute();
    }

    public boolean isOtaSupportedCb() {
        Log.i(TAG, "isOtaSupportedCb: mOtaSupported = " + mOtaSupported + ", mSecureServiceSupported = " + mSecureServiceSupported);
        return (mOtaSupported || mSecureServiceSupported);
    }

    public void startOta(String firmwareFile, boolean isDfu) {
        Log.i(TAG, "startOta: isDfu:" + isDfu + ", name = " + firmwareFile);
        if (mOtaUpgrade == null) {
            mOtaUpgrade = new OtaUpgrade(mMeshNativeHelper, mCallback, mGatt, mRequestQueue, mOtaSupported, mSecureServiceSupported, (mMtuSize - 17));
        }

        // MTU size is to accomodate extra bytes added from encryption
        mOtaUpgrade.start(firmwareFile, isDfu);
    }

    public int stopOta() {
        Log.i(TAG, "stopOta");
        if (mOtaUpgrade != null) {
            return mOtaUpgrade.stop();
        }
        return MeshController.MESH_CLIENT_ERR_INVALID_ARGS;
    }

    public void otaUpgradeApply() {
        Log.i(TAG, "otaUpgradeApply");
        if (mOtaUpgrade == null) {
            mOtaUpgrade = new OtaUpgrade(mMeshNativeHelper, mCallback, mGatt, mRequestQueue, mOtaSupported, mSecureServiceSupported, (mMtuSize - 17));
        }
        mOtaUpgrade.apply();
    }

    private  Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_GATT_CONNECTED:
                Log.i(TAG, "MSG_GATT_CONNECTED: mtu = " + Constants.MTU_SIZE_GATT);
                if (mGatt != null) {
                    boolean res = mGatt.requestMtu(Constants.MTU_SIZE_GATT);
                    Log.i(TAG, "MSG_GATT_CONNECTED: res = " + res);
                    if (res) {
                        mHandler.sendEmptyMessageDelayed(MSG_GATT_MTU_TIMEOUT, GATT_MTU_TIMEOUT);
                    }
                    else {
                        mGatt.disconnect();
                        mGatt = null;
                    }
                }
                break;
            case MSG_GATT_MTU_TIMEOUT:
                Log.i(TAG, "MSG_GATT_MTU_TIMEOUT");
                if (mGatt != null) {
                    mGatt.disconnect();
                    mGatt = null;
                }
                break;
            case MSG_GATT_SERVICE_DISCOVERY:
                Log.i(TAG, "MSG_GATT_SERVICE_DISCOVERY");
                if (mGatt != null) {
                    if (!mGatt.discoverServices()) {
                        mGatt.disconnect();
                        mGatt = null;
                        Log.e(TAG, "MSG_GATT_SERVICE_DISCOVERY: failed");
                    }
                }
                else {
                    Log.e(TAG, "MSG_GATT_SERVICE_DISCOVERY: GATT is null");
                }
                break;
            case MSG_DEVICE_CONNECTED:
                Log.i(TAG, "MSG_DEVICE_CONNECTED: mtu = " + mMtuSize);
                mMeshNativeHelper.meshClientConnectionStateChanged((short)1, mMtuSize);
                mMeshNativeHelper.meshClientSetGattMtu(mMtuSize);
                break;
            }
        }
    };
}
