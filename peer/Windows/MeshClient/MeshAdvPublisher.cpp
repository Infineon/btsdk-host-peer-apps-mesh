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
* Mesh Publisher : Creates and starts BLE advertisements
* Advertisement data: Manufacturer Specific Data: Company ID + Proxy on Req + Network ID
*
*/

#include "stdafx.h"
#include "MeshClient.h"
#include "MeshAdvPublisher.h"
#include <Sddl.h>
#include "MeshClientDlg.h"



#ifdef __cplusplus
extern "C"
{
#endif
    wiced_bool_t mesh_advertising_start(uint16_t company_id, uint16_t service_id, uint8_t* data, uint8_t data_len);
    void mesh_advertising_stop(void);
#ifdef __cplusplus
}
#endif

#pragma comment(lib, "RuntimeObject.lib")

#ifndef BD_ADDR_LEN
#define BD_ADDR_LEN     6
typedef UINT8 BD_ADDR[BD_ADDR_LEN];
#endif

#define MESH_NETWORK_ID_LEN       8

static CMeshAdvPublisher* g_pCMeshAdvPublisher = NULL;

// #define STREAM_TO_BDADDR(a, p)   {register int _i; register UINT8 *pbda = (UINT8 *)a + BD_ADDR_LEN - 1; for (_i = 0; _i < BD_ADDR_LEN; _i++) *pbda-- = *p++;}

extern "C" void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data);

extern "C" CRITICAL_SECTION cs;
/*
static void BthAddrToBDA(BD_ADDR bda, ULONGLONG *btha)
{
    BYTE *p = (BYTE *)btha;
    STREAM_TO_BDADDR(bda, p);
}
*/
// callback function is called when publisher status changes
HRESULT CMeshAdvPublisher::OnAdvertisementPublisherStatusChanged(IBluetoothLEAdvertisementPublisher* publisher, IBluetoothLEAdvertisementPublisherStatusChangedEventArgs* args)
{
    HRESULT hr;

    if (m_bStop)
    {
        ods("LEPublisher Stopping...ignore ADV");
        return S_OK;
    }
    BluetoothError error;
    hr = args->get_Error(&error);

    BluetoothLEAdvertisementPublisherStatus status;
    hr = args->get_Status(&status);

    ods("OnAdvertisementPublisherStatusChanged error:%x  status:%x", error, status);

    return S_OK;
}

int CMeshAdvPublisher::InitializePublisher(uint16_t company_id, uint16_t service_id, uint8_t *data, uint8_t data_len)
{
    HRESULT hr = 0;

    ods("InitializePublisher service:%x data length:%d", service_id, data_len);

    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        ods("initialize failed: initialize:%x", initialize);
        return initialize;
    }

    // Get the activation factory for the IBluetoothLEAdvertisementPublisherFactory interface.
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementPublisher).Get(), &bleAdvPublisherFactory);
    if (FAILED(hr))
    {
        ods("GetActivationFactory failed: hr:%x", hr);
        return hr;
    }

    Wrappers::HStringReference class_id_advertisement(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisement);
    hr = RoActivateInstance(class_id_advertisement.Get(), reinterpret_cast<IInspectable**>(bleAdvertisement.GetAddressOf()));

    if (FAILED(hr))
    {
        ods("RoActivateInstance-> bleAdvertisement failed: hr:%x", hr);
        return hr;
    }

    //===========
    // Get Advertisement Data Sections
    ComPtr <ABI::Windows::Foundation::Collections::IVector<ABI::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataSection*>> vecData;
    hr = bleAdvertisement->get_DataSections(&vecData);
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

    ComPtr<ABI::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementDataSection> ds;

    Wrappers::HStringReference class_id_datasection(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementDataSection);
    hr = RoActivateInstance(class_id_datasection.Get(), reinterpret_cast<IInspectable**>(ds.GetAddressOf()));

    //try putting the datatype into the data section here
    hr = ds->put_DataType(0xFF);// Manufacturer Specific Information (0xFF)
    if (FAILED(hr))
    {
        ods("ds->put_DataType(0xFF) failed. hr:%x", hr);
        return S_OK;
    }

    //--> Begin composing manufacturer specific data
    byte manufacturer_data[4] = { 0 };
    ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(), &bufferFactory);
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    const int length = 2 + 2 + data_len; // Company ID (2 bytes) + Service ID(2 bytes) + Service Data(data_len)
    hr = bufferFactory->Create(length, &buffer);
    hr = buffer->put_Length(length);
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    hr = buffer.As(&byteAccess);
    byte *bytes;
    hr = byteAccess->Buffer(&bytes);

    manufacturer_data[0] = company_id & 0xff;
    manufacturer_data[1] = (company_id >> 8) & 0xff;
    manufacturer_data[2] = service_id & 0xff;
    manufacturer_data[3] = (service_id >> 8) & 0xff;
    memcpy(bytes, manufacturer_data, 4);

    memcpy(bytes + 4, data, data_len);
    //--> End composing manufacturer specific data
    // Put the data in the data section
    hr = ds->put_Data(buffer.Get());
    // Append the data section to the vector
    hr = vecData->Append(ds.Get());
    //===========
    // Create publisher and initialize with the advertisement
    hr = bleAdvPublisherFactory->Create(bleAdvertisement.Get(), &blePublisher);
    if (FAILED(hr))
    {
        ods("bleAdvPublisherFactory->Create failed: hr:%x", hr);
        return hr;
    }

    if (blePublisher == NULL)
    {
        ods("blePublisher is NULL. hr:%x", hr);
        return 1;
    }
    handler = Callback<ITypedEventHandler<BluetoothLEAdvertisementPublisher*, BluetoothLEAdvertisementPublisherStatusChangedEventArgs*>>
        (this, &CMeshAdvPublisher::OnAdvertisementPublisherStatusChanged);

    hr = blePublisher->add_StatusChanged(handler.Get(), &publisherToken);
    if (FAILED(hr))
    {
        ods("blePublisher->add_Received failed: hr:%x", hr);
        return hr;
    }

    ods("InitializePublisher - return");
    return 0;
}

int CMeshAdvPublisher::StartLEAdvertisementPublisher()
{
    ods("StartLEAdvertisementPublisher");
    HRESULT hr = -1;

    m_bStop = FALSE;

    if (blePublisher)
    {
        hr = blePublisher->Start();
        if (FAILED(hr))
        {
            ods("blePublisher->Start failed: hr:%x", hr);
            return hr;
        }
    }
    return 0;
}

int CMeshAdvPublisher::StopLEAdvertisementPublisher()
{
    ods("StopLEAdvertisementPublisher");
    HRESULT hr = -1;

    m_bStop = TRUE;

    if (blePublisher)
        hr = blePublisher->Stop();

    if (FAILED(hr))
    {
        ods("blePublisher->Stop failed: hr:%x", hr);
        return hr;
    }
    return 0;
}

CMeshAdvPublisher::CMeshAdvPublisher()
{
    m_bStop = FALSE;
}

CMeshAdvPublisher::~CMeshAdvPublisher()
{
}

wiced_bool_t mesh_advertising_start(uint16_t company_id, uint16_t service_id, uint8_t *data, uint8_t data_len)
{
    EnterCriticalSection(&cs);
    ods("mesh_advertising_start: g_pCMeshAdvPublisher:%d", g_pCMeshAdvPublisher);
    if (g_pCMeshAdvPublisher)
    {
        g_pCMeshAdvPublisher->StopLEAdvertisementPublisher();
        delete g_pCMeshAdvPublisher;
    }

    g_pCMeshAdvPublisher = new CMeshAdvPublisher();
    g_pCMeshAdvPublisher->InitializePublisher(company_id, service_id, data, data_len);
    g_pCMeshAdvPublisher->StartLEAdvertisementPublisher();
    LeaveCriticalSection(&cs);
    return WICED_TRUE;
}

void mesh_advertising_stop(void)
{
    EnterCriticalSection(&cs);
    ods("mesh_advertising_stop: g_pCMeshAdvPublisher:%d", g_pCMeshAdvPublisher);
    if (g_pCMeshAdvPublisher)
    {
        g_pCMeshAdvPublisher->StopLEAdvertisementPublisher();
        delete g_pCMeshAdvPublisher;
        g_pCMeshAdvPublisher = NULL;
    }
    LeaveCriticalSection(&cs);
}
