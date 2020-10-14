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
package com.cypress.le.mesh.meshcore;

import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.UUID;
import java.util.concurrent.CountDownLatch;

public class MeshNativeHelper {

    static {
        System.loadLibrary("native-lib");
    }

    private static final String TAG = "MeshNativeHelper";

    private static final int MESH_COMMAND_GATT_PROXY_PKT      = 0;
    private static final int MESH_COMMAND_GATT_PROVISION_PKT  = 1;
    private static final int MESH_EVENT_GATT_PROXY_PKT        = 0;
    private static final int MESH_EVENT_GATT_PROVISION_PKT    = 1;

    static IMeshNativeCallback mCallback;

    private static MeshNativeHelper instance = new MeshNativeHelper();
    private boolean connectedToNetwork;

    public static MeshNativeHelper getInstance() {
        return instance;
    }

    public void registerNativeCallback(IMeshNativeCallback cb) {
        Log.d(TAG, "registerNativeCallback");
        mCallback = cb;
    }

    static void ProcessGattPacket(short opcode, byte[] p_data, int len) {
        Log.d(TAG, "ProcessGattPacket");
        switch (opcode)
        {
            case MESH_EVENT_GATT_PROXY_PKT:
                Log.d(TAG, "MESH_EVENT_GATT_PROXY_PKT");
                mCallback.onProxyGattPktReceivedCallback(p_data,len);
                break;
            case MESH_EVENT_GATT_PROVISION_PKT:
                Log.d(TAG, "MESH_EVENT_GATT_PROVISION_PKT");
                mCallback.onProvGattPktReceivedCallback(p_data,len);
                break;
        }
    }

    static void meshClientProvSendCb (byte[] p_data , int len){
        Log.d(TAG, "meshClientProvSendCb");
        mCallback.onProvGattPktReceivedCallback(p_data,len);

    }

    static void meshClientProxySendCb (byte[] p_data , int len){
        Log.d(TAG, "meshClientProxySendCb" + toHexString(p_data));
        mCallback.onProxyGattPktReceivedCallback(p_data,len);

    }

    static void meshClientDbStateCb(String meshName) {
        Log.d(TAG, "meshClientDbStateCb");
        mCallback.onDatabaseChangedCallback(meshName);
    }

    private static  Handler mTimerHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            timerCallback(msg.what);
        }
    };

    static void startTimercb(final int timerId,  final int timeout)  {
            Message msg = new Message();
            msg.what = timerId;
            msg.arg1 = timeout;
            mTimerHandler.sendMessageDelayed(msg,timeout);
    }

    static void stopTimercb(int timerId) {
        mTimerHandler.removeMessages(timerId);
    }

    static void Sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    static long getTickCountCb(){
        Sleep(10);
        long curTime =  SystemClock.elapsedRealtime();
        Log.d(TAG, "getTickCountCb = " + curTime);
        return curTime;
    }

    public static UUID getUuidFromByteArray(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        long high = bb.getLong();
        long low = bb.getLong();
        UUID uuid = new UUID(high, low);
        return uuid;
    }

    static void meshClientComponentInfoCallback(byte status, String componentName, String componentInfo) {
        Log.d(TAG, "meshClientComponentInfoCallback status :"+ status+ "component Name:"+componentName+ "componentInfo:"+componentInfo);
        mCallback.onComponentInfoStatus(status, componentName, componentInfo);
    }

    static void meshClientUnProvisionedDeviceCb(byte[] uuid, int oob, String name) {
        Log.d(TAG," meshClientUnProvisionedDeviceCb "+toHexString(uuid));
        UUID uuid_temp = getUuidFromByteArray(uuid);
        mCallback.onDeviceFound(uuid_temp, name);
    }

    static void meshClientLinkStatusCb(byte isConnected, int connId, short addr, byte isOverGatt) {
        Log.d(TAG," meshClientLinkStatus "+isConnected);
        mCallback.onLinkStatus(isConnected, connId, addr, isOverGatt);
    }

    static void meshClientLightLcModeStatusCb(String deviceName, int mode)
    {
        Log.d(TAG," meshClientLightLcModeStatusCb "+deviceName);
        mCallback.onLightLcModeStatus(deviceName, mode);
    }

    static void meshClientLightLcOccupancyModeStatusCb(String deviceName, int mode)
    {
        Log.d(TAG," meshClientLightLcOccupancyModeStatusCb "+deviceName);
        mCallback.onLightLcOccupancyModeStatus(deviceName, mode);
    }

    static void meshClientLightLcPropertyStatusCb(String deviceName, int propertyId, int value)
    {
        Log.d(TAG," meshClientLightLcPropertyStatusCb "+deviceName);
        mCallback.onLightLcPropertyStatus(deviceName, propertyId, value);
    }

    static void ProcessData(short opcode, byte[] p_data, int len)
    {
        Log.d(TAG, "ProcessData");
    }

    synchronized public void SendRxProxyPktToCore(byte[] p_data, int length){
        SendGattPktToCore((short)MESH_COMMAND_GATT_PROXY_PKT, p_data, length);
    }

    synchronized public void SendRxProvisPktToCore(byte[] p_data, int length){
        SendGattPktToCore((short)MESH_COMMAND_GATT_PROVISION_PKT, p_data, length);
    }

    char[] dump_hex_string(byte[] data)
    {
        char[] hexArray = "0123456789ABCDEF".toCharArray();
        char[] hexChars = new char[data.length * 2];
        for ( int j = 0; j < data.length; j++ ) {
            int v = data[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return hexChars;
    }

    boolean stopThread = false;
    byte[] byteSendBuffer = null;
    boolean isSendThreadInitialized = false;
    boolean isRecvThreadInitialized = false;
    DatagramSocket dsSend = null;
    InetAddress receiverAddress = null;
    int PORT_NUM = 9877;


    public class NetSendThread implements Runnable {
        public NetSendThread(byte[] sendBuffer) {
            mSendBuffer = sendBuffer;
        }
        public void run() {
            try {
                DatagramPacket packet = new DatagramPacket(mSendBuffer, mSendBuffer.length, receiverAddress, PORT_NUM);
                dsSend.send(packet);
            } catch (IOException e) {
                Log.d(TAG, "SendWicedCommandToUART Exception");
                e.printStackTrace();
            }
        }
        private byte[] mSendBuffer;
    }

    public void SendWicedCommandToUART(short opcode, byte[] p_data, int length) {

        if(!isRecvThreadInitialized) {

            final CountDownLatch latch = new CountDownLatch(1);

            Thread threadRx = new Thread(new Runnable() {

                @Override
                public void run() {

                    Log.d(TAG, "isRecvThreadInitialized");

                    DatagramSocket dsRecv = null;
                    int PORT_NUM = 9877;

                    try {
                        dsRecv = new DatagramSocket(PORT_NUM);
                    }catch (IOException e)
                    {
                        e.printStackTrace();
                    }
                    latch.countDown();

                    while(!stopThread) {
                        try {
                            // read the response from server
                            byte[] responsebuf = new byte[1024];
                            DatagramPacket response = new DatagramPacket(responsebuf, responsebuf.length);
                            dsRecv.receive(response);

                            Log.d(TAG, "Recv Thread Response Length: " + response.getLength());

                            byte buffer[] = response.getData();

                            // This is the entire wiced event forwarded to the application
                            // Get opcode and len and the data after that
                            if (buffer[0] == 0x19) {
                                short opcode = (short) unsignedBytesToLong(buffer, 2, 1);
                                short len = (short) unsignedBytesToLong(buffer, 2, 3);

                                Log.d(TAG, "Recv Thread Response opcode:" + String.format("%04x", opcode) + " Length: " + len);
                                if (len == response.getLength() - 5) {
                                    byte databuf[] = new byte[len];
                                    System.arraycopy(buffer, 5, databuf, 0, len);

                                    ProcessData(opcode, databuf, len);
                                }
                                else {
                                    Log.d(TAG, "Recv Thread illegal length:" + len + responsebuf.length);
                                }
                            }
                            else {
                                Log.d(TAG, "Recv Thread illegal packet:" + String.format("%02x", buffer[0]));
                            }
                        } catch (IOException e) {
                            Log.d(TAG, "SendWicedCommandToUART Exception");
                            e.printStackTrace();
                        }
                    }

                    dsRecv.close();
                }
            });

            isRecvThreadInitialized = true;

            threadRx.start();
            try {
                // this method will block the thread of latch until its released later from thread
                latch.await();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        Log.d(TAG, "SendWicedCommandToUART opcode " + String.format("%04x", opcode) +  " length: " + length);

        if(!isSendThreadInitialized) {
            isSendThreadInitialized = true;
            String IP_ADDR = "192.168.1.16";  // "10.198.225.18";
            try {
                receiverAddress = InetAddress.getByName(IP_ADDR);
                dsSend = new DatagramSocket();
            } catch (IOException e) {
                Log.d(TAG, "SendWicedCommandToUART Exception");
                e.printStackTrace();
            }
        }
        byteSendBuffer = new byte[length + 5];

        byteSendBuffer[0] = 0x19;
        byteSendBuffer[1] = (byte)(opcode & 0xff);
        byteSendBuffer[2] = (byte)((opcode >> 8) & 0xff);
        byteSendBuffer[3] = (byte)(length & 0xff);
        byteSendBuffer[4] = (byte)((length >> 8) & 0xff);

        for(int i = 0; i < length; i++)
            byteSendBuffer[i + 5] = p_data[i];

        String s = new String(dump_hex_string(byteSendBuffer));
        Log.d(TAG, "SendWicedCommandToUART p_data: " + s);

        Runnable r = new NetSendThread(byteSendBuffer);
        new Thread(r).start();
    }

    // use_stack = false to choose controller GATT
    boolean use_stack = true;

    public void SendWicedCommand(short opcode, byte[] p_data, int length)
    {
        if(!use_stack)
            SendWicedCommandToUART(opcode, p_data, length);
    }

    public static native void SendGattPktToCore(short type, byte[] p_data, int length);

    /**
     * Converts a byte array into a long value
     *
     * @param bytes
     * @param bytesToRead
     * @param offset
     * @return
     */
    public static long unsignedBytesToLong(byte[] bytes, int bytesToRead, int offset) {
        int shift = 0;
        long l = 0;
        for (int i = offset; i < offset + bytesToRead; i++) {
            int byteFF = bytes[i] & 0xFF;
            l = l + ((long) (bytes[i] & 0xFF) << (8 * shift++));
        }
        return l;
    }

    //MESH CLIENT APIS
    public native void setFileStorge(String fileStorge);
    public static native int meshClientNetworkExists(String meshName);
    public static native int meshClientNetworkCreate(String provisionerName, String meshName);
    public static native int meshClientNetworkOpen(String provisionerName, String meshName);
    public static native int meshClientNetworkDelete(String provisionerName,String meshName);
    public static native void meshClientNetworkClose();
    public static native void meshClientInit();
    public static native int meshClientGroupCreate(String groupName, String parentGroupName);
    public static native int meshClientGroupDelete(String groupName);
    public static native String[] meshClientGetAllNetworks();
    public static native String[] meshClientGetAllGroups(String inGroup);
    public static native String[] meshClientGetAllProvisioners();
    public static native String[] meshClientGetDeviceComponents(byte[] uuid);
    public static native String[] meshClientGetGroupComponents(String groupName);
    public static native String[] meshClientGetTargetMethods(String componentName);
    public static native String[] meshClientGetControlMethods(String componentName);
    public static native int meshClientGetComponentType(String componentName);
    public static native int meshClientIsLightController(String componentName);
    public static native int meshClientRename(String oldName, String newName);
    public static native int meshClientMoveComponentToGroup(String componentName, String fromGroupName, String toGroupName);
    public static native int meshClientConfigurePublication(String componentName,byte isClient,String method, String targetName, int publishPeriod);
    public static native byte meshClientProvision(String deviceName, String groupName, byte[] uuid, byte identifyDuration);
    public static native byte meshClientConnectNetwork(byte useGattProxy, byte scanDuration);
    public static native byte meshClientDisconnectNetwork(byte useGattProxy);
    public static native void meshClientSetGattMtu(int mtu);
    public static native boolean meshClientIsConnectedToNetwork();
    public static native byte meshClientGetComponentInfo(String componentName);
    public static native int meshClientonoffGet(String deviceName);
    public static native int meshClientonoffSet(String deviceName, byte onoff, boolean reliable, int transitionTime, short delay);
    public static native int meshClientLevelGet(String deviceName);
    public static native int meshClientLevelSet(String deviceName, short level, boolean reliable, int transitionTime, short delay);
    public static native int meshClientHslGet(String deviceName);
    public static native int meshClientHslSet(String deviceName, int lightness, int hue, int saturation, boolean reliable, int transitionTime, short delay);
    public static native int meshClientSetDeviceConfig(String deviceName, int isGattProxy, int isFriend, int isRelay, int beacon, int relayXmitCount, int relayXmitInterval, int defaultTtl, int netXmitCount, int netXmitInterval);
    public static native int meshClientSetPublicationConfig(int publishCredentialFlag, int publishRetransmitCount, int publishRetransmitInterval, int publishTtl);
    public static native byte meshClientResetDevice(String componentName);
    public static native int meshClientVendorDataSet(String deviceName, short companyId, short modelId, byte opcode, boolean disable_ntwk_retransmit, byte[] buffer, short len);
    public static native int meshClientIdentify(String name, byte duration);
    public static native int meshClientLightnessGet(String deviceName);
    public static native int meshClientLightnessSet(String deviceName, int lightness, boolean interim, int transition_time, short delay);
    public static native int meshClientCtlGet(String deviceName);
    public static native int meshClientAddComponentToGroup(String componentName, String groupName);
    public static native int meshClientCtlSet(String deviceName, int lightness, short temperature, short deltaUv, boolean interim, int transition_time, short delay);
    public static native int meshClientListenForAppGroupBroadcasts(String controlMethod, String groupName, boolean startListen);
    public static native String meshClientGetPublicationTarget(String componentName, byte isClient, String method);
    public static native int meshClientGetPublicationPeriod(String componentName, byte isClient, String method);
    //Light Lc APIS
    public static native int meshClientGetLightLcMode(String componentName);
    public static native int meshClientSetLightLcMode(String componentName, int mode);
    public static native int meshClientGetLightLcOccupancyMode(String componentName);
    public static native int meshClientSetLightLcOccupancyMode(String componentName, int mode);
    public static native int meshClientGetLightLcProperty(String componentName, int propertyId);
    public static native int meshClientSetLightLcProperty(String componentName, int propertyId, int val);
    public static native int meshClientSetLightLcOnoffSet(String componentName, byte onoff, boolean reliable, int transitionTime, int delay);

    // OTA crypt helpers
    public static native byte[] meshClientOTADataEncrypt(String componentName,byte[] buffer, int len);
    public static native byte[] meshClientOTADataDecrypt(String componentName,byte[] buffer, int len);

    //MESH CLIENT GATT APIS
    public static native void meshClientScanUnprovisioned(int start, byte[] uuid);
    public static native boolean meshClientIsConnectingProvisioning();
    public static native void meshClientConnectionStateChanged(short conn_id, short mtu);
    public static native void meshClientAdvertReport(byte[] bdaddr, byte addrType, byte rssi, byte[] advData, int advLen);
    public static native byte meshConnectComponent(String componentName, byte useProxy, byte scanDuration);

    //MESH TIMER APIS
    public static native void timerCallback(long l);

    //IMPORT EXPORT
    public static native String meshClientNetworkImport(String provisionerName, String jsonString);
    public static native String meshClientNetworkExport(String meshName);

    public static native String[] meshClientGetComponentGroupList(String componentName);
    public static native int meshClientRemoveComponentFromGroup(String componentName, String groupName);

    //DFU APIS
    public static native int  meshClientDfuStart(String firmwareFile, byte dfuMethod);
    public static native int  meshClientDfuStop();
    public static native void meshClientDfuOtaFinished(byte status);
    public static native void meshClientDfuGetStatus(int statusInterval);

    //SENSOR APIS
    public static native int meshClientSensorCadenceSet(String deviceName, int propertyId, short fastCadencePeriodDivisor, boolean triggerType,
                                                        int triggerDeltaDown, int triggerDeltaUp, int minInterval, int fastCadenceLow, int fastCadenceHigh);
    public static native short[] meshClientSensorSettingsGetPropIds(String componentName, int propertyId);
    public static native int[] meshClientSensorPropertyListGet(String componentName);
    public static native int meshClientSensorSettingSet(String componentName, int propertyId, short settingPropertyId, byte[] val);
    public static native int meshClientSensorGet(String componentName, int propertyId);

    public static native int meshClientNetworkConnectionChanged(int connId);

    static void meshClientProvisionCompletedCb(byte isSuccess, byte[] uuid) {
        mCallback.meshClientProvisionCompletedCb(isSuccess, uuid);
    }

    static void meshClientDisconnectCb(int connId) {
        mCallback.meshClientDisconnect(connId);
    }

    static void meshClientConnectCb(byte[] bdAddr) {
        mCallback.meshClientConnect(bdAddr);
    }

    static void meshClientAdvScanStartCb() {
        mCallback.meshClientAdvScanStartCb();
    }

    static void meshClientAdvScanStopCb() {
        mCallback.meshClientAdvScanStopCb();
    }

    static void meshClientNetworkOpenedCb(byte status) {mCallback.onNetworkOpenedCb(status);}

    static void meshClientOnOffStateCb(String deviceName, byte targetOnOff, byte presentOnOff, int remainingTime) {
        Log.d(TAG, "onoffStatus dev:"+deviceName+ " target status:"+ targetOnOff + " present status:"+ presentOnOff +" remaining time:" + remainingTime);
        mCallback.meshClientOnOffStateCb(deviceName, targetOnOff, presentOnOff, remainingTime);
    }

    static void meshClientLevelStateCb(String deviceName, short targetLevel, short presentLevel, int remainingTime) {
        Log.d(TAG, "meshClientLevelStateCb dev:"+deviceName+ " target level:"+ targetLevel + " present level:"+ presentLevel +" remaining time:" + remainingTime);
        mCallback.meshClientLevelStateCb(deviceName, targetLevel, presentLevel, remainingTime);
    }

    static void meshClientHslStateCb(String deviceName, int lightness, int hue, int saturation, int remainingTime) {
        Log.d(TAG, "meshClientHslStateCb dev:"+deviceName+ " lightness:"+lightness + " hue" + hue + " saturation:" +saturation + " remaining time:" + remainingTime);
        mCallback.meshClientHslStateCb(deviceName, lightness, hue, saturation, remainingTime);
    }

    static void meshClientCtlStateCb(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {
        Log.d(TAG, "meshClientCtlStateCb dev:"+deviceName+ " lightness:"+presentLightness);
        mCallback.meshClientCtlStateCb(deviceName, presentLightness, presentTemperature, targetLightness, targetTemperature, remainingTime);
    }

    static void meshClientLightnessStateCb(String deviceName, int target, int present, int remainingTime) {
        Log.d(TAG, "meshClientLightnessStateCb dev:"+deviceName+ " lightness:"+present);
        mCallback.meshClientLightnessStateCb(deviceName, target, present, remainingTime);
    }

    static void meshClientNodeConnectStateCb(byte isConnected, String componentName) {
        Log.d(TAG, "meshClientNodeConnectStateCb componentName:"+componentName+ " isConnected:"+isConnected);
        mCallback.meshClientNodeConnectStateCb(isConnected, componentName);
    }

    static void meshClientSetAdvScanTypeCb(byte scantype) {
        Log.d(TAG, "meshClientSetAdvScanTypeCb");
        mCallback.meshClientSetScanTypeCb(scantype);
    }

    static boolean meshClientDfuIsOtaSupportedCb() {
        Log.i(TAG, "meshClientDfuIsOtaSupportedCb");
        return mCallback.meshClientDfuIsOtaSupportedCb();
    }

    static void meshClientDfuStartOtaCb(String firmwareFileName) {
        Log.i(TAG, "meshClientDfuStartOtaCb: " + firmwareFileName);
        mCallback.meshClientDfuStartOtaCb(firmwareFileName);
    }

    static void meshClientDfuStatusCb(byte state, byte[] data) {
        Log.i(TAG, "meshClientDfuStatus");
        mCallback.meshClientDfuStatusCb(state, data);
    }

    static void meshClientSensorStatusCb(String componentName, int propertyid, byte[] data) {
        Log.d(TAG, "meshClientSensorStatus");
        mCallback.meshClientSensorStatusCb(componentName, propertyid, data);
    }

    static void meshClientVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen) {
        Log.d(TAG, "meshClientSensorStatus");
        mCallback.meshClientVendorStatusCb(src, companyId, modelId, opcode, ttl, data, dataLen);
    }

    static String toHexString(byte[] bytes) {
        int len = bytes.length;
        if(len == 0)
            return null;

        char[] buffer = new char[len * 3 - 1];

        for (int i = 0, index = 0; i < len; i++) {
            if (i > 0) {
                buffer[index++] = ' ';
            }

            int data = bytes[i];
            if (data < 0) {
                data += 256;
            }

            byte n = (byte) (data >>> 4);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            }
            else {
                buffer[index++] = (char) ('A' + n - 10);
            }

            n = (byte) (data & 0x0F);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            }
            else {
                buffer[index++] = (char) ('A' + n - 10);
            }
        }
        return new String(buffer);
    }

}
