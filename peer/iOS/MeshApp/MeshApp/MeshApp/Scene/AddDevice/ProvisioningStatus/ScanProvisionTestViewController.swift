/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
//
//  ScanProvisionTestViewController.swift
//  MeshApp
//
//  Created by Dudley Du on 2019/11/6.
//

import UIKit
import MeshFramework

class ScanProvisionTestViewController: UIViewController {
    public static let shared = ScanProvisionTestViewController()

    @IBOutlet weak var topNavigationItem: UINavigationItem!
    @IBOutlet weak var leftBarBttonItem: UIBarButtonItem!
    @IBOutlet weak var rightBarBttonItem: UIBarButtonItem!
    @IBOutlet weak var activityIndicator: UIActivityIndicatorView!
    @IBOutlet weak var unprovisionedDeviceNameLabel: UILabel!
    @IBOutlet weak var deviceUUIDLabel: UILabel!
    @IBOutlet weak var provisionedDeviceNameLabel: UILabel!
    @IBOutlet weak var logContentTextView: UITextView!
    @IBOutlet weak var stopTestBtn: UIButton!
    @IBOutlet weak var startTestBtn: UIButton!
    @IBOutlet weak var PassTimesLabel: UILabel!

    private var vcOpenedTime = Date(timeIntervalSinceNow: 0)
    private var isScanProvisionTestEnabled: Bool = false
    private var provisionCount: Int = 0
    private var scanProvisionTestStage: Int = 0
    private var testTimer: Timer?
    private var provisionStartTime: TimeInterval = 0

    public static var unprovisionedDeviceName: String?
    public static var deviceUuid: UUID?
    public static var provisionGroupName: String?
    public static var provisionedDeviceName: String?

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        topNavigationItem.rightBarButtonItem = nil
        stopActivityIndicator()
        logContentTextView.text = ""
        logContentTextView.layer.borderWidth = 1
        logContentTextView.layer.borderColor = UIColor.gray.cgColor
        logContentTextView.isEditable = false
        logContentTextView.isSelectable = false
        logContentTextView.layoutManager.allowsNonContiguousLayout = false

        stopTestBtn.isEnabled = false
        startTestBtn.isEnabled = true
        isScanProvisionTestEnabled = false
        provisionCount = 0
        PassTimesLabel.text = "Pass Times: \(provisionCount)"
        PassTimesLabel.backgroundColor = UIColor.white

        ScanProvisionTestViewController.provisionedDeviceName = nil
        guard let testUnprovisionedDeviceName = ScanProvisionTestViewController.unprovisionedDeviceName, let testUnprovisionedDeviceUuid = ScanProvisionTestViewController.deviceUuid,
            let groupName = ScanProvisionTestViewController.provisionGroupName else {
            if ScanProvisionTestViewController.unprovisionedDeviceName == nil {
                log("invalid unprovisioned device name: nil, stop testing")
            } else {
                log("invalid unprovisioned device UUID: nil, stop testing")
            }
            unprovisionedDeviceNameLabel.text = "Device Name: " +  (ScanProvisionTestViewController.unprovisionedDeviceName ?? "Unknown Device Name")
            deviceUUIDLabel.text = "Device UUID: " + (ScanProvisionTestViewController.deviceUuid?.uuidString ?? "")
            return
        }
        log("Scan/provision test with device name: \(testUnprovisionedDeviceName), uuid: \(testUnprovisionedDeviceUuid.uuidString), group name: \(groupName)")
        unprovisionedDeviceNameLabel.text = "Device Name: " + testUnprovisionedDeviceName
        deviceUUIDLabel.text = "Device UUID: " + testUnprovisionedDeviceUuid.uuidString

        notificationInit()
    }

    override func viewDidDisappear(_ animated: Bool) {
        NotificationCenter.default.removeObserver(self)
        ScanProvisionTestViewController.unprovisionedDeviceName = nil
        ScanProvisionTestViewController.deviceUuid = nil
        ScanProvisionTestViewController.provisionedDeviceName = nil
        scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
        super.viewDidDisappear(animated)
    }

    func notificationInit() {
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NODE_CONNECTION_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_NETWORK_LINK_STATUS_CHANGED), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_NETWORK_DATABASE_CHANGED), object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_CLIENT_DEVICE_FOUND), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(notificationHandler(_:)),
                                               name: Notification.Name(rawValue: MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STATUS), object: nil)
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
            if scanProvisionTestStage != MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_SCANNING {
                return
            }

            if let foundUnprovisionedDevice = MeshNotificationConstants.getFoundMeshDevice(userInfo: userInfo) {
                log("discovered device UUID=\(foundUnprovisionedDevice.uuid.uuidString), name=\(foundUnprovisionedDevice.name)")
                if foundUnprovisionedDevice.uuid == ScanProvisionTestViewController.deviceUuid {
                    DispatchQueue.main.async {
                        self.stopScan()
                        if self.isScanProvisionTestEnabled {
                            self.log("found target testing unprovisioned device: \(foundUnprovisionedDevice.name)")
                            self.doProvisionTest()
                        } else {
                            self.log("scan/provision test has been disabled")
                            self.onStopTestBtnClick(self.stopTestBtn)
                        }
                    }
                }
            }
        case Notification.Name(rawValue: MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STATUS):
            guard let testStatus = MeshNotificationConstants.getScanProvisionTestStatus(userInfo: userInfo) else {
                return
            }
            if testStatus.stage != scanProvisionTestStage {
                log("scan/provision stage status miss-matched, expected: \(scanProvisionTestStage), real: \(testStatus.stage)")
                scanProvisionTestStage = testStatus.stage
            }

            // start next test step
            switch testStatus.stage {
            case MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_PROVISIONING:
                if testStatus.status == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_STATUS_NETWORK_BUSY {
                    log("warning: mesh network is busying, waiting 10 seconds to try provisioning again. status: \(testStatus.status)")
                    DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(10)) {
                        self.doProvisionTest()
                    }
                    return
                }

                guard testStatus.status == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_STATUS_SUCCESS else {
                    log("error: received invalid provisoin completed status: \(testStatus.status) from mesh library")
                    log("provision failed at \(provisionCount + 1) times testing")
                    DispatchQueue.main.async {
                        self.onStopTestBtnClick(self.stopTestBtn)
                    }
                    return
                }

                ScanProvisionTestViewController.provisionedDeviceName = MeshFrameworkManager.shared.getMeshComponentsByDevice(uuid: ScanProvisionTestViewController.deviceUuid!)?.first
                guard let provisionedName = ScanProvisionTestViewController.provisionedDeviceName else {
                    log("error, provision completed, but unable to read provisionedName")
                    log("provision failed at \(provisionCount + 1) times testing")
                    DispatchQueue.main.async {
                        self.onStopTestBtnClick(self.stopTestBtn)
                    }
                    return
                }

                log("provision finished success")
                provisionedDeviceNameLabel.text = "Provisioned Name: " + provisionedName

                // This test check is aimed to capture the provision process takes longer time than normal symptom.
                // Note, the test failure on this DOSE NOT means the provisioning failure.
                // When failed on this test, the scan/provision still working fine and completed on success.
                let provisionFinishedTime = Date().timeIntervalSince(vcOpenedTime)
                let spendTime = provisionFinishedTime - self.provisionStartTime
                log("provision completed spent time: \(String(format: "%.6f", spendTime)) seconds")
                guard true, (spendTime < 30) else {
                    log("error: provision finished during time exceed 30 seconds, spendTime=\(String(format: "%.6f", spendTime))")
                    log("provision failed at \(provisionCount + 1) times testing")
                    DispatchQueue.main.async {
                        self.onStopTestBtnClick(self.stopTestBtn)
                    }
                    return
                }

                // try send identify command to the device to make sure the device added successfully.
                // then delete the provisioned device.
                DispatchQueue.main.async {
                    self.deviceIdentify(deviceName: provisionedName)
                }
                break
            case MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_SCANNING:
                guard testStatus.status == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_STATUS_SUCCESS else {
                    log("error: scanning failed")
                    DispatchQueue.main.async {
                        self.onStopTestBtnClick(self.stopTestBtn)
                    }
                    return
                }
                log("scanning finished success")
                break
            case MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_DELETING:
                guard testStatus.status == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_STATUS_SUCCESS else {
                    log("error: device delete/reset failed")
                    DispatchQueue.main.async {
                        self.onStopTestBtnClick(self.stopTestBtn)
                    }
                    return
                }

                ScanProvisionTestViewController.provisionedDeviceName = nil
                provisionedDeviceNameLabel.text = "Provisioned Name: "
                log("device delete/reset finished success")
                break
            default:
                break
            }
        default:
            break
        }
    }

    func log(_ message: String) {
        meshLog(message)
        let seconds = Date().timeIntervalSince(vcOpenedTime)
        let msg = String(format: "[%.3f] \(message)\n", seconds)
        logContentTextView.text += msg
        let bottom = NSRange(location: logContentTextView.text.count, length: 1)
        logContentTextView.scrollRangeToVisible(bottom)
    }

    func _doProvisionTest() {
        // The provisoning operation must wait until the mesh network is idle, otherwise will encounter invalid state immediately.
        // When there are multiple devices exsiting in the mesh network, especically there are some devices that unreachable,
        // there are much more time required for the mesh provisioner to update the keys after a mesh device deleted/changed.
        guard !MeshFrameworkManager.shared.isMeshClientProvisionKeyRefreshing() else {
            stopTestBtn.isEnabled = true
            log("warning: mesh network is busying, waiting 5 senconds for it ready for idle")
            DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(5)) {
                if !self.isScanProvisionTestEnabled {
                    self.log("scan/provision test has been disable")
                    self.scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
                    self.onStopTestBtnClick(self.stopTestBtn)
                    return
                }
                self._doProvisionTest()
            }
            return
        }

        guard let deviceName = ScanProvisionTestViewController.unprovisionedDeviceName,
            let deviceUuid = ScanProvisionTestViewController.deviceUuid,
            let groupName = ScanProvisionTestViewController.provisionGroupName else {
                log("error, doProvisionTest, invalid provisioning data, device name: \(ScanProvisionTestViewController.unprovisionedDeviceName ?? "nil"), uuid: \(ScanProvisionTestViewController.deviceUuid?.uuidString ?? "nil"), group name: \(ScanProvisionTestViewController.provisionGroupName ?? "nil")")
                log("provision failed at \(provisionCount + 1) times testing")
                onStopTestBtnClick(stopTestBtn)
            return
        }

        guard let provisioningStatusPopoverViewController = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: MeshAppStoryBoardIdentifires.PROVISIONING_STATUS_POPOVER) as? ProvisioningStatusPopoverViewController else {
            log("error, doProvisionTest, failed to initialize ProvisioningStatusPopoverViewController instance")
            log("provision failed at \(provisionCount + 1) times testing")
            onStopTestBtnClick(stopTestBtn)
            return
        }

        if self.isScanProvisionTestEnabled {
            provisioningStatusPopoverViewController.deviceName = deviceName
            provisioningStatusPopoverViewController.deviceUuid = deviceUuid
            provisioningStatusPopoverViewController.groupName = groupName
            provisioningStatusPopoverViewController.modalPresentationStyle = .custom
            provisioningStatusPopoverViewController.isScanProvisionTestEnabled = true
            self.provisionStartTime = Date().timeIntervalSince(vcOpenedTime)
            self.present(provisioningStatusPopoverViewController, animated: true, completion: nil)
        } else {
            log("scan/provision test has been disable")
            scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
            onStopTestBtnClick(stopTestBtn)
        }
    }

    func doProvisionTest() {
        log("start provision test ...")
        scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_PROVISIONING

        _doProvisionTest()
    }

    func doDeleteTest(provisionedDeviceName: String) {
        log("do device delete/reset test")
        scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_DELETING
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.log("error: delete device, failed to connect to the mesh network, error: \(error)")
                self.onStopTestBtnClick(self.stopTestBtn)
                return
            }

            MeshFrameworkManager.shared.meshClientDeleteDevice(deviceName: provisionedDeviceName) { (networkName: String?, error: Int) in
                guard error == MeshErrorCode.MESH_SUCCESS else {
                    self.log("error: delete device/reset, failed to remove the device, error: \(error)")
                    self.onStopTestBtnClick(self.stopTestBtn)
                    return
                }

                self.log("device delete/reset finished success")
                ScanProvisionTestViewController.provisionedDeviceName = nil
                self.provisionedDeviceNameLabel.text = "Provisioned Name: "

                // restart scanning
                if self.isScanProvisionTestEnabled {
                    self.scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_SCANNING
                    self.startScan()
                } else {
                    self.log("scan/provision test has been disable")
                    self.scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
                    self.onStopTestBtnClick(self.stopTestBtn)
                }
            }
        }
    }

    func deviceIdentify(deviceName: String) {
        log("do device identify test")
        MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.log("error: device identify test, failed to connect to the mesh network, error: \(error)")
                self.onStopTestBtnClick(self.stopTestBtn)
                return
            }

            let error = MeshFrameworkManager.shared.meshClientIdentify(name: deviceName, duration: 2)
            guard error == MeshErrorCode.MESH_SUCCESS else {
                self.log("error: device identify test, failed to send identify command, error: \(error)")
                self.onStopTestBtnClick(self.stopTestBtn)
                return
            }

            self.log("device identify test, send identify command success")
            self.provisionCount += 1
            self.PassTimesLabel.text = "Pass Times: \(self.provisionCount)"

            // now delete the device for next test loop.
            if self.isScanProvisionTestEnabled {
                self.doDeleteTest(provisionedDeviceName: deviceName)
            } else {
                self.log("scan/provision test has been disable")
                self.scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
                self.onStopTestBtnClick(self.stopTestBtn)
            }
        }
    }

    @objc private func testTimerHandler() {
        if scanProvisionTestStage == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_SCANNING {
            if !MeshGattClient.shared.centralManager.isScanning {
                MeshGattClient.shared.scanUnprovisionedDeviceStart()
            }
        }
    }

    func startScan() {
        log("start scanning")
        MeshGattClient.shared.scanUnprovisionedDeviceStart()
        testTimer?.invalidate()
        testTimer = Timer.scheduledTimer(timeInterval: TimeInterval(10),
                                         target: self,
                                         selector: #selector(testTimerHandler),
                                         userInfo: nil,
                                         repeats: true)
    }

    func stopScan() {
        log("stop scanning")
        testTimer?.invalidate()
        testTimer = nil
        MeshGattClient.shared.scanUnprovisionedDeviceStop()
    }

    func startActivityIndicator() {
        activityIndicator.startAnimating()
        activityIndicator.isHidden = false
    }

    func stopActivityIndicator() {
        activityIndicator.stopAnimating()
        activityIndicator.isHidden = true
    }

    @IBAction func onClearBtnClick(_ sender: UIButton) {
        logContentTextView.text = ""
    }

    @IBAction func onStopTestBtnClick(_ sender: UIButton) {
        stopScan()
        stopActivityIndicator()
        isScanProvisionTestEnabled = false
        stopTestBtn.isEnabled = false
        startTestBtn.isEnabled = true
        if scanProvisionTestStage == MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE {
            PassTimesLabel.backgroundColor = UIColor.green
            log("stop scan/provision test")
        } else {
            PassTimesLabel.backgroundColor = UIColor.red
            log("stop scan/provision test failed")
        }
        log("passed scan/provision test count: \(provisionCount)\n")
    }

    @IBAction func onStartTestBtnClick(_ sender: UIButton) {
        startActivityIndicator()
        stopTestBtn.isEnabled = true
        startTestBtn.isEnabled = false
        isScanProvisionTestEnabled = true

        log("\n\n")
        log("start scan/provision test")
        provisionCount = 0
        PassTimesLabel.text = "Pass Times: \(provisionCount)"
        scanProvisionTestStage = MeshNotificationConstants.MESH_SCAN_PROVISION_TEST_STAGE_IDLE
        PassTimesLabel.backgroundColor = UIColor.white
        DispatchQueue.main.async {
            self.stopScan()
            self.doProvisionTest()
        }
    }

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
        if let identifier = segue.identifier {
            meshLog("ComponentViewController, segue.identifier=\(identifier)")
            switch identifier {
            case MeshAppStoryBoardIdentifires.SEGUE_SCAN_PROVISION_BACK:
                onStopTestBtnClick(stopTestBtn)
                self.dismiss(animated: false, completion: nil)
            default:
                break
            }
        }
    }
}
