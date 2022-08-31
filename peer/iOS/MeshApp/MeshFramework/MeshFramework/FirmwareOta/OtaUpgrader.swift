/*
 * Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
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
 * OTA upgrader process implementation.
 */

import Foundation

public enum OtaCharacteristic {
    case controlPointCharacteristic
    case dataCharacteristic
    case appInfoCharacteristic
}

/* The firmwareVersionBuild field only valid when the metadataVersion >= 3; if not valid, it was always set to 0.  */
public typealias OtaDfuMetadata = (companyId: UInt16, firwmareId: Data, productId: UInt16, hardwareVeresionId: UInt16,
    firmwareVersionMajor: UInt8, firmwareVersionMinor: UInt8, firmwareVersionRevision: UInt16, firmwareVersionBuild: UInt16,
    metadata: Data, metadataVersion: UInt8)

public protocol OtaUpgraderProtocol {
    /*
     * The OTA adapter must call this interface when the connection state to the remote OTA device has changed.
     */
    func didUpdateConnectionState(isConnected: Bool, error: Error?)

    /*
     * The OTA adapter must call this interface when the OTA service and charactieristics discovering has completed.
     * After the OTA service has been discovered, the OTA adapter should save those instacen for later usage.
     */
    func didUpdateOtaServiceCharacteristicState(isDiscovered: Bool, error: Error?)

    /*
     * The OTA adapter must call this interface when the notification states of the OTA service Control Pointer characteristic is udpated,
     */
    func didUpdateNotificationState(isEnabled: Bool, error: Error?)

    /*
     * The OTA adapter must call this interface when any data received from any OTA service characteristic,
     * inlcuding the characterisitic indication/notification data and read value data.
     */
    func didUpdateValueFor(characteristic: OtaCharacteristic, value: Data?, error: Error?)
}

open class OtaUpgrader: OtaUpgraderProtocol {
    public static let shared = OtaUpgrader()
    public static var dfuState: Int = MeshDfuState.MESH_DFU_STATE_INIT
    public static var activeDfuType: Int?
    public static var activeDfuFwImageFileName: String?
    public static var activeDfuFwMetadataFileName: String?

    private var dfuType: Int = MeshDfuType.APP_OTA_TO_DEVICE
    private var dfuMetadata: OtaDfuMetadata?
    private var isOtaTransferForDfu: Bool = false
    public var isDfuOtaUploading: Bool {
        return isOtaTransferForDfu
    }
    private var isOtaAbortForDfu: Bool = false
    public static var meshDfuState: Int = MeshDfuState.MESH_DFU_STATE_INIT
    public var isMeshDfuIdle: Bool {
        get {
            if OtaUpgrader.meshDfuState == MeshDfuState.MESH_DFU_STATE_INIT ||
                OtaUpgrader.meshDfuState == MeshDfuState.MESH_DFU_STATE_COMPLETE ||
                OtaUpgrader.meshDfuState == MeshDfuState.MESH_DFU_STATE_FAILED {
                return true
            }
            return false
        }
    }

    public class func otaDfuProcessCompleted() {
        OtaUpgrader.storeActiveDfuInfo(dfuType: nil, fwImageFileName: nil, fwMetadataFileName: nil)
    }
    public class func storeActiveDfuInfo(dfuType: Int?, fwImageFileName: String?, fwMetadataFileName: String?) {
        OtaUpgrader.activeDfuType = dfuType
        OtaUpgrader.activeDfuFwImageFileName = fwImageFileName
        OtaUpgrader.activeDfuFwMetadataFileName = fwMetadataFileName
    }

    private var otaDevice: OtaDeviceProtocol?
    open var delegate: OtaDeviceProtocol? {
        get {
            return otaDevice
        }
        set {
            otaDevice = newValue
        }
    }


    private var otaCommandTimer: Timer?
    private let lock = NSLock()
    public var isOtaUpgradeRunning: Bool = false
    public var isDeviceConnected: Bool = false

    private var fwImage: Data?
    private var fwImageSize: Int = 0
    private var fwOffset: Int = 0
    private var transferringSize: Int = 0
    private var fwCrc32 = CRC32_INIT_VALUE;
    private var maxOtaPacketSize: Int = 155

    private var state: OtaState = .idle
    private var completeError: OtaError?

    private var isGetComponentInfoRunning: Bool = false

    open func otaUpgradeStatusReset() {
        lock.lock()
        isGetComponentInfoRunning = false
        isOtaUpgradeRunning = false
        isDeviceConnected = false
        completeError = nil
        state = .idle
        OtaUpgrader.meshDfuState = MeshDfuState.MESH_DFU_STATE_INIT
        otaUpgradeResetOtaDevice()
        lock.unlock()
    }

    open func otaUpgradeResetOtaDevice() {
        otaDevice?.otaDeviceHasConnected = false
        otaDevice?.otaDevice = nil
        otaDevice?.otaService = nil
        otaDevice?.otaControlPointCharacteristic = nil
        otaDevice?.otaDataCharacteristic = nil
        otaDevice?.otaAppInfoCharacteristic = nil
    }

    open func dumpOtaUpgradeStatus() {
        meshLog("dumpOtaUpgradeStatus, otaState:\(state.description), isOtaUpgradeRunning:\(isOtaUpgradeRunning), isDeviceConnected:\(isDeviceConnected)")
    }

    /* Default Cypress WICED OTA */
    private func otaUpgradeWicedOtaStart() -> Int
    {
        guard let targetOtaDevice = self.otaDevice else {
            meshLog("error: OtaUpgrader, otaUpgradeWicedOtaStart, invalid target OTA device, self.otaDevice=nil")
            OtaNotificationData.init(otaError: OtaError(state: .idle, code: OtaErrorCode.INVALID_OBJECT_INSTANCES, desc: "target OTA device instance is nil")).post()
            return MeshErrorCode.MESH_ERROR_INVALID_ARGS
        }
        guard let _ = self.fwImage, self.fwImageSize > 0 else {
            meshLog("error: OtaUpgrader, otaUpgradeWicedOtaStart, invalid firmware image, fwImageSize=\(self.fwImageSize)")
            OtaNotificationData.init(otaError: OtaError(state: .idle, code: OtaErrorCode.INVALID_FW_IMAGE, desc: "invalid firmware image data")).post()
            return MeshErrorCode.MESH_ERROR_INVALID_ARGS
        }

        meshLog("OtaUpgrader, otaUpgradeWicedOtaStart, start WICED OTA for device: \(targetOtaDevice.getDeviceName())")
        OtaNotificationData.init(otaError: OtaError(state: .idle, code: OtaErrorCode.SUCCESS, desc: "WICED OTA upgrade started")).post()
        DispatchQueue.main.async {
            self.state = .idle
            self.stateMachineProcess()
        }

        // Now, OTA upgrade processing has been started, progress status will be updated through OtaConstants.Notification.OTA_COMPLETE_STATUS notificaitons.
        return OtaErrorCode.SUCCESS
    }

    open func otaUpgradeDfuStart(for device: OtaDeviceProtocol, dfuType: Int, fwImage: Data, dfuMetadata: OtaDfuMetadata? = nil)
    {
        guard isMeshDfuIdle, (self.state == .idle || self.state == .complete) else {
            meshLog("error: OtaUpgrader, otaUpgradeDfuStart, DFU already started")
            OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.BUSYING, desc: "DFU process already started")).post()
            return
        }
        if dfuMetadata == nil, dfuType != MeshDfuType.APP_OTA_TO_DEVICE {
            meshLog("error: OtaUpgrader, otaUpgradeDfuStart, invalid metadata, nil")
            let dfuTypeString = (dfuType == MeshDfuType.APP_DFU_TO_ALL) ? "APP_DFU_TO_ALL" : "PROXY_DFU_TO_ALL"
            OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.BUSYING, desc: "start DFU failed, no metadata found for dfuType: \(dfuTypeString)")).post()
            return
        }

        self.otaDevice = device
        self.fwImage = fwImage
        self.dfuType = dfuType
        self.dfuMetadata = dfuMetadata

        self.fwOffset = 0
        self.fwImageSize = fwImage.count
        self.transferringSize = 0
        self.fwCrc32 = CRC32_INIT_VALUE
        self.maxOtaPacketSize = OtaUpgrader.getMaxDataTransferSize(deviceType: device.getDeviceType())
        self.completeError = nil

        if device.getDeviceType() == .ble {
            // do AIROC OTA process for LE device.
            let _ = self.otaUpgradeWicedOtaStart()
            return
        }

        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: OtaUpgrader, otaUpgradeDfuStart, failed to connect to the mesh network, error: \(error)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "failed to connect to mesh network")).post()
                return
            }

            if dfuType == MeshDfuType.APP_OTA_TO_DEVICE {
                // do WICED OTA process. Default OTA method for LE or specific Mesh Device only.
                let _ = self.otaUpgradeWicedOtaStart()
            } else {
                // do DFU process.
                OtaUpgrader.meshDfuState = MeshDfuState.MESH_DFU_STATE_INIT
                let error = MeshFrameworkManager.shared.meshClientDfuStart(dfuMethod: dfuType, firmwareId: dfuMetadata!.firwmareId, metadata: dfuMetadata!.metadata)
                if error == MeshErrorCode.MESH_SUCCESS {
                    OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "DFU process started success")).post()
                    // let mesh library keep reporting the DFU status.
                    let dfuGetStatusRet = MeshFrameworkManager.shared.meshClientDfuGetStatus()
                    if dfuGetStatusRet != MeshErrorCode.MESH_SUCCESS {
                        meshLog("warning: OtaUpgrader, otaUpgradeDfuStart, failed to start geting DFU status, error: \(dfuGetStatusRet)")
                        OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "warning: failed to start DFU status reporting")).post()
                    } else {
                        meshLog("OtaUpgrader, otaUpgradeDfuStart, call geting DFU status started success, internval: \(Int(DFU_DISTRIBUTION_STATUS_TIMEOUT))")
                        OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "start DFU status reporting success")).post()
                    }
                } else {
                    meshLog("error: OtaUpgrader, otaUpgradeDfuStart, failed to start DFU process, error: \(error)")
                    OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "failed to start DFU process")).post()
                }
            }
        }
    }

    open func otaUpgradeDfuStop()
    {
        guard !self.isOtaAbortForDfu else {
            return  // stop command has been sent.
        }

        // Try to abort the WICED OTA process.
        if self.otaDevice?.otaDeviceHasConnected ?? false,
            self.state.rawValue >= OtaState.prepareForDownload.rawValue,
            self.state.rawValue < OtaState.abort.rawValue {
            meshLog("OtaUpgrader, otaUpgradeDfuStop, abort WICED OTA process, current state=\(self.state)")
            DispatchQueue.main.async {
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "aborting WICED OTA process")).post()
                meshLog("OtaUpgrader, otaUpgradeDfuStop, abort WICED OTA process, set to abort state=\(OtaState.abort)")
                if self.dfuType != MeshDfuType.APP_OTA_TO_DEVICE {
                    self.isOtaAbortForDfu = true
                }
                self.state = .abort
                self.stateMachineProcess()
            }
        }

        // Try to stop any mesh DFU process.
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: OtaUpgrader, otaUpgradeDfuStop, failed to connect to mesh network, error=\(error)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.ERROR_DFU_MESH_NETWORK_NOT_CONNECTED, desc: "failed to connect to mesh network")).post()
                return
            }

            let error = MeshFrameworkManager.shared.meshClientDfuStop()
            if error == MeshErrorCode.MESH_SUCCESS {
                meshLog("OtaUpgrader, otaUpgradeDfuStop, stop DFU process success")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "DFU process stopped succcess")).post()
            } else {
                meshLog("error: OtaUpgrader, otaUpgradeDfuStop, failed to stop DFU process, error=\(error)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "failed to stop DFU process")).post()
            }

            let getDfuStatusError = MeshFrameworkManager.shared.meshClientDfuGetStatus(interval: 0)
            if error == MeshErrorCode.MESH_SUCCESS {
                meshLog("OtaUpgrader, otaUpgradeDfuStop, stop DFU status reporting success")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "stop DFU status reporting success")).post()
            } else {
                meshLog("error: OtaUpgrader, otaUpgradeDfuStop, failed to stop DFU status reporting, error=\(getDfuStatusError)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "failed to stop DFU status reporting")).post()
            }

            OtaUpgrader.meshDfuState = MeshDfuState.MESH_DFU_STATE_INIT
        }
    }

    open func otaUpgradeGetDfuStatus() {
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: OtaUpgrader, otaUpgradeGetDfuStatus, failed to connect to mesh network, error=\(error)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "get DFU status, failed to connect to mesh network")).post()
                return
            }

            let error = MeshFrameworkManager.shared.meshClientDfuGetStatus()
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: OtaUpgrader, otaUpgradeGetDfuStatus, failed to start get DFU status reporting, error=\(error)")
                OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: error, desc: "failed to start DFU status reporting")).post()
                return
            }
            meshLog("OtaUpgrader, otaUpgradeGetDfuStatus, start get DFU status reporting success")
            OtaNotificationData.init(otaState: .dfuCommand, otaError: OtaError(code: OtaErrorCode.SUCCESS, desc: "start DFU status reporting success")).post()
        }
    }

    /**
     * API for DFU firmware upgrade process to upload the firmware image to DFU distributior.
     * This API will be invoked by the mesh library callback API to start the firmware image uploading
     * after called the isOtaSupportedForDfu() for checked the OTA is supported.
     */
    open func startOtaTransferForDfu() {
        // The DFU distributor has been selected, and use WICED OTA process to upload the new firmware image to the distributor.
        guard let otaDevice = self.otaDevice, otaDevice.otaDeviceHasConnected,
            let _ = otaDevice.otaDevice as? CBPeripheral,
            let _ = otaDevice.otaControlPointCharacteristic as? CBCharacteristic,
            let _ = otaDevice.otaDataCharacteristic as? CBCharacteristic else {
                meshLog("error: OtaUpgrader, startOtaTransferForDfu, invaid otaDevice instance or not connected or not support OTA state,  otaDevice:\(String(describing: self.otaDevice))")
            return
        }

        DispatchQueue.main.async {
            meshLog("OtaUpgrader, startOtaTransferForDfu, try to upload new firmware to distributor")
            self.isOtaTransferForDfu = true
            self.state = .enableNotification
            self.stateMachineProcess()
        }
    }

    /**
     * API for DFU firmware upgrade process to check if the target device support WICED firmware OTA process or not.
     * Bceause the DFU firmware upgrade process also use the WICED firmware OTA process to upload the new firmware image to DFU distributor.
     * This API will be invoked by the mesh library callback API.
     */
    open func isOtaSupportedForDfu() -> Bool {
        guard let connectedPeripheral = MeshNativeHelper.getCurrentConnectedPeripheral() else {
            meshLog("error: OtaUpgrader, isOtaSupportedForDfu, no LE/Mesh device found connected, return false")
            return false
        }
        let otaControlPointUuids = [OtaConstants.BLE.UUID_CHARACTERISTIC_CONTROL_POINT, OtaConstants.BLE_V2.UUID_CHARACTERISTIC_CONTROL_POINT]
        let otaDataUuids = [OtaConstants.BLE.UUID_CHARACTERISTIC_DATA, OtaConstants.BLE_V2.UUID_CHARACTERISTIC_DATA]
        guard let otaService = connectedPeripheral.services?.filter({OtaConstants.UUID_GATT_OTA_SERVICES.contains($0.uuid)}).first,
            let otaCharacteristics = otaService.characteristics, otaCharacteristics.count >= 2,
            let otaControlPoint = otaCharacteristics.filter({otaControlPointUuids.contains($0.uuid)}).first,
            let otaData = otaCharacteristics.filter({otaDataUuids.contains($0.uuid)}).first else {
                meshLog("error: OtaUpgrader, isOtaSupportedForDfu, no OTA service and characteristics found, return false")
                return false
        }

        // The OtaManager.shared.activeOtaDevice instance should be the same instance as the OtaUpgrader.shared.otaDevice.
        if OtaUpgrader.shared.otaDevice == nil {
            OtaUpgrader.shared.otaDevice = OtaMeshDevice(meshName: "", peripheral: connectedPeripheral)
            OtaManager.shared.activeOtaDevice = OtaUpgrader.shared.otaDevice
        }
        OtaUpgrader.shared.otaDevice!.otaDeviceHasConnected = true
        OtaUpgrader.shared.otaDevice!.otaService = otaService
        OtaUpgrader.shared.otaDevice!.otaControlPointCharacteristic = otaControlPoint
        OtaUpgrader.shared.otaDevice!.otaDataCharacteristic = otaData
        meshLog("OtaUpgrader, isOtaSupportedForDfu, found OTA service and characteristics, return true")
        return true
    }

    ///
    /// Interfaces for receiving notification or response data from remote device.
    ///
    open func didUpdateConnectionState(isConnected: Bool, error: Error?) {
        DispatchQueue.main.async {
            self._didUpdateConnectionState(isConnected: isConnected, error: error)
        }
    }
    open func _didUpdateConnectionState(isConnected: Bool, error: Error?) {
        guard isOtaUpgradeRunning, state != .idle else {
            otaDevice?.otaDeviceHasConnected = false
            otaDevice?.otaService = nil
            otaDevice?.otaControlPointCharacteristic = nil
            otaDevice?.otaDataCharacteristic = nil
            otaDevice?.otaAppInfoCharacteristic = nil
            return
        }

        /*
         * [Dudley] test purpose.
         * some old device, there no response data for the verify command, and the device will reset itself after about 2 seconds when verify succes,
         * when verify failed, the verify response with failure status will be received immeidately.
         * so, here process the disconnection event as verify success and the upgrade process has been successfully done in firmware side.
         * For new devices, it should be removed if not required.
         */
        if state == .verify,  otaCommandTimer?.isValid ?? false, !isConnected {
            otaVerifyResponse(data: Data(repeating: 0, count: 1), error: nil)
            otaDevice?.otaDeviceHasConnected = false
            return
        }

        stopOtaCommandTimer()
        guard error == nil, isConnected else {
            otaDevice?.otaDeviceHasConnected = false
            otaDevice?.otaService = nil
            otaDevice?.otaControlPointCharacteristic = nil
            otaDevice?.otaDataCharacteristic = nil
            otaDevice?.otaAppInfoCharacteristic = nil
            if error != nil {
                meshLog("error: OtaUpgrader, didUpdateConnectionState, unexpected disconnect from or failed to connect to remote OTA device, error:\(error!)")
                let errorDomain = (error! as NSError).domain
                let errorCode = (error! as NSError).code
                if errorCode == CBError.Code.connectionTimeout.rawValue, errorDomain == CBErrorDomain {
                    completeError = OtaError(state: state, code: OtaErrorCode.ERROR_DEVICE_CONNECT, desc: "The connection has timed out unexpectedly.")
                } else {
                    completeError = OtaError(state: state, code: OtaErrorCode.ERROR_DEVICE_CONNECT, desc: "disconnected from remote device or failed to connect to the remote device")
                }
            } else {
                meshLog("error: OtaUpgrader, didUpdateConnectionState, disconnected from remote OTA device")
                completeError = OtaError(state: state, code: OtaErrorCode.ERROR_DEVICE_DISCONNECT, desc: "disconnect from remote device")
            }
            OtaNotificationData(otaError: completeError!).post()
            if state.rawValue > OtaState.enableNotification.rawValue {
                state = .abort
            } else {
                state = .complete
            }
            stateMachineProcess()
            return
        }

        otaDevice?.otaDeviceHasConnected = true
        otaDevice?.otaDevice = MeshNativeHelper.getCurrentConnectedPeripheral()
        OtaNotificationData(otaState: state, otaError: nil).post()
        // device has connected, discovery OTA service. Delay 500ms for connecting stable and MESH service discover completed.
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .milliseconds(500)) {
            self.state = .otaServiceDiscover
            self.stateMachineProcess()
        }
    }

    /*
     * The OTA adapter must call this interface when the OTA service and charactieristics discovering has completed.
     * After the OTA service has been discovered, the OTA adapter should save those instacen for later usage.
     */
    open func didUpdateOtaServiceCharacteristicState(isDiscovered: Bool, error: Error?) {
        DispatchQueue.main.async {
            self._didUpdateOtaServiceCharacteristicState(isDiscovered: isDiscovered, error: error)
        }
    }
    open func _didUpdateOtaServiceCharacteristicState(isDiscovered: Bool, error: Error?) {
        // The WICED OTA process not running, ignore it.
        guard isOtaUpgradeRunning, isDeviceConnected, state.rawValue == OtaState.otaServiceDiscover.rawValue else {
            return
        }

        stopOtaCommandTimer()
        guard error == nil, isDiscovered else {
            OtaManager.shared.dumpOtaStatus()
            if error != nil {
                meshLog("error: OtaUpgrader, didUpdateOtaServiceCharacteristicState, failed to discover OTA GATT service, error:\(error!)")
                completeError = OtaError(state: state, code: OtaErrorCode.ERROR_DISCOVER_SERVICE, desc: "discover OTA service with error")
            } else {
                meshLog("error: OtaUpgrader, didUpdateOtaServiceCharacteristicState, no OTA GATT service discovered from remote OTA device")
                completeError = OtaError(state: state, code: OtaErrorCode.ERROR_DEVICE_OTA_NOT_SUPPORTED, desc: "no OTA service discovered")
            }
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        OtaNotificationData(otaState: state, otaError: nil).post()
        self.state = .enableNotification
        stateMachineProcess()
    }

    /*
     * The OTA adapter must call this interface when the notification states of the OTA service Control Pointer characteristic is udpated,
     */
    open func didUpdateNotificationState(isEnabled: Bool, error: Error?) {
        DispatchQueue.main.async {
            self._didUpdateNotificationState(isEnabled: isEnabled, error: error)
        }
    }
    open func _didUpdateNotificationState(isEnabled: Bool, error: Error?) {
        stopOtaCommandTimer()

        guard isOtaUpgradeRunning, isDeviceConnected, state != .idle else {
            return
        }

        guard error == nil, isEnabled else {
            OtaManager.shared.dumpOtaStatus()
            if error != nil {
                meshLog("error: OtaUpgrader, didUpdateNotificationState, failed to enable OTA Control Point characteristic notification, error:\(error!)")
                completeError = OtaError(state: state, code: OtaErrorCode.ERROR_CHARACTERISTIC_NOTIFICATION_UPDATE, desc: "enable notification with error")
            } else {
                meshLog("error: OtaUpgrader, didUpdateOtaServiceCharacteristicState, OTA Control Point characteristic notification not enabled")
                completeError = OtaError(state: state, code: OtaErrorCode.ERROR_CHARACTERISTIC_NOTIFICATION_UPDATE, desc: "notification disabled")
            }
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        OtaNotificationData(otaState: state, otaError: nil).post()
        state = .prepareForDownload
        stateMachineProcess()
    }

    /*
     * The OTA adapter must call this interface when any data received from any OTA service characteristic,
     * inlcuding the characterisitic indication/notification data and read value data.
     */
    open func didUpdateValueFor(characteristic: OtaCharacteristic, value: Data?, error: Error?) {
        DispatchQueue.main.async {
            self._didUpdateValueFor(characteristic: characteristic, value: value, error: error)
        }
    }
    open func _didUpdateValueFor(characteristic: OtaCharacteristic, value: Data?, error: Error?) {
        guard isOtaUpgradeRunning, isDeviceConnected, state != .idle else {
            return
        }

        switch state {
        case .readAppInfo:
            otaReadAppInfoResponse(data: value, error: error)
        case .prepareForDownload:
            otaPrepareForDownloadResponse(data: value, error: error)
        case .startDownload:
            otaStartDownloadResponse(data: value, error: error)
        case .dataTransfer:
            otaTransferDataResponse(data: value, error: error)
        case .verify:
            otaVerifyResponse(data: value, error: error)
        case .abort:
            otaAbortResponse(data: value, error: error)
        default:
            meshLog("warnning: OtaUpgrader, didUpdateValueFor, state=\(state.description)")
            break
        }
    }

    private func stateMachineProcess() {
        switch self.state {
        case .idle:
            isOtaUpgradeRunning = true
            self.state = .connect
            self.stateMachineProcess()
        case .connect:
            self.otaConnect()
        case .otaServiceDiscover:
            isDeviceConnected = true
            self.discoverOtaServiceCharacteristics()
        case .readAppInfo:
            self.otaReadAppInfo()
        case .enableNotification:
            self.otaEnableNotification()
        case .prepareForDownload:
            self.otaPrepareForDownload()
        case .startDownload:
            self.otaStartDownload()
        case .dataTransfer:
            self.otaTransferData()
        case .verify:
            self.otaVerify()
        case .abort:
            self.otaAbort()
        case .complete:
            self.otaCompleted()
            lock.lock()
            otaDevice?.otaDeviceHasConnected = false
            isDeviceConnected = false
            isOtaUpgradeRunning = false
            isGetComponentInfoRunning = false
            lock.unlock()
            meshLog("OtaUpgrader, stateMachineProcess, exit")
        default:
            meshLog("OtaUpgrader, stateMachineProcess, unexpected invalid state: \(state)")
            break
        }
    }

    private func otaConnect() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaConnect, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        otaDevice.connect()
    }

    private func discoverOtaServiceCharacteristics() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, discoverOtaServiceCharacteristics, invalid delegate:nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        otaDevice.discoverOtaServiceCharacteristic()
    }

    private func otaReadAppInfo() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaReadAppInfo, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        if otaDevice.otaAppInfoCharacteristic != nil {
            startOtaCommandTimer()
            otaDevice.readValue(from: .appInfoCharacteristic)
        } else {
            self.otaReadAppInfoResponse(data: nil, error: nil)
        }
    }

    private func otaReadAppInfoResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()

        // alawys ignore the readAppInfo error status, because many device doesn't support AppInfo characateristic at all.
        if let appInfoData = data {
            if appInfoData.count == 4 {
                let appId  = UInt16(UInt8(appInfoData[0])) + (UInt16(UInt8(appInfoData[1])) << 8)
                let appVerMajor = UInt8(appInfoData[2])
                let appVerMinor = UInt8(appInfoData[3])
                let appInfoString = String(format: "AppId: 0x%04X, AppVersion: %d.%d",
                                           appId, appVerMajor, appVerMinor)
                OtaNotificationData(otaError: OtaError(state: state, code: OtaErrorCode.SUCCESS, desc: appInfoString)).post()
            } else if appInfoData.count == 5 {
                let appId  = UInt16(UInt8(appInfoData[0])) + (UInt16(UInt8(appInfoData[1])) << 8)
                let appVerPrefixNumber = UInt8(appInfoData[2])
                let appVerMajor = UInt8(appInfoData[3])
                let appVerMinor = UInt8(appInfoData[4])
                let appInfoString = String(format: "AppId: 0x%04X, AppVersion: %d.%d.%d",
                                           appId, appVerPrefixNumber, appVerMajor, appVerMinor)
                OtaNotificationData(otaError: OtaError(state: state, code: OtaErrorCode.SUCCESS, desc: appInfoString)).post()
            }
        } else {
            if let otaDevice = self.otaDevice, otaDevice.getDeviceType() == .mesh {
                if self.isGetComponentInfoRunning, self.otaCommandTimer?.isValid ?? false {
                    return  // avoid the getComponentInfo command send multiple times.
                }

                if !MeshFrameworkManager.shared.isMeshNetworkConnected() {
                    self.state = .enableNotification
                    self.stateMachineProcess()
                    return
                }

                startOtaCommandTimer()
                self.isGetComponentInfoRunning = true
                MeshFrameworkManager.shared.getMeshComponentInfo(componentName: otaDevice.getDeviceName()) { (componentName: String, componentInfo: String?, error: Int) in
                    self.isGetComponentInfoRunning = false
                    self.stopOtaCommandTimer()
                    if error == MeshErrorCode.MESH_SUCCESS, let componentInfo = componentInfo {
                        OtaNotificationData(otaError: OtaError(state: self.state, code: OtaErrorCode.SUCCESS, desc: componentInfo)).post()
                    }

                    self.state = .enableNotification
                    self.stateMachineProcess()
                }
                return
            }
        }

        state = .enableNotification
        stateMachineProcess()
    }

    private func otaEnableNotification() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaEnableNotification, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        meshLog("OtaUpgrader, try to enable notification")
        otaDevice.enableOtaNotification(enabled: true)
    }

    private func otaPrepareForDownload() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaPrepareForDownload, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        meshLog("OtaUpgrader, prepare download")
        var otaCommand = OtaCommandData(command: .prepareDownload)
        if dfuType != MeshDfuType.APP_OTA_TO_DEVICE, let metadata = self.dfuMetadata {
            otaCommand = OtaCommandData(command: .prepareDownload, companyId: metadata.companyId, firmwareId: metadata.firwmareId)
        }
        meshLog("OtaUpgrader, otaPrepareForDownload, OTA_VERSION_\(otaDevice.otaVersion), otaCommand.value.count=\(otaCommand.value.count)")
        otaDevice.writeValue(to: .controlPointCharacteristic, value: otaCommand.value) { (data, error) in
            guard error == nil else {
                self.otaPrepareForDownloadResponse(data: data, error: error)
                return
            }
        }
    }

    private func otaPrepareForDownloadResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()
        guard error == nil else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaPrepareForDownload, failed to write Prepare for Download command, error:\(error!)")
            completeError = OtaError(state: state, code: OtaErrorCode.FAILED, desc: "failed to write Prepare for Download command")
            OtaNotificationData(otaError: completeError!).post()
            state = .complete
            stateMachineProcess()
            return
        }

        let status = OtaCommandStatus.parse(from: data)
        if status == .success {
            OtaNotificationData(otaState: state, otaError: nil).post()
            state = .startDownload
        } else {
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_RESPONSE_VALUE, desc: "failed to execute Prepare for Download command, response: \(status.description())")
            OtaNotificationData(otaState: state, otaError: completeError).post()
            state = .complete
        }
        stateMachineProcess()
    }

    private func otaStartDownload() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaStartDownload, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .abort
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        meshLog("OtaUpgrader, start download")
        let otaCommand = OtaCommandData(command: .startDownload, lParam: UInt32(fwImageSize))
        otaDevice.writeValue(to: .controlPointCharacteristic, value: otaCommand.value) { (data, error) in
            guard error == nil else {
                self.otaStartDownloadResponse(data: data, error: error)
                return
            }
        }
    }

    private func otaStartDownloadResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()
        guard error == nil else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaStartDownload, failed to write Start Download command, error:\(error!)")
            completeError = OtaError(state: state, code: OtaErrorCode.FAILED, desc: "failed to write Start Download command")
            OtaNotificationData(otaError: completeError!).post()
            state = .abort
            stateMachineProcess()
            return
        }

        let status = OtaCommandStatus.parse(from: data)
        meshLog("OtaUpgrader, otaStartDownloadResponse, status:\(status.description())")
        if status == .success {
            OtaNotificationData(otaState: state, otaError: nil).post()
            state = .dataTransfer
        } else {
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_RESPONSE_VALUE, desc: "ota start download response with failure")
            OtaNotificationData(otaState: state, otaError: completeError).post()
            state = .abort
        }
        stateMachineProcess()
    }

    private func otaTransferData() {
        guard let otaDevice = self.otaDevice, let fwImage = self.fwImage else {
            OtaManager.shared.dumpOtaStatus()
            if self.otaDevice == nil {
                meshLog("error: OtaUpgrader, otaTransferData, otaDevice instance is nil")
                completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            } else {
                meshLog("error: OtaUpgrader, otaTransferData, invalid fwImage nil")
                completeError = OtaError(state: state, code: OtaErrorCode.INVALID_FW_IMAGE, desc: "fw image data is nil")
            }
            OtaNotificationData(otaError: completeError!, fwImageSize: fwImageSize, transferredImageSize: fwOffset).post()
            state = .abort
            stateMachineProcess()
            return
        }

        if fwOffset == 0 {
            // send notifcation that indicate tranferring started.
            OtaNotificationData(otaState: state, otaError: nil, fwImageSize: fwImageSize, transferredImageSize: fwOffset).post()
        }

        let transferSize = fwImageSize - fwOffset
        transferringSize = (transferSize > maxOtaPacketSize) ? maxOtaPacketSize : transferSize
        if transferringSize > 0 {
            let range: Range = fwOffset..<(fwOffset + transferringSize)
            let transferData = fwImage.subdata(in: range)
            fwCrc32 = OtaUpgrader.calculateCrc32(crc32: fwCrc32, data: transferData)
            if (fwOffset + transferringSize) >= fwImageSize {
                fwCrc32 ^= CRC32_INIT_VALUE     // this is the last packet, get final calculated fw image CRC value.
            }
            meshLog("OtaUpgrader, otaTransferData, fwImageSize:\(fwImageSize), write at offset:\(fwOffset), size:\(transferringSize)")
            startOtaCommandTimer()
            otaDevice.writeValue(to: .dataCharacteristic, value: transferData, completion: self.otaTransferDataResponse)
        } else {
            meshLog("warnning: OtaUpgrader, otaTransferData, no more data for transferring, fwImageSize:\(fwImageSize), offset:\(fwOffset)")
            stopOtaCommandTimer()
            fwOffset = fwImageSize
            if !isOtaTransferForDfu {
                OtaNotificationData(otaState: state, otaError: nil, fwImageSize: self.fwImageSize, transferredImageSize: fwOffset).post()
            }

            state = .verify
            stateMachineProcess()
        }
    }

    private func otaTransferDataResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()
        guard error == nil else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaTransferData, failed to write transfer data, error:\(error!)")
            completeError = OtaError(state: state, code: OtaErrorCode.FAILED, desc: "failed to write transfer data")
            if !isOtaTransferForDfu {
                OtaNotificationData(otaError: completeError!, fwImageSize: fwImageSize, transferredImageSize: fwOffset).post()
            }
            state = .abort
            stateMachineProcess()
            return
        }

        fwOffset += transferringSize
        if !isOtaTransferForDfu {
            OtaNotificationData(otaState: state, otaError: nil, fwImageSize: self.fwImageSize, transferredImageSize: fwOffset).post()
        }

        if fwOffset >= fwImageSize {
            meshLog("OtaUpgrader, otaTransferData, fwImageSize:\(fwImageSize), totally transferred size:\(fwOffset), done")
            state = .verify
        }
        stateMachineProcess()
    }

    private func otaVerify() {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaVerify, otaDevice instance is nil")
            completeError = OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")
            OtaNotificationData(otaError: completeError!).post()
            state = .abort
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        let otaCommand = OtaCommandData(command: .verify, lParam: UInt32(fwCrc32))
        meshLog("OtaUpgrader, otaVerify, CRC32=\(String.init(format: "0x%X", fwCrc32))")
        otaDevice.writeValue(to: .controlPointCharacteristic, value: otaCommand.value) { (data, error) in
            guard error == nil else {
                self.otaVerifyResponse(data: data, error: error)
                return
            }
        }
    }

    private func otaVerifyResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()
        guard error == nil else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaVerify, failed to write Verify command, error:\(error!)")
            completeError = OtaError(state: state, code: OtaErrorCode.FAILED, desc: "failed to write Verify command")
            OtaNotificationData(otaError: completeError!).post()
            state = .abort
            stateMachineProcess()
            return
        }

        let status = OtaCommandStatus.parse(from: data)
        meshLog("OtaUpgrader, otaVerifyResponse, status:\(status.description())")
        if status == .success {
            OtaNotificationData(otaState: state, otaError: nil).post()
            state = .complete
        } else {
            completeError = OtaError(state: state, code: OtaErrorCode.ERROR_OTA_VERIFICATION_FAILED, desc: "firmware downloaded image CRC32 verification failed")
            OtaNotificationData(otaState: state, otaError: completeError).post()
            state = .abort
        }
        stateMachineProcess()
    }

    private func otaAbort(manuallyStopped: Bool = false) {
        guard let otaDevice = self.otaDevice else {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaAbort, otaDevice instance is nil")
            //OtaNotificationData(otaError: OtaError(state: state, code: OtaErrorCode.INVALID_PARAMETERS, desc: "otaDevice instance is nil")).post()
            state = .complete
            stateMachineProcess()
            return
        }

        startOtaCommandTimer()
        let otaCommand = OtaCommandData(command: .abort, lParam: UInt32(fwImageSize))
        otaDevice.writeValue(to: .controlPointCharacteristic, value: otaCommand.value) { (data, error) in
            guard error == nil else {
                self.otaAbortResponse(data: data, error: error)
                return
            }
        }
    }

    private func otaAbortResponse(data: Data?, error: Error?) {
        stopOtaCommandTimer()

        var abortError: OtaError?
        if let error = error {
            OtaManager.shared.dumpOtaStatus()
            meshLog("error: OtaUpgrader, otaAbortResponse, failed to write Abort command, error:\(error)")
            abortError = OtaError(state: state, code: OtaErrorCode.FAILED, desc: "failed to write Abort command")
        } else {
            let status = OtaCommandStatus.parse(from: data)
            meshLog("OtaUpgrader, otaAbortResponse, response status:\(status.description())")
            if status == .success {
                abortError = OtaError(state: state, code: OtaErrorCode.ERROR_OTA_ABORTED, desc: "OTA processed has been aborted")
            } else {
                abortError = OtaError(state: state, code: OtaErrorCode.INVALID_RESPONSE_VALUE, desc: "failed abort OTA process, status: \(status.description())")
            }
        }
        completeError = completeError ?? abortError
        OtaNotificationData(otaState: state, otaError: nil).post()

        state = .complete
        stateMachineProcess()
    }

    private func otaCompleted() {
        stopOtaCommandTimer()
        OtaManager.shared.dumpOtaStatus()

        if let cmptError = completeError {
            meshLog("error: OtaUpgrader, otaCompleted, WICED OTA completed with error: (\(cmptError.code), \(cmptError.description))")
        } else {
            meshLog("OtaUpgrader, otaCompleted, WICED OTA completed with success")
        }
        OtaNotificationData(otaState: .complete, otaError: completeError).post()

        if self.isOtaTransferForDfu {
            if let _ = self.completeError {
                MeshFrameworkManager.shared.meshClientDfuOtaFinish(status: MeshErrorCode.MESH_ERROR_INVALID_STATE)   // finished with some error encountered.
            } else {
                MeshFrameworkManager.shared.meshClientDfuOtaFinish(status: MeshErrorCode.MESH_SUCCESS)   // finished with success status.
            }
            self.isOtaTransferForDfu = false
        }
        if self.isOtaAbortForDfu || completeError != nil {
            let isAborting = self.isOtaAbortForDfu
            // Try to stop mesh DFU process if encountered any error.
            OtaUpgrader.shared.otaUpgradeDfuStop()
            if isAborting  {
                self.isOtaAbortForDfu = false
            }
        }
    }
}

extension OtaUpgrader {
    public enum OtaState: Int {
        case idle = 0
        case connect = 1
        case otaServiceDiscover = 2
        case readAppInfo = 3
        case enableNotification = 4
        case prepareForDownload = 5
        case startDownload = 6
        case dataTransfer = 7
        case verify = 8
        case abort = 9
        case complete = 10
        case dfuCommand = 11

        public var description: String {
            switch self {
            case .idle:
                return "idle"
            case .connect:
                return "connect"
            case .otaServiceDiscover:
                return "otaServiceDiscover"
            case .readAppInfo:
                return "readAppInfo"
            case .enableNotification:
                return "enableNotification"
            case .prepareForDownload:
                return "prepareForDownload"
            case .startDownload:
                return "startDownload"
            case .dataTransfer:
                return "dataTransfer"
            case .verify:
                return "verify"
            case .abort:
                return "abort"
            case .complete:
                return "complete"
            case .dfuCommand:
                return "dfuCommand"
            }
        }
    }

    private enum OtaCommand: Int {
        case prepareDownload = 1
        case startDownload = 2
        case verify = 3
        case finish = 4
        case getStatus = 5      // not currently used
        case clearStatus = 6    // not currently used
        case abort = 7
        case apply = 8
    }

    private struct OtaCommandData {
        // dataSize: 4 bytes; command: 1 byte; parameters: max 4 bytes
        private var bytes: [UInt8]
        var value: Data {
            return Data(bytes)
        }
        var count: Int {
            return bytes.count
        }

        init(command: OtaCommand) {
            let dataSize = 1
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
        }

        // The fwID data must be DFU_FW_ID_LENGTH (8) bytes long.
        init(command: OtaCommand, companyId: UInt16, firmwareId: Data) {
            let dataSize = 1 + 2 + firmwareId.count
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
            bytes[1] = UInt8((companyId >> 8) & 0xFF)
            bytes[2] = UInt8(companyId & 0xFF)
            for i in 0..<firmwareId.count {
                bytes[3 + i] = UInt8(firmwareId[i])
            }
        }

        init(command: OtaCommand, sParam: UInt16) {
            let dataSize = 3
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
            bytes[1] = UInt8(sParam & 0xFF)
            bytes[2] = UInt8((sParam >> 8) & 0xFF)
        }

        init(command: OtaCommand, lParam: UInt32) {
            let dataSize = 5
            bytes = [UInt8](repeating: 0, count: dataSize)
            bytes[0] = UInt8(command.rawValue)
            bytes[1] = UInt8(lParam & 0xFF)
            bytes[2] = UInt8((lParam >> 8) & 0xFF)
            bytes[3] = UInt8((lParam >> 16) & 0xFF)
            bytes[4] = UInt8((lParam >> 24) & 0xFF)
        }
    }

    // OTA Command Response status.
    private enum OtaCommandStatus: UInt8 {
        case success = 0
        case unsupported = 1
        case illegal = 2
        case verificationFailed = 3
        case invalidImage = 4
        case invalidImageSize = 5
        case moreData = 6
        case invalidAppId = 7
        case invalidVersion = 8
        case continueStatus = 9
        case invalidParameters = 10
        case sendCommandFailed = 11
        case timeout = 12
        case commandResponseError = 13

        static func parse(from data: Data?) -> OtaCommandStatus {
            var status: OtaCommandStatus = .unsupported
            if let respData = data, respData.count > 0 {
                status = OtaCommandStatus.init(rawValue: UInt8(respData[0])) ?? .unsupported
            }
            return status
        }

        func description() -> String {
            switch self {
            case .success:
                return "success"
            case .unsupported:
                return "unsupported command"
            case .illegal:
                return "illegal state"
            case .verificationFailed:
                return "image varification failed"
            case .invalidAppId:
                return "invalid App Id"
            case .invalidImage:
                return "invalid image"
            case .invalidImageSize:
                return "invalid image size"
            case .invalidVersion:
                return "invalid version"
            case .moreData:
                return "more data"
            case .continueStatus:
                return "continue"
            case .sendCommandFailed:
                return "failed to write command or data"
            case .invalidParameters:
                return "invalid parameters or invalid objects"
            case .timeout:
                return "timeout"
            case .commandResponseError:
                return "commandResponseError"
            }
        }
    }
}

extension OtaUpgrader {
    private func startOtaCommandTimer() {
        stopOtaCommandTimer()

        var interval: TimeInterval = 10
        if state == .connect {
            interval += TimeInterval(exactly: MeshConstants.MESH_DEFAULT_SCAN_DURATION) ?? 30.0
        } else if state == .otaServiceDiscover || state == .verify {
            interval = 30
        }

        if #available(iOS 10.0, *) {
            otaCommandTimer = Timer.scheduledTimer(withTimeInterval: interval, repeats: false, block: { (Timer) in
                self.onOtaCommandTimeout()
            })
        } else {
            otaCommandTimer = Timer.scheduledTimer(timeInterval: interval, target: self,
                                                selector: #selector(self.onOtaCommandTimeout),
                                                userInfo: nil, repeats: false)
        }
    }

    private func stopOtaCommandTimer() {
        otaCommandTimer?.invalidate()
        otaCommandTimer = nil
    }

    @objc private func onOtaCommandTimeout() {
        DispatchQueue.main.async {
            self._onOtaCommandTimeout()
        }
    }
    private func _onOtaCommandTimeout() {
        if state == .readAppInfo {
            // AppInfo read are not supported on some devices, so bypass the error if it happen.
            otaReadAppInfoResponse(data: nil, error: nil)
            return
        }

        if completeError == nil {
            completeError = OtaError(state: state, code: OtaErrorCode.TIMEOUT, desc: "execute ota command or write ota data timeout error")
        }
        OtaNotificationData.init(otaError: completeError!).post()

        switch state {
        case .idle:
            fallthrough
        case .abort:
            state = .complete
        case .connect:
            self._didUpdateConnectionState(isConnected: false, error: CBError(CBError.Code.connectionTimeout))
        case .otaServiceDiscover:
            self._didUpdateOtaServiceCharacteristicState(isDiscovered: false, error: CBError(CBError.Code.unknown))
        default:
            state = .abort
        }
        stateMachineProcess()
    }
}

extension OtaUpgrader {
    /* The max MTU size is 158 on iOS version < 10, and is 185 when iOS version >= 10. */
    static func getMaxDataTransferSize(deviceType: OtaDeviceType) -> Int {
        let mtuSize = PlatformManager.SYSTEM_MTU_SIZE
        if deviceType == .homeKit {
            return 255  // Max 255 without any error, 2 data packets with 1 ack response.
        } else if deviceType == .mesh {
            return (mtuSize - 3 - 17)   // 3 link layer header bytes, exter 17 Mesh encryption bytes
        }
        return (mtuSize - 3)    // 3 link layer header bytes
    }

    /*
     * Help function to calculate CRC32 checksum for specific data.
     *
     * @param crc32     CRC32 value for calculating.
     *                  The initalize CRC value must be CRC32_INIT_VALUE.
     * @param data      Data required to calculate CRC32 checksum.
     *
     * @return          Calculated CRC32 checksum value.
     *
     * Note, after the latest data has been calculated, the final CRC32 checksum value must be calculuated as below as last step:
     *      crc32 ^= CRC32_INIT_VALUE
     */
    static func calculateCrc32(crc32: UInt32, data: Data) -> UInt32 {
        return MeshNativeHelper.updateCrc32(crc32, data:data)
    }

    // The input @path can be a full path or a relative path under the App's "Documents" directory.
    public static func readParseFirmwareImage(at path: String) -> Data? {
        let filePath: String = path.starts(with: "/") ? path : (NSHomeDirectory() + "/Documents/" + path)
        var isDirectory = ObjCBool(false)
        let exists = FileManager.default.fileExists(atPath: filePath, isDirectory: &isDirectory)
        guard exists, !isDirectory.boolValue, let fwImageData = FileManager.default.contents(atPath: filePath), fwImageData.count > 0 else {
            meshLog("error: OtaUpgrader, readParseFirmwareImage, failed to read and parse firmware image: \(filePath)")
            #if MESH_DFU_ENABLED
            MeshNativeHelper.meshClientSetDfuFilePath(nil);
            #endif
            return nil
        }
        meshLog("OtaUpgrader, readParseFirmwareImage, fwImageData.count: \(fwImageData.count) bytes")
        #if MESH_DFU_ENABLED
        MeshNativeHelper.meshClientSetDfuFilePath(filePath);
        #endif
        return fwImageData
    }

    // The input @path can be a full path or a relative path under the App's "Documents" directory.
    // The firmwareId only supported after metadataVersion is set to 4.
    public static func readParseMetadataImage(at path: String, firmwareId: String? = nil) -> OtaDfuMetadata? {
        var cid: UInt16?    // Company ID. It's company_id in firmware's mesh_config.
        var fwId: Data?     // Firmware ID, 8 bytes when metadataVersion = 2; 10 bytes when metadataVersion >= 3.
        var pid: UInt16?    // Product ID, 2 bytes. It's product_id in firmware's mesh_config.
        var hwid: UInt16?   // HW Version ID, 2 bytes. It's vednor_id in firmware's mesh_config.
        var fwVerMaj: UInt8?
        var fwVerMin: UInt8?
        var fwVerRev: UInt16?
        var fwVerBuild: UInt16 = 0
        var metadata: Data?  // Only valid when the metadataVersion >= 3
        var metadataVersion: UInt8 = 1

        // The latest Mesh DFU metadata format, version 4, process.
        // the metadata file stored 64 bytes metadata which stored in binary data.
        // the firmware ID string is read from the manifest.json file and passed into this function.
        if let firmwareId = firmwareId {
            do {
                let documentsPath: URL? = path.starts(with: "/") ? nil : URL(fileURLWithPath: NSHomeDirectory() + "/Documents", isDirectory: true)
                let urlPath = URL(fileURLWithPath: path, isDirectory: false, relativeTo: documentsPath)
                let metadataData = try Data(contentsOf: urlPath)
                let firmwareIdString = firmwareId.trimmingCharacters(in: .whitespaces)
                guard firmwareIdString.count >= 22, let fwIdData = firmwareIdString.dataFromHexadecimalString() else {
                    meshLog("error: OtaUpgrader, readFwMetadataImageFile, firmwareId string: \(firmwareIdString)")
                    #if MESH_DFU_ENABLED
                    MeshNativeHelper.meshClientClearDfuFwMetadata()
                    #endif
                    return nil
                }
                metadataVersion = 4
                fwId = fwIdData             // the data stored in little-endian in manifest.json file.
                metadata = metadataData   // the metadata stored in metadata file is in binary format.
                cid = UInt16((UInt16(fwIdData[1]) << 8) | UInt16(fwIdData[0]))
                pid = UInt16((UInt16(fwIdData[3]) << 8) | UInt16(fwIdData[2]))
                hwid = UInt16((UInt16(fwIdData[5]) << 8) | UInt16(fwIdData[4]))
                fwVerMaj = UInt8(fwIdData[6])
                fwVerMin = UInt8(fwIdData[7])
                fwVerRev = UInt16(UInt8(fwIdData[8]))
                fwVerBuild = UInt16((UInt16(fwIdData[10]) << 8) | UInt16(fwIdData[9]))

                // Always update the siglone DFU FW metadata when new firmware image read successfully.
                guard let cid = cid, let fwId = fwId, let pid = pid, let hwid = hwid,
                    let fwVerMaj = fwVerMaj, let fwVerMin = fwVerMin, let fwVerRev = fwVerRev,
                    let metadata = metadata else {
                        meshLog("error: OtaUpgrader, readFwMetadataImageFile, failed to read metadata and parse firmwareId string")
                        #if MESH_DFU_ENABLED
                        MeshNativeHelper.meshClientClearDfuFwMetadata()
                        #endif
                        return nil
                }
                meshLog("OtaUpgrader, readFwMetadataImageFile, firmwareId(\(fwId.count)): \(fwId.dumpHexBytes())")
                meshLog("OtaUpgrader, readFwMetadataImageFile, metadata(\(metadata.count)): \(metadata.dumpHexBytes())")
                #if MESH_DFU_ENABLED
                MeshNativeHelper.meshClientSetDfuFwMetadata(fwId, metadata: metadata)
                #endif
                return (cid, fwId, pid, hwid, fwVerMaj, fwVerMin, fwVerRev, fwVerBuild, metadata, metadataVersion)
            } catch {
                meshLog("error: OtaUpgrader, readFwMetadataImageFile, failed to read : \(path)")
                #if MESH_DFU_ENABLED
                MeshNativeHelper.meshClientClearDfuFwMetadata()
                #endif
            }
            return nil
        }

        do {
            let documentsPath: URL? = path.starts(with: "/") ? nil : URL(fileURLWithPath: NSHomeDirectory() + "/Documents", isDirectory: true)
            let urlPath = URL(fileURLWithPath: path, isDirectory: false, relativeTo: documentsPath)
            let data = try String(contentsOf: urlPath, encoding: .utf8)
            let lines = data.components(separatedBy: .newlines)
            for oneline in lines {
                let line = oneline.trimmingCharacters(in: .whitespaces)
                if line.hasPrefix("CID=0x"), line.count >= 10 {
                    metadataVersion = 1
                    let index = line.index(line.startIndex, offsetBy: 6)
                    if let value = Int(line[index...], radix: 16) {
                        cid = UInt16(value)
                    }
                } else if line.hasPrefix("FWID=0x"), line.count >= 23 {
                    metadataVersion = 1
                    let index = line.index(line.startIndex, offsetBy: 7)
                    let hexString = String(line[index...])
                    fwId = hexString.dataFromHexadecimalString()  // the data stored in big-endian in image_info file.
                    if let fwIdData = fwId {
                        pid = UInt16((UInt16(fwIdData[0]) << 8) | UInt16(fwIdData[1]))
                        hwid = UInt16((UInt16(fwIdData[2]) << 8) | UInt16(fwIdData[3]))
                        fwVerMaj = UInt8(fwIdData[4])
                        fwVerMin = UInt8(fwIdData[5])
                        fwVerRev = UInt16((UInt16(fwIdData[6]) << 8) | UInt16(fwIdData[7]))
                    }
                    metadata = Data()
                } else if line.hasPrefix("Firmware ID = 0x"), line.count >= 36 {
                    if line.count == 36 {
                        metadataVersion = 2
                    } else {
                        metadataVersion = 3
                    }
                    let index = line.index(line.startIndex, offsetBy: 16)
                    let hexString = String(line[index...])
                    fwId = hexString.dataFromHexadecimalString()  // the data stored in big-endian in image_info file.
                    if let fwIdData = fwId {
                        cid = UInt16((UInt16(fwIdData[0]) << 8) | UInt16(fwIdData[1]))
                        pid = UInt16((UInt16(fwIdData[2]) << 8) | UInt16(fwIdData[3]))
                        hwid = UInt16((UInt16(fwIdData[4]) << 8) | UInt16(fwIdData[5]))
                        if metadataVersion == 2 {
                            fwVerMaj = UInt8(fwIdData[6])
                            fwVerMin = UInt8(fwIdData[7])
                            fwVerRev = UInt16((UInt16(fwIdData[8]) << 8) | UInt16(fwIdData[9]))
                        } else {
                            fwVerMaj = UInt8(fwIdData[6])
                            fwVerMin = UInt8(fwIdData[7])
                            fwVerRev = UInt16(UInt8(fwIdData[8]))
                            fwVerBuild = UInt16((UInt16(fwIdData[9]) << 8) | UInt16(fwIdData[10]))
                        }
                    }
                } else if line.hasPrefix("Metadata Data = 0x"), line.count >= 28 {
                    var check_metadataVersion: Int = 3
                    if line.count == 28 {
                        check_metadataVersion = 2
                    }
                    if metadataVersion != check_metadataVersion {
                        meshLog("error: OtaUpgrader, readFwMetadataImageFile, invalid matadata image, the Firmware ID data not match with Metadata Data. metadataVersion: \(metadataVersion), check_metadataVersion: \(check_metadataVersion)")
                        #if MESH_DFU_ENABLED
                        MeshNativeHelper.meshClientClearDfuFwMetadata()
                        #endif
                        return nil
                    }
                    let index = line.index(line.startIndex, offsetBy: 20)
                    let hexString = String(line[index...])
                    metadata = hexString.dataFromHexadecimalString()  // the data stored in big-endian in image_info file.
                }
            }

            guard let cid = cid, let fwId = fwId, let pid = pid, let hwid = hwid,
                let fwVerMaj = fwVerMaj, let fwVerMin = fwVerMin, let fwVerRev = fwVerRev,
                let metadata = metadata else {
                    meshLog("error: OtaUpgrader, readFwMetadataImageFile, invalid the matadata image.\n\(data)")
                    #if MESH_DFU_ENABLED
                    MeshNativeHelper.meshClientClearDfuFwMetadata()
                    #endif
                    return nil
            }
            // Always update the siglone DFU FW metadata when new firmware image read successfully.
            #if MESH_DFU_ENABLED
            MeshNativeHelper.meshClientSetDfuFwMetadata(fwId, metadata: metadata)
            #endif
            return (cid, fwId, pid, hwid, fwVerMaj, fwVerMin, fwVerRev, fwVerBuild, metadata, metadataVersion)
        } catch {
            meshLog("error: OtaUpgrader, readFwMetadataImageFile, failed to read \"\(path)\". \(error)")
        }
        #if MESH_DFU_ENABLED
        MeshNativeHelper.meshClientClearDfuFwMetadata()
        #endif
        return nil
    }

    public static func readDfuManifestFile(at path: String) -> (firmwareFile: String, metadataFile: String, firmwareId: String)? {
        do {
            meshLog("OtaUpgrader, readDfuManifestFile: \(path)")
            let documentsPath: URL? = path.starts(with: "/") ? nil : URL(fileURLWithPath: NSHomeDirectory() + "/Documents", isDirectory: true)
            let urlPath = URL(fileURLWithPath: path, isDirectory: false, relativeTo: documentsPath)
            let data = try Data(contentsOf: urlPath)
            let jsonData: Any = try JSONSerialization.jsonObject(with: data, options: JSONSerialization.ReadingOptions.mutableContainers)
            guard let jsonDictionary = jsonData as? Dictionary<String, Any>,
                let manifestDictionary = jsonDictionary["manifest"] as? Dictionary<String, Any>,
                let firmwareDictionary = manifestDictionary["firmware"] as? Dictionary<String, Any>,
                let firmwareFile = firmwareDictionary["firmware_file"] as? String,
                let metadataFile = firmwareDictionary["metadata_file"] as? String,
                let firmwareId = firmwareDictionary["firmware_id"] as? String,
                let baseUrlPath = urlPath.baseURL
            else {
                meshLog("error: OtaUpgrader, readDfuManifestFile, failed to read and parse the manifest file: \"\(path)\"")
                return nil
            }
            let pathComponents = path.components(separatedBy: "/")
            var pathPrefix: String = ""
            for (i, component) in pathComponents.enumerated() {
                if i == (pathComponents.count - 1) {
                    break
                } else {
                    pathPrefix += component + "/"
                }
            }
            let filewareFilePath = pathPrefix + firmwareFile    // relative path to the App's Documents path.
            let metadaaFilePath = pathPrefix + metadataFile     // relative path to the App's Documents path.
            meshLog("baseUrlPath: \(baseUrlPath)")
            meshLog("firmware_file: \(firmwareFile)")
            meshLog("metadata_file: \(metadataFile)")
            meshLog("firmware_id: \(firmwareId)")
            meshLog("filewareFilePath: \(filewareFilePath)")
            meshLog("metadaaFilePath: \(metadaaFilePath)")
            return (filewareFilePath, metadaaFilePath, firmwareId)
        } catch {
            meshLog("error: OtaUpgrader, readDfuManifestFile, failed to read manifest file: \(path). \(error)")
        }
        return nil
    }
}

extension String {
    // The hexadecimal string must be no whitespace characters between the string.
    func dataFromHexadecimalString() -> Data? {
        var hexData: Data = Data()
        var hexString = self.trimmingCharacters(in: .whitespaces)
        if (self.hasPrefix("0x") || self.hasPrefix("0X")) {
            let hexIndex = self.index(self.startIndex, offsetBy: 2)
            hexString = String(self[hexIndex...])
        }
        if (hexString.count % 2) != 0 {
            hexString = "0" + hexString
        }

        for i in stride(from: 0, to: hexString.count, by: 2) {
            let startIndex = hexString.index(hexString.startIndex, offsetBy: i)
            let endIndex = hexString.index(hexString.startIndex, offsetBy: (i+1))
            let hexByteStr = hexString[startIndex...endIndex]
            if let byteValue = UInt8(hexByteStr, radix: 16) {
                hexData.append(byteValue)
            }
        }
        return (hexData.count == 0) ? nil : Data(hexData)
    }
}
