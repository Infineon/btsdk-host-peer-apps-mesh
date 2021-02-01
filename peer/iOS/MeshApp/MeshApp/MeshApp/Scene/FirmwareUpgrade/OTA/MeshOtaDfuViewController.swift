/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
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
//
//  MeshOtaDfuViewController.swift
//  MeshApp
//
//  Created by Dudley Du on 2019/4/2.
//

import UIKit
import MeshFramework

class MeshOtaDfuViewController: UIViewController {
    @IBOutlet weak var dfuNavigationBar: UINavigationBar!
    @IBOutlet weak var dfuNavigationItem: UINavigationItem!
    @IBOutlet weak var dfuNavigationLeftButtonItem: UIBarButtonItem!
    @IBOutlet weak var dfuNavigationRightButtonItem: UIBarButtonItem!

    @IBOutlet weak var deviceNameView: UIView!
    @IBOutlet weak var deviceNameLabel: UILabel!
    @IBOutlet weak var deviceTypeLable: UILabel!
    @IBOutlet weak var activityIndicator: UIActivityIndicatorView!

    @IBOutlet weak var meshDfuContentView: UIView!
    @IBOutlet weak var dfuTypeLabel: UILabel!
    @IBOutlet weak var dfuTypeDropDownButton: CustomDropDownButton!
    @IBOutlet weak var versionLabel: UILabel!
    @IBOutlet weak var dfuFwImagesDropDownButton: CustomDropDownButton!
    @IBOutlet weak var dfuMetadataImagesDropDownButton: CustomDropDownButton!
    @IBOutlet weak var versionTitleLabel: UILabel!
    @IBOutlet weak var toDeviceTitleLable: UILabel!
    @IBOutlet weak var toDeviceChoseDropDownButton: CustomDropDownButton!

    @IBOutlet weak var buttonsTopView: UIView!
    @IBOutlet weak var buttunsLeftSubView: UIView!
    @IBOutlet weak var buttonsRightSubView: UIView!
    @IBOutlet weak var getDfuStatusButton: CustomLayoutButton!
    @IBOutlet weak var applyDfuButton: CustomLayoutButton!
    @IBOutlet weak var startUpgradeButton: CustomLayoutButton!
    @IBOutlet weak var stopUpgradeButton: CustomLayoutButton!

    @IBOutlet weak var progressView: UIProgressView!
    @IBOutlet weak var upgradeLogLabel: UILabel!
    @IBOutlet weak var upgradePercentageLabel: UILabel!
    @IBOutlet weak var upgradeLogTextView: UITextView!

    private var otaBasicDate = Date(timeIntervalSinceNow: 0)
    private var dfustatusTimer: Timer?

    private let OTA_GET_DFU_STATUS_MONITOR_INTERVAL = 30    // unit: seconds.

    var deviceName: String?
    var groupName: String?  // When groupName is not nil, it comes from CmponentViewControl; if groupName is nil, it comes from FirmwareUpgradeViewController.

    var tmpCBDeviceObject: AnyObject?     // only valid when the view controller active.

    var otaDevice: OtaDeviceProtocol?
    var otaFwImageNames: [String] = []
    var otaMetadataImageNames: [String] = []
    var selectedProxyToDeviceName: String?
    var selectedFwImageName: String?
    var selectedMetadataImageName: String?
    var otaDfuFirmware: Data?
    var otaDfuMetadata: OtaDfuMetadata?

    var otaUpdatedStarted: Bool = false
    var lastTransferredPercentage: Int = -1  // indicates invalid value, will be udpated.

    override func viewDidLoad() {
        super.viewDidLoad()
        meshLog("MeshOtaDfuViewController, viewDidLoad")

        // Do any additional setup after loading the view.c
        otaDevice = OtaManager.shared.activeOtaDevice
        tmpCBDeviceObject = otaDevice?.otaDevice
        if deviceName == nil {
            deviceName = otaDevice?.getDeviceName() ?? MeshConstantText.UNKNOWN_DEVICE_NAME
        }

        notificationInit()
        viewInit()
    }

    override func viewDidDisappear(_ animated: Bool) {
        self.dfustatusTimer?.invalidate()
        self.dfustatusTimer = nil
        NotificationCenter.default.removeObserver(self)
        OtaManager.shared.resetOtaUpgradeStatus()
        tmpCBDeviceObject = nil
        super.viewDidDisappear(animated)
    }

    func getValidProxyToDeviceList() -> [String] {
        var validToDevices: [String] = []
        if let proxyDevice = self.otaDevice, let groups = MeshFrameworkManager.shared.getAllMeshNetworkGroups() {
            for group in groups {
                if group == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME ||  group.hasPrefix("dfu_") {
                    continue
                }

                let components = MeshFrameworkManager.shared.getMeshGroupComponents(groupName: group) ?? []
                validToDevices.append(contentsOf: components)
            }
            validToDevices.removeAll(where: { $0 == proxyDevice.getDeviceName() })
            return validToDevices
        }
        return []
    }

    func updateVersionAndToDeviceUI(dfuType: Int) {
        if dfuType == MeshDfuType.APP_OTA_TO_DEVICE {
            getDfuStatusButton.isEnabled = false
        } else {
            getDfuStatusButton.isEnabled = true
        }

        toDeviceTitleLable.isHidden = true
        toDeviceChoseDropDownButton.isHidden = true
        toDeviceChoseDropDownButton.isEnabled = false
        versionTitleLabel.isHidden = false
        versionTitleLabel.isHidden = false
    }

    func viewInit() {
        /*
         * Note, the Apply function has been implemented in the internal of the DFU devices, so not used any more.
         * When implementing any new desgin, the Apply button should be removed.
         */
        applyDfuButton.isEnabled = false
        applyDfuButton.isHidden = true

        dfuNavigationItem.title = "Mesh DFU"
        versionLabel.text = "Not Avaiable"
        upgradeLogTextView.text = ""
        log("OTA Upgrade view loaded")
        log("OTA device type: \(otaDevice?.getDeviceType() ?? OtaDeviceType.mesh)")
        log("OTA device name: \"\(otaDevice?.getDeviceName() ?? "Not Avaiable")\"")

        getDfuStatusButton.setTitleColor(UIColor.gray, for: .disabled)
        applyDfuButton.setTitleColor(UIColor.gray, for: .disabled)
        startUpgradeButton.setTitleColor(UIColor.gray, for: .disabled)
        stopUpgradeButton.setTitleColor(UIColor.gray, for: .disabled)

        dfuNavigationItem.rightBarButtonItem = nil  // not used currently.
        upgradeLogTextView.layer.borderWidth = 1
        upgradeLogTextView.layer.borderColor = UIColor.gray.cgColor
        upgradeLogTextView.isEditable = false
        upgradeLogTextView.isSelectable = false
        upgradeLogTextView.layoutManager.allowsNonContiguousLayout = false

        otaUpdatedStarted = false
        lastTransferredPercentage = -1  // indicates invalid value, will be udpated.
        otaProgressUpdated(percentage: 0.0)

        toDeviceChoseDropDownButton.dropDownItems = getValidProxyToDeviceList()
        toDeviceChoseDropDownButton.setSelection(select: 0)
        dfuTypeDropDownButton.dropDownItems = MeshDfuType.DFU_TYPE_TEXT_LIST
        if let dfuType = OtaUpgrader.activeDfuType, let dfuTypeString = MeshDfuType.getDfuTypeText(type: dfuType) {
            dfuTypeDropDownButton.setSelection(select: dfuTypeString)
        } else {
            dfuTypeDropDownButton.setSelection(select: MeshDfuType.getDfuTypeText(type: MeshDfuType.PROXY_DFU_TO_ALL)!)
        }

        selectedProxyToDeviceName = toDeviceChoseDropDownButton.selectedString
        updateVersionAndToDeviceUI(dfuType: MeshDfuType.getDfuType(by: dfuTypeDropDownButton.selectedString) ?? 0)

        guard let otaDevice = self.otaDevice else {
            meshLog("error: MeshOtaDfuViewController, viewInit, invalid otaDevice instance nil")
            log("error: invalid nil OTA device object")
            DispatchQueue.main.async {
                UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil OTA device object.", title: "Error")
            }
            return
        }
        deviceNameLabel.text = otaDevice.getDeviceName()
        deviceTypeLable.text = OtaManager.getOtaDeviceTypeString(by: otaDevice.getDeviceType())
        if otaDevice.getDeviceType() != .mesh {
            dfuTypeDropDownButton.isEnabled = false
        }
        stopAnimating()

        DispatchQueue.main.async {
            // read and update firmware image list.
            self.firmwareImagesInit()
            self.dfuFwImagesDropDownButton.dropDownItems = self.otaFwImageNames
            self.dfuMetadataImagesDropDownButton.dropDownItems = self.otaMetadataImageNames
            self.otaDfuMetadata = nil
            if self.otaFwImageNames.count > 0 {
                if let imageName = OtaUpgrader.activeDfuFwImageFileName, let index = self.otaFwImageNames.firstIndex(of: imageName) {
                    self.dfuFwImagesDropDownButton.setSelection(select: index)
                } else {
                    self.dfuFwImagesDropDownButton.setSelection(select: 0)
                }
                self.selectedFwImageName = self.dfuFwImagesDropDownButton.selectedString
                self.log("Selected image file name: \"\(self.selectedFwImageName!)\"for firmware OTA.")
            }
            if self.otaMetadataImageNames.count > 0 {
                if let metadataImageName = OtaUpgrader.activeDfuFwMetadataFileName, let index = self.otaMetadataImageNames.firstIndex(of: metadataImageName) {
                    self.dfuMetadataImagesDropDownButton.setSelection(select: index)
                } else {
                    self.dfuMetadataImagesDropDownButton.setSelection(select: 0)
                }
                self.dfuMetadataImagesDropDownButton.setSelection(select: 0)
                self.selectedMetadataImageName = self.dfuMetadataImagesDropDownButton.selectedString
                self.log("Selected image info file name: \"\(self.selectedMetadataImageName!)\" for firmware OTA.")
                self.doDfuMetadataImagesDropDownButtonClick()
            }
        }
    }

    func notificationInit() {
        /* [Dudley]: When do firmware OTA, the mesh notification should be suppressed to avoid any confusion.
         NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
         name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED), object: nil)
         NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
         name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED), object: nil)
         NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
         name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED), object: nil)
         */

        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: OtaConstants.Notification.OTA_STATUS_UPDATED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DFU_STATUS), object: nil)
    }

    @objc func notificationHandler(_ notification: Notification) {
        guard let userInfo = notification.userInfo else {
            return
        }
        switch notification.name {
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED):
            if let nodeConnectionStatus = MeshNotificationConstants.getNodeConnectionStatus(userInfo: userInfo) {
                self.showToast(message: "Device \"\(nodeConnectionStatus.componentName)\" \((nodeConnectionStatus.status == MeshConstants.MESH_CLIENT_NODE_CONNECTED) ? "has connected." : "is unreachable").")
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED):
            if let linkStatus = MeshNotificationConstants.getLinkStatus(userInfo: userInfo) {
                self.showToast(message: "Mesh network has \((linkStatus.isConnected) ? "connected" : "disconnected").")
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED):
            if let networkName = MeshNotificationConstants.getNetworkName(userInfo: userInfo) {
                self.showToast(message: "Database of mesh network \(networkName) has changed.")
            }
        case Notification.Name(rawValue: OtaConstants.Notification.OTA_STATUS_UPDATED):
            if let otaStatus = OtaConstants.Notification.getOtaNotificationData(userInfo: userInfo) {
                let otaState = OtaUpgrader.OtaState(rawValue: otaStatus.otaState) ?? OtaUpgrader.OtaState.idle
                if otaStatus.errorCode == OtaErrorCode.SUCCESS {
                    if otaStatus.otaState == OtaUpgrader.OtaState.idle.rawValue {
                        log("OTA state: \(otaStatus.description).")
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.readAppInfo.rawValue {
                        // try to get and show the read firmware version from remote device.
                        let appInfo = String(otaStatus.description.trimmingCharacters(in: CharacterSet.whitespaces))
                        log("OTA state: \(otaState.description) finished success. \(appInfo)")
                        var version = ""
                        if let otaDevice = self.otaDevice, otaDevice.getDeviceType() == .mesh, appInfo.hasPrefix("CID:") {
                            let componentInfoValue = MeshComponentInfo(componentInfo: appInfo)
                            version = componentInfoValue.VER
                        } else {
                            let appVersion = String(appInfo.split(separator: " ").last ?? "")
                            if !appVersion.isEmpty, appVersion != "success" {
                                let characterSet = CharacterSet(charactersIn: "0123456789.")
                                version = appVersion.trimmingCharacters(in: characterSet)
                            }
                        }
                        if !version.isEmpty {
                            versionLabel.text = version
                        }
                        if let device = self.otaDevice {
                            log("Device: \(device.getDeviceName()), Component Info: \(appInfo)")
                        }
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.dataTransfer.rawValue {
                        if otaStatus.transferredImageSize == 0 {
                            log("OTA state: \(otaState.description) started.")
                        }
                        // Update and log firmware image download percentage value.
                        otaProgressUpdated(percentage: Float(otaStatus.transferredImageSize) / Float(otaStatus.fwImageSize))
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.complete.rawValue {
                        otaUpdatedStarted = false
                        if MeshDfuType.getDfuType(by: dfuTypeDropDownButton.selectedString) == MeshDfuType.APP_OTA_TO_DEVICE {
                            self.log("done: OTA image download completed success.")
                            self.stopAnimating()
                            UtilityManager.showAlertDialogue(parentVC: self,
                                                             message: "Congratulation! OTA process completed successfully.",
                                                             title: "Success", completion: nil,
                                                             action: UIAlertAction(title: "OK", style: .default,
                                                                                   handler: { (action) in
                                                                                    //self.onLeftBarButtonItemClick(self.backBarButtonItem)
                                                             }))
                        } else if OtaUpgrader.shared.isDfuOtaUploading {
                            self.startAnimating()
                            self.log("uploading firmware image to distirbutor completed success.")
                        }
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.dfuCommand.rawValue {
                        self.log("done: \(otaStatus.description)")
                        updateGetDfuStatusButton()
                    } else {
                        // Log normal OTA upgrade successed step.
                        log("OTA state: \(otaState.description) finished success.")
                        if otaState == OtaUpgrader.OtaState.otaServiceDiscover, let otaDevice = self.otaDevice {
                            log("OTA version: OTA_VERSION_\(otaDevice.otaVersion)")
                        }
                    }
                } else {
                    if otaStatus.otaState >= OtaUpgrader.OtaState.complete.rawValue {
                        otaUpdatedStarted = false
                        updateGetDfuStatusButton()
                        // OTA upgrade process finished
                        self.log("done: OTA upgrade stopped with error. Error Code: \(otaStatus.errorCode), \(otaStatus.description)\n")
                        self.stopAnimating()
                        UtilityManager.showAlertDialogue(parentVC: self,
                                                         message: "Oops! DFU OTA process stopped with some error, please reset devices and retry again later.")
                    } else {
                        // Log normal OTA upgrade failed step.
                        log("error: OTA state: \(otaState.description) failed. Error Code:\(otaStatus.errorCode), message:\(otaStatus.description)")
                    }
                }
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DFU_STATUS):
            if let dfuStatus = MeshNotificationConstants.getDfuStatus(userInfo: userInfo) {
                meshLog("MeshOtaDfuViewController, notificationHandler, DFU status: \(dfuStatus.state), data: \(dfuStatus.data.dumpHexBytes())")
                switch dfuStatus.state {
                case MeshDfuState.MESH_DFU_STATE_VALIDATE_NODES:
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU finding nodes")
                    otaDfuProgressUpdated(message: "DFU finding nodes")
                case MeshDfuState.MESH_DFU_STATE_GET_DISTRIBUTOR:
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU choosing Distributor");
                    otaDfuProgressUpdated(message: "DFU choosing distributor")
                case MeshDfuState.MESH_DFU_STATE_UPLOAD:
                    guard dfuStatus.data.count > 0, let progress = dfuStatus.data.first else {
                        meshLog("MeshOtaDfuViewController, notificationHandler, DFU uploading firmware to the Distributor");
                        self.log("DFU uploading firmware to the Distributor")
                        otaDfuProgressUpdated(message: "DFU uploading")
                        break
                    }
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU upload progress: \(progress)%");
                    otaDfuProgressUpdated(message: "DFU uploading", percentage: Double(progress / 2))
                case MeshDfuState.MESH_DFU_STATE_DISTRIBUTE:
                    guard dfuStatus.data.count > 0 else {
                        meshLog("MeshOtaDfuViewController, notificationHandler, DFU Distributor distributing firmware to nodes");
                        self.log("DFU Distributor distributing firmware to nodes")
                        otaDfuProgressUpdated(message: "DFU distributing")
                        break
                    }

                    let bytes = [UInt8](dfuStatus.data)
                    let numberNodes = UInt16(bytes[0]) + (UInt16(bytes[1]) << 8)
                    if (numberNodes * 4) != (dfuStatus.data.count - 2) {
                        meshLog("error: MeshOtaDfuViewController, notificationHandler, DFU bad Distributor data length");
                        self.log("warning: DFU bad Distributor data length")
                        break
                    }
                    var offset = 2
                    var progress: Int = 100
                    for i in 0..<numberNodes {
                        // Node data: 2 bytes address, 1 byte phase, 1 byte progress
                        let srcAddr = UInt16(bytes[0]) + (UInt16(bytes[1]) << 8)
                        let phase = bytes[offset + 2]
                        let nodeProgress = Int(bytes[offset + 3])
                        meshLog("MeshOtaDfuViewController, notificationHandler, DFU distribution node[\(i)] src: \(String(format: "0x%04x", srcAddr)), phase: \(phase), progress: \(progress)")
                        // find the minimum DFU transfer progress.
                        if phase == MeshFirmwareUpdatePhase.MESH_DFU_FW_UPDATE_PHASE_TRANSFER_ACTIVE {
                            if progress > nodeProgress {
                                progress = nodeProgress
                            }
                        } else if phase == MeshFirmwareUpdatePhase.MESH_DFU_FW_UPDATE_PHASE_IDLE {
                            progress = 0
                            break
                        }

                        offset += 4
                    }
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU distribution progress: \(progress)%");
                    otaDfuProgressUpdated(message: "DFU distributing", percentage: Double(progress / 2 + 50))
                case MeshDfuState.MESH_DFU_STATE_APPLY:
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU applying firmware");
                    otaDfuProgressUpdated(message: "DFU applying firmware")
                case MeshDfuState.MESH_DFU_STATE_COMPLETE:
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU completed");
                    otaDfuProgressUpdated(message: "DFU completed", percentage: Double(100))

                    if dfuStatus.data.count > 0 {
                        let bytes = [UInt8](dfuStatus.data)
                        let numberNodes = UInt16(bytes[0]) + (UInt16(bytes[1]) << 8)
                        if (numberNodes * 4) != (dfuStatus.data.count - 2) {
                            meshLog("error: MeshOtaDfuViewController, notificationHandler, DFU bad complete data length");
                            self.log("warning: DFU bad complete data length")
                        } else {
                            var offset = 2
                            for i in 0..<numberNodes {
                                // Node data: 2 bytes address, 1 byte phase, 1 byte progress
                                let srcAddr = UInt16(bytes[0]) + (UInt16(bytes[1]) << 8)
                                let phase = bytes[offset + 2]
                                meshLog("MeshOtaDfuViewController, notificationHandler, DFU complete, node[\(i)]: \(String(format: "0x%04x", srcAddr)), DFU \((phase == MeshFirmwareUpdatePhase.MESH_DFU_FW_UPDATE_PHASE_APPLY_SUCCESS) ? "successed" : "failed")")

                                offset += 4
                            }
                        }
                    }
                    // try to cancel the reporting of DFU status.
                    let result = MeshFrameworkManager.shared.meshClientDfuGetStatus(interval: 0)
                    if result != MeshErrorCode.MESH_SUCCESS {
                        meshLog("error: MeshOtaDfuViewController, notificationHandler, MESH_DFU_STATE_COMPLETE, failed to stop DFU get status");
                        self.log("warning: failed to stop DFU status reporting")
                    } else {
                        self.log("DFU get status stopped")
                    }

                    // OTA upgrade process finished
                    otaUpdatedStarted = false
                    updateGetDfuStatusButton()
                    self.log("done: DFU upgrade success.")
                    self.stopAnimating()
                    UtilityManager.showAlertDialogue(parentVC: self,
                                                     message: "Congratulation! DFU upgrade completed successfully.",
                                                     title: "Success", completion: nil,
                                                     action: UIAlertAction(title: "OK", style: .default,
                                                                           handler: { (action) in
                                                                            //self.onLeftBarButtonItemClick(self.backBarButtonItem)
                                                     }))
                case MeshDfuState.MESH_DFU_STATE_FAILED:
                    meshLog("MeshOtaDfuViewController, notificationHandler, DFU failed");
                    otaDfuProgressUpdated(message: "DFU failed")
                    // try to cancel the reporting of DFU status.
                    let result = MeshFrameworkManager.shared.meshClientDfuGetStatus(interval: 0)
                    if result != MeshErrorCode.MESH_SUCCESS {
                        meshLog("error: MeshOtaDfuViewController, notificationHandler, MESH_DFU_STATE_FAILED, failed to stop DFU get status");
                        self.log("warning: failed to stop DFU get status")
                    } else {
                        self.log("DFU get status stopped")
                    }

                    // OTA upgrade process finished
                    otaUpdatedStarted = false
                    updateGetDfuStatusButton()
                    self.log("done: DFU upgrade stopped with some error encountered")
                    self.stopAnimating()
                    UtilityManager.showAlertDialogue(parentVC: self,
                                                     message: "Oops! DFU upgrade stopped/cancelled with some error, please recovery devices and retry again later.")
                default:
                    break
                }
            }

            self.getDfuStatusButton.isEnabled = true
        default:
            break
        }
    }

    func clearDfuGroups() {
        guard let networkName = MeshFrameworkManager.shared.getOpenedMeshNetworkName() else { return }
        let allGroups = MeshFrameworkManager.shared.getAllMeshNetworkGroups(networkName: networkName) ?? []
        for group in allGroups {
            if group.hasPrefix("dfu_") {
                MeshFrameworkManager.shared.deleteMeshGroup(groupName: group) { (networkName: String?, _ error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        meshLog("error: MeshOtaDfuViewController, clearDfuGroups, failed to delete DFU group:\(group), error:\(error)")
                        return
                    }
                    meshLog("MeshOtaDfuViewController, clearDfuGroups, delete DFU group:\(group) done")
                }
            }
        }
    }

    func firmwareImagesInit() {
        let defaultDocumentsPath = NSHomeDirectory() + "/Documents"
        let meshPath = "mesh"
        let fwImagePath = "\(meshPath)/fwImages"
        let meshSearchPath = "\(defaultDocumentsPath)/\(meshPath)"
        let fwImagesSearchPath = "\(defaultDocumentsPath)/\(fwImagePath)"

        otaFwImageNames.removeAll()
        otaMetadataImageNames.removeAll()
        let foundInFwImages = addFirmwareImageNames(atPath: meshSearchPath, prefix: meshPath)
        let foundInMesh = addFirmwareImageNames(atPath: fwImagesSearchPath, prefix: fwImagePath)
        let foundInDocuments = addFirmwareImageNames(atPath: defaultDocumentsPath)
        if !foundInFwImages, !foundInMesh, !foundInDocuments {
            meshLog("error: MeshOtaDfuViewController, firmwareImagesInit, no valid firmware images found")
            UtilityManager.showAlertDialogue(parentVC: self, message: "No valid firmware images found under App's \"Documents/mesh/fwImages\", \"Documents/mesh\" and  \"Documents\" directories. Please copy valid firmware images into your device, then try again later.", title: "Error")
        }
    }

    func addFirmwareImageNames(atPath: String, prefix: String? = nil) -> Bool {
        var isDirectory = ObjCBool(false)
        let exists = FileManager.default.fileExists(atPath: atPath, isDirectory: &isDirectory)
        if !exists || !isDirectory.boolValue {
            meshLog("error: MeshOtaDfuViewController, addFirmwareImageNames, \(atPath) not exsiting")
            return false
        }

        let namePrefix = ((prefix == nil) || (prefix?.isEmpty ?? true)) ? "" : (prefix! + "/")
        if let files = try? FileManager.default.contentsOfDirectory(atPath: atPath) {
            for fileName in files {
                // ecdsa256_genkey.exe tool will auto append .signed extension to the signed image file name.
                if fileName.hasSuffix(".bin") || fileName.hasSuffix(".bin.signed") {
                    otaFwImageNames.append(namePrefix + fileName)
                    meshLog("MeshOtaDfuViewController, addFirmwareImageNames, found image: \(namePrefix + fileName)")
                } else if fileName.hasPrefix("image_info") || fileName.hasSuffix("image_info") {
                    otaMetadataImageNames.append(namePrefix + fileName)
                    meshLog("MeshOtaDfuViewController, addFirmwareImageNames, found metadata image: \(namePrefix + fileName)")
                } else if fileName.hasSuffix(".json") {
                    otaMetadataImageNames.append(namePrefix + fileName)
                }
            }
        }

        if otaFwImageNames.isEmpty, otaMetadataImageNames.isEmpty {
            return false
        }
        return true
    }

    func getFullPath(for selectFileName: String) -> String {
        return NSHomeDirectory() + "/Documents/" + selectFileName
    }

    /* Update UI for WICED firmware OTA process state changed. */
    func otaProgressUpdated(percentage: Float) {
        let pct: Float = (percentage > 1.0) ? 1.0 : ((percentage < 0.0) ? 0.0 : percentage)
        let latestPercentage = Int(pct * 100)
        if percentage <= 0.0 {
            upgradePercentageLabel.text = String(format: "%d%%", latestPercentage)
        } else {
            upgradePercentageLabel.text = String(format: "%d", latestPercentage) + ((latestPercentage >= 100) ? "%, Completed" : "%, Downloading")
        }
        progressView.progress = pct

        if otaUpdatedStarted, lastTransferredPercentage != latestPercentage {
            log("transferred size: \(latestPercentage)%%")
            lastTransferredPercentage = latestPercentage
        }
    }

    /* Update UI for DFU firmware upgrade process process state changed. */
    func otaDfuProgressUpdated(message: String, percentage: Double? = nil) {
        if let progress = percentage, progress <= 100.0, progress >= 0.0 {
            progressView.progress = Float(progress / 100.0)
            upgradePercentageLabel.text = "\(Int(progress))%, \(message)"
            self.log("\(message), \(Int(progress))%%")
        } else {
            upgradePercentageLabel.text = "\(Int(progressView.progress * 100))%, \(message)"
            self.log(message)
        }
    }

    func log(_ message: String, force: Bool = false) {
        if !force, let timer = self.dfustatusTimer, timer.isValid {
            return  // disable the log when in DFU Upgrading monitoring state.
        }
        if message.count > 0, message != "\n" {
            let seconds = Date().timeIntervalSince(otaBasicDate)
            let msg = String(format: "[%.3f] \(message)\n", seconds)
            upgradeLogTextView.text += msg
        } else {
            upgradeLogTextView.text += "\n"
        }
        let bottom = NSRange(location: upgradeLogTextView.text.count, length: 1)
        upgradeLogTextView.scrollRangeToVisible(bottom)
    }

    func startAnimating() {
        applyDfuButton.isEnabled = false
        startUpgradeButton.isEnabled = false
        stopUpgradeButton.isEnabled = true
        activityIndicator.startAnimating()
        activityIndicator.isHidden = false
        dfuTypeDropDownButton.isEnabled = false
        dfuFwImagesDropDownButton.isEnabled = false
        dfuMetadataImagesDropDownButton.isEnabled = false
    }

    func stopAnimating() {
        activityIndicator.stopAnimating()
        activityIndicator.isHidden = true
        applyDfuButton.isEnabled = true
        startUpgradeButton.isEnabled = true
        stopUpgradeButton.isEnabled = true
        dfuTypeDropDownButton.isEnabled = true
        dfuFwImagesDropDownButton.isEnabled = true
        dfuMetadataImagesDropDownButton.isEnabled = true
    }

    func updateGetDfuStatusButton() {
        guard let selectedDfuType = MeshDfuType.getDfuType(by: dfuTypeDropDownButton.selectedString),
            selectedDfuType != MeshDfuType.APP_OTA_TO_DEVICE else {
            return
        }

        if MeshFrameworkManager.shared.isDfuStatusReportingEnabled {
            getDfuStatusButton.setTitle("Stop\nDFU\nStatus", for: .normal)
        } else {
            getDfuStatusButton.setTitle("Get\nDFU\nStatus", for: .normal)
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    @IBAction func onDfuNavigationLeftButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("MeshOtaDfuViewController, onDfuNavigationLeftButtonItemClick")
        if OtaUpgrader.shared.isOtaUpgradeRunning {
            UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh DFU process is uploading the firmware image, please wait until the firmware image has been completely uploaded.", title: "Warning")
            return
        }

        OtaManager.shared.resetOtaUpgradeStatus()
        if let otaDevice = self.otaDevice, otaDevice.getDeviceType() == .mesh, let groupName = self.groupName {
            meshLog("MeshOtaDfuViewController, navigate back to ComponentViewController page)")
            UserSettings.shared.currentActiveGroupName = groupName
            UserSettings.shared.currentActiveComponentName = otaDevice.getDeviceName()
            UtilityManager.navigateToViewController(targetClass: ComponentViewController.self)
        } else {
            meshLog("MeshOtaDfuViewController, navigate to FirmwareUpgradeViewController page)")
            if let otaDevice = self.otaDevice, otaDevice.getDeviceType() != .mesh {
                otaDevice.disconnect()
            }
            UtilityManager.navigateToViewController(targetClass: FirmwareUpgradeViewController.self)
        }
        self.dismiss(animated: true, completion: nil)
    }

    @IBAction func onDfuNavigationRightButtonItemClick(_ sender: UIBarButtonItem) {
    }

    @IBAction func onDfuTypeDropDownButtonClick(_ sender: CustomDropDownButton) {
        dfuTypeDropDownButton.showDropList(width: 220, parent: self) {
            meshLog("\(self.dfuTypeDropDownButton.selectedIndex), \(self.dfuTypeDropDownButton.selectedString)")
            self.log("Selected DFU type: \(self.dfuTypeDropDownButton.selectedString) for firmware OTA.")
            self.updateVersionAndToDeviceUI(dfuType: MeshDfuType.getDfuType(by: self.dfuTypeDropDownButton.selectedString) ?? 0)

            OtaUpgrader.storeActiveDfuInfo(dfuType: MeshDfuType.getDfuType(by: self.dfuTypeDropDownButton.selectedString),
                                           fwImageFileName: self.dfuFwImagesDropDownButton.selectedString,
                                           fwMetadataFileName: self.dfuMetadataImagesDropDownButton.selectedString)
        }
    }

    // Obsoluted, not support any more.
    @IBAction func onToDeviceChoseDropDownButtonCLick(_ sender: CustomDropDownButton) {
        let width = Int(meshDfuContentView.bounds.size.width) - 16
        toDeviceChoseDropDownButton.showDropList(width: width, parent: self) {
            meshLog("selectedProxyToDevice, \(self.toDeviceChoseDropDownButton.selectedIndex), \(self.toDeviceChoseDropDownButton.selectedString)")
            self.log("Selected proxy to device name: \(self.toDeviceChoseDropDownButton.selectedString)")
            self.selectedProxyToDeviceName = self.toDeviceChoseDropDownButton.selectedString
        }
    }


    @IBAction func onDfuFwImagesDropDownButtonClick(_ sender: CustomDropDownButton) {
        let width = Int(meshDfuContentView.bounds.size.width) - 16
        dfuFwImagesDropDownButton.showDropList(width: width, parent: self) {
            meshLog("selectedFwImageName, \(self.dfuFwImagesDropDownButton.selectedIndex), \(self.dfuFwImagesDropDownButton.selectedString)")
            self.log("Selected image name: \(self.dfuFwImagesDropDownButton.selectedString).")
            self.selectedFwImageName = self.dfuFwImagesDropDownButton.selectedString
            if let firmwareFile = self.selectedFwImageName {
                self.otaDfuFirmware = OtaUpgrader.readParseFirmwareImage(at: firmwareFile)
            }

            OtaUpgrader.storeActiveDfuInfo(dfuType: MeshDfuType.getDfuType(by: self.dfuTypeDropDownButton.selectedString),
                                           fwImageFileName: self.dfuFwImagesDropDownButton.selectedString,
                                           fwMetadataFileName: self.dfuMetadataImagesDropDownButton.selectedString)
        }
    }

    private func doDfuMetadataImagesDropDownButtonClick() {
        meshLog("selectedMetadataImageName, \(self.dfuMetadataImagesDropDownButton.selectedIndex), \(self.dfuMetadataImagesDropDownButton.selectedString)")
        self.log("Selected image info name: \(self.dfuMetadataImagesDropDownButton.selectedString).")
        self.selectedMetadataImageName = self.dfuMetadataImagesDropDownButton.selectedString
        if let metadataFile = self.selectedMetadataImageName {
            #if false   // metadata version <= 3.
            self.otaDfuMetadata = OtaUpgrader.readParseMetadataImage(at: metadataFile)
            #else       // metadata version = 4
            guard let (firmwareFile, metadataFile, firmwareId) = OtaUpgrader.readDfuManifestFile(at: metadataFile) else {
                self.log("error: failed to read and parse the selected manifest file.")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to read and parse the manifest file: \(self.selectedMetadataImageName!). Please copy the correct manifest file with the correct firmware file and metadata file, then retry again.")
                return
            }
            guard let firmwareImageData = OtaUpgrader.readParseFirmwareImage(at: firmwareFile) else {
                self.log("error: failed to read firmware image: \(firmwareFile)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to read and parse the firmware image file: \(firmwareFile). Please copy the correct firmware file with the manifest file, then try again.")
                return
            }
            guard let metadata = OtaUpgrader.readParseMetadataImage(at: metadataFile, firmwareId: firmwareId) else {
                self.log("error: failed to read metadata: \(metadataFile).")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to read and parse the metadata image file: \(metadataFile). Please copy the correct metadata file with the manifest file, then try again.")
                return
            }

            self.dfuFwImagesDropDownButton.dropDownItems = [firmwareFile]
            self.dfuFwImagesDropDownButton.setSelection(select: 0)
            self.selectedFwImageName = self.dfuFwImagesDropDownButton.selectedString
            self.log("read firmware file: \(firmwareFile)")
            self.log("read metadata file: \(metadataFile)")
            self.log("read firmware ID: \(firmwareId)")
            self.log("read firmware version: \(metadata.firmwareVersionMajor).\(metadata.firmwareVersionMinor).\(metadata.firmwareVersionRevision).\(metadata.firmwareVersionBuild)")
            self.otaDfuFirmware = firmwareImageData
            self.otaDfuMetadata = metadata
            #endif
        }

        OtaUpgrader.storeActiveDfuInfo(dfuType: MeshDfuType.getDfuType(by: self.dfuTypeDropDownButton.selectedString),
                                       fwImageFileName: self.dfuFwImagesDropDownButton.selectedString,
                                       fwMetadataFileName: self.dfuMetadataImagesDropDownButton.selectedString)
    }
    @IBAction func onDfuMetadataImagesDropDownButtonClick(_ sender: CustomDropDownButton) {
        let width = Int(meshDfuContentView.bounds.size.width) - 16
        dfuMetadataImagesDropDownButton.showDropList(width: width, parent: self) {
            self.doDfuMetadataImagesDropDownButtonClick()
        }
    }

    @IBAction func onGetDfuStatusButtonClick(_ sender: CustomLayoutButton) {
        meshLog("MeshOtaDfuViewController, onGetDfuStatusButtonClick")
        self.log("")
        self.log("Get DFU status reporting")
        OtaUpgrader.shared.otaUpgradeGetDfuStatus()
    }

    @IBAction func onApplyDfuButtonClick(_ sender: CustomLayoutButton) {
        // The apply function has been removed, not used anymore.
        meshLog("MeshOtaDfuViewController, onApplyDfuButtonClick")
        self.log("")
        self.log("Apply DFU upgrade images")
    }

    @IBAction func onStartUpgradeButtonClick(_ sender: CustomLayoutButton) {
        meshLog("MeshOtaDfuViewController, onStartUpgradeButtonClick")
        self.log("")
        self.log("Start OTA upgrade ...")

        let selectedDfuType = MeshDfuType.getDfuType(by: dfuTypeDropDownButton.selectedString)!
        guard let otaDevice = self.otaDevice else {
            meshLog("error: MeshOtaDfuViewController, onStartUpgradeButtonClick, invalid device or device name, nil")
            log("error: start upgrade, invalid nil OTA device instance")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil OTA device instance.")
            return
        }
        guard let firmwareFile = self.selectedFwImageName, let otaDfuFirmware = OtaUpgrader.readParseFirmwareImage(at: firmwareFile) else {
                meshLog("error: MeshOtaDfuViewController, onStartUpgradeButtonClick, failed to read and parse firmware image file")
                self.log("error: start upgrade, failed to read and parse firmware image file")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to read and parse firmware image file.")
                return
        }
        if self.otaDfuMetadata == nil, let metadataFile = self.selectedMetadataImageName {
            self.otaDfuMetadata = OtaUpgrader.readParseMetadataImage(at: metadataFile)
            if let dfuMetadata = self.otaDfuMetadata {
                meshLog("MeshOtaDfuViewController, onStartUpgradeButtonClick, read new firmwareFile=\(firmwareFile) metadataFile=\(metadataFile), DFU Type=\(selectedDfuType)")
                meshLog("    Firmware ID: \(dfuMetadata.firwmareId.dumpHexBytes())")
                if dfuMetadata.metadataVersion <= 2 {
                    meshLog("    Company ID: \(String(format: "0x%04x", dfuMetadata.companyId)), Product ID: \(String(format: "0x%04x", dfuMetadata.productId)), Vednor ID: \(String(format: "0x%04x", dfuMetadata.hardwareVeresionId)), firmware version=\(dfuMetadata.firmwareVersionMajor).\(dfuMetadata.firmwareVersionMinor).\(dfuMetadata.firmwareVersionRevision)")
                } else {
                    meshLog("    Company ID: \(String(format: "0x%04x", dfuMetadata.companyId)), Product ID: \(String(format: "0x%04x", dfuMetadata.productId)), Vednor ID: \(String(format: "0x%04x", dfuMetadata.hardwareVeresionId)), firmware version=\(dfuMetadata.firmwareVersionMajor).\(dfuMetadata.firmwareVersionMinor).\(dfuMetadata.firmwareVersionRevision).\(dfuMetadata.firmwareVersionBuild)")
                }
                meshLog("    Metadata Data: \(dfuMetadata.metadata.dumpHexBytes())")
            }
        }
        // For APP_OTA_TO_DEVICE, the metadata file is not required., could be ignored.
        if self.otaDfuMetadata == nil, selectedDfuType != MeshDfuType.APP_OTA_TO_DEVICE {
            meshLog("error: MeshOtaDfuViewController, onStartUpgradeButtonClick, failed to read and parse firmware metadata file, DFU Type=\(selectedDfuType)")
            self.log("error: start upgrade, failed to read and parse firmware metadata file")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to read and parse firmware metadata file.")
            return
        }

        otaUpdatedStarted = false
        lastTransferredPercentage = -1  // indicates invalid value, will be udpated.
        otaProgressUpdated(percentage: 0.0)
        if self.otaDevice?.otaDevice == nil {
            // The CoreBlutooth peripheral object will been cleaned when disconnected.
            // So, if use keep state this OTA page, try to restore the peripheral object to speed up the process.
            self.otaDevice?.otaDevice = tmpCBDeviceObject
        }

        self.startAnimating()
        meshLog("MeshOtaDfuViewController, onStartUpgradeButtonClick, calling OtaUpgrader.shared.otaUpgradeDfuStart()")
        if selectedDfuType == MeshDfuType.APP_OTA_TO_DEVICE {
            self.log("WICED OTA upgrade process started")
        } else {
            self.log("\(MeshDfuType.getDfuTypeText(type: selectedDfuType) ?? "DFU") process started")
        }
        OtaUpgrader.shared.otaUpgradeDfuStart(for: otaDevice, dfuType: selectedDfuType, fwImage: otaDfuFirmware, dfuMetadata: self.otaDfuMetadata)
        self.otaUpdatedStarted = true
    }

    @IBAction func onStopUpgradeButtonClick(_ sender: CustomLayoutButton) {
        meshLog("MeshOtaDfuViewController, onStopUpgradeButtonClick")
        self.log("")

        OtaUpgrader.shared.otaUpgradeDfuStop()

        self.stopAnimating()
        self.otaUpdatedStarted = false
        meshLog("MeshOtaDfuViewController, onStopUpgradeButtonClick, done: DFU stop command send success")
        self.log("done: OTA DFU upgrade stopped by user.")
    }
}
