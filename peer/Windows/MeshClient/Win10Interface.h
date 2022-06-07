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
* Win10Interface.h : Header file
* Helper interfaces to Windows GATT ASync APIs
*
*/

#pragma once
#include "BtInterface.h"

#include <iostream>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <wrl\event.h>
#include <stdio.h>
#include <windows.devices.bluetooth.h>
#include <windows.devices.bluetooth.advertisement.h>
#include <windows.devices.bluetooth.genericattributeprofile.h>
#include <memory>
#include <functional>
#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <Robuffer.h>
#include <vector>

using namespace std;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace ABI::Windows::Devices::Bluetooth::Advertisement;

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

typedef enum Error {
    NoError,
    UnknownError,
    UnknownRemoteDeviceError,
    NetworkError,
    InvalidBluetoothAdapterError,
    ConnectionError,
    AdvertisingError,
} tError;

typedef enum ControllerState {
    UnconnectedState = 0,
    ConnectingState,
    ConnectedState,
    DiscoveringState,
    DiscoveredState,
    ClosingState,
    AdvertisingState,
} tServiceState;

struct ServiceCharEntry {
    ServiceCharEntry() {}
    ServiceCharEntry(GUID g, ComPtr<IVectorView<GattCharacteristic *>>  c)
        : svcGuid(g)
        , charVec(c)
    {
    }

    ComPtr<IVectorView<GattCharacteristic *>> charVec;
    GUID svcGuid;
};

struct ValueChangedEntry {
    ValueChangedEntry() {}
    ValueChangedEntry(Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic> c,
        EventRegistrationToken t)
        : characteristic(c)
        , token(t)
    {
    }

    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic> characteristic;
    EventRegistrationToken token;
};

class CBtWin10Interface : public CBtInterface
{
public:
    CBtWin10Interface (BLUETOOTH_ADDRESS *bth, LPVOID NotificationContext);
    virtual ~CBtWin10Interface();

    BOOL Init(BOOL ConnectProvisioning);
    BOOL SetDescriptorValue(const GUID *p_guidServ, const GUID *p_guidChar, USHORT uuidDescr, BTW_GATT_VALUE *pValue);
    BOOL WriteCharacteristic(const GUID *p_guidServ, const GUID *p_guidChar, BOOL without_resp, BTW_GATT_VALUE *pValue);

     BOOL RegisterNotification(const GUID *p_guidServ, const GUID *p_guidChar);
     void PostNotification(int charHandle, ComPtr<IBuffer> buffer);

    ComPtr<IGattDeviceService> getNativeService(const GUID *p_guidServ);
    ComPtr<IGattCharacteristic> getNativeCharacteristic(const GUID *p_guidServ, const GUID *p_guidChar);

    void setError(tError newError);
    void setState(tServiceState newState);
    tServiceState getState();
    void ResetInterface();

    BOOL GetDescriptorValue(USHORT *DescriptorValue);
    BOOL SetDescriptorValue(USHORT Value);

    BOOL SendWsUpgradeCommand(BTW_GATT_VALUE *pValue);
    BOOL SendWsUpgradeCommand(BYTE Command);
    BOOL SendWsUpgradeCommand(BYTE Command, USHORT sParam);
    BOOL SendWsUpgradeCommand(BYTE Command, ULONG lParam);
    BOOL SendWsUpgradeData(BYTE *Data, DWORD len);
    BOOL CheckForOTAServices(const GUID* guid_ota_service, const GUID* guid_ota_sec_service);
    BOOL CheckForProvProxyServices();
    UINT16 GetMTUSize();

    BOOL m_bConnected;

    BOOL m_bServiceFound;
private:
    HANDLE m_hDevice;
    HANDLE m_hEvent;
    HANDLE m_hErrorEvent;
    BOOL m_bDataWritePending;

    USHORT num_chars;
    BTH_LE_GATT_CHARACTERISTIC *m_pchar;

    USHORT *pnum_descr;
    BTH_LE_GATT_DESCRIPTOR **m_ppdescr;

    BLUETOOTH_GATT_EVENT_HANDLE m_pEventHandle, m_pEventHandle2, m_pEventHandle3, m_pEventHandle4;

    std::vector <ServiceCharEntry> mServiceCharsVector;
    std::vector <ValueChangedEntry> mValueChangedTokens;

    EventRegistrationToken mStatusChangedToken;
    tServiceState m_state;
    tError m_error;

    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice> mDevice;
};
