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
 * Unprovisioned device scanning and management view controller implementation.
 */

import UIKit
import MeshFramework

class UnprovisionedDevicesViewController: UIViewController {
    @IBOutlet weak var navigationBar: UINavigationBar!
    @IBOutlet weak var navigatioinBarItem: UINavigationItem!
    @IBOutlet weak var backBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var settingsBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var titleLable: UILabel!
    @IBOutlet weak var messageLabel: UILabel!
    @IBOutlet weak var unprovisonedDevicesTableView: UITableView!
    @IBOutlet weak var activityIndicator: UIActivityIndicatorView!

    private var indicatorUpdateTimer: Timer?
    private let tableRefreshControl = UIRefreshControl()

    var groupName: String?

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        viewInit()
        restartScan()
    }

    override func viewDidDisappear(_ animated: Bool) {
        stopScan()
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

        NotificationCenter.default.addObserver(self, selector: #selector(self.notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DEVICE_FOUND), object: nil)
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
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DEVICE_FOUND):
            unprovisonedDevicesTableView.reloadData()
        default:
            break
        }
    }

    func viewInit() {
        self.navigatioinBarItem.rightBarButtonItem = nil  // not used currently
        if self.groupName == nil, let groupName = UserSettings.shared.currentActiveGroupName {
            self.groupName = groupName
        }
        self.navigatioinBarItem.title = self.groupName ?? ""

        unprovisonedDevicesTableView.delegate = self
        unprovisonedDevicesTableView.dataSource = self
        unprovisonedDevicesTableView.separatorStyle = .none

        if #available(iOS 10.0, *) {
            self.unprovisonedDevicesTableView.refreshControl = self.tableRefreshControl
        } else {
            self.unprovisonedDevicesTableView.addSubview(self.tableRefreshControl)
        }
        self.tableRefreshControl.tintColor = UIColor.blue
        self.tableRefreshControl.addTarget(self, action: #selector(pullToRefresh), for: .valueChanged)
    }

    @objc func pullToRefresh() {
        self.tableRefreshControl.beginRefreshing()
        MeshGattClient.shared.clearUnprovisionedDeviceList()
        self.unprovisonedDevicesTableView.reloadData()
        self.restartScan()
        self.tableRefreshControl.endRefreshing()
    }

    @objc func indicatorUpdateHandler() {
        if MeshGattClient.shared.centralManager.state == .poweredOn && MeshGattClient.shared.centralManager.isScanning {
            indicatorStartAnimating()
        } else {
            indicatorStopAnimating()
        }
    }

    func indicatorStartAnimating() {
        activityIndicator.startAnimating()
        activityIndicator.isHidden = false
        if indicatorUpdateTimer == nil {
            indicatorUpdateTimer = Timer.scheduledTimer(timeInterval: 1, target: self, selector: #selector(indicatorUpdateHandler), userInfo: nil, repeats: true)
        }
    }

    func indicatorStopAnimating() {
        indicatorUpdateTimer?.invalidate()
        indicatorUpdateTimer = nil
        activityIndicator.stopAnimating()
        activityIndicator.isHidden = true
    }

    func startScan() {
        indicatorStartAnimating()
        MeshGattClient.shared.scanUnprovisionedDeviceStart()
    }

    func stopScan() {
        indicatorStopAnimating()
        MeshGattClient.shared.scanUnprovisionedDeviceStop()
    }

    func restartScan() {
        stopScan()
        startScan()
    }

    @IBAction func onSettingsBarButtonItemClick(_ sender: UIBarButtonItem) {
    }
}

extension UnprovisionedDevicesViewController: UITableViewDataSource, UITableViewDelegate {

    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if MeshGattClient.shared.getUnprovisionDeviceList().count == 0 {
            return 1
        }
        return MeshGattClient.shared.getUnprovisionDeviceList().count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let unprovisionedDevices = MeshGattClient.shared.getUnprovisionDeviceList()
        if unprovisionedDevices.count == 0 {
            // Show empty cell.
            guard let cell = self.unprovisonedDevicesTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.UNPROVISIONED_DEVICES_LIST_EMPTY_CELL, for: indexPath) as? UnprovisionedDeviceListEmptyTableViewCell else {
                return UITableViewCell()
            }
            return cell
        }

        guard let cell = self.unprovisonedDevicesTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.UNPROVISIONED_DEVICES_LIST_CELL, for: indexPath) as? UnprovisionedDeviceListTableViewCell else {
            return UITableViewCell()
        }
        //cell.deviceIconImage.image =
        for (name, uuid) in unprovisionedDevices[indexPath.row] {
            cell.deviceNameLabel.text = name

            cell.parentVC = self
            cell.unprovisionedDeviceName = name
            cell.unprovisionedDeviceUuid = uuid
            cell.groupName = groupName
            break
        }
        let bgColorView = UIView(frame: cell.frame)
        bgColorView.backgroundColor = UIColor.orange
        cell.selectedBackgroundView = bgColorView
        return cell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let unprovisionedDevices = MeshGattClient.shared.getUnprovisionDeviceList()
        if unprovisionedDevices.count == 0 {
            return
        }

        if indexPath.row < unprovisionedDevices.count,
            let deviceName = unprovisionedDevices[indexPath.row].keys.first,
            let uuid = unprovisionedDevices[indexPath.row].values.first,
            let groupName = self.groupName {
            if let provisioningStatusPopoverViewController = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.PROVISIONING_STATUS_POPOVER) as? ProvisioningStatusPopoverViewController {
                // Before starting the provisioning, call the stop scan firstly to avoid any collision caused provision failure issue.
                stopScan()

                provisioningStatusPopoverViewController.parentVc = self
                provisioningStatusPopoverViewController.deviceName = deviceName
                provisioningStatusPopoverViewController.deviceUuid = uuid
                provisioningStatusPopoverViewController.groupName = groupName
                provisioningStatusPopoverViewController.modalPresentationStyle = .custom
                meshLog("UnprovisionedDevicesViewController, tableView didSelectRowAt indexPath:\(indexPath.row), deviceName=\(deviceName), uuid=\(uuid), groupName=\(groupName)")
                self.present(provisioningStatusPopoverViewController, animated: true, completion: nil)
            } else {
                meshLog("error: UnprovisionedDevicesViewController, tableView didSelectRowAt indexPath:\(indexPath.row), failed to initialize ProvisioningStatusPopoverViewController")
            }
        } else {
            // should not happen.
            let deviceName = unprovisionedDevices[indexPath.row].keys.first ?? "nil"
            let uuid = unprovisionedDevices[indexPath.row].values.first?.uuidString ?? "nil"
            let groupName = self.groupName ?? "nil"
            meshLog("error: UnprovisionedDevicesViewController, tableView didSelectRowAt indexPath:\(indexPath.row), deviceName:\(deviceName), uuid:\(uuid), groupName:\(groupName)")
        }
    }
}
