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
 * Deleting devices table view cell implementation.
 */

import UIKit
import MeshFramework

class DeletingDevicesTableViewCell: UITableViewCell {
    @IBOutlet weak var deviceTypeIconImage: UIImageView!
    @IBOutlet weak var deviceNameLabel: UILabel!
    @IBOutlet weak var operationStatusView: UIView!
    @IBOutlet weak var deviceStatusMessageLabel: UILabel!
    @IBOutlet weak var deviceSyncingIconButton: UIButton!
    @IBOutlet weak var deviceConnectButton: UIButton!
    @IBOutlet weak var deviceDeleteButton: UIButton!

    var parentVC: UIViewController?

    let messageDeviceIsReachable = "Device is reachable"
    let messageDeviceNotResponsable = "Device not responsable"
    let messageDeviceIsInSyncing = "Device is in syncing"

    var deviceName: String? {
        didSet {
            guard let deviceName = self.deviceName, !deviceName.isEmpty else {
                return
            }
            deviceNameLabel.text = deviceName
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
        meshDeviceStatusInit()

        self.deviceConnectButton.isEnabled = false
        self.deviceConnectButton.isHidden = true
        self.operationStatusView.layer.cornerRadius = self.operationStatusView.frame.size.width/2
        self.operationStatusView.clipsToBounds = true
    }

    func meshDeviceStatusInit() {
        guard let deviceName = self.deviceName else {
            return
        }
        let deviceType = MeshFrameworkManager.shared.getMeshComponentType(componentName: deviceName)
        // TODO(optional): udpate deviceTypeIconImage.image based the device type.
        switch deviceType {
        case MeshConstants.MESH_COMPONENT_LIGHT_HSL:
            break
        case MeshConstants.MESH_COMPONENT_LIGHT_CTL:
            break
        case MeshConstants.MESH_COMPONENT_LIGHT_DIMMABLE:
            break
        case MeshConstants.MESH_COMPONENT_VENDOR_SPECIFIC:
            break
        default:
            break
        }

    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        //super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

    @IBAction func onDeviceSyncingIconButtonClick(_ sender: UIButton) {
        meshLog("DeletingDevicesTableViewCell, onDeviceSyncingIconButtonClick")
        deviceConnectSync()
    }

    @IBAction func onDeviceConnectButtonClick(_ sender: UIButton) {
        meshLog("DeletingDevicesTableViewCell, onDeviceConnectButtonClick")
        deviceConnectSync()
    }

    @IBAction func onDeviceDeleteButtonClick(_ sender: UIButton) {
        meshLog("DeletingDevicesTableViewCell, onDeviceDeleteButtonClick")
        guard let deviceName = self.deviceName else {
            return
        }
        MeshFrameworkManager.shared.meshClientDeleteDevice(deviceName: deviceName) { (networkName: String?, error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("DeletingDevicesTableViewCell, onDeviceDeleteButtonClick, failed to delete the device, error=\(error)")
                return
            }

            meshLog("DeletingDevicesTableViewCell, onDeviceDeleteButtonClick, delete the device success")
        }
    }

    func deviceConnectSync() {
        guard let deviceName = self.deviceName else {
            return
        }

        self.deviceSyncingIconButton.imageView?.startRotate()
        self.operationStatusView.backgroundColor = UIColor.yellow
        self.deviceStatusMessageLabel.text = messageDeviceIsInSyncing
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: DeletingDevicesTableViewCell, deviceConnectSync, failed to connect to mesh network, error=\(error)")
                self.deviceSyncingIconButton.imageView?.stopRotate()
                self.operationStatusView.backgroundColor = UIColor.red
                self.deviceStatusMessageLabel.text = self.messageDeviceNotResponsable

                if let vc = self.parentVC {
                    UtilityManager.showAlertDialogue(parentVC: vc, message: "Unable to connect to the mesh network or device is not responsable. Error Code: \(error)")
                }
                return
            }

            MeshFrameworkManager.shared.meshClientOnOffGet(deviceName: deviceName) { (deviceName: String,  isOn: Bool, isPresent: Bool, remainingTime: UInt32, error: Int) in
                self.deviceSyncingIconButton.imageView?.stopRotate()

                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: DeletingDevicesTableViewCell, deviceConnectSync, meshClientOnOffGet failed, error=\(error)")
                    self.operationStatusView.backgroundColor = UIColor.red
                    self.deviceStatusMessageLabel.text = self.messageDeviceNotResponsable

                    if let vc = self.parentVC {
                        UtilityManager.showAlertDialogue(parentVC: vc, message: "Device is not found ro responsable or encounter error. Error Code: \(error).")
                    }
                    return
                }

                self.operationStatusView.backgroundColor = UIColor.green
                self.deviceStatusMessageLabel.text = self.messageDeviceIsReachable
                meshLog("error: DeletingDevicesTableViewCell, deviceConnectSync, meshClientOnOffGet success")
            }
        }
    }
}
