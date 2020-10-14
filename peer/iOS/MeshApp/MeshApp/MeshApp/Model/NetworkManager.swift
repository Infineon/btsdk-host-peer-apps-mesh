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
 * Network storage and access simulation implementation.
 */

import Foundation
import MeshFramework

class NetworkManager {
    static let shared = NetworkManager()

    /// MARK: placeholder functions of network operations.

    /**
     Upload and store the specific file into network for current loggin user.

     @param fileName    The file name should be uploaded and stored in network space for current user.
     @param content     The content of the file should be uploaded and stored.

     @return            None
     */
    func uploadMeshFile(fileName: String, content: Data, completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            UserSettings.shared.setMeshFile(fileName: fileName, content: content)
            completion(status)
        }
    }

    func downloadMeshFile(fileName: String, completion: @escaping (Int, Data?) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            let content = UserSettings.shared.getMeshFile(fileName: fileName) as? Data
            completion(status, content)
        }
    }

    func deleteMeshFile(fileName: String, completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            UserSettings.shared.deleteMeshFile(fileName: fileName)
            completion(status)
        }
    }

    func uploadMeshFiles(completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            if let meshFiles = MeshFrameworkManager.shared.getUserMeshFileNameList(), meshFiles.count > 0 {
                for fileName in meshFiles {
                    if let content = MeshFrameworkManager.shared.readMeshFile(fileName: fileName) {
                        self.uploadMeshFile(fileName: fileName, content: content) { (status) in
                            // TODO: do something after netwokr mesh files updated if required.
                        }
                    }
                }
            }
            completion(status)
        }
    }

    func restoreMeshFiles(completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            if let meshFiles = UserSettings.shared.getMeshFilesList(), meshFiles.count > 0 {
                for fileName in meshFiles {
                    self.downloadMeshFile(fileName: fileName) { (status, content) in
                        _ = MeshFrameworkManager.shared.restoreMeshFile(fileName: fileName, content: content)
                    }
                }
            }
            completion(status)
        }
    }

    func deleteMeshFiles(completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            let status: Int = 0
            if let meshFiles = MeshFrameworkManager.shared.getUserMeshFileNameList(), meshFiles.count > 0 {
                for fileName in meshFiles {
                    self.deleteMeshFile(fileName: fileName, completion: { (status) in
                        // TODO: do something after netwokr mesh file delete if required.
                    })
                }
            }
            completion(status)
        }
    }

    /// MARK: regiser and store the new account information into network.
    ///       return 0 on success; 1 duplicated name failed; 2 unknown register failed.
    func accountRegister(name: String, password: String, completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            var status: Int = 0

            guard name.count > 0, password.count > 0 else {
                completion(1)   // invalid name or password.
                return
            }

            if UserSettings.shared.accounts[name] != nil {
                status = 1      // the same name has been registerred.
            } else {
                UserSettings.shared.accounts[name] = password
            }
            completion(status)
        }
    }

    /// MARK: authentication the account name and password through network.
    ///       return 0 on success; 1 account specified by the name not exists; 2 account name and password authentication failed.
    func accountLogin(name: String, password: String, completion: @escaping (Int) -> Void) {
        DispatchQueue.main.async {
            var status: Int = 0

            if name.count > 0, password.count > 0, let storedPwd = UserSettings.shared.accounts[name] {
                if storedPwd != password {
                    status = 2
                }
            } else {
                status = 1
            }
            completion(status)
        }
    }
}
