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
 * Mesh App relative constants value definitions.
 */

import UIKit
import Foundation

struct MeshAppConstants {
    static let MESH_DEFAULT_ALL_COMPONENTS_GROUP_NAME = "All (Fixed)"  // default parent group name for all other group in the network group.

    static let GROUPT_DETAIL_SEGMENTED_CONTROL = 0
    static let GROUPT_DETAIL_SEGMENTED_ALL_DEVICE = 1
    static let GROUPT_DETAIL_SEGMENTED_DEFAULT = GROUPT_DETAIL_SEGMENTED_CONTROL

    // TODO: Update the support min and max CTL temperature range based on real CTL light device.
    // Based on Mesh Model v1.0, section 6.1.3.3 Light CTL Temperature Range, the max and min should between 800 ~ 20000 Kelvin.
    static let LIGHT_TEMPERATURE_MIN: CGFloat = 2000            // unit: Kelvin, the min value should be set based on specific devices.
    static let LIGHT_TEMPERATURE_MAX: CGFloat = 8000            // unit: Kelvin, the max value should be set based on specific devices.
    static let LIGHT_MIN_VISIBLE_LIGHTNESS: CGFloat = 2.0       // unit: percentage, should be updated based on real device.

    static let LIGHT_HSL_DELAY_TIME: Int = 0            // unit: ms
    static let LIGHT_CTL_DELAY_TIME: Int = 0            // unit: ms
}

struct MeshAppStoryBoardIdentifires {
    static let START_SCREEN = "StartViewController"
    static let LOGIN_SCREEN = "LoginViewController"
    static let SIGNUP_SCREEN = "SignUpViewController"
    static let PROFILE_SETTINGS_SCREEN = "ProfileSettingsViewController"
    static let NETWORK_LIST = "NetworkListViewController"
    static let NETWORK_LIST_CELL = "NetworkListTableViewCell"
    static let NETWORK_LIST_EMPTY_CELL = "NetworkListEmptyTableViewCell"
    static let TRANSITION = "TransitionViewController"
    static let GROUP_LIST = "GroupListViewController"
    static let GROUP_LIST_CELL = "GroupListTableViewCell"
    static let GROUP_LIST_EMPTY_CELL = "GroupListEmptyTableViewCell"
    static let GROUP_DETAILS = "GroupDetailsViewController"
    static let GROUP_DETAILS_CONTROLS = "GroupDetailsControlsViewController"
    static let GROUP_DETAILS_DEVICE_LIST = "GroupDetailsDeviceListViewController"
    static let DEVICE_LIST_CELL = "DeviceListTableViewCell"
    static let DEVICE_LIST_EMPTY_CELL = "DeviceListEmptyTableViewCell"

    static let NETWORK_LIST_SELECT_POPOVER = "SelectNetworkPopoverViewController"
    static let NETWORK_LIST_SELECT_POPOVER_CELL = "SelectNetworkPopoverTableViewCell"

    static let GROUP_DETAILS_CONTROLS_CONTENT = "GroupDetailsControlsContent"
    static let GROUP_DETAILS_DEVICE_LIST_CONTENT = "GroupDetailsDeviceListContent"

    static let COMPONENT = "ComponentViewController"

    static let UNPROVISIONED_DEVICES = "UnprovisionedDevicesViewController"
    static let UNPROVISIONED_DEVICES_LIST_EMPTY_CELL = "UnprovisionedDeviceListEmptyTableViewCell"
    static let UNPROVISIONED_DEVICES_LIST_CELL = "UnprovisionedDeviceListTableViewCell"
    static let PROVISIONING_STATUS_POPOVER = "ProvisioningStatusPopoverViewController"

    static let MENU_SCREEN = "MenuViewController"
    static let MENU_TITLE_CELL = "MenuTitleTableViewCell"
    static let MENU_NETWORK_STATUS_CELL = "MenuNetworkStatusTableViewCell"
    static let MENU_MY_NETWORKS_CELL = "MenuMyNetworkTableViewCell"
    static let MENU_MY_GROUPS_CELL = "MenuMyGroupsTableViewCell"
    static let MENU_FIRMWARE_UPGRADE_CELL = "MenuFirmwareUpgradeTableViewCell"
    static let MENU_DELETING_DEVICES_CELL = "MenuDeletingDevicesTableViewCell"

    static let DELETING_DEVIES = "DeletingDevicesViewController"
    static let DELETING_DEVIES_EMPTY_CELL = "DeletingDevicesEmptyTableViewCell"
    static let DELETING_DEVIES_CELL = "DeletingDevicesTableViewCell"

    static let FIRMWARE_UPGRADE = "FirmwareUpgradeViewController"
    static let FIRMWARE_UPGRADE_CELL = "FirmwareUpgradeTableViewCell"
    static let FIRMWARE_UPGRADE_EMPTY_CELL = "FirmwareUpgradeEmptyTableViewCell"
    static let DEVICE_OTA_UPGRADE = "DeviceOtaUpgradeViewController"
    static let MESH_OTA_DFU = "MeshOtaDfuViewController"

    static let VENDOR_SPECIFIC_DATA = "VendorSpecificDataViewController"
    static let LIGHT_LC_SETTINGS = "LightLCSettingsViewController"
    static let PUBLICATION_CONFIGURE = "PublicationConfigureViewController"

    static let POPOVER_VIEW = "PopoverViewController"
    static let POPOVER_CELL = "PopoverTableViewCell"

    static let SENSOR = "SensorViewController"
    static let SENSOR_SETTING = "SensorSettingViewController"

    static let SEGUE_GROUP_CELL_SELECT = "GroutListCellSelectedSegue"
    static let SEGUE_GROUP_CELL_ALL_DEVICES = "GroutListCellAllDevicesSegue"
    static let SEGUE_GROUP_CELL_GROUP_CONTROL = "GroutListCellGroupControlsSegue"
    static let SEGUE_COMPONENT_BACK_TO_GROUP_DETAILS = "ComponentViewControllerBackToGroupDetailsSegue"
    static let SEGUE_GROUP_DETAIL_TO_ADD_DEVICE = "GroupDetailsToAddDviceSegue"
    static let SEGUE_SENSOR_CONFIGURE = "SensorToSensorSettingSegue"
    static let SEGUE_LIGHT_LC_SETTINGS = "LightLCSettingsSegue"

    static let SCAN_PROVISION_TEST_VIEW = "ScanProvisionTestViewController"
    static let SEGUE_SCAN_PROVISION_BACK = "ScanProvisionTestViewControllerBackSegue"
}

struct MeshAppImageNames {
    static let hslColorImage = "hslColor"
    static let ctlColorImage = "ctlColor"
    static let backIconImage = "backIcon"
    static let lightOnImage = "light"
    static let lightOffImage = "lightOff"
    static let lightsOnImage = "lights"
    static let lightsOffImage = "lightsOff"
    static let brightnessOnImage = "brightnessOn"
    static let brightnessOffImage = "brightnessOff"
    static let cypressLogoImage = "cypressLogo"
    static let checkboxFilledImage = "checkboxFilled"
    static let checkboxEmptyImage = "checkboxEmpty"
    static let leftTriangleImage = "leftTriangle"
    static let rightTriangleImage = "rightTriangle"
    static let downTriangleImage = "downTriangle"
    static let sensorImage = "sensor"
    static let uploadImage = "upload"
    static let stopImage = "stop"
    static let metadataImage = "metadata"
    static let applyImage = "apply"
    static let attachImage = "attach"
    static let infoRedImage = "infoRed"
    static let brightnessImage = "brightness"
}

extension UIColor {
    static let errorColor = UIColor(red: 213/255, green: 0, blue: 0, alpha: 1)
    static let orangeBgColor = UIColor(red: 255/255, green: 102/255, blue: 1/255, alpha: 1)
    static let lineColor = UIColor(red: 140/255, green: 140/255, blue: 140/255, alpha: 1)
    static let redStatusColor = UIColor(red: 211/255, green: 0, blue: 21/255, alpha: 1)
    static let greenStatusColor = UIColor(red: 66/255, green: 189/255, blue: 12/255, alpha: 1)
    static let grayStatusColor = UIColor(red: 110/255, green: 107/255, blue: 111/255, alpha: 1)
}

struct MeshComponentValueKeys {
    static let onoff = "onoff"
    static let level = "level"
    static let hslLightness = "hslLightness"
    static let ctlLightness = "ctlLightness"
    static let dimmableLightness = "dimmableLightness"
    static let hue = "hue"
    static let saturation = "saturation"
    static let temperature = "temperature"
    static let deltaUv = "deltaUv"
    static let data = "data"
}

struct MeshConstantText {
    static let UNKNOWN_DEVICE_NAME = "Unknown Device Name"
}
