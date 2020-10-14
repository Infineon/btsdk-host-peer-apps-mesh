/*
 * Copyright 2016-2020, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
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
 * Sensor view controller implementation.
 */

import UIKit
import MeshFramework


/*
 * [Dudley] Note, SensorViewController contains a reserved top navigation bar,
 * because it will be loaded into ComponentViewController as content view,
 * so the top navigation bar should not be displayed.
 * But it cannot be deleted, the height has been set to 0 (default height is 44),
 * because the device name view is layouted based on it, so cannot delete it.
 */
class SensorViewController: UIViewController {
    @IBOutlet weak var deviceNameView: UIView!
    @IBOutlet weak var deviceNameLabel: UILabel!
    @IBOutlet weak var deviceControlView: CustomCardView!
    @IBOutlet weak var devicePropertyIdLabel: UILabel!
    @IBOutlet weak var devicePropertyIdDropListButton: CustomDropDownButton!
    @IBOutlet weak var deviceGetVersionButton: UIButton!
    @IBOutlet weak var deviceGetSensorDataButton: UIButton!
    @IBOutlet weak var deviceConfigureButton: UIButton!
    @IBOutlet weak var deviceDataScribeLabel: UILabel!
    @IBOutlet weak var deviceDataSubscribeSwitch: UISwitch!
    @IBOutlet weak var sensorDataView: CustomCardView!
    @IBOutlet weak var sensorDataTitleLabel: UILabel!
    @IBOutlet weak var sensorDataClearButton: UIButton!
    @IBOutlet weak var sensorDataDisplayTextView: UITextView!
    @IBOutlet weak var sensorDataListoningIndicator: UIActivityIndicatorView!

    private var vcOpenedTime = Date(timeIntervalSinceNow: 0)
    private var isGetVersionInProgressing: Bool = false
    private var isGetSensorDataInProgressing: Bool = false

    var groupName: String?
    var deviceName: String? {
        didSet {
            if deviceNameLabel != nil {
                deviceNameLabel.text = deviceName ?? MeshConstantText.UNKNOWN_DEVICE_NAME
            }
        }
    }
    var componentType: Int = MeshConstants.MESH_COMPONENT_UNKNOWN
    private var selectedPropertyId: Int?
    private var sensorDataSubscribedStatus: [Int: Bool] = [MeshPropertyId.UNKNOWN: false]

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        viewInit()
    }

    func viewInit() {
        deviceNameLabel.text = deviceName ?? MeshConstantText.UNKNOWN_DEVICE_NAME
        if let deviceName = self.deviceName, componentType == MeshConstants.MESH_COMPONENT_UNKNOWN {
            componentType = MeshFrameworkManager.shared.getMeshComponentType(componentName: deviceName)
        }

        sensorDataDisplayTextView.text = ""
        sensorDataDisplayTextView.layer.borderWidth = 1
        sensorDataDisplayTextView.layer.borderColor = UIColor.gray.cgColor
        sensorDataDisplayTextView.isEditable = false
        sensorDataDisplayTextView.isSelectable = false
        sensorDataDisplayTextView.layoutManager.allowsNonContiguousLayout = false

        // always set to false by deafult, because we don't know if the device publish data still subscribed or not.
        deviceDataSubscribeSwitch.isOn = false
        devicePropertyIdDropListButton.dropDownItems = getPropertyIds()
        devicePropertyIdDropListButton.setSelection(select: 0)
        updateSelectedPropertyId()

        updateListoningAnimation()
    }

    func getPropertyIds() -> [String] {
        guard let componentName = self.deviceName else {
            return []
        }

        guard let propertyList = MeshFrameworkManager.shared.meshClientSensorPropertyListGet(componentName: componentName) else {
            return []
        }

        var propertyIds: [String] = []
        for propertyId in propertyList {
            let propertyIdText = MeshPropertyId.getPropertyIdText(propertyId)
            if propertyIdText.hasPrefix(MeshPropertyId.UNKNOWN_TEXT) && false {  // enable it if do not want show the unknown items.
                continue
            }
            propertyIds.append(propertyIdText)
            sensorDataSubscribedStatus[propertyId] = false
        }
        return propertyIds
    }

    func updateSelectedPropertyId() {
        guard !devicePropertyIdDropListButton.selectedString.isEmpty else {
            return  // No real property id existed and selected.
        }

        selectedPropertyId = MeshPropertyId.getPropertyIdByText(devicePropertyIdDropListButton.selectedString)
        deviceDataSubscribeSwitch.isOn = sensorDataSubscribedStatus[selectedPropertyId ?? MeshPropertyId.UNKNOWN] ?? false
    }

    func notificationInit() {
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED), object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_SENSOR_STATUS), object: nil)
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
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_SENSOR_STATUS):
            if let sensorStatus = MeshNotificationConstants.getSensorStatus(userInfo: userInfo), let deviceName = self.deviceName {
                if sensorStatus.deviceName == deviceName, deviceDataSubscribeSwitch.isOn {
                    UserSettings.shared.setComponentStatus(componentName: sensorStatus.deviceName,
                                                           values: [MeshComponentValueKeys.data: sensorStatus.data])

                    parseSensorData(deviceName: deviceName, propertyId: sensorStatus.propertyId, value: sensorStatus.data)
                }
            }
        default:
            break
        }
    }

    @IBAction func onDevicePropertyIdDropListButtonClick(_ sender: CustomDropDownButton) {
        meshLog("SensorViewController, onDevicePropertyIdDropListButtonClick")
        devicePropertyIdDropListButton.showDropList(width: 220, parent: self) {
            self.updateSelectedPropertyId()

            meshLog("SensorViewController, onDevicePropertyIdDropListButtonClick, selectedIndex=\(self.devicePropertyIdDropListButton.selectedIndex), selectedString=\(self.devicePropertyIdDropListButton.selectedString), selectedPropertyId=\(String(describing: self.selectedPropertyId))")
        }
    }

    func log(_ message: String) {
        let seconds = Date().timeIntervalSince(vcOpenedTime)
        let msg = String(format: "[%.3f] \(message)\n", seconds)
        sensorDataDisplayTextView.text += msg
        let bottom = NSRange(location: sensorDataDisplayTextView.text.count, length: 1)
        sensorDataDisplayTextView.scrollRangeToVisible(bottom)
    }

    func parseSensorData(deviceName: String, propertyId: Int, value: Data) {
        switch propertyId {
        case MeshPropertyId.LIGHT_CONTROL_AMBIENT_LUX_LEVEL_ON:
            if let luxValue = MeshPropertyId.parseAmbientLuxLevelOnValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(String(format: "%.2f", luxValue))")
            }
        case MeshPropertyId.MOTION_SENSED:
            if let motionSensed = MeshPropertyId.parseMotionSensedValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(String(format: "%.2f", motionSensed))")
            }
        case MeshPropertyId.PRESENCE_DETECTED:
            if let presenceDetected = MeshPropertyId.parsePresenceDetectedValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(presenceDetected ? 1 : 0)")
            }
        case MeshPropertyId.PRESENT_AMBIENT_LIGHT_LEVEL:
            if let lightLevel = MeshPropertyId.parseAmbientLightLevelValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(String(format: "%.2f", lightLevel))")
            }
        case MeshPropertyId.PRESENT_AMBIENT_TEMPERATURE:
            if let temperature = MeshPropertyId.parseAmbientTemperatureValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(String(format: "%.2f", temperature))")
            }
        case MeshPropertyId.TOTAL_DEVICE_RUNTIME:
            if let runtime = MeshPropertyId.parseTotalDeviceRuntimeValue(value) {
                log("\(MeshPropertyId.getPropertyIdText(propertyId)): \(runtime)")
            }
        default:
            break
        }
    }

    @IBAction func onDeviceGetVersionButtonClick(_ sender: UIButton) {
        meshLog("SensorViewController, onDeviceGetVersionButtonClick")
        guard let deviceName = self.deviceName else {
            meshLog("error: SensorViewController, onDeviceGetVersionButtonClick, invalid deviceName nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid or unknown of the sensor device name.")
            return
        }

        isGetVersionInProgressing = true
        updateListoningAnimation()
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.isGetVersionInProgressing = false
                self.updateListoningAnimation()
                meshLog("error: SensorViewController, onDeviceGetVersionButtonClick, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            MeshFrameworkManager.shared.getMeshComponentInfo(componentName: deviceName) { (componentName: String, componentInfo: String?, error: Int) in
                self.isGetVersionInProgressing = false
                self.updateListoningAnimation()
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: SensorViewController, onDeviceGetVersionButtonClick, failed to call getMeshComponentInfo API, error:\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to get Mesh Component Inforamtion. Error Code: \(error).")
                    return
                }
                guard let componentInfo = componentInfo else {
                    meshLog("error: SensorViewController, onDeviceGetVersionButtonClick, invalid componentInfo: nil for componentName:\(componentName)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid or empty component information value received.")
                    return
                }

                let componentInfoValue = MeshComponentInfo(componentInfo: componentInfo)
                self.log("Get Version: \(componentInfoValue.description)")
            }
        }
    }

    @IBAction func onDeviceGetSensorDataButtonClick(_ sender: UIButton) {
        meshLog("SensorViewController, onDeviceGetSensorDataButtonClick")
        guard let deviceName = self.deviceName else {
            meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, invalid deviceName nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid or unknown of the sensor device name.")
            return
        }
        guard let propertyId = selectedPropertyId else {
            meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, invalid selectedPropertyId nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid Property ID value, please selet a valid Property ID firstly.")
            return
        }

        isGetSensorDataInProgressing = true
        updateListoningAnimation()
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.isGetSensorDataInProgressing = false
                self.updateListoningAnimation()
                meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            // When sensor data subscribed, call the meshClientSensorGet without completion to let sensor data received through notificaiton to avoid process twice.
            if self.deviceDataSubscribeSwitch.isOn {
                let error = MeshFrameworkManager.shared.meshClientSensorGet(componentName: deviceName, propertyId: propertyId)
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, failed to call meshClientSensorGet API, error:\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to send out Sensor Get comamnd. Error Code: \(error).")
                    return
                }
                meshLog("SensorViewController, onDeviceGetSensorDataButtonClick, meshClientSensorGet API called success, sensor data should be received through notification")
                return
            }

            MeshFrameworkManager.shared.meshClientSensorGet(componentName: deviceName, propertyId: propertyId, completion: { (deviceName: String, propertyId: Int, data: Data?, error: Int) in
                self.isGetSensorDataInProgressing = false
                self.updateListoningAnimation()
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, failed to call meshClientSensorGet API, error:\(error)")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to send out Sensor Get comamnd or wait for sensor data timed out. Error Code: \(error).")
                    return
                }
                guard let data = data else {
                    meshLog("error: SensorViewController, onDeviceGetSensorDataButtonClick, invalid received sensor data nil, ignore")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid sensor data received, ignored.")
                    return
                }

                meshLog("SensorViewController, onDeviceGetSensorDataButtonClick, meshClientSensorGet done success")
                UserSettings.shared.setComponentStatus(componentName: deviceName, values: [MeshComponentValueKeys.data: data])
                self.parseSensorData(deviceName: deviceName, propertyId: propertyId, value: data)
            })
        }
    }

    @IBAction func onDeviceDataSubscribeSwitchClick(_ sender: UISwitch) {
        meshLog("SensorViewController, onDeviceDataSubscribeSwitchClick, deviceDataSubscribeSwitch.isOn=\(deviceDataSubscribeSwitch.isOn)")
        guard let deviceName = self.deviceName else {
            meshLog("error: SensorViewController, onDeviceDataSubscribeSwitchClick, invalid sensor device name: nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid sensor device name: nil.")
            return
        }
        guard let groupName = self.groupName else {
            meshLog("error: SensorViewController, onDeviceDataSubscribeSwitchClick, invalid group name: nil")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid group name: nil.")
            return
        }

        let publishTarget = MeshFrameworkManager.shared.meshClientGetPublicationTarget(componentName: deviceName, isClient: false, method: MeshControl.METHOD_SENSOR)
        let publishPeriod = MeshFrameworkManager.shared.meshClientGetPublicationPeriod(componentName: deviceName, isClient: false, method: MeshControl.METHOD_SENSOR)
        if publishTarget != groupName || publishPeriod == 0 {
            self.deviceDataSubscribeSwitch.isOn = !self.deviceDataSubscribeSwitch.isOn
            meshLog("warnning: SensorViewController, onDeviceDataSubscribeSwitchClick, please set publish setting firstly.")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Sensor data publish setting not set, please configure the sensor device to publish data to \"\(groupName)\" and set the publish period time to non-zero value firstly.",
                                             title: "Warnning",
                                             cancelHandler: { (action: UIAlertAction) in return },
                                             okayHandler: { (action: UIAlertAction) in
                                                self.performSegue(withIdentifier: MeshAppStoryBoardIdentifires.SEGUE_SENSOR_CONFIGURE, sender: nil)
                                                }
                                             )
            return
        }

        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.deviceDataSubscribeSwitch.isOn = !self.deviceDataSubscribeSwitch.isOn
                meshLog("error: SensorViewController, onDeviceDataSubscribeSwitchClick, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            let error = MeshFrameworkManager.shared.meshClientlistenForAppGroupBroadcast(controlMethod: MeshControl.METHOD_SENSOR, groupName: groupName, startListening: self.deviceDataSubscribeSwitch.isOn)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.deviceDataSubscribeSwitch.isOn = !self.deviceDataSubscribeSwitch.isOn
                meshLog("error: SensorViewController, onDeviceDataSubscribeSwitchClick, failed to call meshClientlistenForAppGroupBroadcast API, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to subscribe for Sensor data. Error Code: \(error).")
                return
            }

            meshLog("SensorViewController, onDeviceDataSubscribeSwitchClick, \(self.deviceDataSubscribeSwitch.isOn ? "subscribe to" : "stop subscribe the") sensor data done success.")
            if let propertyId = self.selectedPropertyId {
                self.sensorDataSubscribedStatus[propertyId] = self.deviceDataSubscribeSwitch.isOn
            }
            self.updateListoningAnimation()
        }
    }

    @IBAction func onSensorDataClearButtonClick(_ sender: UIButton) {
        meshLog("SensorViewController, onSensorDataClearButtonClick")
        sensorDataDisplayTextView.text = ""
    }

    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
        if let identifier = segue.identifier {
            meshLog("ComponentViewController, segue.identifier=\(identifier)")
            switch identifier {
            case MeshAppStoryBoardIdentifires.SEGUE_SENSOR_CONFIGURE:
                if let sensorSettingVC = segue.destination as? SensorSettingViewController {
                    sensorSettingVC.groupName = self.groupName
                    sensorSettingVC.componentName = self.deviceName
                    sensorSettingVC.componentType = self.componentType
                    sensorSettingVC.propertyId = self.selectedPropertyId ?? MeshPropertyId.UNKNOWN
                }
            default:
                break
            }
        }
    }

    func updateListoningAnimation() {
        if isGetVersionInProgressing || isGetSensorDataInProgressing || deviceDataSubscribeSwitch.isOn {
            sensorDataListoningIndicator.startAnimating()
            sensorDataListoningIndicator.isHidden = false
        } else {
            sensorDataListoningIndicator.stopAnimating()
            sensorDataListoningIndicator.isHidden = true
        }
    }
}
