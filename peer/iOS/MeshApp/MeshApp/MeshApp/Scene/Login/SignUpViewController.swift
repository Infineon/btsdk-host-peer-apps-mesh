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

/** @file
 *
 * Sign up view controller implementation.
 */

import UIKit
import MeshFramework

class SignUpViewController: UIViewController {
    @IBOutlet weak var errorInfoView: UIView!
    @IBOutlet weak var errorInfoTextFiled: UILabel!
    @IBOutlet weak var accountEmailTextField: UITextField!
    @IBOutlet weak var accountPasswordTextField: UITextField!
    @IBOutlet weak var accoutnReenterPasswordTextField: UITextField!
    @IBOutlet weak var signUpButton: UIButton!
    @IBOutlet weak var createNewAccoutDetailView: UIView!

    let activeIndcator = CustomIndicatorView()

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        notificationInit()
        errorInfoView.isHidden = true
        accountEmailTextField.delegate = self
        accountPasswordTextField.delegate = self
        accoutnReenterPasswordTextField.delegate = self
        let tapBackGround = UITapGestureRecognizer(target: self, action: #selector(onTapBackGround))
        self.view.addGestureRecognizer(tapBackGround)
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

    @objc func onTapBackGround() {
        self.view.endEditing(true)
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    func validateInputAccount() -> Bool {
        if accountEmailTextField.text == nil || accountEmailTextField.text!.isEmpty ||
            !UtilityManager.validateEmail(email: accountEmailTextField.text) {
            errorInfoTextFiled.text = "Invalid Email Address"
            errorInfoView.isHidden = false
            return false
        }

        if accountPasswordTextField.text == nil || accountPasswordTextField.text!.isEmpty ||
            accoutnReenterPasswordTextField.text == nil || accountPasswordTextField.text != accoutnReenterPasswordTextField.text {
            errorInfoTextFiled.text = "Password Not Match"
            errorInfoView.isHidden = false
            return false
        }

        return true
    }

    @IBAction func onSignUpButtonUp(_ sender: Any) {
        guard validateInputAccount(), let email = accountEmailTextField.text, let password = accountPasswordTextField.text  else {
            return
        }

        activeIndcator.showAnimating(parentView: self.view, isTransparent: true)
        DispatchQueue.main.async { [weak self] in
            NetworkManager.shared.accountRegister(name: email, password: password) { (status: Int) -> Void in
                self?.activeIndcator.stopAnimating()

                if status != 0 {
                    meshLog("onSignUpButtonUp, NetworkManager.shared.accountRegister status=\(status)")
                    self?.errorInfoTextFiled.text = "Invalid Account"
                    self?.errorInfoView.isHidden = false
                    return
                }

                UserSettings.shared.lastActiveAccountEmail = email
                UtilityManager.navigateToViewController(targetClass: LoginViewController.self)
            }
        }
    }
}

extension SignUpViewController: UITextFieldDelegate {
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        if textField == accountEmailTextField {
            _ = accountPasswordTextField.becomeFirstResponder()
        } else if textField == accountPasswordTextField {
            _ = accoutnReenterPasswordTextField.becomeFirstResponder()
        } else if textField == accoutnReenterPasswordTextField {
            textField.resignFirstResponder()
            onSignUpButtonUp(signUpButton as Any)
        }
        return false
    }

    func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        if !errorInfoView.isHidden {
            errorInfoView.isHidden = true
        }
        return true
    }

    /// MARK: avoid the TextField to be hidden by the keyboard when inputing.
    func textFieldDidBeginEditing(_ textField: UITextField) {
        createNewAccoutDetailView.frame = CGRect(x: createNewAccoutDetailView.frame.origin.x,
                                                 y: createNewAccoutDetailView.frame.origin.y - 100,
                                                 width: createNewAccoutDetailView.frame.size.width,
                                                 height: createNewAccoutDetailView.frame.size.height)
    }

    func textFieldDidEndEditing(_ textField: UITextField) {
        createNewAccoutDetailView.frame = CGRect(x: createNewAccoutDetailView.frame.origin.x,
                                                 y: createNewAccoutDetailView.frame.origin.y + 100,
                                                 width: createNewAccoutDetailView.frame.size.width,
                                                 height: createNewAccoutDetailView.frame.size.height)
    }
}
