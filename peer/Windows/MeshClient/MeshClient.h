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
* MeshClient.h : main header file for the application
*
*/

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "wiced_bt_mesh_core.h"

#include "resource.h"       // main symbols


// CMeshClientApp:
// See MeshClient.cpp for the implementation of this class
//

class CMeshClientApp : public CWinApp
{
public:
    CMeshClientApp();

// Overrides
public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
};

#define BTW_GATT_UUID_SERVCLASS_BATTERY                     0x180F    /* Battery Service  */
#define BTW_GATT_UUID_SERVCLASS_DEVICE_INFO                 0x180A    /* Device Information Service  */

#define BTW_GATT_UUID_CHAR_DIS_SYSTEM_ID                    0x2A23
#define BTW_GATT_UUID_CHAR_DIS_MODEL_NUMBER                 0x2A24
#define BTW_GATT_UUID_CHAR_DIS_SERIAL_NUMBER                0x2A25
#define BTW_GATT_UUID_CHAR_DIS_FIRMWARE_REVISION            0x2A26
#define BTW_GATT_UUID_CHAR_DIS_HARDWARE_REVISION            0x2A27
#define BTW_GATT_UUID_CHAR_DIS_SOFTWARE_REVISION            0x2A28
#define BTW_GATT_UUID_CHAR_DIS_MANUFACTURER_NAME            0x2A29

#define BTW_GATT_UUID_CHAR_BATTERY_LEVEL                    0x2A19
#define BTW_GATT_UUID_CHAR_BATTERY_STATE                    0x2A1A
#define BTW_GATT_UUID_CHAR_BATTERY_STATELEVEL               0x2A1B

#define BTW_GATT_UUID_DESCRIPTOR_USER_DESCRIPTION           0x2901
#define BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG              0x2902      /*  Client Characteristic Configuration */
#define BTW_GATT_UUID_DESCRIPTOR_PRESENTATION_FORMAT        0x2904
#define BTW_GATT_UUID_DESCRIPTOR_NUMBER_OF_DIGITALS         0x2909

extern GUID guidSvcWSUpgrade;
extern GUID guidCharWSUpgradeControlPoint;
extern GUID guidCharWSUpgradeData;

// {9E5D1E47-5C13-43A0-8635-82AD38A1386F}
static const GUID GUID_OTA_FW_UPGRADE_SERVICE = { 0xae5d1e47, 0x5c13, 0x43a0,{ 0x86, 0x35, 0x82, 0xad, 0x38, 0xa1, 0x38, 0x1f } };

// {C7261110-F425-447A-A1BD-9D7246768BD8}
static const GUID GUID_OTA_SEC_FW_UPGRADE_SERVICE = { 0xc7261110, 0xf425, 0x447a,{ 0xa1, 0xbd, 0x9d, 0x72, 0x46, 0x76, 0x8b, 0xd8 } };

// {E3DD50BF-F7A7-4E99-838E-570A086C666B}
static const GUID GUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT = { 0xa3dd50bf, 0xf7a7, 0x4e99,{ 0x83, 0x8e, 0x57, 0xa, 0x8, 0x6c, 0x66, 0x1b } };

// {92E86C7A-D961-4091-B74F-2409E72EFE36}
static const GUID GUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA = { 0xa2e86c7a, 0xd961, 0x4091,{ 0xb7, 0x4f, 0x24, 0x9, 0xe7, 0x2e, 0xfe, 0x26 } };

const GUID guidBT                               = {0,                                                               0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidSvcMeshProvisioning              = { WICED_BT_MESH_CORE_UUID_SERVICE_PROVISIONING,                   0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
const GUID guidSvcMeshProxy                     = { WICED_BT_MESH_CORE_UUID_SERVICE_PROXY,                          0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
const GUID guidCharProvisioningDataIn           = { WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROVISIONING_DATA_IN,    0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
const GUID guidCharProvisioningDataOut          = { WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROVISIONING_DATA_OUT,   0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
const GUID guidCharProxyDataIn                  = { WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROXY_DATA_IN,           0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
const GUID guidCharProxyDataOut                 = { WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROXY_DATA_OUT,          0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };

const GUID guidSvcBattery                       = {BTW_GATT_UUID_SERVCLASS_BATTERY,         0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidCharBatLevel                     = {BTW_GATT_UUID_CHAR_BATTERY_LEVEL,        0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidClntConfigDesc                   = {BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG,  0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidSvcDeviceInfo                    = {BTW_GATT_UUID_SERVCLASS_DEVICE_INFO,      0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidCharModelNumber                  = {BTW_GATT_UUID_CHAR_DIS_MODEL_NUMBER,      0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidCharManufacturer                 = {BTW_GATT_UUID_CHAR_DIS_MANUFACTURER_NAME, 0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
const GUID guidCharSystemId                     = {BTW_GATT_UUID_CHAR_DIS_SYSTEM_ID,         0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

extern "C" void ods(char * fmt_str, ...);
extern "C" void ble_tracen(const char *p_str, UINT32 len);
extern void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha);
extern void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid);
extern void Log(WCHAR *fmt, ...);

extern CMeshClientApp theApp;
