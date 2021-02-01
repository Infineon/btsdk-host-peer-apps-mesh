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
* Win10Interface.cpp : implementation file
* Helper interfaces to Windows GATT ASync APIs
*/

#include "stdafx.h"
#include <setupapi.h>
#include "MeshClient.h"
#include "MeshClientDlg.h"
#include "wiced_mesh_client.h"
#include "afxdialogex.h"

#define ASSERT_SUCCEEDED(hr) {if(FAILED(hr)) return (hr == S_OK);}
#define RETURN_IF_FAILED(p, q) { FAILED(hr) ? { cout << p;} : q;}

#define HR_FAIL -1

typedef ITypedEventHandler<GattCharacteristic *, GattValueChangedEventArgs *> ValueChangedHandler;
typedef ITypedEventHandler<BluetoothLEDevice *, IInspectable *> StatusHandler;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//converts BD_ADDR to the __int64
#define bda2int(a) (a ? ((((__int64)a[0])<<40)+(((__int64)a[1])<<32)+(((__int64)a[2])<<24)+(((__int64)a[3])<<16)+(((__int64)a[4])<<8)+((__int64)a[5])) : 0)

extern "C" void ods(char * fmt_str, ...);

char* guid2str(GUID guid)
{
    WCHAR szService[80];
    static char buff[256] = { 0 };
    UuidToString(szService, 80, &guid);
    sprintf_s(buff, "%S", szService);
    return buff;
}

template <typename T>
inline HRESULT _await_impl(const Microsoft::WRL::ComPtr<T> &asyncOp, UINT timeout)
{
    Microsoft::WRL::ComPtr<IAsyncInfo> asyncInfo;
    HRESULT hr = asyncOp.As(&asyncInfo);
    if (FAILED(hr))
        return hr;

    AsyncStatus status;
    UINT tval = 0;
    hr = HR_FAIL;
    while (SUCCEEDED(hr = asyncInfo->get_Status(&status)) && status == AsyncStatus::Started)
    {
        Sleep(500);
        if (timeout)
        {
            if (tval < timeout)
                tval += 500;
            else
                break;
        }
    }

    if (FAILED(hr) || status != AsyncStatus::Completed) {
        HRESULT ec;
        hr = asyncInfo->get_ErrorCode(&ec);
        if (FAILED(hr))
            return hr;
        hr = asyncInfo->Close();
        if (FAILED(hr))
            return hr;
        return ec;
    }

    return hr;
}

template <typename T, typename U>
inline HRESULT await(const Microsoft::WRL::ComPtr<T> &asyncOp, U *results, UINT timeout = 0)
{
    HRESULT hr = _await_impl(asyncOp, timeout);
    if (FAILED(hr))
        return hr;

    return asyncOp->GetResults(results);
}

static char* byteArrayFromBuffer(const ComPtr<IBuffer> &buffer, bool isWCharString = false)
{
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    HRESULT hr = buffer.As(&byteAccess);
    char *data;
    hr = byteAccess->Buffer(reinterpret_cast<byte **>(&data));
    UINT32 size;
    hr = buffer->get_Length(&size);

    return data;
}

void CBtWin10Interface::setState(tServiceState state)
{
    m_state = state;
}

tServiceState CBtWin10Interface::getState()
{
    return m_state;
}

void CBtWin10Interface::setError(tError error)
{
    m_error = error;
}

ComPtr<IGattDeviceService> CBtWin10Interface::getNativeService(const GUID *p_guidServ)
{
    GUID serviceUuid = *p_guidServ;
    ComPtr<IGattDeviceService> deviceService;

    HRESULT hr;
    hr = mDevice->GetGattService(serviceUuid, &deviceService);
    if (FAILED(hr))
        ods("Could not obtain native service for Uuid: %S", guid2str(serviceUuid));
    return deviceService;
}

ComPtr<IGattCharacteristic> CBtWin10Interface::getNativeCharacteristic(const GUID *p_guidServ, const GUID *p_guidChar)
{
    GUID serviceUuid = *p_guidServ;
    GUID  charUuid = *p_guidChar;

    ComPtr<IGattCharacteristic> characteristic;

    ods("%S", __FUNCTION__);

    for (const ServiceCharEntry &entry : mServiceCharsVector) {
        HRESULT hr;

        if (memcmp(&serviceUuid, &entry.svcGuid, sizeof(GUID)) == 0)
        {
            UINT characteristicsCount = 0;
            hr = entry.charVec->get_Size(&characteristicsCount);

            for (UINT j = 0; j < characteristicsCount; j++) {
                hr = entry.charVec->GetAt(j, &characteristic);
                GUID charuuid = { 0 };
                characteristic->get_Uuid(&charuuid);
                if (memcmp(&charuuid, &charUuid, sizeof(GUID)) == 0)
                    return characteristic;
            }
        }
    }

    return characteristic;
}

void CBtWin10Interface::PostNotification(int charHandle, ComPtr<IBuffer> buffer)
{
    ods("Characteristic change notification");
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    HRESULT hr = buffer.As(&byteAccess);

    char *data;
    hr = byteAccess->Buffer(reinterpret_cast<byte **>(&data));

    UINT32 size;
    hr = buffer->get_Length(&size);

    ComPtr<IGattCharacteristic> characteristic;
    BOOL bFound = FALSE;

    ods("%S", __FUNCTION__);

    GUID charuuid = { 0 };
    UINT16 charattrHandle = 0;

    for (const ServiceCharEntry &entry : mServiceCharsVector)
    {
        HRESULT hr;
        UINT characteristicsCount = 0;
        hr = entry.charVec->get_Size(&characteristicsCount);

        for (UINT j = 0; j < characteristicsCount; j++) {
            hr = entry.charVec->GetAt(j, &characteristic);

            characteristic->get_Uuid(&charuuid);
            charattrHandle = 0;
            characteristic->get_AttributeHandle(&charattrHandle);

            if (charattrHandle == charHandle)
            {
                bFound = TRUE;
                break;
            }
        }

        if (bFound)
            break;
    }

    if (!bFound)
        return;

    CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
    DWORD dwCharInstance = 0;
    BTW_GATT_VALUE *p = (BTW_GATT_VALUE *)malloc(sizeof(BTW_GATT_VALUE));
    if (!p)
        return;

    if (charuuid.Data1 == guidCharWSUpgradeControlPoint.Data1)
    {
        p->len = mesh_client_ota_data_decrypt(NULL, (uint8_t *)data, (uint16_t)size, p->value, sizeof(p->value));
    }
    else
    {
        p->len = (USHORT)size;
        memcpy(p->value, data, size);
    }
    pDlg->PostMessage(WM_USER + (charuuid.Data1 & 0xff), (WPARAM)dwCharInstance, (LPARAM)p);
}

BOOL CBtWin10Interface::RegisterNotification(const GUID *p_guidServ, const GUID *p_guidChar)
{
    GUID serviceUuid = *p_guidServ;
    GUID charUuid = *p_guidChar;

    ods("RegisterNotification characteristic:%S", guid2str(charUuid));
    ods(" in service %S for value changes", guid2str(serviceUuid));

    for (const ValueChangedEntry &entry : mValueChangedTokens)
    {
        GUID guuid;
        HRESULT hr;
        hr = entry.characteristic->get_Uuid(&guuid);
        ASSERT_SUCCEEDED(hr);

        if (memcmp(&guuid, &charUuid, sizeof(GUID)) == 0)
        {
            ods("Already registered");
            return FALSE;
        }
    }

    ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(p_guidServ, p_guidChar);

    if (!characteristic)
    {
        ods("Characteristic NULL");
        return FALSE;
    }

    EventRegistrationToken token;
    HRESULT hr;
    hr = characteristic->add_ValueChanged(Callback<ValueChangedHandler>([this](IGattCharacteristic *characteristic, IGattValueChangedEventArgs *args) {
        HRESULT hr;
        UINT16 handle;
        hr = characteristic->get_AttributeHandle(&handle);
        ASSERT_SUCCEEDED(hr);
        ComPtr<IBuffer> buffer;
        hr = args->get_CharacteristicValue(&buffer);
        ASSERT_SUCCEEDED(hr);
        PostNotification(handle, buffer);

        return (bool)TRUE;

    }).Get(), &token);
    ASSERT_SUCCEEDED(hr);

    mValueChangedTokens.push_back(ValueChangedEntry(characteristic, token));

    return TRUE;
}

BOOL CBtWin10Interface::Init(BOOL ConnectProvisioning)
{
    ods("%s", __FUNCTION__);

    __int64 remoteDevice = m_bth.ullLong;

    if (remoteDevice == 0) {
        ods("Invalid/null remote device address");
        setError(UnknownRemoteDeviceError);
        return FALSE;
    }

    setState(ConnectingState);

    HRESULT hr;
    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &deviceStatics);
    ASSERT_SUCCEEDED(hr);
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(remoteDevice, &deviceFromIdOperation);
    ASSERT_SUCCEEDED(hr);

    //hr = await(deviceFromIdOperation, mDevice.GetAddressOf());
    //ASSERT_SUCCEEDED(hr);

    //======>
    HANDLE handles[2];
    handles[0] = m_hEvent;
    handles[1] = m_hErrorEvent;

    hr = deviceFromIdOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<BluetoothLEDevice* >>
        ([this](IAsyncOperation<BluetoothLEDevice*> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error)
        {
            ods("get services Async operation failed: %d", status);
            SetEvent(m_hErrorEvent);
            return S_OK;
        }
        SetEvent(m_hEvent);
        return S_OK;
    }).Get());

    ASSERT_SUCCEEDED(hr);

    DWORD dwRet = WaitForMultipleObjects(2, handles, FALSE, 5000);
    BOOL bError = FALSE;

    if ((dwRet == WAIT_OBJECT_0 + 1) || (dwRet == WAIT_TIMEOUT))
    {
        ods("get device operation failed Error: %d", dwRet);
        bError = TRUE;
    }
    else if (dwRet == WAIT_OBJECT_0)
    {
        HRESULT hr;
        hr = deviceFromIdOperation->GetResults(&mDevice);
        if (FAILED(hr))
        {
            ods("get device operation failed");
            bError = TRUE;
        }
    }

    if (bError)
    {
        ods("Error getting device");
        return FALSE;
    }
    //======>
    if (!mDevice) {
        ods("Could not find LE device");
        setError(InvalidBluetoothAdapterError);
        setState(UnconnectedState);
        return FALSE;
    }
    BluetoothConnectionStatus status;
    hr = mDevice->get_ConnectionStatus(&status);
    ASSERT_SUCCEEDED(hr);

    hr = mDevice->add_ConnectionStatusChanged(Callback<StatusHandler>([this](IBluetoothLEDevice *dev, IInspectable *)
    {
        BluetoothConnectionStatus status;
        HRESULT hr;
        hr = dev->get_ConnectionStatus(&status);
        ASSERT_SUCCEEDED(hr);
        if (m_state == ConnectingState && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected)
        {
            setState(ConnectedState);
            // PostMessage Connected message
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_CONNECTED, (WPARAM)0, (LPARAM)0);
        }
        else if (m_state == ConnectedState && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Disconnected)
        {
            setState(UnconnectedState);
            // PostMessage disconnected message
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_DISCONNECTED, (WPARAM)0, (LPARAM)0);
        }
        return (bool)TRUE;
    }).Get(), &mStatusChangedToken);
    ASSERT_SUCCEEDED(hr);

    if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected)
    {
        setState(ConnectedState);
        // PostMessage Connected message
        return TRUE;
    }

    ComPtr<IAsyncOperation<GattDeviceServicesResult*>> services_op;
    ComPtr<IBluetoothLEDevice3> mDevice3;
    hr = mDevice.As(&mDevice3);

    ComPtr<IGattDeviceServicesResult> servicesresult;
    hr = mDevice3->GetGattServicesWithCacheModeAsync(BluetoothCacheMode_Uncached, &services_op);
    //======>
    handles[0] = m_hEvent;
    handles[1] = m_hErrorEvent;

    hr = services_op->put_Completed(Callback<IAsyncOperationCompletedHandler<GattDeviceServicesResult* >>
        ([this](IAsyncOperation<GattDeviceServicesResult*> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error)
        {
            ods("get services Async operation failed: %d", status);
            SetEvent(m_hErrorEvent);
            return S_OK;
        }
        SetEvent(m_hEvent);
        return S_OK;
    }).Get());

    ASSERT_SUCCEEDED(hr);

    dwRet = WaitForMultipleObjects(2, handles, FALSE, 5000);
    bError = FALSE;

    if ((dwRet == WAIT_OBJECT_0 + 1) || (dwRet == WAIT_TIMEOUT))
    {
        ods("get services operation failed Error: %d", dwRet);
        bError = TRUE;
    }
    else if (dwRet == WAIT_OBJECT_0)
    {
        HRESULT hr;
        hr = services_op->GetResults(&servicesresult);
        if (FAILED(hr))
        {
            ods("get services operation failed");
            bError = TRUE;
        }

        GattCommunicationStatus com_status;
        hr = servicesresult->get_Status(&com_status);
        if ((hr != S_OK) || com_status != GattCommunicationStatus_Success)
        {
            ods("get services Comm Status: %d", com_status);
            bError = TRUE;
        }
    }

    if(bError)
    {
        ods("Error getting services");
        return FALSE;
    }
//======>

    ComPtr<IVectorView <GattDeviceService *>> deviceServices;
    hr = servicesresult->get_Services(&deviceServices);
    ASSERT_SUCCEEDED(hr);

    UINT serviceCount = 0;
    hr = deviceServices->get_Size(&serviceCount);
    ASSERT_SUCCEEDED(hr);

    if (!serviceCount)
    {
        ods("Error getting services");
        CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
        pDlg->PostMessage(WM_MESH_DEVICE_DISCONNECTED, (WPARAM)0, (LPARAM)0);
        return FALSE;
    }

    for (UINT i = 0; i < serviceCount; i++) {

        ComPtr<IGattDeviceService> service;
        hr = deviceServices->GetAt(i, &service);
        ASSERT_SUCCEEDED(hr);

        GUID uuid = { 0 };
        service->get_Uuid(&uuid);

        UINT16 attribHandle = 0;
        service->get_AttributeHandle(&attribHandle);

        ComPtr<IGattDeviceService3> service3;
        hr = service.As(&service3);
        ASSERT_SUCCEEDED(hr);

        ComPtr<IAsyncOperation<GattCharacteristicsResult*>> charop;
        ComPtr<IGattCharacteristicsResult> charresult;
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;

        hr = service3->GetCharacteristicsWithCacheModeAsync(BluetoothCacheMode_Uncached, &charop);
        ASSERT_SUCCEEDED(hr);

        //hr = await(charop, charresult.GetAddressOf());
        //ASSERT_SUCCEEDED(hr);

//======>
        handles[0] = m_hEvent;
        handles[1] = m_hErrorEvent;

        hr = charop->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCharacteristicsResult* >>
            ([this](IAsyncOperation<GattCharacteristicsResult*> *op, AsyncStatus status)
        {
            if (status == AsyncStatus::Canceled || status == AsyncStatus::Error)
            {
                ods("get services Async operation failed: %d", status);
                SetEvent(m_hErrorEvent);
                return S_OK;
            }
            SetEvent(m_hEvent);
            return S_OK;
        }).Get());

        ASSERT_SUCCEEDED(hr);

        dwRet = WaitForMultipleObjects(2, handles, FALSE, 5000);
        bError = FALSE;

        if ((dwRet == WAIT_OBJECT_0 + 1) || (dwRet == WAIT_TIMEOUT))
        {
            ods("get characterisctics operation failed Error: %d", dwRet);
            bError = TRUE;
        }
        else if (dwRet == WAIT_OBJECT_0)
        {
            HRESULT hr;
            hr = charop->GetResults(&charresult);
            if (FAILED(hr))
            {
                ods("get characterisctics operation failed");
                bError = TRUE;
            }

            GattCommunicationStatus com_status;
            hr = charresult->get_Status(&com_status);
            if ((hr != S_OK) || com_status != GattCommunicationStatus_Success)
            {
                ods("get characterisctics Comm Status: %d", com_status);
                bError = TRUE;
            }
        }

        if (bError)
        {
            ods("Error getting characterisctics");
            return FALSE;
        }
//======>

        hr = charresult->get_Characteristics(&characteristics);
        ASSERT_SUCCEEDED(hr);

        if (hr == E_ACCESSDENIED) {
            // Everything will work as expected up until this point if the manifest capabilties
            // for bluetooth LE are not set.
            ods("Could not obtain characteristic list. Please check your manifest capabilities");
            setState(UnconnectedState);
            setError(ConnectionError);
            return FALSE;
        }
        else {
            ASSERT_SUCCEEDED(hr);
        }

        mServiceCharsVector.push_back(ServiceCharEntry(uuid, characteristics));

        BOOL DataOut = 0;
        BOOL DataIn = 0;

        UINT characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        ASSERT_SUCCEEDED(hr);
        for (UINT j = 0; j < characteristicsCount; j++)
        {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(j, &characteristic);
            ASSERT_SUCCEEDED(hr);

            GUID charuuid = { 0 };
            characteristic->get_Uuid(&charuuid);

            if (ConnectProvisioning)
            {
                if (memcmp(&charuuid, &guidCharProvisioningDataIn, sizeof(GUID)) == 0)
                    DataIn = TRUE;
                else if (memcmp(&charuuid, &guidCharProvisioningDataOut, sizeof(GUID)) == 0)
                    DataOut = TRUE;
            }
            else
            {
                if (memcmp(&charuuid, &guidCharProxyDataIn, sizeof(GUID)) == 0)
                    DataIn = TRUE;
                else if (memcmp(&charuuid, &guidCharProxyDataOut, sizeof(GUID)) == 0)
                    DataOut = TRUE;
            }
            UINT16 charattrHandle = 0;
            characteristic->get_AttributeHandle(&charattrHandle);

            ComPtr<IAsyncOperation<GattReadResult *>> op;
            GattCharacteristicProperties props;
            hr = characteristic->get_CharacteristicProperties(&props);
            ASSERT_SUCCEEDED(hr);
            if (!(props & GattCharacteristicProperties_Read))
                continue;
        }
    }

    return TRUE;
}

CBtWin10Interface::CBtWin10Interface(BLUETOOTH_ADDRESS *bth, LPVOID NotificationContext)
    : CBtInterface(bth, NULL, NotificationContext)
{
    m_bServiceFound = FALSE;
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hErrorEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_bDataWritePending = FALSE;
}

CBtWin10Interface::~CBtWin10Interface()
{
    ResetInterface();
}

void CBtWin10Interface::ResetInterface()
{
    if (mDevice && mStatusChangedToken.value)
        mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);

    ods("Unregistering %d  value change tokens", mValueChangedTokens.size());

    for (const ValueChangedEntry &entry : mValueChangedTokens)
        entry.characteristic->remove_ValueChanged(entry.token);

    mValueChangedTokens.clear();
    mServiceCharsVector.clear();

    // Release pointer to disconnect
    mDevice = nullptr;
}

BOOL CBtWin10Interface::SetDescriptorValue(const GUID *p_guidServ, const GUID *p_guidChar, USHORT uuidDescr, BTW_GATT_VALUE *pValue)
{
    GUID serviceUuid = *p_guidServ;
    GUID charUuid = *p_guidChar;

    ods("SetDescriptorValue Descriptor %04x characteristic: %04x in service %04x", uuidDescr, p_guidChar->Data1, p_guidServ->Data1);

    HRESULT hr;
    ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(p_guidServ, p_guidChar);
    if (!characteristic) {
        ods("Could not obtain native characteristic");
        return S_OK;
    }

    GattClientCharacteristicConfigurationDescriptorValue value = GattClientCharacteristicConfigurationDescriptorValue_None;

    if (((pValue->value[0] & 1) != 0))
        value = GattClientCharacteristicConfigurationDescriptorValue_Notify;

    else if (((pValue->value[0] & 2) != 0))
        value = (GattClientCharacteristicConfigurationDescriptorValue) (GattClientCharacteristicConfigurationDescriptorValue_Indicate);

    ComPtr<IAsyncOperation<enum GattCommunicationStatus>> writeOp;
    hr = characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(value, &writeOp);
    ASSERT_SUCCEEDED(hr);
    hr = writeOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus >>([this](IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            ods("Descriptor write operation failed");
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_DISCONNECTED, (WPARAM)0, (LPARAM)0);
            return S_OK;
        }
        GattCommunicationStatus result;
        HRESULT hr;
        hr = op->GetResults(&result);
        if (FAILED(hr)) {
            ods("Could not obtain result for descriptor");
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_DISCONNECTED, (WPARAM)0, (LPARAM)0);
            return S_OK;
        }
        if (result != GattCommunicationStatus_Success) {
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_DISCONNECTED, (WPARAM)0, (LPARAM)0);
            return S_OK;
        }
        if (result == GattCommunicationStatus_Success) {
            CMeshClientDlg *pDlg = (CMeshClientDlg *)m_NotificationContext;
            pDlg->PostMessage(WM_MESH_DEVICE_CCCD_PUT_COMPLETE, (WPARAM)0, (LPARAM)0);
            return S_OK;
        }
        return S_OK;
    }).Get());
    ASSERT_SUCCEEDED(hr);
    return S_OK;
}

BOOL CBtWin10Interface::WriteCharacteristic(const GUID *p_guidServ, const GUID *p_guidChar, BOOL without_resp, BTW_GATT_VALUE *pValue)
{
    GUID serviceUuid = *p_guidServ;
    GUID charUuid = *p_guidChar;

    ods("WriteCharacteristic characteristic %04x in service %04x len:%d", p_guidChar->Data1, p_guidServ->Data1, pValue->len);

    HRESULT hr;
    ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(p_guidServ, p_guidChar);
    if (!characteristic) {
        ods("Could not obtain native characteristic");
        return FALSE;
    }
    ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(), &bufferFactory);
    ASSERT_SUCCEEDED(hr);
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    const int length = pValue->len;
    hr = bufferFactory->Create(length, &buffer);
    ASSERT_SUCCEEDED(hr);
    hr = buffer->put_Length(length);
    ASSERT_SUCCEEDED(hr);
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    hr = buffer.As(&byteAccess);
    ASSERT_SUCCEEDED(hr);
    byte *bytes;
    hr = byteAccess->Buffer(&bytes);
    ASSERT_SUCCEEDED(hr);
    memcpy(bytes, pValue->value, length);
    ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
    GattWriteOption option = without_resp ? GattWriteOption_WriteWithoutResponse : GattWriteOption_WriteWithResponse;
    hr = characteristic->WriteValueWithOptionAsync(buffer.Get(), option, &writeOp);
    ASSERT_SUCCEEDED(hr);

    hr = writeOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>([this](IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            ods("Characteristic write operation failed %d", status);
            return FALSE;
        }
        GattCommunicationStatus result;
        HRESULT hr;
        hr = op->GetResults(&result);
        if (hr == E_BLUETOOTH_ATT_INVALID_ATTRIBUTE_VALUE_LENGTH) {
            ods("Characteristic write operation was tried with invalid value length");
            return FALSE;
        }

        if (result != GattCommunicationStatus_Success) {
            return FALSE;
        }

        if(m_bDataWritePending)
        {
            ods("Set event m_bDataWritePending");
            SetEvent(m_hEvent);
        }

        return TRUE;
    }).Get());

    ASSERT_SUCCEEDED(hr);
    return hr == S_OK;
}

BOOL CBtWin10Interface::GetDescriptorValue(USHORT *Value)
{
    HRESULT hr = E_FAIL;

    return hr == S_OK;
}

BOOL CBtWin10Interface::SetDescriptorValue(USHORT Value)
{
    HRESULT hr = E_FAIL;
    // register for notifications and indications with the status
    BTW_GATT_VALUE gatt_value;
    gatt_value.len = 2;
    gatt_value.value[0] = 3;
    gatt_value.value[1] = 0;
    hr = SetDescriptorValue(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &gatt_value);
    return hr == S_OK;
}

BOOL CBtWin10Interface::SendWsUpgradeCommand(BTW_GATT_VALUE *pValue)
{
    ods("+%S\n", __FUNCTIONW__);

    if (pValue->len == 0)
        return FALSE;

    BTW_GATT_VALUE encryptedValue;
    encryptedValue.len = mesh_client_ota_data_encrypt(NULL, pValue->value, pValue->len, encryptedValue.value, pValue->len + 17);
    return WriteCharacteristic(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint, FALSE, &encryptedValue);
}

BOOL CBtWin10Interface::SendWsUpgradeCommand(BYTE Command)
{
    BTW_GATT_VALUE value = { 1, Command };
    BTW_GATT_VALUE encryptedValue;
    encryptedValue.len = mesh_client_ota_data_encrypt(NULL, value.value, value.len, encryptedValue.value, value.len + 17);
    return SendWsUpgradeCommand( &value);
}

BOOL CBtWin10Interface::SendWsUpgradeCommand(BYTE Command, USHORT sParam)
{
    BTW_GATT_VALUE value = { 3, Command, (BYTE)(sParam & 0xff), (BYTE)((sParam >> 8) & 0xff) };
    BTW_GATT_VALUE encryptedValue;
    encryptedValue.len = mesh_client_ota_data_encrypt(NULL, value.value, value.len, encryptedValue.value, value.len + 17);
    return SendWsUpgradeCommand(&value);
}

BOOL CBtWin10Interface::SendWsUpgradeCommand(BYTE Command, ULONG lParam)
{
    BTW_GATT_VALUE value = { 5, Command, lParam & 0xff, (lParam >> 8) & 0xff, (lParam >> 16) & 0xff, (lParam >> 24) & 0xff };
    BTW_GATT_VALUE encryptedValue;
    encryptedValue.len = mesh_client_ota_data_encrypt(NULL, value.value, value.len, encryptedValue.value, value.len + 17);
    return SendWsUpgradeCommand(&value);
}

BOOL CBtWin10Interface::SendWsUpgradeData(BYTE *Data, DWORD len)
{
    ods("+%S\n", __FUNCTIONW__);
    BTW_GATT_VALUE value;
    HRESULT hr = E_FAIL;

    if (len > (sizeof(value.value) - 4))
    {
        ods("-%S data too long\n", __FUNCTIONW__);
        return (FALSE);
    }

    if (len == 0 || len + 17 >= GATT_MAX_ATTR_LEN)
    {
        ods("WsUpgrade bad length:%d\n", len);
        return FALSE;
    }
    value.len = mesh_client_ota_data_encrypt(NULL, Data, (uint16_t)len, value.value, (uint16_t)(len + 17));
#if 0
    if (len >= GATT_MAX_ATTR_LEN)
        return FALSE;

    memcpy(value.value, Data, len);

    value.len = (USHORT) len;
    if (value.len == 0)
        return FALSE;
#endif
    ResetEvent(m_hEvent);
    m_bDataWritePending = TRUE;

    hr =  WriteCharacteristic(&guidSvcWSUpgrade, &guidCharWSUpgradeData, FALSE, &value);

    if (WaitForSingleObject(m_hEvent, 3000) != WAIT_OBJECT_0)
        ods("Write event timeout occured");

    m_bDataWritePending = FALSE;
    return hr == S_OK;
}

BOOL CBtWin10Interface::CheckForOTAServices(const GUID * guid_ota_service, const GUID * guid_ota_sec_service)
{
    ComPtr<IGattDeviceService> otaService = getNativeService(guid_ota_service);
    ComPtr<IGattDeviceService> otaSecService = getNativeService(guid_ota_sec_service);

    m_bServiceFound = m_bSecure = FALSE;

    if (otaService)
        m_bServiceFound = TRUE;

    if (otaSecService)
    {
        m_bServiceFound = TRUE;
        m_bSecure = TRUE;
    }

    return m_bServiceFound;
}

BOOL CBtWin10Interface::CheckForProvProxyServices()
{
    ComPtr<IGattDeviceService> provService = getNativeService(&guidSvcMeshProvisioning);
    ComPtr<IGattDeviceService> proxyService = getNativeService(&guidSvcMeshProxy);

    return provService || proxyService;
}

UINT16 CBtWin10Interface::GetMTUSize()
{
    ComPtr<IGattDeviceService> provService = getNativeService(&guidSvcMeshProvisioning);
    ComPtr<IGattDeviceService> proxyService = getNativeService(&guidSvcMeshProxy);
    ComPtr<IGattDeviceService3> service3;

    HRESULT hr;

    if (provService)
        hr = provService.As(&service3);
    else if (proxyService)
        hr = proxyService.As(&service3);
    else
        return 0;
    ASSERT_SUCCEEDED(hr);

    ComPtr<IGattSession> gatt_session;
    hr = service3->get_Session(&gatt_session);
    ASSERT_SUCCEEDED(hr);

    UINT16 mtu = 0;
    hr = gatt_session->get_MaxPduSize(&mtu);

    return mtu;
}
