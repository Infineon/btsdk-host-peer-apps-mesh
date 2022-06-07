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
 * Group List view controller implementation.
 */

import UIKit
import MeshFramework

struct GroupListCellData {
    var components: [String]
    var groupName: String
}

class GroupListViewController: UIViewController {
    @IBOutlet weak var topNavigationBarItem: UINavigationItem!
    @IBOutlet weak var menuBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var moreSettingsButton: UIBarButtonItem!
    @IBOutlet weak var myGroupsLabel: UILabel!
    @IBOutlet weak var addGroupButton: CustomRoundedRectButton!
    @IBOutlet weak var groupsTableView: UITableView!

    private var meshGropusData: [GroupListCellData] = []
    private var selectedGroupDetailsVC: GroupDetailsViewController?

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        meshGroupsInit()
        notificationInit()
        viewInit()
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
                meshGroupsInit()
                updateGroupCount()
                groupsTableView.reloadData()
            }
        default:
            break
        }
    }

    func updateGroupCount() {
        myGroupsLabel.text = (meshGropusData.count > 0) ? "My Groups (\(meshGropusData.count))" : "My Groups"
    }

    func viewInit() {
        topNavigationBarItem.rightBarButtonItem = nil
        topNavigationBarItem.title = UserSettings.shared.currentActiveMeshNetworkName
        updateGroupCount()
        groupsTableView.dataSource = self
        groupsTableView.delegate = self
        groupsTableView.separatorStyle = .none
    }

    func meshGroupsInit() {
        meshGropusData.removeAll()
        guard let networkName = MeshFrameworkManager.shared.getOpenedMeshNetworkName() else {
            meshLog("error: GroupListViewController, meshGroupsInit, UserSettings.shared.currentActiveMeshNetworkName is nil")
            return
        }

        var allGroups = MeshFrameworkManager.shared.getAllMeshNetworkGroups(networkName: networkName) ?? []
        if allGroups.count == 0 {
            // Create default home primary group which cannot be deleted, when remove a group, the default operation is to move
            // all devices in that group to the default home primary group.
            let error = MeshFrameworkManager.shared.createMeshGroup(groupName: MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME)
            allGroups = MeshFrameworkManager.shared.getAllMeshNetworkGroups(networkName: networkName) ?? []
            if error != MeshErrorCode.MESH_SUCCESS || allGroups.count == 0 {
                meshLog("error: GroupListViewController, meshGroupsInit, network name:\(networkName) failed to create default group:\(MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME)")
                return
            }
            meshLog("GroupListViewController, meshGroupsInit, network name:\(networkName), create default group:\(MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME) success")
        }
        meshLog("GroupListViewController, meshGroupsInit, network name:\(networkName), allGroups=\(allGroups)")

        // Add non-default group names.
        var defaultHomeGroupName: String?
        for groupName in allGroups {
            if groupName == MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME {
                defaultHomeGroupName = groupName
                continue
            } else if groupName.hasPrefix("dfu_") {
                continue    // do not show the mesh library generated dfu_xxxx groups.
            }
            let components = MeshFrameworkManager.shared.getMeshGroupComponents(groupName: groupName) ?? []
            meshGropusData.append(GroupListCellData(components: components, groupName: groupName))
        }

        /*
         * TODO: this feature can be open or disabled based requirement.
         * [Dudley]: Always put the default home primary group name as the latest item in the table view
         *           or hidden it if do not want to how it, here for demo perpose, we show the detaul group name at last.
         *           And only when there is any component device in default group or other groups exist, the default will be shown.
         */
        let hideDefaultAllFixedGroup = false     // set to false to show "All(Fixed)" group in the group list view.
        if let groupName = defaultHomeGroupName, !hideDefaultAllFixedGroup {
            let components = MeshFrameworkManager.shared.getlAllMeshGroupComponents(groupName: groupName) ?? []
            if components.count > 0 || meshGropusData.count > 0 {
                meshGropusData.append(GroupListCellData(components: components, groupName: groupName))
            }
        }
    }

    @IBAction func onMenuBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("GroupListViewController, onMenuBarButtonItemClick")
        UtilityManager.navigateToViewController(sender: self, targetVCClass: MenuViewController.self, modalPresentationStyle: UIModalPresentationStyle.overCurrentContext)
    }

    @IBAction func onMoreSettingsButtonClick(_ sender: UIBarButtonItem) {
        meshLog("GroupListViewController, onMoreSettingsButtonClick")
    }

    @IBAction func onAddGroupButtonClick(_ sender: CustomRoundedRectButton) {
        meshLog("GroupListViewController, onAddGroupButtonClick")
        let alertController = UIAlertController(title: "Add a Group", message: nil, preferredStyle: .alert)
        alertController.addTextField { (textField: UITextField) in
            textField.placeholder = "Enter the Group Name"
        }
        alertController.addAction(UIAlertAction(title: "Cancel", style: .default, handler: nil))
        alertController.addAction(UIAlertAction(title: "Confirm", style: .default, handler: { (action: UIAlertAction) -> Void in
            if let textField = alertController.textFields?.first, let newGroupName = textField.text, newGroupName.count > 0 {
                // Always create new groups as subgroup of the defualt group, so can control all devices in the different groups in on parent group.
                let error = MeshFrameworkManager.shared.createMeshGroup(groupName: newGroupName, parentGroupName: MeshAppConstants.MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME)
                if error != MeshErrorCode.MESH_SUCCESS {
                    meshLog("GroupListViewController, failed to createMeshGroup with name=\"\(newGroupName)\"")
                    UtilityManager.showAlertDialogue(parentVC: self, message: "Failed to Create Group with name: \"\(newGroupName)\". Error Code: \(error)")
                } else {
                    meshLog("GroupListViewController, createMeshGroup with name=\"\(newGroupName)\" success")
                    self.meshGroupsInit()
                    self.updateGroupCount()
                    self.groupsTableView.reloadData()
                }
            } else {
                UtilityManager.showAlertDialogue(parentVC: self, message: "Empty or Invalid Group Name!", title: "Error")
            }
        }))
        self.present(alertController, animated: true, completion: nil)
    }

    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
        if let identifier = segue.identifier {
            switch identifier {
            case MeshAppStoryBoardIdentifires.SEGUE_GROUP_CELL_SELECT:
                if let groupDetailVC = segue.destination as? GroupDetailsViewController {
                    groupDetailVC.currentSegmentedSelectedIndex = MeshAppConstants.GROUPT_DETAIL_SEGMENTED_DEFAULT
                    selectedGroupDetailsVC = groupDetailVC
                }
            case MeshAppStoryBoardIdentifires.SEGUE_GROUP_CELL_ALL_DEVICES:
                if let groupDetailVC = segue.destination as? GroupDetailsViewController {
                    groupDetailVC.currentSegmentedSelectedIndex = MeshAppConstants.GROUPT_DETAIL_SEGMENTED_ALL_DEVICE
                    selectedGroupDetailsVC = groupDetailVC
                }
            case MeshAppStoryBoardIdentifires.SEGUE_GROUP_CELL_GROUP_CONTROL:
                if let groupDetailVC = segue.destination as? GroupDetailsViewController {
                    groupDetailVC.currentSegmentedSelectedIndex = MeshAppConstants.GROUPT_DETAIL_SEGMENTED_CONTROL
                    selectedGroupDetailsVC = groupDetailVC
                }
            default:
                break
            }
        }
    }
}

extension GroupListViewController: UITableViewDataSource, UITableViewDelegate {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if meshGropusData.count == 0 {
            return 1
        }
        return meshGropusData.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        if meshGropusData.count == 0 {
            // Show empty cell.
            guard let cell = self.groupsTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.GROUP_LIST_EMPTY_CELL, for: indexPath) as? GroupListEmptyTableViewCell else {
                return UITableViewCell()
            }
            cell.emptyMessage.text = "Your group seems empty! Try adding a group."
            return cell
        }

        guard let cell = self.groupsTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.GROUP_LIST_CELL, for: indexPath) as? GroupListTableViewCell else {
            return UITableViewCell()
        }
        cell.groupName = meshGropusData[indexPath.row].groupName
        cell.showGroupAllDevicesButton.setTitle("All Devices (\(meshGropusData[indexPath.row].components.count))", for: .normal)
        return cell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if meshGropusData.count == 0 {
            return
        }

        if indexPath.row < meshGropusData.count {
            UserSettings.shared.currentActiveGroupName = meshGropusData[indexPath.row].groupName
            if let vc = selectedGroupDetailsVC {
                vc.groupName = UserSettings.shared.currentActiveGroupName
            }
            // Do nothing, segue navigate to the group detail scene and set defalut with group controls item enabled.
        }
    }
}
