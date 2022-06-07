/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
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
 * Device OTA Upgrade View implementation.
 */

import UIKit
import MeshFramework

class DeviceOtaUpgradeViewController: UIViewController {
    @IBOutlet weak var topNavigationItem: UINavigationItem!
    @IBOutlet weak var backBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var settingsBarButtomItem: UIBarButtonItem!
    @IBOutlet weak var otaUpgradeTitleLabel: UILabel!
    @IBOutlet weak var otaDeviceNameLabel: UILabel!
    @IBOutlet weak var otaDeviceFwVersionLabel: UILabel!
    @IBOutlet weak var otaDropDownView: CustomDropDownView!
    @IBOutlet weak var otaBackgroundView: UIView!
    @IBOutlet weak var otaDeviceFiwImageHintLabel: UILabel!
    @IBOutlet weak var otaFirmwareUpgradeButton: UIButton!
    @IBOutlet weak var otaProgressBar: UIProgressView!
    @IBOutlet weak var otaProgressPercentage: UILabel!
    @IBOutlet weak var otaUpgradeLogTextView: UITextView!
    @IBOutlet weak var otaUpgradingActivityIndicator: UIActivityIndicatorView!
    @IBOutlet weak var otaDeviceTypeLable: UILabel!

    private var otaBasciDate = Date(timeIntervalSinceNow: 0)

    var deviceName: String?
    var groupName: String?  // When groupName is not nil, it comes from CmponentViewControl; if groupName is nil, it comes from FirmwareUpgradeViewController.

    var tmpCBDeviceObject: AnyObject?     // only valid when the view controller active.
    var otaDevice: OtaDeviceProtocol?
    var otaFwImageNames: [String] = []
    var selectedFwImageName: String?

    var otaUpdatedStarted: Bool = false
    var lastTransferredPercentage: Int = -1  // indicates invalid value, will be udpated.

    override func viewDidLoad() {
        super.viewDidLoad()
        meshLog("DeviceOtaUpgradeViewController, viewDidLoad")

        // Do any additional setup after loading the view.
        otaDevice = OtaManager.shared.activeOtaDevice
        tmpCBDeviceObject = otaDevice?.otaDevice

        notificationInit()
        viewInit()
    }

    override func viewDidDisappear(_ animated: Bool) {
        NotificationCenter.default.removeObserver(self)
        OtaManager.shared.resetOtaUpgradeStatus()
        tmpCBDeviceObject = nil
        super.viewDidDisappear(animated)
    }

    func viewInit() {
        otaUpgradeLogTextView.text = ""
        log("OTA Upgrade view loaded")
        log("OTA device type: \(otaDevice?.getDeviceType() ?? OtaDeviceType.ble)")
        log("OTA device name: \"\(otaDevice?.getDeviceName() ?? "Unknown Name")\"")

        topNavigationItem.rightBarButtonItem = nil  // not used currently.
        otaUpgradeLogTextView.layer.borderWidth = 1
        otaUpgradeLogTextView.layer.borderColor = UIColor.gray.cgColor
        otaUpgradeLogTextView.isEditable = false
        otaUpgradeLogTextView.isSelectable = false
        otaUpgradeLogTextView.layoutManager.allowsNonContiguousLayout = false

        otaDropDownView.dropDownDelegate = self

        otaUpdatedStarted = false
        lastTransferredPercentage = -1  // indicates invalid value, will be udpated.
        otaProgressUpdated(percentage: 0.0)

        guard let otaDevice = self.otaDevice else {
            meshLog("error: DeviceOtaUpgradeViewController, viewInit, invalid otaDevice instance nil")
            log("error: invalid nil OTA device object")
            DispatchQueue.main.async {
                UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil OTA device object.", title: "Error")
            }
            return
        }
        otaDeviceNameLabel.text = otaDevice.getDeviceName()
        otaDeviceFwVersionLabel.text = "Not Avaiable"
        otaDeviceTypeLable.text = OtaManager.getOtaDeviceTypeString(by: otaDevice.getDeviceType())
        topNavigationItem.title = "OTA Upgrade"

        otaFirmwareUpgradeButton.setTitleColor(UIColor.gray, for: .disabled)
        stopOtaUpgradingAnimating()

        DispatchQueue.main.async {
            // read and update firmware image list.
            self.firmwareImagesInit()
            self.otaDropDownView.dropListItems = self.otaFwImageNames
            // automatically select the first found firmware image by default.
            if self.otaDropDownView.dropListItems.count > 0 {
                let selectedIndex: Int = 0
                self.otaDropDownView.selectedIndex = selectedIndex
                self.otaDropDownView.text = self.otaDropDownView.dropListItems[selectedIndex]
                self.customDropDownViewDidUpdateValue(self.otaDropDownView, selectedIndex: selectedIndex)
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
                            otaDeviceFwVersionLabel.text = version
                        }
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.dataTransfer.rawValue {
                        if otaStatus.transferredImageSize == 0 {
                            log("OTA state: \(otaState.description) started.")
                        }
                        // Update and log firmware image download percentage value.
                        otaProgressUpdated(percentage: Float(otaStatus.transferredImageSize) / Float(otaStatus.fwImageSize))
                    } else if otaStatus.otaState == OtaUpgrader.OtaState.complete.rawValue {
                        otaUpdatedStarted = false
                        // OTA upgrade process finished, navigate to previous view controller if success.
                        self.stopOtaUpgradingAnimating()
                        self.log("done: OTA upgrade completed success.\n")
                        UtilityManager.showAlertDialogue(parentVC: self,
                                                         message: "Congratulation! OTA process has finshed successfully.",
                                                         title: "Success", completion: nil,
                                                         action: UIAlertAction(title: "OK", style: .default,
                                                                               handler: { (action) in
                                                                                //self.onLeftBarButtonItemClick(self.backBarButtonItem)
                                                         }))
                    } else {
                        // Log normal OTA upgrade successed step.
                        log("OTA state: \(otaState.description) finished success.")
                        if otaState == OtaUpgrader.OtaState.otaServiceDiscover, let otaDevice = self.otaDevice {
                            log("OTA version: OTA_VERSION_\(otaDevice.otaVersion)")
                        }
                    }
                } else {
                    if otaStatus.otaState == OtaUpgrader.OtaState.complete.rawValue {
                        otaUpdatedStarted = false
                        // OTA upgrade process finished
                        self.log("done: OTA upgrade stopped with error. Error Code: \(otaStatus.errorCode), \(otaStatus.description)\n")
                        self.stopOtaUpgradingAnimating()
                        UtilityManager.showAlertDialogue(parentVC: self,
                                                         message: "Oops! OTA process stopped with some error, please reset device and retry again later.")
                    } else {
                        // Log normal OTA upgrade failed step.
                        log("error: OTA state: \(otaState.description) failed. Error Code:\(otaStatus.errorCode), message:\(otaStatus.description)")
                    }
                }
            }
        default:
            break
        }
    }

    func firmwareImagesInit() {
        let defaultDocumentsPath = NSHomeDirectory() + "/Documents"
        let meshPath = "mesh"
        let fwImagePath = "\(meshPath)/fwImages"
        let meshSearchPath = "\(defaultDocumentsPath)/\(meshPath)"
        let fwImagesSearchPath = "\(defaultDocumentsPath)/\(fwImagePath)"

        otaFwImageNames.removeAll()
        let foundInFwImages = addFirmwareImageNames(atPath: meshSearchPath, prefix: meshPath)
        let foundInMesh = addFirmwareImageNames(atPath: fwImagesSearchPath, prefix: fwImagePath)
        let foundInDocuments = addFirmwareImageNames(atPath: defaultDocumentsPath)
        if !foundInFwImages, !foundInMesh, !foundInDocuments {
            meshLog("error: DeviceOtaUpgradeViewController, firmwareImagesInit, no valid firmware images found")
            UtilityManager.showAlertDialogue(parentVC: self, message: "No valid firmware images found under App's \"Documents/mesh/fwImages\", \"Documents/mesh\" and  \"Documents\" directories. Please copy valid firmware images into your device, then try again later.", title: "Error")
        }
    }

    func addFirmwareImageNames(atPath: String, prefix: String? = nil) -> Bool {
        var isDirectory = ObjCBool(false)
        let exists = FileManager.default.fileExists(atPath: atPath, isDirectory: &isDirectory)
        if !exists || !isDirectory.boolValue {
            meshLog("error: DeviceOtaUpgradeViewController, addFirmwareImageNames, \(atPath) not exsiting")
            return false
        }

        let namePrefix = ((prefix == nil) || (prefix?.isEmpty ?? true)) ? "" : (prefix! + "/")
        if let files = try? FileManager.default.contentsOfDirectory(atPath: atPath) {
            for fileName in files {
                // ecdsa256_genkey.exe tool will auto append .signed extension to the signed image file name.
                if fileName.hasSuffix(".bin") || fileName.hasSuffix(".bin.signed") {
                    otaFwImageNames.append(namePrefix + fileName)
                    meshLog("DeviceOtaUpgradeViewController, addFirmwareImageNames, found image: \(namePrefix + fileName)")
                }
            }
        }

        if otaFwImageNames.isEmpty {
            return false
        }
        return true
    }

    func getFullPath(for selectFileName: String) -> String {
        return NSHomeDirectory() + "/Documents/" + selectFileName
    }

    func otaProgressUpdated(percentage: Float) {
        let pct: Float = (percentage > 1.0) ? 1.0 : ((percentage < 0.0) ? 0.0 : percentage)
        let latestPercentage = Int(pct * 100)
        otaProgressPercentage.text = String(format: "%d", latestPercentage) + "%"
        otaProgressBar.progress = percentage

        if otaUpdatedStarted, lastTransferredPercentage != latestPercentage {
            log("transferred size: \(latestPercentage)%%")
            lastTransferredPercentage = latestPercentage
        }
    }

    func log(_ message: String) {
        let seconds = Date().timeIntervalSince(otaBasciDate)
        let msg = String(format: "[%.3f] \(message)\n", seconds)
        otaUpgradeLogTextView.text += msg
        let bottom = NSRange(location: otaUpgradeLogTextView.text.count, length: 1)
        otaUpgradeLogTextView.scrollRangeToVisible(bottom)
    }

    @IBAction func onOtaFirmwareUpgradeButtonClick(_ sender: UIButton) {
        meshLog("DeviceOtaUpgradeViewController, onOtaFirmwareUpgradeButtonClick")
        log("OTA update button triggerred")
        otaUpdatedStarted = false
        lastTransferredPercentage = -1  // indicates invalid value, will be udpated.
        otaProgressUpdated(percentage: 0.0)

        guard let otaDevice = self.otaDevice else {
            meshLog("error: DeviceOtaUpgradeViewController, onOtaFirmwareUpgradeButtonClick, invalid OTA device instance")
            log("error: invalid nil OTA device object")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil OTA device object.")
            return
        }
        var fwImagePath = NSHomeDirectory() + "/Documents/"
        guard let fwImageName = self.selectedFwImageName else {
            meshLog("error: DeviceOtaUpgradeViewController, onOtaFirmwareUpgradeButtonClick, no firmware image selected")
            log("error: no firmware image selected")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Please select a firmware image firstly, then try again.")
            return
        }

        fwImagePath = fwImagePath + fwImageName
        var isDirectory = ObjCBool(false)
        let exists = FileManager.default.fileExists(atPath: fwImagePath, isDirectory: &isDirectory)
        guard exists, !isDirectory.boolValue, let fwImageData = FileManager.default.contents(atPath: fwImagePath) else {
            meshLog("error: DeviceOtaUpgradeViewController, onOtaFirmwareUpgradeButtonClick, selected firmware image not exists")
            log("error: unable to read the content of the selected firmware image")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Firmware image \"\(fwImagePath)\" not found or failed to read the image file. Please copy and select valid firmware images into your device, then retry later.", title: "Error")
            return
        }
        meshLog("DeviceOtaUpgradeViewController, onOtaFirmwareUpgradeButtonClick, otaDevice=\(otaDevice), fwImagePath=\(fwImagePath), imageSize=\(fwImageData.count)")

        // the CoreBlutooth peripheral object has been cleaned when disconnected, here try to restore it for quick action if possible.
        if self.otaDevice?.otaDevice == nil {
            self.otaDevice?.otaDevice = tmpCBDeviceObject
        }

        doOtaFirmwareUpgrade(otaDevice: otaDevice, fwImage: fwImageData)
    }

    func doOtaFirmwareUpgrade(otaDevice: OtaDeviceProtocol, fwImage: Data) {
        DispatchQueue.main.async {  // because self.otaDevice instance is from main thread, so make suer it running in main thread.
            // Try to start OTA process.
            meshLog("DeviceOtaUpgradeViewController, doOtaFirmwareUpgrade, OTA process running")
            self.startOtaUpgradingAnimating()
            otaDevice.startOta(fwImage: fwImage, dfuMetadata: nil, dfuType: MeshDfuType.APP_OTA_TO_DEVICE)
            self.log("OTA upgrade process started")
            self.otaUpdatedStarted = true
        }
    }

    @IBAction func onLeftBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("DeviceOtaUpgradeViewController, onLeftBarButtonItemClick")
        if OtaUpgrader.shared.isOtaUpgradeRunning {
            UtilityManager.showAlertDialogue(parentVC: self, message: "Firmware OTA updating is in progress, please wait until the OTA updating has completed.", title: "Warning")
            return
        }

        OtaManager.shared.resetOtaUpgradeStatus()
        if let otaDevice = self.otaDevice, otaDevice.getDeviceType() == .mesh, let groupName = self.groupName {
            meshLog("DeviceOtaUpgradeViewController, navigate back to ComponentViewController page)")
            UserSettings.shared.currentActiveGroupName = groupName
            UserSettings.shared.currentActiveComponentName = otaDevice.getDeviceName()
            UtilityManager.navigateToViewController(targetClass: ComponentViewController.self)
        } else {
            meshLog("DeviceOtaUpgradeViewController, navigate to FirmwareUpgradeViewController page)")
            if let otaDevice = self.otaDevice, otaDevice.getDeviceType() != .mesh {
                otaDevice.disconnect()
            }
            UtilityManager.navigateToViewController(targetClass: FirmwareUpgradeViewController.self)
        }
        self.dismiss(animated: true, completion: nil)
    }

    @IBAction func onRightBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("DeviceOtaUpgradeViewController, onRightBarButtonItemClick")
    }

    func startOtaUpgradingAnimating() {
        otaFirmwareUpgradeButton.isEnabled = false
        otaUpgradingActivityIndicator.startAnimating()
        otaUpgradingActivityIndicator.isHidden = false
        otaDropDownView.isEnabled = false
    }

    func stopOtaUpgradingAnimating() {
        otaUpgradingActivityIndicator.stopAnimating()
        otaUpgradingActivityIndicator.isHidden = true
        otaFirmwareUpgradeButton.isEnabled = true
        otaDropDownView.isEnabled = true
    }
}

extension DeviceOtaUpgradeViewController: CustomDropDownViewDelegate {
    func customDropDwonViewWillShowDropList(_ dropDownView: CustomDropDownView) {
        meshLog("customDropDwonViewWillShowDropList, dropListItems=\(dropDownView.dropListItems)")
    }

    func customDropDownViewDidUpdateValue(_ dropDownView: CustomDropDownView, selectedIndex: Int) {
        meshLog("customDropDownViewDidUpdateValue, selectedIndex=\(selectedIndex), text=\(String(describing: dropDownView.text))")
        if let selectedText = dropDownView.text, selectedText.count > 0 {
            selectedFwImageName = selectedText
            otaFirmwareUpgradeButton.isEnabled = true
            log("selected firmware image: \"\(selectedText)\"")
        } else {
            log("error: no firmware image selected, please copy vaild firmware images and try again later")
            selectedFwImageName = nil
            otaFirmwareUpgradeButton.isEnabled = false
        }
    }
}
