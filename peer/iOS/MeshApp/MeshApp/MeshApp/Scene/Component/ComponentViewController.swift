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

/** @file
 *
 * Component device detail view controller implementation.
 */

import UIKit
import MeshFramework

enum ComponentPopoverChoices: String {
    case identify = "Identify"
    case rename = "Rename Device"
    case delete = "Delete Device"
    case move = "Move to Other Group"
    case add = "Add to Other Group"
    case remove = "Remove from this Group"
    case ota = "Firmware OTA"
    case setVendorData = "Set Vendor Data"
    case configurePublication = "Configure Publication"

    static var allValues = [ComponentPopoverChoices.identify.rawValue,
                            ComponentPopoverChoices.rename.rawValue,
                            ComponentPopoverChoices.delete.rawValue,
                            ComponentPopoverChoices.add.rawValue,
                            ComponentPopoverChoices.move.rawValue,
                            ComponentPopoverChoices.remove.rawValue,
                            ComponentPopoverChoices.ota.rawValue,
                            ComponentPopoverChoices.setVendorData.rawValue,
                            ComponentPopoverChoices.configurePublication.rawValue]
}

class ComponentViewController: UIViewController {
    @IBOutlet weak var backBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var settingBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var contentView: UIView!

    var groupName: String?
    var deviceName: String?
    var componentType: Int = MeshConstants.MESH_COMPONENT_UNKNOWN

    private var operationType: ComponentPopoverChoices?
    private var componentOperationTimer: Timer?
    private let indicatorView = CustomIndicatorView()
    private var popoverSelectedGroupName: String?

    private var usingMoveToGroupApi: Bool = true

    private var mConponentControlVC: UIViewController?
    lazy var conponentControlVC: UIViewController = {
        if let vc = self.mConponentControlVC {
            return vc
        }

        if componentType >= MeshConstants.MESH_COMPONENT_SENSOR_SERVER {
            guard let controlVC = self.storyboard?.instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.SENSOR) as? SensorViewController else {
                meshLog("error: ComponentViewController, failed to load \(MeshAppStoryBoardIdentifires.SENSOR) ViewController from Main storyboard")
                return UIViewController()
            }
            return controlVC as UIViewController
        } else if componentType == MeshConstants.MESH_COMPONENT_UNKNOWN {
            // For unknown device, show set vendor specific data view.
            guard let controlVC = self.storyboard?.instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.VENDOR_SPECIFIC_DATA) as? VendorSpecificDataViewController else {
                meshLog("error: ComponentViewController, failed to load \(MeshAppStoryBoardIdentifires.VENDOR_SPECIFIC_DATA) ViewController from Main storyboard")
                return UIViewController()
            }
            return controlVC as UIViewController
        }

        guard let controlVC = self.storyboard?.instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.GROUP_DETAILS_CONTROLS) as? GroupDetailsControlsViewController else {
            meshLog("error: ComponentViewController, failed to load \(MeshAppStoryBoardIdentifires.GROUP_DETAILS_CONTROLS) ViewController from Main storyboard")
            return UIViewController()
        }
        return controlVC as UIViewController
    }()

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        contentViewInit()
    }

    override func viewDidDisappear(_ animated: Bool) {
        NotificationCenter.default.removeObserver(self)
        super.viewDidDisappear(animated)
    }

    func notificationInit() {
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED), object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_VENDOR_SPECIFIC_DATA_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_ON_OFF_STATUS), object: nil)
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
            guard let dbChangedNetworkName = MeshNotificationConstants.getNetworkName(userInfo: userInfo),
                dbChangedNetworkName == UserSettings.shared.currentActiveMeshNetworkName else {
                    meshLog("error, ComponentViewController, onNetworkDatabaseChanged, DB change event network name not match")
                    return
            }
            self.showToast(message: "Database of mesh network \(dbChangedNetworkName) has changed.")

            if let opType = operationType {
                if opType == .move && !usingMoveToGroupApi {
                    return
                }

                componentOperationTimer?.invalidate()
                componentOperationTimer = nil

                switch opType {
                case .identify:
                    onDeviceIdentify()
                case .delete:
                    onComponentDeviceDelete()
                case .move:
                    if self.usingMoveToGroupApi {
                        onMovedToGroupHandler(.none, nil)
                    }
                case .rename:
                    break
                case .add:
                    onAddToGroupHandler(.none, nil)
                case .remove:
                    break
                case .ota:
                    break
                case .setVendorData:
                    break
                case .configurePublication:
                    break
                }
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_VENDOR_SPECIFIC_DATA_CHANGED):
            if let _ = self.conponentControlVC as? VendorSpecificDataViewController {
                return  // let VendorSpecificDataViewController process the notification event data.
            }
            guard let vendorData = MeshNotificationConstants.getVendorSpecificData(userInfo: userInfo) else {
                meshLog("error, ComponentViewController, getVendorSpecificData, no valid data received, userInfo=\(userInfo)")
                return
            }
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: """
                Received Vendor Specific Data:\n
                Company ID: \(String.init(format: "0x%04x", vendorData.companyId)) (\(vendorData.companyId))
                Model ID: \(String.init(format: "0x%04x", vendorData.modelId)) (\(vendorData.modelId))
                OpCode: \(String.init(format: "0x%02x", vendorData.opcode)) (\(vendorData.opcode))
                TTL: \(String.init(format: "0x%02x", vendorData.ttl)) (\(vendorData.ttl))
                Data(hex): \(vendorData.data.dumpHexBytes())
                """,
                title: "Information")
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_ON_OFF_STATUS):
            guard let onOffStatus = MeshNotificationConstants.getOnOffStatus(userInfo: userInfo) else { return }
            self.showToast(message: "Device \"\(onOffStatus.deviceName)\" has been turned \(onOffStatus.isOn ? "ON" : "OFF")")
        default:
            break
        }
    }

    func contentViewInit() {
        self.groupName = UserSettings.shared.currentActiveGroupName
        self.deviceName = UserSettings.shared.currentActiveComponentName ?? MeshConstantText.UNKNOWN_DEVICE_NAME
        if let deviceName = self.deviceName {
            self.componentType = MeshFrameworkManager.shared.getMeshComponentType(componentName: deviceName)
        }
        meshLog("ComponentViewController, contentViewInit, devceName:\(String(describing: self.deviceName)), componentType:\(self.componentType)")
        if componentType >= MeshConstants.MESH_COMPONENT_SENSOR_SERVER {
            let vc = self.conponentControlVC
            if let controlVC = vc as? SensorViewController {
                controlVC.groupName = self.groupName
                controlVC.deviceName = self.deviceName
                controlVC.componentType = componentType
            }
        } else if componentType == MeshConstants.MESH_COMPONENT_UNKNOWN {
            let vc = self.conponentControlVC
            if let controlVC = vc as? VendorSpecificDataViewController {
                controlVC.groupName = self.groupName
                controlVC.deviceName = self.deviceName
                controlVC.componentType = componentType
            }
        } else {
            let vc = self.conponentControlVC
            if let controlVC = vc as? GroupDetailsControlsViewController {
                controlVC.isDeviceControl = true
                controlVC.groupName = self.groupName
                controlVC.deviceName = self.deviceName
                controlVC.componentType = componentType
            }
        }
        self.addChild(self.conponentControlVC)
        self.conponentControlVC.didMove(toParent: self)
        self.conponentControlVC.view.frame = self.contentView.bounds
        self.contentView.addSubview(self.conponentControlVC.view)
    }

    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
        if let identifier = segue.identifier {
            meshLog("ComponentViewController, segue.identifier=\(identifier)")
            switch identifier {
            case MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS:
                if let groupDetailVC = segue.destination as? GroupDetailsViewController {
                    groupDetailVC.currentSegmentedSelectedIndex = MeshAppConstants.GROUPT_DETAIL_SEGMENTED_ALL_DEVICE
                }
            default:
                break
            }
        }
    }

    @IBAction func onSettingBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("ComponentViewController, onSettingBarButtonItemClick")
        guard let currentGroupName = self.groupName else {
            meshLog("ComponentViewController, onSettingBarButtonItemClick, invalid group name nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Show Setting button list table failed, invalid group name.")
            return
        }
        var choices = ComponentPopoverChoices.allValues
        if currentGroupName == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME {
            choices.removeAll(where: {$0 == ComponentPopoverChoices.move.rawValue})
            choices.removeAll(where: {$0 == ComponentPopoverChoices.remove.rawValue})
        }
        let controller = PopoverChoiceTableViewController(choices: choices) { (index: Int, selection: String) in
            meshLog("ComponentViewController, onSettingBarButtonItemClick, index=\(index), selection=\(selection)")
            guard let choice = ComponentPopoverChoices.init(rawValue: selection) else { return }

            switch choice {
            case .identify:
                self.onDeviceIdentify()
            case .rename:
                self.onComponentDeviceRename()
            case .delete:
                self.onComponentDeviceDelete()
            case .move:
                self.onComponentDeviceMoveToGroup()
            case .add:
                self.onComponentDeviceAddToGroup()
            case .remove:
                self.onComponentRemoveFromCurrentGroup()
            case .ota:
                self.onFirmwareOta()
            case .setVendorData:
                self.onSetVendorSpecifcData()
            case .configurePublication:
                self.onConfigurePublication()
            }
        }
        controller.preferredContentSize = CGSize(width: 220, height: controller.getPreferredPopoverViewSize())
        controller.showPopoverPresentation(parent: self, sourceView: sender.value(forKey: "view") as? UIView)
    }

    func onDeviceIdentify() {
        meshLog("ComponentViewController, onDeviceIdentify")
        guard let deviceName = self.deviceName else {
            meshLog("error: ComponentViewController, onDeviceIdentify, invalid deviceName nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or nil.")
            return
        }

        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("ComponentViewController, deviceIdentify, device:\(deviceName), failed to connect to mesh network")
                return
            }

            let error = MeshFrameworkManager.shared.meshClientIdentify(name: deviceName, duration: 10)
            meshLog("ComponentViewController, deviceIdentify, device:\(deviceName), error:\(error)")
        }
    }

    func onComponentDeviceRename() {
        meshLog("ComponentViewController, onComponentDeviceRename, show input new name dialogue")
        let alertController = UIAlertController(title: ComponentPopoverChoices.rename.rawValue, message: nil, preferredStyle: .alert)
        alertController.addTextField { (textField: UITextField) in
            textField.placeholder = "Enter New Device Name"
        }
        alertController.addAction(UIAlertAction(title: "Cancel", style: .default, handler: nil))
        alertController.addAction(UIAlertAction(title: "Confirm", style: .default, handler: { (action: UIAlertAction) -> Void in
            guard let textField = alertController.textFields?.first,
                let newDeviceName = textField.text, newDeviceName.count > 0,
                let oldDeviceName = self.deviceName else {
                UtilityManager.showAlertDialogue(parentVC: self, message: "Empty or Invalid new device name!", title: "Error")
                return
            }

            self.operationType = .rename
            self.indicatorView.showAnimating(parentVC: self)
            MeshFrameworkManager.shared.meshClientRename(oldName: oldDeviceName, newName: newDeviceName) { (networkName: String?, error: Int) in
                self.indicatorView.stopAnimating()
                self.operationType = nil
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: ComponentViewController, failed to call meshClientRename with new deviceName=\(newDeviceName), error=\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to rename \"\(oldDeviceName)\" device. Error Code: \(error)")
                    return
                }

                /*
                 * [Dudley]:
                 * Mesh library will always append the mesh unique address string as suffix to avoid device name conflict, the format is " (XXXX)".
                 * Note, the suffix is started to a space, and the XXXX is a for hexdecail value characters.
                 * so, the new new for the mesh device should also be apppended with the mesh unique address string before using it.
                 */
                let newMeshDeviceName = newDeviceName + UtilityManager.getMeshAddressSuffixString(meshComponentName: oldDeviceName)
                self.deviceName = newMeshDeviceName
                if self.componentType >= MeshConstants.MESH_COMPONENT_SENSOR_SERVER,
                    let controlVC = self.conponentControlVC as? SensorViewController {
                    controlVC.deviceName = self.deviceName
                } else if let controlVC = self.conponentControlVC as? GroupDetailsControlsViewController {
                    controlVC.deviceName = self.deviceName
                    controlVC.nameLabel.text = self.deviceName  // update the device name shown in the control view.
                }
                // update device status values in UserSettings.
                if let values = UserSettings.shared.getComponentStatus(componentName: oldDeviceName) {
                    UserSettings.shared.removeComponentStatus(componentName: oldDeviceName)
                    UserSettings.shared.setComponentStatus(componentName: newMeshDeviceName, values: values)
                }

                meshLog("ComponentViewController, rename \"\(oldDeviceName)\" device to new name=\"\(newMeshDeviceName)\" success")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Rename device from \"\(oldDeviceName)\" to \"\(newMeshDeviceName)\" sucess", title: "Success")
            }
        }))
        self.present(alertController, animated: true, completion: nil)
    }

    func onComponentDeviceDelete() {
        guard let deviceName = self.deviceName else {
            meshLog("error: ComponentViewController, onComponentDeviceDeletedelete, invalid deviceName:\(String(describing: self.deviceName))")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to delete the target device!", title: "Error")
            return
        }

        self.operationType = .delete
        self.componentOperationTimer?.invalidate()
        self.componentOperationTimer = Timer.scheduledTimer(timeInterval: 20, target: self, selector: #selector(self.componentOperationTimeoutHandler), userInfo: nil, repeats: false)
        indicatorView.showAnimating(parentVC: self)
        MeshFrameworkManager.shared.meshClientDeleteDevice(deviceName: deviceName) { (_ networkName: String?, _ error: Int) in
            self.resetOperationStatus()
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: ComponentViewController, onComponentDeviceDeletedelete, meshClientDeleteDevice deviceName:\(deviceName) failed, error=\(error)")
                if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh network is busying, please try again to delete device \"\(deviceName)\" a little later.", title: "Warnning")
                } else {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to delete device \"\(deviceName)\". Error Code: \(error).", title: "Error")
                }
                return
            }

            // update device status values in UserSettings.
            UserSettings.shared.removeComponentStatus(componentName: deviceName)

            meshLog("ComponentViewController, onComponentDeviceDelete, the mesh device: \(deviceName) has been deleted success, error=\(error)")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "The mesh device has been deleted successfully.",
                                             title: "Success", completion: nil,
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in
                                                                    self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS, sender: nil)
                                             }))
        }
    }

    func onComponentDeviceAddToGroup() {
        meshLog("ComponentViewController, onComponentDeviceAddToGroup, show input new name dialogue")
        if let groupList = getPopoverSelectionItemList(popoverType: .componentMoveToGroup) {
            PopoverViewController.parentViewController = self
            PopoverViewController.popoverCompletion = onAddToGroupHandler
            PopoverViewController.popoverType = .componentAddToGroup
            PopoverViewController.popoverItems = groupList

            UtilityManager.navigateToViewController(sender: self, targetVCClass: PopoverViewController.self, modalPresentationStyle: UIModalPresentationStyle.overCurrentContext)
        }
    }

    func onAddToGroupHandler(_ btnType: PopoverButtonType, _ selectedItem: String?) {
        guard let deviceName = self.deviceName else {
            meshLog("error: ComponentViewController, onAddToGroupHandler, invalid device name or group name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or group name.")
            return
        }

        // on DB changed callback process.
        if btnType == .none && operationType == .add, let newGroupName = popoverSelectedGroupName {
            self.resetOperationStatus()
            guard let componets = MeshFrameworkManager.shared.getMeshGroupComponents(groupName: newGroupName), componets.filter({$0 == deviceName}).count > 0 else {
                meshLog("error: ComponentViewController, onAddToGroupHandler, failed to find device:\(deviceName) in new group:\(newGroupName)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to add device:\(deviceName) in new group:\(newGroupName)")
                return
            }

            // The mesh device has been added to new success.
            meshLog("ComponentViewController, onAddToGroupHandler, add device:\(deviceName) to:\(newGroupName) success")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "The mesh device has been added to new group successfully.",
                                             title: "Success", completion: nil,
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in
                                                                    self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS, sender: nil)
                                             }))
            return
        }

        // Porcess on user selection for delete device.
        self.resetOperationStatus()
        guard btnType == .confirm, let newGroupName = selectedItem, !newGroupName.isEmpty else {
            if btnType == .confirm {
                meshLog("error: ComponentViewController, onAddToGroupHandler, no add to group name selected")
                UtilityManager.showAlertDialogue(parentVC: self, message: "No or invalid add to group name selected")
            } else {
                meshLog("ComponentViewController, onAddToGroupHandler, cancelled")
            }
            return
        }

        indicatorView.showAnimating(parentVC: self)
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.indicatorView.stopAnimating()
                meshLog("ComponentViewController, onMovedToGroupSelected, unable to connect to the mesh network")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to connect to mesh network caused by unable to add the device to new group. Please try to sync network systus firstly.")
                return
            }

            self.operationType = .add
            self.popoverSelectedGroupName = newGroupName
            self.componentOperationTimer = Timer.scheduledTimer(timeInterval: 20, target: self, selector: #selector(self.componentOperationTimeoutHandler), userInfo: nil, repeats: false)
            MeshFrameworkManager.shared.addMeshComponent(componentName: deviceName, toGroup: newGroupName, completion: { (networkName: String?, error: Int) in
                self.resetOperationStatus()
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: ComponentViewController, onAddToGroupHandler, add device:\(deviceName) to new group:\(newGroupName) failed, error=\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to add the device to selected group. Error Code: \(error)")
                    return
                }
                meshLog("ComponentViewController, onAddToGroupHandler, addMeshComponent command sent success, waiting mesh DB changed event")
            })
        }
    }

    func onComponentDeviceMoveToGroup() {
        meshLog("ComponentViewController, onComponentDeviceMoveToGroup, show input new name dialogue")
        if let groupList = getPopoverSelectionItemList(popoverType: .componentMoveToGroup) {
            PopoverViewController.parentViewController = self
            PopoverViewController.popoverCompletion = onMovedToGroupHandler
            PopoverViewController.popoverType = .componentMoveToGroup
            PopoverViewController.popoverItems = groupList

            UtilityManager.navigateToViewController(sender: self, targetVCClass: PopoverViewController.self, modalPresentationStyle: UIModalPresentationStyle.overCurrentContext)
        }
    }

    func onMovedToGroupHandler(_ btnType: PopoverButtonType, _ selectedItem: String?) {
        guard let deviceName = self.deviceName, let groupName = self.groupName else {
            meshLog("error: ComponentViewController, onMovedToGroupSelected, invalid device name or group name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or group name.")
            return
        }

        // on DB changed callback process.
        if btnType == .none && operationType == .move, let newGroupName = popoverSelectedGroupName {
            self.resetOperationStatus()
            guard let groups = MeshFrameworkManager.shared.getMeshComponentGroupList(componentName: deviceName), groups.count > 0 else {
                meshLog("error: ComponentViewController, onMovedToGroupHandler, failed to get device group list, groups is nil")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to connect to mesh network caused by unable to move the device to new group. Please try to sync network status firstly.")
                return
            }

            if groups.filter({$0 == newGroupName}).first != nil &&
                (groups.filter({$0 == groupName}).first == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME || groups.filter({$0 == groupName}).first == nil) {
                // The mesh device has been moved success.
                meshLog("ComponentViewController, onNetworkDatabaseChanged, move device:\(deviceName) from:\(groupName) to:\(String(describing: popoverSelectedGroupName)) success")
                UtilityManager.showAlertDialogue(parentVC: self,
                                                 message: "The mesh device has been moved to new group successfully.",
                                                 title: "Success", completion: nil,
                                                 action: UIAlertAction(title: "OK", style: .default,
                                                                       handler: { (action) in
                                                                        self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS, sender: nil)
                                                 }))
            } else {
                meshLog("error: ComponentViewController, onNetworkDatabaseChanged, failed to move device:\(deviceName) from:\(groupName) to:\(String(describing: popoverSelectedGroupName))")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to move mesh device to new group.")
            }
            return
        }

        // Porcess on user selection for delete device.
        self.resetOperationStatus()
        guard btnType == .confirm, let newGroupName = selectedItem, !newGroupName.isEmpty else {
            if btnType == .confirm {
                meshLog("error: ComponentViewController, onMovedToGroupSelected, no moved to group name selected")
                UtilityManager.showAlertDialogue(parentVC: self, message: "No or invalid moved to group name selected")
            } else {
                meshLog("ComponentViewController, onMovedToGroupSelected, cancelled")
            }
            return
        }

        indicatorView.showAnimating(parentVC: self)
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.indicatorView.stopAnimating()
                meshLog("ComponentViewController, onMovedToGroupSelected, unable to connect to the mesh network, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to connect to mesh network caused by unable to move the device to new group. Please try to sync network systus firstly.")
                return
            }

            self.operationType = .move
            self.popoverSelectedGroupName = newGroupName
            self.componentOperationTimer = Timer.scheduledTimer(timeInterval: 60, target: self, selector: #selector(self.componentOperationTimeoutHandler), userInfo: nil, repeats: false)

            ///
            /// Use the method of adding the device to new group, then removed from old group instead of using move API.
            ///
            // self.usingMoveToGroupApi = false  // enable this line for test purpose.
            if !self.usingMoveToGroupApi {
                MeshFrameworkManager.shared.addMeshComponent(componentName: deviceName, toGroup: newGroupName) { (networkName: String?, error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        self.resetOperationStatus()
                        meshLog("error: ComponentViewController, onMovedToGroupHandler, failed to add device:\(deviceName) to group:\(newGroupName), error:\(error)")
                        UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to add the component device to the target move to group. Error Code: \(error).")
                        return
                    }

                    if groupName == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME {
                        self.resetOperationStatus()
                        meshLog("ComponentViewController, onMovedToGroupHandler, move device:\(deviceName) from group:\(groupName) to group:\(newGroupName) success")
                        UtilityManager.showAlertDialogue(parentVC: self,
                                                         message: "The mesh device has been moved to new group successfully.",
                                                         title: "Success", completion: nil,
                                                         action: UIAlertAction(title: "OK", style: .default,
                                                                               handler: { (action) in
                                                                                self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS, sender: nil)
                                                         }))
                        return
                    }

                    MeshFrameworkManager.shared.removeMeshComponent(componentName: deviceName, fromGroup: groupName, completion: { (networkName: String?, error: Int) in
                        self.resetOperationStatus()
                        guard error == MeshErrorCode.MESH_SUCCESS else {
                            meshLog("error: ComponentViewController, onMovedToGroupHandler, failed to remove device:\(deviceName) from group:\(groupName), error:\(error)")
                            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to remove the component device from original group after moved to the target group. Error Code: \(error).")
                            return
                        }

                        meshLog("ComponentViewController, onMovedToGroupHandler, move device:\(deviceName) from group:\(groupName) to group:\(newGroupName) success")
                        UtilityManager.showAlertDialogue(parentVC: self,
                                                         message: "The mesh device has been moved to new group successfully.",
                                                         title: "Success", completion: nil,
                                                         action: UIAlertAction(title: "OK", style: .default,
                                                                               handler: { (action) in
                                                                                self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS, sender: nil)
                                                         }))
                    })
                }
                return
            }

            ///
            /// Use the method of calling the moveMeshComponent() API to implement the move to new group operations.
            ///

            // In this demo, the default group will contain all devices, when the device is moved from the default group, it means added to the new group.
            if groupName == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME {
                if let subscribedGroups = MeshFrameworkManager.shared.getMeshComponentGroupList(componentName: deviceName) {
                    let nonDefaultGroups = subscribedGroups.filter({$0 != MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME})
                    if nonDefaultGroups.count > 0 {
                        self.resetOperationStatus()
                        // The device has been added into other group beside the default group.
                        meshLog("error: ComponentViewController, onMovedToGroupHandler, \(deviceName) has been added in the \(nonDefaultGroups[0]) group")
                        UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to move this device to group \"\(newGroupName)\", it has been added in the \"\(nonDefaultGroups[0])\" group. Please navigate to the \"\(nonDefaultGroups[0])\" group detail page, then move this device to the target group to make sure it's the expected operation.")
                        return
                    } else {
                        // The device only exsiting in the default group, so add it to the target group.
                        MeshFrameworkManager.shared.addMeshComponent(componentName: deviceName, toGroup: newGroupName) { (networkName: String?, error: Int) in

                        }
                    }
                } else {
                    // The device doesn't exist in any group, try to added into new group.
                    MeshFrameworkManager.shared.addMeshComponent(componentName: deviceName, toGroup: newGroupName) { (networkName: String?, error: Int) in

                    }
                }
            } else {
                MeshFrameworkManager.shared.moveMeshComponent(componentName: deviceName, fromGroup: groupName, toGroup: newGroupName) { (networkName: String?, error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        self.resetOperationStatus()
                        meshLog("error: ComponentViewController, moveMeshComponent, \(deviceName) from \(groupName) to \(newGroupName) failed, error=\(error)")
                        UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to move the device to selected group. Error Code: \(error)")
                        return
                    }
                    meshLog("ComponentViewController, moveMeshComponent command sent success, waiting mesh DB changed event")
                }
            }
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.resetOperationStatus()
                meshLog("error: ComponentViewController, moveMeshComponent, \(deviceName) from \(groupName) to \(newGroupName) failed, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to move the device to selected group. Error Code: \(error)")
                return
            }
            meshLog("ComponentViewController, moveMeshComponent command sent success, waiting mesh DB changed event")
        }
    }

    @objc func componentOperationTimeoutHandler() {
        componentOperationTimer?.invalidate()
        componentOperationTimer = nil
        meshLog("error: ComponentViewController, componentOperationTimeoutHandler, operation timeout, operationType=\(String(describing: operationType?.rawValue))")
        if let opType = operationType {
            switch opType {
            case .delete:
                onComponentDeviceDelete()
            case .move:
                onMovedToGroupHandler(.none, nil)
            default:
                UtilityManager.showAlertDialogue(parentVC: self, message: "\"\(opType.rawValue)\" operation timeout.")
                break
            }
        }
        indicatorView.stopAnimating()
    }

    func getPopoverSelectionItemList(popoverType: PopoverType) -> [String]? {
        guard let deviceName = self.deviceName,
            let networkName = MeshFrameworkManager.shared.getOpenedMeshNetworkName(),
            var groupList = MeshFrameworkManager.shared.getAllMeshNetworkGroups(networkName: networkName) else {
                meshLog("error: ComponentViewController, getPopoverGroupList, Failed to get group list")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to get group list.")
                return nil
        }
        // Do not show the group names that currently subscribed for this device.
        if let subscribedGroups = MeshFrameworkManager.shared.getMeshComponentGroupList(componentName: deviceName) {
            for group in subscribedGroups {
                groupList.removeAll(where: {$0 == group})
            }
        }
        groupList.removeAll(where: {$0 == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME})
        guard groupList.count > 0 else {
            meshLog("error: ComponentViewController, getPopoverGroupList, No other group can be added or moved to. Currnetly scribed groups: \(groupList)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "No other group can be selected. Please create a new group firstly.")
            return nil
        }

        return groupList
    }

    func onComponentRemoveFromCurrentGroup() {
        guard let deviceName = self.deviceName, let groupName = self.groupName else {
                meshLog("error: ComponentViewController, onComponentRemoveFromCurrentGroup, invalid device name or group name")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or group name is nil.")
                return
        }

        meshLog("ComponentViewController, onComponentRemoveFromCurrentGroup, remove device:\"\(deviceName)\" from group:\"\(groupName)\"")
        indicatorView.showAnimating(parentVC: self)
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.indicatorView.stopAnimating()
                meshLog("error: ComponentViewController, onComponentRemoveFromCurrentGroup, failed to connect to mesh network, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to connect to the mesh network. Error Code: \(error)")
                return
            }

            MeshFrameworkManager.shared.removeMeshComponent(componentName: deviceName, fromGroup: groupName)  { (networkName: String?, error: Int) in
                self.indicatorView.stopAnimating()
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: ComponentViewController, onComponentRemoveFromCurrentGroup, invalid device name or group name, error=\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to remove device:\"\(deviceName)\" from group:\"\(groupName)\". Error Code: \(error)")
                    return
                }

                // removed success.
                meshLog("ComponentViewController, onComponentRemoveFromCurrentGroup, remove device:\"\(deviceName)\" from group:\"\(groupName)\" message has been sent")
                self.onRemovedFromGroupHandler()
            }
        }
    }

    func onRemovedFromGroupHandler() {
        guard let deviceName = self.deviceName, let groupName = self.groupName else {
            meshLog("error: ComponentViewController, onRemovedFromGroupHandler, invalid device name or group name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or group name.")
            return
        }

        self.resetOperationStatus()
        if let groups = MeshFrameworkManager.shared.getMeshComponentGroupList(componentName: deviceName), let _ = groups.filter({$0 == groupName}).first {
            meshLog("error: ComponentViewController, onRemovedFromGroupHandler, failed to remove device:\(deviceName) from group:\(groupName)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to remove device:\(deviceName) from group:\(groupName)")
        } else {
            meshLog("ComponentViewController, onRemovedFromGroupHandler, Remove device:\(deviceName) from group:\(groupName) success")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Device:\(deviceName) has been removed from group:\(groupName) successfully.", title: "Success")
        }
    }

    func onFirmwareOta() {
        meshLog("ComponentViewController, onFirmwareOta, go to device firmware OTA page")
        meshLog("ComponentViewController, IS_MESH_DFU_ENABLED: \(IS_MESH_DFU_ENABLED)")
        if IS_MESH_DFU_ENABLED, let vc = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: String(describing: MeshOtaDfuViewController.self)) as? MeshOtaDfuViewController {
            vc.modalPresentationStyle = .fullScreen
            vc.deviceName = self.deviceName
            vc.groupName = self.groupName
            if let deviceName = self.deviceName {
                OtaManager.shared.activeOtaDevice = OtaMeshDevice(meshName: deviceName)
            }
            self.present(vc, animated: true, completion: nil)
        } else if let vc = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: String(describing: DeviceOtaUpgradeViewController.self)) as? DeviceOtaUpgradeViewController {
            vc.modalPresentationStyle = .fullScreen
            vc.deviceName = self.deviceName
            vc.groupName = self.groupName
            if let deviceName = self.deviceName {
                OtaManager.shared.activeOtaDevice = OtaMeshDevice(meshName: deviceName)
            }
            self.present(vc, animated: true, completion: nil)
        } else {
            if IS_MESH_DFU_ENABLED {
                meshLog("error: ComponentViewController, onFirmwareOta, failed to instantiateViewController: MeshOtaDfuViewController")
            } else {
                meshLog("error: ComponentViewController, onFirmwareOta, failed to instantiateViewController: DeviceOtaUpgradeViewController")
            }
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to instantiate OTA View Controller.", title: "Error")
        }
    }

    func onSetVendorSpecifcData() {
        meshLog("ComponentViewController, onSetVendorData, show input vendor specific data dialogue")

        guard let deviceName = self.deviceName, let _ = self.groupName else {
            meshLog("error: ComponentViewController, onSetVendorSpecifcData, invalid device name or group name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid device name or group name.")
            return
        }

        let alertController = UIAlertController(title: "Set Vendor Specific Data", message: nil, preferredStyle: .alert)
        alertController.addTextField { (textField: UITextField) in
            textField.borderStyle = UITextField.BorderStyle.roundedRect
            textField.placeholder = "\(MeshConstants.MESH_VENDOR_COMPANY_ID)  Vendor Copany Id (optional, decimal) "
            textField.returnKeyType = UIReturnKeyType.next
        }
        alertController.addTextField { (textField: UITextField) in
            textField.borderStyle = UITextField.BorderStyle.roundedRect
            textField.placeholder = "\(MeshConstants.MESH_VENDOR_MODEL_ID)  Vendor Model Id (optional, decimal)"
            textField.returnKeyType = UIReturnKeyType.next
        }
        alertController.addTextField { (textField: UITextField) in
            textField.borderStyle = UITextField.BorderStyle.roundedRect
            textField.placeholder = "\(MeshConstants.MESH_VENDOR_OPCODE1)  OpCode (optional, decimal)"
            textField.returnKeyType = UIReturnKeyType.next
        }
        alertController.addTextField { (textField: UITextField) in
            textField.borderStyle = UITextField.BorderStyle.roundedRect
            textField.placeholder = "Vendor Specific Data (hexdecimal)"
            textField.returnKeyType = UIReturnKeyType.done
        }
        alertController.addAction(UIAlertAction(title: "Cancel", style: .default, handler: nil))
        alertController.addAction(UIAlertAction(title: "Confirm", style: .default, handler: { (action: UIAlertAction) -> Void in
            guard alertController.textFields?.count == 4, let data = UtilityManager.convertHexDigitStringToData(hexDigit: alertController.textFields?[3].text) else {
                UtilityManager.showAlertDialogue(parentVC: self, message: "No vendor specific data set, please input hexdecimal vendor specific data firstly!", title: "Error")
                return
            }

            let companyId: Int! = UtilityManager.convertDigitStringToInt(digit: alertController.textFields?.first?.text) ?? MeshConstants.MESH_VENDOR_COMPANY_ID
            let modelId: Int! = UtilityManager.convertDigitStringToInt(digit: alertController.textFields?[1].text) ?? MeshConstants.MESH_VENDOR_MODEL_ID
            let opcode: Int! = UtilityManager.convertDigitStringToInt(digit: alertController.textFields?[2].text) ?? MeshConstants.MESH_VENDOR_OPCODE1

            MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected(handler: { (error) in
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: ComponentViewController, onSetVendorSpecifcData. Error Code: \(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to set the vendor data, Mesh network not connected yet!")
                    return
                }

                meshLog("onSetVendorSpecifcData, deviceName: \(deviceName), companyId: \(String(describing: companyId)), modelId: \(String(describing: modelId)), opcode: \(String(describing: opcode)), data: \(data.dumpHexBytes())")
                let error = MeshFrameworkManager.shared.meshClientVendorDataSet(deviceName: deviceName, companyId: companyId, modelId: modelId, opcode: opcode, disable_ntwk_retransmit: MeshConstants.MESH_VENDOR_DISABLE_NETWORK_RETRANSMIT, data: data)
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("ComponentViewController, onSetVendorSpecifcData, failed to send out data, error: \(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to send out the vendor data, please check the values of Company Id, Model Id and OpCode are all set correctly! Error Code: \(error)", title: "Error")
                    return
                }
                meshLog("ComponentViewController, onSetVendorSpecifcData, send out success")
            })
        }))
        self.present(alertController, animated: true, completion: nil)
    }

    func onConfigurePublication() {
        guard let vc = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.PUBLICATION_CONFIGURE) as? PublicationConfigureViewController else {
            meshLog("error: ComponentViewController, onConfigurePublication, failed to load \(MeshAppStoryBoardIdentifires.PUBLICATION_CONFIGURE) View Controller")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to load \(MeshAppStoryBoardIdentifires.PUBLICATION_CONFIGURE) View Controller.", title: "Error")
            return
        }
        vc.deviceName = self.deviceName
        vc.groupName = self.groupName
        vc.componentType = self.componentType
        vc.modalPresentationStyle = .fullScreen
        self.present(vc, animated: true, completion: nil)
    }

    func resetOperationStatus() {
        self.componentOperationTimer?.invalidate()
        self.componentOperationTimer = nil
        self.indicatorView.stopAnimating()
        self.operationType = .none
        self.popoverSelectedGroupName = nil
    }
}
