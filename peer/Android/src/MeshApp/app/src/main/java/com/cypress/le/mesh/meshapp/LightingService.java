/*
* Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
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
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;


import com.cypress.le.mesh.meshframework.IMeshControllerCallback;
import com.cypress.le.mesh.meshframework.MeshController;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Stack;
import java.util.UUID;
//import java.util.concurrent.RunnableFuture;
//import java.util.Random;


public class LightingService extends Service implements IMeshControllerCallback {
    private static String TAG = "LightingService";
    private static boolean ACK_NEEDED = false;
    static ArrayList<String> networks = new ArrayList<>();
    static String currentNetwork;
    static ArrayList<String> groups = new ArrayList<>();
    static ArrayList<String> rooms = new ArrayList<>();
    static ArrayList<String> lights = new ArrayList<>();
    static ArrayList<String> seeks = new ArrayList<>();
    private MeshController mMesh = null;
    private static String mCurrentGroup = null;
    private static Stack stack = new Stack();
    List<BluetoothDevice> listDevices = new ArrayList<BluetoothDevice>();
    private static MeshApp mApp;
    private final IBinder mBinder = new MyBinder();
    private String mAccessKey = "AKIAI64XPGOM5MPQCI7Q";
    private String mSecretKey = "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";
    private String mRestShadowEndpoint = "https://A38TD4KE8SEEKY.iot.us-east-1.amazonaws.com";
    private static final int TRACK_HSL_CHANGE_HSL_SET = 0;
    private static final int TRACK_HSL_CHANGE_RANGE_SET = 1;
    private static String otaUpgradeNode = null;

//    public static boolean isConnectedToNetwork = false;

    LightingService serviceReference= null;
    int time = 0;
    private IServiceCallback mCallback;

    public LightingService() {
    }

    @Override
    public void onCreate() {
        super.onCreate();

    }

    @Override
    public void onDestroy() {
        getMesh().unInitialize();
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    public int createRoom(String room, String ntw) {
        int res = 0;
//        counter = 0;
        Log.d(TAG,"Calling createRoom");
        if(currentNetwork!=null)
            res = mMesh.createGroup(room, ntw);

        if(res != 0) {
            rooms.add(room);
            Log.d(TAG, "new room" + room);
        }

        return res;
    }

    public int createSubGroup(String groupName, String parentGroupName) {
        int res = 0;
        if(mMesh !=null)
            res =mMesh.createGroup(groupName,parentGroupName);
        return res;
    }


    public int deleteRoom(String room) {
        return mMesh.deleteGroup(room);
    }

    public int deleteGroup(String group) {
        return mMesh.deleteGroup(group);
    }

//    public boolean DeleteNetwork(String networkName) {
//        mMesh.deleteNetwork(networkName);
//        return true;
//    }

    public List<String> getGroupContents(String groupName) {
        String[] components =  mMesh.getGroupComponents(groupName);
        return new ArrayList<String>(Arrays.asList(components));
    }

    public List<String> getallRooms() {
        Log.d(TAG, "getallRooms currentNetwork  =" + currentNetwork);
        if(currentNetwork!=null){
            groups = mMesh.getAllGroups(currentNetwork);
            return groups;
        } else {
            return null;
        }
    }

    public ArrayList<String> getTopLevelGroups(String groupName) {
        if(currentNetwork!=null){
            ArrayList<String> group = new ArrayList<String>(Arrays.asList(mMesh.getSubGroups(groupName)));
            if(group!=null)
            {
                for(int i=0;i<group.size();i++) {
                    Log.d(TAG, "received group " + group.get(i));
                    if(!groups.contains(group.get(i))) {
                        groups.add(group.get(i));
                    }
                }
                return group;
            } else {
                Log.d(TAG, "group = null");
                return null;
            }
        }
        return null;
    }

    public ArrayList<String> getGroupComponents(String groupName) {
        if(currentNetwork!=null){
            Log.d(TAG, "current ntw= " + currentNetwork);
            ArrayList<String> components = new ArrayList<String>(Arrays.asList(mMesh.getGroupComponents(groupName)));
            if(components!=null)
            {
                for(int i=0;i<components.size();i++) {
                    Log.d(TAG, "light id" + components.get(i) + " name = " + components.get(i));
                    if(!lights.contains(components.get(i))) {
                        lights.add(components.get(i));
                    }
                }
                return components;
            }
        }
        return null;
    }

    public void renameComponent(String oldComponentName, String newComponentName) {
        mMesh.rename(oldComponentName, newComponentName);
    }

/*
    public static void assignToDevice(String bleMeshLightDevice, BLEMeshElement element, BLEMeshModel model, int address) {
        Log.d(TAG,"assignToDevice :modelId"+model.getModelId());
        model.assosiatePublishAddress(mApp.getCurrApplication(), address);

    }



    public static boolean addLight(int groupId, String bleMeshLightDevice,String groupType) {
        String group = null;
        Enumeration<Integer> contents =  stack.elements();

        while(contents.hasMoreElements()) {
            int val = contents.nextElement();
            if(rooms.containsKey(val))
                group =rooms.get(val);
            else if(groups.containsKey(val))
                group =groups.get(val);
            // bleMeshLightDevice.addToGroup(group);
        }

        if(groupType.equals("room")) {
            group = getroom(groupId);
        } else {
            group = getgroup(groupId);
        }

        components.put(bleMeshLightDevice.getId(),bleMeshLightDevice);
        if(group!=null) {
            Log.d(TAG, "group is not null");
            return bleMeshLightDevice.addToGroup(group);
        } else {
            Log.d(TAG, "group is null");
            return false;
        }

    }
*/

    public String getOtaUpgradeNode() {
        Log.d(TAG, "getOtaUpgradeNode" + otaUpgradeNode);
        return otaUpgradeNode;
    }

    public void setOtaUpgradeNode(String node) {
        Log.d(TAG, "setOtaUpgradeNode" + otaUpgradeNode);
        otaUpgradeNode = node;
    }

    public static boolean addNetworkToList(String newNetwork) {
        if (newNetwork != null) {
            Log.d(TAG, "addNetworkToList, newNetwork = " + newNetwork);
            networks.add(newNetwork);
            currentNetwork = newNetwork;
            return true;
        } else {
            Log.e(TAG, "Error creating network!!");
            return false;
        }
    }

    public static String getCurrentNetwork() {
        Log.d(TAG, "getCurrentNetwork = " + currentNetwork);
        return currentNetwork;
    }

    public  ArrayList<String> getAllNetworks() {
        return new ArrayList<String>(Arrays.asList(mMesh.getAllNetworks()));
    }

    public Boolean changeNetwork(String newNetwork) {

        currentNetwork = newNetwork;
        Log.d(TAG, "changeNetwork = " + currentNetwork + "to "+newNetwork);
        lights = new ArrayList<String>();
        groups = new ArrayList<String>();
        rooms = new ArrayList<String>(Arrays.asList(mMesh.getAllNetworks()));

        return true;
    }

    public boolean isConnectedToNetwork() {
        if(mMesh != null)
            return mMesh.isConnectedToNetwork();
        else
            return false;
    }

    public String getCurrNetwork() {
        if(currentNetwork!=null)
            return currentNetwork;
        else
            return null;
    }

    public void setCurrentGroup(String groupName) {
        Log.d(TAG, "setCurrentGroup =" + groupName);
        mCurrentGroup = groupName;
    }
    public String getCurrentGroup() {
        Log.d(TAG, "getCurrentGroup =" + mCurrentGroup);
        return mCurrentGroup;
    }

    public static void pushStack(String groupName){

        stack.push(groupName);
        Log.d(TAG, "pushStack stack peek" + stack.toString());
    }
    public static String popStack(){
        String val =(String) stack.pop();
        Log.d(TAG, "popStack stack peek" + stack.toString());
        return val;
    }

    public void setMesh(MeshController mesh) {
        mMesh = mesh;
    }

    public MeshController getMesh() {
        return mMesh;
    }

    public byte connectToNetwork(int transport) {
        int currTransport = getTransport();
        if(currTransport == Constants.TRANSPORT_GATT && transport != currTransport)
            mMesh.disconnectNetwork();

        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mApp.setPreferredTransport(transport);
        if (currentNetwork != null) {
            return mMesh.connectNetwork((byte)100);
        }
        return MeshController.MESH_CLIENT_ERR_INVALID_STATE;
    }

    public byte connectComponent(String componentName, byte scanDuration) {
        return mMesh.connectComponent(componentName, scanDuration);
    }

    public int startDfu(String firmwareFile, byte dfuMethod) {
        return mMesh.startDfu(firmwareFile, dfuMethod);
    }

    public int stopDfu() {
        return mMesh.stopDfu();
    }

    public void getDfuStatus(int statusInterval) {
        mMesh.getDfuStatus(statusInterval);
    }

    public int startOta(String firmwareFile) {
        return mMesh.startOta(firmwareFile);
    }

    public int stopOta() {
        return mMesh.stopOta();
    }

    public void OnLevelChange(String deviceName, int level) {
        mMesh.levelSet(deviceName, (short)level, true, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
    }

    public void onHslValueChange(String deviceName, int lightness, int hue, int saturation) {
        Log.d(TAG, "onHslValueChange device=" + deviceName);
        mMesh.hslSet(deviceName, (short)lightness, (short)hue, (short)saturation, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
    }

    public void onOnoffSet(String deviceName, boolean brightness) {
        Log.d(TAG, "OnLightBrightinessChange devicename =" + deviceName);
        if(brightness)
            mMesh.onoffSet(deviceName, true, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
        else
            mMesh.onoffSet(deviceName, false, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
    }

    public void OnoffGet(String deviceName) {
        Log.d(TAG, "OnoffGet devicename =" + deviceName);
        int ret = mMesh.onoffGet(deviceName);
        Log.d(TAG,"onOffGet return = " + ret);
    }
    public int getTransport() {
        return mApp.getPreferredTransport();
    }

/*
    public int getGroupBrightness(int groupId) {
        Log.d(TAG, "getGroupBrightness id =" + groupId);
        String group = null;
        if(rooms.containsKey(groupId)) {
            Log.d(TAG, "getGroupBrightness rooms");
            group = rooms.get(groupId);
        }
        else if(groups.containsKey(groupId)) {
            Log.d(TAG, "getGroupBrightness groups");
            group = groups.get(groupId);
        }

        int sumBrightness = 0;
        List<String> lightdevices = group.getAllDevices();
        if(lightdevices!=null) {
            for(int i=0 ;i<lightdevices.size();i++) {
                if(components.containsKey(lightdevices.get(i).getId())) {
                    String light = components.get(lightdevices.get(i).getId());
                    Log.d(TAG, "getGroupBrightness light adding" + getLightBrightness(light.getId()));
                    sumBrightness = sumBrightness+ getLightBrightness(light.getId());
                }
            }
            try {
                sumBrightness = (int)(sumBrightness/lightdevices.size());
            }catch(ArithmeticException e) {
                e.printStackTrace();
                sumBrightness=0;
            }
        }
        return 0;
    }
*/
    public boolean setGroupBrightness (String group, boolean brightness) {
        onOnoffSet(group, brightness);
        Log.d(TAG, "setGroupBrightness getting brightness" + (brightness ));
        return true;
    }
/*
    public void removeDeviceFromGroup(String device, int groupId) {
        device.removeFromGroup(groups.get(groupId));
    }

    public ArrayList<Integer> getBrightnessValues(ArrayList<String> components) {
        ArrayList<Integer> brightness = new ArrayList<Integer>();
        if(components!=null) {
            for(int i=0 ; i <components.size();i++) {
                if(seeks.containsKey(components.get(i).getId())) {
                    brightness.add(seeks.get(components.get(i).getId()));
                } else {
                    seeks.put(components.get(i).getId(),100);
                    brightness.add(seeks.get(components.get(i).getId()));
                }
            }
        }
        return brightness;

    }

    public void deleteNetwork(int networkSelected) {
        Log.d(TAG, "deleteNetwork");
        networks.remove(networkSelected);
    }

    public void deleteBIG(String bigNode) {
        Log.d(TAG, "deleteBIG not available");
        //currentNetwork.deleteBIG(bigNode);

    }
*/

    public void setHSLStartTracking() {
        mMesh.startTracking();
    }

    public void setHSLStopTracking() {
        mMesh.stopTracking();
    }

    @Override
    public void onProvisionComplete(UUID device, byte status) {
        mCallback.onProvisionComplete(device, status);
    }

    @Override
    public void onOnOffStateChanged(String deviceName, byte targetOnOff, byte presentOnOff, int remainingTime) {
        Log.d(TAG, "onOnOffStateChanged Name: "+ deviceName);
//        if(ACK_NEEDED) {
//            if(!groups.contains(deviceName))
//                mCallback.onOnOffStateChanged(deviceName, targetOnOff, presentOnOff, remainingTime);
//        }

        mCallback.onOnOffStateChanged(deviceName, targetOnOff, presentOnOff, remainingTime);
    }

    @Override
    public void onLevelStateChanged(String deviceName, short targetLevel, short presentLevel, int remainingTime) {
        Log.d(TAG, "onLevelStateChanged");
        mCallback.onLevelStateChanged(deviceName, targetLevel, presentLevel,remainingTime);
    }

    @Override
    public void onHslStateChanged(String deviceName, int lightness, int hue, int saturation, int remainingTime) {
        Log.d(TAG, "onHslStateChanged");
        mCallback.onHslStateChanged(deviceName, lightness, hue, saturation, remainingTime);
    }

    @Override
    public void onMeshServiceStatusChanged(int status) {
        if(status == MeshController.MESH_SERVICE_CONNECTED) {
            Intent intent = new Intent(Constants.SERVICE_CONNECTED);
            sendBroadcast(intent);
        } else if(status == MeshController.MESH_SERVICE_DISCONNECTED) {
            Intent intent = new Intent(Constants.SERVICE_DISCONNECTED);
            sendBroadcast(intent);
        }
    }



    @Override
    public void onDeviceFound(UUID uuid,  String name) {
        mCallback.onDeviceFound(uuid, name);
    }


    @Override
    public void onNetworkConnectionStatusChanged(byte transport, byte status) {
        Log.d(TAG, "onNetworkConnectionStatusChanged status :"+status);
        mCallback.onNetworkConnectionStatusChanged(transport,status);
    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {
        Log.d(TAG, "onCtlStateChanged status :"+deviceName);
        mCallback.onCtlStateChanged(deviceName, presentLightness,presentTemperature,targetLightness,targetTemperature,remainingTime);
    }

    @Override
    public void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime) {
        Log.d(TAG, "onLightnessStateChanged status :"+deviceName);
        mCallback.onLightnessStateChanged(deviceName, target, present, remainingTime);
    }

    @Override
    public void onNodeConnectionStateChanged(final byte status, final String componentName) {
        Log.d(TAG, "onNodeConnectionStateChanged new status :"+componentName);
        mCallback.onNodeConnStateChanged(status, componentName);

    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {
        Log.d(TAG, "onOTAUpgradeStatus status :"+status+ " percentage:"+percentComplete);
        mCallback.onOtaStatus(status,percentComplete);
        if(percentComplete == 100) {
            otaUpgradeNode = null;
        }
    }

    @Override
    public void onReceivedProxyPktFromCore(byte[] data, int length) {
        Log.d(TAG, "onReceivedProxyPktFromCore");
        FragmentRoom.sendData(data,length);
    }

    @Override
    public void onNetworkOpenedCallback(byte status) {
        Log.d(TAG, "onNetworkOpenedCallback");
        mCallback.onNetworkOpenedCallback(status);
    }

    @Override
    public void onDatabaseChanged(String meshName) {
        Log.d(TAG,"onDatabaseChanged :"+meshName);

        /*below code can be enabled to test continous group creation to check the empty string issue */
//        final Random rand = new Random();
//        int val;
//        if(counter < 100){
//            counter ++;
//            mMesh.exportNetwork(meshName);
//            String[] allGrps1 = mMesh.getAllGroups(meshName);
//            Log.d(TAG,"number of groups = " +allGrps1.length);
//            mMesh.createGroup("abc"+counter+":" +rand.nextInt(500), meshName);
//            String[] allGrps = mMesh.getAllGroups(meshName);
//            if(allGrps.length !=  counter){
//                Log.d(TAG,"Empty String Case is hit exit");
//                counter = 100;
//            }
//        }
    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, String componentInfo) {
        mCallback.onComponentInfoStatus(status, componentName, componentInfo);
    }

    @Override
    public void onDfuStatus(byte status, byte[] data) {
        mCallback.onDfuStatus(status, data);
    }

    @Override
    public void onSensorStatusCb(String componentName, int propertyId, byte[] data) {
        mCallback.onSensorStatusCb(componentName, propertyId, data);
    }

    @Override
    public void onVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen) {
        mCallback.onVendorStatusCb(src, companyId, modelId, opcode, ttl, data, dataLen);
    }

    @Override
    public void onLightLcModeStatus(String componentName, int mode) {
        mCallback.onLightLcModeStatus(componentName, mode);
    }

    @Override
    public void onLightLcOccupancyModeStatus(String componentName, int mode) {
        mCallback.onLightLcOccupancyModeStatus(componentName, mode);
    }

    @Override
    public void onLightLcPropertyStatus(String componentName, int propertyId, int value) {
        mCallback.onLightLcPropertyStatus(componentName, propertyId, value);
    }

    public MeshController initializeMesh() {
        mMesh = MeshController.initialize(getApplicationContext(),this);
        return mMesh;
    }

    public void registerCb(IServiceCallback callback) {
        mCallback = callback;
    }

    public void setMeshApp(MeshApp meshApp) {
        mApp = meshApp;
    }

    public void ctlSet(String mCompName, int temp, int lightness, int transtime, int delay) {
        mMesh.ctlSet(mCompName, (short)lightness, (short)temp, (short)0, true, transtime, (short)delay);
    }


    public class MyBinder extends Binder {
        LightingService getService() {
            return LightingService.this;
        }
    }

    public interface IServiceCallback {
        void onMeshServiceStatusChangeCb(int status);
        void onDeviceFound(UUID uuid, String name);
        void onProvisionComplete(UUID device, byte status);
        void onHslStateChanged(String deviceName, int lightness, int hue, int saturation, int remainingTime);
        void onOnOffStateChanged(String deviceName, byte targetOnOff, byte presentOnOff, int remainingTime);
        void onLevelStateChanged(String deviceName, short targetLevel, short presentLevel, int remainingTime);
        void onNetworkConnectionStatusChanged(byte transport, byte status);
        void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime);
        void onNodeConnStateChanged(byte isConnected, String componentName);
        void onOtaStatus(byte status, int percentComplete);
        void onNetworkOpenedCallback(byte status);
        void onComponentInfoStatus(byte status, String componentName, String componentInfo);
        void onDfuStatus(byte state, byte[] data);
        void onSensorStatusCb(String componentName, int propertyId, byte[] data);
        void onVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen);
        void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime);
        void onLightLcModeStatus(String componentName, int mode);
        void onLightLcOccupancyModeStatus(String componentName, int mode);
        void onLightLcPropertyStatus(String componentName, int propertyId, int value);
    }

}
