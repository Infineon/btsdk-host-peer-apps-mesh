package com.cypress.le.mesh.meshapp;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
//import android.provider.Settings;
//import android.support.v4.graphics.ColorUtils;
//import android.test.suitebuilder.TestMethod;
import android.text.TextUtils;
import android.util.Log;

import com.cypress.le.mesh.meshframework.IMeshControllerCallback;

import com.cypress.le.mesh.meshframework.MeshController;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

import static com.cypress.le.mesh.meshapp.Constants.toHexString;

public class MeshAm extends Service {

    private static final int MSG_SCAN_TIMEOUT = 1;
    private static final int MSG_DEFUALT_TIMEOUT = 2;
    private static final int MESH_DEFUALT_TIMEOUT = 100;
    MeshController mMeshController;

    MeshBluetoothDevice mCurrBtDev;
    String mCurrentNtw;
    ArrayList<MeshBluetoothDevice> mDevices = new ArrayList<MeshBluetoothDevice>();
    BroadcastReceiver.PendingResult pendingResult;
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.create.network --es name "network_name" --es provName "provisioner_name"
     */
    public static final String CREATE_NETWORK          = "com.cypress.le.mesh.meshapp.create.network";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.open.network --es name "network_name" --es provName "provisioner_name"
     */
    public static final String OPEN_NETWORK            = "com.cypress.le.mesh.meshapp.open.network";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.close.network"
     */
    public static final String CLOSE_NETWORK           = "com.cypress.le.mesh.meshapp.close.network";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.create.group --es groupName "group_name" --es parentGroupName "parent_group"
     */
    public static final String CREATE_GROUP            = "com.cypress.le.mesh.meshapp.create.group";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.rename --es oldName "old_name" --es newName "new_name"
     */
    public static final String RENAME                  = "com.cypress.le.mesh.meshapp.rename";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.groups --es inGroup "inGroup"
     */
    public static final String GET_ALL_GROUPS          = "com.cypress.le.mesh.meshapp.get.all.groups";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.networks
     */
    public static final String GET_ALL_NETWORKS        = "com.cypress.le.mesh.meshapp.get.all.networks";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.provisioners
     */
    public static final String GET_ALL_PROVISIONERS    = "com.cypress.le.mesh.meshapp.get.all.provisioners";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.device.components --es uuid "uuid in form ff00ff00-ff00-ff00-ff00-ff00ff00ff00"
     */
    public static final String GET_DEVICE_COMPONENTS   = "com.cypress.le.mesh.meshapp.get.device.components";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.group.components --es groupName "group_name"
     */
    public static final String GET_GROUP_COMPONENTS    = "com.cypress.le.mesh.meshapp.get.group.components";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.component.type --es componentName "component_name"
     */
    public static final String GET_COMPONENT_TYPE      = "com.cypress.le.mesh.meshapp.get.component.type";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.identify --es name "name" --ei duration "duration"
     */
    //private static final String IDENTIFY                = "com.cypress.le.mesh.meshapp.identify";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.scan.mesh.devices.start --es uuid "uuid in form ff00ff00-ff00-ff00-ff00-ff00ff00ff00" --ei duration 1000
     */
    private static final String START_SCAN_MESH_DEVICES = "com.cypress.le.mesh.meshapp.scan.mesh.devices.start";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.scan.mesh.devices.stop
     */

    private static final String STOP_SCAN_MESH_DEVICES  = "com.cypress.le.mesh.meshapp.scan.mesh.devices.stop";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.provision --es groupName "mygroup" --es deviceName "mylight" --es identity 10 --es uuid "ff00ff00-ff00-ff00-ff00-ff00ff00ff00"
     */
    private static final String PROVISION               = "com.cypress.le.mesh.meshapp.provision";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.reset.device --es componentName "componentName"
     */
    private static final String RESET_DEVICE            = "com.cypress.le.mesh.meshapp.reset.device";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.target.methods --es componentName "componentName"
     */
    private static final String GET_TARGET_METHODS      = "com.cypress.le.mesh.meshapp.get.target.methods";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.control.methods --es componentName "componentName"
     */
    private static final String GET_CONTROL_METHODS     = "com.cypress.le.mesh.meshapp.get.control.methods";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.connect.component --es componentName "componentName" --ei useGATTProxy 1 --ei scanDuration 1000
     */
    private static final String CONNECT_COMPONENT       = "com.cypress.le.mesh.meshapp.connect.component";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.onoff.get --es componentName "mylight\ \(0002\)"
     * note  : on the adb to support special characters escape sequence should be added (\)
     * in the above command the component name is "mylight (0002)" this has to be represented as "mylight\ \(0002\)"
     */

    private static final String ONOFF_GET               = "com.cypress.le.mesh.meshapp.onoff.get";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.onoff.set --es componentName "mylight\ \(0002\)" --ez onOff true --ez reliable true --ei delay 0 --ei transitionTime 0
     */
    private static final String ONOFF_SET               = "com.cypress.le.mesh.meshapp.onoff.set";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.level.get --es componentName "mylight\ \(0002\)"
     */
    private static final String LEVEL_GET               = "com.cypress.le.mesh.meshapp.level.get";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.level.set --es componentName "mylight\ \(0002\)" --ez level 11 --ez reliable true --ei delay 0 --ei transitionTime 0
     */
    private static final String LEVEL_SET               = "com.cypress.le.mesh.meshapp.level.set";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.lightness.get --es componentName "mylight\ \(0002\)"
     */
    private static final String LIGHTNESS_GET           = "com.cypress.le.mesh.meshapp.lightness.get";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.lightness.set --es componentName "mylight\ \(0002\)" --ei lightness 100 --ez reliable true --ei delay 0 --ei transitionTime 0
     */
    private static final String LIGHTNESS_SET           = "com.cypress.le.mesh.meshapp.lightness.set";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.hsl.get --es componentName "mylight\ \(0002\)"
     */
    private static final String HSL_GET                 = "com.cypress.le.mesh.meshapp.hsl.get";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.hsl.set --es componentName "mylight\ \(0002\)" --ei hue 15 --ei saturation 20 --ei lightness 100 --ez onOff true --ez reliable true --ei delay 0 --ei transitionTime 0
     */
    private static final String HSL_SET                 = "com.cypress.le.mesh.meshapp.hsl.set";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.ctl.get --es componentName "mylight\ \(0002\)"
     */
    private static final String CTL_GET                 = "com.cypress.le.mesh.meshapp.ctl.get";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.ctl.set --es componentName "mylight\ \(0002\)" --ei temperature 15 --ei deltaUv 20 --ei lightness 100 --ez onOff true --ez reliable true --ei delay 0 --ei transitionTime 0
     */
    private static final String CTL_SET                 = "com.cypress.le.mesh.meshapp.ctl.set";
    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.import.network --es provName "myprov" --es filepath "/sdcard/exports/newnetwork.json"
     */
    private static final String IMPORT_NETWORK          = "com.cypress.le.mesh.meshapp.import.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.export.network --es networkName "myNetwork"
     */
    private static final String EXPORT_NETWORK          = "com.cypress.le.mesh.meshapp.export.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.start.ota.upgrade --es componentName "componentName" --es fileName "fileName"
     */
    private static final String START_OTA_UPGRADE       = "com.cypress.le.mesh.meshapp.start.ota.upgrade";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.stop.ota.upgrade
     */
    private static final String STOP_OTA_UPGRADE        = "com.cypress.le.mesh.meshapp.stop.ota.upgrade";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.connect.network --ei transport 1 --ei scanDuration 1000
     */
    private static final String CONNECT_NETWORK         = "com.cypress.le.mesh.meshapp.connect.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.delete.group --es groupName "groupName"
     */
    private static final String DELETE_GROUP            = "com.cypress.le.mesh.meshapp.delete.group";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.delete.network --es provisionerName "myprov" --es componentName "componentName"
     */
    private static final String DELETE_NETWORK          = "com.cypress.le.mesh.meshapp.delete.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.disconnect.network --ei transport 1
     */
    private static final String DISCONNECT_NETWORK      = "com.cypress.le.mesh.meshapp.disconnect.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.current.network
     */
    private static final String GET_CURRENT_NETWORK     = "com.cypress.le.mesh.meshapp.get.current.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.initialize
     */
    private static final String INITIALIZE              = "com.cypress.le.mesh.meshapp.initialize";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.is.connected.to.network
     */
    private static final String IS_CONNECTED_TO_NETWORK = "com.cypress.le.mesh.meshapp.is.connected.to.network";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.is.network.exist --es meshName "myNetwork"
     */
    private static final String IS_NETWORK_EXIST        = "com.cypress.le.mesh.meshapp.is.network.exist";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.move.component.to.group --es componentName "myComponent" --es fromGroupName "groupName" --es toGroupName "groupName"
     */
    private static final String MOVE_COMPONENT_TO_GROUP = "com.cypress.le.mesh.meshapp.move.component.to.group";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.remove.component.from.group --es componentName "myComponent" --es groupName "groupName"
     */
    private static final String REMOVE_COMPONENT_FROM_GROUP = "com.cypress.le.mesh.meshapp.remove.component.from.group";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.add.component.to.group --es componentName "myComponent" --es groupName "groupName"
     */
    private static final String ADD_COMPONENT_TO_GROUP = "com.cypress.le.mesh.meshapp.add.component.to.group";

    //private static final String SEND_RCVD_PROXY_PKT     = "com.cypress.le.mesh.meshapp.send.rcvd.proxy.pkt";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.start.tracking
     */
    private static final String START_TRACKING          = "com.cypress.le.mesh.meshapp.start.tracking";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.stop.tracking
     */
    private static final String STOP_TRACKING           = "com.cypress.le.mesh.meshapp.stop.tracking";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.uninitialize
     */
    private static final String UN_INITIALIZE           = "com.cypress.le.mesh.meshapp.uninitialize";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.set.publication.config --es deviceName "deviceName" --ei deviceType 0 --ei publishPeriod 10
     * --es publishCredentialFlag 0 --ei publishRetransmitCount 0 --ei publishRetransmitInterval 500
     * --ei publishTtl 63
     */
    private static final String SET_PUBLICATION_CONFIG  = "com.cypress.le.mesh.meshapp.set.publication.config";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.set.device.config --es deviceName "deviceName" --ez isGattProxy true --ez isFriend true
     * --ez isRelay true --ei relayXmitCount 3 --ei relayXmitInterval 100 --ei defaultTtl 63
     * --ei netXmitCount 3 --ei netXmitInterval 100
     */
    private static final String SET_DEVICE_CONFIG       = "com.cypress.le.mesh.meshapp.set.device.config";

    /**
     * @usage adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.component.info --es componentName "mylight\ \(0002\)"
     */
    private static final String GET_COMPONENT_INFO      = "com.cypress.le.mesh.meshapp.get.component.info";
    private static final String TAG                     = "MeshAm";

    BroadcastReceiver mReceiver;
    boolean isProvisioning = false;
    boolean isImportInProgress = false;
    String mUuid = null;

    {
        mReceiver = new BroadcastReceiver() {


            @Override
            public void onReceive(Context context, final Intent intent) {
                Log.d(TAG, "Recevied intent" + intent.getAction());

                switch (intent.getAction()) {
                    case CREATE_NETWORK: {
                        //create a network
                        Bundle extras = intent.getExtras();
                        String ntwName = "DEFAULT NETWORK";
                        String provName = "DEFAULT PROVISIONER";
                        if (extras.containsKey("name")) {
                            ntwName = extras.getString("name");
                            provName = extras.getString("provName");
                        }
                        int res = mMeshController.createNetwork(provName, ntwName);
                        Log.d(TAG, "create Network return : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
                    case OPEN_NETWORK: {
                        //create a network
                        Bundle extras = intent.getExtras();
                        String ntwName = "DEFAULT NETWORK";
                        String provName = "DEFAULT PROVISIONER";
                        if (extras.containsKey("name")) {
                            ntwName = extras.getString("name");
                        }
                        if (extras.containsKey("provName")) {
                            provName = extras.getString("provName");
                        }

                        pendingResult = goAsync();
                        final String networkName = ntwName;
                        final String provisionerName = provName;

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                Message msg = new Message();
                                msg.what = MSG_DEFUALT_TIMEOUT;
                                mCurrentNtw = networkName;
                                mHandler.sendMessageDelayed(msg, MESH_DEFUALT_TIMEOUT);
                                Log.d(TAG,"opennetwork : prov:"+provisionerName+"networkName:"+networkName);

                                int res = mMeshController.openNetwork(provisionerName, networkName);
                                Log.d(TAG,"openNetworkresult : "+res);
                                if(res != MeshController.MESH_CLIENT_SUCCESS && pendingResult != null) {
                                    pendingResult.setResultCode(res);
                                    pendingResult.finish();
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case CLOSE_NETWORK: {
                        //close a network
                        int res = mMeshController.closeNetwork();
                        Log.d(TAG, "Close Network return : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
                    case RESET_DEVICE: {
                        Bundle extras = intent.getExtras();
                        String componentName = null;
                        if (extras.containsKey("componentName")) {
                            componentName = extras.getString("componentName");
                        }
                        int res = mMeshController.resetDevice(componentName);
                        Log.d(TAG, "Reset device : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
/*                    case IDENTIFY: {
                        Bundle extras = intent.getExtras();
                        if (!extras.containsKey("name")) {
                            Log.d(TAG, "No name field present");
                        }
                        if (!extras.containsKey("duration")) {
                            Log.d(TAG, "No duration field present");
                        }
                        String name = extras.getString("name");
                        byte duration = (byte)extras.getInt("duration");
                        int res = mMeshController.identify(name, duration);
                        Log.d(TAG, "identify result : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
*/
                    case START_SCAN_MESH_DEVICES: {
                        Log.d(TAG, "START_SCAN_MESH_DEVICES start");
                        Bundle extras = intent.getExtras();
                        if (extras == null) {
                            Log.d(TAG, "No extras present");
                        }
                        if (!extras.containsKey("uuid")) {
                            Log.d(TAG, "No uuid field present");
                        }
                        if (!extras.containsKey("duration")) {
                            Log.d(TAG, "No duration field present");
                        }
                        pendingResult = goAsync();
                        Message msg = new Message();
                        msg.what = MSG_SCAN_TIMEOUT;
                        mUuid = extras.getString("uuid");
                        int duration = extras.getInt("duration");
                        mHandler.sendMessageDelayed(msg, duration);
                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                mMeshController.scanMeshDevices(true, null);
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;
                    case STOP_SCAN_MESH_DEVICES: {
                        Log.d(TAG, "START_SCAN_MESH_DEVICES stop");
                        mMeshController.scanMeshDevices(false, null);
                        setResultCode(MeshController.MESH_CLIENT_SUCCESS);
                    }
                    break;
                    case CREATE_GROUP: {
                        Log.d(TAG, "CREATE_GROUP");
                        Bundle extras = intent.getExtras();
                        String groupName = "DEFAULT GROUP";
                        String parentGroupName = "ALL";
                        if (extras.containsKey("groupName")) {
                            groupName = extras.getString("groupName");
                        }
                        if (extras.containsKey("parentGroupName")) {
                            parentGroupName = extras.getString("parentGroupName");
                        }
                        Log.d(TAG, "Create group  : " + groupName+" parent"+parentGroupName);

                        int res = mMeshController.createGroup(groupName, parentGroupName);
                        Log.d(TAG, "Create group return : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
                    case RENAME: {
                        Log.d(TAG, "RENAME");
                        Bundle extras = intent.getExtras();
                        String oldName = null;
                        String newName = null;
                        if (!extras.containsKey("oldName")) {
                            Log.d(TAG, "No oldName field present");
                            return;
                        }
                        if (!extras.containsKey("newName")) {
                            Log.d(TAG, "No newName field present");
                            return;
                        }
                        oldName = extras.getString("oldName");
                        newName = extras.getString("newName");
                        int res = mMeshController.rename(oldName, newName);
                        Log.d(TAG, "Rename return : " + returnIntTypeToString(res));
                        setResultCode(res);
                    }
                    break;
                    case GET_ALL_GROUPS: {
                        Log.d(TAG, "GET_ALL_GROUPS");
                        Bundle extras = intent.getExtras();
                        String inGroup = null;

                        if (!extras.containsKey("inGroup")) {
                            Log.d(TAG, "No inGroup field present");
                            return;
                        }
                        inGroup = extras.getString("inGroup");
                        String[] groups = mMeshController.getSubGroups(inGroup);
                        Log.d(TAG, "getAllGroups return : " + groups);
                        String groupArray = TextUtils.join(",",groups);
                        setResultData(groupArray);
                    }
                    break;
                    case GET_ALL_NETWORKS: {
                        Log.d(TAG, "GET_ALL_NETWORKS");
                        String[] networks = mMeshController.getAllNetworks();
                        String ntwks = TextUtils.join(",", networks);
                        Log.d(TAG, "getAllNetworks return : " + ntwks);
                        setResultData(ntwks);
                    }
                    break;
                    case GET_ALL_PROVISIONERS: {
                        Log.d(TAG, "GET_ALL_PROVISIONERS");
                        String[] provisionersArray = mMeshController.getAllProvisioners();
                        Log.d(TAG, "getAllProvisioners return : " + provisionersArray);
                        String provisioners = TextUtils.join(",", provisionersArray);
                        setResultData(provisioners);
                    }
                    break;
                    case GET_DEVICE_COMPONENTS: {
                        Log.d(TAG, "GET_DEVICE_COMPONENTS");
                        Bundle extras = intent.getExtras();
                        String uuidStr = null;
                        if (!extras.containsKey("uuid")) {
                            Log.d(TAG, "No uuid field present");
                            return;
                        }
                        uuidStr = extras.getString("uuid");
                        UUID uuid = UUID.fromString(uuidStr);
                        long msb = uuid.getMostSignificantBits();
                        byte[] buffer = new byte[16];
                        int index = 0;
                        for (int i = 0; i < 8; i++) {
                            buffer[index + 7 - i] = (byte)(msb & 0xff);
                            msb = (msb >> 8);
                        }
                        index = index + 8;
                        long lsb = uuid.getLeastSignificantBits();
                        for (int i = 0; i < 8; i++) {
                            buffer[index + 7 - i] = (byte)(lsb & 0xff);
                            lsb = (lsb >> 8);
                        }
                        Log.d(TAG,"uuid : "+toHexString(buffer));

                        String[] componentArray = mMeshController.getDeviceComponents(buffer);
                        Log.d(TAG, "getDeviceComponents return : " + componentArray);
                        String components = TextUtils.join(",", componentArray);
                        setResultData(components);
                    }
                    break;
                    case GET_COMPONENT_TYPE: {
                        Log.d(TAG, "GET_COMPONENT_TYPE");
                        Bundle extras = intent.getExtras();
                        String componentName = null;
                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "No componentName field present");
                            return;
                        }
                        componentName = extras.getString("componentName");
                        int componentType = mMeshController.getComponentType(componentName);
                        Log.d(TAG, "getComponentType return : " + componentType);
                        setResultCode(componentType);
                    }
                    break;
                    case GET_GROUP_COMPONENTS: {
                        Log.d(TAG, "GET_GROUP_COMPONENTS");
                        Bundle extras = intent.getExtras();
                        String groupName = null;
                        if (!extras.containsKey("groupName")) {
                            Log.d(TAG, "No groupName field present");
                            return;
                        }
                        groupName = extras.getString("groupName");
                        String[] componentsArray = mMeshController.getGroupComponents(groupName);

                        String components = TextUtils.join(",", componentsArray);
                        Log.d(TAG, "getGroupcomponent "+components);
                        setResultData(components);
                    }
                    break;

                    case PROVISION: {
                        Bundle extras = intent.getExtras();
                        String uuidStr = null;
                        String deviceName = null;
                        String groupName = null;
                        MeshBluetoothDevice device = null;
                        byte identity = 0;
                        if (!extras.containsKey("deviceName") ||
                                !extras.containsKey("uuid") ||
                                !extras.containsKey("groupName") ||
                                !extras.containsKey("identityDuration")
                                ) {
                            Log.d(TAG,"invalid params");
                            setResultCode(Activity.RESULT_CANCELED);
                        }
                        if (extras.containsKey("deviceName")) {
                            deviceName = extras.getString("deviceName");
                        }
                        if (extras.containsKey("uuid")) {
                            uuidStr = extras.getString("uuid");
                        }
                        if (extras.containsKey("groupName")) {
                            groupName = extras.getString("groupName");
                        }
                        if (extras.containsKey("identityDuration")) {
                            identity = (byte)extras.getInt("identityDuration");
                        }
                        UUID uuid = UUID.fromString(uuidStr);
                        Log.d(TAG, "UUID seletcted :" + uuidStr);
                        if (mDevices != null) {
                            for (int i = 0; i < mDevices.size(); i++) {
                                Log.d(TAG, "UUID :" + mDevices.get(i).mUUID.toString() + " compare:" + uuid.toString());
                                if (mDevices.get(i).mUUID.equals(uuid)) {
                                    device = mDevices.get(i);
                                    Log.d(TAG, "Device found!!!");
                                    break;
                                }
                            }

                        } else {
                            return;
                        }
                        mMeshController.setDeviceConfig(
                                null,
                                Constants.DEFAULT_IS_GATT_PROXY,
                                Constants.DEFAULT_IS_FRIEND,
                                Constants.DEFAULT_IS_RELAY,
                                Constants.DEFAULT_SEND_NET_BEACON,
                                Constants.DEFAULT_RELAY_XMIT_COUNT,
                                Constants.DEFAULT_RELAY_XMIT_INTERVAL,
                                Constants.DEFAULT_TTL,
                                Constants.DEFAULT_NET_XMIT_COUNT,
                                Constants.DEFAULT_NET_XMIT_INTERVAL
                        );

                        mMeshController.setPublicationConfig(
                                Constants.DEFAULT_PUBLISH_CREDENTIAL_FLAG,
                                Constants.DEFAULT_RETRANSMIT_COUNT,
                                Constants.DEFAULT_RETRANSMIT_INTERVAL,
                                Constants.DEFAULT_PUBLISH_TTL
                        );
                        pendingResult = goAsync();
                        final String finalDeviceName = deviceName;
                        final String finalGroupName = groupName;
                        final MeshBluetoothDevice finalDevice = device;
                        final byte finalIdentity = identity;
                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                isProvisioning = true;
                                byte res = mMeshController.provision(finalDeviceName, finalGroupName, finalDevice.mUUID, finalIdentity);
                                if(res != MeshController.MESH_CLIENT_SUCCESS)
                                {
                                    isProvisioning = false;
                                    setResultCode(res);
                                    pendingResult.finish();
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;

                    case GET_TARGET_METHODS: {
                        Bundle extras = intent.getExtras();
                        String componentName = null;
                        if (extras.containsKey("componentName")) {
                            componentName = extras.getString("componentName");
                        }
                        String[] res = mMeshController.getTargetMethods(componentName);
                        String targetMethods = TextUtils.join(",", res);
                        setResultData(targetMethods);
                    }
                    break;

                    case GET_CONTROL_METHODS: {
                        Bundle extras = intent.getExtras();
                        String componentName = null;
                        if (extras.containsKey("componentName")) {
                            componentName = extras.getString("componentName");
                        }
                        String[] res = mMeshController.getControlMethods(componentName);
                        String controlMethods = TextUtils.join(",", res);
                        setResultData(controlMethods);
                    }
                    break;
                    case IMPORT_NETWORK: {
                        Bundle extras = intent.getExtras();
                        if (!extras.containsKey("provisionerName")) {
                            Log.d(TAG, "provisionerName field is not present");
                        }
                        if (!extras.containsKey("filepath")) {
                            Log.d(TAG, "filepath field is not present");
                        }
                        final String provisionerName = extras.getString("provisionerName");
                        final String path = extras.getString("filepath");
                        final String ifxpath = path.substring(0, path.length() - 4) + "ifx.json";

                        pendingResult = goAsync();
                        isImportInProgress = true;

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                String json = null;
                                String ifxjson = null;
                                int size = 0;
                                try {
                                    FileInputStream is = new FileInputStream(path);
                                    size = is.available();
                                    byte[] buffer = new byte[size];
                                    is.read(buffer);
                                    is.close();
                                    json = new String(buffer, "UTF-8");
                                    Log.d(TAG,"content : "+json.length() + "actual len:"+size);
                                    JSONObject obj = new JSONObject(json);
                                    try {
                                        is = new FileInputStream(ifxpath);
                                        is.read(buffer);
                                        is.close();
                                        ifxjson = new String(buffer, "UTF-8");
                                    }
                                    catch (FileNotFoundException e) {
                                        ifxjson = new String("");
                                    }
                                    mCurrentNtw = mMeshController.importNetwork(provisionerName, json, ifxjson);
                                    Log.d(TAG,"mCurrentNtw : "+mCurrentNtw);
                                } catch (IOException e) {
                                    e.printStackTrace();
                                } catch (JSONException e) {
                                    e.printStackTrace();
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case EXPORT_NETWORK: {
                        Bundle extras = intent.getExtras();
                        String meshName = null;

                        if (!extras.containsKey("networkName")) {
                            Log.d(TAG, "networkName field is not present");
                        }
                        meshName = extras.getString("networkName");
                        String jsonString = mMeshController.exportNetwork(meshName);
                        //Log.d(TAG, "export network:"+jsonString);
                        setResultData(jsonString);
                    }
                    break;
                    case START_OTA_UPGRADE: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("fileName")) {
                            Log.d(TAG, "fileName field is not present");
                        }
                        String fileName = extras.getString("fileName");
                        int res = mMeshController.startOta(fileName);
                        setResultCode(res);
                    }
                    break;
                    case STOP_OTA_UPGRADE: {
                        Bundle extras = intent.getExtras();
                        if (!extras.containsKey("dfuMethod")) {
                            Log.d(TAG, "dfuMethod field is not present");
                        }
                        byte dfuMethod =(byte)extras.getInt("dfuMethod");
                        int res = mMeshController.stopOta();
                        setResultCode(res);
                    }
                    break;
                    case ONOFF_GET: {
                        Log.d(TAG,"ONOFF_GET");
                        Bundle extras = intent.getExtras();
                        String componentName = null;
                        if (extras.containsKey("componentName")) {
                            componentName = extras.getString("componentName");
                        }

                        pendingResult = goAsync();
                        final String finalComponentName = componentName;
                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.onoffGet(finalComponentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    setResultCode(res);
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case ONOFF_SET: {
                        Bundle extras = intent.getExtras();
                        Log.d(TAG,"ONOFF_SET");

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("onOff")) {
                            Log.d(TAG, "onOff is not present");
                            return;
                        }
                        if (!extras.containsKey("reliable")) {
                            Log.d(TAG, "reliable is not present");
                            return;
                        }
                        if (!extras.containsKey("transitionTime")) {
                            Log.d(TAG, "transitionTime is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final Boolean onoff = extras.getBoolean("onOff");
                        final Boolean reliable = extras.getBoolean("reliable");
                        final int delay = extras.getInt("delay");
                        final int transitionTime = extras.getInt("transitionTime");
                        Log.d(TAG,"ONOFF_SET onoff"+onoff);
                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.onoffSet(componentName, onoff, reliable, transitionTime, (short) delay);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {

                                    if(pendingResult != null)
                                    {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }
                                    return null;
                                }
                                if(!reliable)
                                {

                                    if(pendingResult != null)
                                    {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }
                                }

                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case LEVEL_GET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                        }
                        pendingResult = goAsync();
                        final String componentName = extras.getString("componentName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.levelGet(componentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    setResultCode(res);
                                    pendingResult.finish();
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;
                    case LEVEL_SET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("level")) {
                            Log.d(TAG, "level is not present");
                            return;
                        }
                        if (!extras.containsKey("reliable")) {
                            Log.d(TAG, "reliable is not present");
                            return;
                        }
                        if (!extras.containsKey("transitionTime")) {
                            Log.d(TAG, "transitionTime is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final short level = (short) extras.getInt("level");
                        final Boolean reliable = extras.getBoolean("reliable");
                        final short delay = (short) extras.getInt("delay");
                        final int transitionTime = extras.getInt("transitionTime");
                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.levelSet(componentName, level, reliable, transitionTime, delay);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    setResultCode(res);
                                    if(pendingResult != null)
                                        pendingResult.finish();
                                    pendingResult = null;
                                    return null;
                                }
                                if(!reliable)
                                {
                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case LIGHTNESS_GET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                        }
                        pendingResult = goAsync();
                        final String componentName = extras.getString("componentName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.lightnessGet(componentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {

                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }

                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;
                    case LIGHTNESS_SET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("lightness")) {
                            Log.d(TAG, "lightness is not present");
                            return;
                        }
                        if (!extras.containsKey("reliable")) {
                            Log.d(TAG, "reliable is not present");
                            return;
                        }
                        if (!extras.containsKey("transitionTime")) {
                            Log.d(TAG, "transitionTime is not present");
                            return;
                        }
                        if (!extras.containsKey("delay")) {
                            Log.d(TAG, "delay is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final short lightness = (short) extras.getInt("lightness");
                        final Boolean reliable = extras.getBoolean("reliable");
                        final short delay = (short) extras.getInt("delay");
                        final int transitionTime = extras.getInt("transitionTime");
                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.lightnessSet(componentName, lightness, reliable, transitionTime, delay);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {

                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }

                                    pendingResult = null;
                                    return null;
                                }
                                if(!reliable)
                                {
                                    setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case HSL_SET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("hue")) {
                            Log.d(TAG, "hue is not present");
                            return;
                        }
                        if (!extras.containsKey("saturation")) {
                            Log.d(TAG, "saturation is not present");
                            return;
                        }

                        if (!extras.containsKey("lightness")) {
                            Log.d(TAG, "lightness is not present");
                            return;
                        }

                        if (!extras.containsKey("reliable")) {
                            Log.d(TAG, "reliable is not present");
                            return;
                        }
                        if (!extras.containsKey("transitionTime")) {
                            Log.d(TAG, "transitionTime is not present");
                            return;
                        }
                        if (!extras.containsKey("delay")) {
                            Log.d(TAG, "delay is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final int lightness = extras.getInt("lightness");
                        final int hue = extras.getInt("hue");
                        final int saturation = extras.getInt("saturation");
                        final Boolean reliable = extras.getBoolean("reliable");
                        final short delay = extras.getShort("delay");
                        final int transitionTime = extras.getInt("transitionTime");
                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.hslSet(componentName, lightness, hue, saturation, reliable, transitionTime, delay);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {

                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }
                                    pendingResult = null;
                                    return null;
                                }
                                if(!reliable)
                                {
                                    setResultCode(res);
                                    if(pendingResult != null)
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case HSL_GET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                        }
                        pendingResult = goAsync();
                        final String componentName = extras.getString("componentName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.hslGet(componentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;
                    case CTL_SET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("lightness")) {
                            Log.d(TAG, "lightness is not present");
                            return;
                        }
                        if (!extras.containsKey("temperature")) {
                            Log.d(TAG, "temperature is not present");
                            return;
                        }

                        if (!extras.containsKey("deltaUv")) {
                            Log.d(TAG, "deltaUv is not present");
                            return;
                        }

                        if (!extras.containsKey("reliable")) {
                            Log.d(TAG, "reliable is not present");
                            return;
                        }
                        if (!extras.containsKey("transitionTime")) {
                            Log.d(TAG, "transitionTime is not present");
                            return;
                        }
                        if (!extras.containsKey("delay")) {
                            Log.d(TAG, "delay is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final int lightness = extras.getInt("lightness");
                        final short temperature = (short) extras.getInt("temperature");
                        final short deltaUv = (short) extras.getInt("deltaUv");
                        final Boolean reliable = extras.getBoolean("reliable");
                        final short delay = extras.getShort("delay");
                        final int transitionTime = extras.getInt("transitionTime");
                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.ctlSet(componentName, lightness, temperature, deltaUv, reliable, transitionTime, delay);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }

                                    pendingResult = null;
                                    return null;
                                }
                                if(!reliable)
                                {
                                    setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case CTL_GET: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                        }
                        pendingResult = goAsync();
                        final String componentName = extras.getString("componentName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.ctlGet(componentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    if(pendingResult != null) {
                                        pendingResult.setResultCode(res);
                                        pendingResult.finish();
                                    }
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;
                    case CONNECT_COMPONENT: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "ComponentName is not present");
                            return;
                        }
                        if (!extras.containsKey("useGATTProxy")) {
                            Log.d(TAG, "useGATTProxy is not present");
                            return;
                        }
                        if (!extras.containsKey("scanDuration")) {
                            Log.d(TAG, "scanDuration is not present");
                            return;
                        }

                        final String componentName = extras.getString("componentName");
                        final byte useGATTProxy = (byte) extras.getInt("useGATTProxy");
                        final byte scanDuration = (byte) extras.getInt("scanDuration");

                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.connectComponent(componentName, scanDuration);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    pendingResult.setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;

                    case CONNECT_NETWORK: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("transport")) {
                            Log.d(TAG, "transport is not present");
                            return;
                        }
                        if (!extras.containsKey("scanDuration")) {
                            Log.d(TAG, "scanDuration is not present");
                            return;
                        }

                        final int transport = extras.getInt("transport");
                        final int scanDuration = extras.getInt("scanDuration");

                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                byte res = mMeshController.connectNetwork((byte)scanDuration);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    pendingResult.setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;

                    case DELETE_GROUP: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("groupName")) {
                            Log.d(TAG, "groupName is not present");
                            return;
                        }

                        final String groupName = extras.getString("groupName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                int res = mMeshController.deleteGroup(groupName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    pendingResult.setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case DELETE_NETWORK: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("provisionerName")) {
                            Log.d(TAG, "provisionerName is not present");
                            return;
                        }

                        if (!extras.containsKey("meshName")) {
                            Log.d(TAG, "meshName is not present");
                            return;
                        }

                        final String provisionerName = extras.getString("provisionerName");
                        final String meshName = extras.getString("meshName");

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                int res = mMeshController.deleteNetwork(provisionerName, meshName);
                                setResultCode(res);
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;
                    case DISCONNECT_NETWORK: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("transport")) {
                            Log.d(TAG, "transport is not present");
                            return;
                        }

                        final byte transport = (byte) extras.getInt("transport");

                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                byte res = mMeshController.disconnectNetwork();
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    setResultCode(res);
                                    pendingResult.finish();
                                    pendingResult = null;
                                }
                                return null;
                            }
                        };
                        ask.execute();

                    }
                    break;

                    case GET_CURRENT_NETWORK: {
                        String res = mMeshController.getCurrentNetwork();
                        setResultData(res);
                    }
                    break;

                    case INITIALIZE: {
                        mMeshController = mMeshController.initialize(getApplicationContext(), mCallback);
                        setResultCode(MeshController.MESH_CLIENT_SUCCESS);
                    }
                    break;

                    case IS_CONNECTED_TO_NETWORK: {
                        boolean res = mMeshController.isConnectedToNetwork();
                        Log.d(TAG, "isConnectedtoNetwork : "+res);
                        setResultCode(res?1:0);
                    }
                    break;

                    case IS_NETWORK_EXIST: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("meshName")) {
                            Log.d(TAG, "meshName is not present");
                            return;
                        }

                        final String meshName = extras.getString("meshName");
                        int res = mMeshController.isNetworkExist(meshName);
                        setResultCode(res);
                    }
                    break;
                    case MOVE_COMPONENT_TO_GROUP: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                            return;
                        }
                        if (!extras.containsKey("fromGroupName")) {
                            Log.d(TAG, "fromGroupName is not present");
                            return;
                        }
                        if (!extras.containsKey("toGroupName")) {
                            Log.d(TAG, "toGroupName is not present");
                            return;
                        }
                        final String componentName = extras.getString("componentName");
                        final String fromGroupName = extras.getString("fromGroupName");
                        final String toGroupName = extras.getString("toGroupName");
                        int res = mMeshController.moveComponentToGroup(componentName, fromGroupName, toGroupName);
                        setResultCode(res);
                    }
                    break;
                    case ADD_COMPONENT_TO_GROUP: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                            return;
                        }
                        if (!extras.containsKey("groupName")) {
                            Log.d(TAG, "groupName is not present");
                            return;
                        }
                        final String componentName = extras.getString("componentName");
                        final String groupName = extras.getString("groupName");
                        int res = mMeshController.addComponentToGroup(componentName, groupName);
                        setResultCode(res);
                    }
                    break;
                    case REMOVE_COMPONENT_FROM_GROUP: {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                            return;
                        }
                        if (!extras.containsKey("groupName")) {
                            Log.d(TAG, "groupName is not present");
                            return;
                        }
                        final String componentName = extras.getString("componentName");
                        final String groupName = extras.getString("groupName");
                        int res = mMeshController.removeComponentFromGroup(componentName, groupName);
                        setResultCode(res);
                    }
                    break;
                    case START_TRACKING: {
                        mMeshController.startTracking();
                        setResultCode(MeshController.MESH_CLIENT_SUCCESS);
                    }
                    break;

                    case STOP_TRACKING: {
                        mMeshController.stopTracking();
                        setResultCode(MeshController.MESH_CLIENT_SUCCESS);
                    }
                    break;

                    case UN_INITIALIZE: {
                        mMeshController.unInitialize();
                        setResultCode(MeshController.MESH_CLIENT_SUCCESS);
                    }
                    break;
                    case SET_PUBLICATION_CONFIG: {

                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("publishPeriod")) {
                            Log.d(TAG, "publishPeriod is not present");
                            return;
                        }
                        if (!extras.containsKey("publishCredentialFlag")) {
                            Log.d(TAG, "publishCredentialFlag is not present");
                            return;
                        }
                        if (!extras.containsKey("publishRetransmitCount")) {
                            Log.d(TAG, "publishRetransmitCount is not present");
                            return;
                        }
                        if (!extras.containsKey("publishRetransmitInterval")) {
                            Log.d(TAG, "publishRetransmitInterval is not present");
                            return;
                        }
                        if (!extras.containsKey("publishTtl")) {
                            Log.d(TAG, "publishTtl is not present");
                            return;
                        }

                        String deviceName = extras.getString("deviceName");
                        int deviceType = extras.getInt("deviceType");
                        int publishCredentialFlag = extras.getInt("publishCredentialFlag");
                        int publishRetransmitCount = extras.getInt("publishRetransmitCount");
                        int publishRetransmitInterval = extras.getInt("publishRetransmitInterval");
                        int publishTtl = extras.getInt("publishTtl");

                        int res = mMeshController.setPublicationConfig(publishCredentialFlag, publishRetransmitCount, publishRetransmitInterval, publishTtl);
                        setResultCode(res);
                    }
                    break;
                    case SET_DEVICE_CONFIG: {

                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("deviceName")) {
                            Log.d(TAG, "deviceName is not present");
                            return;
                        }
                        if (!extras.containsKey("isGattProxy")) {
                            Log.d(TAG, "isgattProxy is not present");
                            return;
                        }
                        if (!extras.containsKey("isFriend")) {
                            Log.d(TAG, "isFriend is not present");
                            return;
                        }
                        if (!extras.containsKey("isRelay")) {
                            Log.d(TAG, "isRelay is not present");
                            return;
                        }
                        if (!extras.containsKey("relayXmitCount")) {
                            Log.d(TAG, "relayXmitCount is not present");
                            return;
                        }
                        if (!extras.containsKey("relayXmitInterval")) {
                            Log.d(TAG, "relayXmitInterval is not present");
                            return;
                        }
                        if (!extras.containsKey("defaultTtl")) {
                            Log.d(TAG, "defaultTtl is not present");
                            return;
                        }
                        if (!extras.containsKey("netXmitCount")) {
                            Log.d(TAG, "netXmitCount is not present");
                            return;
                        }
                        if (!extras.containsKey("netXmitInterval")) {
                            Log.d(TAG, "v is not present");
                            return;
                        }

                        String deviceName = extras.getString("deviceName");
                        Boolean isGattProxy = extras.getBoolean("isGattProxy");
                        Boolean isFriend = extras.getBoolean("isFriend");
                        Boolean isRelay = extras.getBoolean("isRelay");
                        Boolean sendNetBeacon = extras.getBoolean("sendNetBeacon");
                        int relayXmitCount = extras.getInt("relayXmitCount");
                        int relayXmitInterval = extras.getInt("relayXmitInterval");
                        int defaultTtl = extras.getInt("defaultTtl");
                        int netXmitCount = extras.getInt("netXmitCount");
                        int netXmitInterval = extras.getInt("netXmitInterval");

                        int res = mMeshController.setDeviceConfig(deviceName, isGattProxy, isFriend, isRelay, sendNetBeacon, relayXmitCount, relayXmitInterval, defaultTtl, netXmitCount, netXmitInterval);
                        setResultCode(res);
                    }
                    break;
                    case GET_COMPONENT_INFO : {
                        Bundle extras = intent.getExtras();

                        if (!extras.containsKey("componentName")) {
                            Log.d(TAG, "componentName is not present");
                            return;
                        }
                        final String componentName = extras.getString("componentName");

                        int res = mMeshController.getComponentInfo(componentName);
                        setResultCode(res);

                        pendingResult = goAsync();

                        AsyncTask ask = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {

                                int res = mMeshController.getComponentInfo(componentName);
                                if (res != MeshController.MESH_CLIENT_SUCCESS) {
                                    pendingResult.setResultCode(res);
                                    if (pendingResult != null)
                                        pendingResult.finish();
                                    pendingResult = null;
                                    return null;
                                }
                                return null;
                            }
                        };
                        ask.execute();
                    }
                    break;

             /*  case SEND_RCVD_PROXY_PKT : {
                    Bundle extras = intent.getExtras();

                    if(!extras.containsKey("data")) {
                        Log.d(TAG,"data is not present");
                        return;
                    }

                    final byte[] data = extras.getString("groupName");
                    int res = mMeshController.sendReceivedProxyPacket(data);
                    Bundle bundle = new Bundle();
                    bundle.putInt("DATA",res);
                    setResult(Activity.RESULT_OK,"moveComponentToGroupResult", bundle);
                } break;
                */
                }
            }
        };
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        //start and bind to meshService

     //   mMeshController = MeshController.initialize(getApplicationContext(),mCallback);
        IntentFilter filter = new IntentFilter();
        filter.addAction(CREATE_NETWORK);
        filter.addAction(OPEN_NETWORK);
        filter.addAction(CLOSE_NETWORK);
        filter.addAction(CREATE_GROUP);
        filter.addAction(RENAME);
        filter.addAction(RESET_DEVICE);
        filter.addAction(GET_CONTROL_METHODS);
        filter.addAction(GET_TARGET_METHODS);
        filter.addAction(GET_ALL_GROUPS);
        filter.addAction(GET_ALL_NETWORKS);
        filter.addAction(GET_ALL_PROVISIONERS);
      //  filter.addAction(IDENTIFY);
        filter.addAction(GET_DEVICE_COMPONENTS);
        filter.addAction(GET_GROUP_COMPONENTS);
        filter.addAction(GET_COMPONENT_TYPE);
        filter.addAction(IMPORT_NETWORK);
        filter.addAction(EXPORT_NETWORK);
        filter.addAction(START_OTA_UPGRADE);
        filter.addAction(STOP_OTA_UPGRADE);
        filter.addAction(CONNECT_COMPONENT);
        filter.addAction(START_SCAN_MESH_DEVICES);
        filter.addAction(STOP_SCAN_MESH_DEVICES);
        filter.addAction(PROVISION);
        filter.addAction(ONOFF_GET);
        filter.addAction(ONOFF_SET);
        filter.addAction(LEVEL_GET);
        filter.addAction(LEVEL_SET);
        filter.addAction(LIGHTNESS_GET);
        filter.addAction(LIGHTNESS_SET);
        filter.addAction(HSL_GET);
        filter.addAction(HSL_SET);
        filter.addAction(CTL_GET);
        filter.addAction(CTL_SET);
        filter.addAction(CONNECT_NETWORK);
        filter.addAction(DELETE_GROUP);
        filter.addAction(DELETE_NETWORK);
        filter.addAction(DISCONNECT_NETWORK);
        filter.addAction(GET_CURRENT_NETWORK);
        filter.addAction(INITIALIZE);
        filter.addAction(IS_CONNECTED_TO_NETWORK);
        filter.addAction(IS_NETWORK_EXIST);
        filter.addAction(MOVE_COMPONENT_TO_GROUP);
        filter.addAction(REMOVE_COMPONENT_FROM_GROUP);
        filter.addAction(ADD_COMPONENT_TO_GROUP);
     //   filter.addAction(SEND_RCVD_PROXY_PKT);
        filter.addAction(START_TRACKING);
        filter.addAction(STOP_TRACKING);
        filter.addAction(UN_INITIALIZE);
        filter.addAction(SET_DEVICE_CONFIG);
        filter.addAction(SET_PUBLICATION_CONFIG);
        filter.addAction(GET_COMPONENT_INFO);
        registerReceiver(mReceiver, filter);

    }

    IMeshControllerCallback mCallback = new IMeshControllerCallback() {
        @Override
        public void onProvisionComplete(UUID device, byte status) {
            Log.d(TAG,"onProvisionComplete status:"+status);
            if(pendingResult != null && (status == MeshController.MESH_CLIENT_PROVISION_STATUS_SUCCESS || status == MeshController.MESH_CLIENT_PROVISION_STATUS_FAILED)) {
                pendingResult.setResultData("device:"+device+", status:"+status);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onOnOffStateChanged(String name, byte targetOnOff, byte presentOnOff, int remainingTime) {
            if(pendingResult != null) {
                String result = "name:"+name+", onoff:"+ targetOnOff;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onLevelStateChanged(String name, short targetLevel, short presentLevel, int remainingTime) {
            if(pendingResult != null) {
                String result = "name:"+name+", level:"+ targetLevel;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onHslStateChanged(String name, int lightness, int hue, int saturation, int remainingTime) {
            if(pendingResult != null) {
                String result = "name:"+name+", lightness:"+lightness+", hue:"+hue+", saturation:"+saturation+", remaining time:"+remainingTime;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onMeshServiceStatusChanged(int status) {
            Log.d(TAG,"onMeshServiceStatusChanged status:"+((status == 0)?"DISCONNECTED":"CONNECTED"));
            if(pendingResult != null) {
                pendingResult.setResultCode(status);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onDeviceFound(UUID uuid, String name) {
            Log.d(TAG,"onDeviceFound device:"+name+ " UUID:"+uuid +"mUUID"+mUuid);
            if(uuid.toString().equals(mUuid)){

                Log.d(TAG,"Found device !!!");
                MeshBluetoothDevice device = new MeshBluetoothDevice(uuid, name);

                if(!mDevices.contains(device))
                    mDevices.add(new MeshBluetoothDevice(uuid, name));
                if(pendingResult != null) {
                    pendingResult.setResultData("uuid :"+uuid.toString());
                    pendingResult.finish();
                    pendingResult = null;
                }
                mHandler.removeMessages(MSG_SCAN_TIMEOUT);
            }
//            if(mDevices!=null) {
//                for (int i = 0; i < mDevices.size(); i++) {
//                    if (mDevices.get(i).mUUID.equals(uuid)) {
//                        return;
//                    }
//                }
//                mDevices.add(new MeshBluetoothDevice(uuid, name));
//            }

        }

        @Override
        public void onNetworkConnectionStatusChanged(byte transport, byte status) {
            Log.d(TAG,"onNetworkConnectionStatusChanged status:"+status);
            if(pendingResult != null && !isProvisioning) {
                String result = "transport:"+transport+", status:"+status;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {

            if(pendingResult != null) {
                String result = "presentLightness:"+presentLightness+", presentTemperature:"+presentTemperature+
                        ", targetLightness:"+targetLightness+", targetTemperature:"+targetTemperature+", remainingTime:"+remainingTime+", deviceName:"+deviceName;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime) {

            if(pendingResult != null) {
                String result = "target:"+target+", present:"+present+
                        ", remainingTime:"+remainingTime+", deviceName:"+deviceName;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onNodeConnectionStateChanged(byte status, String componentName) {
            Log.d(TAG,"onNodeConnectionStateChanged status:"+status);

            if(pendingResult != null) {
                String result = "status:"+status+", componentName:"+componentName;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onOtaStatus(byte status, int percentComplete) {
            Log.d(TAG,"onOtaStatus status:"+status);

            if(pendingResult != null) {
                String result = "status:"+status+", percentComplete:"+percentComplete;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onReceivedProxyPktFromCore(byte[] data, int length) {
            Log.d(TAG,"onReceivedProxyPktFromCore length:"+length);

            if(pendingResult != null) {
                String result = "data:"+ Arrays.toString(data)+", length:"+length;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onNetworkOpenedCallback(byte status) {
            Log.d(TAG,"onNetworkOpenedCallback status:"+status + "current network:"+mCurrentNtw);

            if(pendingResult != null) {
                String result = "status:"+status+" network:"+mCurrentNtw;
                pendingResult.setResultData(result);
                pendingResult.finish();
                pendingResult = null;
                isImportInProgress = false;
            } else {
                Log.d(TAG,"onNetworkOpenedCallback pending result is null");
            }
        }

        @Override
        public void onDatabaseChanged(String meshName) {
            Log.d(TAG,"onDatabaseChanged :"+meshName);

            if(pendingResult != null && !isImportInProgress) {
                Log.d(TAG,"onDatabaseChanged sending result");
                pendingResult.setResultData(meshName);
                pendingResult.finish();
                pendingResult = null;
            } else{
                Log.d(TAG,"onDatabaseChanged result not sent");
            }
        }

        @Override
        public void onComponentInfoStatus(byte status, String componentName, String componentInfo) {
            Log.d(TAG,"onComponentInfoStatus :"+status);

            if(pendingResult != null) {
                pendingResult.setResultData("status:"+status+" componentName:"+componentName+" componentInfo:"+componentInfo);
                pendingResult.finish();
                pendingResult = null;
            }
        }

        @Override
        public void onDfuStatus(byte status, byte[] data) {

        }

        @Override
        public void onSensorStatusCb(String componentName, int propertyId, byte[] data) {

        }

        @Override
        public void onVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen) {

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
    };

    String returnIntTypeToString(int ret) {
        switch(ret) {
            case MeshController.MESH_CLIENT_SUCCESS : return "MESH_CLIENT_SUCCESS";
            case MeshController.MESH_CLIENT_ERR_NOT_FOUND : return "MESH_CLIENT_ERR_DEVICE_NOT_FOUND";
            case MeshController.MESH_CLIENT_PROVISION_STATUS_SUCCESS : return "MESH_CLIENT_PROVISION_STATUS_SUCCESS";
        }
        return "RETURN_TYPE_UNKNOWN";
    }

    Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg){
            if(msg.what == MSG_SCAN_TIMEOUT) {
                if (pendingResult != null) {
                    ArrayList<String> uuids = new ArrayList<String>();
                    for (int i = 0; i < mDevices.size(); i++) {
                        Log.d(TAG,"UUID :"+mDevices.get(i).mUUID.toString());
                        uuids.add(mDevices.get(i).mUUID.toString());
                    }

                    String components = TextUtils.join(",", uuids);
                    pendingResult.setResultData(components);
                    pendingResult.finish();
                    pendingResult = null;
                }
            }
        }
    };
}
