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
 * User settings and App permanent storage implementation.
 */

import Foundation
import MeshFramework

struct UserSettingsConstants {
    static let KEY_IS_ACCOUNT_ACTIVE = "key_is_account_active"
    static let KEY_IS_ACCOUNT_LOGGED_IN = "key_is_account_logged_in"
    static let KEY_ACTIVE_ACCOUNT_NAME = "key_active_account_name"
    static let KEY_ACTIVE_ACCOUNT_EMAIL = "key_active_account_email"
    static let KEY_ACTIVE_ACCOUNT_PASSWORD = "key_active_account_password"
    static let KEY_ACCOUNTS_DICTIONARY = "key_accounts_dictionary"
    static let KEY_ACCOUNTS_NAME_DICTIONARY = "key_accounts_name_dictionary"
    static let KEY_LAST_REGISTERRED_ACCOUNT_EMAIL = "key_last_registerred_active_account_email"
    static let KEY_PROVISIONER_NAME = "key_provisioner_name"
    static let KEY_MESH_NETWORK_NAME = "key_mesh_network_name"
    static let KEY_LAST_MESH_NETWORK_NAME = "key_last_mesh_network_name"
    static let KEY_MESH_FILE_NAMES_LIST = "key_mesh_file_names_list"

    static let KEY_MESH_LAST_PROVISIONED_DEVICE_UUID = "key_mesh_last_provisioned_device_uuid"
    static let KEY_UNIQUE_ID = "key_unique_id"

    static let KEY_STATUS_COMPONENT_NAME_ARRAY = "key_status_component_name_array"

    static let KEY_PUBLISH_DATA_SUBSCRIBED_COMPONENTS = "key_publish_data_subscribed_comonents"

    static let KEY_IS_SENSOR_FAST_CADENCE_ENABLED = "key_is_sensor_fast_cadence_enabled"
    static let KEY_IS_SENSOR_TRIGGER_PUB_ENABLED = "key_is_sensor_trigger_pub_enabled"
    static let KEY_SENSOR_FAST_CADENCE_MEASURE_TYPE = "key_sensor_fast_cadence_measure_type"
}

class UserSettings: NSObject {
    static let shared = UserSettings()

    private var _database: UserDefaults?
    var database: UserDefaults {
        if let db = _database {
            return db
        } else {
            _database = UserDefaults(suiteName: "com.cypress.le.mesh.MeshApp.networksimulator.storage")
        }
        if let db = _database {
            return db
        } else {
            meshLog("error: failed to create App UserDefaults with bundleIdentifier")
            return UserDefaults.standard
        }
    }

    var isAccoutActive: Bool {
        get {
            return database.value(forKey: UserSettingsConstants.KEY_IS_ACCOUNT_ACTIVE) as? Bool ?? false
        }
        set(value) {
            isAccountLoggedIn = value
            database.set(value, forKey: UserSettingsConstants.KEY_IS_ACCOUNT_ACTIVE)
            database.synchronize()
        }
    }

    var isAccountLoggedIn: Bool = false
    var isAutoLoginning: Bool = false

    var activeName: String? {
        get {
            if let name = database.value(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_NAME) as? String, !name.isEmpty {
                return name
            }
            return "@me"
        }
        set(value) {
            if let value = value {
                database.set(value, forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_NAME)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_NAME)
            }
            database.synchronize()
        }
    }

    var activeEmail: String? {
        get {
            if let pwd = database.value(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_EMAIL) as? String, !pwd.isEmpty {
                return pwd
            }
            return nil
        }
        set(value) {
            if let value = value {
                database.set(value, forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_EMAIL)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_EMAIL)
            }
            database.synchronize()
        }
    }

    var activePssword: String? {
        get {
            if let pwd = database.value(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_PASSWORD) as? String, !pwd.isEmpty {
                return pwd
            }
            return nil
        }
        set(value) {
            if let value = value {
                database.set(value, forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_PASSWORD)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_ACTIVE_ACCOUNT_PASSWORD)
            }
            database.synchronize()
        }
    }

    var lastActiveAccountEmail: String? {
        get {
            if let name = database.value(forKey: UserSettingsConstants.KEY_LAST_REGISTERRED_ACCOUNT_EMAIL) as? String, !name.isEmpty {
                return name
            }
            return nil
        }
        set(value) {
            if let value = value {
                database.set(value, forKey: UserSettingsConstants.KEY_LAST_REGISTERRED_ACCOUNT_EMAIL)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_LAST_REGISTERRED_ACCOUNT_EMAIL)
            }
            database.synchronize()
        }
    }

    func generateProvisionerName(account: String?) -> String {
        let uniqueId = account ?? "anonym" + "_" + MeshFrameworkManager.shared.uniqueId
        let provisionerName = MeshFrameworkManager.shared.generateProvisionerName(uniqueId: uniqueId)
        self.provisionerName = provisionerName
        return provisionerName
    }

    // the provisionerName must be used after user logged in.
    var provisionerName: String {
        get {
            if let name = database.value(forKey: UserSettingsConstants.KEY_PROVISIONER_NAME) as? String, !name.isEmpty {
                return name
            }
            if self.activeEmail == nil {
                meshLog("error: UserSettings, provisionerName is read before user account logged in")
            }
            return generateProvisionerName(account: self.activeEmail)
        }
        set(value) {
            if value.isEmpty {
                database.removeObject(forKey: UserSettingsConstants.KEY_PROVISIONER_NAME)
            } else {
                database.set(value, forKey: UserSettingsConstants.KEY_PROVISIONER_NAME)
            }
            database.synchronize()
        }
    }

    var currentActiveMeshNetworkName: String? {
        get {
            if let name = database.value(forKey: UserSettingsConstants.KEY_MESH_NETWORK_NAME) as? String, !name.isEmpty {
                return name
            }
            return nil
        }
        set(value) {
            if let value = value {
                database.set(value, forKey: UserSettingsConstants.KEY_MESH_NETWORK_NAME)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_MESH_NETWORK_NAME)
            }
            database.synchronize()
        }
    }

    var lastProvisionedDeviceUuid: UUID? {
        get {
            if let archivedData = database.value(forKey: UserSettingsConstants.KEY_MESH_LAST_PROVISIONED_DEVICE_UUID) as? Data {
                return NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? UUID
            }
            return nil
        }
        set(value) {
            if let uuid = value {
                let data = NSKeyedArchiver.archivedData(withRootObject: uuid)
                database.set(data, forKey: UserSettingsConstants.KEY_MESH_LAST_PROVISIONED_DEVICE_UUID)
            } else {
                database.removeObject(forKey: UserSettingsConstants.KEY_MESH_LAST_PROVISIONED_DEVICE_UUID)
            }
            database.synchronize()
        }
    }

    func getLastProvisionedDevice() -> String? {
        guard let uuid = lastProvisionedDeviceUuid else {
            return nil
        }
        return MeshFrameworkManager.shared.getMeshComponentsByDevice(uuid: uuid)?.first
    }

    func resetCurrentAccount() {
        lastProvisionedDeviceUuid = nil
        UserSettings.shared.currentActiveComponentName = nil
        UserSettings.shared.currentActiveGroupComponents.removeAll()
        UserSettings.shared.currentActiveGroupName = nil
        UserSettings.shared.isAccountLoggedIn = false
        UserSettings.shared.isAccoutActive = false
        UserSettings.shared.openingMeshNetworkName = nil
        UserSettings.shared.meshNetworks.removeAll()
        UserSettings.shared.provisionerName = ""
        UserSettings.shared.currentActiveMeshNetworkName = nil
        UserSettings.shared.isCurrentActiveMeshNetworkOpenned = false

        UserSettings.shared.lastActiveAccountEmail = nil
        UserSettings.shared.activePssword = nil
        UserSettings.shared.activeEmail = nil
        UserSettings.shared.activeName = nil
    }

    var meshNetworks: [String] = []

    var openingMeshNetworkName: String?
    var isCurrentActiveMeshNetworkOpenned: Bool = false

    var currentActiveGroupName: String?
    var currentActiveGroupComponents: [Any] = []
    var currentActiveComponentName: String?
}

// Simulate network storage.
extension UserSettings {
    // [account_email, account_password]
    var accounts: [String: String] {
        get {
            if let archivedData = database.value(forKey: UserSettingsConstants.KEY_ACCOUNTS_DICTIONARY) as? Data,
                let accounts = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String : String] {
                return accounts
            }
            return [:]
        }
        set(value) {
            let data = NSKeyedArchiver.archivedData(withRootObject: value)
            database.set(data, forKey: UserSettingsConstants.KEY_ACCOUNTS_DICTIONARY)
            database.synchronize()
        }
    }

    // [account_email, account_name_default_@me]
    var accountsName: [String: String] {
        get {
            if let archivedData = database.value(forKey: UserSettingsConstants.KEY_ACCOUNTS_NAME_DICTIONARY) as? Data,
                let names = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String : String] {
                return names
            }
            return [:]
        }
        set(value) {
            let data = NSKeyedArchiver.archivedData(withRootObject: value)
            database.set(data, forKey: UserSettingsConstants.KEY_ACCOUNTS_NAME_DICTIONARY)
            database.synchronize()
        }
    }

    // For all *.json files, there are all dedicated to a specific User.
    // But for all *.bin mesh files, there are all dedicated to a specific installation of the App, which means
    // when user login on anthoter device, or within same device wiht the App to be reinstalled, the *.bin files will be different.
    func getKeyName(for email: String, identifier: String) -> String {
        if identifier.hasSuffix(".bin") {
            return "mesh_\(email)_\(MeshFrameworkManager.shared.uniqueId)_\(identifier)"
        } else {
            return "mesh_\(email)_\(identifier)"
        }
    }

    func getMeshFilesList() -> [String]? {
        if let userAccount = self.activeEmail {
            let key = getKeyName(for: userAccount, identifier: UserSettingsConstants.KEY_MESH_FILE_NAMES_LIST)
            if let archivedData  = database.value(forKey: key) as? Data,
                let nameList = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String] {
                return nameList
            }
        } else {
            meshLog("error: invalid active user name, the mesh library must be initialized before using the getMeshFilesList API")
        }
        return nil
    }

    func setMeshFilesList(fileNameList: [String]) {
        if let userAccount = self.activeEmail, fileNameList.count > 0 {
            let archivedData = NSKeyedArchiver.archivedData(withRootObject: fileNameList)
            let key = getKeyName(for: userAccount, identifier: UserSettingsConstants.KEY_MESH_FILE_NAMES_LIST)
            database.set(archivedData, forKey: key)
            database.synchronize()
        } else {
            meshLog("error: invalid active user name, the mesh library must be initialized before using the setMeshFilesList API")
        }
    }

    func getMeshFile(fileName: String) -> Any? {
        if let userAccount = self.activeEmail, fileName.count > 0 {
            let key = getKeyName(for: userAccount, identifier: fileName)
            if let archivedData  = database.value(forKey: key) as? Data,
                let data = NSKeyedUnarchiver.unarchiveObject(with: archivedData) {
                return data
            }
        } else {
            meshLog("error: invalid active user name, the mesh library must be initialized before using the getMeshFile API")
        }
        return nil
    }

    func setMeshFile(fileName: String, content: Any) {
        if let userAccount = self.activeEmail, fileName.count > 0 {
            let archivedData = NSKeyedArchiver.archivedData(withRootObject: content)
            let key = getKeyName(for: userAccount, identifier: fileName)
            database.set(archivedData, forKey: key)
            database.synchronize()
        } else {
            meshLog("error: invalid active user name, the mesh library must be initialized before using the setMeshFile API")
        }
    }

    func deleteMeshFile(fileName: String) {
        if let userAccount = self.activeEmail, fileName.count > 0 {
            let key = getKeyName(for: userAccount, identifier: fileName)
            database.removeObject(forKey: key)
            var meshFilesList = getMeshFilesList()
            if let list = meshFilesList, list.count > 0 {
                for (index, _) in list.enumerated() {
                    meshFilesList?.remove(at: index)
                    break
                }
                setMeshFilesList(fileNameList: meshFilesList ?? [])
            }
            database.synchronize()
        } else {
            meshLog("error: invalid active user name, the mesh library must be initialized before using the deleteMeshFile API")
        }
    }
}

// API for get/set mesh component status data. Before using these APIs, must make sure the mesh network has been opened successfully.
extension UserSettings {
    private var meshNetworkKey: String? {
        guard let networkName = currentActiveMeshNetworkName else {
            return nil
        }
        return "mesn_\(provisionerName)_\(networkName)"
    }

    func getComponentKey(comonentName: String) -> String? {
        guard let meshNetworkKey = meshNetworkKey else {
            return nil
        }
        return meshNetworkKey + "_\(comonentName)_status"
    }

    private var statusComponentNamesArray: [String]? {
        get {
            guard let meshNetworkKey = meshNetworkKey else { return nil }

            let key = "\(meshNetworkKey)_\(UserSettingsConstants.KEY_STATUS_COMPONENT_NAME_ARRAY)"
            if let archivedData = database.value(forKey: key) as? Data,
                let names = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String] {
                return names
            }
            return nil
        }
        set(value) {
            guard let meshNetworkKey = meshNetworkKey else { return }

            let key = "\(meshNetworkKey)_\(UserSettingsConstants.KEY_STATUS_COMPONENT_NAME_ARRAY)"
            guard let value = value else {
                database.removeObject(forKey: key)
                database.synchronize()
                return
            }

            let data = NSKeyedArchiver.archivedData(withRootObject: value)
            database.set(data, forKey: key)
            database.synchronize()
        }
    }


    func setComponentStatus(componentName: String, values: [String: Any]) {
        guard let key = getComponentKey(comonentName: componentName) else {
            return
        }

        // merge and update new values into existing values.
        var newValues = getComponentStatus(componentName: componentName) ?? [:]
        for (key, value) in values {
            newValues[key] = value
        }

        if newValues.isEmpty {
            database.removeObject(forKey: key)
        } else {
            let data = NSKeyedArchiver.archivedData(withRootObject: newValues)
            database.set(data, forKey: key)
        }
        database.synchronize()
    }

    func removeComponentStatus(componentName: String) {
        guard let key = getComponentKey(comonentName: componentName) else {
            return
        }

        database.removeObject(forKey: key)
        database.synchronize()
    }

    func getComponentStatus(componentName: String) -> [String: Any]? {
        guard let key = getComponentKey(comonentName: componentName),
            let archivedData = database.value(forKey: key) as? Data,
            let values = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String : Any] else {
            return nil
        }
        return values
    }

    var publishDataSubscribedComponents: [String] {
        get {
            if let archivedData = database.value(forKey: UserSettingsConstants.KEY_PUBLISH_DATA_SUBSCRIBED_COMPONENTS) as? Data,
                let names = NSKeyedUnarchiver.unarchiveObject(with: archivedData) as? [String] {
                return names
            }
            return []
        }
        set(value) {
            let data = NSKeyedArchiver.archivedData(withRootObject: value)
            database.set(data, forKey: UserSettingsConstants.KEY_PUBLISH_DATA_SUBSCRIBED_COMPONENTS)
            database.synchronize()
        }
    }
    func addPublishDataSubscribedComponent(conponentName: String) {
        publishDataSubscribedComponents.append(conponentName)
    }
    func removePublishDataSubscribedComponent(conponentName: String) {
        publishDataSubscribedComponents.removeAll(where: {$0 == conponentName})
    }
    func hasComponentSubscribedPublishData(componentName: String) -> Bool {
        return publishDataSubscribedComponents.filter({$0 == componentName}).count > 0 ? true : false
    }

    var isSensorFastCadenceEnabled: Bool {
        get {
            return database.value(forKey: UserSettingsConstants.KEY_IS_SENSOR_FAST_CADENCE_ENABLED) as? Bool ?? false
        }
        set(value) {
            database.set(value, forKey: UserSettingsConstants.KEY_IS_SENSOR_FAST_CADENCE_ENABLED)
            database.synchronize()
        }
    }

    var isSensorTriggerPubEnabled: Bool {
        get {
            return database.value(forKey: UserSettingsConstants.KEY_IS_SENSOR_TRIGGER_PUB_ENABLED) as? Bool ?? false
        }
        set(value) {
            database.set(value, forKey: UserSettingsConstants.KEY_IS_SENSOR_TRIGGER_PUB_ENABLED)
            database.synchronize()
        }
    }

    var sensorFastCadenceMeasureType: String {
        get {
            return database.value(forKey: UserSettingsConstants.KEY_SENSOR_FAST_CADENCE_MEASURE_TYPE) as? String ?? MeshControl.getMeasurementTypeText(type: MeshControl.MEASUREMENT_TYPE_INSIDE)!
        }
        set(value) {
            database.set(value, forKey: UserSettingsConstants.KEY_SENSOR_FAST_CADENCE_MEASURE_TYPE)
            database.synchronize()
        }
    }
}
