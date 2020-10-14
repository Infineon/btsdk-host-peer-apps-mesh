package com.cypress.le.mesh.meshframework;
import java.util.UUID;

/**
 * This abstract class is used to implement MeshController callbacks.
 * <p>
 * All Network related events are notified to application through {@link IMeshControllerCallback}
 * </p>
 */
public interface IMeshControllerCallback {

    /**
     * Indicates network is disconnected
     */
    public static final byte NETWORK_CONNECTION_STATE_DISCONNECTED       = 0;
    /**
     * Indicates network is connected
     */
    public static final byte NETWORK_CONNECTION_STATE_CONNECTED          = 1;

    /**
     * Indicates that OTA upgrade device is connected
     */
    public static final byte OTA_UPGRADE_STATUS_CONNECTED                = 0;
    /**
     * Indicates that OTA upgrade device is disconnected
     */
    public static final byte OTA_UPGRADE_STATUS_DISCONNECTED             = 1;
    /**
     * Indicates that OTA upgrade is in progress
     */
    public static final byte OTA_UPGRADE_STATUS_IN_PROGRESS              = 2;
    /**
     * Indicates that OTA upgrade is complete
     */
    public static final byte OTA_UPGRADE_STATUS_COMPLETED                = 3;
    /**
     * Indicates that OTA upgrade is aborted
     */
    public static final byte OTA_UPGRADE_STATUS_ABORTED                  = 4;
    /**
     * Indicates that OTA upgrade is not supported
     */
    public static final byte OTA_UPGRADE_STATUS_NOT_SUPPORTED            = 5;
    /**
     * Indicates that OTA upgrade service is not found in the device
     */
    public static final byte OTA_UPGRADE_STATUS_SERVICE_NOT_FOUND        = 6;
    public static final byte OTA_UPGRADE_STATUS_UPGRADE_TO_ALL_STARTED   = 7;
    /**
     * Indicates that the provisioning process is unsuccessful.
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_FAILED         = 0;
    /**
     * Indicates that the unprovisioned devices are being scanned
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_SCANNING       = 1;
    /**
     * Indicates that the provisioning device is being connected
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_CONNECTING     = 2;
    /**
     * Indicates an ongoing provisioning process
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_PROVISIONING   = 3;
    /**
     * Indicates an ongoing configuration process
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_CONFIGURING    = 4;
    /**
     * Indicates that the provisioning and configuration process is complete.
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_SUCCESS        = 5;
    /**
     * Indicates that the provisioning process is complete.
     */
    public static final byte MESH_CLIENT_PROVISION_STATUS_END            = 6;

    /**
     * Indicates that the message was unable to be delivered to the device.
     */
    public static final byte MESH_CLIENT_NODE_WARNING_UNREACHABLE        = 0;
    /**
     * Indicates that the device is connected
     */
    public static final byte MESH_CLIENT_NODE_CONNECTED                  = 1;
    /**
     * Indicates that the device is not reachable expects user to take an action
     */
    public static final byte MESH_CLIENT_NODE_ERROR_UNREACHABLE          = 2;

    /**
     * Indicates that the mesh service is connected
     */
    public static final int MESH_SERVICE_CONNECTED                       = 0x01;

    /**
     * Indicates that the mesh service is disconnected
     */
    public static final int MESH_SERVICE_DISCONNECTED                    = 0x00;

//DFU STATES
    /**
     * Indicates that distribution state is ready, distribution is not active
     */
    public static final byte DISTRIBUTION_STATUS_READY                   = 0x00; ///< ready, distribution is not active
    /**
     * Indicates that distribution state is active
     */
    public static final byte DISTRIBUTION_STATUS_ACTIVE                  = 0x01;
    /**
     * Indicates that no such Company ID and Firmware ID combination is present
     */
    public static final byte DISTRIBUTION_STATUS_WRONG_ID                = 0x02;
    /**
     * Indicates that mesh is busy with different distribution
     */
    public static final byte DISTRIBUTION_STATUS_BUSY                    = 0x03;
    /**
     * Indicates that update nodes list is too long
     */
    public static final byte DISTRIBUTION_STATUS_LIST_TOO_LONG           = 0x04;
    /**
     * Indicates that distribution failed
     */
    public static final byte DISTRIBUTION_STATUS_FAILED                  = 0x05;
    /**
     * Indicates that distribution completed
     */
    public static final byte DISTRIBUTION_STATUS_COMPLETED               = 0x06;

    /**
     * Callback invoked upon completion of provisioning and configuration of an unprovisioned mesh device.
     *<p>
     *     Use {@link MeshController#provision} to start provisioning and configuration process
     *</p>
     *  @param deviceUuid  UUID of device just provisioned and configured
     *  @param status  Result of Provisioning operation. The status can be <ul>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_FAILED}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_SCANNING}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_CONNECTING}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_PROVISIONING}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_CONFIGURING}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_SUCCESS}, </li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_PROVISION_STATUS_END} </li></ul>
     */
    void onProvisionComplete(UUID deviceUuid, byte status);

    /**
     * OnOff status received from a component
     *
     * @param name  Name of the component which reported OnOff state change
     * @param targetOnOff Target OnOff state of the component
     * @param presentOnOff Present OnOff state of the component
     * @param remainingTime Transition time from present state to target state
     */
    void onOnOffStateChanged(String name, byte targetOnOff, byte presentOnOff, int remainingTime);

    /**
     * Level status received from a component
     *
     * @param name  Name of the component to which reported the Level state change
     * @param targetLevel Target Level of the component
     * @param presentLevel Present Level of the component
     * @param remainingTime Transition time from present Level to target Level
     */
    void onLevelStateChanged(String name, short targetLevel, short presentLevel, int remainingTime);

    /**
     * HSL status received from a component
     * @param name  Name of the component which reported HSL state change
     * @param lightness Current lightness of the component
     * @param hue Current hue of the component
     * @param remainingTime Transition time from present state to target state
     *
     */
    void onHslStateChanged(String name, int lightness, int hue, int saturation, int remainingTime);

    /**
     * Event triggered whenever MeshService is bound or unbound
     *
     * @param status The values can be <ul>
     * <li>{@link #MESH_SERVICE_CONNECTED},</li>
     * <li>{@link #MESH_SERVICE_DISCONNECTED}</li></ul>
     */
    void onMeshServiceStatusChanged(int status);

    /**
     * Callback is invoked when an unprovisioned mesh device had been detected.
     * The application can use {@link MeshController#scanMeshDevices} to search for unprovisioned mesh devices.
     *
     * @param uuid UUID of the found unprovisioned device.
     * @param name Name of the found unprovisioned device.
     */
    void onDeviceFound(UUID uuid, String name);

    /**
     * Callback indicating the connection with the mesh network is established or lost.
     * @param transport Transport values can be
     * {@link MeshController#TRANSPORT_IP} or {@link MeshController#TRANSPORT_GATT}
     * @param status {@link IMeshControllerCallback#NETWORK_CONNECTION_STATE_DISCONNECTED} if network connection state is disconnected,
     * {@link IMeshControllerCallback#NETWORK_CONNECTION_STATE_CONNECTED} if network connection state is connected
     */
    void onNetworkConnectionStatusChanged(byte transport, byte status);

    /**
     * CTL status received from a component
     * @param deviceName Name of the component for which response is received
     * @param presentLightness Current lightness of the component
     * @param presentTemperature Current temperature of the component
     * @param targetLightness Target lightness of component
     * @param targetTemperature Current temperature of component
     * @param remainingTime Remaining time to transit to target state
     */
    void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime);

    /**
     * Lightness status received from a component.
     * @param deviceName Name of the component for which response is received
     * @param target Target lightness of component
     * @param present Current lightness of component
     * @param remainingTime Remaining time to transit to target state
     */
    void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime);

    /**
     * The callback is triggered as a a result of {@link MeshController#connectComponent} to indicate if connection to the network has been successful.
     *
     * @param status Current connection state of the node the status can be
     * <ul>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_NODE_CONNECTED},</li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_NODE_ERROR_UNREACHABLE},</li>
     * <li>{@link IMeshControllerCallback#MESH_CLIENT_NODE_WARNING_UNREACHABLE}</li>
     * </ul>
     * @param componentName Name of the component
     */
    void onNodeConnectionStateChanged(byte status, String componentName);


    /**
     *  Callback invoked to indicate status of OTA firmware upgrade.
     *
     * @param status Status of OTA firmware upgrade.The values can be<ul>
     * <li>{@link #OTA_UPGRADE_STATUS_CONNECTED},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_DISCONNECTED},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_IN_PROGRESS},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_COMPLETED},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_ABORTED},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_NOT_SUPPORTED},</li>
     * <li>{@link #OTA_UPGRADE_STATUS_SERVICE_NOT_FOUND}</li></ul>
     * @param percentComplete Percentage of OTA firmware upgrade completed.
     */
    void onOtaStatus(byte status, int percentComplete);

    /**
     *  When MeshController is connected to an external proxy such  gateway through cloud, application would require an interface through which it can receive the proxy data from the Mesh Stack. Typically application would send the received proxy packet to the gateway using an IoT Protocol of his choice.
     *
     * @param data Data to be sent over oob (cloud) when transport is {@link MeshController#TRANSPORT_IP}
     * @param length length of the data to be sent
     */
    void onReceivedProxyPktFromCore(byte[] data, int length);

    /**
     * Open network response.
     * This callback is triggered as a result of {@link MeshController#openNetwork }
     *
     * @param status status of the network open operation
     */
    void onNetworkOpenedCallback(byte status);

    /**
     * Callback called when internal database is changed.
     * This callback can be triggered as a result of multiple operations including {@link MeshController#provision}, {@link MeshController#moveComponentToGroup}, {@link MeshController#resetDevice}.
     *
     * @param meshName Name of the mesh network whose database was changed
     */
    void onDatabaseChanged(String meshName);

    /**
     * Component info callback is used to indicate the status of the get component info operation
     * and return the retrieved information.
     *
     * @param status status of getcomponent info operation
     * @param componentName Name of the component
     * @param componentInfo provides Information about the component and is represented as a string  in the following format companyID:ProductID:VendorID
     */
    void onComponentInfoStatus(byte status, String componentName, String componentInfo);

    /**
     * Status of current distribution process
     *
     * @param status status of the ongoing distribution process the values can be
     *               <ul>
     *               <li>{@link #DISTRIBUTION_STATUS_READY} ready, distribution is not active</li>
     *               <li>{@link #DISTRIBUTION_STATUS_ACTIVE} distribution is active</li>
     *               <li>{@link #DISTRIBUTION_STATUS_WRONG_ID} no such Company ID and Firmware ID combination</li>
     *               <li>{@link #DISTRIBUTION_STATUS_BUSY} busy with different distribution</li>
     *               <li>{@link #DISTRIBUTION_STATUS_LIST_TOO_LONG} update nodes list is too long</li>
     *               <li>{@link #DISTRIBUTION_STATUS_FAILED} distribution failed</li>
     *               <li>{@link #DISTRIBUTION_STATUS_COMPLETED} distribution completed</li>
     *               </ul>
     * @param data state related data
     */
    void onDfuStatus(byte state, byte[] data);

    /**
     * Sensor Status callback is used to indicate the status of the sensorGet operation
     * @param componentName  Name of the component
     * @param propertyId  Target Name of the component
     * @param data Current Sensor value
     */
    void onSensorStatusCb(String componentName, int propertyId, byte[] data);

    /**
     * Vendor Status callback is used to indicate the status of the Vendor operation
     * @param source  Source id of device
     * @param companyId  Company ID of the vendor
     * @param modelId  model ID of the vendor
     * @param opcode Opcode
     * @param ttl ttl
     * @param data Vendor data
     * @param dataLen Vendor data length
     */
    void onVendorStatusCb(short source, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen);

    /**
     * LC Mode status callback is executed as a result of the Get/Set operation
     * @param componentName Name of the component
     * @param mode
     */
    void onLightLcModeStatus(String componentName, int mode);

    /**
     * LC Occupancy Mode status callback is executed as a result of the Get/Set operation
     * @param componentName Name of the component
     * @param mode
     */
    void onLightLcOccupancyModeStatus(String componentName, int mode);

    /**
     * Property status callback is executed as a result of the Get/Set operation
     * @param componentName Name of the component
     * @param propertyId
     * @param value
     */
    void onLightLcPropertyStatus(String componentName, int propertyId, int value);
}
