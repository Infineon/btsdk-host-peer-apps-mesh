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
* Mesh Scanner : Scans for advertisements and identifies LE devices
*
*/

#include "stdafx.h"
#include "MeshClient.h"
#include "MeshScanner.h"
#include <Sddl.h>
#include "MeshClientDlg.h"

#pragma comment(lib, "RuntimeObject.lib")

#ifndef BD_ADDR_LEN
#define BD_ADDR_LEN     6
typedef UINT8 BD_ADDR[BD_ADDR_LEN];
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    wiced_bool_t mesh_adv_scan_start(void);
    void wiced_bt_ble_set_scan_mode(uint8_t is_active);
    void mesh_adv_scan_stop(void);
#ifdef __cplusplus
}
#endif


static CMeshScanner* g_pCMeshScanner = NULL;

//#define STREAM_TO_BDADDR(a, p)   {register int _i; register UINT8 *pbda = (UINT8 *)a + BD_ADDR_LEN - 1; for (_i = 0; _i < BD_ADDR_LEN; _i++) *pbda-- = *p++;}

extern "C" void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data);

extern "C" CRITICAL_SECTION cs;

static void BthAddrToBDA(BD_ADDR bda, ULONGLONG *btha)
{
    BYTE *p = (BYTE *)btha;
    STREAM_TO_BDADDR(bda, p);
}

// callback function is called for each receved advert
HRESULT CMeshScanner::OnAdvertisementReceived(IBluetoothLEAdvertisementWatcher* watcher, IBluetoothLEAdvertisementReceivedEventArgs* args)
{
    HRESULT hr;
    BYTE datatype;
    UINT32 length;
    UINT64 address;

    static BD_ADDR bda;
    static BluetoothLEAdvertisementType  type;
    static HString sName;
    static BYTE raw_advert[62];
    static BYTE raw_advert_len = 0;
    char name[31];
    if (m_bStop)
    {
        ods("LEWatcher Stopping...ignore ADV");
        return S_OK;
    }
    hr = args->get_AdvertisementType(&type);
    ods("nAdvertisementReceived type:%x", type);

    if ((type != BluetoothLEAdvertisementType_ConnectableUndirected) && (type != BluetoothLEAdvertisementType_ScanResponse))
    {
        return S_OK;
    }
    INT16 rssi;
    args->get_RawSignalStrengthInDBm(&rssi);
    hr = args->get_BluetoothAddress(&address);
    if (FAILED(hr))
    {
        ods("OnAdvertisementReceived address not retrieved");
        return S_OK;
    }
    ComPtr<IBluetoothLEAdvertisement> bleAdvert;
    hr = args->get_Advertisement(&bleAdvert);
    if (FAILED(hr))
    {
        ods("get_Advertisement failed. hr:%x", hr);
        return S_OK;
    }
    if ((type == BluetoothLEAdvertisementType_ScanResponse) && (raw_advert_len != 0))
    {
        // Get Name of the device
        hr = bleAdvert->get_LocalName(sName.GetAddressOf());
        ods("Local Name: %S hr:%x", sName.GetRawBuffer(nullptr), hr);

        WideCharToMultiByte(CP_UTF8, 0, sName.GetRawBuffer(nullptr), -1, name, 31, NULL, NULL);

        raw_advert[raw_advert_len++] = (BYTE)strlen(name) + 1;
        raw_advert[raw_advert_len++] = 0x09;    // BTM_BLE_ADVERT_TYPE_NAME_COMPLETE

        for (BYTE i = 0; i < (BYTE)strlen(name); i++)
            raw_advert[raw_advert_len++] = name[i];

        raw_advert[raw_advert_len++] = 0;

        char buff[256] = { 0 };
        for (UINT32 i = 0; i < raw_advert_len; ++i)
            sprintf_s(buff + 3 * i, sizeof(buff) - 3 * i, "%02x ", raw_advert[i]);

        ods("Adv:%s from bda:%02x%02x%02x%02x%02x%02x", buff, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

        mesh_client_app_adv_report_t *p_adv_report = (mesh_client_app_adv_report_t *) malloc(sizeof(mesh_client_app_adv_report_t));
        if (p_adv_report)
        {
            CMeshClientDlg *pDlg = (CMeshClientDlg *)theApp.m_pMainWnd;
            memset(p_adv_report, 0, sizeof(mesh_client_app_adv_report_t));
            p_adv_report->addr_type = 0;
            p_adv_report->rssi = (int8_t) rssi;
            memcpy(p_adv_report->bda, bda, BD_ADDR_LEN);
            memcpy(p_adv_report->adv_data, raw_advert, sizeof(raw_advert));
            pDlg->PostMessage(WM_MESH_DEVICE_ADV_REPORT, (WPARAM)0, (LPARAM)p_adv_report);
        }
        //mesh_client_advert_report(bda, 0, (int8_t)rssi, raw_advert);

        raw_advert_len = 0;
    }
    else if (type == BluetoothLEAdvertisementType_ConnectableUndirected)
    {
        // if previous advert came with no scan response, send it down with previously received bda.
        if (raw_advert_len != 0)
        {
            raw_advert[raw_advert_len++] = 0;
            char buff[256] = { 0 };
            for (UINT32 i = 0; i < raw_advert_len; ++i)
                sprintf_s(buff + 3 * i, sizeof(buff) - 3 * i, "%02x ", raw_advert[i]);

            ods("Adv:%s from bda:%02x%02x%02x%02x%02x%02x", buff, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
           // mesh_client_advert_report(bda, 0, (int8_t)rssi, raw_advert);
            mesh_client_app_adv_report_t *p_adv_report = (mesh_client_app_adv_report_t *)malloc(sizeof(mesh_client_app_adv_report_t));
            if (p_adv_report)
            {
                CMeshClientDlg *pDlg = (CMeshClientDlg *)theApp.m_pMainWnd;
                memset(p_adv_report, 0, sizeof(mesh_client_app_adv_report_t));
                p_adv_report->addr_type = 0;
                p_adv_report->rssi = (int8_t)rssi;
                memcpy(p_adv_report->bda, bda, BD_ADDR_LEN);
                memcpy(p_adv_report->adv_data, raw_advert, sizeof(raw_advert));
                pDlg->PostMessage(WM_MESH_DEVICE_ADV_REPORT, (WPARAM)0, (LPARAM)p_adv_report);
            }

            raw_advert_len = 0;
        }
        // Get Services
        ComPtr<ABI::Windows::Foundation::Collections::IVector<GUID>> vecGuid;

        hr = bleAdvert->get_ServiceUuids(&vecGuid);

        if (FAILED(hr))
        {
            ods("get_ServiceUuids failed. hr:%x", hr);
            return S_OK;
        }
        UINT guidCount = 0;
        GUID guid = { 0 };
        hr = vecGuid->get_Size(&guidCount);

        const GUID guidProvService = { 0x1827, 0x00, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
        const GUID guidProxyService = { 0x1828, 0x00, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
        BOOL ProvServicePresent = FALSE;
        BOOL ProxyServicePresent = FALSE;

        if (SUCCEEDED(hr))
        {
            for (int i = 0; i < (int)guidCount; ++i)
            {
                hr = vecGuid->GetAt(i, &guid);
                if (SUCCEEDED(hr))
                {
                    WCHAR szService[80] = { 0 };
                    UuidToString(szService, 80, &guid);
                    ods("index:%d  GUID:%S", i, szService);
                    if ((guid.Data2 == guidProvService.Data2) && (guid.Data3 == guidProvService.Data3) && (memcmp(guid.Data4, guidProvService.Data4, sizeof(guidProvService.Data4)) == 0))
                    {
                        ProvServicePresent = (guid.Data1 == guidProvService.Data1);
                        ProxyServicePresent = (guid.Data1 == guidProxyService.Data1);
                    }
                }
                else
                {
                    ods("vecGuid->GetAt(%d) failed. hr:%x", i, hr);
                }
            }
        }
        if (!ProvServicePresent && !ProxyServicePresent)
            return S_OK;

        // Get Advertisement Data
        ComPtr <ABI::Windows::Foundation::Collections::IVector<ABI::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataSection*>> vecData;
        hr = bleAdvert->get_DataSections(&vecData);
        if (FAILED(hr))
        {
            ods("get_DataSections failed. hr:%x", hr);
            return S_OK;
        }
        UINT count = 0;
        hr = vecData->get_Size(&count);
        if (FAILED(hr))
        {
            ods("vecData->get_Size failed. hr:%x", hr);
            return S_OK;
        }

        for (UINT i = 0; i < count; ++i)
        {
            ComPtr<ABI::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementDataSection> ds;

            hr = vecData->GetAt(i, &ds);
            if (FAILED(hr))
            {
                ods("vecData->GetAt(%d) failed. hr:%x", i, hr);
                continue;
            }

            ComPtr<ABI::Windows::Storage::Streams::IBuffer> ibuf;
            datatype = 0;
            hr = ds->get_DataType(&datatype);
            if (FAILED(hr))
            {
                ods("ds->get_DataType failed. i:%d hr:%x", i, hr);
                continue;
            }
            // ods("Data Type:%d", datatype);

            hr = ds->get_Data(&ibuf);
            if (FAILED(hr))
            {
                ods("ds->get_Data failed. i:%d hr:%x", i, hr);
                continue;
            }

            Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> pBufferByteAccess;
            ibuf.As(&pBufferByteAccess);

            // Get pointer to pixel bytes
            byte* pdatabuf = nullptr;
            pBufferByteAccess->Buffer(&pdatabuf);

            length = 0;
            hr = ibuf->get_Length(&length);
            if (FAILED(hr))
            {
                ods("ibuf->get_Length failed. i:%d hr:%x", i, hr);
                continue;
            }

            BthAddrToBDA(bda, &address);

            raw_advert[raw_advert_len++] = length + 1;
            raw_advert[raw_advert_len++] = datatype;

            for (UINT32 i = 0; i < length; ++i)
                raw_advert[raw_advert_len++] = *(pdatabuf + i);

        }
        // If doing active scan, wait for scan response, otherwise ship the adv to the stack.
        if (!m_bActiveScan)
        {
            raw_advert[raw_advert_len++] = 0;
            char buff[256] = { 0 };
            for (UINT32 i = 0; i < raw_advert_len; ++i)
                sprintf_s(buff + 3 * i, sizeof(buff) - 3 * i, "%02x ", raw_advert[i]);

            ods("Adv:%s from bda:%02x%02x%02x%02x%02x%02x", buff, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
           // mesh_client_advert_report(bda, 0, (int8_t)rssi, raw_advert);
            mesh_client_app_adv_report_t *p_adv_report = (mesh_client_app_adv_report_t *)malloc(sizeof(mesh_client_app_adv_report_t));
            if (p_adv_report)
            {
                CMeshClientDlg *pDlg = (CMeshClientDlg *)theApp.m_pMainWnd;
                memset(p_adv_report, 0, sizeof(mesh_client_app_adv_report_t));
                p_adv_report->addr_type = 0;
                p_adv_report->rssi = (int8_t)rssi;
                memcpy(p_adv_report->bda, bda, BD_ADDR_LEN);
                memcpy(p_adv_report->adv_data, raw_advert, sizeof(raw_advert));
                pDlg->PostMessage(WM_MESH_DEVICE_ADV_REPORT, (WPARAM)0, (LPARAM)p_adv_report);
            }
            // mesh_client_advert_report(bda, 0, (int8_t)rssi, raw_advert);
            raw_advert_len = 0;
        }
    }
    return S_OK;
}

int CMeshScanner::InitializeWatcher()
{
    HRESULT hr = 0;

    ods("InitializeWatcher");

    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        ods("initialize failed: initialize:%x", initialize);
        return initialize;
    }

    // Get the activation factory for the IBluetoothLEAdvertisementWatcherFactory interface.
     hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementWatcher).Get(), &bleAdvWatcherFactory);
    if (FAILED(hr))
    {
        ods("GetActivationFactory failed: hr:%x", hr);
        return hr;
    }

    Wrappers::HStringReference class_id_filter2(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementFilter);
    hr = RoActivateInstance(class_id_filter2.Get(), reinterpret_cast<IInspectable**>(bleFilter.GetAddressOf()));

    hr = bleAdvWatcherFactory->Create(bleFilter.Get(), &bleWatcher);
    if (FAILED(hr))
    {
        ods("bleAdvWatcherFactory->Create failed: hr:%x", hr);
        return hr;
    }

    if (bleWatcher == NULL)
    {
        ods("bleWatcher is NULL. hr:%x", hr);
        return 1;
    }
    handler = Callback<ITypedEventHandler<BluetoothLEAdvertisementWatcher*, BluetoothLEAdvertisementReceivedEventArgs*>>
        (this, &CMeshScanner::OnAdvertisementReceived);

    hr = bleWatcher->add_Received(handler.Get(), &watcherToken);
    if (FAILED(hr))
    {
        ods("bleWatcher->add_Received failed: hr:%x", hr);
        return hr;
    }

    ods("InitializeWatcher - return");
    return 0;
}

int CMeshScanner::SetScanMode(BYTE isActiveScan)
{
    HRESULT hr = 0;

    ods("StartLEAdvertisementWatcher active:%d", isActiveScan);
    m_bActiveScan = isActiveScan;

    if (bleWatcher)
    {
        hr = bleWatcher->put_ScanningMode(isActiveScan ? BluetoothLEScanningMode_Active : BluetoothLEScanningMode_Passive);
        if (FAILED(hr))
        {
            ods("bleWatcher-put_ScanningMode failed: hr:%x", hr);
        }
    }
    return hr;
}

int CMeshScanner::StartLEAdvertisementWatcher()
{
    ods("StartLEAdvertisementWatcher");
    HRESULT hr = -1;

    m_bStop = FALSE;

    if (bleWatcher)
    {
        hr = bleWatcher->Start();
        if (FAILED(hr))
        {
            ods("bleWatcher->Start failed: hr:%x", hr);
            return hr;
        }
    }
    return 0;
}

int CMeshScanner::StopLEAdvertisementWatcher()
{
    ods("StopLEAdvertisementWatcher");
    HRESULT hr = -1;

    m_bStop = TRUE;

    if (bleWatcher)
        hr = bleWatcher->Stop();

    if (FAILED(hr))
    {
        ods("bleWatcher->Stop failed: hr:%x", hr);
        return hr;
    }
    return 0;
}

CMeshScanner::CMeshScanner()
{
    m_bStop = FALSE;
    m_bActiveScan = FALSE;
}

CMeshScanner::~CMeshScanner()
{
}

void wiced_bt_ble_set_scan_mode(uint8_t is_active)
{
    EnterCriticalSection(&cs);
    if (g_pCMeshScanner)
        g_pCMeshScanner->SetScanMode(is_active);
    LeaveCriticalSection(&cs);
}

wiced_bool_t mesh_adv_scan_start(void)
{
    EnterCriticalSection(&cs);
    ods("mesh_adv_scan_start: g_pCMeshScanner:%d", g_pCMeshScanner);
    if (!g_pCMeshScanner)
    {
        LeaveCriticalSection(&cs);
        return WICED_FALSE;
    }

    CMeshClientDlg *pDlg = (CMeshClientDlg *)theApp.m_pMainWnd;
    pDlg->UpdateScanState(TRUE);
    g_pCMeshScanner->StartLEAdvertisementWatcher();
    LeaveCriticalSection(&cs);
    return WICED_TRUE;
}

void mesh_adv_scan_stop(void)
{
    EnterCriticalSection(&cs);
    ods("mesh_adv_scan_stop: g_pCMeshScanner:%d", g_pCMeshScanner);
    if (g_pCMeshScanner)
    {
        CMeshClientDlg *pDlg = (CMeshClientDlg *)theApp.m_pMainWnd;
        pDlg->UpdateScanState(FALSE);
        g_pCMeshScanner->StopLEAdvertisementWatcher();
    }
    LeaveCriticalSection(&cs);
}

wiced_bool_t mesh_adv_scanner_open()
{
    EnterCriticalSection(&cs);
    ods("mesh_adv_scan_start: g_pCMeshScanner:%d", g_pCMeshScanner);
    if (g_pCMeshScanner)
    {
        LeaveCriticalSection(&cs);
        return WICED_FALSE;
    }
    g_pCMeshScanner = new CMeshScanner();
    g_pCMeshScanner->InitializeWatcher();
    LeaveCriticalSection(&cs);
    return WICED_TRUE;
}

void mesh_adv_scanner_close(void)
{
    EnterCriticalSection(&cs);
    ods("mesh_adv_scan_close: g_pCMeshScanner:%d", g_pCMeshScanner);
    if (g_pCMeshScanner)
    {
        CMeshScanner* pCMeshScanner = g_pCMeshScanner;
        g_pCMeshScanner = NULL;
        pCMeshScanner->StopLEAdvertisementWatcher();
        delete pCMeshScanner;
    }
    LeaveCriticalSection(&cs);
}
