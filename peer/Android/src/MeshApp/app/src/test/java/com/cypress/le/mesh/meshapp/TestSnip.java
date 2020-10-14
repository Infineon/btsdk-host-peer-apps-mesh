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
package com.cypress.le.mesh.meshapp;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v4.graphics.ColorUtils;

import com.cypress.le.mesh.meshframework.IMeshControllerCallback;
import com.cypress.le.mesh.meshframework.MeshController;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

public class TestSnip extends Service implements IMeshControllerCallback{

    MeshController mController;
    ArrayList<MeshBluetoothDevice> mDevices = new ArrayList<MeshBluetoothDevice>();
    String mCurrentComponent;
    String mGroupName = "MyGroup";
    String mProvisioner = "Nadal";
    String mCurrentNetwork = "MyHome";
    static final int RESULT_SUCCESS = 1;
    static final int RESULT_FAILURE = 0;
    static final int RESULT_CONNECTED = 0;

    void initialize() {
        // start and bind to meshService once the service is bound,
        // onMeshServiceStatusChanged callback is called.
        mController = MeshController.initialize(getApplicationContext(),this);
    }

    public void onColorSelected(int mColor) {
        float hsl[] = new float[3];
        ColorUtils.colorToHSL(mColor,hsl);
        int hue = (int)hsl[0];
        int saturation = (int)(hsl[1]*100) ;
        int lightness = (int)(hsl[2]*100);
        mController.hslSet(mCurrentComponent,(short)lightness, (short)hue, (short)saturation, false, 0, (short) 0);
    }

    @Override
    public void onProvisionComplete(MeshBluetoothDevice device, byte status) {
        //get all the components in the current group and update UI.
        ArrayList<String> components = new ArrayList<String>(Arrays.asList(mController.getGroupComponents(mGroupName)));
        //currently considering first component in the group only
        mCurrentComponent = components.get(0);

        //set HSL value to color 120
        //if HSL value is set properly onHslStateChanged will be called with the proper status
        onColorSelected(120);
    }

    @Override
    public void onProvisionComplete(UUID deviceUuid, byte status) {

    }

    @Override
    public void onOnOffStateChanged(String deviceName, byte onOff) {

    }

    @Override
    public void onLevelStateChanged(String deviceName, short level) {

    }

    @Override
    public void onHslStateChanged(String deviceName, int lightness, int hue, int saturation) {
        //hsl state changed
    }

    @Override
    public void onMeshServiceStatusChanged(int status) {
        if(status == RESULT_CONNECTED) {
            // create a network with the appropriate provisioner name
            // and Network name check the result if create is successful

            if(mController.createNetwork(mProvisioner,mCurrentNetwork) == RESULT_SUCCESS) {
                // if network creation is success, open the network
                if(mController.openNetwork(mProvisioner, mCurrentNetwork) == RESULT_SUCCESS) {
                    // if Network open is successful create a group
                    if(mController.createGroup(mGroupName,mCurrentNetwork) == RESULT_SUCCESS) {
                        // Start scanning unprovisioned mesh devices, the devices found will be
                        // populated in onDeviceFound callback
                        mController.scanMeshDevices(true, null);
                    }
                }
            }
        }
    }

    @Override
    public void onDeviceFound(UUID uuid, String name) {

    }

    @Override
    public void onDeviceFound(MeshBluetoothDevice device, int rssi) {
        //populate the unprovisioned mesh devices to UI, currently provisioning first device found
        if(!mDevices.contains(device))
            mDevices.add(device);

        //stop the unprovisioned device scan
        mController.scanMeshDevices(false, null);

        //provision the selected device to a respective group, specify identification duration
        //once the device is provisioned and configured, onProvisionComplete callback will be called.
        //complete operation may take around 20seconds
        mController.provision("mydevice",mGroupName, device, (byte)10);
    }

    @Override
    public void onDeviceFound(UUID uuid, int oob, String name) {

    }

    @Override
    public void onNetworkConnectionStatusChanged(byte transport, byte status) {

    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {

    }

    @Override
    public void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime) {

    }

    @Override
    public void onNodeConnectionStateChanged(byte isConnected, String componentName) {

    }

    @Override
    public void onOTAUpgradeStatus(byte status, int percentComplete) {

    }

    @Override
    public void onReceivedProxyPktFromCore(byte[] data, int length) {

    }

    @Override
    public void onResetStatus(byte status, String componentName) {

    }

    @Override
    public void onNetworkOpenedCallback(byte status) {

    }

    @Override
    public void onDatabaseChanged(String meshName) {

    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, String componentInfo) {

    }

    @Override
    public void onDfuStatus(byte status, int currBlkNo, int totalBlocks) {

    }

    @Override
    public void onSensorStatusCb(String componentName, int propertyId, byte[] data) {

    }

    @Override
    public void onDfuStatus(byte status, int currBlkNo) {

    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        initialize();
        super.onCreate();
    }
}
