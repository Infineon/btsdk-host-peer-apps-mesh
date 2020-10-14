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
 * Network List table view cell implementation.
 */

import UIKit
import MeshFramework

class NetworkListTableViewCell: UITableViewCell {
    @IBOutlet weak var meshNetworkNameLabel: UILabel!
    @IBOutlet weak var networkIconImage: UIImageView!
    @IBOutlet weak var networkMessageLabel: UILabel!
    @IBOutlet weak var networkOpenStatusSwitch: UISwitch!

    private let indicatorView = CustomIndicatorView()

    var networkName: String? {
        didSet {
            meshNetworkNameLabel.text = networkName
            updateNetworkName()
        }
    }

    func updateNetworkName() {
        if let cellNetworkName = networkName {
            if let networkName = MeshFrameworkManager.shared.getOpenedMeshNetworkName(), networkName == cellNetworkName {
                networkOpenStatusSwitch.setOn(true, animated: true)
            } else {
                networkOpenStatusSwitch.setOn(false, animated: true)
            }
            meshLog("NetworkListTableViewCell, mesh network: \(cellNetworkName) is  \(networkOpenStatusSwitch.isOn ? "opened" : "closed")")
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
        networkOpenStatusSwitch.setOn(false, animated: false)
        updateNetworkName()
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }
    @IBAction func onNetworkOpenStatusSwitchClick(_ sender: UISwitch) {
        meshLog("NetworkListTableViewCell, onNetworkOpenStatusSwitchClick, switch button is \(networkOpenStatusSwitch.isOn ? "ON" : "OFF")")
        // Note, this switch is not interactive UI controller, it's just to display the status of mesh network is opened or not.
    }
}
