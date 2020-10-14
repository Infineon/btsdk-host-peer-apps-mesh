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
package com.cypress.le.mesh.meshframework;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.util.Calendar;

import com.cypress.le.mesh.meshcore.MeshNativeHelper;


// Main activity for the the OTA Client application
public class OtaUpgrade {
    private static final String TAG = "OtaUpgrade";

    // MTU size is set to 158 instead of 259 to cooperate with Pixel devices
    private static final int SERIAL_GATT_REQUEST_MTU = 158;
    private static final int SERIAL_GATT_DEFAULT_MTU = 23;

    private static final int WS_UPGRADE_CONNECTED                         = 0x0;
    private static final int WS_UPGRADE_RESPONSE_OK                       = 0x1;
    private static final int WS_UPGRADE_CONTINUE                          = 0x2;
    private static final int WS_UPGRADE_START_VERIFICATION                = 0x3;
    private static final int WS_UPGRADE_APPLY_FIRMWARE                    = 0x4;
    private static final int WS_UPGRADE_RESPONSE_FAILED                   = 0x5;
    private static final int WS_UPGRADE_ABORT                             = 0x6;
    private static final int WS_UPGRADE_DISCONNECTED                      = 0x7;

    private static final int WS_UPGRADE_STATE_IDLE                        = 0x0;
    private static final int WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD = 0x1;
    private static final int WS_UPGRADE_STATE_DATA_TRANSFER               = 0x2;
    private static final int WS_UPGRADE_STATE_VERIFICATION                = 0x3;
    private static final int WS_UPGRADE_STATE_VERIFIED                    = 0x4;
    private static final int WS_UPGRADE_STATE_ABORTED                     = 0x5;

    private MeshNativeHelper        mMeshNativeHelper = null;
    private IMeshGattClientCallback mOtaStatusCb      = null;
    private GattUtils.RequestQueue  mRequestQueue     = null ;
    private BluetoothGatt           mGatt;
    private int                     mMtu;
    private boolean                 mIsDfu;

    private byte[]  mPatch = null;
    private int     mPatchSize = 0;
    private int     mPatchOffset;
    private int     mPatchCrc32;

    private int     mState;
    private long    mTime;
    private boolean mInTransfer = false;

    private boolean mOtaSupported           = false;
    private boolean mSecureServiceSupported = false;

    public OtaUpgrade(MeshNativeHelper meshNativeHelper, IMeshGattClientCallback callback, BluetoothGatt gatt, GattUtils.RequestQueue requestQueue, boolean otaSupported, boolean secureServiceSupported, int mtu) {
        Log.i(TAG, "OtaUpgrade: mtu = " + mtu + ", otaSupported = " + otaSupported + ", secureServiceSupported = " + secureServiceSupported);
        mMeshNativeHelper = meshNativeHelper;
        mOtaStatusCb      = callback;
        mGatt             = gatt;
        mRequestQueue     = requestQueue;
        mOtaSupported     = otaSupported;
        mSecureServiceSupported = secureServiceSupported;

        mMtu = mtu > SERIAL_GATT_REQUEST_MTU ? SERIAL_GATT_REQUEST_MTU : mtu;
    }

    public void start(String firmwareFileName, boolean isDfu) {
        Log.i(TAG, "start: isDfu:" + isDfu + ", firmwareFileName = " + firmwareFileName);

        mIsDfu = isDfu;

        mPatch = readFirmwareFile(firmwareFileName);
        if (mPatch != null) {
            mPatchSize = mPatch.length;
        }
        else {
            Log.i(TAG, "start: firmwareFile is empty");
            return;
        }
        Log.i(TAG, "start: mPatchSize = " + mPatchSize);

        mPatchCrc32 = 0xFFFFFFFF;
        for (byte b : mPatch) {
            mPatchCrc32 = (mPatchCrc32 >>> 8) ^ Constants.CRC32_TABLE[(mPatchCrc32 ^ b) & 0xff];
        }
        // Flip bits
        mPatchCrc32 = mPatchCrc32 ^ 0xffffffff;

        // Initiate the OTA Update procedure
        mState = WS_UPGRADE_STATE_IDLE;

        processEvent(WS_UPGRADE_CONNECTED);

        mTime = Calendar.getInstance().getTimeInMillis();
    }

    public int stop(){
        Log.i(TAG, "stop: mInTransfer = " + mInTransfer + ", mState = " + mState);
        if (mInTransfer || mState == WS_UPGRADE_STATE_DATA_TRANSFER) {
            mState = WS_UPGRADE_ABORT;
            sendWsUpgradeCommand(Constants.WICED_OTA_UPGRADE_COMMAND_ABORT);
            return MeshController.MESH_CLIENT_SUCCESS;
        }
        return MeshController.MESH_CLIENT_ERR_INVALID_STATE;
    }

    public void onOtaDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        Log.i(TAG, "onDescriptorRead: status = " + status);
        mRequestQueue.next();  // Execute the next queued request, if any
    }

    public void onOtaDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        Log.i(TAG, "onDescriptorWrite: status = " + status);
        mRequestQueue.next();  // Execute the next queued request, if any
    }

    public void apply() {
        registerOtaNotification(true);
        processEvent(WS_UPGRADE_APPLY_FIRMWARE);
    }

    public void onOtaCharacteristicChanged(BluetoothGattCharacteristic characteristic) {
        Log.i(TAG, "onOtaCharacteristicChanged");
        try {
            String s   = characteristic.getStringValue(0);
            byte[] val = characteristic.getValue();
            byte[] decryptVal = mMeshNativeHelper.meshClientOTADataDecrypt("temp", val, val.length);
            if (decryptVal.length == 1) {
                switch (decryptVal[0]) {
                case Constants.WICED_OTA_UPGRADE_STATUS_OK:
                    processEvent(WS_UPGRADE_RESPONSE_OK);
                    break;
                case Constants.WICED_OTA_UPGRADE_STATUS_CONTINUE:
                    processEvent(WS_UPGRADE_CONTINUE);
                    break;
                default:
                    processEvent(WS_UPGRADE_RESPONSE_FAILED);
                    break;
                }
            }
        } catch (Throwable t) {
            Log.e(TAG, "error", t);
        }
    }

    public void onOtaCharacteristicWrite(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic, int status) {
        // Log.e(TAG, "onOtaCharacteristicWrite");
        if (status == 0) {
            try {
                processCharacteristicWrite(characteristic);
            } catch (Throwable t) {
                Log.e(TAG, "onOtaCharacteristicWrite error", t);
            }
        }
        mRequestQueue.next();// Execute the next queued request, if any
    }

    private byte[] readFirmwareFile(String firmwareFileName) {
        Log.i(TAG, "readFirmwareFile: " + firmwareFileName);
        if (firmwareFileName == null) {
            Log.i(TAG, "FirmwareFile is null");
            return null;
        }

        File firmwareFile = new File(firmwareFileName);
        if (firmwareFile == null || !firmwareFile.exists()) {
            Log.i(TAG, "FirmwareFile " + firmwareFileName + " does not exist");
            return null;
        }

        FileInputStream input = null;
        try {
            input = new FileInputStream(firmwareFile);
            int len = (int) firmwareFile.length();
            byte[] data = new byte[len];
            int count, total = 0;
            while ((count = input.read(data, total, len - total)) > 0) {
                total += count;
            }
            return data;
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
        finally {
            if (input != null) {
                try {
                    input.close();
                }
                catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
        return null;
    }

    /**
     * Write the ota configuration characteristic to the device
     */
    private void writeOTAControlPointCharacteristic(byte[] charValue) {
        BluetoothGattCharacteristic characteristic = null;
        Log.i(TAG, "writeOTAControlPointCharacteristic: " + Constants.toHexString(charValue));

        try {
            characteristic = GattUtils.getCharacteristic(mGatt,
                    mSecureServiceSupported ? Constants.UUID_SERVICE_CYPRESS_OTA_SEC_FW_UPGRDE : Constants.UUID_SERVICE_CYPRESS_OTA_FW_UPGRDE,
                    Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT);
            byte[] encData = encryptOTAData(charValue,charValue.length);
            int val = encData.length;
            mRequestQueue.addWriteCharacteristic(mGatt, characteristic, encData);
            mRequestQueue.execute();
        } catch (Throwable t) {
            Log.w(TAG, "Error Writing CP Characteristic");
        }
    }

    /**
     * Write the ota configuration characteristic to the device
     */
    private void writeOTAControlDataCharacteristic(byte[] charValue) {
        BluetoothGattCharacteristic characteristic = null;
        //Log.i(TAG, "writeOTAControlDataCharacteristic");
        // String s = new String(serial_gatt_dump_hex_string(charValue));
        // Log.i(TAG, "writeOTAControlDataCharacteristic value " + s);

        try {
            characteristic = GattUtils.getCharacteristic(mGatt,
                    mSecureServiceSupported ? Constants.UUID_SERVICE_CYPRESS_OTA_SEC_FW_UPGRDE : Constants.UUID_SERVICE_CYPRESS_OTA_FW_UPGRDE,
                    Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_DATA);
            byte[] encData = encryptOTAData(charValue,charValue.length);
            mRequestQueue.addWriteCharacteristic(mGatt, characteristic, encData);
            mRequestQueue.execute();
        } catch (Throwable t) {
            Log.w(TAG, "Error Writing CP Data characteristic");
        }
    }

    private byte[] encryptOTAData(byte[] charValue, int length) {
        byte[] ret;
        ret = mMeshNativeHelper.meshClientOTADataEncrypt("temp",charValue,length);
        return ret;
    }

    /**
     * Write the ota input descriptor to the device
     */
    private void registerOtaNotification(boolean notify) {
        Log.i(TAG, "registerOtaNotification notify"+notify);

        // Set the enable/disable notification settings
        BluetoothGattCharacteristic notifyCharacteristic = GattUtils.getCharacteristic(
                mGatt,
                mSecureServiceSupported ? Constants.UUID_SERVICE_CYPRESS_OTA_SEC_FW_UPGRDE : Constants.UUID_SERVICE_CYPRESS_OTA_FW_UPGRDE,
                Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT);
        BluetoothGattDescriptor descriptor = GattUtils.getDescriptor(mGatt,
                mSecureServiceSupported ? Constants.UUID_SERVICE_CYPRESS_OTA_SEC_FW_UPGRDE : Constants.UUID_SERVICE_CYPRESS_OTA_FW_UPGRDE,
                Constants.UUID_CHARACTERISTIC_CYPRESS_OTA_FW_UPGRDE_CONTROL_POINT,
                Constants.UUID_DESCRIPTOR_CHARACTERISTIC_UPDATE_NOTIFICATION);

        if(notifyCharacteristic == null) {
            Log.i(TAG, "notifyCharacteristic value is null");
            mOtaStatusCb.onOtaStatus(IMeshControllerCallback.OTA_UPGRADE_STATUS_SERVICE_NOT_FOUND, 0);
            return;
        }

        byte[] value =  new byte[2];
        if (notify == true) {
            mGatt.setCharacteristicNotification(notifyCharacteristic, true);
            value[0] = 0x3;
            value[1] = 0x0;
        } else {
            mGatt.setCharacteristicNotification(notifyCharacteristic, false);
            value[0] = 0x0;
            value[1] = 0x0;
        }
        Log.i(TAG, "addWriteDescriptor");
        if(mGatt  == null)
            Log.i(TAG, "mGatt is null");
        if(descriptor  == null)
            Log.i(TAG, "descriptor is null");
        mRequestQueue.addWriteDescriptor(mGatt, descriptor, value);
        mRequestQueue.execute();
    }

    /**
     * Callback invoked by the Android framework when a write characteristic
     * successfully completes
     *
     * @param characteristic
     */
    public void processCharacteristicWrite(final BluetoothGattCharacteristic characteristic) {
        processProgress(mState);
        // Continue if there is data to be sent
        // Log.i(TAG, "after updateProgress mInTransfer"+mInTransfer);
        if (mInTransfer)
            sendOtaImageData();
    }

    private void processProgress(int state) {
        // Log.i(TAG, "processProgress: state = " + state);
        int total = mPatchSize;
        int param = 0;

        if (state == WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD)
        {
            Log.i(TAG, "processProgress: WAIT_FOR_READY_FOR_DOWNLOAD");
            total = mPatchSize;
        }
        else if (state == WS_UPGRADE_STATE_DATA_TRANSFER)
        {
            param = mPatchOffset;
            // String status = "processProgress offset: "  + mPatchOffset + " of total: " + mPatchSize;

            //update progress to APP
            float val = ((float)mPatchOffset/mPatchSize) * 100;
            mOtaStatusCb.onOtaStatus(IMeshControllerCallback.OTA_UPGRADE_STATUS_IN_PROGRESS, (int)val);
            Log.i(TAG, "processProgress: offset/total = " + mPatchOffset + "/" + mPatchSize + ", " + val + "%");

            // Log.i(TAG, status);

            if (param == total)
            {
                Log.i(TAG, "processProgress: Sent Entire file, total = " + total);
                processEvent(WS_UPGRADE_START_VERIFICATION);
            }
        }
        else if (state == WS_UPGRADE_STATE_VERIFIED) {
            Log.i(TAG, "processProgress: VERIFIED");
            long elapsed_time = Calendar.getInstance().getTimeInMillis() - mTime;
            String status = "Success " + (elapsed_time / 1000) + "sec (" + (mPatchSize * 8 * 1000 / elapsed_time) + "kbps)";

            mOtaStatusCb.onOtaStatus(IMeshControllerCallback.OTA_UPGRADE_STATUS_COMPLETED, 100);

            // Notify DFU OTA is done. Start DFU now
            if (mIsDfu) {
                mMeshNativeHelper.meshClientDfuOtaFinished(MeshController.MESH_CLIENT_SUCCESS);
            }
        }
        else if (state == WS_UPGRADE_STATE_ABORTED) {
            Log.i(TAG, "processProgress Aborted");
            mOtaStatusCb.onOtaStatus(IMeshControllerCallback.OTA_UPGRADE_STATUS_ABORTED, 0);

            // Notify OTA failed
            if (mIsDfu) {
                mMeshNativeHelper.meshClientDfuOtaFinished(IMeshControllerCallback.OTA_UPGRADE_STATUS_ABORTED);
            }
        }
    }

    private void sendOtaImageData() {
        // String status = "sendOtaImageData file param: "  + mPatchOffset + " sent of total: " + mPatchSize;
        // Log.i(TAG, status);

        if ((mPatchSize > mPatchOffset) && (mState != WS_UPGRADE_STATE_ABORTED) && mInTransfer)
        {
            int dwBytes = mPatchSize - mPatchOffset;

            int mtu = mMtu - 3;

            dwBytes = (dwBytes > mtu) ? mtu : dwBytes;

            byte[] value = new byte[dwBytes];
            for (int i = 0; i < dwBytes; i++) {
                value[i] = mPatch[mPatchOffset + i];
            }

            // If this is the last packet finalize CRC
            if ((mPatchOffset + dwBytes) == mPatchSize)
            {
                mInTransfer = false;
            }

            writeOTAControlDataCharacteristic(value);

            mPatchOffset += dwBytes;

        }
        if (mState == WS_UPGRADE_STATE_ABORTED)
        {
            sendWsUpgradeCommand(Constants.WICED_OTA_UPGRADE_COMMAND_ABORT);
        }
        return;
    }

    private void sendWsUpgradeCommand(int command) {
        byte[] buffer =  new byte[1];
        buffer[0] = (byte) command;
        writeOTAControlPointCharacteristic(buffer);
    }

    private void sendWsUpgradeCommand(byte[] command) {
        writeOTAControlPointCharacteristic(command);
    }

    private void sendWsUpgradeCommand(int command, int value) {
        byte[] buffer =  new byte[5];
        buffer[0] = (byte) command;
        buffer[1] = (byte) (value & 0xff);
        buffer[2] = (byte) ((value >> 8) & 0xff);
        buffer[3] = (byte) ((value >> 16) & 0xff);
        buffer[4] = (byte) ((value >> 24) & 0xff);
        writeOTAControlPointCharacteristic(buffer);
    }

    private void processEvent(int event) {
        Log.i(TAG, "processEvent: state = " + mState + ", event = " + event);
        if (event == WS_UPGRADE_DISCONNECTED) {
            mState = WS_UPGRADE_STATE_IDLE;
            return;
        }
        else if (event == WS_UPGRADE_APPLY_FIRMWARE) {
            mState = WS_UPGRADE_STATE_IDLE;
            sendWsUpgradeCommand(Constants.WICED_OTA_UPGRADE_COMMAND_APPLY);
            return;
        }

        switch (mState) {
        case WS_UPGRADE_STATE_IDLE:
            if (event == WS_UPGRADE_CONNECTED) {
                // Register for notifications and indications with the status
                registerOtaNotification(true);

                byte value[] = new byte[1];
                value[0] = Constants.WICED_OTA_UPGRADE_COMMAND_PREPARE_DOWNLOAD;

                sendWsUpgradeCommand(value);
                mState = WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD;

                processProgress(mState);
            }
            break;

        case WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD:
            if (event == WS_UPGRADE_RESPONSE_OK) {
                mPatchOffset = 0;
                mState = WS_UPGRADE_STATE_DATA_TRANSFER;
                sendWsUpgradeCommand(Constants.WICED_OTA_UPGRADE_COMMAND_DOWNLOAD, (int)mPatchSize);
            }
            break;

        case WS_UPGRADE_STATE_DATA_TRANSFER:
            if (event == WS_UPGRADE_RESPONSE_OK) {
                    // Create thread reading unsolicited events
//                    CreateThread( NULL, 0, sendOtaImageData, this, 0, NULL);
//                    new Thread(new Runnable() {
//                        public void run() {
//                            mPatchOffset = 0;
//                            mInTransfer = true;
//                            sendOtaImageData();
//                        }
//                    }).start();

                if(mInTransfer == false) {
                    mPatchOffset = 0;
                    mInTransfer = true;
                    sendOtaImageData();
                }
            }
            else if (event == WS_UPGRADE_CONTINUE) {
            }
            else if (event == WS_UPGRADE_START_VERIFICATION) {
                mState = WS_UPGRADE_STATE_VERIFICATION;
                sendWsUpgradeCommand(Constants.WICED_OTA_UPGRADE_COMMAND_VERIFY, mPatchCrc32);
            }
            else if (event == WS_UPGRADE_ABORT) {
                Log.i(TAG, "processEvent state : " + WS_UPGRADE_STATE_DATA_TRANSFER + " event: " + WS_UPGRADE_ABORT);
                mState = WS_UPGRADE_STATE_ABORTED;
            }
            break;

        case WS_UPGRADE_STATE_VERIFICATION:
            if (event == WS_UPGRADE_RESPONSE_OK) {
                Log.i(TAG, "processEvent state : " + WS_UPGRADE_STATE_VERIFICATION + " event: " + WS_UPGRADE_RESPONSE_OK);
                mState = WS_UPGRADE_STATE_VERIFIED;
                processProgress(mState);
            }
            else if (event == WS_UPGRADE_RESPONSE_FAILED) {
                Log.i(TAG, "processEvent state : " + WS_UPGRADE_STATE_VERIFICATION + " event: " + WS_UPGRADE_RESPONSE_FAILED);
                mState = WS_UPGRADE_STATE_ABORTED;
                processProgress(mState);
            }
            break;

        case WS_UPGRADE_STATE_ABORTED:
            if (event == WS_UPGRADE_RESPONSE_OK) {
                Log.i(TAG, "processEvent state : " + WS_UPGRADE_STATE_ABORTED + " event: " + WS_UPGRADE_RESPONSE_OK);
                processProgress(mState);
            }
            break;

        default:
            break;
        }
    }

    /**
     * Callback invoked by the Android framework when a characteristic
     * notification is received
     *
     * @param characteristic
     */
    public void processCharacteristicNotification(final BluetoothGattCharacteristic characteristic) {
    }
}
