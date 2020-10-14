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
 * Sensor setting view controller implementation.
 */

import UIKit
import MeshFramework

fileprivate enum SensorSettingCustomDropDownViewTag: Int {
    case publishTo
    case cadenceMeasurementType
    case cadenceMeasurementLoaction
    case cadencePublishPeriod
    case sensorProperties
}

class SensorSettingViewController: UIViewController {
    @IBOutlet weak var titleNavigationItem: UINavigationItem!
    @IBOutlet weak var leftNavigationBarButtonitem: UIBarButtonItem!
    @IBOutlet weak var rightNavigationBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var publishSettingsCardView: CustomCardView!
    @IBOutlet weak var cadenceSettingsCardView: CustomCardView!
    @IBOutlet weak var SensorSettingsCardView: CustomCardView!
    @IBOutlet weak var publishToDropListButton: CustomDropDownButton!
    @IBOutlet weak var publishPeriodTimeTextView: UITextField!
    @IBOutlet weak var publishSetButton: UIButton!

    @IBOutlet weak var cadenceStatusTriggerTypeDropDwonButton: CustomDropDownButton!

    @IBOutlet weak var cadenceSettingsFastCadenceButton: UIButton!
    @IBOutlet weak var cadenceSettingsTriggersButton: UIButton!
    @IBOutlet weak var cadenceSetButton: UIButton!

    @IBOutlet weak var fastCadenceMeasurementDropListbutton: CustomDropDownButton!
    @IBOutlet weak var fastCadenceRangeMinValueTextView: UITextField!
    @IBOutlet weak var fastCadenceRangeMaxValueTextView: UITextField!
    @IBOutlet weak var fastCadencePublishPeriodDropListButton: CustomDropDownButton!

    @IBOutlet weak var fastCadenceView: UIView!
    @IBOutlet weak var triggerView: UIView!
    @IBOutlet weak var triggerPublishNoMoreThanSecondsTextView: UITextField!
    @IBOutlet weak var triggerPublishIncreaseUnitTextView: UITextField!
    @IBOutlet weak var triggerPublishDecreaseUnitTextView: UITextField!

    @IBOutlet weak var sensorPropertyDropListButton: CustomDropDownButton!
    @IBOutlet weak var sensorSettingPropertyValueTextView: UITextField!
    @IBOutlet weak var sensorSettingSetButton: UIButton!
    @IBOutlet weak var scrollView: UIScrollView!

    var groupName: String?
    var componentName: String?
    var componentType: Int = MeshConstants.MESH_COMPONENT_UNKNOWN
    var propertyId: Int = MeshPropertyId.UNKNOWN

    var fastCadenceViewRect: CGRect?
    var triggerViewRect: CGRect?

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        viewInit()
    }

    func viewInit() {
        titleNavigationItem.rightBarButtonItem = nil

        cadenceSettingsFastCadenceButton.titleEdgeInsets.left = 10
        cadenceSettingsTriggersButton.titleEdgeInsets.left = 10

        cadenceSettingsFastCadenceButton.setImage(UIImage(named: MeshAppImageNames.checkboxFilledImage), for: .selected)
        cadenceSettingsTriggersButton.setImage(UIImage(named: MeshAppImageNames.checkboxFilledImage), for: .selected)

        sensorSettingSetButton.setTitleColor(UIColor.lightGray, for: .disabled)

        publishPeriodTimeTextView.delegate = self
        fastCadenceRangeMinValueTextView.delegate = self
        fastCadenceRangeMaxValueTextView.delegate = self
        triggerPublishNoMoreThanSecondsTextView.delegate = self
        triggerPublishIncreaseUnitTextView.delegate = self
        triggerPublishDecreaseUnitTextView.delegate = self
        sensorSettingPropertyValueTextView.delegate = self

        initDefaultValues()
    }

    func initDefaultValues() {
        guard let componentName = self.componentName else {
            return
        }

        publishToDropListButton.dropDownItems = getPublishToTargets()
        cadenceStatusTriggerTypeDropDwonButton.dropDownItems = MeshControl.TRIGGER_TYPE_TEXT_LIST
        fastCadenceMeasurementDropListbutton.dropDownItems = MeshControl.MEASUREMENT_TYPE_TEXT_LIST
        fastCadencePublishPeriodDropListButton.dropDownItems = MeshControl.FAST_CADENCE_PERIOD_DIVISOR_TEXT_LIST
        let sensorSettingProperties = getSensorSettingProperties()
        sensorPropertyDropListButton.dropDownItems = sensorSettingProperties

        cadenceSettingsFastCadenceButton.isSelected = UserSettings.shared.isSensorFastCadenceEnabled
        onFastCadenceButtonSelected(isSelected: cadenceSettingsFastCadenceButton.isSelected)
        cadenceSettingsTriggersButton.isSelected = UserSettings.shared.isSensorTriggerPubEnabled
        onTriggerButtonSelected(isSelected: cadenceSettingsTriggersButton.isSelected)
        sensorSettingEnable(isEnabled: !sensorSettingProperties.isEmpty)

        // initialize Publication Settings.
        if let publishTarget = MeshFrameworkManager.shared.meshClientGetPublicationTarget(componentName: componentName, isClient: false, method: MeshControl.METHOD_SENSOR) {
            publishToDropListButton.setSelection(select: publishTarget)
        } else {
            publishToDropListButton.setSelection(select: 0)
        }
        let publishPeriod = MeshFrameworkManager.shared.meshClientGetPublicationPeriod(componentName: componentName, isClient: false, method: MeshControl.METHOD_SENSOR)
        if publishPeriod > 0 {
            publishPeriodTimeTextView.text = "\(publishPeriod)"
        } else {
            publishPeriodTimeTextView.text = "10000"    // set to default 10000ms.
        }

        // initialize Cadence Settings.
        if let cadenceSettings = MeshFrameworkManager.shared.meshClientSensorCadenceGet(deviceName: componentName, propertyId: self.propertyId) {
            if let triggerTypeText = MeshControl.getTriggerTypeText(type: cadenceSettings.triggerType) {
                cadenceStatusTriggerTypeDropDwonButton.setSelection(select: triggerTypeText)
            } else {
                cadenceStatusTriggerTypeDropDwonButton.setSelection(select: 0)
            }

            // settings for Fast Cadence.
            if cadenceSettings.fastCadencePeriodDivisor > 1 {
                UserSettings.shared.isSensorFastCadenceEnabled = true
                cadenceSettingsFastCadenceButton.isSelected = true

                if cadenceSettings.fastCadenceLow == cadenceSettings.fastCadenceHigh {
                    fastCadenceRangeMinValueTextView.text = "\(cadenceSettings.fastCadenceLow)"
                    fastCadenceRangeMaxValueTextView.text = "\(cadenceSettings.fastCadenceHigh)"
                    fastCadenceMeasurementDropListbutton.setSelection(select: UserSettings.shared.sensorFastCadenceMeasureType)
                } else if cadenceSettings.fastCadenceLow > cadenceSettings.fastCadenceHigh {
                    fastCadenceRangeMinValueTextView.text = "\(cadenceSettings.fastCadenceHigh)"
                    fastCadenceRangeMaxValueTextView.text = "\(cadenceSettings.fastCadenceLow)"
                    fastCadenceMeasurementDropListbutton.setSelection(select: MeshControl.MEASUREMENT_TYPE_OUTSIDE)
                    UserSettings.shared.sensorFastCadenceMeasureType = MeshControl.getMeasurementTypeText(type: MeshControl.MEASUREMENT_TYPE_OUTSIDE)!
                } else {
                    fastCadenceRangeMinValueTextView.text = "\(cadenceSettings.fastCadenceLow)"
                    fastCadenceRangeMaxValueTextView.text = "\(cadenceSettings.fastCadenceHigh)"
                    fastCadenceMeasurementDropListbutton.setSelection(select: MeshControl.MEASUREMENT_TYPE_INSIDE)
                    UserSettings.shared.sensorFastCadenceMeasureType = MeshControl.getMeasurementTypeText(type: MeshControl.MEASUREMENT_TYPE_INSIDE)!
                }
            }

            fastCadencePublishPeriodDropListButton.setSelection(select: MeshControl.getFastCadencePeriodDivisorText(divisor: cadenceSettings.fastCadencePeriodDivisor))

            // settings for Triggers.
            if cadenceSettings.triggerDeltaUp != 0 || cadenceSettings.triggerDeltaDown != 0 {
                UserSettings.shared.isSensorTriggerPubEnabled = true
                cadenceSettingsTriggersButton.isSelected = true

                triggerPublishIncreaseUnitTextView.text = "\(cadenceSettings.triggerDeltaUp)"
                triggerPublishDecreaseUnitTextView.text = "\(cadenceSettings.triggerDeltaDown)"
            }
            triggerPublishNoMoreThanSecondsTextView.text = "\(cadenceSettings.minInterval / 1000)"  // // convert by from ms to seconds.
        }

        // initialize sensor property settings.
        sensorPropertyDropListButton.setSelection(select: 0)
    }

    func getPublishToTargets() -> [String] {
        var targets: [String] = []
        if let groups = MeshFrameworkManager.shared.getAllMeshNetworkGroups() {
            for group in groups {
                targets.append(group)
            }
        }
        targets.append(contentsOf: MeshControl.PUBLICATION_TARGETS)
        return targets
    }

    func getSensorSettingProperties() -> [String] {
        var properties: [String] = []
        guard let componentName = self.componentName else {
            return properties
        }

        if let settingPropertyIds = MeshFrameworkManager.shared.meshClientSensorSettingGetPropertyIds(componentName: componentName, propertyId: self.propertyId) {
            for id in settingPropertyIds {
                properties.append(MeshPropertyId.getPropertyIdText(id))
            }
        }
        return properties
    }

    func onTriggerButtonSelected(isSelected: Bool) {
        let color: UIColor = isSelected ? UIColor.black : UIColor.lightGray
        for viewItem in triggerView.subviews {
            if let label = viewItem as? UILabel {
                label.textColor = color
            } else if let textFeild = viewItem as? UITextField {
                textFeild.textColor = color
            }
        }

        triggerPublishNoMoreThanSecondsTextView.isEnabled = isSelected
        triggerPublishIncreaseUnitTextView.isEnabled = isSelected
        triggerPublishDecreaseUnitTextView.isEnabled = isSelected
    }

    func onFastCadenceButtonSelected(isSelected: Bool) {
        let color: UIColor = isSelected ? UIColor.black : UIColor.lightGray
        for viewItem in fastCadenceView.subviews {
            if let label = viewItem as? UILabel {
                label.textColor = color
            } else if let textFeild = viewItem as? UITextField {
                textFeild.textColor = color
            }
        }

        fastCadenceMeasurementDropListbutton.isEnabled = isSelected
        fastCadenceRangeMinValueTextView.isEnabled = isSelected
        fastCadenceRangeMaxValueTextView.isEnabled = isSelected
        fastCadencePublishPeriodDropListButton.isEnabled = isSelected
    }

    func sensorSettingEnable(isEnabled: Bool) {
        let color: UIColor = isEnabled ? UIColor.black : UIColor.lightGray
        for viewItem in SensorSettingsCardView.subviews {
            if let label = viewItem as? UILabel {
                label.textColor = color
            } else if let textFeild = viewItem as? UITextField {
                textFeild.textColor = color
            }
        }

        sensorPropertyDropListButton.isEnabled = isEnabled
        sensorSettingPropertyValueTextView.isEnabled = isEnabled
        sensorSettingSetButton.isEnabled = isEnabled
        SensorSettingsCardView.isHidden = !isEnabled
    }

    func notificationInit() {
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: UIWindow.keyboardWillShowNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: UIWindow.keyboardWillHideNotification, object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED), object: nil)
    }

    @objc func notificationHandler(_ notification: Notification) {
        guard let userInfo = notification.userInfo else {
            return
        }
        switch notification.name {
        case UIWindow.keyboardWillShowNotification:
            adjustingHeightWithKeyboard(show: true, notification: notification)
        case UIWindow.keyboardWillHideNotification:
            adjustingHeightWithKeyboard(show: false, notification: notification)
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
        default:
            break
        }
    }


    @IBAction func onFastCadenceSettingButtonClick(_ sender: UIButton) {
        cadenceSettingsFastCadenceButton.isSelected = !cadenceSettingsFastCadenceButton.isSelected
        onFastCadenceButtonSelected(isSelected: cadenceSettingsFastCadenceButton.isSelected)
    }

    @IBAction func onTriggerSettingButtonClick(_ sender: UIButton) {
        cadenceSettingsTriggersButton.isSelected = !cadenceSettingsTriggersButton.isSelected
        onTriggerButtonSelected(isSelected: cadenceSettingsTriggersButton.isSelected)
    }

    // MARK: - UI operations

    @IBAction func onPublishToDropListButtonClick(_ sender: CustomDropDownButton) {
        publishToDropListButton.showDropList(width: 220, parent: self) {
            meshLog("\(self.publishToDropListButton.selectedIndex), \(self.publishToDropListButton.selectedString)")
        }
    }

    @IBAction func onCadenceStatusTriggerTypeDropDwonButtonClick(_ sender: CustomDropDownButton) {
        cadenceStatusTriggerTypeDropDwonButton.showDropList(width: 150, parent: self) {
            meshLog("onCadenceStatusTriggerTypeDropDwonButtonClick, \(self.cadenceStatusTriggerTypeDropDwonButton.selectedIndex), \(self.cadenceStatusTriggerTypeDropDwonButton.selectedString)")
        }
    }

    @IBAction func onFastCadenceMeasurementDropListbuttonClick(_ sender: CustomDropDownButton) {
        fastCadenceMeasurementDropListbutton.showDropList(width: 150, parent: self) {
            meshLog("fastCadenceMeasurementDropListbutton, \(self.fastCadenceMeasurementDropListbutton.selectedIndex), \(self.fastCadenceMeasurementDropListbutton.selectedString)")
        }
    }

    @IBAction func onFastCadencePublishPeriodDropListButtonClick(_ sender: CustomDropDownButton) {
        fastCadencePublishPeriodDropListButton.showDropList(width: 100, parent: self) {
            meshLog("fastCadencePublishPeriodDropListButton, \(self.fastCadencePublishPeriodDropListButton.selectedIndex), \(self.fastCadencePublishPeriodDropListButton.selectedString)")
        }
    }

    @IBAction func onSensorPropertyDropListButtonClick(_ sender: CustomDropDownButton) {
        sensorPropertyDropListButton.showDropList(width: 220, parent: self) {
            meshLog("sensorPropertyDropListButton, \(self.sensorPropertyDropListButton.selectedIndex), \(self.sensorPropertyDropListButton.selectedString)")
        }
    }

    // MARK: - apply setting data to the remote device

    @IBAction func onPublishSetButtonClick(_ sender: UIButton) {
        meshLog("SensorSettingViewController, onPublishSetButtonClick")
        guard let componentName = self.componentName else {
            meshLog("error: SensorSettingViewController, onPublishSetButtonClick, invalid nil mesh component name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil mesh component name.")
            return
        }
        guard let periodText = publishPeriodTimeTextView.text, !periodText.isEmpty else {
            meshLog("error: SensorSettingViewController, onPublishSetButtonClick, period textField is empty or not set")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Please set the correct publish period time value in milliseconds.")
            return
        }

        let publishTarget = publishToDropListButton.selectedString
        var publishPeriod = 10000
        if let value = Int(periodText), value <= Int32.max {
            publishPeriod = value
        } else {
            meshLog("error: SensorSettingViewController, onPublishSetButtonClick, invalid priod time value or too big, periodText=\(periodText)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid period time value, please set the correct value in milliseconds.")
            return
        }

        meshLog("SensorSettingViewController, onPublishSetButtonClick, input values, publishTarget:\(publishTarget), publishPeriod:\(publishPeriod), groupName=\(String(describing: self.groupName))")
        if let groupName = self.groupName, groupName != publishTarget {
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Are you sure to set publish target to \"\(publishTarget)\" instead of active group \"\(groupName)\"?\nClick OK to continue.\nClick Cancle to change the publish target.",
                title: "Warnning",
                cancelHandler: { (action: UIAlertAction) in return },
                okayHandler: { (action: UIAlertAction) in
                    DispatchQueue.main.async {
                        self.doPublishSet(componentName: componentName, publishTargetName: publishTarget, publishPeriod: publishPeriod)
                    }
                }
            )
        }

        doPublishSet(componentName: componentName, publishTargetName: publishTarget, publishPeriod: publishPeriod)
    }
    func doPublishSet(componentName: String, publishTargetName: String, publishPeriod: Int) {
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorViewController, doPublishSet, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            var error = MeshFrameworkManager.shared.setMeshPublicationConfiguration()
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorSettingViewController, onPublishSetButtonClick, failed to call setMeshPublicationConfiguration API, error:\(error)")
                if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh network is busying, please try again a little later.")
                } else {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to set mesh publication configuration. Error Code: \(error).")
                }
                return
            }

            error = MeshFrameworkManager.shared.configureMeshPublication(componentName: componentName, isClient: false, method: MeshControl.METHOD_SENSOR, targetName: publishTargetName, publishPeriod: publishPeriod)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorSettingViewController, onPublishSetButtonClick, failed to call setMeshPublicationConfiguration API, error:\(error)")
                if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh network is busying, please try again a little later.")
                } else {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to set mesh publication configuration. Error Code: \(error).")
                }
                return
            }

            meshLog("SensorSettingViewController, onPublishSetButtonClick, configureMeshPublication success, publishTarget:\(publishTargetName), publishPeriod:\(publishPeriod)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Publish Set done successfully.", title: "Success")
        }
    }

    @IBAction func onCadenceSetButtonClick(_ sender: UIButton) {
        meshLog("SensorSettingViewController, onCadenceSetButtonClick")
        guard let componentName = self.componentName else {
            meshLog("error: SensorSettingViewController, onCadenceSetButtonClick, invalid nil mesh component name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil mesh component name.")
            return
        }
        guard let minIntervalText = triggerPublishNoMoreThanSecondsTextView.text else {
            meshLog("error: SensorSettingViewController, onCadenceSetButtonClick, triggerPublishNoMoreThanSecondsTextView not set")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Please set minimum interval not less than 1 second(s).")
            return
        }

        let triggerType: Int = MeshControl.getTriggerType(typeText: cadenceStatusTriggerTypeDropDwonButton.selectedString) ?? MeshConstants.SENSOR_TRIGGER_TYPE

        var fastCadencePeriodDivisor: Int = 1
        var fastCadenceLow: Int = 0
        var fastCadenceHigh: Int = 0
        if cadenceSettingsFastCadenceButton.isSelected {
            fastCadencePeriodDivisor = MeshControl.getFastCadencePeriodDivisor(divisorText: fastCadencePublishPeriodDropListButton.selectedString)
            let isInside = (MeshControl.getMeasurementType(typeText: fastCadenceMeasurementDropListbutton.selectedString) ?? MeshControl.MEASUREMENT_TYPE_INSIDE) == MeshControl.MEASUREMENT_TYPE_INSIDE ? true : false
            let newLow = Int(fastCadenceRangeMinValueTextView.text ?? "") ?? MeshConstants.SENSOR_FAST_CADENCE_LOW
            fastCadenceRangeMinValueTextView.text = "\(newLow)"
            let newHigh = Int(fastCadenceRangeMaxValueTextView.text ?? "") ?? MeshConstants.SENSOR_FAST_CADENCE_HIGH
            fastCadenceRangeMinValueTextView.text = "\(newHigh)"
            fastCadenceLow = isInside ? newLow : newHigh
            fastCadenceHigh = isInside ? newHigh : newLow
        }

        var triggerDeltaDown: Int = 0
        var triggerDeltaUp: Int = 0
        var minInterval = MeshConstants.SENSOR_MIN_INTERVAL * 1000
        if cadenceSettingsTriggersButton.isSelected {
            if let value = Int(minIntervalText), value <= (Int32.max / 1000) {
                if value < MeshConstants.SENSOR_MIN_INTERVAL {
                    triggerPublishNoMoreThanSecondsTextView.text = "\(MeshConstants.SENSOR_MIN_INTERVAL)"
                } else {
                    minInterval = value * 1000
                }
            } else {
                // if the input value is invaid or not set, set to default value.
                meshLog("warning: invalid input triggerPublishNoMoreThanSecondsTextView.text:\(String(describing: triggerPublishNoMoreThanSecondsTextView.text)) value, reset to default: \(MeshConstants.SENSOR_MIN_INTERVAL)")
                triggerPublishNoMoreThanSecondsTextView.text = "\(MeshConstants.SENSOR_MIN_INTERVAL)"
            }

            if let value = Int(triggerPublishIncreaseUnitTextView.text ?? ""), value <= Int32.max {
                triggerDeltaUp = value
            } else {
                meshLog("warning: invalid input triggerPublishIncreaseUnitTextView.text:\(String(describing: triggerPublishIncreaseUnitTextView.text)) value, reset to default: 0")
                triggerPublishIncreaseUnitTextView.text = "0"
            }
            if let value = Int(triggerPublishDecreaseUnitTextView.text ?? ""), value <= Int32.max {
                triggerDeltaDown = value
            } else {
                meshLog("warning: invalid input triggerPublishDecreaseUnitTextView.text:\(String(describing: triggerPublishDecreaseUnitTextView.text)) value, reset to default: 0")
                triggerPublishDecreaseUnitTextView.text = "0"
            }
        }

        // store the settings that cannot be reterieved in APIs in app when it executed.
        UserSettings.shared.sensorFastCadenceMeasureType = fastCadenceMeasurementDropListbutton.selectedString
        UserSettings.shared.isSensorFastCadenceEnabled = cadenceSettingsFastCadenceButton.isSelected
        UserSettings.shared.isSensorTriggerPubEnabled = cadenceSettingsTriggersButton.isSelected

        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorViewController, onCadenceSetButtonClick, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            let error = MeshFrameworkManager.shared.meshClientSensorCadenceSet(deviceName: componentName, propertyId: self.propertyId,
                                                                               fastCadencePeriodDivisor: fastCadencePeriodDivisor,
                                                                               triggerType: triggerType,
                                                                               triggerDeltaDown: triggerDeltaDown,
                                                                               triggerDeltaUp: triggerDeltaUp,
                                                                               minInterval: minInterval,
                                                                               fastCadenceLow: fastCadenceLow,
                                                                               fastCadenceHigh: fastCadenceHigh)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorSettingViewController, onCadenceSetButtonClick, failed to call meshClientSensorCadenceSet API, error:\(error)")
                if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh network is busying, please try again a little later.")
                } else {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to set mesh sensor cadence setting values. Error Code: \(error).")
                }
                return
            }

            meshLog("SensorSettingViewController, onCadenceSetButtonClick, call meshClientSensorCadenceSet done success")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Cadence Set done successfully.", title: "Success")
        }
    }

    @IBAction func onSensorSettingSetButtonClick(_ sender: UIButton) {
        meshLog("SensorSettingViewController, onSensorSettingSetButtonClick")
        guard let componentName = self.componentName else {
            meshLog("error: SensorSettingViewController, onSensorSettingSetButtonClick, invalid nil mesh component name")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Invalid nil mesh component name.")
            return
        }
        guard let valueText = sensorSettingPropertyValueTextView.text, !valueText.isEmpty else {
            meshLog("error: SensorSettingViewController, onSensorSettingSetButtonClick, sensor setting value textField is empty or not set")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Please set the property setting value firstly before click the Sensor Setting Set button.")
            return
        }

        let settingPropertyId = MeshPropertyId.getPropertyIdByText(sensorPropertyDropListButton.selectedString)
        let bytes = [UInt8](repeating: 0, count: 2)
        let propertyData = UtilityManager.convertHexDigitStringToData(hexDigit: sensorSettingPropertyValueTextView.text) ?? Data(bytes: bytes, count: 2)
        meshLog("SensorSettingViewController, onSensorSettingSetButtonClick, settingPropertyId:\(String(format: "0x%04x", settingPropertyId)) set values: \(propertyData.dumpHexBytes())")

        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorViewController, onSensorSettingSetButtonClick, failed to connect to mesh network, error:\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Unable to connect to the mesh network. Error Code: \(error).")
                return
            }

            let error = MeshFrameworkManager.shared.meshClientSensorSettingSet(componentName: componentName, propertyId: self.propertyId, settingPropertyId: settingPropertyId, value: propertyData)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SensorSettingViewController, onSensorSettingSetButtonClick, failed to call meshClientSensorSettingSet API, error:\(error)")
                if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Mesh network is busying, please try again a little later.")
                } else {
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to set mesh sensor setting values. Error Code: \(error).")
                }
                return
            }

            meshLog("SensorSettingViewController, onSensorSettingSetButtonClick, call meshClientSensorSettingSet API done success)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Sensor Setting Set done successfully.", title: "Success")
        }
    }

    @IBAction func onLeftTopNagivationBarButtonItemClick(_ sender: UIBarButtonItem) {
        self.dismiss(animated: true, completion: nil)
    }
}

extension SensorSettingViewController: UITextFieldDelegate {
    // used to make sure the input UITextField view won't be covered by the keyboard.
    func adjustingHeightWithKeyboard(show: Bool, notification: Notification) {
        guard let userInfo = notification.userInfo else { return }
        guard let keyboardFrame = (userInfo[UIWindow.keyboardFrameBeginUserInfoKey] as? NSValue)?.cgRectValue else { return }
        let changeInHeight = (keyboardFrame.size.height + 5) * (show ? 1 : -1)
        scrollView.contentInset.bottom += changeInHeight
        scrollView.scrollIndicatorInsets.bottom += changeInHeight
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        // hide the keyboard when click on the screen outside the keyboard.
        self.view.endEditing(true)
    }

    public func textFieldShouldBeginEditing(_ textField: UITextField) -> Bool {
        return true
    }

    public func textFieldDidBeginEditing(_ textField: UITextField) {
    }

    public func textFieldShouldEndEditing(_ textField: UITextField) -> Bool {
        return true
    }

    public func textFieldDidEndEditing(_ textField: UITextField) {
        self.view.endEditing(true)
    }

    public func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        return true
    }

    public func textFieldShouldClear(_ textField: UITextField) -> Bool {
        return true
    }

    public func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        self.view.endEditing(true)
        return true
    }
}
