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
 * Device List table view cell implementation.
 */

import UIKit
import MeshFramework

class DeviceListTableViewCell: UITableViewCell {
    @IBOutlet weak var deviceIconImage: UIImageView!
    @IBOutlet weak var deviceNameLabel: UILabel!
    @IBOutlet weak var controlsButton: UIButton!
    @IBOutlet weak var turnOnButton: UIButton!
    @IBOutlet weak var turnOffButton: UIButton!

    var groupName: String?
    var deviceName: String?
    var deviceType: Int = MeshConstants.MESH_COMPONENT_UNKNOWN
    var isOn: Bool = false {
        didSet {
            switch deviceType {
            case MeshConstants.MESH_COMPONENT_GENERIC_ON_OFF_CLIENT:
                fallthrough
            case MeshConstants.MESH_COMPONENT_GENERIC_ON_OFF_SERVER:
                // TODO[optional]: use the special images for ON/OFF devices if have.
                deviceIconImage.image = UIImage(named: (isOn ? MeshAppImageNames.lightOnImage : MeshAppImageNames.lightOffImage))
            case MeshConstants.MESH_COMPONENT_GENERIC_LEVEL_CLIENT:
                fallthrough
            case MeshConstants.MESH_COMPONENT_GENERIC_LEVEL_SERVER:
                // TODO[optional]: use the special images for level devices if have.
                deviceIconImage.image = UIImage(named: (isOn ? MeshAppImageNames.lightOnImage : MeshAppImageNames.lightOffImage))
            case MeshConstants.MESH_COMPONENT_LIGHT_HSL:
                // TODO[optional]: use the special images for HSL light devices if have.
                deviceIconImage.image = UIImage(named: (isOn ? MeshAppImageNames.lightOnImage : MeshAppImageNames.lightOffImage))
            case MeshConstants.MESH_COMPONENT_LIGHT_CTL:
                // TODO[optional]: use the special images for CTL light devices if have.
                deviceIconImage.image = UIImage(named: (isOn ? MeshAppImageNames.lightOnImage : MeshAppImageNames.lightOffImage))
            case MeshConstants.MESH_COMPONENT_LIGHT_DIMMABLE:
                // TODO[optional]: use the special images for lightness light devices if have.
                deviceIconImage.image = UIImage(named: (isOn ? MeshAppImageNames.lightOnImage : MeshAppImageNames.lightOffImage))
            case MeshConstants.MESH_COMPONENT_SENSOR_SERVER:
                fallthrough
            case MeshConstants.MESH_COMPONENT_SENSOR_CLIENT:
                // TODO[optional]: use the special images for sensor devices if have.
                deviceIconImage.image = UIImage(named: MeshAppImageNames.sensorImage)
            default:
                // TODO[optional]: use the special images for unknown type devices if have, such as vendor specific data set demo device.
                deviceIconImage.image = UIImage(named: MeshAppImageNames.sensorImage)
                break
            }
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        //super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

    @IBAction func onControlButtonClick(_ sender: UIButton) {
        meshLog("DeviceListTableViewCell, onControlButtonClick")
        if let name = self.deviceNameLabel.text, name.count > 0 {
            UserSettings.shared.currentActiveGroupName = groupName
            UserSettings.shared.currentActiveComponentName = deviceName
            UtilityManager.navigateToViewController(targetClass: ComponentViewController.self)
        }
    }

    @IBAction func onTurnOnButtonClick(_ sender: UIButton) {
        meshLog("DeviceListTableViewCell, onTurnOnButtonClick, deviceName=\(String(describing: deviceName))")
        turnDeviceOnOff(deviceName: deviceName, isOn: true)
    }

    @IBAction func onTurnOffButtonClick(_ sender: UIButton) {
        meshLog("DeviceListTableViewCell, onTurnOffButtonClick, deviceName=\(String(describing: deviceName))")
        turnDeviceOnOff(deviceName: deviceName, isOn: false)
    }

    /**
     Turn the remote device ON or OFF.

     @param deviceName      Name of the target mesh device to turn ON or OFF.
     @param isON            Indicate the operation of turn ON (turn) or turn OFF (false).
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
                            For mesh device, it's set to true by default.

     @return                None.

     Note, when the reliable set to true, the MeshNotificationConstants.MESH_CLIENT_ON_OFF_STATUS event will be sent to GroupDetailsDeviceListViewController instance,
     so, the device status will be updated and shown in the UI.
     */
    func turnDeviceOnOff(deviceName: String?, isOn: Bool, reliable: Bool = true) {
        if let deviceName = deviceName {
            MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error) in
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: DeviceListTableViewCell, turnDeviceOnOff(deviceName:\(deviceName), isOn:\(isOn)), failed to connect to the mesh network")
                    return
                }

                let error = MeshFrameworkManager.shared.meshClientOnOffSet(deviceName: deviceName, isOn: isOn, reliable: reliable)
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: DeviceListTableViewCell, meshClientOnOffSet(deviceName:\(deviceName), isOn:\(isOn), reliable=\(reliable)) failed, error=\(error)")
                    return
                }
                meshLog("DeviceListTableViewCell, meshClientOnOffSet(deviceName:\(deviceName), isOn:\(isOn), reliable=\(reliable)) message sent out success")
            }
        }
    }
}
