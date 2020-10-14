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
 * Network Status table cell for Menu Scene implementation.
 */

import UIKit
import MeshFramework

class MenuNetworkStatusTableViewCell: UITableViewCell {
    @IBOutlet weak var networkStatusTitleLabel: UILabel!
    @IBOutlet weak var networkStatusLightView: UIView!
    @IBOutlet weak var networkStatusMessageLabel: UILabel!
    @IBOutlet weak var networkStatusReflashButton: UIButton!
    @IBOutlet weak var segmentationLineView: UIView!

    var parentVC: UIViewController?

    let networkConnectedMsg = "Network is connected"
    let networkDisconnectedMsg = "Network is disconnected"
    let networkConnectingMsg = "Searching for network"

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
        self.networkStatusLightView.layer.cornerRadius = self.networkStatusLightView.frame.size.width/2
        self.networkStatusLightView.clipsToBounds = true

        if MeshFrameworkManager.shared.isMeshNetworkConnected() {
            self.networkStatusLightView.backgroundColor = UIColor.green
            self.networkStatusMessageLabel.text = networkConnectedMsg
        } else {
            self.networkStatusLightView.backgroundColor = UIColor.red
            self.networkStatusMessageLabel.text = networkDisconnectedMsg
        }
    }

    func onNetworkLinkStatusChanged() {
        if MeshFrameworkManager.shared.isMeshNetworkConnected() {
            self.networkStatusLightView.backgroundColor = UIColor.green
            self.networkStatusMessageLabel.text = self.networkConnectedMsg
        } else {
            self.networkStatusLightView.backgroundColor = UIColor.red
            self.networkStatusMessageLabel.text = self.networkDisconnectedMsg
        }
        self.networkStatusReflashButton.imageView?.stopRotate()
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        //super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
        if self.networkStatusReflashButton.imageView?.isRotating ?? true {
            return  // busying already.
        }

        if selected {
            if let vc = self.parentVC, !UserSettings.shared.isCurrentActiveMeshNetworkOpenned {
                UtilityManager.showAlertDialogue(parentVC: vc, message: "Please select and open a mesh network before searching for the network.")
                return
            }

            connectingToMeshNetwork()
        }
    }

    @IBAction func onNetworkStatusReflashButtonClick(_ sender: UIButton) {
        meshLog("MenuNetworkStatusTableViewCell, onNetworkStatusReflashButtonClick")
        connectingToMeshNetwork()
    }

    func connectingToMeshNetwork() {
        guard let _ = MeshFrameworkManager.shared.getOpenedMeshNetworkName() else {
            if let vc = self.parentVC {
                UtilityManager.showAlertDialogue(parentVC: vc, message: "No mesh network opened, please select and open a mesh network firstly.")
            }
            return
        }

        if MeshFrameworkManager.shared.isMeshNetworkConnected() {
            self.networkStatusLightView.backgroundColor = UIColor.green
            self.networkStatusMessageLabel.text = networkConnectedMsg
            networkStatusReflashButton.imageView?.stopRotate()
        } else {
            self.networkStatusLightView.backgroundColor = UIColor.yellow
            self.networkStatusMessageLabel.text = networkConnectingMsg
            networkStatusReflashButton.imageView?.startRotate()
            MeshFrameworkManager.shared.connectMeshNetwork { (isConnected: Bool, connId: Int, addr: Int, isOverGatt: Bool, error: Int) in
                meshLog("MenuNetworkStatusTableViewCell, connectingToMeshNetwork completion, isConnected:\(isConnected), connId:\(connId), addr:\(addr), isOverGatt:\(isOverGatt), error:\(error)")
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    if let vc = self.parentVC {
                        var message = "Failed to connect to mesh network. Error Code: \(error)."
                        if error == MeshErrorCode.MESH_ERROR_INVALID_STATE {
                            message = "Mesh network is busying, please try again a litte later."
                        }
                        self.onNetworkLinkStatusChanged()
                        UtilityManager.showAlertDialogue(parentVC: vc, message: message)
                    }
                    return
                }
                self.onNetworkLinkStatusChanged()
            }
        }
    }
}

extension UIImageView {
    func startRotate() {
        let rotationAnimation = CABasicAnimation(keyPath: "transform.rotation.z")
        rotationAnimation.toValue = NSNumber(value: Double.pi * 2)
        rotationAnimation.duration = 1
        rotationAnimation.isCumulative = true
        rotationAnimation.repeatCount = MAXFLOAT
        layer.add(rotationAnimation, forKey: "rotationAnimation")
    }

    func stopRotate() {
        layer.removeAllAnimations()
    }

    var isRotating: Bool {
        if let _ = layer.animation(forKey: "rotationAnimation") {
            return true
        }
        return false
    }
}
