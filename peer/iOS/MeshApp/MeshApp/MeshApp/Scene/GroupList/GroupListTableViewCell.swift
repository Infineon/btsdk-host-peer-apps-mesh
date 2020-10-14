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
 * Group List table view cell implementation.
 */

import UIKit
import MeshFramework

class GroupListTableViewCell: UITableViewCell {
    @IBOutlet weak var groupNameLabel: UILabel!
    @IBOutlet weak var groupIconImage: UIImageView!
    @IBOutlet weak var groupTurnOnButton: UIButton!
    @IBOutlet weak var groupTurnOffButton: UIButton!
    @IBOutlet weak var showGroupAllDevicesButton: UIButton!
    @IBOutlet weak var showGroupControlsButton: UIButton!

    var groupName: String? {
        didSet {
            if groupNameLabel != nil {
                groupNameLabel.text = groupName ?? "Unknown"
            }
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

    @IBAction func onGroupTurnOnButtonClick(_ sender: UIButton) {
        meshLog("GroupListTableViewCell, onGroupTurnOnButtonClick, TURN ON all devices within mesh group: \(String(describing: groupName))")
        turnGroupDevicesOnOff(groupName: groupName, isOn: true)
    }

    @IBAction func onGroupTurnOffButtonClick(_ sender: UIButton) {
        meshLog("GroupListTableViewCell, onGroupTurnOnButtonClick, TURN OFF all devices within mesh group: \(String(describing: groupName))")
        turnGroupDevicesOnOff(groupName: groupName, isOn: false)
    }

    @IBAction func onShowGroupAllDevicesButtonClick(_ sender: UIButton) {
        meshLog("GroupListTableViewCell, onShowGroupAllDevicesButtonClick, group name: \(String(describing: groupName))")
        if let groupName = groupName {
            UserSettings.shared.currentActiveGroupName = groupName
            // do nothing, done through segue, navigate to the group detail scene and set defalut with all group devices item enabled.
        }
    }

    @IBAction func onShowGroupControlsButtonClick(_ sender: UIButton) {
        meshLog("GroupListTableViewCell, onShowGroupControlsButtonClick, group name: \(String(describing: groupName))")
        if let groupName = groupName {
            UserSettings.shared.currentActiveGroupName = groupName
            // do nothing, done through segue, navigate to the group detail scene and set defalut with group controls item enabled.
        }
    }

    /**
     Turn the remote devices ON or OFF which subscribed by the specific group.

     @param deviceName      Name of the mesh group where all the devices should be turned ON or OFF.
     @param isON            Indicate the operation of turn ON (turn) or turn OFF (false).
     @param reliable        Indicates to send using the Acknowledged (true) or the Unacknowledged (false) message.
                            For mesh group, it's set to false by default.

     @return                None.

     Note, when the reliable set to true, all the compenents which have its On/Off callback routine invoked with its status update value.
     */
    func turnGroupDevicesOnOff(groupName: String?, isOn: Bool, reliable: Bool = false) {
        if let groupName = groupName {
            MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error) in
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: GroupListTableViewCell, turnGroupDevicesOnOff(groupName:\(groupName), isOn:\(isOn)), failed to connect to the mesh network")
                    return
                }

                let error = MeshFrameworkManager.shared.meshClientOnOffSet(deviceName: groupName, isOn: isOn, reliable: reliable)
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    meshLog("error: GroupListTableViewCell, meshClientOnOffSet(groupName:\(groupName), isOn:\(isOn), reliable=\(reliable)) failed, error=\(error)")
                    return
                }
                meshLog("GroupListTableViewCell, meshClientOnOffSet(groupName:\(groupName), isOn:\(isOn), reliable=\(reliable)) message sent out success")
            }
        }
    }
}
