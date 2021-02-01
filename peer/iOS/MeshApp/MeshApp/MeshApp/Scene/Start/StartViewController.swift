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
 * App Start view controller implementation.
 */

import UIKit
import MeshFramework

class StartViewController: UIViewController {
    @IBOutlet weak var statusContentLable: UILabel!

    override func viewDidLoad() {
        super.viewDidLoad()

        // TODO: should find the solution to remove this API call and avoid Main Thread Checker error and blocking.
        OtaManager.shared.waitingForHomeKitPreInit()

        // Do any additional setup after loading the view.
        notificationInit()
        DispatchQueue.main.async { [weak self] in
            if UserSettings.shared.isAccountLoggedIn, let email = UserSettings.shared.activeEmail {
                // Login succcess, continue initializing, using default Mesh Storage path.
                self?.meshInitializing(name: email)
            } else if let email = UserSettings.shared.activeEmail, let password = UserSettings.shared.activePssword {
                self?.statusContentLable.text = "Automatically Logining ..."
                NetworkManager.shared.accountLogin(name: email, password: password) { (status: Int) -> Void in
                    if status != 0 {
                        // Failed to login automatically.
                        UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
                    } else {
                        // Login succcess, continue initializing, using default Mesh Storage path.
                        self?.meshInitializing(name: email)
                    }
                }
            } else {
                // Invalid accout info, failed to login automatically.
                UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
            }
        }
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
        default:
            break
        }
    }

    func meshInitializing(name: String) {
        self.statusContentLable.text = "Initializing Mesh Storage ..."
        var error = MeshFrameworkManager.shared.initMeshStorage(forUser: name)  // use default storage for each user.
        guard error == MeshErrorCode.MESH_SUCCESS, let _ = MeshFrameworkManager.shared.getUserStoragePath() else {
            meshLog("error: StartViewController, meshInitializing, initMeshStorage failed, error=\(error)")
            UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to create mesh stroage directory! Please double check that the mesh storage directory is readable and writable.", title: "Error")
            UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
            return
        }

        // For auto login, the mesh App and account mesh data must be existing already, so do not need to download from network.
        if UserSettings.shared.isAutoLoginning && MeshFrameworkManager.shared.isProvisionerUuidFileExits() {
            meshLog("StartViewController, meshInitializing, provisionerName=\(UserSettings.shared.provisionerName), isProvisionerUuidFileExits=\(MeshFrameworkManager.shared.isMeshNetworkConnected())")
            self.statusContentLable.text = "Initializing Mesh Library ..."
            error = MeshFrameworkManager.shared.initMeshLibrary(forUser: name)
            if error != MeshErrorCode.MESH_SUCCESS {
                meshLog("error: StartViewController, meshInitializing, initMeshLibrary failed, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to initialize the Mesh Library! Please try to login again later.", title: "Error")
                UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
                return
            }

            // Jump to Mesh Network List scene.
            UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
            return
        }

        // Try to download user's mesh networks data from network, and store them to user's current mesh storage path.
        self.statusContentLable.text = "Downloading to your Networks ..."
        NetworkManager.shared.restoreMeshFiles() { (status) in
            if status != 0 {
                meshLog("error: StartViewController, meshInitializing, initMeshLibrary failed, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to download your mesh data! Please double check the newtork is accessable, then try to login again", title: "Error")
                UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
                return
            }

            meshLog("StartViewController, meshInitializing, provisionerName=\(UserSettings.shared.provisionerName), isProvisionerUuidFileExits=\(MeshFrameworkManager.shared.isMeshNetworkConnected())")
            self.statusContentLable.text = "Initializing Mesh Library ..."
            error = MeshFrameworkManager.shared.initMeshLibrary(forUser: name)
            if error != MeshErrorCode.MESH_SUCCESS {
                meshLog("error: StartViewController, meshInitializing, initMeshLibrary failed, error=\(error)")
                UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to initialize the Mesh Library! Please try to login again later.", title: "Error")
                UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
                return
            }

            // Jump to Mesh Network List scene.
            UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

}
