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
 * Unprovisioned device list cell implementation.
 */

import UIKit

class UnprovisionedDeviceListTableViewCell: UITableViewCell {
    @IBOutlet weak var deviceIconImage: UIImageView!
    @IBOutlet weak var deviceNameLabel: UILabel!
    @IBOutlet weak var addSymbolLabel: UILabel!
    @IBOutlet weak var scanProvisionTestBtn: UIButton!

    var parentVC: UnprovisionedDevicesViewController?
    var unprovisionedDeviceName: String?
    var unprovisionedDeviceUuid: UUID?
    var groupName: String?

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

    @IBAction func onScanProvisionTestBtnClick(_ sender: UIButton) {
        if let pVc = parentVC, let targetName = unprovisionedDeviceName, let targetUuid = unprovisionedDeviceUuid, let targetGroupName = groupName {
            DispatchQueue.main.async {
                ScanProvisionTestViewController.unprovisionedDeviceName = targetName
                ScanProvisionTestViewController.deviceUuid = targetUuid
                ScanProvisionTestViewController.provisionGroupName = targetGroupName
                UtilityManager.navigateToViewController(sender: pVc, targetVCClass: ScanProvisionTestViewController.self)
            }
        } else {
            if let vc = parentVC {
                UtilityManager.showAlertDialogue(parentVC: vc, message: "Invalid test device info, name: \(unprovisionedDeviceName ?? "nil"), uuid: \(unprovisionedDeviceUuid?.uuidString ?? "nil"), groupName: \(groupName ?? "nil")")
            }
        }
    }
}
