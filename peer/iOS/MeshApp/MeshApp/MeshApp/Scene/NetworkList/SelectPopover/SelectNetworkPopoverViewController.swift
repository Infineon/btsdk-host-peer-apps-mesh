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
 * Custom show select network popover view controller implementation.
 */

import UIKit
import MeshFramework

enum SelectNetworkPopoverAction: String {
    case deleteNetwork = "Delete Network"
    case exportNetwork = "Export Network"
    case importNetwork = "Import Network"
}

class SelectNetworkPopoverViewController: UIViewController {
    @IBOutlet weak var popoverView: UIView!
    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var networkTableView: UITableView!
    @IBOutlet weak var cancelButton: UIButton!
    @IBOutlet weak var confirmButton: UIButton!

    static var popoverAction: SelectNetworkPopoverAction = .deleteNetwork

    private var networkNames: [String] = []
    private var radioButtons: [Int: CustomRadioButton] = [:]

    private let indicatorView = CustomIndicatorView()

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        networkTableView.dataSource = self
        networkTableView.delegate = self
        networkTableView.separatorStyle = .none
        resetSelectedNetworkName()

        switch SelectNetworkPopoverViewController.popoverAction {
        case .deleteNetwork:
            networkNames = UserSettings.shared.meshNetworks
            titleLabel.text = SelectNetworkPopoverAction.deleteNetwork.rawValue
        case .exportNetwork:
            networkNames = UserSettings.shared.meshNetworks
            titleLabel.text = SelectNetworkPopoverAction.exportNetwork.rawValue
        case .importNetwork:
            if let exportedNetworks = MeshFrameworkManager.shared.meshNetworksUnderExportedMeshStorage {
                networkNames = exportedNetworks
            }
            titleLabel.text = SelectNetworkPopoverAction.importNetwork.rawValue
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

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    func resetSelectedNetworkName() {
        radioButtons = [:]
        SelectNetworkPopoverTableViewCell.selectedCell = nil
    }

    func getSelectedNetworkName() -> String? {
        guard let selectedButton = SelectNetworkPopoverTableViewCell.selectedCell,
            radioButtons.count > 0, networkNames.count >= radioButtons.count else {
            return nil
        }

        for (index, button) in radioButtons {
            if selectedButton.isEqual(button) {
                return networkNames[index]
            }
        }
        return nil
    }

    @IBAction func onCancelButtonClick(_ sender: UIButton) {
        meshLog("SelectNetworkPopoverViewController, onCancelButtonClick")
        resetSelectedNetworkName()
        UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
    }

    @IBAction func onConfirmButtonClick(_ sender: UIButton) {
        if let networkName = getSelectedNetworkName() {
            switch SelectNetworkPopoverViewController.popoverAction {
            case .deleteNetwork:
                onDeleteNetworkConfirm(provisioinerName: UserSettings.shared.provisionerName, networkName: networkName)
            case .exportNetwork:
                onExportNetworkConfirm(provisioinerName: UserSettings.shared.provisionerName, networkName: networkName)
            case .importNetwork:
                onImportNetworkConfirm(provisioinerName: UserSettings.shared.provisionerName, networkName: networkName)
            }
            return
        } else {
            meshLog("warnning: SelectNetworkPopoverViewController, onConfirmButtonClick, no network selected, networkName=nil")
        }
        UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self)
    }

    func onDeleteNetworkConfirm(provisioinerName: String, networkName: String) {
        indicatorView.showAnimating(parentView: self.view)
        MeshFrameworkManager.shared.closeMeshNetwork()
        let error = MeshFrameworkManager.shared.deleteMeshNetwork(provisioinerName: provisioinerName, networkName: networkName)
        indicatorView.stopAnimating()
        guard error == MeshErrorCode.MESH_SUCCESS else {
            meshLog("SelectNetworkPopoverViewController, onConfirmButtonClick, failed to deleteMeshNetwork, name=\(networkName)")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Failed to delete mesh network: \"\(networkName)\". Error Code: \(error)",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
            )
            return
        }

        UserSettings.shared.meshNetworks.removeAll(where: {$0 == networkName})
        meshLog("SelectNetworkPopoverViewController, onConfirmButtonClick, deleteMeshNetwork, name=\(networkName), success")
        UtilityManager.showAlertDialogue(parentVC: self,
                                         message: "Mesh network: \"\(networkName)\" has been deleted.",
                                         title: "Success",
                                         action: UIAlertAction(title: "OK", style: .default,
                                                               handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
        )
    }

    func onExportNetworkConfirm(provisioinerName: String, networkName: String) {
        indicatorView.showAnimating(parentView: self.view)
        guard let exportedJsonString = MeshFrameworkManager.shared.meshClientNetworkExport(networkName: networkName) else {
            indicatorView.stopAnimating()
            meshLog("SelectNetworkPopoverViewController, onConfirmButtonClick, failed to do meshClientNetworkExport, name=\(networkName)")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Failed to export mesh network: \"\(networkName)\" to \("filepath")",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
                                            )
            return
        }

        meshLog("SelectNetworkPopoverViewController, onConfirmButtonClick, meshClientNetworkExport, name=\(networkName), success")
        let error = MeshFrameworkManager.shared.writeExportedMeshNetwork(networkName: networkName, jsonContent: exportedJsonString)
        guard let exportedIfxJsonString = MeshFrameworkManager.shared.meshClientNetworkExport(networkName: networkName + ".ifx") else {
            indicatorView.stopAnimating()
            return
        }
        MeshFrameworkManager.shared.writeExportedMeshNetwork(networkName: networkName + ".ifx", jsonContent: exportedIfxJsonString)
        indicatorView.stopAnimating()
        if error == MeshErrorCode.MESH_SUCCESS {
            meshLog("SelectNetworkPopoverViewController, writeExportedMeshNetwork success")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Mesh network: \"\(networkName)\" has been exported and written to path: \(MeshFrameworkManager.shared.defaultExportStoragePath ?? MeshFrameworkManager.shared.defaultExportStorageFolderName).",
                                             title: "Success",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
                                            )
        } else {
            meshLog("error: SelectNetworkPopoverViewController, writeExportedMeshNetwork failed, write path: \(MeshFrameworkManager.shared.defaultExportStoragePath ?? MeshFrameworkManager.shared.defaultExportStorageFolderName)")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Failed to write the export Mesh network: \"\(networkName)\" to path: \(MeshFrameworkManager.shared.defaultExportStoragePath ?? MeshFrameworkManager.shared.defaultExportStorageFolderName).",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
                                            )
        }
    }

    func onImportNetworkConfirm(provisioinerName: String, networkName: String) {
        indicatorView.showAnimating(parentView: self.view)
        meshLog("SelectNetworkPopoverViewController, onImportNetworkConfirm, meshNetworkName=\(networkName)")
        guard let importJsonString = MeshFrameworkManager.shared.readExportedMeshNetwork(networkName: networkName) else {
            indicatorView.stopAnimating()
            meshLog("error: SelectNetworkPopoverViewController, onImportNetworkConfirm, failed to read \(networkName).json file")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Failed to read the import Mesh network file: \"\(networkName).json\".",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
                                            )
            return
        }

        meshLog("SelectNetworkPopoverViewController, onImportNetworkConfirm, importJsonString=\(importJsonString)")
        let realNetworkName = MeshFrameworkManager.shared.meshClientNetworkImport(provisioinerName: provisioinerName, jsonString: importJsonString) { (_ networkName: String?, _ status: Int, _ error: Int) in
            self.indicatorView.stopAnimating()
            guard error == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SelectNetworkPopoverViewController, onImportNetworkConfirm, failed to call meshClientNetworkImport, error=\(error)")
                return
            }
            guard status == MeshErrorCode.MESH_SUCCESS else {
                meshLog("error: SelectNetworkPopoverViewController, onImportNetworkConfirm, open mesh network failed, status=\(status)")
                return
            }

            meshLog("SelectNetworkPopoverViewController, onImportNetworkConfirm, mesh network: \(String(describing: networkName)) openned success")
        }

        guard let parsedNetworkName = realNetworkName else {
            indicatorView.stopAnimating()
            meshLog("error: SelectNetworkPopoverViewController, failed to import the network from the input \(networkName).json file")
            UtilityManager.showAlertDialogue(parentVC: self,
                                             message: "Failed to import Mesh network from file: \"\(networkName).json\".",
                                             action: UIAlertAction(title: "OK", style: .default,
                                                                   handler: { (action) in UtilityManager.navigateToViewController(targetClass: NetworkListViewController.self) })
                                            )
            return
        }

        meshLog("SelectNetworkPopoverViewController, onImportNetworkConfirm, openning mesh network: \(parsedNetworkName) from input \(networkName).json file")
        UserSettings.shared.currentActiveMeshNetworkName = parsedNetworkName
        UserSettings.shared.isCurrentActiveMeshNetworkOpenned = false
        UtilityManager.navigateToViewController(sender: self, targetVCClass: TransitionViewController.self)
    }
}

extension SelectNetworkPopoverViewController: UITableViewDataSource, UITableViewDelegate {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return networkNames.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let cell = self.networkTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.NETWORK_LIST_SELECT_POPOVER_CELL, for: indexPath) as? SelectNetworkPopoverTableViewCell else {
            return UITableViewCell()
        }
        cell.radioButton.setTitle(networkNames[indexPath.row], for: .normal)
        radioButtons[indexPath.row] = cell.radioButton
        return cell
    }
}
