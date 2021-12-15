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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

/**
 * This class provides functionality to communicate and manage BLE mesh devices .
 * {@link MeshController} allows you to create a BLE mesh network, add device, configure and control them.
 * <p>Fundamentally, this is the starting point for all mesh applications</p>
 */

public class MeshController {

    private static final String TAG = "MeshController";

    /**
     * Indicates the component is unknown
     */
    public static final int COMPONENT_TYPE_UNKNOWN                = 0;

    /**
     * Indicates the component is an on-off client
     */
    public static final int COMPONENT_TYPE_GENERIC_ON_OFF_CLIENT  = 1;

    /**
     * Indicates the component is a level client
     */
    public static final int COMPONENT_TYPE_GENERIC_LEVEL_CLIENT   = 2;

    /**
     * Indicates the component is an on-off server
     */
    public static final int COMPONENT_TYPE_GENERIC_ON_OFF_SERVER  = 3;

    /**
     * Indicates the component is a level server
     */
    public static final int COMPONENT_TYPE_GENERIC_LEVEL_SERVER   = 4;

    /**
     * Indicates the component is a power onoff server
     */
    public static final int COMPONENT_TYPE_POWER_ONOFF_SERVER     = 5;

    /**
     * Indicates the component is a power onoff server
     */
    public static final int COMPONENT_TYPE_POWER_LEVEL_SERVER     = 6;

    /**
     * Indicates the component is of type dimmable light
     */
    public static final int COMPONENT_TYPE_LIGHT_DIMMABLE         = 7;

    /**
     * Indicates the component is a HSL server
     */
    public static final int COMPONENT_TYPE_LIGHT_HSL              = 8;

    /**
     * Indicates the component is a CTL light
     */
    public static final int COMPONENT_TYPE_LIGHT_CTL              = 9;

    /**
     * Indicates the component is of type light XYL
     */
    public static final int COMPONENT_TYPE_LIGHT_XYL              = 10;

    /**
     * Indicates the component is sensor
     */
    public static final int COMPONENT_TYPE_SENSOR                 = 11;

    /**
     * Indicates the component is sensor client
     */
    public static final int COMPONENT_TYPE_SENSOR_CLIENT          = 12;
    /**
     * Indicates the component is vendor specific
     */
    public static final int COMPONENT_TYPE_VENDOR_SPECIFIC        = 13;

    /**
     * Indicates that the transport is of type GATT
     */
    public static final byte TRANSPORT_GATT = 1;
    /**
     * Indicates that the transport is Cloud
     */
    public static final int TRANSPORT_IP = 2;

    /**
     * Indicates that the mesh service is connected
     */
    public static final int MESH_SERVICE_CONNECTED = 0x01;

    /**
     * Indicates that the mesh service is disconnected
     */
    public static final int MESH_SERVICE_DISCONNECTED = 0x00;

    /**
     * Indicated that the transition time is set to default value
     */
    public static final int MESH_TRANSITION_TIME_DEFAULT  = 0xFFFFFFFF;

    /**
     * Indicates that the provision status is failed
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_FAILED         =0;

    /**
     * Indicates that the provision status is failed
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_SUCCESS        =5;

    /**
     * Indicates that the operation was success
     */
    public static final byte MESH_CLIENT_SUCCESS                 =0;

    /**
     * Indicates that the state is invalid
     */
    public static final byte MESH_CLIENT_ERR_INVALID_STATE       =1;

    /**
     * Indicates that the network is not connected
     */
    public static final byte MESH_CLIENT_ERR_NOT_CONNECTED       =2;

    /**
     * Indicates that the device is not found
     */
    public static final byte MESH_CLIENT_ERR_NOT_FOUND           =3;

    /**
     * Indicates that the network is closed
     */
    public static final byte MESH_CLIENT_ERR_NETWORK_CLOSED      =4;

    /**
     * Indicates that core is out of memory
     */
    public static final byte MESH_CLIENT_ERR_NO_MEMORY           =5;

    /**
     * Indicates that the method is not available
     */
    public static final byte MESH_CLIENT_ERR_METHOD_NOT_AVAIL    =6;

    /**
     * Indicates Network Database error
     */
    public static final byte MESH_CLIENT_ERR_NETWORK_DB          =7;

    /**
     * Indicates invalid arguments
     */
    public static final byte MESH_CLIENT_ERR_INVALID_ARGS        =8;

    /**
     * Indicates that the name is already present
     */
    public static final byte MESH_CLIENT_ERR_DUPLICATE_NAME      =9;

    /**
     * Indicates that the previous procedure is not complete
     */
    public static final byte MESH_CLIENT_ERR_PROCEDURE_NOT_COMPLETE      =10;

    /**
     * Indicates the address to be assigned to broadcast message to all proxies in the network
     */
    public static final int MESH_CLIENT_ADDRESS_ALL_PROXIES        = 0xFFFC;

    /**
     * Indicates the address to be assigned to broadcast message to all friends in the network
     */
    public static final int MESH_CLIENT_ADDRESS_ALL_FRIENDS        = 0xFFFD;

    /**
     * Indicates the address to be assigned to broadcast message to all relays in the network
     */
    public static final int MESH_CLIENT_ADDRESS_ALL_RELAYS        = 0xFFFE;

    /**
     * Indicates the address to be assigned to broadcast message to all nodes in the network
     */
    public static final int MESH_CLIENT_ADDRESS_ALL_NODES        = 0xFFFF;

    /**
     * Indicates that the sensor cadence values has to be sent in native format length
     */
    public static final boolean MESH_CLIENT_TRIGGER_TYPE_NATIVE        = false;

    /**
     * Indicates that the sensor cadence values has to be sent in percentage format
     */
    public static final boolean MESH_CLIENT_TRIGGER_TYPE_PERCENTAGE    = true;

    /**
     * Indicates that the LC mode is enabled
     */
    public static final int MESH_CLIENT_LC_MODE_ON                  = 1;

    /**
     * Indicates that the LC mode is disabled
     */
    public static final int MESH_CLIENT_LC_MODE_OFF                  = 0;

    /**
     * Indicates that the LC Occupancy mode is enabled
     */
    public static final int MESH_CLIENT_LC_OCCUPANCY_MODE_ON         = 1;

    /**
     * Indicates that the LC Occupancy mode is disabled
     */
    public static final int MESH_CLIENT_LC_OCCUPANCY_MODE_OFF         = 0;

    private static final int COMMAND_FAILURE = 0;
    private static final int COMMAND_SUCCESS = 1;

    /**
     * Indicates the mesh service is not connected successfully.
     */
    public static final int MESH_CLIENT_SERVICE_NOT_CONNECTED = 20;
    /**
     * Indicates that the component type is unknown
     */
    public static final int DEVICE_TYPE_UNKNOWN                = 0;
    /**
     * Indicates that the component type is generic onoff client
     */
    public static final int DEVICE_TYPE_GENERIC_ON_OFF_CLIENT  = 1;
    /**
     * Indicates that the component type is generic level client
     */
    public static final int DEVICE_TYPE_GENERIC_LEVEL_CLIENT   = 2;
    /**
     * Indicates that the component type is generic onoff server
     */
    public static final int DEVICE_TYPE_GENERIC_ON_OFF_SERVER  = 3;
    /**
     * Indicates that the component type is generic level server
     */
    public static final int DEVICE_TYPE_GENERIC_LEVEL_SERVER   = 4;
    /**
     * Indicates that the component type is dimmable light
     */
    public static final int DEVICE_TYPE_LIGHT_DIMMABLE         = 5;
    /**
     * Indicates that the component type is power outlet
     */
    public static final int DEVICE_TYPE_POWER_OUTLET           = 6;
    /**
     * Indicates that the component type is light hsl
     */
    public static final int DEVICE_TYPE_LIGHT_HSL              = 7;
    /**
     * Indicates that the component type is light ctl
     */
    public static final int DEVICE_TYPE_LIGHT_CTL              = 8;
    /**
     * Indicates that the component type is light xyl
     */
    public static final int DEVICE_TYPE_LIGHT_XYL              = 9;
    /**
     * Indicates that the component type is vendor specific
     */
    public static final int DEVICE_TYPE_VENDOR_SPECIFIC        = 10;

    /**
     * Indicates that the control method is of type onoff
     */
    public static final String MESH_CONTROL_METHOD_ONOFF      = "ONOFF";
    /**
     * Indicates that the control method is of type level
     */
    public static final String MESH_CONTROL_METHOD_LEVEL      = "LEVEL";
    /**
     * Indicates that the control method is of type lightness
     */
    public static final String MESH_CONTROL_METHOD_LIGHTNESS  = "LIGHTNESS";
    /**
     * Indicates that the control method is of type power
     */
    public static final String MESH_CONTROL_METHOD_POWER      = "POWER";
    /*
     * Indicates that the control method is of type HSL
     */
    public static final String MESH_CONTROL_METHOD_HSL        = "HSL";
    /**
     * Indicates that the control method is of type CTL
     */
    public static final String MESH_CONTROL_METHOD_CTL        = "CTL";
    /**
     * Indicates that the control method is of type XYL
     */
    public static final String MESH_CONTROL_METHOD_XYL        = "XYL";
    /**
     * Indicates that the control method is of type SENSOR
     */
    public static final String MESH_CONTROL_METHOD_SENSOR     = "SENSOR";
    /**
     * Indicates that the control method is of type vendor
     */
    public static final String MESH_CONTROL_METHOD_VENDOR     = "VENDOR_";

    private static MeshService service = null;
    private static IMeshControllerCallback mMeshControllerCb = null;
    private static Context mCtx = null;
    private static MeshController myInstance = new MeshController();
    private static String mCurrNetwork = null;

    private MeshController() {}

    private static ServiceConnection svcConn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            MeshService.LocalBinder binderSrv = (MeshService.LocalBinder) binder;
            service = binderSrv.getService();
            service.register(mMeshControllerCb);

            Log.i(TAG, "onServiceConnected");
            mMeshControllerCb.onMeshServiceStatusChanged(MESH_SERVICE_CONNECTED);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            service = null;
            Log.i(TAG, "onServiceDisconnected");
            mMeshControllerCb.onMeshServiceStatusChanged(MESH_SERVICE_DISCONNECTED);
        }
    };

    protected static MeshService getService() {
        return service;
    }

    /**
     * Provides an instance of MeshController
     *
     * @param context Context of the application
     * @param cb IMeshControllerCallback instance to be registered
     * @return MeshController instance
     */
    public static MeshController initialize(Context context, IMeshControllerCallback cb) {
        Log.d(TAG,"initialize :  release version 6.2.4.17");
        mMeshControllerCb = cb;
        mCtx = context;
        Intent intent = new Intent(mCtx, MeshService.class);
        Log.i(TAG, "bindingService");
        boolean res = mCtx.bindService(intent, svcConn, Context.BIND_AUTO_CREATE);
        return myInstance;
    }

    /**
     * Returns the name of the currently opened mesh network
     * @return Name of the currently opened network, null if the network is not opened.
     */
    public String getCurrentNetwork() {
        return mCurrNetwork;
    }

    /**
     * Checks if the network with specified name exists. The function checks the local storage
     * for the database file. It does not attempt to establish connection to the network.
     * @param meshName Name of the network to be checked
     * @return 1 on success, or 0 otherwise.
     * {@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.
     */
    public int isNetworkExist(String meshName) {
        Log.d(TAG,"isNetworkExist");
        if (isServiceConnected()) {
            return service.isNetworkExist(meshName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Create a mesh network.
     *
     * The createNetwork method allows application to create a new network database.
     * After the network is created, application can open, or export the network passing
     * the same mesh network name as a parameter.
     *
     * @param provisionerName User friendly provisioner name. This name uniquely identifies the provisioner. This field cannot be empty or null.
     * @param meshName User friendly mesh network name. This name uniquely identifies the mesh network. This field cannot be empty or null.
     * @return the return values can be <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} on success,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int createNetwork(String provisionerName, String meshName) {
        Log.d(TAG,"createNetwork");
        if (isServiceConnected()) {
            return service.createNetwork(provisionerName, meshName);
        }
       return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Open Network.
     * <p>This function is called to open the network and initialize the mesh stack with
     * parameters stored in the mesh network database. If function returns
     * {@link #MESH_CLIENT_SUCCESS} the network database has been located and
     * the stack initialization has been started. When initialization is complete, the
     * {@link IMeshControllerCallback#onNetworkOpenedCallback(byte)} is executed.
     * The function shall use the same parameter values as were used during at the time when
     * the network was creating either by the {@link #createNetwork(String, String)} or the {@link
     * #importNetwork(String, String)} method. </p>
     * It is possible that while network is being opened, the mesh stack needs to make changes to the network database.  In that case, the {@link
     * IMeshControllerCallback#onDatabaseChanged(String meshName)} callback will be executed.
     * prior to using this api, application is expected to call{@link #initialize(Context context, IMeshControllerCallback cb) }

     * @param provisionerName Provisioner name. Shall be the same name as when network was
     * created or imported.
     * @param meshName Network to be opened. Shall be the same name as when network was
     * created or imported.
     * @return The return values can be <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if database for the network with specified name does not exist,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} if device is not found,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} if command is accepted and stack configuration has started.</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int openNetwork(String provisionerName, String meshName) {
        Log.d(TAG,"openNetwork");
        mCurrNetwork = meshName;

        if (isServiceConnected()) {
            return service.openNetwork(provisionerName, meshName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     *  Close the currently active network.
     *
     * @return The return values can be <ul>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} on success,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int closeNetwork() {
        Log.d(TAG,"closeNetwork");
        if (isServiceConnected()) {
            service.closeNetwork();
            return MESH_CLIENT_SUCCESS;
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Create a new group in a parent group.
     *
     * <p>Devices in a mesh network can be organized in groups. If a controlling device (for example, a light switch) is a part of the group, it sends on/off commands to the group rather than to an individual device. If a target device (for example, a bulb) is a part of a group, it processes the commands addressed to the group, as well as commands addressed to the target device itself. </p>
     * <p>If a group has a parent group, all commands sent to the parent group will be processed by members of the group. For example, if a nightstand is a part of a bedroom group which has a parent a second-floor group, the nightstand can be turned on/off by sending a command to the nightstand itself, to the bedroom group and to the second-floor group. </p>
     * <p>The group is empty when it is created. A new device (both of the control and of the target type) can be provisioned to be a part of the group. Devices and components can also be moved in and out of a group. </p>
     * <p>A group name shall be unique name in the mesh network. The group name shall not match the name of any of the existing mesh components. </p>
     * <p>If function succeeds the new group is created in the database and {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed. </p>
     *
     * @param groupName Mesh Group name.
     * @param parentGroupName A parent group in which new group should be created. Use network name if group must be created in network.
     * @return The return values can be <ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_DUPLICATE_NAME} if the core is out of memory,</li>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} on success,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int createGroup(String groupName, String parentGroupName) {
        Log.d(TAG,"createGroup");
        if (isServiceConnected()) {
            return service.createGroup(groupName, parentGroupName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Delete a mesh group.
     *
     * <p>If the group to be deleted is empty, the mesh database is updated, and the operation completes immediately.
     * If there are devices in the group to be deleted, each device needs to be reconfigured not to be a part of the group.
     * This operation cannot be started if some other provisioning or configuration operation is in progress.
     * When delete operation is completed, the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)}
     * callback is executed.</p>
     *
     * @param groupName The name of the group to be deleted.
     * @return The return values can be <ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_DB} if there is error in mesh network database,</li>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} on success,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int deleteGroup(String groupName) {
        Log.d(TAG,"deleteGroup");
        if (isServiceConnected()) {
            return service.deleteGroup(groupName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Return a list of all existing mesh networks.
     *
     * @return String[] list of mesh networks, null otherwise
     */
    public String[] getAllNetworks() {
        Log.d(TAG,"getAllNetworks");
        if (isServiceConnected()) {
            return service.getAllNetworks();
        }
        return null;
    }

    /**
     * Return a list of all subgroups of a group.
     *
     * @param inGroup Group name whose subgroups has to be returned. Network name, to get all the parent groups of the network.
     * @return String[] list of mesh groups, null otherwise
     */
    public String[] getSubGroups(String inGroup) {
        Log.d(TAG,"getSubGroups");
        if (isServiceConnected()) {
            return service.getAllGroups(inGroup);
        }
        return null;
    }

    /**
     * Return all subgroups of a group down the tree.
     *
     * @param inGroup Group name whose subgroups has to be returned. Network name, to get all the groups of the network.
     * @return String[] list of mesh groups, null otherwise
     */
    public ArrayList<String> getAllGroups(String inGroup) {
        Log.d(TAG,"getAllGroups");
        ArrayList<String> groups = new ArrayList<String>(Arrays.asList(getSubGroups(inGroup)));
        ArrayList<String> temp = new ArrayList<String>();;
        for (String group : groups)
            temp.addAll(getAllGroups(group));
        groups.addAll(temp);
        return groups;
    }

    /**
     * Return a list of all provisioners of the currently opened mesh network.
     *
     * @return String[] list of provisioners, null otherwise
     */
    public String[] getAllProvisioners() {
        Log.d(TAG,"getAllProvisioners");
        if (isServiceConnected()) {
            return service.getAllProvisioners();
        }
        return null;
    }

    /**
     * Return a list of all components of the mesh device
     *
     * <p>A mesh device may consist of several components.
     * For example, there can be one mesh device servicing a fan with several dimmable lights.
     * Each component of the device is treated separately.
     * It can be controlled by different devices and can be assigned to the same or different group.</p>
     *
     * @param uuid UUID of the device whose components must be returned.
     * @return String[] list of components, null otherwise
     */
    public String[] getDeviceComponents(byte[] uuid) {
        Log.d(TAG,"getDeviceComponents");
        if (isServiceConnected()) {
            return service.getDeviceComponents(uuid);
        }
        return null;
    }

    /**
     * Return a list of all components of the group
     *
     * @param groupName Name of the group whose components has to be returned
     * @return String[] list of components, null otherwise
     */
    public String[] getGroupComponents(String groupName) {
        Log.d(TAG,"getGroupComponents");
        if (isServiceConnected()) {
            return service.getGroupComponents(groupName);
        }
        return null;
    }

     /**
     * Fetch the type of component.
     *
     * @param componentName Name of the component whose type has to be returned.
     * @return The return values can be<ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_DB} if there is error in mesh network database,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>returns component type on success . Return value can be one of<ul>
     * <li>{@link #DEVICE_TYPE_UNKNOWN}</li>
     * <li>{@link #DEVICE_TYPE_GENERIC_ON_OFF_CLIENT}</li>
     * <li>{@link #DEVICE_TYPE_GENERIC_LEVEL_CLIENT}</li>
     * <li>{@link #DEVICE_TYPE_GENERIC_ON_OFF_SERVER}</li>
     * <li>{@link #DEVICE_TYPE_GENERIC_LEVEL_SERVER}</li>
     * <li>{@link #DEVICE_TYPE_LIGHT_DIMMABLE}</li>
     * <li>{@link #DEVICE_TYPE_POWER_OUTLET}</li>
     * <li>{@link #DEVICE_TYPE_LIGHT_HSL}</li>
     * <li>{@link #DEVICE_TYPE_LIGHT_CTL}</li>
     * <li>{@link #DEVICE_TYPE_LIGHT_XYL}</li>
     * <li>{@link #DEVICE_TYPE_VENDOR_SPECIFIC}
     * </li></ul>
     </ul>
     */
    public int getComponentType(String componentName) {
        Log.d(TAG,"getComponentType");
        if (isServiceConnected()) {
            return service.getComponentType(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Check if the component is a light controller
     * @param componentName Name of the component to be checked
     * @return The return values can be<ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>1 if component is a light controller</li>
     * <li>0 if component is not a light controller</li>
     */
    public int isLightController(String componentName) {
        Log.d(TAG,"getComponentType");
        if (isServiceConnected()) {
            return service.isLightController(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Update the component's/group's name.
     *
     * <p>The rename method updates the mesh database. If parameters are accepted, the name is updated and the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed.
     * The new name shall be unique in the mesh network and shall not match the name of any of the existing mesh components or groups. </p>
     *
     * @param oldName Current name of the Component/Group.
     * @param newName New Component/Group name to be updated to.
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if there is error in mesh network database,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} on success.</ul>
     */
    public int rename(String oldName, String newName) {
        Log.d(TAG,"renameComponent");
        if (isServiceConnected()) {
            return service.rename(oldName, newName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Identify method may be used to instruct a device or a group
     * of devices to identify themselves for a certain duration.
     * <p>The identification method (blinking, vibrating, etc.) varies depending on the type of the target device and device manufacturer.
     * For the operation to succeed, the network shall be opened, and connection to the network shall be established.</p>
     *
     * @param name Name of the device or the group.
     * @param duration duration in milliseconds
     *
     * @return The return values can be<ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if there is error in mesh network database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} if the name does not match any device or a group in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} on success.</li></ul>
     */

    public int identify(String name, byte duration) {
        Log.d(TAG,"identify");
        if (isServiceConnected()) {
            return service.identify(name, duration);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Move component to group.
     *
     *<p> The move to group operation performs reconfiguration of the specified component
     * over the mesh network. The operation cannot be started if a provisioning or some other configuration is in progress.
     * When move to group operation is completed, the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed.
     * For the operation to succeed, the network shall be opened, and connection to the network shall be established.</p>
     *
     * @param componentName Name of the Component.
     * @param fromGroupName Name of the group from which the component must be moved from.
     * @param toGroupName Name of the group to which the component must be moved to.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} either the fromGroup or the toGroup is not a name of one of currently opened mesh network groups,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if in the component with specified name is not found in the mesh network database,</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE} another configuration operation is in progress,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the operation has been started.</li></ul>
     */

    public int moveComponentToGroup(String componentName, String fromGroupName, String toGroupName) {
        Log.d(TAG,"moveComponentToGroup");
        if (isServiceConnected()) {
            return service.moveComponentToGroup(componentName, fromGroupName, toGroupName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Get list of groups to which component belongs to.
     *
     * @param componentName Name of the Component.
     *
     * @return List of groups or NULL
     */
    public String[] getComponentGroupList(String componentName){
        Log.d(TAG,"getComponentGroupList");
        if (isServiceConnected()) {
            return service.getComponentGroupList(componentName);
        }
        return null;
    }

    /**
     * Remove component from group.
     *
     * <p>Removes the component with the specified name from the group.
     * The remove component operation performs reconfiguration of the component over the mesh network.
     * The operation cannot be started if a provisioning or some other configuration is in progress.
     * For the operation to succeed, the network shall be opened, and connection to the network shall be established.
     * When remove operation is completed, the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed.</p>
     *
     * @param componentName Name of the Component.
     * @param groupName Name of the group from which the component must be removed from.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} the groupName is not a name of one of the currently opened mesh network groups,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if in the component with specified name is not found in the mesh network database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the operation has been started.</li></ul>
     */

    public int removeComponentFromGroup(String componentName, String groupName) {
        Log.d(TAG,"removeComponentFromGroup");
        if (isServiceConnected()) {
            return service.removeComponentFromGroup(componentName, groupName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Add component to group.
     *
     * <p>Adds the component with the specified name to the existing group.
     * The add component operation performs reconfiguration of the component over the mesh network.
     * The operation cannot be started if a provisioning or some other configuration is in progress.
     * For the operation to succeed, the network shall be opened, and connection to the network shall be established.
     * When add operation is completed, the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed.</p>
     *
     * @param componentName Name of the Component.
     * @param groupName Name of the group to which the component must be added to
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} the groupName is not a name of one of the currently opened mesh network’s groups,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if there is error in db,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the operation has been started.</li></ul>
     */

    public int addComponentToGroup(String componentName, String groupName) {
        Log.d(TAG,"addComponentToGroup");
        if (isServiceConnected()) {
            return service.addComponentToGroup(componentName, groupName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Configure publication.
     *
     * <p>When a device is initially provisioned and configured to be a part of the mesh network the controlling components of the device are configured to send commands to the group the device is provisioned in.  The target components are not configured to send status changes when the state of the device is changed. In some cases, this needs to be changed. For example, even when a switch is initially provisioned to be a part of a group, the user might want to reconfigure the switch to control a specific light rather than all the lights in the group. In another example, when there is a dedicated lighting control panel in a house, the user might want every light in the house to report the status changes to the control panel. </p>
     * <p>The configure publication method configures the device to send commands or status change notifications to a specific device, group of devices, or one of the predefined types including "all-nodes", "all-proxies", "all_friends", or "all-relays". The predefined type "none" removes currently active publication. </p>
     * <p>The method should be one of the methods returned by the {@link #getTargetMethods(String)} or returned by the {@link #getControlMethods(String)} method.</p>
     * <p>Before calling the Configure Publication method, the application may want to change the publication parameters using the {@link #setPublicationConfig } method. The {@link #setPublicationConfig} method, the application may select if the device needs to send publications periodically or when the state of the device changes.</p>
     * The configure publication operation performs reconfiguration of the component over the mesh network. The operation cannot be started if a provisioning or some other configuration is in progress. For the operation to succeed, the network shall be opened, and connection to the network shall be established.
     * When configure publication operation is completed, the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback is executed.
     * @param componentName Name of the component to be configured
     * @param isClient is client method
     * @param method one of the methods returned by the {@link #getTargetMethods(String)} or returned by the {@link #getControlMethods(String)} method.
     * @param targetName Name of the group to be assigned.
     * @param publishPeriod interval at which status messages are published by the selected function of the device. If the value is zero, the device will not send periodic publication, but only when the state is changed.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} the componentName or the targetName is not valid in the opened mesh network,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if there is error in mesh network database,</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE} another configuration operation is in progress,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the operation has been started.</li></ul>
     */

    public int configurePublication(String componentName, boolean isClient, String method, String targetName, int publishPeriod) {
        Log.d(TAG,"configurePublication");
        if (isServiceConnected()) {
            return service.configurePublication(componentName, isClient, method, targetName, publishPeriod);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * The provision method can be used by an application to add a new device to the opened mesh network. Before the provisioning process can be started, the application shall perform the scan and select an unprovisioned device.
     *
     * <p>After the provisioning completes, the mesh stack performs initial configuration of all components of the new device. Provisioning and configuration process consist of several steps. The completion of each step is indicated to the application through the {@link IMeshControllerCallback#onProvisionComplete} callback so that additional feedback could be provided to the user. </p>
     *
     * <p>Note that at the end of the provisioning and configuration the application can stay connected to the network or may be disconnected.  This depends on the capabilities of the device and on preferred configuration. The application can monitor {@link IMeshControllerCallback#onNetworkConnectionStatusChanged} notifications, or the application can check the connection status    using {@link #isConnectedToNetwork} method. </p>
     *
     * When provisioning and configuration is complete, the application can use the {@link #getDeviceComponents} method to find all components of the device. For each component the application can use {@link #getControlMethods} and {@link #getTargetMethods} to find what the new device can do.
     *
     * @param deviceName Device name to be set after provisioning.
     * @param deviceUuid uuid of the unprovisioned device found upon calling scanMeshDevices.
     * @param groupName Name of the group that the device is configured into if provisioning completes successfully.
     * @param identifyDuration Identify duration while provisioning.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE} another provisioning or configuration operation is in progress,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the operation has been started.</li></ul>
     */

    public byte provision(String deviceName, String groupName, UUID deviceUuid, byte identifyDuration) {
        Log.d(TAG,"provision");
        if (isServiceConnected()) {
            return service.provision(deviceName, groupName, deviceUuid, identifyDuration);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Connect to currently opened network.
     *
     * <p>To perform any operation on a mesh device, for example, reconfiguration or control,
     * the application needs to connect to the network.
     * The connection is established over GATT and involves searching for a proxy device which belongs
     * to the opened network in the immediate radio range and establishing and setting up the BLE connection.
     * When operation completes, the {@link IMeshControllerCallback#onNetworkConnectionStatusChanged} method is executed.</p>
     *
     * @param scanDuration Scan timeout indicates how long the mesh stack should search for a proxy device before declaring failure.
     * @return <ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_STATE} mesh client is in invalid state,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} the connect operation has been started successfully.</li></ul>
     */


    public byte connectNetwork(byte scanDuration) {
        Log.d(TAG,"connectNetwork");
        if (isServiceConnected()) {
            return service.connectNetwork(scanDuration);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     *  Disconnect from the current network.
     *  The disconnect method can be used to disconnect from the currently opened network.  When operation completes, the {@link IMeshControllerCallback#onNetworkConnectionStatusChanged  } method is executed.
     *
     * @return<ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE} mesh client is in invalid state,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} the disconnect operation has been started successfully.</li></ul>
     */

    public byte disconnectNetwork() {
        Log.d(TAG,"disconnectNetwork");
        if (isServiceConnected()) {
            return service.disconnectNetwork();
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;

    }

    /**
     * Retrieve On/Off state of a component or all components in a group.
     *
     * <p>The method involves sending a message over the mesh network and processing
     * a reply from one or multiple devices.  When a reply from a component is received,
     * the {@link IMeshControllerCallback#onOnOffStateChanged} method is executed.
     * If the query is for the group, the callback is executed for each component of the group.</p>
     *
     * @param componentName Name of the component or a group
     * @return<ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} On/Off method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} A component or a group with the name componentName is not found in the database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int onoffGet(String componentName) {
        Log.d(TAG,"onoffGet");
        if (isServiceConnected()) {
            return service.onoffGet(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set On/Off state of a component or all components in a group
     *
     * <p>The method involves sending a message over the mesh network and processing
     * a reply from one or multiple devices.  When a reply from a component is received,
     * the {@link IMeshControllerCallback#onOnOffStateChanged} method is executed.
     * If the query is for the group, the callback is executed for each component of the group.</p>
     *
     * @param componentName Name of the component
     * @param onoff on/off state of the component to be set. true for on and false for off
     * @param ackRequired true if ack is needed, false otherwise.
     * @param transitionTime Time in milliseconds or {@link #MESH_TRANSITION_TIME_DEFAULT} if default value has to be used.
     * @param delay Time in milliseconds
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} On/Off method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND}, A component or a group with the name componentName is not found in the database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int onoffSet(String componentName, boolean onoff, boolean ackRequired, int transitionTime, short delay) {
        Log.d(TAG,"onoffSet");
        if (isServiceConnected()) {
            return service.onoffSet(componentName, onoff, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Get Level state of a component or all components in a group.
     *
     * <p>The method involves sending a message over the mesh network and processing a
     * reply from one or multiple devices. When a reply from a component is received,
     * the {@link IMeshControllerCallback#onLevelStateChanged} method is executed.
     * If the query is for the group, the callback is executed for each component of the group.</p>
     *
     * @param componentName Name of the component or the group
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} On/Off method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND}, A component or a group with the name componentName is not found in the database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int levelGet(String componentName) {
        Log.d(TAG,"levelGet");
        if (isServiceConnected()) {
            return service.levelGet(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set Level state of a component or all components in a group.
     *
     * <p>The method involves sending a message over the mesh network and processing a reply from one or multiple devices.
     * When a reply from a component is received, the {@link IMeshControllerCallback#onLevelStateChanged} method is executed.
     * If the query is for the group, the callback is executed for each component of the group.</p>
     *
     * @param componentName Name of the component
     * @param level The target value of the Level state (value range : -32768 to +32767)
     * @param ackRequired true if ack is needed, false otherwise.
     * @param transitionTime Transition time to the target level in milliseconds or {@link #MESH_TRANSITION_TIME_DEFAULT} if default value has to be used.
     * @param delay Delay before starting the transition in milliseconds
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} Level method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND}, A component or a group with the name componentName is not found in the database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int levelSet(String componentName, short level, boolean ackRequired, int transitionTime, short delay) {
        Log.d(TAG,"LevelSet "+level);
        if (isServiceConnected()) {
            return service.levelSet(componentName, level, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Get HSL state of a component or all components in a group.
     *
     * This method can be used by the application to get current values for hue, saturation and lightness of a light or a group of lights.
     * If function call returns success, the request has been successfully sent out.
     * When a reply from a component is received, the {@link IMeshControllerCallback#onHslStateChanged} method is executed.
     * If the query is for the group, the callback is executed for each component of the group.
     * @param componentName Name of the component or the group
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} HSL method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} A component or a group with specified name is not found in the database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */
    public int hslGet(String componentName) {
        Log.d(TAG,"hslGet");
        if (isServiceConnected()) {
            return service.hslGet(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set HSL state of a component or all components in a group.
     *
     * <p>This method can be used by the application to set hue, saturation and brightness level of a light or a group of lights.  If function call returns success, the request has been successfully sent out. If ackRequired parameter is set to true, the {@link IMeshControllerCallback#onHslStateChanged} method will be executed at the time when the reply from the component is received. If the request is for the group, the callback is executed for each component of the group that sends the status.
     * The transition time parameter can be used to transition to the target CTL state over specified period.
     * The delay parameter can be used to tell the target device to start transitioning to the target CTL state after a delay.
     * It is not recommended to send a request with acknowledgement to a group of more than several lights.</p>
     *
     * @param componentName Name of the component or the group
     * @param lightness New value of the lightness. Valid range is from 0 to 65535: where 0 represents the lowest level of the lightness (light turned off) and 65535 represents highest level of light emitted by the device.
     * @param hue The hue of a color light emitted by the component. The valid range is 0 to 65535 which corresponds to the (hue * 360) / 65535.
     * @param saturation The saturation of a color light. The valid range is 0 to 65535, where 0 represents the lowest and 65535 represent the highest perceived saturation of a color light.
     * @param ackRequired Set to true to receive confirmation that the request has been processed by the target device, false otherwise.
     * @param transitionTime Time to transition to the target level in milliseconds or {@link MeshController#MESH_TRANSITION_TIME_DEFAULT} The valid range is 0 – 37200000 (10.5 hours). The accuracy depends on the value.  It is about 100ms if the transition time is less than 6 seconds and decreases to 10 minutes if the value is more than 10 minutes.
     * @param delay Delay in milliseconds before starting the transition.  The valid range is 0 to 1275 (1.2 seconds).
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} The network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} HSL method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} A component or a group with specified name is not found in the database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int hslSet(String componentName, int lightness, int hue, int saturation, boolean ackRequired, int transitionTime, short delay) {
        Log.d(TAG,"hslSet");
        if (isServiceConnected()) {
            return service.hslSet(componentName, lightness, hue, saturation, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }


    private boolean isServiceConnected() {
        if (service != null)
            return true;
        Log.d(TAG,"Mesh Service Disconnected!!");
        return false;
    }

    /**
     *  Uninitialize mesh controller.
     */
    public void unInitialize() {
        Log.d(TAG,"unInitialize");
        if (isServiceConnected()) {
            mCtx.unbindService(svcConn);
            service = null;
        }
    }

    /**
     *  This API shall be called by the application to indicate to the Mesh Service that the user started a continuous operation,
     *  for example, start changing the level using a slider, or start changing the lightness/hue/saturation
     *  using a color control. The application shall call stopTracking method when user
     *  releases the control.
     *
     */
    public void startTracking() {
        Log.d(TAG,"startTracking");
        if (isServiceConnected()) {
           service.startTracking();
        }
    }


    /**
     * This API shall be called by the application to indicate to the Mesh Service that the user stopped the continuous operation,
     * for example, stopped changing the level using a slider, or stopped changing the lightness/hue/saturation
     * using a color control.
     */
    public void stopTracking() {
        Log.d(TAG,"stopTracking");
        if (isServiceConnected()) {
            service.stopTracking();
        }
    }

    /**
     *  Check if connection to network exists.
     * @return true if connected, false otherwise
     */
    public boolean isConnectedToNetwork() {
        if (isServiceConnected()) {
            return service.isConnectedToNetwork();
        }
        return false;
    }

    /**
     * Scan for unprovisioned Mesh devices that are in the vicinity of the currently opened mesh network.
     * The scan results are obtained {@link IMeshControllerCallback#onDeviceFound}
     * @param start true to start scan unprovisioned devices, false to stop scan.
     * @param uuid uuid of the device which shall be used for remote provisioning
     * @return true if scan is started, false otherwise.
     */
    public boolean scanMeshDevices(boolean start, UUID uuid) {
        Log.d(TAG,"scanMeshDevices");
        if (isServiceConnected()) {
            return service.scanMeshDevices(start, uuid);
        }
        return false;
    }


    /**
     * Set Device Configuration.
     * <p>This method can be used to set up a default configuration that will be used to configure newly provisioned devices or to reconfigure an existing device.
     * If the deviceName parameter is NULL, the configuration parameters will apply to the devices that a newly provisioned. Operation completes synchronously, and return indicates if parameters have been accepted.
     * If the deviceName parameter is not NULL, the mesh stack will attempt to reconfigure already provisioned and configured device. The success indicates that the operation has been started. When the operation completes the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback will be executed.</p>
     *
     * @param deviceName the name of the device whose device configuration is set. deviceName parameter is NULL, the configuration parameters apply to the devices that are added to the mesh network.
     * @param isGattProxy true if the device GATT Proxy feature must be enabled. The parameter has no effect if the device does not support GATT Proxy feature.
     * @param isFriend true if the device Friend feature must be enabled. The parameter has no effect if the device does not support Friend feature.
     * @param isRelay true if the device Relay feature must be enabled. The parameter has no effect if the device does not support Relay feature.
     * @param sendNetBeacon true if the secure network beacon must be enabled.
     * @param relayXmitCount 0-7 If the Relay feature is enabled, this parameter indicates how many times the message shall be retransmitted by the device. Zero indicates that the device shall send it once.
     * @param relayXmitInterval If the Relay feature is enabled and relayXmitCount is not zero, this parameter indicates the interval in milliseconds between message retransmissions.
     * @param defaultTtl 0x00-0x7F value which is used to set TTL while sending messages
     * @param netXmitCount 0-7 indicates the number of network retransmissions when a message is originated from the node. Zero indicates that the device shall send each network segment once.
     * @param netXmitInterval If netXmitCount is not zero, this value indicates the delay in milliseconds between segments’ transmissions.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} if the network database is not setup correctly,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} A component or a group with specified name is not found in the database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} The operation has started successfully.</li></ul>
     */

    public int setDeviceConfig(String deviceName, boolean isGattProxy, boolean isFriend, boolean isRelay, boolean sendNetBeacon, int relayXmitCount, int relayXmitInterval, int defaultTtl, int netXmitCount, int netXmitInterval)
    {
        Log.d(TAG,"setDeviceConfig");
        if (isServiceConnected()) {
            return service.setDeviceConfig(deviceName, (isGattProxy==true)?1:0, (isFriend==true)?1:0, (isRelay==true)?1:0, (sendNetBeacon==true)?1:0, relayXmitCount, relayXmitInterval, defaultTtl, netXmitCount, netXmitInterval);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     *  Set Publication Configuration.
     *
     *  <p>This method can be used to set up default publication parameters that will be used to configure newly provisioned devices or to reconfigure existing devices. </p>
     *  <p>When a new device is being added to the mesh network the publication parameters are used for the main function of the device. For example, if a dimmer is being provisioned, the publication will be configured the Generic Level Client model of the device. </p>
     *  <p>After a device has been added to the mesh network, the application can use the {@link #configurePublication} method to apply parameters set by Set Publication Configuration method to a specific function of the device. </p>
     *  <p>The method completes synchronously, and return indicates if parameters have been accepted. </p>
     *  <p>If the publishPeriod parameter is not zero, the device will send publication periodically. Otherwise the device will send publication once when the state of the device changes. For example, a dimmable light can be configured to send the Status message when the level is changed locally. </p>
     * @param publishCredentialFlag 0-1 controls the credentials used to publish messages from a device.
     * @param publishRetransmitCount 0-7 indicates the number of times that a publication message is retransmitted by a device. Value 0 indicates that the message is send once.  Note that the message may still be retransmitted several times on the network layer.
     * @param publishRetransmitInterval If the publishRetransmitCount is not zero, the value indicates the interval in milliseconds between retransmissions of the publication message.
     * @param publishTtl 0-7F is TTL value (maximum number of hops) for outgoing messages published by the device.
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} if publication parameters are accepted and will be used to configure publications.</li></ul>
     */
    public int setPublicationConfig(int publishCredentialFlag, int publishRetransmitCount, int publishRetransmitInterval, int publishTtl)
    {
        Log.d(TAG,"SetPublicationConfig");
        if (isServiceConnected()) {
            return service.setPublicationConfig(publishCredentialFlag, publishRetransmitCount, publishRetransmitInterval, publishTtl);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Perform factory reset of the device.
     *
     * <p>This method starts execution of the factory reset of the device. Not to compromise the mesh network security, after the node has been removed from the mesh network, all the security keys that were known to the device are changed. </p>
     * <p>The success indicates that the operation has been started. When the operation completes the {@link IMeshControllerCallback#onDatabaseChanged(String meshName)} callback will be executed. </p>
     *
     * @param componentName Name of the device to be reset
     * @return <ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_CLOSED} the mesh network is not opened,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_STATE} another configuration operation is in progress,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NOT_FOUND} if device with specified name is not found in the mesh database,</li>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} if command is accepted and stack configuration has started the operation,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */

    public int resetDevice(String componentName)
    {
        Log.d(TAG,"resetDevice");
        if (isServiceConnected()) {
            return service.resetDevice(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Send vendor specific data to a device or group of device.
     *
     * This function allows the application to send arbitrary data to a specific device or a group of devices.
     * @param deviceName Name of the destination device or group of devices
     * @param companyId Company Id of the vendor
     * @param modelId Model Id of the vendor model
     * @param opcode Opcode of the vendor message
     * @param buffer Vendor data to be sent
     * @param len Length of the vendor data
     * @return <ul>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NETWORK_CLOSED} the mesh network is not opened,</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_INVALID_STATE} another configuration operation is in progress,</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} The vendor specific method is not available on the specified component or the group.</li>
     * <li>{@link MeshController#MESH_CLIENT_ERR_NOT_FOUND} if a component or a group with specified name is not found in the mesh database,</li>
     * <li>{@link MeshController#MESH_CLIENT_SUCCESS} if command is accepted and stack configuration has started the operation,</li>
     * <li>{@link MeshController#MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */

    public int vendorDataSend(String deviceName, short companyId, short modelId, byte opcode,boolean disable_ntwk_retransmit, byte[] buffer, short len)
    {
        Log.d(TAG,"vendorDataSend");
        if (isServiceConnected()) {
            return service.vendorDataSet(deviceName, companyId, modelId, opcode,disable_ntwk_retransmit, buffer, len);
        }
        return COMMAND_FAILURE;
    }

    /**
     * Get lightness state of a component or all components in a group.
     *
     * <p>This method allows application to send a query to a device or a group of devices to report the current lightness level.</p>
     * @param deviceName Name of the component whose lightness value is requested
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} if a component or a group with specified name is not found in the mesh database,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} Get Lightness method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} a request has been sent to the specified device or group of devices.</li></ul>
     */
    public int lightnessGet(String deviceName) {
        Log.d(TAG,"lightnessGet");
        if (isServiceConnected()) {
            return service.lightnessGet(deviceName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set lightness level.
     *
     * <p>This method can be used by the application to set lightness level of a light or a group of lights.  If function call returns success, the request has been successfully sent out. If ackRequired parameter is set to true, the {@link IMeshControllerCallback#onLightnessStateChanged} method will be executed at the time the reply from the component is received. If the request is for the group, the callback is executed for each component of the group that sends the status.
     * The transition time parameter can be used to transition to the target lightness over specified period.
     * The delay parameter can be used to tell the target device to start transitioning to the target lightness value after a delay.
     * It is not recommended to send a request with acknowledgement to a group of more than several lights.</p>
     *
     * @param name Name of the component or the group of components
     * @param lightness New value of the lightness. Valid range is from 0 to 65535: where 0 represents the lowest level of the lightness
     *                  (light turned off) and 65535 represents highest level of light emitted by the device.
     * @param ackRequired Set to true to receive confirmation that the request has been processed by the target device, false otherwise.
     * @param transitionTime Time to transition to the target level in milliseconds or {@link #MESH_TRANSITION_TIME_DEFAULT}
     *                       The valid range is 0 – 37200000 (10.5 hours). The accuracy depends on the value.  It is about 100ms if the transition time
     *                       is less than 6 seconds and decreases to 10 minutes if the value is more than 10 minutes.
     * @param delay Delay in milliseconds before starting the transition.  The valid range is 0 to 1275 (1.2 seconds).
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with the specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} Set Lightness method is not available on the specified component or the group,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} a request has been sent to the specified device or the group of devices.</li></ul>
     */
    public int lightnessSet(String name, int lightness, boolean ackRequired, int transitionTime, short delay) {
        Log.d(TAG,"lightnessSet");
        if (isServiceConnected()) {
            return service.lightnessSet(name, lightness, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Get CTL state of a component or all components in a group.
     *
     * <p>This method can be used by the application to get current state of a color temperature and brightness of a light or a group of lights.  If function call returns success, the request has been successfully sent out.
     * When a reply from a component is received, the {@link IMeshControllerCallback#onCtlStateChanged} method is executed. If the query is for the group, the callback is executed for each component of the group.
     * It is not recommended to send the request to a group of more than several lights.</p>
     * @param name Name of the component or the group of components
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} Get CTL method is not available on the specified component or the group.</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} a request has been sent to the specified device or group of devices..</li></ul>
     */

    public int ctlGet(String name) {
        Log.d(TAG,"ctlGet");
        if (isServiceConnected()) {
            return service.ctlGet(name);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set CTL state of a light or a group of lights.
     *
     * <p>This method can be used by the application to set color temperature and brightness level of a light or a group of lights.  If function call returns success, the request has been successfully sent out. If ackRequired parameter is set to true, the {@link IMeshControllerCallback#onCtlStateChanged} method will be executed at the time when the reply from the component is received. If the request is for the group, the callback is executed for each component of the group that sends the status.
     * The transition time parameter can be used to transition to the target CTL state over specified period.
     * The delay parameter can be used to tell the target device to start transitioning to the target CTL state after a delay.
     * It is not recommended to send a request with acknowledgement to a group of more than several lights.</p>
     *
     * @param name Name of the component or a group
     * @param lightness New value of the lightness. Valid range is from 0 to 65535: where 0 represents the lowest level of the lightness (light turned off) and 65535 represents highest level of light emitted by the device.
     * @param temperature The desired color temperature of white light in kelvin. The valid range is 800 to 20,000.
     * @param deltaUv The distance from the Black Body curve. The valid range is -32768 to  32767 which corresponds to -1.0 to 1.0 with a 16-bit resolution.
     * @param ackRequired Set to true to receive confirmation that the request has been processed by the target device, false otherwise.
     * @param transitionTime Time to transition to the target level in milliseconds or {@link MeshController#MESH_TRANSITION_TIME_DEFAULT} The valid range is 0 – 37200000 (10.5 hours). The accuracy depends on the value.  It is about 100ms if the transition time is less than 6 seconds and decreases to 10 minutes if the value is more than 10 minutes.
     * @param delay Delay in milliseconds before starting the transition.  The valid range is 0 to 1275 (1.2 seconds).
     *
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with the specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} A mesh network is not connected while a connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL} Set CTL method is not available on the specified component or the group.</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} A request has been sent to the specified device or group of devices.</li></ul>
     */

    public int ctlSet(String name, int lightness, short temperature, short deltaUv, boolean ackRequired, int transitionTime, short delay) {
        Log.d(TAG,"ctlSet");
        if (isServiceConnected()) {
            return service.ctlSet(name, lightness, temperature, deltaUv, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }


    /**
     *  Gets a list of target methods.
     *
     * <p>During provisioning and configuration process, the mesh stack retrieves the composition data from the device and saves it in the mesh database. This function can be used to retrieve from the database the list of methods how the component can be controlled. For example, the component which represents the color light will return HSL, Lightness and On/Off methods. </p>
     * <p>The information obtained using this method along with the information obtained using the {@link #getControlMethods} method can be used to bind devices together. For example, a switch component which supports OnOff Control method can be bound to a colored, dimmable, or on/off light because all of them support OnOff Target method. </p>
     * @param componentName Name of the component for which the information is requested.
     * @return list of methods that can be used to control the device. The values can be <ul>
     * <li>{@link #MESH_CONTROL_METHOD_ONOFF},</li>
     * <li>{@link #MESH_CONTROL_METHOD_LEVEL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_LIGHTNESS},</li>
     * <li>{@link #MESH_CONTROL_METHOD_POWER},</li>
     * <li>{@link #MESH_CONTROL_METHOD_HSL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_CTL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_XYL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_VENDOR} </li></ul>
     */

    public String[] getTargetMethods(String componentName) {
        Log.d(TAG,"getTargetMethods");
        if (isServiceConnected()) {
            return service.getTargetMethods(componentName);
        }
        return null;
    }

    /**
     *  Gets a list of control methods.
     *
     <p>During provisioning and configuration process, the mesh stack retrieves the composition data from the device and saves it in the mesh database. This function can be used to retrieve from the database the list of control methods supported by the component. For example, a switch will support the On/Off method while a dimmer will support the Level method. </p>
     <p>The information obtained using this method along with the information obtained using the {@link #getTargetMethods} method can be used to bind devices together. For example, a switch component which supports OnOff Control method can be bound to a colored, dimmable, or on/off light because all of them support OnOff Target method. </p>
     * @param componentName Name of the component for which the information is requested.
     * @return list of methods that the device can control. The values can be <ul>
     * <li>{@link #MESH_CONTROL_METHOD_ONOFF},</li>
     * <li>{@link #MESH_CONTROL_METHOD_LEVEL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_LIGHTNESS},</li>
     * <li>{@link #MESH_CONTROL_METHOD_POWER},</li>
     * <li>{@link #MESH_CONTROL_METHOD_HSL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_CTL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_XYL},</li>
     * <li>{@link #MESH_CONTROL_METHOD_VENDOR}</li></ul>
     */
    public String[] getControlMethods(String componentName) {
        Log.d(TAG,"getControlMethods");
        if (isServiceConnected()) {
            return service.getControlMethods(componentName);
        }
        return null;
    }

    /**
     *  Connect to a specific node in the currently opened network.
     *
     *  <p>When {@link #connectNetwork} method is used, the connection is established to arbitrary device
     *  which supports Proxy functionality. In some cases, the application may want to establish a connection to a specific device.
     *  Using this method, the application can identify the component to connect to. </p>
     *  <p>Note that for the connection to be established, the component has to be in the immediate radio range
     *  from the phone if {@link #TRANSPORT_GATT} is used or from the gateway if {@link #TRANSPORT_IP} is used . </p>
     *  At the end of the operation the {@link IMeshControllerCallback#onNodeConnectionStateChanged}
     *  callback is executed indicating if connection has been successful.
     *
     * @param componentName Name of the component to connect to.
     * @param scanDuration Scan timeout indicates how long the mesh stack should scan for a proxy device.

     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE} The network is already connected.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with the specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS} Mesh stack started searching for the specified component.</li></ul>
     */

    public byte connectComponent(String componentName, byte scanDuration) {
        Log.d(TAG,"connectComponent");
        if (isServiceConnected()) {
            return service.connectComponent(componentName, scanDuration);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Start device firmware upgrade
     *
     * @param firmwareFile the firmware file name
     * @param metadataFile the MetaData file name
     * {@link #DFU_METHOD_APP_TO_ALL},
     * {@link #DFU_METHOD_PROXY_TO_DEVICE},
     * {@link #DFU_METHOD_APP_TO_ALL},
     * {@link #DFU_METHOD_APP_TO_DEVICE}
     * @return <ul>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with the specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL},</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS}.</li></ul>
     */

    public int startDfu(String firmwareFile, byte dfuMethod) {
        if (isServiceConnected()) {
            return service.startDfu(firmwareFile, dfuMethod);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    public int stopDfu() {
        if (isServiceConnected()) {
            service.stopDfu();
            return MESH_CLIENT_SUCCESS;
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    public void getDfuStatus(int statusInterval) {
        if (isServiceConnected()) {
            service.getDfuStatus(statusInterval);
        }
    }

    public int startOta(String firmwareFile) {
        if (isServiceConnected()) {
            service.startOta(firmwareFile);
            return MESH_CLIENT_SUCCESS;
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Stop OTA upgrade
     * @return the return values can be <ul>
     * <li>{@link #MESH_CLIENT_SUCCESS}</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_STATE},</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li></ul>
     */
    public int stopOta() {
        Log.d(TAG,"stopOta");
        if (isServiceConnected()) {
            service.stopOta();
            return MESH_CLIENT_SUCCESS;
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Send the received proxy packet from oob (cloud) to the meshcore
     * @param data Data received from the cloud
     *
     * @return true if sent successfully false otherwise
     */
    public boolean sendReceivedProxyPacket(byte[] data) {
        Log.d(TAG,"sendReceivedData");
        if (isServiceConnected()) {
            return service.sendReceivedData(data);
        }
        return false;
    }

    /**
     * Import network to the device.
     * @param provisionerName provisionerName
     * @param jsonString JSON string from the file to be imported,
     *                   On successful import, Network will be opened
     *                   and {@link IMeshControllerCallback#onNetworkOpenedCallback(byte)} will be called.
     * @return network name on success null otherwise
     */
    public String importNetwork(String provisionerName, String jsonString) {
        Log.d(TAG,"importNetwork");
        String result = null;
        if (isServiceConnected()) {
            result = service.importNetwork(provisionerName, jsonString);
           if(result != null)
           {
               mCurrNetwork = result;
           }
        }
        return result;
    }

    /**
     * Export existing network.
     *
     * <p>The whole database is returned as a string. Export is not possible while a device is being provisioned or reconfigured.
     *
     * @param meshName : Name of the meshNetwork to be exported</p>
     * @return jsonString : JSON string of the network to be exported, null if network is busy or if no network is available.
     */
    public String exportNetwork(String meshName){
        Log.d(TAG,"exportNetwork");
        if (isServiceConnected()) {
            return service.exportNetwork(meshName);
        }
        return null;
    }


    /**
     * Delete network database.
     *
     * Deletes mesh network locally by deleting the network.json file
     * app developers are expected to export the network info before deletion
     *
     * @param provisionerName User friendly provisioner name.
     * @param meshName User friendly mesh network name.
     * @return 0 if failure 1 otherwise <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} </li></ul>
     */
    public int deleteNetwork(String provisionerName, String meshName) {
        Log.d(TAG,"deleteNetwork");
        if (isServiceConnected()) {
            return service.deleteNetwork(provisionerName, meshName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Gets information about property of the component.
     *
     * @param componentName Component name of the device.
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error.</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a group of components with the specified name is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} The mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li></ul>
     */
    public byte getComponentInfo(String componentName) {
        Log.d(TAG,"getComponentInfo");
        if (isServiceConnected()) {
            return service.getComponentInfo(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Notify mesh client library that network connection is changed externally via cloud
     * Typically this is used when mesh controller is connected/disconnected to mesh gateway/cloud
     * @param connId Connection id of connection, 0 on disconnection
     * @return {@link #MESH_CLIENT_SERVICE_NOT_CONNECTED},<ul>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li></ul>
     */
    public int networkConnectionChanged(int connId) {
        Log.d(TAG,"networkConnectionChanged");
        if (isServiceConnected()) {
            return service.networkConnectionChanged(connId);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set Cadence of a sensor.
     *
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @param fastCadencePeriodDivisor Divisor of the publish period at which sensor has to send status messages when value is in between fastcadence low and high
     * @param triggerType the type sensor has to send data in. The two values can be {@link #MESH_CLIENT_TRIGGER_TYPE_NATIVE} or {@link #MESH_CLIENT_TRIGGER_TYPE_PERCENTAGE}
     * @param triggerDeltaDown the change unit in which if value changes status has to be sent
     * @param triggerDeltaUp the change unit in which if value changes status has to be sent
     * @param minInterval the minimum interval in which sensor has to send the status message in milliseconds.
     * @param fastCadenceLow the low value which triggers fast cadence
     * @param fastCadenceHigh the high value till which triggers fast cadence
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * </ul>
     */
    public int sensorCadenceSet(String componentName, int propertyId, short fastCadencePeriodDivisor, boolean triggerType,
                                int triggerDeltaDown, int triggerDeltaUp, int minInterval, int fastCadenceLow, int fastCadenceHigh) {
        Log.d(TAG,"sensorCadenceSet");
        if (isServiceConnected()) {
            return service.sensorCadenceSet(componentName, propertyId, fastCadencePeriodDivisor, triggerType,
            triggerDeltaDown, triggerDeltaUp, minInterval, fastCadenceLow, fastCadenceHigh);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Get settings of a sensor.
     *
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @return list of settings supported by sensor, null otherwise
     */
    public short[] sensorSettingsGet(String componentName, int propertyId)
    {
        Log.d(TAG,"sensorSettingsGet");
        if (isServiceConnected()) {
            return service.sensorSettingsGet(componentName, propertyId);
        }
        return null;
    }

    /**
     * Get List of properties supported by sensor.
     *
     * @param componentName Component name of the device.
     * @return List of properties supported by sensor, null otherwise.
     */
    public int[] sensorPropertyListGet (String componentName)
    {
        Log.d(TAG,"sensorPropertyListGet");
        if (isServiceConnected()) {
            return service.sensorPropertyListGet(componentName);
        }
        return null;
    }

    /**
     * Set the setting value of the sensor.
     *
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @param settingPropertyId Setting Property Id of sensor. This Id is got as a response to {@link #sensorSettingsGet(String, int)}
     * @param val value to be set for the setting
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * </ul>
     */
    public int sensorSettingSet(String componentName, int propertyId, short settingPropertyId, byte[] val)
    {
        Log.d(TAG,"sensorSettingSet");
        if (isServiceConnected()) {
            return service.sensorSettingSet(componentName, propertyId, settingPropertyId, val);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     *  Get the current value from sensor.
     *
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * </ul>
     */
    public int sensorGet(String componentName, int propertyId)
    {
        Log.d(TAG,"sensorGet");
        if (isServiceConnected()) {
            return service.sensorGet(componentName, propertyId);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * This function configures mesh library to receive and pass to the application, messages that are sent to the group.
     * For example. This method can be used to register to receive, for example, “SENSOR” messages sent in a specific group.
     *
     * @param controlMethod If the control_method parameter is NULL, the library will register to receive messages for all types of control_methods.  Alternatively the control_method can be set to one of the predefined strings: ONOFF, LEVEL, POWER, HSL, CTL, XYL, VENDOR_XXXX or SENSOR. given by  {@link #getControlMethods(String)}
     * @param groupName If the group_name is NULL, the library will register to receive messages sent to all the groups. Otherwise the group_name shall be set to the name of the existing group.
     * @param startListening If start_listen is false the library un-subscribes to receive messages from the specified group or all the groups. If start_listen is true the library subscribes to receive messages from the specified group or all the groups.
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters</li>
     * </ul>
     */
    public int listenForAppGroupBroadcasts(String controlMethod, String groupName, boolean startListening)
    {
        Log.d(TAG,"listenForAppGroupBroadcasts");
        if (isServiceConnected()) {
            return service.listenForAppGroupBroadcasts(controlMethod, groupName, startListening);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Gets the target name to whom publication is set.
     *
     * @param componentName componentName Component name of the device.
     * @param isClient true if it is a controlMethod, false if it is a targetMethod.
     * @param method one of the methods returned by the {@link #getTargetMethods(String)} or returned by the {@link #getControlMethods(String)} method.
     * @return target name to whom the publication is set, null otherwise.
     */
    public String getPublicationTarget(String componentName, boolean isClient, String method)
    {
        Log.d(TAG,"getPublicationTarget");
        if (isServiceConnected()) {
            return service.getPublicationTarget(componentName, isClient, method);
        }
        return null;
    }

    /**
     * Gets the current publication period of the component.
     *
     * @param componentName Component name of the device.
     * @param isClient true if it is a controlMethod, false if it is a targetMethod.
     * @param method one of the methods returned by the {@link #getTargetMethods(String)} or returned by the {@link #getControlMethods(String)} method.
     * @return publication period of the component.
     * {@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.
     */
    public int getPublicationPeriod(String componentName, boolean isClient, String method)
    {
        Log.d(TAG,"getPublicationPeriod");
        if (isServiceConnected()) {
            return service.getPublicationPeriod(componentName, isClient, method);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Gets the value Light LC Mode (manual/automatic)
     * @param componentname Component name of the device.
     * @return the values can be<ul>
     * <li>{@link #MESH_CLIENT_LC_MODE_ON}</li>
     * <li>{@link #MESH_CLIENT_LC_MODE_OFF}</li></ul>
     */
    public int getLightLcMode(String componentname)
    {
        Log.d(TAG,"getLightLcMode");
        if (isServiceConnected()) {
            return service.getLightLcMode(componentname);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Sets the Light LC Mode.  If mode is set to true, the device is operating in the automated mode.
     * @param componentName  Component name of the device.
     * @param mode the values can be<ul>
     *       <li>{@link #MESH_CLIENT_LC_MODE_ON}</li>
     *       <li>{@link #MESH_CLIENT_LC_MODE_OFF}</li></ul>
     * @return
     */
    public int setLightLcMode(String componentName, int mode)
    {
        Log.d(TAG,"getLightLcMode");
        if (isServiceConnected()) {
            return service.setLightLcMode(componentName, mode);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Gets the value of the property with specified property id
     * @param componentName Component name of the device.
     * @return the values can be<ul>
     * <li>{@link #MESH_CLIENT_LC_OCCUPANCY_MODE_ON}</li>
     * <li>{@link #MESH_CLIENT_LC_OCCUPANCY_MODE_OFF}</li></ul>
     */
    public int getLightLcOccupancyMode(String componentName)
    {
        Log.d(TAG,"getLightLcOccupancyMode");
        if (isServiceConnected()) {
            return service.getLightLcOccupancyMode(componentName);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Sets the Light LC Occupancy Mode.  If mode is set to true, the device is operating in the automated mode.
     * @param componentName Component name of the device.
     * @param mode the values can be<ul>
     * <li>{@link #MESH_CLIENT_LC_OCCUPANCY_MODE_ON}</li>
     * <li>{@link #MESH_CLIENT_LC_OCCUPANCY_MODE_OFF}</li></ul>
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li></ul>
     */
    public int setLightLcOccupancyMode(String componentName, int mode)
    {
        Log.d(TAG,"setLightLcOccupancyMode");
        if (isServiceConnected()) {
            return service.setLightLcOccupancyMode(componentName, mode);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Property Get.
     * If operation is successful, the callback will be executed when reply from the peer is received.
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li></ul>

     */
    public int getLightLcProperty(String componentName, int propertyId)
    {
        Log.d(TAG,"getLightLcProperty");
        if (isServiceConnected()) {
            return service.getLightLcProperty(componentName, propertyId);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Property Set
     * If operation is successful, the callback will be executed when reply from the peer is received.
     * @param componentName Component name of the device.
     * @param propertyId Property Id of the sensor.
     * @param value value to be set for the component
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     */
    public int setLightLcProperty(String componentName, int propertyId, int value)
    {
        Log.d(TAG,"setLightLcProperty");
        if (isServiceConnected()) {
            return service.setLightLcProperty(componentName, propertyId, value);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

    /**
     * Set Light Controller On/Off state of a device
     * @param componentName Component name of the device.
     * @param onoff on/off state of the component to be set. true for on and false for off
     * @param ackRequired true if ack is needed, false otherwise.
     * @param transitionTime Time in milliseconds or {@link #MESH_TRANSITION_TIME_DEFAULT} if default value has to be used.
     * @param delay Time in milliseconds
     * @return <ul>
     * <li>{@link #MESH_CLIENT_SERVICE_NOT_CONNECTED} The mesh service is not initialized.</li>
     * <li>{@link #MESH_CLIENT_SUCCESS},</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_FOUND} a component or a propertyId is not found in the mesh database.</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_DB} mesh network database error,</li>
     * <li>{@link #MESH_CLIENT_ERR_NO_MEMORY} the mesh stack cannot allocate memory to perform the operation,</li>
     * <li>{@link #MESH_CLIENT_ERR_NETWORK_CLOSED} if the network is not opened</li>
     * <li>{@link #MESH_CLIENT_ERR_METHOD_NOT_AVAIL}</li>
     * <li>{@link #MESH_CLIENT_ERR_INVALID_ARGS} on invalid parameters</li>
     * <li>{@link #MESH_CLIENT_ERR_NOT_CONNECTED} a mesh network is not connected and connection to the mesh network is required to perform this operation.</li>
     */
    public int setLightLcOnOff(String componentName, byte onoff, boolean ackRequired, int transitionTime, int delay)
    {
        Log.d(TAG,"setLightLcOnOff");
        if (isServiceConnected()) {
            return service.setLightLcOnOff(componentName, onoff, ackRequired, transitionTime, delay);
        }
        return MESH_CLIENT_SERVICE_NOT_CONNECTED;
    }

}
