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
 * Firmware Upgrade View implementation.
 */

import UIKit
import MeshFramework

/**
 * Currently, by default, the Mesh Device Firmware Update (DFU) is disabled (false), becuase this function has been fully suppported yet.
 * When the Mesh DFU is supported and ready, user can enabled it in the MeshApp's setting page.
 */
public var IS_MESH_DFU_ENABLED: Bool {
    get {
        #if MESH_DFU_ENABLED
        return UserDefaults.standard.bool(forKey: "mesh_dfu_enabled_preference")
        #else
        return false
        #endif
    }
}

class FirmwareUpgradeViewController: UIViewController, OtaManagerDelegate {
    @IBOutlet weak var navigationBarItem: UINavigationItem!
    @IBOutlet weak var menuBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var rightBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var firmwareUpgradeTitleLabel: UILabel!
    @IBOutlet weak var firmwareUpgradeMessageLabel: UILabel!
    @IBOutlet weak var discoverredDevicesTableView: UITableView!
    @IBOutlet weak var deviceScanIndicator: UIActivityIndicatorView!

    private var indicatorUpdateTimer: Timer?
    private let tableRefreshControl = UIRefreshControl()

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        viewInit()
    }

    override func viewDidDisappear(_ animated: Bool) {
        if let _ = indicatorUpdateTimer {
            stopScan()
        }
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

    func viewInit() {
        navigationBarItem.rightBarButtonItem = nil  // not used currently.

        if let _ = UserSettings.shared.activeEmail {
            navigationBarItem.title = UserSettings.shared.activeName ?? "@me"
        } else {
            navigationBarItem.title = ""
            navigationBarItem.leftBarButtonItem?.image = UIImage(named: MeshAppImageNames.backIconImage)
        }

        self.tableRefreshControl.tintColor = UIColor.blue
        self.tableRefreshControl.addTarget(self, action: #selector(pullToRefresh), for: .valueChanged)

        DispatchQueue.main.async {
            // Execuated in UI thread to avoid "Main Thread Checker: UI API called on a background thread: -[UIApplication applicationState]" issue.
            self.discoverredDevicesTableView.delegate = self
            self.discoverredDevicesTableView.dataSource = self
            self.discoverredDevicesTableView.separatorStyle = .none

            if #available(iOS 10.0, *) {
                self.discoverredDevicesTableView.refreshControl = self.tableRefreshControl
            } else {
                self.discoverredDevicesTableView.addSubview(self.tableRefreshControl)
            }

            OtaManager.shared.delegate = self
            OtaManager.shared.clearOtaDevices()
            self.pullToRefresh()
        }
    }


    ///
    /// Implementation of OtaManagerDelegate
    ///
    func onOtaDevicesUpdate() {
        discoverredDevicesTableView.reloadData()
    }

    @objc func pullToRefresh() {
        tableRefreshControl.beginRefreshing()
        stopScan()
        OtaManager.shared.clearOtaDevices()
        startScan()
        tableRefreshControl.endRefreshing()
    }

    @objc func indicatorUpdateHandler() {
        if MeshGattClient.shared.centralManager.state == .poweredOn && MeshGattClient.shared.centralManager.isScanning {
            indicatorStartAnimating()
        } else {
            indicatorStopAnimating()
        }
    }

    func indicatorStartAnimating() {
        deviceScanIndicator.startAnimating()
        deviceScanIndicator.isHidden = false
        if indicatorUpdateTimer == nil {
            indicatorUpdateTimer = Timer.scheduledTimer(timeInterval: 1, target: self, selector: #selector(indicatorUpdateHandler), userInfo: nil, repeats: true)
        }
    }

    func indicatorStopAnimating() {
        indicatorUpdateTimer?.invalidate()
        indicatorUpdateTimer = nil
        deviceScanIndicator.stopAnimating()
        deviceScanIndicator.isHidden = true
    }

    func startScan() {
        OtaManager.shared.startScan()
        indicatorStartAnimating()
    }

    func stopScan() {
        indicatorStopAnimating()
        OtaManager.shared.stopScan()
    }

    @IBAction func onMenuBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("FirmwareUpgradeViewController, onMenuBarButtonItemClick")
        if let _ = UserSettings.shared.activeEmail {
            UtilityManager.navigateToViewController(sender: self, targetVCClass: MenuViewController.self, modalPresentationStyle: UIModalPresentationStyle.overCurrentContext)
        } else {
            UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
        }
    }

    @IBAction func onRightBarButtonItemClick(_ sender: UIBarButtonItem) {
        meshLog("FirmwareUpgradeViewController, onRightBarButtonItemClick")
    }
}

extension FirmwareUpgradeViewController: UITableViewDataSource, UITableViewDelegate {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if OtaManager.shared.otaDevices.count == 0 {
            return 1    // show empty table cell
        }
        return OtaManager.shared.otaDevices.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        if OtaManager.shared.otaDevices.count == 0 {
            guard let cell = self.discoverredDevicesTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.FIRMWARE_UPGRADE_EMPTY_CELL, for: indexPath) as? FirmwareUpgradeEmptyTableViewCell else {
                return UITableViewCell()
            }
            return cell
        }

        guard let cell = self.discoverredDevicesTableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.FIRMWARE_UPGRADE_CELL, for: indexPath) as? FirmwareUpgradeTableViewCell else {
            return UITableViewCell()
        }

        let otaDevice = OtaManager.shared.otaDevices[indexPath.row]
        cell.otaDevice = otaDevice
        cell.deviceTypeLabel.text = OtaManager.getOtaDeviceTypeString(by: otaDevice.getDeviceType())
        cell.deviceNameLabel.text = otaDevice.getDeviceName()
        return cell
    }

    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if OtaManager.shared.otaDevices.count == 0 {
            return
        }

        meshLog("FirmwareUpgradeViewController, tableView didSelectRowAt, row=\(indexPath.row)")
        stopScan()
        OtaManager.shared.activeOtaDevice = OtaManager.shared.otaDevices[indexPath.row]
        meshLog("FirmwareUpgradeViewController, IS_MESH_DFU_ENABLED: \(IS_MESH_DFU_ENABLED), is_Mesh_Network_Opened: \(MeshFrameworkManager.shared.getOpenedMeshNetworkName() == nil ? false : true)")
        if IS_MESH_DFU_ENABLED, let _ = MeshFrameworkManager.shared.getOpenedMeshNetworkName() {
            UtilityManager.navigateToViewController(sender: self, targetVCClass: MeshOtaDfuViewController.self)
        } else {
            UtilityManager.navigateToViewController(sender: self, targetVCClass: DeviceOtaUpgradeViewController.self)
        }
    }
}
