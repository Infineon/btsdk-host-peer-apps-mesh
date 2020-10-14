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
 * Cusotmer menu view controller implementation.
 */

import UIKit
import MeshFramework

enum MenuItems: Int {
    case Title = 0
    case NetworkStatus = 1
    case MyNetworks = 2
    case MyGroups = 3
    case FirmwareUpgrade = 4
    case DeletingDevices = 5
}

class MenuViewController: UIViewController {
    @IBOutlet weak var backgroundView: UIView!
    @IBOutlet weak var menuTableView: UITableView!
    @IBOutlet weak var contentView: UIView!
    @IBOutlet weak var menuTableViewTrailingLayoutConstraint: NSLayoutConstraint!

    private var networkStatusCell: MenuNetworkStatusTableViewCell?

    var menuItems: [MenuItems] = [MenuItems.Title,
                                  MenuItems.NetworkStatus,
                                  MenuItems.MyNetworks,
                                  MenuItems.MyGroups,
                                  MenuItems.FirmwareUpgrade,
                                  //MenuItems.DeletingDevices,  // [TODO] not required.
    ]

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        viewInit()
        notificationInit()
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
                if let cell = networkStatusCell {
                    cell.onNetworkLinkStatusChanged()
                }
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED):
            if let networkName = MeshNotificationConstants.getNetworkName(userInfo: userInfo) {
                self.showToast(message: "Database of mesh network \(networkName) has changed.")
            }
        default:
            break
        }
    }

    func viewInit() {
        networkStatusCell = nil
        menuTableViewTrailingLayoutConstraint.constant = self.view.bounds.size.width * 0.3

        menuTableView.dataSource = self
        menuTableView.delegate = self
        menuTableView.separatorStyle = .none

        // These items only vaild when the mesh network has been opened.
        if MeshFrameworkManager.shared.getOpenedMeshNetworkName() == nil {
            if menuItems.contains(MenuItems.DeletingDevices) {
                menuItems.remove(at: MenuItems.DeletingDevices.rawValue)
            }
            if menuItems.contains(MenuItems.MyGroups) {
                menuItems.remove(at: MenuItems.MyGroups.rawValue)
            }
        }
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if let touch = touches.first {
            let location: CGPoint = touch.location(in: backgroundView)
            if location.x > menuTableView.frame.size.width {
                meshLog("MenuViewController, clicked the gray area, dismiss menu UI")
                // TODO: dismiss the menu UI when clicked the gray area.
                //self.navigationController?.popViewController(animated: true)
                self.dismiss(animated: true, completion: nil)
            }
        }
    }
}

extension MenuViewController: UITableViewDataSource, UITableViewDelegate {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return menuItems.count    // This value must be udpated based on real UI desgin.
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        var newCell: UITableViewCell?

        let item = menuItems[indexPath.row]
        switch item {
        case .Title:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_TITLE_CELL, for: indexPath) as? MenuTitleTableViewCell {
                cell.parentVC = self
                cell.accountEmailLabel.text = UserSettings.shared.activeEmail
                cell.accountNameLabel.text = UserSettings.shared.activeName
                newCell = cell
            }

        case .NetworkStatus:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_NETWORK_STATUS_CELL, for: indexPath) as? MenuNetworkStatusTableViewCell {
                cell.parentVC = self
                newCell = cell
                networkStatusCell = cell
            }

        case .MyNetworks:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_MY_NETWORKS_CELL, for: indexPath) as? MenuMyNetworkTableViewCell {
                cell.parentVC = self
                newCell = cell
            }
        case .MyGroups:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_MY_GROUPS_CELL, for: indexPath) as? MenuMyGroupsTableViewCell {
                cell.parentVC = self
                newCell = cell
            }
        case .FirmwareUpgrade:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_FIRMWARE_UPGRADE_CELL, for: indexPath) as? MenuFirmwareUpgradeTableViewCell {
                cell.parentVC = self
                newCell = cell
            }
        case .DeletingDevices:
            if let cell = menuTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.MENU_DELETING_DEVICES_CELL, for: indexPath) as? MenuDeletingDevicesTableViewCell {
                cell.parentVC = self
                newCell = cell
            }
        }

        guard let dequeuedCell = newCell else {
            return UITableViewCell()
        }
        return dequeuedCell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        meshLog("MenuViewController, tableView didSelectRowAt: \(indexPath.row)")

        let item = menuItems[indexPath.row]
        switch item {
        case .Title:
            UtilityManager.navigateToViewController(targetClass: ProfileSettingsViewController.self)
        case .NetworkStatus:
            break
        case .MyNetworks:
            UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
        case .MyGroups:
            if UserSettings.shared.isCurrentActiveMeshNetworkOpenned, let _ = UserSettings.shared.currentActiveMeshNetworkName {
                UtilityManager.navigateToViewController(targetClass: GroupListViewController.self)
            } else {
                // Must create or select and open a netowrk firstly before access the group list.
                UtilityManager.showAlertDialogue(parentVC: self,
                                                 message: "Please create or open a mesh network firstly before searching mesh groups.",
                                                 title: "Warning",
                                                 completion: nil,
                                                 action: UIAlertAction(title: "OK", style: .default,
                                                                       handler: { (action) in
                                                                        UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
                                                 }))
            }
        case .FirmwareUpgrade:
            UtilityManager.navigateToViewController(targetClass: FirmwareUpgradeViewController.self)
        case .DeletingDevices:
            if UserSettings.shared.isCurrentActiveMeshNetworkOpenned, let _ = UserSettings.shared.currentActiveMeshNetworkName {
                UtilityManager.navigateToViewController(targetClass: DeletingDevicesViewController.self)
            } else {
                UtilityManager.showAlertDialogue(parentVC: self,
                                                 message: "Please select and open a mesh network firstly before listing any mesh devices in deleting.",
                                                 title: "Warning",
                                                 completion: nil,
                                                 action: UIAlertAction(title: "OK", style: .default,
                                                                       handler: { (action) in
                                                                        UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
                                                 }))
            }
        }
    }
}
