/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * This file implements the MeshFramworkManager class which is main class wrap all libwicedmesh functions.
 * The caller Mesh App should invoke most of the mesh functions through the MeshFrameworkManager.shared instance.
 */

import Foundation
import CoreBluetooth

public func meshLog(_ items: Any..., seperator: String = "", terminator: String = "")
{
    if let logMsg = items.first as? String {
        MeshNativeHelper.meshClientLog("[MeshApp] " + logMsg)
    } else {
        #if DEBUG
        print(items, separator: seperator, terminator: terminator)
        #endif
    }
}

open class MeshFrameworkManager {
    public static let shared = MeshFrameworkManager()
    private let lock = NSLock()
    private var currentUserName: String?
    private var openingProvisionerName: String?
    private var openedProvisionerName: String?
    private var openingNetworkName: String?
    private var openedNetworkName: String?
    private var pesistentDb: UserDefaults?      // Useed to store App gloable uniquely and full of App life data.
    private var unreachableMeshDevices: [String] = []


    /**
     Protype of the importing and opening mesh network completion callback routine.

     @param networkName     Indicates the mesh network name is importing or opening.
     @param status          Indicates the mesh network importing or opening completion status.
     Only MeshErrorCode.MESH_SUCCESS indicates the mesh network is opened success, otherwise indicates openned failed or not opened.
     @param error           Indicates the calling result of invoke openMeshNetwork or meshClientNetworkImport API.
     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then the networkName and status arguments are valid.
     Otherwise, the arguments of networkName and status should be ignored.
     */
    public typealias OpenMeshNetworkCallback = (_ networkName: String?, _ status: Int, _ error: Int) -> ()

    /**
     Protype of the mesh database change callback.Indicates the name of the mesh network.

     @param networkName     Indicates the name of the mesh network.
     @param error               Indicates the result of calling mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then the networkName argument are valid.
     Otherwise, the arguments of networkName should be ignored.
     */
    public typealias MeshDatabaseChangedCallback = (_ networkName: String?, _ error: Int) -> ()
    private var meshDatabaseChangedCb: MeshDatabaseChangedCallback?

    /**
     Protype of the mesh network connection connected or disconnected link status udpated callback.

     @param isConnected     1 - indicates the mesh client currently has conencted to the mesh network; otherwise 0, the mesh client has disconnected from the mesh network.
     @param connId          The connection indentify for the status updated network connection.
     @param addr            Not used yet, always 0.
     @param isOverGatt      Indicates the mesh network connection is established through GATT bearer (1) or through ADV bearer (0).
     @param error           Indicates the result of calling the mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then all other arguements are valid.
     Otherwise, all other arguments are invalid and should be ignored.
     */
    public typealias MeshNetworkLinkStatusUpdatedCallback = (_ isConnected: Bool, _ connId: Int, _ addr: Int, _ isOverGatt: Bool, _ error: Int) -> ()
    private var meshNetworkLinkStatusUpdatedCb: [MeshNetworkLinkStatusUpdatedCallback] = []

    /**
     Protype of get component information return callback.

     @param componentName   The name of the mesh component where the componentInfo data returned from.
     @param componentInfo   The component inforamtion returned from the device.
     @param error           Indicates the result of calling the mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then all other arguements are valid.
     Otherwise, all other arguments are invalid and should be ignored.
     */
    public typealias GetMeshComponentInfoCallback = (_ componentName: String, _ componentInfo: String?, _ error: Int) -> ()
    private var getMeshComponentInfoCb: [String : GetMeshComponentInfoCallback] = [:]

    /**
     Protype of mesh device on and off status udpate callback.

     @param deviceName      The name of the mesh device.
     @param isOn            Indicates the power of hte mesh device is ON or OFF.
     @param isPresent       Indicates the mesh device is present or not.
     @param remainingTime   Indicates remain transition time to transfer to new ON/OFF status.
     @param error           Indicates the result of calling mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then all other arguements are valid.
     Otherwise, all other arguments in the callback were invalid and should be ignored.
     */
    public typealias MeshOnOffStatusCallback = (_ deviceName: String, _ isOn: Bool, _ isPresent: Bool, _ remainingTime: UInt32, _ error: Int) -> ()
    private var meshClientOnOffStatusCb: [String: MeshOnOffStatusCallback] = [:]

    /**
     Protype of mesh sensor status or value udpated/changed callback.

     @param deviceName      The name of the mesh sensor device.
     @param propertyId      Indicates the property ID of the updated/changed data.
     @param data            Indicates the updated/changed data depending on the property ID value.
     @param error           Indicates the result of calling mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then all other arguements are valid.
     Otherwise, all other arguments in the callback were invalid and should be ignored.
     */
    public typealias MeshSensorStatusChangedCallback = (_ deviceName: String, _ propertyId: Int, _ data: Data?, _ error: Int) -> ()
    private var meshSensorStatusChangedCallback: [String: MeshSensorStatusChangedCallback] = [:]

    /**
     Protype of connect mesh device status udpate callback.

     @param status          Indicates the connection status to the mesh device. The value of status can be one of follow value.
                                    MeshConstants.MESH_CLIENT_NODE_WARNING_UNREACHABLE = 0
                                    MeshConstants.MESH_CLIENT_NODE_CONNECTED = 1
                                    MeshConstants.MESH_CLIENT_NODE_ERROR_UNREACHABLE = 2
     @param componentName   The name of the mesh device.
     @param error           Indicates the result of calling mesh library API, or timeout, or the procedure result of the mesh operation.

     Note, only when the error value is MeshErrorCode.MESH_SUCCESS, then the status argument are valid.
     Otherwise, all other arguments in the callback were invalid and should be ignored.
     */
    public typealias MeshClientConnectComponentCallback = (_ status: Int, _ componentName: String, _ error: Int) -> ()
    private var componentConnectCb: MeshClientConnectComponentCallback?

    /**
     Initialize the file storage repository that the mesh library will internally generated and automatiaclly managed after initialized.
     This initMeshStorage API must be executed before calling the mesh library API initMeshLibrary.

     @param user                User account email address that uniquely identify a user.
     @param rootStoragePath     Root directory path of the file repository where all mesh library genereated and managed files located.
                                By default, when this parameter is set to nil, the root mesh library repository directory path is App's Docunemts/mesh directory:
                                e.g.: "/private/var/mobile/Containers/Data/Application/XXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/Documents/mesh/"

     @return                    Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func initMeshStorage(forUser user: String, rootStoragePath: String? = nil) -> Int {
        // Set mesh storage path for current user. By default, the root storage path is the Document folder under Home directory.
        if let rootStoragePath = rootStoragePath {
            let error = MeshStorageSettings.shared.setMeshRootStorage(rootStoragePath: rootStoragePath)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: initMeshLibrary, failed to setMeshRootStorage to \(rootStoragePath), error=\(error)")
                return error
            }
        } else {
            // Default mesh root storage is "/private/var/mobile/Containers/Data/Application/XXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/Documents"
            let documentsPath = NSHomeDirectory() + "/Documents"
            let error = MeshStorageSettings.shared.setMeshRootStorage(rootStoragePath: documentsPath)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: initMeshLibrary, failed to setMeshRootStorage to default documentDirectory, \(documentsPath), error=\(error)")
                return error
            }
        }

        let error = MeshStorageSettings.shared.setUserStorage(for: user)
        guard error == MeshErrorCode.MESH_SUCCESS, let fileStorage = MeshStorageSettings.shared.currentUserStoragePath else {
            meshLog("error: initMeshLibrary, failed to setUserStorage path for user, \(user), error=\(error)")
            return error
        }
        meshLog("initMeshStorage, success for user:\(user), storage: \(fileStorage)")
        // Do not remove the below code.
        // It's aimed to initialize the Bluetooth CBCentralManager earler, so to avoid later not initialized error.
        //let _ = MeshGattClient.shared.centralManager.isScanning
        return MeshErrorCode.MESH_SUCCESS
    }

    /**
     Initialize mesh library.
     The mesh library must be initailized fisrtly before calling any mesh APIs, and should only be initialized once before deinitialized.

     @param user    User account email address that uniquely identify a user.

     @return        Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func initMeshLibrary(forUser user: String) -> Int {
        lock.lock()
        if currentUserName != nil {
            lock.unlock()
            return MeshErrorCode.MESH_ERROR_MESH_LIBRARY_HAS_INITIALIZED
        }
        lock.unlock()

        // Set storage path where mesh library will create, read and write files within it.
        if let filesStorage = MeshStorageSettings.shared.currentUserStoragePath {
            let uniqueUuid = UUID(uuidString: self.uniqueId)
            let error = Int(MeshNativeHelper.setFileStorageAtPath(filesStorage, provisionerUuid: uniqueUuid))
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: initMeshLibrary, failed to setFileStorageAtPath path, \(filesStorage), error=\(error)")
                return error
            }
        }

        // Register Callback to mesh core library.
        MeshNativeHelper.getSharedInstance().registerNativeCallback(self)

        // Initialize mesh core as mesh client role.
        MeshNativeHelper.meshClientInit()

        lock.lock()
        currentUserName = user
        lock.unlock()

        #if MESH_DFU_ENABLED
        meshLog("initMeshLibrary MESH_DFU_ENABLED")
        #else
        meshLog("initMeshLibrary MESH_DFU_DISABLED")
        #endif

        // Call CBCentralManager API in advance to for system initailze the CBCentralManager instance as quickly as possible.
        let _ = MeshGattClient.shared.centralManager.isScanning
        return MeshErrorCode.MESH_SUCCESS
    }

    /**
     De-Initialize mesh library.

     @return        None.
     */
    open func deinitMeshLibrary() {
        MeshFrameworkManager.shared.disconnectMeshNetwork { (isConnected: Bool, connId: Int, addr: Int, isOverGatt: Bool, error: Int) in
            MeshFrameworkManager.shared.closeMeshNetwork()
        }

        lock.lock()
        meshDatabaseChangedCb = nil
        meshNetworkLinkStatusUpdatedCb.removeAll()
        getMeshComponentInfoCb.removeAll()
        meshClientOnOffStatusCb.removeAll()
        meshSensorStatusChangedCallback.removeAll()
        componentConnectCb = nil
        componentConnectName = nil

        currentUserName = nil
        openingNetworkName = nil
        openedNetworkName = nil
        openingProvisionerName = nil
        openedProvisionerName = nil
        unreachableMeshDevices.removeAll()
        lock.unlock()
    }

    /**
     Create a mesh network.

     @param provisioinerName    The name of the provisioner which much uniquely identify a user on a device.
     @param networkName         The name of the mesh network to be createdd.

     @return                    Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func createMeshNetwork(provisioinerName: String, networkName: String) -> Int {
        if provisioinerName.isEmpty || networkName.isEmpty {
            return MeshErrorCode.MESH_ERROR_INVALID_ARGS
        }
        MeshFrameworkManager.shared.disconnectMeshNetwork { (isConnected: Bool, connId: Int, addr: Int, isOverGatt: Bool, error: Int) in }
        openingNetworkName = nil
        openedNetworkName = nil
        openingProvisionerName = nil
        openedProvisionerName = nil
        return Int(MeshNativeHelper.meshClientNetworkCreate(provisioinerName, meshName: networkName))
    }

    /**
     Delete a mesh network.

     @param provisioinerName    The name of the provisioner which much uniquely identify a user on a device.
     @param networkName         The name of the mesh network to be deleted. The mesh network must be previousely created by createMeshNetwork API.

     @return                    Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func deleteMeshNetwork(provisioinerName: String, networkName: String) -> Int {
        if provisioinerName.isEmpty || networkName.isEmpty {
            return MeshErrorCode.MESH_ERROR_INVALID_ARGS
        }
        return Int(MeshNativeHelper.meshClientNetworkDelete(provisioinerName, meshName: networkName))
    }

    private var openMeshNetworkCb: OpenMeshNetworkCallback?
    private var openMeshNetworkTimer: Timer?
    /**
     Open a network from the mesh network formated json string, it may the string value exported by meshClientNetworkExport API before.

     @param provisioinerName    The mesh provisioner name that try to import and open the mesh network.
     @param jsonString          Mesh network foramted json string.
     @param completion          Callback routine when the import and open operation was completed.

     @return                    nil if encounters any error, or the real mesh network name parsed from the input json string.

     Note, when meshClientNetworkImport API return the real mesh network name, it doesn't mean the mesh network has been opened successfully.
     When the mesh network opening completed, the callback routine will be invoked, and the opening status can be checked through the status argument.
     */
    open func openMeshNetwork(provisioinerName: String, networkName: String, completion: @escaping OpenMeshNetworkCallback) {
        guard !provisioinerName.isEmpty, !networkName.isEmpty else {
            meshLog("error: MeshFrameworkManager, openMeshNetwork, invalid arguments, provisioinerName:\(provisioinerName), networkName:\(networkName)")
            completion(nil, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_INVALID_ARGS)
            return
        }
        lock.lock()
        guard openMeshNetworkCb == nil else {
            lock.unlock()
            meshLog("error: MeshFrameworkManager, openMeshNetwork, mesh network is busy on opening \"\(String(describing: openingNetworkName))\"")
            completion(nil, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }
        openMeshNetworkCb = completion
        lock.unlock()

        openingProvisionerName = provisioinerName
        openingNetworkName = networkName
        openMeshNetworkTimer?.invalidate()
        openMeshNetworkTimer = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_NETWORK_OPEN_TIMETOUT),
                                                    target: self, selector: #selector(openMeshNetworkTimeoutHandler),
                                                    userInfo: nil, repeats: false)
        let error = Int(MeshNativeHelper.meshClientNetworkOpen(provisioinerName, meshName: networkName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: MeshFrameworkManager, openMeshNetwork, call meshClientNetworkOpen failed, error=\(error)")
            openMeshNetworkTimer?.invalidate()
            openMeshNetworkTimer = nil
            completion(networkName, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, error)
            lock.lock()
            openMeshNetworkCb = nil
            lock.unlock()
            return
        }
        // waiting for network open status callback.
    }
    @objc private func openMeshNetworkTimeoutHandler() {
        meshLog("error: MeshFrameworkManager, openMeshNetwork, waiting for \(MeshConstants.MESH_CLIENT_NETWORK_OPEN_TIMETOUT) seconds timeout error")
        openMeshNetworkTimer?.invalidate()
        openMeshNetworkTimer = nil
        openMeshNetworkCb?(openingNetworkName, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        lock.lock()
        openMeshNetworkCb = nil
        lock.unlock()
        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_OPENNED_CB),
                                        object: nil, userInfo: [MeshNotificationConstants.USER_INFO_KEY_NETWORK_OPENNED_STATUS: MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT])
    }

    open func getOpeningMeshNetworkName() -> String? {
        return openingNetworkName
    }

    open func getOpenedMeshNetworkName() -> String? {
        return openedNetworkName
    }

    open func getOpeningMeshProvisionerName() -> String? {
        return openingProvisionerName
    }

    open func getOpenedMeshProvisionerName() -> String? {
        return openedProvisionerName
    }

    /**
     Close a mesh network if it was opened through openMeshNetwork API.

     @return        None.
     */
    open func closeMeshNetwork() {
        openedNetworkName = nil
        openingNetworkName = nil
        openingProvisionerName = nil
        openedProvisionerName = nil
        MeshNativeHelper.meshClientNetworkClose()
    }

    open func isMeshNetworkExists(networkName: String) -> Bool {
        return (MeshNativeHelper.meshClientNetworkExists(networkName) != 0) ? true : false
    }

    open func isMeshNetworkConnected() -> Bool {
        if MeshFrameworkManager.shared.getOpenedMeshNetworkName() == nil {
            return false
        }
        return MeshNativeHelper.meshClientIsConnectedToNetwork()
    }

    open func isMeshProvisionConnecting() -> Bool {
        return MeshNativeHelper.meshClientIsConnectingProvisioning()
    }

    /**
     Export the whole mesh network's content in json formated string. The exported data can be used as a backup or migrate to anohter provisioner device.

     @param networkName     Indicates which mesh network should be exported.

     @return                    nil when mesh network is busying, or the json formated string that contains the whole content of the mesh network.

     Note, when meshClientNetworkExport return nil, it doesn't indicate any error, it indicates the mesh network and the mesh library is busy doing some, or
     syncing the network keys and data, so it's not suitale to export the mesh network content currently, because it may be changed immediately,
     so when this API return nil, you can try it later.
     It's suggested to call the meshClientNetworkExport API to export the mesh network content on every time the onDatabaseChangedCb callback be invoked.
     */
    open func meshClientNetworkExport(networkName: String) -> String? {
        return MeshNativeHelper.meshClientNetworkExport(networkName)
    }

    /**
     Import and open a network from the mesh network formated json string, it may the string value exported by meshClientNetworkExport API before.

     @param provisioinerName    The mesh provisioner name that try to import and open the mesh network.
     @param jsonString          Mesh network foramted json string.
     @param completion          Callback routine when the import and open operation was completed.

     @return                    nil if encounters any error, or the real mesh network name parsed from the input json string.

     Note, when meshClientNetworkImport API return the real mesh network name, it doesn't mean the mesh network has been opened successfully.
     When the mesh network opening completed, the callback routine will be invoked, and the opening status can be checked through the status argument.
     */
    open func meshClientNetworkImport(provisioinerName: String, jsonString: String, ifxJsonString: String, completion: @escaping OpenMeshNetworkCallback) -> String? {
        guard !provisioinerName.isEmpty, !jsonString.isEmpty else {
            meshLog("error: MeshFrameworkManager, meshClientNetworkImport, invalid arguments, provisioinerName:\(provisioinerName), jsonString:\(jsonString)")
            completion(nil, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_INVALID_ARGS)
            return nil
        }
        lock.lock()
        guard openMeshNetworkCb == nil else {
            lock.unlock()
            meshLog("error: MeshFrameworkManager, openMeshNetwork, mesh network is busy on opening \"\(String(describing: openingNetworkName))\"")
            completion(nil, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return nil
        }
        openMeshNetworkCb = completion
        lock.unlock()

        openMeshNetworkTimer?.invalidate()
        openMeshNetworkTimer = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_NETWORK_OPEN_TIMETOUT),
                                                    target: self, selector: #selector(openMeshNetworkTimeoutHandler),
                                                    userInfo: nil, repeats: false)
        guard let openingMeshNetworkName = MeshNativeHelper.meshClientNetworkImport(provisioinerName, jsonString: jsonString, ifxJsonString: ifxJsonString) else {
            meshLog("error: MeshFrameworkManager, meshClientNetworkImport failed, return openingMeshNetworkName is nil")
            openMeshNetworkTimer?.invalidate()
            openMeshNetworkTimer = nil
            completion(nil, MeshErrorCode.MESH_ERROR_NETWORK_CLOSED, MeshErrorCode.MESH_ERROR_INVALID_STATE)
            lock.lock()
            openMeshNetworkCb = nil
            lock.unlock()
            return nil
        }
        openingNetworkName = openingMeshNetworkName
        openingProvisionerName = provisioinerName
        // waiting for network open status callback.
        return openingMeshNetworkName
    }

    /**
     Try to connect to the currently opened mesh network.
     The mesh client can opearte or control the mesh components in the mesh network only when it has connected to the mesh network.

     @param scanDuration    The maximum scanning duration time in seconds for the mesh client to discover a mesh device to conenct to.
                            If the scan duration time exceeded, then the connection completion callback will be invoked with not connected status.
     @param completion      Callback routine when the import and open operation was completed.

     @return                None.

     Note, when the connectMeshNetwork API was called successfully, the onLinkStatus callback routine will be invoked after completed or scan duration timeout.
     */
    open func connectMeshNetwork(scanDuration: Int = MeshConstants.MESH_DEFAULT_SCAN_DURATION, completion: @escaping MeshNetworkLinkStatusUpdatedCallback) {
        if isMeshNetworkConnected() {
            completion(true, 1, 0, true, MeshErrorCode.MESH_SUCCESS)
            return
        }

        let error = Int(MeshNativeHelper.meshClientConnectNetwork(UInt8(MeshConstants.MESH_DEFAULT_USE_GATT_PROXY), scanDuration: UInt8(scanDuration)))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(false, 0, 0, true, error)
            return
        }

        // After meshClientConnectNetwork() API called success, the onLinkStatus callback will finally be invoked when completed.
        meshNetworkLinkStatusUpdatedCb.append(completion)
    }

    /**
     Try to disconnect from the currently opened mesh network.
     After disconnected fromt the mesh network successfully, the mesh client cannot opearte or control any mesh component any more.

     @param completion      Callback routine when the import and open operation was completed.

     @return                None.

     Note, when the connectMeshNetwork API was called successfully, the onLinkStatus callback routine will be invoked after completed or scan duration timeout.
     */
    open func disconnectMeshNetwork(completion: @escaping MeshNetworkLinkStatusUpdatedCallback) {
        guard isMeshNetworkConnected() else {
            completion(false, 0, 0, true, MeshErrorCode.MESH_SUCCESS)
            return
        }

        let error = Int(MeshNativeHelper.meshClientDisconnectNetwork(UInt8(MeshConstants.MESH_DEFAULT_USE_GATT_PROXY)))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(false, 0, 0, true, error)
            return
        }

        // After disconnected success, the onLinkStatus callback will finally be invoked when completed.
        meshNetworkLinkStatusUpdatedCb.append(completion)
    }

    /**
     Run the handler only when the mesh client has connected to the opened mesh network.
     If the mesh network has been connected currently, this function will try to connect to the mesh network firstly,
     then executing the handler when connected successfully.

     @param scanDuration    The maximum scanning duration time in seconds for the mesh client to discover a mesh device to conenct to.
                            If the scan duration time exceeded, then the connection completion callback will be invoked with not connected status.
     @param handler         The handler to the routine which will be executed when the mesh network has been connected.
                            In error parameter indicates the whether the handler routine has been exectued or not.
                            When the error parameter is set to MeshErrorCode.MESH_SUCCESS, then the handler routine can continue the process.
                            Otherwise, the handler routine should stop the process and check the detail error value.

     @return                None.

     Note, when the connectMeshNetwork API was called successfully, the onLinkStatus callback routine will be invoked after completed or scan duration timeout.
     */
    open func runHandlerWithMeshNetworkConnected(scanDuration: Int = MeshConstants.MESH_DEFAULT_SCAN_DURATION, handler: @escaping (_ error: Int) -> ()) {
        // the mesh network must be opened firstly before connecting to the mesh network
        guard let _ = MeshFrameworkManager.shared.openedNetworkName else {
            meshLog("error: MeshFrameworkManager, runHandlerWithMeshNetworkConnected, no mesh network opened yet.")
            handler(MeshErrorCode.MESH_ERROR_NETWORK_CLOSED)
            return
        }

        if MeshFrameworkManager.shared.isMeshNetworkConnected() {
            handler(MeshErrorCode.MESH_SUCCESS)
            return
        }

        MeshFrameworkManager.shared.connectMeshNetwork(scanDuration: scanDuration) { (isConnected: Bool, connId: Int, addr: Int, isOverGatt: Bool, error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS, MeshFrameworkManager.shared.isMeshNetworkConnected() else {
                meshLog("error: MeshFrameworkManager, runHandlerWithMeshNetworkConnected, connectMeshNetwork failed, isConnected:\(isConnected), error:\(error)")
                handler(MeshErrorCode.MESH_ERROR_NOT_CONNECTED)
                return
            }

            handler(MeshErrorCode.MESH_SUCCESS)
        }
    }

    open func getAllMeshProvisioners() -> [String]? {
        return MeshNativeHelper.meshClientGetAllProvisioners()
    }

    open func getAllMeshNetworks() -> [String]? {
        return MeshNativeHelper.meshClientGetAllNetworks()
    }

    open func isMeshGroup(name: String) -> Bool {
        return MeshNativeHelper.meshClientIsGroup(name);
    }

    /**
     Get all mesh groups that directly create under the specific group.

     @return        String array list of the names of the direct subgroups or nil if no direct subgroup found.

     Note, any groups under any subgroup won't be returned in the result list.
     To get all mesh groups under the specific group, please use the getAllSubGroups API instead.
     To get all top mesh groups within the network, set the value of the groupName paramater to the mesh network name.
     */
    open func getDirectSubGroups(groupName: String) -> [String]? {
        return MeshNativeHelper.meshClientGetAllGroups(groupName)
    }

    /**
     Get all mesh groups under the specific group, including the groups under subgroups and deeper groups.

     @return        String array list of the names of the mesh groups or nil if no group found.

     Note, to get all all mesh groups within the mesh network, please directly use getAllMeshNetworkGroups API instead.
     */
    open func getAllSubGroups(groupName: String) -> [String]? {
        var groups: [String] = []
        if let subGroups = getDirectSubGroups(groupName: groupName) {
            for traverseGroup in subGroups {
                groups.append(traverseGroup)
                if let traveredGroups = getAllSubGroups(groupName: traverseGroup) {
                    groups.append(contentsOf: traveredGroups)
                }
            }
        }
        return groups.isEmpty ? nil : groups
    }

    /**
     Get all mesh groups in current opened mesh network, including subgroups and deeper groups.

     @return        String array list of the names of the mesh groups or nil if no group found.
     */
    open func getAllMeshNetworkGroups() -> [String]? {
        if let networkName = MeshFrameworkManager.shared.getOpenedMeshNetworkName() {
            return getAllMeshNetworkGroups(networkName: networkName)
        }
        return nil
    }

    /**
     Get all mesh groups in the specified mesh network whatever it was openned or not, including subgroups and deeper groups.

     @return        String array list of the names of the mesh groups or nil if no group found.
     */
    open func getAllMeshNetworkGroups(networkName: String) -> [String]? {
        var allGroups: [String] = []
        if let topGroups = MeshNativeHelper.meshClientGetAllGroups(networkName) {
            for topGroup in topGroups {
                allGroups.append(topGroup)
                if let subGroups = getAllSubGroups(groupName: topGroup) {
                    allGroups.append(contentsOf: subGroups)
                }
            }
        }
        return allGroups.isEmpty ? nil : allGroups
    }

    /**
     Get all mesh components that under (subscribed by) the specific group.

     @return        String array list of the names of the directed (subscribed) components or nil if no component found.

     Note, all the components that under (subscribed by) the subgroups or deeper groups won't be returned.
     */
    open func getMeshGroupComponents(groupName: String) -> [String]? {
        return MeshNativeHelper.meshClientGetGroupComponents(groupName)
    }

    /**
     Get all mesh components that under (subscribed by) the specific group, including the components under (subscribed by) subgroups and deeper groups.

     @return        String array list of the names of the (subscribed) components under the mesh group or nil if no component found.
     */
    open func getlAllMeshGroupComponents(groupName: String) -> [String]? {
        var allComponents: [String] = []
        if let components = getMeshGroupComponents(groupName: groupName) {
            allComponents.append(contentsOf: components)
        }
        if let subGroups = getAllSubGroups(groupName: groupName) {
            for group in subGroups {
                if let components = getMeshGroupComponents(groupName: group) {
                    allComponents.append(contentsOf: components)
                }
            }
        }
        allComponents.removeDuplicates()  // remove duplicated devices that may added to multiple groups.
        return allComponents.isEmpty ? nil : allComponents
    }

    open func createMeshGroup(groupName: String, parentGroupName: String = "") -> Int {
        return Int(MeshNativeHelper.meshClientGroupCreate(groupName, parentGroupName: parentGroupName))
    }

    open func deleteMeshGroup(groupName: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientGroupDelete(groupName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(openedNetworkName, error)
            meshDatabaseChangedCb = nil
            return
        }
        // wait for database changed callback.
    }

    open func isMeshGroupName(name: String) -> Bool {
        if let allGroups = getAllMeshNetworkGroups() {
            if allGroups.filter({$0 == name}).count > 0 {
                // Tt's a group name, for group name, always use reliable method.
                return true
            }
        }
        return false
    }

    open func getMeshComponentsByDevice(uuid: UUID) ->[String]? {
        return MeshNativeHelper.meshClientGetDeviceComponents(uuid)
    }

    open func getMeshComponentType(componentName: String) -> Int {
        return Int(MeshNativeHelper.meshClientGetComponentType(componentName))
    }

    open func getMeshTargetMethods(componentName: String) -> [String]? {
        return MeshNativeHelper.meshClientGetTargetMethods(componentName)
    }

    open func getMeshControlMethods(componentName: String) -> [String]? {
        return MeshNativeHelper.meshClientGetControlMethods(componentName)
    }

    private var getMeshComponentInfoTimer: Timer?
    open func getMeshComponentInfo(componentName: String, completion: @escaping GetMeshComponentInfoCallback) {
        if let _ = getMeshComponentInfoCb[componentName], getMeshComponentInfoTimer?.isValid ?? false {
            completion(componentName, nil, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        getMeshComponentInfoCb[componentName] = completion
        getMeshComponentInfoTimer?.invalidate()
        getMeshComponentInfoTimer = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                         target: self, selector: #selector(getMeshComponentInfoTimeoutHandler),
                                                         userInfo: componentName, repeats: false)
        let error = Int(MeshNativeHelper.meshClientGetComponentInfo(componentName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            getMeshComponentInfoCb.removeValue(forKey: componentName)
            completion(componentName, nil, error)
            return
        }
        // wait for get mesh component information callback.
    }
    @objc private func getMeshComponentInfoTimeoutHandler(_ timer: Timer) {
        guard let componentName = timer.userInfo as? String else {
            getMeshComponentInfoTimer?.invalidate()
            getMeshComponentInfoTimer = nil
            return
        }
        meshLog("error: MeshFrameworkManager, getMeshComponentInfo for device: \(componentName) timeout")

        if let completion = getMeshComponentInfoCb[componentName] {
            getMeshComponentInfoCb.removeValue(forKey: componentName)
            completion(componentName, nil, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        }
        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_COMPONENT_INFO_UPDATED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName,
                                                   MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_INFO: "" as Any])
    }

    /**
     Rename a mesh device or a mesh group with a new name.

     @param oldName         The mesh device name currently have in the mesh network.
     @param newName         The new device name that will be udpated to the device with in the mesh network.
     @param completion      Callback routine when the device rename operation was completed.

     @return                None

     Note, the specific oldName will be firslty matched with the group name, if a group name matached, then the new name will be renamed to that group instead of device.
     Until there is no group name matched, then the old name will be matched to device name. And the new name will be applied in a device's name matched.
     After renamed successfully, the mesh network database changed callback routine will be invoked.
     */
    open func meshClientRename(oldName: String, newName: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientRename(oldName, newName: newName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(openedNetworkName, error)
            meshDatabaseChangedCb = nil
            return
        }
        // now, wait database changed event callback.
    }

    /**
     Move a provisioned mesh device from one group into a new group.

     @param componentName   The device name of the target device.
     @param fromGroup       The name of the group where the mesh device will be removed from.
     @param toGroup         The name of the group where the mesh device will be moved into.
     @param completion      Callback roution after the operation of moving mesh component to new group completed.

     @return                None.

     Note, after moved successfully, the mesh network database changed callback routine will be invoked.
     */
    open func moveMeshComponent(componentName: String, fromGroup: String, toGroup: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientMoveComponent(toGroup: componentName, from: fromGroup, to: toGroup))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(openedNetworkName, error)
            meshDatabaseChangedCb = nil
            return
        }
        // now, wait database changed event callback.
    }

    /**
     Add a provisioned mesh device to a specific mesh group.

     @param componentName  The device name of the target device.
     @param groupName      The name of group where the mesh device will be added to.

     @return                None.

     Note, after added successfully, the mesh network database changed callback routine will be invoked.
     */
    open func addMeshComponent(componentName: String, toGroup: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientAddComponent(componentName, toGorup: toGroup))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(openedNetworkName, error)
            meshDatabaseChangedCb = nil
            return
        }
        // now, wait database changed event callback.
    }

    /**
     Get the group names list which the target device belongs to, or the groups current subscried the targed device.
     One mesh device can be added to mulitple groups.

     @param componentName   The device name of the target device.

     @return                Mesh group name array or nil if the mesh device not belong to any group.
     */
    open func getMeshComponentGroupList(componentName: String) -> [String]? {
        return MeshNativeHelper.meshClientGetComponentGroupList(componentName)
    }

    /**
     Remove the mesh device from the specific mesh group only.
     When the device has been added to multiple groups, other groups are not affected.
     If the the specific mesh group is the only group that the target device belong, then after be removed from the group,
     the mesh device can be reterived by getMeshGroupComponents(groupName:) API with the mesh network name.

     @param componentName  The device name of the target device.
     @param groupName      The name of the group where the mesh device will be removed from.

     @return               None.

     Note, after removed successfully, the mesh network database changed callback routine will be invoked.
     */
    private var removeMeshComponentTimer: Timer?
    open func removeMeshComponent(componentName: String, fromGroup: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        removeMeshComponentTimer = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_REMOVE_COMPONENT_FROM_GROUP_TIMEOUT),
                                                        target: self, selector: #selector(removeMeshComponentTimeoutCompleted),
                                                        userInfo: nil, repeats: false)

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientRemoveComponent(componentName, from: fromGroup))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            removeMeshComponentTimer?.invalidate()
            removeMeshComponentTimer = nil

            meshDatabaseChangedCb = nil
            completion(openedNetworkName, error)
            return
        }
        // now, wait database changed event callback.
    }
    @objc func removeMeshComponentTimeoutCompleted() {
        removeMeshComponentTimer?.invalidate()
        removeMeshComponentTimer = nil

        if let completion = meshDatabaseChangedCb {
            meshDatabaseChangedCb = nil
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
            return
        }
    }

    /**
     Delete the provisioned device from current mesh network.
     After the delete operation done successfully, the device will advertise mesh provisioning service data gain.
     so, it can be provisioned and added to other network or group again.

     @param deviceName      The device name of the target device which will be removed from the mesh network.

     @return                None.

     Note, after removed successfully, the mesh network database changed callback routine will be invoked.
     */
    open func meshClientDeleteDevice(deviceName: String, completion: @escaping MeshDatabaseChangedCallback) {
        guard meshDatabaseChangedCb == nil else {
            completion(openedNetworkName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshDatabaseChangedCb = completion
        let error = Int(MeshNativeHelper.meshClientResetDevice(deviceName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            completion(openedNetworkName, error)
            meshDatabaseChangedCb = nil
            return
        }

        /* Now, wait database changed event callback.
         *
         * TODO(Dudley): currently after reset device, if there is any unresponable devices, then the database changed callback routine won't be executed long time.
         *               Because if there is any unrespondable devies, then mesh network KR process (Key Refresh) will by implemented on every connectioin establshed,
         *               it's aimed to udpate all keys to all devies within the network. And only all devices has been udpated, then the deleted device will be
         *               really removed from the json file and mesh network. So, only at that time, the database changed callback will be invoked.
         *               So, currently, call the completion callback directly after send the reset device command successfully.
         */
        completion(openedNetworkName, MeshErrorCode.MESH_SUCCESS)
        meshDatabaseChangedCb = nil
    }

    open func setMeshDeviceConfiguration(deviceName: String = "",
                                         isGattProxy: Int = MeshConstants.MESH_DEFAULT_IS_GATT_PROXY,
                                         isFriend: Int = MeshConstants.MESH_DEFAULT_IS_FRIEND,
                                         isRelay: Int = MeshConstants.MESH_DEFAULT_IS_RELAY,
                                         beacon: Int = MeshConstants.MESH_DEFAULT_BEACON,
                                         relayXmitCount: Int = MeshConstants.MESH_DEFAULT_RELAY_XMIT_COUNT,
                                         relayXmitInterval: Int = MeshConstants.MESH_DEFAULT_RELAY_XMIT_INTERVAL,
                                         defaultTtl: Int = MeshConstants.MESH_DEFAULT_TTL,
                                         netXmitCount: Int = MeshConstants.MESH_DEFAULT_NET_XMIT_COUNT,
                                         netXmitInterval: Int = MeshConstants.MESH_DEFAULT_NET_XMIT_INTERVAL) -> Int {
        return Int(MeshNativeHelper.meshClientSetDeviceConfig(deviceName, isGattProxy: Int32(isGattProxy), isFriend: Int32(isFriend),
                                                              isRelay: Int32(isRelay), beacon: Int32(beacon), relayXmitCount: Int32(relayXmitCount),
                                                              relayXmitInterval: Int32(relayXmitInterval), defaultTtl: Int32(defaultTtl),
                                                              netXmitCount: Int32(netXmitCount), netXmitInterval: Int32(netXmitInterval)))
    }

    open func setMeshPublicationConfiguration(publishCredentialFlag: Int = MeshConstants.MESH_DEFAULT_PUBLISH_CREDENTIAL_FLAG,
                                              publishRetransmitCount: Int = MeshConstants.MESH_DEFAULT_RETRANSMIT_COUNT,
                                              publishRetransmitInterval: Int = MeshConstants.MESH_DEFAULT_RETRANSMIT_INTERVAL,
                                              publishTtl: Int = MeshConstants.MESH_DEFAULT_PUBLISH_TTL) -> Int {
        return Int(MeshNativeHelper.meshClientSetPublicationConfig(Int32(publishCredentialFlag),
                                                                   publishRetransmitCount: Int32(publishRetransmitCount),
                                                                   publishRetransmitInterval: Int32(publishRetransmitInterval),
                                                                   publishTtl: Int32(publishTtl)))
    }

    open func configureMeshPublication(componentName: String, isClient: Bool, method: String, targetName: String, publishPeriod: Int = MeshConstants.MESH_DEFAULT_PUBLISH_PERIOD) -> Int {
        return Int(MeshNativeHelper.meshClientConfigurePublication(componentName, isClient: isClient ? 1 : 0, method: method, targetName: targetName, publishPeriod: Int32(publishPeriod)))
    }

    /**
     Provisioning and add an unprovisioned device into the mesh network.

     @param deviceName          The name of the unprovisioned mesh device. Can be reterieved on onDeviceFound callback.
     @param uuid                The unique UUID that uniquely identifies the unprovisioned mesh device. Can be reterieved on onDeviceFound callback.
     @param groupName           The group name which the unprovisioned mesh device will be added to after it was provisioned and added succefully.
     @param identifyDuration    The maximum time to wait for the unprovisoned mesh device be found again during provsioning process.

     @return                    MeshErrorCode.MESH_SUCCESS on started provsioning success; otherwise, indicates encounterred some error, and provisioning has been stopped.

     Note, during provisioning process, the meshClientProvisionCompletedCb callback will be invoked multiple times whith the corresponding provsioning status set.
     Normally, a successfully provsioning process will receive provsioning status in the sequence of 2,3,6,4,5.
     After status 5 received, the provsioning was done succesfully.
     But if status 0 was received, then indicates the provsioining has been finished was some errors, and the provisioning has been stopped.
     During provisioning process, the mesh library will try 3 times if any error detected, so the maximum provisioning time is 3 * identifyDuration seconds.

     Also, after provisioning and added the mesh device successfully, the mesh network database changed callback routine will be invoked.
     */
    private var provisionTimer: Timer?
    private var provisionUuid: UUID?
    open func meshClientProvision(deviceName: String, uuid: UUID, groupName: String,
                                  identifyDuration: Int = MeshConstants.MESH_CLIENT_PROVISION_IDENTIFY_DURATION) -> Int {
        meshLog("meshClientProvision, deviceName=\(deviceName), uuid=\(uuid.description), groupName=\(groupName), identifyDuration=\(identifyDuration)")
        lock.lock()
        if let _ = provisionUuid {
            lock.unlock()
            meshLog("error: meshClientProvision, called again before previously provisioning is completed")
            return MeshErrorCode.MESH_ERROR_API_IS_BUSYING
        }
        provisionUuid = uuid
        lock.unlock()

        var error = setMeshDeviceConfiguration()
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientProvision, failed to setMeshDeviceConfiguration, error=\(error)")
            lock.lock()
            provisionUuid = nil
            lock.unlock()
            return error
        }

        error = setMeshPublicationConfiguration()
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientProvision, failed to setMeshPublicationConfiguration, error=\(error)")
            lock.lock()
            provisionUuid = nil
            lock.unlock()
            return error
        }

        error = Int(MeshNativeHelper.meshClientProvision(deviceName, groupName: groupName, uuid: uuid, identifyDuration: UInt8(identifyDuration)))
        if error != MeshErrorCode.MESH_SUCCESS {
            meshLog("error: meshClientProvision, failed to call meshClientProvision, error=\(error)")
            provisionTimer?.invalidate()
            provisionTimer = nil
            lock.lock()
            provisionUuid = nil
            lock.unlock()
        }
        return error
    }
    @objc func meshClientProvisionTimeoutCompleted() {
        lock.lock()
        if let uuid = provisionUuid {
            provisionUuid = nil
            lock.unlock()
            meshLog("error: meshClientProvisionTimeoutCompleted, timeout")
            meshClientProvisionCompletedCb(UInt8(MeshConstants.MESH_CLIENT_PROVISION_STATUS_FAILED), uuid: uuid)
            return
        }
        lock.unlock()
    }

    /**
     Send Generic OnOff Get Acknowledged/Unacknowledged message to target device to reterieve ON/OFF status.
     See <<Mesm Model>> spec, section 3.2.1 Generic OnOff messages.

     @param deviceName      The componenent name of the target device in the mesh network.

     @return                Mesh error code, see definitions in MeshErrorCode for more details.

     Note, when the mesh client ON/OFF get operation was completed, the meshClient(onOffSet callback routine will be invoked.
     */
    open func meshClientOnOffGet(deviceName: String) -> Int {
        return Int(MeshNativeHelper.meshClient(onOffGet: deviceName))
    }
    open func meshClientOnOffGet(deviceName: String, completion: @escaping MeshOnOffStatusCallback) {
        guard meshClientOnOffStatusCb[deviceName] == nil else {
            meshLog("error: meshClientOnOffGet, command has been sent to device:\(deviceName), busying")
            completion(deviceName, false, false, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientOnOffStatusCb[deviceName] = completion
        let error = Int(MeshNativeHelper.meshClient(onOffGet: deviceName))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientOnOffGet, failed to call meshClient(onOffGet with deviceName:\(deviceName), error:\(error)")
            completion(deviceName, false, false, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    /**
     Send Generic OnOff Set Acknowledged/Unacknowledged message to target device. See <<Mesm Model>> spec, section 3.2.1 Generic OnOff messages.

     @param deviceName      The componenent name of the target device in the mesh network.
     @param isOn            The target value of the Generic OnOff sate.
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
     @param transitionTime  The time to transitioin to the target state from the present state. See <<Mesh Model>> spec section 3.1.3 for the data format.
     @param delay           Message execution delay in millisecond, should be multiple of 5 millisecond (5 millisecond interval for each step).

     @return                Mesh error code, see definitions in MeshErrorCode for more details.

     Note, when the mesh client ON/OFF set operation was completed, the meshClient(onOffSet callback routine will be invoked when the send out command was
     an acknowledged command type.
     If the device name is group name, and reliable is true, the component under the group will have the callback invoked when completed,
     but the group name itself won't have the callback invoked with its name.
     */
    open func meshClientOnOffSet(deviceName: String, isOn: Bool, reliable: Bool,
                                 transitionTime: UInt32 = MeshConstants.MESH_DEFAULT_TRANSITION_TIME,
                                 delay: Int = MeshConstants.MESH_DEFAULT_ONOFF_DELAY) -> Int {
        return Int(MeshNativeHelper.meshClient(onOffSet: deviceName, onoff: isOn ? 1 : 0, reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
    }
    open func meshClientOnOffSet(deviceName: String, isOn: Bool, reliable: Bool,
                                 transitionTime: UInt32 = MeshConstants.MESH_DEFAULT_TRANSITION_TIME,
                                 delay: Int = MeshConstants.MESH_DEFAULT_ONOFF_DELAY,
                                 completion: @escaping MeshOnOffStatusCallback) {
        guard meshClientOnOffStatusCb[deviceName] == nil else {
            meshLog("error: meshClientOnOffSet, command has been sent to device:\(deviceName), busying")
            completion(deviceName, false, false, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        var isReliable = reliable
        if MeshFrameworkManager.shared.isMeshGroupName(name: deviceName) {
            isReliable = false
        }
        if isReliable {
            meshClientOnOffStatusCb[deviceName] = completion
        }
        let error = Int(MeshNativeHelper.meshClient(onOffSet: deviceName, onoff: isOn ? 1 : 0, reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientOnOffSet, failed to call meshClient(onOffSet with deviceName:\(deviceName), error:\(error)")
            completion(deviceName, false, false, 0, error)
            return
        }

        // for unreliable command sent, the completion routine will be executed immediately, because no completion callback will be called.
        guard isReliable else {
            meshLog("meshClientOnOffSet, send unreliable meshClient(onOffSet command successfully to deviceName:\(deviceName)")
            completion(deviceName, isOn, true, transitionTime, MeshErrorCode.MESH_SUCCESS)
            return
        }
        // wait for completion callback. Only reliable command will have the callback routine invoked when completed.
    }

    open func meshClientLevelGet(deviceName: String) -> Int {
        return Int(MeshNativeHelper.meshClientLevelGet(deviceName))
    }

    /**
     Send Generic Level Set Acknowledged/Unacknowledged message to target device. See <<Mesm Model>> spec, section 3.2.2 Generic Level messages.

     @param deviceName      The componenent name of the target device in the mesh network.
     @param level           The target value of the Generic Level sate.
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
     @param transitionTime  The time to transitioin to the target state from the present state. See <<Mesh Model>> spec section 3.1.3 for the data format.
     @param delay           Message execution delay in millisecond, should be multiple of 5 millisecond (5 millisecond interval for each step).

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func meshClientLevelSet(deviceName: String, level: Int, reliable: Bool, transitionTime: UInt32, delay: Int) -> Int {
        if TrackingHelper.shared.isTracking {
            TrackingHelper.shared.levelSetMessage(componentName: deviceName, level: level, transitionTime: transitionTime, delay: delay)
            return MeshErrorCode.MESH_SUCCESS
        }
        return Int(MeshNativeHelper.meshClientLevelSet(deviceName, level: Int16(level), reliable: reliable,
                                                       transitionTime: transitionTime, delay: UInt16(delay)))
    }

    open func meshClientHslGet(deviceName: String) -> Int {
        return Int(MeshNativeHelper.meshClientHslGet(deviceName))
    }

    /**
     Send Light HSL Set Acknowledged/Unacknowledged message to target device. See <<Mesm Model>> spec, section 6.3.3 Light HSL messages.

     @param deviceName      The componenent name of the target device in the mesh network.
     @param lightness       The target value of the Light HSL Lightness sate.
     @param hue             The target value of the Light HSL Hue sate.
     @param saturation      The target value of the Light HSL Saturation sate.
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
     @param transitionTime  The time to transitioin to the target state from the present state. See <<Mesh Model>> spec section 3.1.3 for the data format.
     @param delay           Message execution delay in millisecond, should be multiple of 5 millisecond (5 millisecond interval for each step).

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func meshClientHslSet(deviceName: String, lightness: Int, hue: Int, saturation: Int, reliable: Bool, transitionTime: UInt32, delay: Int) -> Int {
        if TrackingHelper.shared.isTracking {
            TrackingHelper.shared.hslSetMessage(componentName: deviceName, lightness: lightness, hue: hue, saturation: saturation, transitionTime: transitionTime, delay: delay)
            return MeshErrorCode.MESH_SUCCESS
        }
        return Int(MeshNativeHelper.meshClientHslSet(deviceName, lightness: UInt16(lightness), hue: UInt16(hue), saturation: UInt16(saturation),
                                                     reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
    }

    open func meshClientLightnessGet(deviceName: String) -> Int {
        return Int(MeshNativeHelper.meshClientLightnessGet(deviceName))
    }

    /**
     Send Light Lightness Set Acknowledged/Unacknowledged message to target device. See <<Mesm Model>> spec, section 6.3.1 Light Lightness messages.

     @param deviceName      The componenent name of the target device in the mesh network.
     @param lightness       The target value of the Light Lightness Actual sate.
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
     @param transitionTime  The time to transitioin to the target state from the present state. See <<Mesh Model>> spec section 3.1.3 for the data format.
     @param delay           Message execution delay in millisecond, should be multiple of 5 millisecond (5 millisecond interval for each step).

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func meshClientLightnessSet(deviceName: String, lightness: Int, reliable: Bool, transitionTime: UInt32, delay: Int) -> Int {
        if TrackingHelper.shared.isTracking {
            TrackingHelper.shared.lightnessSetMessage(componentName: deviceName, lightness: lightness, transitionTime: transitionTime, delay: delay)
            return MeshErrorCode.MESH_SUCCESS
        }
        return Int(MeshNativeHelper.meshClientLightnessSet(deviceName, lightness: UInt16(lightness),
                                                           reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
    }

    open func meshClientCtlGet(deviceName: String) -> Int {
        return Int(MeshNativeHelper.meshClientCtlGet(deviceName))
    }

    /**
     Send Light CTL Set Acknowledged/Unacknowledged message to target device. See <<Mesm Model>> spec, section 6.3.2 Light CTL messages.

     @param deviceName      The componenent name of the target device in the mesh network.
     @param lightness       The target value of the Light CTL Lightness sate.
     @param temperature     The target value of the Light CTL Temperature sate.
     @param deltaUv         The target value of the Light CTL Delta UV sate.
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
     @param transitionTime  The time to transitioin to the target state from the present state. See <<Mesh Model>> spec section 3.1.3 for the data format.
     @param delay           Message execution delay in millisecond, should be multiple of 5 millisecond (5 millisecond interval for each step).

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func meshClientCtlSet(deviceName: String, lightness: Int, temperature: Int, deltaUv: Int,
                               reliable: Bool, transitionTime: UInt32, delay: Int) -> Int {
        if TrackingHelper.shared.isTracking {
            TrackingHelper.shared.ctlSetMessage(componentName: deviceName, lightness: lightness, temperature: temperature, deltaUv: deltaUv, transitionTime: transitionTime, delay: delay)
            return MeshErrorCode.MESH_SUCCESS
        }
        return Int(MeshNativeHelper.meshClientCtlSet(deviceName, lightness: UInt16(lightness), temperature: UInt16(temperature), deltaUv: UInt16(deltaUv),
                                                     reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
    }

    open func meshClientVendorDataSet(deviceName: String,
                                      companyId: Int = MeshConstants.MESH_VENDOR_COMPANY_ID,
                                      modelId: Int = MeshConstants.MESH_VENDOR_MODEL_ID,
                                      opcode: Int,
                                      disable_ntwk_retransmit: Int = MeshConstants.MESH_VENDOR_DISABLE_NETWORK_RETRANSMIT,
                                      data: Data) -> Int {
        return Int(MeshNativeHelper.meshClientVendorDataSet(deviceName, companyId: UInt16(companyId), modelId: UInt16(modelId), opcode: UInt8(opcode),  disable_ntwk_retransmit: UInt8(disable_ntwk_retransmit), data: data))
    }

    /**
     Send Identify message to a device or a group of devices to identify itself for a certain duration. See <<Mesh Profile>> section 4.2.9 Attention Timer.

     @param name            The componenent name of the target device or the group name of the target devices' group.
     @param duration        The duration time in seconds, determines how long the element shall remain attracting human's attention.

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func meshClientIdentify(name: String, duration: Int = 10) -> Int {
        return Int(MeshNativeHelper.meshClientIdentify(name, duration: UInt8(duration)))
    }

    /**
     Start or stop LE scanning for unprovisioned mesh devices.
     Listoning for the Mesh Provisioning UUID and Mesh Proxy UUID in the LE advertisement data.

     @param startStop       true - start scanning; false - stop scanning.

     @return                None.
     */
    open func meshClientScanUnprovisionedDevice(start: Bool, uuid: Data? = nil) {
        MeshNativeHelper.meshClientScanUnprovisioned(start ? 1 : 0, uuid: uuid)
    }

    /**
     When application manages the connection, it can notify GATT Client that connection has been established or disconnected.
     Currently, with the support of the mesh library, only one connection can be established at any time between the provisioner and target mesh device.

     @param connId          Application managed Connection ID of the GATT connection.

     @return                None.
     */
    open func meshClientConnectionStateChanged(connId: Int) {
        // avoid mesh library crash issue when the mesh library not initalized yet. And it's meaningless for mesh library when the mesh network not opened.
        if let _ = MeshFrameworkManager.shared.getOpenedMeshNetworkName() {
            MeshNativeHelper.meshClientConnectionStateChanged(UInt16(connId), mtu: (connId != 0) ? UInt16(MeshFrameworkManager.shared.mtuSize) : 0)
        }
    }

    open func isMeshProvisioningServiceAdvertisementData(advertisementData: [String : Any]) -> Bool {
        return MeshNativeHelper.isMeshProvisioningServiceAdvertisementData(advertisementData)
    }

    open func isMeshProxyServiceAdvertisementData(advertisementData: [String : Any]) -> Bool {
        return MeshNativeHelper.isMeshProxyServiceAdvertisementData(advertisementData)
    }

    open func isMeshAdvertisementData(advertisementData: [String : Any]) -> Bool {
        return MeshNativeHelper.isMeshAdvertisementData(advertisementData)
    }

    open func meshClientAdvertisementDataReport(peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) {
        guard  let rawAdvData = MeshNativeHelper.getConvertedRawMeshAdvertisementData(peripheral, advertisementData: advertisementData, rssi: rssi) else {
            meshLog("error: MeshFrameworkManager, meshClientAdvertisementDataReport, invalid advertisementData=\(advertisementData) received, failed to convert")
            return
        }
        let bdAddr = MeshNativeHelper.getMeshPeripheralMappedBdAddr(peripheral)
        MeshNativeHelper.meshBdAddrDictAppend(bdAddr, peripheral: peripheral)
        MeshNativeHelper.meshClientLog("[MeshFrameworkManager meshClientAdvertisementDataReport] bdAddr: \(bdAddr.dumpHexBytes()), name:\(peripheral.name ?? "nil"), rssi:\(rssi.int8Value), advData count:\(rawAdvData.count), advData:\(rawAdvData.dumpHexBytes())")
        MeshNativeHelper.meshClientAdvertReport(bdAddr, addrType: MeshConstants.MESH_DEFAULT_PERIPHERAL_BDADDR_TYPE,
                                                rssi: rssi.int8Value, advData: rawAdvData)
    }

    private var componentConnectTimer: Timer?
    private var componentConnectName: String?
    open func meshClientConnectComponent(componentName: String,
                                         scanDuration: Int = MeshConstants.MESH_DEFAULT_COMPONENT_CONNECT_SCAN_DURATION,
                                         useProxy: Bool = MeshConstants.MESH_DEFAULT_COMPONENT_CONNECT_USE_PROXY,
                                         completion: @escaping MeshClientConnectComponentCallback) {
        lock.lock()
        if let _ = componentConnectCb {
            lock.unlock()
            meshLog("error: MeshFrameworkManager, meshClientConnectComponent, connectComponent is busying")
            completion(MeshConstants.MESH_CLIENT_NODE_WARNING_UNREACHABLE, componentName, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }
        componentConnectCb = completion
        componentConnectName = componentName
        componentConnectTimer?.invalidate()
        componentConnectTimer = Timer.scheduledTimer(timeInterval: TimeInterval(scanDuration + MeshConstants.MESH_CLIENT_CONNECT_TIMETOUT),
                                                     target: self, selector: #selector(componentConnectTimeoutCompleted),
                                                     userInfo: nil, repeats: false)
        lock.unlock()

        let error = Int(MeshNativeHelper.meshConnectComponent(componentName, useProxy: useProxy ? 1 : 0, scanDuration: UInt8(scanDuration)))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            lock.lock()
            componentConnectCb = nil
            componentConnectName = nil
            componentConnectTimer?.invalidate()
            componentConnectTimer = nil
            lock.unlock()
            meshLog("error: MeshFrameworkManager, meshClientConnectComponent, componentName:\(componentName), error=\(error)")
            completion(MeshConstants.MESH_CLIENT_NODE_WARNING_UNREACHABLE, componentName, error)
            return
        }

        // wait for meshClientNodeConnectStateCb event callback.
    }
    @objc func componentConnectTimeoutCompleted() {
        lock.lock()
        meshLog("error: MeshFrameworkManager, meshClientConnectComponent, componentConnectTimeoutCompleted, componentName:\(String(describing: componentConnectName))")
        if let completion = componentConnectCb, let componentName = componentConnectName {
            componentConnectCb = nil
            componentConnectName = nil
            componentConnectTimer?.invalidate()
            componentConnectTimer = nil
            lock.unlock()
            completion(MeshConstants.MESH_CLIENT_NODE_ERROR_UNREACHABLE, componentName, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
            return
        }
        componentConnectCb = nil
        componentConnectName = nil
        lock.unlock()
    }

    open func sendReceivedProxyPacketToMeshCore(data: Data) {
        MeshNativeHelper.sendRxProxyPkt(toCore: data)
    }

    open func sendReceivedProvisionPacketToMeshCore(data: Data) {
        MeshNativeHelper.sendRxProvisPkt(toCore: data)
    }

    open func meshClientSetGattMtuSize(mtu: Int = MeshFrameworkManager.shared.mtuSize) {
        MeshNativeHelper.meshClientSetGattMtu(Int32(mtu))
    }

    open func meshClientOtaDataEncrypt(componenetName: String = "temp", data: Data) -> Data? {
        return MeshNativeHelper.meshClientOTADataEncrypt(componenetName, data: data)
    }

    open func meshClientOtaDataDecrypt(componenetName: String = "temp", data: Data) -> Data? {
        return MeshNativeHelper.meshClientOTADataDecrypt(componenetName, data: data)
    }

    open func getUnreachableMeshDevices() -> [String]? {
        return (unreachableMeshDevices.count > 0) ? unreachableMeshDevices : nil
    }

    //////////////////////////////////////////////////////////////////////////
    /// MARK: functions for mesh storage directory operations.
    //////////////////////////////////////////////////////////////////////////

    public func getProvisionerUuidFileName() -> String {
        return MeshNativeHelper.getProvisionerUuidFileName()
    }

    public func isProvisionerUuidFileExits() -> Bool {
        return FileManager.default.fileExists(atPath: getProvisionerUuidFileName())
    }

    open func getUserMeshFileNameList() -> [String]? {
        return MeshStorageSettings.shared.currentAllFileNameUnderUserStorage
    }

    open func getUserMeshFilePathList() -> [String]? {
        return MeshStorageSettings.shared.currentAllFilePathUnderUserStorage
    }

    open func getMeshStoragePath() -> String? {
        return MeshStorageSettings.shared.currentMeshStoragepath
    }

    open func getUserStoragePath() -> String? {
        return MeshStorageSettings.shared.currentUserStoragePath
    }

    /**
     Delete all mesh files and folder under current user's mesh storage directory.

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func deleteUserStorage() -> Int {
        if let userName = currentUserName {
            return MeshStorageSettings.shared.deleteUserStorage(for: userName)
        }
        return MeshErrorCode.MESH_ERROR_INVALID_USER_NAME
    }

    /**
     Delete all mesh user storages under the root mesh storage directory.

     @return                Mesh error code, see definitions in MeshErrorCode for more details.
     */
    open func deleteMeshStorage() -> Int {
        return MeshStorageSettings.shared.deleteMeshStorage()
    }

    open func restoreMeshFile(fileName: String, content: Data?) -> Bool {
        return MeshStorageSettings.shared.restoreMeshFile(fileName: fileName, content: content)
    }

    open func readMeshFile(fileName: String) -> Data? {
        return MeshStorageSettings.shared.readMeshFile(fileName: fileName)
    }

    open var mtuSize: Int {
        return PlatformManager.SYSTEM_MTU_SIZE
    }

    //
    // DFU APIs
    //

    /**
     Send command to the DFU distributor device to report DFU status in every specific internal time, or stop the report the report.

     @param interval                Indicates the internval time that the mesh library should  get the DFU status from the DFU distributor device, and report to
                        When the internval time is set to 0, then the DFU status reporting will be cancelled and stopped..

     @return        If get MESH_SUCCESS, the mesh library has been started to report the DFU status or the reporting has been stop report;
              Otherwise, the return code indicates the error code that caused the failure.
     */
    private var meshClientGetStatusInterval: Int = 0        // 0 - indicates the DFU get status is not running, othewise it has running.
    public var isDfuStatusReportingEnabled: Bool {
        get {
            return (meshClientGetStatusInterval == 0) ? false : true
        }
    }
    open func meshClientDfuGetStatus(interval: Int = Int(DFU_DISTRIBUTION_STATUS_TIMEOUT)) -> Int {
        var ret = MeshErrorCode.MESH_SUCCESS
        self.lock.lock()
        if meshClientGetStatusInterval == interval {
            self.lock.unlock()
            // The DFU get status has running or stopped already, return early.
            return MeshErrorCode.MESH_SUCCESS
        } else {
            #if MESH_DFU_ENABLED
            ret = Int(MeshNativeHelper.meshClientDfuGetStatus(Int32(interval)))
            #else
            ret = MeshErrorCode.MESH_ERROR_SERVICE_NOT_SUPPORT
            #endif
            if (ret == MeshErrorCode.MESH_SUCCESS) {
                meshClientGetStatusInterval = interval
            }
        }
        self.lock.unlock()
        return ret;
    }

    open func meshClientDfuStart(dfuMethod: Int, firmwareId: Data, metadata: Data) -> Int {
        #if MESH_DFU_ENABLED
        return Int(MeshNativeHelper.meshClientDfuStart(Int32(dfuMethod), firmwareId: firmwareId, metadata: metadata))
        #else
        return Int(MeshErrorCode.MESH_ERROR_SERVICE_NOT_SUPPORT)
        #endif
    }

    open func meshClientDfuStop() -> Int {
        #if MESH_DFU_ENABLED
        return Int(MeshNativeHelper.meshClientDfuStop())
        #else
        return Int(MeshErrorCode.MESH_ERROR_SERVICE_NOT_SUPPORT)
        #endif
    }

    open func meshClientDfuOtaFinish(status: Int) {
        #if MESH_DFU_ENABLED
        MeshNativeHelper.meshClientDfuOtaFinish(Int32(status))
        #endif
    }

    //
    // Sensor APIs
    //

    /**
     Get the sensor cadence parameters for a sensor which controls the cadence of sensor reports.

     @param deviceName                  Indicates the mesh sensor device.
     @param propertyId                  Indicates a property ID within a sensor.

     @return                            If get success, return the tuple value all sensor cadence parameters; otherwise return nil indicates failure;
     The value of the returned tuple value contains following values:
            @param fastCadencePeriodDivisor    Indicates the divisor for the publish period.
            @param triggerType                 Defines the unit and format of the triggerDeltaDown and triggerDeltaUp parameters.
            @param triggerDeltaDown            Delta dwon value that triggers a status message.
            @param triggerDeltaUp              Delta up value that triggers a status message.
            @param minInterval                 Minimum interval between two consecutive status messages.
            @param fastCadenceLow              Low value for the fast cadence range.
            @param fastCadenceHigh             High value for the fast cadence range.
     */
    open func meshClientSensorCadenceGet(deviceName: String, propertyId: Int) -> (fastCadencePeriodDivisor: Int, triggerType: Int, triggerDeltaDown: Int, triggerDeltaUp: Int, minInterval: Int, fastCadenceLow: Int, fastCadenceHigh: Int)? {

        var fastCadencePeriodDivisor: Int32 = Int32(MeshConstants.SENSOR_FAST_CADENCE_PERIOD_DIVISOR)
        var triggerType: Int32 = Int32(MeshConstants.SENSOR_TRIGGER_TYPE)
        var triggerDeltaDown: Int32 = Int32(MeshConstants.SENSOR_TRIGGER_DELTA_DOWN)
        var triggerDeltaUp: Int32 = Int32(MeshConstants.SENSOR_TRIGGER_DELTA_UP)
        var minInterval: Int32 = Int32(MeshConstants.SENSOR_MIN_INTERVAL)
        var fastCadenceLow: Int32 = Int32(MeshConstants.SENSOR_FAST_CADENCE_LOW)
        var fastCadenceHigh: Int32 = Int32(MeshConstants.SENSOR_FAST_CADENCE_HIGH)
        let error = Int(MeshNativeHelper.meshClientSensorCadenceGet(deviceName, propertyId: Int32(propertyId),
                                                                    fastCadencePeriodDivisor: &fastCadencePeriodDivisor,
                                                                    triggerType: &triggerType,
                                                                    triggerDeltaDown: &triggerDeltaDown,
                                                                    triggerDeltaUp: &triggerDeltaUp,
                                                                    minInterval: &minInterval,
                                                                    fastCadenceLow: &fastCadenceLow,
                                                                    fastCadenceHigh: &fastCadenceHigh))
        guard error == MeshErrorCode.MESH_SUCCESS else {
            return nil
        }

        return (Int(fastCadencePeriodDivisor), Int(triggerType), Int(triggerDeltaDown), Int(triggerDeltaUp), Int(minInterval), Int(fastCadenceLow), Int(fastCadenceHigh))
    }

    /**
     Set the sensor cadence parameters for a sensor, it controls the cadence of sensor reports.

     @param deviceName                  Indicates the mesh sensor device.
     @param propertyId                  Indicates a property ID within a sensor.
     @param fastCadencePeriodDivisor    Indicates the divisor for the publish period.
     @param triggerType                 Defines the unit and format of the triggerDeltaDown and triggerDeltaUp parameters.
     @param triggerDeltaDown            Delta dwon value that triggers a status message.
     @param triggerDeltaUp              Delta up value that triggers a status message.
     @param minInterval                 Minimum interval between two consecutive status messages.
     @param fastCadenceLow              Low value for the fast cadence range.
     @param fastCadenceHigh             High value for the fast cadence range.

     @return                            Return MeshErrorCode.MESH_SUCCESS if set on success; otherwise failed;
     */
    open func meshClientSensorCadenceSet(deviceName: String, propertyId: Int,
                                         fastCadencePeriodDivisor: Int = MeshConstants.SENSOR_FAST_CADENCE_PERIOD_DIVISOR,
                                         triggerType: Int = MeshConstants.SENSOR_TRIGGER_TYPE,
                                         triggerDeltaDown: Int = MeshConstants.SENSOR_TRIGGER_DELTA_DOWN,
                                         triggerDeltaUp: Int = MeshConstants.SENSOR_TRIGGER_DELTA_UP,
                                         minInterval: Int = MeshConstants.SENSOR_MIN_INTERVAL,
                                         fastCadenceLow: Int = MeshConstants.SENSOR_FAST_CADENCE_LOW,
                                         fastCadenceHigh: Int = MeshConstants.SENSOR_FAST_CADENCE_HIGH) -> Int {
        return Int(MeshNativeHelper.meshClientSensorCadenceSet(deviceName, propertyId: Int32(propertyId), fastCadencePeriodDivisor: Int32(fastCadencePeriodDivisor), triggerType: Int32(triggerType), triggerDeltaDown: Int32(triggerDeltaDown), triggerDeltaUp: Int32(triggerDeltaUp), minInterval: Int32(minInterval), fastCadenceLow: Int32(fastCadenceLow), fastCadenceHigh: Int32(fastCadenceHigh)))
    }

    /**
     Get all support setting property ID in a list for a sensor.

     @param componentName       Indicates the mesh sensor device.
     @param propertyId          Indicates a property ID within a sensor.

     @return                    Return the setting propery ID list in get succeess, or nil if failed to get the list;
     */
    open func meshClientSensorSettingGetPropertyIds(componentName: String, propertyId: Int) -> [Int]? {
        guard let propertyIdsData = MeshNativeHelper.meshClientSensorSettingGetPropertyIds(componentName, propertyId: Int32(propertyId)) else {
            return nil
        }

        // C/Objective-C int type size is same as the Int32 in swift.
        let propertyIdsInt32 = propertyIdsData.withUnsafeBytes { (bytes: UnsafeRawBufferPointer) -> [Int32] in
            let elementSize = Int32.bitWidth / Int8.bitWidth
            let unsafePointer = bytes.baseAddress!
            let buffer = UnsafeBufferPointer(start: unsafePointer.assumingMemoryBound(to: Int32.self), count: propertyIdsData.count / elementSize)
            return Array<Int32>(buffer)
        }
        guard propertyIdsInt32.count > 0 else {
            return nil
        }

        // covert to Int array for easy of use.
        var propertyIdsInt = Array<Int>.init(repeating: 0, count: propertyIdsInt32.count)
        for i in 0..<propertyIdsInt32.count {
            propertyIdsInt[i] = Int(propertyIdsInt32[i])
        }
        return propertyIdsInt
    }

    /**
     Get all support property ID as in a list for a sensor.

     @param componentName       Indicates the mesh sensor device.

     @return                    Return the propery ID list in get succeess, or nil if failed to get the list;
     */
    open func meshClientSensorPropertyListGet(componentName: String) -> [Int]? {
        guard let propertyListData = MeshNativeHelper.meshClientSensorPropertyListGet(componentName) else {
            return nil
        }

        // C/Objective-C int type size is same as the Int32 in swift.
        let propertyListInt32 = propertyListData.withUnsafeBytes { (bytes: UnsafeRawBufferPointer) -> [Int32] in
            let elementSize = Int32.bitWidth / Int8.bitWidth
            let unsafePointer = bytes.baseAddress!
            let buffer = UnsafeBufferPointer(start: unsafePointer.assumingMemoryBound(to: Int32.self), count: propertyListData.count / elementSize)
            return Array<Int32>(buffer)
        }
        guard propertyListInt32.count > 0 else {
            return nil
        }

        // covert to Int array for easy of use.
        var propertyListInt = Array<Int>.init(repeating: 0, count: propertyListInt32.count)
        for i in 0..<propertyListInt32.count {
            propertyListInt[i] = Int(propertyListInt32[i])
        }
        return propertyListInt
    }

    /**
     Configure Sensor Setting state values for a sensor.

     @param componentName       Indicates the mesh sensor device.
     @param propertyId          Indicates a property ID within a sensor.
     @param settingPropertyId   Indicates a property ID of a setting within a sensor.
     @param value               Raw value of a setting within a sensor.

     @return                    Return MeshErrorCode.MESH_SUCCESS on success; otherwise failed;
     */
    open func meshClientSensorSettingSet(componentName: String, propertyId: Int, settingPropertyId: Int, value: Data) -> Int {
        return Int(MeshNativeHelper.meshClientSensorSettingSet(componentName, propertyId: Int32(propertyId), settingPropertyId: Int32(settingPropertyId), value: value))
    }

    /**
     Send the command to get the sensor data from a sensor.

     @param componentName       Indicates the mesh sensor device.
     @param propertyId          Indicates a property ID within a sensor.

     @return                    Return MeshErrorCode.MESH_SUCCESS if the comamnd send out on success; otherwise failed;

     Note, after the sensor data has been received successfully, the onMeshClientSensorStatusChanged callback will executed.
     */
    open func meshClientSensorGet(componentName: String, propertyId: Int) -> Int {
        return Int(MeshNativeHelper.meshClientSensorGet(componentName, propertyId: Int32(propertyId)))
    }

    /**
     Send the command to get the sensor data from a sensor.

     @param componentName       Indicates the mesh sensor device.
     @param propertyId          Indicates a property ID within a sensor.
     @param completion          Callback roution after the operation of get sensor data completed.

     @return                    Return MeshErrorCode.MESH_SUCCESS if the comamnd send out on success; otherwise failed;

     Note, after the operation completed, the the completion will be executed when the onMeshClientSensorStatusChanged callback executed,
     or called with the error parameter set when encounterred with any error.
     */
    open func meshClientSensorGet(componentName: String, propertyId: Int, completion: @escaping MeshSensorStatusChangedCallback) {
        guard meshSensorStatusChangedCallback[componentName] == nil else {
            meshLog("error: meshClientSensorGet, command has been sent to device:\(componentName), busying")
            completion(componentName, propertyId, nil, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientSensorGetTimeoutTimer[componentName]?.invalidate()
        meshClientSensorGetTimeoutTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                              target: self, selector: #selector(meshClientSensorGetTimeoutHandler),
                                                                              userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName,
                                                                                         MeshNotificationConstants.USER_INFO_KEY_PROPERTY_ID: propertyId],
                                                                              repeats: false)
        meshSensorStatusChangedCallback[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientSensorGet(componentName: componentName, propertyId: propertyId)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientSensorGet with componentName:\(componentName), error:\(error)")
            completion(componentName, propertyId, nil, error)
            return
        }
        // waiting for the completion callback.
    }
    private var meshClientSensorGetTimeoutTimer: [String: Timer] = [:]
    @objc func meshClientSensorGetTimeoutHandler(_ timer: Timer) {
        guard let userInfo = timer.userInfo as? [String: Any] else { return }
        guard let componentName = userInfo[MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME] as? String else {
            timer.invalidate()
            return
        }
        guard let propertyId = userInfo[MeshNotificationConstants.USER_INFO_KEY_PROPERTY_ID] as? Int else {
            timer.invalidate()
            return
        }
        meshLog("error: MeshFrameworkManager, meshClientSensorGet, meshClientSensorGetTimeoutHandler, componentName:\(componentName), propertyId;\(propertyId)")

        meshClientSensorGetTimeoutTimer[componentName]?.invalidate()
        meshClientSensorGetTimeoutTimer.removeValue(forKey: componentName)

        if let completion = meshSensorStatusChangedCallback[componentName] {
            meshSensorStatusChangedCallback.removeValue(forKey: componentName)
            completion(componentName, propertyId, nil, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        }
    }


    /**
     Register the mesh client to receive messages from the specific group with specific type.

     @param controlMethod   Indicates the type name of the control method which specified the type of the messages to be received.
     @param groupName       Indicates the mesh group where the messages come from should be received.
     @param startListening  Indicates start (true) or stop (false) the listening and receiving the specific messages.

     @return                Return MeshErrorCode.MESH_SUCCESS on success; otherwise indicates failed to start listening;

     Note, when the controlMethod is nil or empty, the library will register to receive messages sent to all type of messages.
     When the groupName is nil or empty, the library will register to receive messages sent to all the groups.
     */
    open func meshClientlistenForAppGroupBroadcast(controlMethod: String? = nil, groupName: String? = nil, startListening: Bool) -> Int {
        return Int(MeshNativeHelper.meshClientListen(forAppGroupBroadcasts: controlMethod, groupName: groupName, startListen: startListening))
    }

    /**
     Get the publication target name for the specific method name on the specific mesh component.

     @param componentName   The target mesh device name.
     @param isClient        Indicate specified method is a control method (when true) or a target method (when tofalse).
     @param method          The target method name.

     @return                Return the non-zero value as the reterieved pulish period on success;
                            Return 0 when failed to get the publish period value or encontered any error.
     */
    open func meshClientGetPublicationTarget(componentName: String, isClient: Bool, method: String) -> String? {
        guard !componentName.isEmpty, !method.isEmpty else {
            meshLog("error: meshClientGetPublicationTarget, invalid arguments, componentName:\(componentName) or method:\(method)")
            return nil
        }

        return MeshNativeHelper.meshClientGetPublicationTarget(componentName, isClient: isClient, method: method);
    }

    /**
     Get the publication period value for the specific method on the specific mesh component.

     @param componentName   The target mesh device name.
     @param isClient        Indicate specified method is a control method (when true) or a target method (when false).
     @param method          The target method name.

     @return                Return the non-zero value as the reterieved pulish period on success;
                            Return 0 when failed to get the publish period value or encontered any error.
     */
    open func meshClientGetPublicationPeriod(componentName: String, isClient: Bool, method: String) -> Int {
        guard !componentName.isEmpty, !method.isEmpty else {
            meshLog("error: meshClientGetPublicationTarget, invalid arguments, componentName:\(componentName) or method:\(method)")
            return 0
        }

        return Int(MeshNativeHelper.meshClientGetPublicationPeriod(componentName, isClient: isClient, method: method));
    }

    open func meshClientIsLightController(componentName: String) -> Bool {
        return MeshNativeHelper.meshClientIsLightController(componentName)
    }

    open func meshClientGetLightLcMode(componentName: String) -> Int {
        return Int(MeshNativeHelper.meshClientGetLightLcMode(componentName))
    }

    open func meshClientSetLightLcMode(componentName: String, mode: Int) -> Int {
        return Int(MeshNativeHelper.meshClientSetLightLcMode(componentName, mode: Int32(mode)))
    }

    public typealias MeshClientLightLcModeStatusCallback = (_ deviceName: String, _ mode: Int, _ error: Int) -> ()
    private var meshClientLightLcModeStatusCb: [String: MeshClientLightLcModeStatusCallback] = [:]
    private var meshClientLightLcModeStatusCbTimer: [String: Timer] = [:]
    @objc func meshClientLightLcModeStatusTimeoutHandler(_ timer: Timer) {
        guard let userInfo = timer.userInfo as? [String: Any] else { return }
        guard let componentName = userInfo[MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME] as? String else {
            timer.invalidate()
            return
        }
        meshLog("error: MeshFrameworkManager, meshClientLightLcModeStatusTimeoutHandler, timeout, componentName:\(componentName)")

        meshClientLightLcModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcModeStatusCbTimer.removeValue(forKey: componentName)

        if let completion = meshClientLightLcModeStatusCb[componentName] {
            meshClientLightLcModeStatusCb.removeValue(forKey: componentName)
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        }
    }

    open func meshClientGetLightLcMode(componentName: String, completion: @escaping MeshClientLightLcModeStatusCallback) {
        guard meshClientLightLcModeStatusCb[componentName] == nil else {
            meshLog("error: meshClientGetLightLcMode, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcModeStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                              target: self, selector: #selector(meshClientLightLcModeStatusTimeoutHandler),
                                                                              userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                              repeats: false)
        meshClientLightLcModeStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientGetLightLcMode(componentName: componentName)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientGetLightLcMode with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientSetLightLcMode(componentName: String, mode: Int, completion: @escaping MeshClientLightLcModeStatusCallback) {
        guard meshClientLightLcModeStatusCb[componentName] == nil else {
            meshLog("error: meshClientSetLightLcMode, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcModeStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                                 target: self, selector: #selector(meshClientLightLcModeStatusTimeoutHandler),
                                                                                 userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                                 repeats: false)
        meshClientLightLcModeStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientSetLightLcMode(componentName: componentName, mode: mode)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientSetLightLcMode with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientGetLightLcOccupancyMode(componentName: String) -> Int {
        return Int(MeshNativeHelper.meshClientGetLightLcOccupancyMode(componentName))
    }

    open func meshClientSetLightLcOccupancyMode(componentName: String, mode: Int) -> Int {
        return Int(MeshNativeHelper.meshClientSetLightLcOccupancyMode(componentName, mode: Int32(mode)))
    }

    public typealias MeshClientLightLcOccupancyModeStatusCallback = (_ deviceName: String, _ mode: Int, _ error: Int) -> ()
    private var meshClientLightLcOccupancyModeStatusCb: [String: MeshClientLightLcOccupancyModeStatusCallback] = [:]
    private var meshClientLightLcOccupancyModeStatusCbTimer: [String: Timer] = [:]
    @objc func meshClientLightLcOccupancyModeStatusTimeoutHandler(_ timer: Timer) {
        guard let userInfo = timer.userInfo as? [String: Any] else { return }
        guard let componentName = userInfo[MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME] as? String else {
            timer.invalidate()
            return
        }
        meshLog("error: MeshFrameworkManager, meshClientLightLcOccupancyModeStatusTimeoutHandler, timeout, componentName:\(componentName)")

        meshClientLightLcOccupancyModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcOccupancyModeStatusCbTimer.removeValue(forKey: componentName)

        if let completion = meshClientLightLcOccupancyModeStatusCb[componentName] {
            meshClientLightLcOccupancyModeStatusCb.removeValue(forKey: componentName)
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        }
    }

    open func meshClientGetLightLcOccupancyMode(componentName: String, completion: @escaping MeshClientLightLcOccupancyModeStatusCallback) {
        guard meshClientLightLcOccupancyModeStatusCb[componentName] == nil else {
            meshLog("error: meshClientGetLightLcOccupancyMode, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcOccupancyModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcOccupancyModeStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                                          target: self, selector: #selector(meshClientLightLcOccupancyModeStatusTimeoutHandler),
                                                                                          userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                                          repeats: false)
        meshClientLightLcOccupancyModeStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientGetLightLcOccupancyMode(componentName: componentName)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientGetLightLcOccupancyMode with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientSetLightLcOccupancyMode(componentName: String, mode: Int, completion: @escaping MeshClientLightLcOccupancyModeStatusCallback) {
        guard meshClientLightLcOccupancyModeStatusCb[componentName] == nil else {
            meshLog("error: meshClientSetLightLcOccupancyMode, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcOccupancyModeStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcOccupancyModeStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                                          target: self, selector: #selector(meshClientLightLcOccupancyModeStatusTimeoutHandler),
                                                                                          userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                                          repeats: false)
        meshClientLightLcOccupancyModeStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientSetLightLcOccupancyMode(componentName: componentName, mode: mode)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientSetLightLcOccupancyMode with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientGetLightLcProperty(componentName: String, propertyId: Int) -> Int {
        return Int(MeshNativeHelper.meshClientGetLightLcProperty(componentName, propertyId: Int32(propertyId)))
    }

    open func meshClientSetLightLcProperty(componentName: String, propertyId: Int, value: Int) -> Int {
        return Int(MeshNativeHelper.meshClientSetLightLcProperty(componentName, propertyId: Int32(propertyId), value: Int32(value)))
    }

    public typealias MeshClientLightLcPropertyStatusCallback = (_ deviceName: String, _ propertyId: Int, _ value: Int, _ error: Int) -> ()
    private var meshClientLightLcPropertyStatusCb: [String: MeshClientLightLcPropertyStatusCallback] = [:]
    private var meshClientLightLcPropertyStatusCbTimer: [String: Timer] = [:]
    @objc func meshClientLightLcPropertyStatusTimeoutHandler(_ timer: Timer) {
        guard let userInfo = timer.userInfo as? [String: Any] else { return }
        guard let componentName = userInfo[MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME] as? String else {
            timer.invalidate()
            return
        }
        meshLog("error: MeshFrameworkManager, meshClientLightLcPropertyStatusTimeoutHandler, timeout, componentName:\(componentName)")

        meshClientLightLcPropertyStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcPropertyStatusCbTimer.removeValue(forKey: componentName)

        if let completion = meshClientLightLcPropertyStatusCb[componentName] {
            meshClientLightLcPropertyStatusCb.removeValue(forKey: componentName)
            completion(componentName, 0, 0, MeshErrorCode.MESH_ERROR_PRECEDURE_TIMEOUT)
        }
    }

    open func meshClientGetLightLcProperty(componentName: String, propertyId: Int, completion: @escaping MeshClientLightLcPropertyStatusCallback) {
        guard meshClientLightLcPropertyStatusCb[componentName] == nil else {
            meshLog("error: meshClientGetLightLcProperty, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcPropertyStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcPropertyStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                                     target: self, selector: #selector(meshClientLightLcPropertyStatusTimeoutHandler),
                                                                                     userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                                     repeats: false)
        meshClientLightLcPropertyStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientGetLightLcProperty(componentName: componentName, propertyId: propertyId)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientGetLightLcProperty with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientSetLightLcProperty(componentName: String, propertyId: Int, value: Int, completion: @escaping MeshClientLightLcPropertyStatusCallback) {
        guard meshClientLightLcPropertyStatusCb[componentName] == nil else {
            meshLog("error: meshClientSetLightLcProperty, command has been sent to device:\(componentName), busying")
            completion(componentName, 0, 0, MeshErrorCode.MESH_ERROR_API_IS_BUSYING)
            return
        }

        meshClientLightLcPropertyStatusCbTimer[componentName]?.invalidate()
        meshClientLightLcPropertyStatusCbTimer[componentName] = Timer.scheduledTimer(timeInterval: TimeInterval(MeshConstants.MESH_CLIENT_GET_DATA_TIMETOUT),
                                                                                          target: self, selector: #selector(meshClientLightLcPropertyStatusTimeoutHandler),
                                                                                          userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName],
                                                                                          repeats: false)
        meshClientLightLcPropertyStatusCb[componentName] = completion
        let error = MeshFrameworkManager.shared.meshClientSetLightLcProperty(componentName: componentName, propertyId: propertyId, value: value)
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("error: meshClientSensorGet, failed to call meshClientSetLightLcProperty with componentName:\(componentName), error:\(error)")
            completion(componentName, 0, 0, error)
            return
        }
        // waiting for the completion callback.
    }

    open func meshClientSetLightLcOnOffSet(componentName: String, isOn: Bool, reliable: Bool, transitionTime: UInt32, delay: Int) -> Int {
        return Int(MeshNativeHelper.meshClientSetLightLc(onOffSet: componentName, onoff: isOn ? 1 : 0, reliable: reliable, transitionTime: transitionTime, delay: UInt16(delay)))
    }
}

extension MeshFrameworkManager {
    private static let MESH_FRAMEWORK_PESISTENT_DATABASE = "com.cypress.le.mesh.MeshApp.MeshFramework.MeshFrameworkManager.PesistentDatabase"
    private static let KEY_UNIQUE_ID = MeshFrameworkManager.MESH_FRAMEWORK_PESISTENT_DATABASE + ".uniqueId"
    private static let KEY_PROVISIONED_MESH_DEVICES = MeshFrameworkManager.MESH_FRAMEWORK_PESISTENT_DATABASE + ".provisionedMeshDevices"

    private var pesistentDatabase: UserDefaults {
        if let db = pesistentDb {
            return db
        } else {
            pesistentDb = UserDefaults(suiteName: MeshFrameworkManager.MESH_FRAMEWORK_PESISTENT_DATABASE)
        }
        if let db = pesistentDb {
            return db
        } else {
            meshLog("error: MeshFrameworkManager, failed to create pesistentDatabase in UserDefaults. use UserDefaults.standard instand")
            return UserDefaults.standard
        }
    }

    // Unique ID used to uniquly identify the the Mesh device with this MeshApp.
    // Note, the uniqueId will change when the App is uninstalled, then reinstalled.
    open var uniqueId: String {
        get {
            if let uniqueId = pesistentDatabase.value(forKey: MeshFrameworkManager.KEY_UNIQUE_ID) as? String, !uniqueId.isEmpty {
                return uniqueId
            }
            return generateUniqueId()
        }
        set(value) {
            pesistentDatabase.set(value, forKey: MeshFrameworkManager.KEY_UNIQUE_ID)
            pesistentDatabase.synchronize()
        }
    }

    open func generateUniqueId() -> String {
        uniqueId = MeshNativeHelper.generateRfcUuid().description
        meshLog("MeshFrameworkManager: generate app uniqueId: \(uniqueId)")
        return uniqueId
    }

    open func updateUniqueId(uuid: UUID) {
        uniqueId = uuid.description
        meshLog("MeshFrameworkManager: updated with new app uniqueId: \(uniqueId)")
    }

    open func generateProvisionerName(uniqueId: String) -> String {
        if uniqueId.isEmpty {
            return "provisioner_\(self.uniqueId)"
        }
        return "provisioner_\(uniqueId)"
    }

    open func generateMeshNetworkName(uniqueId: String) -> String {
        if uniqueId.isEmpty {
            return "network_\(self.uniqueId)"
        }
        return "network_\(uniqueId)"
    }

    open var defaultExportStorageFolderName: String {
        return MeshStorageSettings.shared.exportMeshStorageName
    }

    open var defaultExportStoragePath: String? {
        return MeshStorageSettings.shared.exportMeshStoragePath
    }

    open var meshNetworksUnderExportedMeshStorage: [String]? {
        return MeshStorageSettings.shared.meshNetworksUnderExportedMeshStorage
    }

    open func readExportedMeshNetwork(networkName: String) -> String? {
        return MeshStorageSettings.shared.readExportedMeshNetwork(networkName: networkName)
    }

    // By default, the export network json file will be stored under @defaultExportStoragePath stroage:
    // e.g.: "/private/var/mobile/Containers/Data/Application/XXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/Documents/mesh/ExportedMeshNetworks/"
    open func writeExportedMeshNetwork(networkName: String, jsonContent: String) -> Int {
        return MeshStorageSettings.shared.writeExportedMeshNetwork(networkName: networkName, jsonContent: jsonContent)
    }

    open var provisionedMeshDevices: [MeshDevice] {
        get {
            let key = MeshFrameworkManager.KEY_PROVISIONED_MESH_DEVICES + "_\(currentUserName ?? "")_"
            if let archivedData = pesistentDatabase.value(forKey: key) as? Data,
                let meshDevices = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [MeshDevice] {
                return meshDevices
            }
            return []
        }
        set(value) {
            let key = MeshFrameworkManager.KEY_PROVISIONED_MESH_DEVICES + "_\(currentUserName ?? "")_"
            let data = NSKeyedArchiver.archivedData(withRootObject: value)
            pesistentDatabase.set(data, forKey: key)
            pesistentDatabase.synchronize()
        }
    }

    open func meshClientIsSameNodeElements(networkName: String, elementName: String, anotherElementName: String) -> Bool {
        return MeshNativeHelper.meshClientIsSameNodeElements(networkName, elementName: elementName, anotherElementName: anotherElementName)
    }

    open func meshClientGetNodeElements(networkName: String, by name: String) -> Int {
        let ret = MeshNativeHelper.meshClientGetNodeElements(networkName, elementName: name)
        if ret < 0 {
            meshLog("error: MeshFramework, meshClientGetNodeElements, \((ret == -2) ? "element Name: \(name) not found" : "network \(networkName).json file not exist")")
            return 0
        }
        return Int(ret)
    }

    open func meshClientIsNodeBlocked(networkName: String, by name: String) -> Bool {
        let ret = MeshNativeHelper.meshClientIsNodeBlocked(networkName, elementName: name)
        if ret < 0 {
            meshLog("error: MeshFramework, meshClientIsNodeBlocked, \((ret == -2) ? "element Name: \(name) not found" : "network \(networkName).json file not exist")")
            return false
        }
        return (ret > 0) ? true : false
    }

    open func isMeshClientProvisionKeyRefreshing() -> Bool {
        return MeshNativeHelper.isMeshClientProvisionKeyRefreshing()
    }
}

extension MeshFrameworkManager: IMeshNativeCallback {
    /**
     * Mesh library calls this routine to get the support from host to write provisioning data to the Mesh Provisioning Data In characteristic of the remote device.
     */
    public func onProvGattPktReceivedCallback(_ connId: UInt16, data: Data) {
        meshLog("IMeshNativeCallback, onProvGattPktReceivedCallback, connId: \(connId), data: \(data.dumpHexBytes())")
        if let peripheral = MeshNativeHelper.getCurrentConnectedPeripheral() {
            MeshGattClient.shared.writeData(for: peripheral, serviceUUID: MeshUUIDConstants.UUID_SERVICE_MESH_PROVISIONING, data: data)
        } else {
            meshLog("error: MeshFrameworkManager, onProvGattPktReceivedCallback, invalid current peripheral nil")
        }
    }

    /**
     * Mesh library calls this routine to get the support from host to write proxy data to the Mesh Proxy Data In characteristic of the remote device.
     */
    public func onProxyGattPktReceivedCallback(_ connId: UInt16, data: Data) {
        meshLog("IMeshNativeCallback, onProxyGattPktReceivedCallback, connId:\(connId), data: \(data.dumpHexBytes())")
        if let peripheral = MeshNativeHelper.getCurrentConnectedPeripheral() {
            MeshGattClient.shared.writeData(for: peripheral, serviceUUID: MeshUUIDConstants.UUID_SERVICE_MESH_PROXY, data: data)
        } else {
            meshLog("error: IMeshNativeCallback, onProxyGattPktReceivedCallback, invalid current peripheral nil")
        }
    }

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) that an unprovisioned mesh device has been found, and with some parsed mesh data for the device.
     */
    public func onDeviceFound(_ uuid: UUID, oob: UInt16, uriHash: UInt32, name: String?) {
        meshLog("IMeshNativeCallback, onDeviceFound, uuid:\(uuid.uuidString), oob:\(oob), uriHash:\(uriHash), name:\(String(describing: name))")
        guard let deviceName = name, deviceName.count > 0 else { return }
        MeshGattClient.shared.onUnprovisionedDeviceFound(uuid: uuid, oob: oob, uriHash: uriHash, name: deviceName)
        // The MeshNotificationConstants.MESH_CLIENT_DEVICE_FOUND notification is sent in MeshGattClient.shared.onUnprovisionedDeviceFound.
        // when onDeviceFound callback is invoked multiple times for a same unprovisioned device, the notification will be sent once.
    }

    /*
     * Obsoleted. Left for supporting old mesh library.
     */
    public func onResetStatus(_ status: UInt8, devName: String) {
        meshLog("IMeshNativeCallback, onResetStatus, status:\(status), deviceName:\(devName)")
    }

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) that mesh client has connected or disconnected to or from the openned mesh network.
     */
    public func onLinkStatus(_ isConnected: UInt8, connId: UInt32, addr: UInt16, isOverGatt: UInt8) {
        meshLog("IMeshNativeCallback, onLinkStatus, isConnected:\(isConnected), connId:\(connId), addr:\(addr), isOverGatt:\(isOverGatt)")
        for completion in meshNetworkLinkStatusUpdatedCb {
            completion((isConnected != 0), Int(connId), Int(addr), (isOverGatt == MeshConstants.MESH_DEFAULT_USE_GATT_PROXY), MeshErrorCode.MESH_SUCCESS)
        }
        meshNetworkLinkStatusUpdatedCb.removeAll()

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_LINK_STATUS_IS_CONNECTED: Int(isConnected),
                                                   MeshNotificationConstants.USER_INFO_KEY_LINK_STATUS_CONNID: Int(connId),
                                                   MeshNotificationConstants.USER_INFO_KEY_LINK_STATUS_ADDR: Int(addr),
                                                   MeshNotificationConstants.USER_INFO_KEY_LINK_STATUS_IS_OVER_GATT: Int(isOverGatt)])
    }

    /**
     * Mesh library calls this routine to notify mesh client that the json database of the opened mesh network has be updated.
     */
    public func onDatabaseChangedCb(_ meshName: String) {
        meshLog("IMeshNativeCallback, onDatabaseChangedCb, meshName: \(meshName)")
        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_DB_CHANGED_MESH_NAME: meshName])

        removeMeshComponentTimer?.invalidate()
        removeMeshComponentTimer = nil

        if let completion = meshDatabaseChangedCb {
            meshDatabaseChangedCb = nil
            completion(meshName, MeshErrorCode.MESH_SUCCESS)
        }
    }

    /**
     * Mesh library calls this routine to notify mesh client that a component inforamtin status data has been received.
     * This callback routine will invoked after mesh client call the getMeshComponentInfo() API.
     */
    public func meshClientComponentInfoStatusCb(_ status: UInt8, componentName: String, componentInfo: String?) {
        meshLog("IMeshNativeCallback, meshClientComponentInfoStatusCb, status: \(status), componentName:\(componentName), componentInfo:\(String(describing: componentInfo))")
        if let completion = getMeshComponentInfoCb[componentName] {
            getMeshComponentInfoCb.removeValue(forKey: componentName)
            completion(componentName, componentInfo, Int(status))
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_COMPONENT_INFO_UPDATED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_NAME: componentName,
                                                   MeshNotificationConstants.USER_INFO_KEY_NODE_COMPONENT_INFO: (componentInfo ?? "") as Any])
    }

    /**
     * Mesh library calls this routine to notify mesh client about open status of the mesh network.
     * This callback routine will invoked after mesh client call the openMeshNetwork() or meshClientNetworkImport() API.
     */
    public func meshClientNetworkOpenCb(_ status: UInt8) {
        meshLog("IMeshNativeCallback, meshClientNetworkOpenCb, status=\(status), openingNetworkName=\(String(describing: openingNetworkName))")
        if status == MeshErrorCode.MESH_SUCCESS, let networkName = openingNetworkName, let openingProvisioner = openingProvisionerName {
            openedNetworkName = networkName                 // MeshFrameworkManager maintained opened mesh network name.
            openedProvisionerName = openingProvisioner      // MeshFrameworkManager maintained opened mesh network provisioner.
        }

        // invoke mesh network open completion callback if exsting.
        openMeshNetworkTimer?.invalidate()
        openMeshNetworkTimer = nil
        openMeshNetworkCb?(openingNetworkName, Int(status), MeshErrorCode.MESH_SUCCESS)
        lock.lock()
        openMeshNetworkCb = nil
        lock.unlock()

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_OPENNED_CB),
                                        object: nil, userInfo: [MeshNotificationConstants.USER_INFO_KEY_NETWORK_OPENNED_STATUS: Int(status)])
    }

    /**
     * Mesh library calls this routine to notify mesh client that the provisioning status of the remote device which specified by the device uuid.
     * This callback routine will invoked after mesh client call the meshClientProvision() API.
     *
     * The valid value of the provisionStatus can see the constant definitions of MeshConstants.MESH_CLIENT_PROVISION_STATUS_XXXX.
     * The value of uuid uniquely identified an unprovisioned or provisioned mesh device, it should not change during the whole life of the mesh device.
     */
    public func meshClientProvisionCompletedCb(_ provisionStatus: UInt8, uuid: UUID) {
        switch Int(provisionStatus) {
        case MeshConstants.MESH_CLIENT_PROVISION_STATUS_SUCCESS:
            lock.lock()
            provisionUuid = nil
            provisionTimer?.invalidate()
            provisionTimer = nil
            lock.unlock()
            meshLog("IMeshNativeCallback, meshClientProvisionCompletedCb, uuid=\(uuid.description), provisionStatus=\(provisionStatus), provision success")
        case MeshConstants.MESH_CLIENT_PROVISION_STATUS_END:
            meshLog("IMeshNativeCallback, meshClientProvisionCompletedCb, uuid=\(uuid.description), provisionStatus=\(provisionStatus), provision end")
            #if true   // Just for test purpose. The mesh library has support the timer monitoring, so the App timer monitoring is not required.
            let testMonitorTimeInterval = MeshConstants.MESH_CLIENT_PROVISION_TIMEOUT_DURATION * 2  // just for long enough test.
            meshLog("IMeshNativeCallback, meshClientProvisionCompletedCb, MESH_CLIENT_PROVISION_STATUS_END, start configuring monitor timer, timeInterval(s): \(testMonitorTimeInterval)")
            provisionTimer = Timer.scheduledTimer(timeInterval: TimeInterval(testMonitorTimeInterval),
                                                  target: self, selector: #selector(meshClientProvisionTimeoutCompleted),
                                                  userInfo: nil, repeats: false)
            #endif
        default:
            if Int(provisionStatus) == MeshConstants.MESH_CLIENT_PROVISION_STATUS_FAILED {
                meshLog("IMeshNativeCallback, meshClientProvisionCompletedCb, uuid=\(uuid.description), provisionStatus=\(provisionStatus), provision failed")
                lock.lock()
                provisionUuid = nil
                provisionTimer?.invalidate()
                provisionTimer = nil
                lock.unlock()
            } else {
                meshLog("IMeshNativeCallback, meshClientProvisionCompletedCb, uuid=\(uuid.description), provisionStatus=\(provisionStatus)")
            }
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_PROVISION_COMPLETE_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_PROVISION_STATUS: Int(provisionStatus),
                                                   MeshNotificationConstants.USER_INFO_KEY_DEVICE_UUID: uuid])
    }


    public func meshClient(onOffStateCb deviceName: String, target: UInt8, present: UInt8, remainingTime: UInt32) {
        meshLog("IMeshNativeCallback, meshClientOnOffStateCb, deviceName:\(deviceName), target:\(target), present:\(present), remainingTime:\(remainingTime)")
        if let completion = meshClientOnOffStatusCb[deviceName] {
            meshClientOnOffStatusCb.removeValue(forKey: deviceName)
            completion(deviceName, (target == 0) ? false : true, (present == 0) ? false : true, remainingTime, MeshErrorCode.MESH_SUCCESS)
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_ON_OFF_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_TARGET: Int(target),
                                                   MeshNotificationConstants.USER_INFO_KEY_PRESENT: Int(present),
                                                   MeshNotificationConstants.USER_INFO_KEY_REMAINING_TIME: UInt32(remainingTime)])
    }

    public func meshClientLevelStateCb(_ deviceName: String, target: Int16, present: Int16, remainingTime: UInt32) {
        meshLog("IMeshNativeCallback, meshClientLevelStateCb, deviceName:\(deviceName), target:\(target), present:\(present), remaining_time:\(remainingTime)")

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_LEVEL_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_LEVEL: Int(target),
                                                   MeshNotificationConstants.USER_INFO_KEY_PRESENT: Int(present),
                                                   MeshNotificationConstants.USER_INFO_KEY_REMAINING_TIME: UInt32(remainingTime)])
    }

    public func meshClientHslStateCb(_ deviceName: String, lightness: UInt16, hue: UInt16, saturation: UInt16, remainingTime: UInt32) {
        meshLog("IMeshNativeCallback, meshClientHslStateCb, deviceName:\(deviceName), lightness:\(lightness), hue:\(hue), saturation:\(saturation), remainingTime:\(remainingTime)")

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_HSL_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_LIGHTNESS: Int(lightness),
                                                   MeshNotificationConstants.USER_INFO_KEY_HUE: Int(hue),
                                                   MeshNotificationConstants.USER_INFO_KEY_SATURATION: Int(saturation),
                                                   MeshNotificationConstants.USER_INFO_KEY_REMAINING_TIME: UInt32(remainingTime)])
    }

    public func meshClientCtlStateCb(_ deviceName: String, presentLightness: UInt16, presentTemperature: UInt16, targetLightness: UInt16, targetTemperature: UInt16, remainingTime: UInt32) {
        meshLog("IMeshNativeCallback, meshClientCtlStateCb, deviceName:\(deviceName), presentLightness:\(presentLightness), presentTemperature:\(presentTemperature), targetLightness:\(targetLightness)")

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_CTL_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_PRESENT_LIGHTNESS: Int(presentLightness),
                                                   MeshNotificationConstants.USER_INFO_KEY_PRESENT_TEMPERATURE: Int(presentTemperature),
                                                   MeshNotificationConstants.USER_INFO_KEY_TARGET_LIGHTNESS: Int(targetLightness),
                                                   MeshNotificationConstants.USER_INFO_KEY_TARGET_TEMPERATURE: Int(targetTemperature),
                                                   MeshNotificationConstants.USER_INFO_KEY_REMAINING_TIME: UInt32(remainingTime)])
    }

    public func meshClientLightnessStateCb(_ deviceName: String, target: UInt16, present: UInt16, remainingTime: UInt32) {
        meshLog("IMeshNativeCallback, meshClientLightnessStateCb, deviceName:\(deviceName), target:\(target), present:\(present), remainingTime:\(remainingTime)")

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_LIGHTNESS_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_TARGET: Int(target),
                                                   MeshNotificationConstants.USER_INFO_KEY_PRESENT: Int(present),
                                                   MeshNotificationConstants.USER_INFO_KEY_REMAINING_TIME: UInt32(remainingTime)])
    }

    public func onMeshClientSensorStatusChanged(_ deviceName: String, propertyId: UInt32, data: Data) {
        meshLog("IMeshNativeCallback, onMeshClientSensorStatusChanged, deviceName:\(deviceName), propertyId:\(propertyId), data: \(data.dumpHexBytes())")
        if let completion = meshSensorStatusChangedCallback[deviceName] {
            meshSensorStatusChangedCallback.removeValue(forKey: deviceName)
            completion(deviceName, Int(propertyId), data, MeshErrorCode.MESH_SUCCESS)
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_SENSOR_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_PROPERTY_ID: Int(propertyId),
                                                   MeshNotificationConstants.USER_INFO_KEY_DATA: data])
    }

    /**
     * Mesh library calls this routine to get the support from host to start LE devices scanning.
     */
    public func meshClientAdvScanStartCb() -> Bool {
        meshLog("IMeshNativeCallback, meshClientAdvScanStartCb")
        MeshGattClient.shared.startScan()
        return true
    }

    /**
     * Mesh library calls this routine to get the support from host to stop LE devices scanning.
     */
    public func meshClientAdvScanStopCb() {
        meshLog("IMeshNativeCallback, meshClientAdvScanStopCb")
        MeshGattClient.shared.stopScan()
    }

    /**
     * Mesh library calls this routine to get the support from host to connect to the specific mesh device based on the LE address.
     */
    public func meshClientConnect(_ bdaddr: Data) -> Bool {
        meshLog("IMeshNativeCallback, meshClientConnect, bdaddr: \(bdaddr.dumpHexBytes())")
        if let peripheral = MeshNativeHelper.meshBdAddrDictGetCBPeripheral(bdaddr) {
            usleep(200000)  //sleep for 200 ms
            MeshGattClient.shared.connect(peripheral: peripheral)
            return true
        }

        meshLog("error: IMeshNativeCallback, meshClientConnect, invalid unknown bdaddr: \(bdaddr.dumpHexBytes())")
        MeshFrameworkManager.shared.meshClientConnectionStateChanged(connId: 0)
        return false
    }

    /**
     * Because the mesh library only supports one network connection, so the connId is not used right now.
     * The value of connId is 0 if not connected, and always 1 when connected.
     */
    public func meshClientDisconnect(_ connId: UInt16) -> Bool {
        meshLog("IMeshNativeCallback, meshClientDisconnect, connId:\(connId)")
        if let peripheral = MeshNativeHelper.getCurrentConnectedPeripheral() {
            usleep(200000)  //sleep for 200 ms
            MeshGattClient.shared.disconnect(peripheral: peripheral)
        } else {
            meshLog("warnnig: IMeshNativeCallback, meshClientDisconnect, connId:\(connId), getCurrentConnectedPeripheral is nil, no peripheral connected")
            MeshFrameworkManager.shared.meshClientConnectionStateChanged(connId: 0)
        }
        return true
    }

    public func meshClientNodeConnectStateCb(_ status: UInt8, componentName: String) {
        meshLog("IMeshNativeCallback, meshClientNodeConnectStateCb, status:\(status), componentName:\(componentName)")
        if status == MeshConstants.MESH_CLIENT_NODE_CONNECTED {
            meshLog("info: meshClientNodeConnectStateCb, mesh device:\(componentName) connected")
            unreachableMeshDevices.removeAll(where: {$0 == componentName})
        } else {
            if status == MeshConstants.MESH_CLIENT_NODE_WARNING_UNREACHABLE {
                meshLog("error: meshClientNodeConnectStateCb, failed to connect to mesh device:\(componentName)")
            } else {
                meshLog("error: meshClientNodeConnectStateCb, mesh device:\(componentName) unreachable")
            }
            if unreachableMeshDevices.filter({$0 == componentName}).count == 0 {
                unreachableMeshDevices.append(componentName)
            }
            /**
             * Notes,
             * App should appropriately show message to user on receiving node unreachable event.
             * Also, app can call getUnreachableMeshDevices() API to get and show all unreachable devices to user.
             */
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_NODE_CONNECTION_STATUS: Int(status),
                                                   MeshNotificationConstants.USER_INFO_KEY_NODE_NAME: componentName])

        lock.lock()
        if let completion = componentConnectCb, let connectName = componentConnectName {
            componentConnectCb = nil
            componentConnectName = nil
            componentConnectTimer?.invalidate()
            componentConnectTimer = nil
            lock.unlock()
            completion(Int(status), connectName, MeshErrorCode.MESH_SUCCESS)
        } else {
            componentConnectCb = nil
            componentConnectName = nil
            lock.unlock()
        }
    }

    /**
     Mesh library call this callback to set scan type to GATT or ADV based type for device canning.
     Currently, only ADV scan type is supported and cannot be changed, so it always return true.
     */
    public func meshClientSetScanTypeCb(_ scanType: UInt8) -> Bool {
        meshLog("IMeshNativeCallback, meshClientSetScanTypeCb, scanType=\(scanType)")
        return true
    }

    #if MESH_DFU_ENABLED
    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) about the DFU status.
     * This API will be triggerred after calling meshClientDfuGetStatus API successfully.
     */
    public func meshClientDfuStatusCb(_ state: UInt8, data: Data) {
        meshLog("IMeshNativeCallback, meshClientDfuStatusCb, state: \(state), data: \(data.dumpHexBytes())")
        OtaUpgrader.meshDfuState = Int(state)
        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DFU_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DFU_STATE: Int(state),
                                                   MeshNotificationConstants.USER_INFO_KEY_DFU_STATE_DATA: data])
    }
    #endif  // #if MESH_DFU_ENABLED

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) about the Vender Specific data changed status.
     * This API will be triggerred after calling meshClientVendorDataSet API successfully.
     */
    public func onMeshClientVendorSpecificDataChanged(_ deviceName: String, companyId: UInt16, modelId: UInt16, opcode: UInt8, ttl: UInt8, data: Data) {
        meshLog("IMeshNativeCallback, onMeshClientVendorSpecificDataChanged, deviceName: \(deviceName), companyId: \(companyId), modelId: \(modelId), opcode: \(opcode), ttl: \(ttl) data: \(data.dumpHexBytes())")

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_VENDOR_SPECIFIC_DATA_CHANGED),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_VENDOR_COMPANY_ID: Int(companyId),
                                                   MeshNotificationConstants.USER_INFO_KEY_VENDOR_MODEL_ID: Int(modelId),
                                                   MeshNotificationConstants.USER_INFO_KEY_VENDOR_OPCODE: Int(opcode),
                                                   MeshNotificationConstants.USER_INFO_KEY_VENDOR_TTL: Int(ttl),
                                                   MeshNotificationConstants.USER_INFO_KEY_DATA: data])
    }

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) about the light LC mode status.
     * This API will be triggerred after calling meshClientGetLightLcMode or meshClientSetLightLcMode API successfully.
     */
    public func onLightLcModeStatusCb(_ deviceName: String, mode: Int32) {
        meshLog("IMeshNativeCallback, onLightLcModeStatusCb, deviceName: \(deviceName), mode: \(mode)")
        meshClientLightLcModeStatusCbTimer[deviceName]?.invalidate()
        meshClientLightLcModeStatusCbTimer.removeValue(forKey: deviceName)
        if let completion = meshClientLightLcModeStatusCb[deviceName] {
            meshClientLightLcPropertyStatusCb.removeValue(forKey: deviceName)
            completion(deviceName, Int(mode), MeshErrorCode.MESH_SUCCESS)
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_LIGHT_LC_MODE_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_LIGHT_LC_MODE: Int(mode)])
    }

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) about the light LC occupancy mode status.
     * This API will be triggerred after calling meshClientGetLightLcOccupancyMode or meshClientSetLightLcOccupancyMode API successfully.
     */
    public func onLightLcOccupancyModeStatusCb(_ deviceName: String, mode: Int32) {
        meshLog("IMeshNativeCallback, onLightLcOccupancyModeStatusCb, deviceName: \(deviceName), mode: \(mode)")
        meshClientLightLcOccupancyModeStatusCbTimer[deviceName]?.invalidate()
        meshClientLightLcOccupancyModeStatusCbTimer.removeValue(forKey: deviceName)
        if let completion = meshClientLightLcOccupancyModeStatusCb[deviceName] {
            meshClientLightLcOccupancyModeStatusCb.removeValue(forKey: deviceName)
            completion(deviceName, Int(mode), MeshErrorCode.MESH_SUCCESS)
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_LIGHT_LC_OCCUPANCY_MODE_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_LIGHT_LC_OCCUPANCY_MODE: Int(mode)])
    }

    /**
     * Mesh library calls this routine to notify uppler layer (such as: App) about the light LC property value status.
     * This API will be triggerred after calling meshClientGetLightLcProperty or meshClientSetLightLcProperty API successfully.
     */
    public func onLightLcPropertyStatusCb(_ deviceName: String, propertyId: Int32, value: Int32) {
        meshLog("IMeshNativeCallback, onLightLcPropertyStatusCb, deviceName: \(deviceName), propertyId: \(propertyId), value: \(value)")
        meshClientLightLcPropertyStatusCbTimer[deviceName]?.invalidate()
        meshClientLightLcPropertyStatusCbTimer.removeValue(forKey: deviceName)
        if let completion = meshClientLightLcPropertyStatusCb[deviceName] {
            meshClientLightLcPropertyStatusCb.removeValue(forKey: deviceName)
            completion(deviceName, Int(propertyId), Int(value), MeshErrorCode.MESH_SUCCESS)
        }

        NotificationCenter.default.post(name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_LIGHT_LC_PROPERTY_STATUS),
                                        object: nil,
                                        userInfo: [MeshNotificationConstants.USER_INFO_KEY_DEVICE_NAME: deviceName,
                                                   MeshNotificationConstants.USER_INFO_KEY_LIGHT_LC_PROPERTY_ID: Int(propertyId),
                                                   MeshNotificationConstants.USER_INFO_KEY_LIGHT_LC_PROPERTY_VALUE: Int(value)])
    }

    public func updateProvisionerUuid(_ uuid: UUID) {
        MeshFrameworkManager.shared.updateUniqueId(uuid: uuid)
    }

    /*
     * implemention of the callbacks for DFU process.
     */
    public func meshClientStartOtaTransferForDfu() {
        #if MESH_DFU_ENABLED
        OtaUpgrader.shared.startOtaTransferForDfu()
        #endif
    }

    public func meshClientIsOtaSupportedForDfu() -> Bool {
        #if MESH_DFU_ENABLED
        return OtaUpgrader.shared.isOtaSupportedForDfu()
        #else
        return false
        #endif
    }
}

public struct MeshComponentInfo {
    public var cid: Int
    public var pid: Int
    public var vid: Int
    public var isVerionAvaible = false
    public var majorVer: Int = 0
    public var minorVer: Int = 0
    public var revisionNum: Int = 0
    public var CID: String
    public var PID: String
    public var VID: String
    public var VER: String

    public init(cid: Int, pid: Int, vid: Int, majorVer: Int? = nil, minorVer: Int? = nil, revisionNum: Int? = nil) {
        self.cid = cid
        self.pid = pid
        self.vid = vid
        if majorVer == nil, minorVer == nil, revisionNum == nil {
            self.isVerionAvaible = false
        } else {
            self.isVerionAvaible = true
        }
        self.majorVer = majorVer ?? 0
        self.minorVer = minorVer ?? 0
        self.revisionNum = revisionNum ?? 0
        self.CID = String(format: "%05d", cid)
        self.PID = String(format: "%05d", pid)
        self.VID = String(format: "%05d", vid)
        self.VER = !self.isVerionAvaible ? "Not Avaiable" : String(format: "%d.%d.%d",
                                                                    self.majorVer, self.minorVer, self.revisionNum)
    }

    // The input value format for these parameters must be decimal value.
    // The format of veresion string is: major_ver.minor_ver.revison_num
    public init(CID: String, PID: String, VID: String, version: String? = nil) {
        self.CID = CID.trimmingCharacters(in: CharacterSet.whitespaces)
        self.PID = PID.trimmingCharacters(in: CharacterSet.whitespaces)
        self.VID = VID.trimmingCharacters(in: CharacterSet.whitespaces)
        self.cid = Int(self.CID) ?? 0
        self.pid = Int(self.PID) ?? 0
        self.vid = Int(self.VID) ?? 0
        if let version = version?.trimmingCharacters(in: CharacterSet.whitespaces) {
            let verArray = version.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == "."})
            if verArray.count >= 3 {
                self.isVerionAvaible = true
                self.majorVer = Int(verArray[0]) ?? 0
                self.minorVer = Int(verArray[1]) ?? 0
                self.revisionNum = Int(verArray[2]) ?? 0
            } else {
                self.isVerionAvaible = false
            }
        }

        self.VER = !self.isVerionAvaible ? "Not Avaiable" : String(format: "%d.%d.%d",
                                                                   self.majorVer, self.minorVer, self.revisionNum)
    }

    public init(componentInfo: String) {
        self.cid = 0
        self.pid = 0
        self.vid = 0
        let infoArray = componentInfo.split(maxSplits: 3, omittingEmptySubsequences: true, whereSeparator: {$0 == " "})
        for item in infoArray {
            if item.hasPrefix("CID:") {
                let cidStrings = item.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == ":"})
                self.cid = Int(cidStrings[1]) ?? 0
            } else if item.hasPrefix("PID:") {
                let pidStrings = item.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == ":"})
                self.pid = Int(pidStrings[1]) ?? 0
            } else if item.hasPrefix("VID:") {
                let vidStrings = item.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == ":"})
                self.vid = Int(vidStrings[1]) ?? 0
            } else if item.hasPrefix("VER:") {
                let verStrings = item.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == ":"})
                let verString = String(verStrings[1])
                let verArray = verString.split(maxSplits: 2, omittingEmptySubsequences: true, whereSeparator: {$0 == "."})
                if verArray.count >= 3 {
                    self.isVerionAvaible = true
                    self.majorVer = Int(verArray[0]) ?? 0
                    self.minorVer = Int(verArray[1]) ?? 0
                    self.revisionNum = Int(verArray[2]) ?? 0
                    self.VER = String(format: "%d.%d.%d", self.majorVer, self.minorVer, self.revisionNum)
                } else {
                    self.isVerionAvaible = false
                }
            }
        }
        self.CID = String(format: "%05d", self.cid)
        self.PID = String(format: "%05d", self.pid)
        self.VID = String(format: "%05d", self.vid)
        self.VER = !self.isVerionAvaible ? "Not Avaiable" : String(format: "%d.%d.%d",
                                                                    self.majorVer, self.minorVer, self.revisionNum)
    }

    public var description: String {
        return String("CID:\(self.CID) PID:\(self.PID) VID:\(self.VID) VER:\(self.VER)")
    }
}
