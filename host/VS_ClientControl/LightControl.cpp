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
// LightControl.cpp : implementation file
//

#include "stdafx.h"
#include "LightControl.h"
#include "ClientControlDlg.h"
#include "SensorConfig.h"
#include "LightLcConfig.h"
#include "afxdialogex.h"
#include "resource.h"
#include "wiced_mesh_client.h"
#ifdef MESH_DFU_ENABLED
#include "wiced_mesh_client_dfu.h"
#endif
#include "hci_control_api.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_provision.h"
#include "wiced_bt_mesh_db.h"
#include "MeshPerformance.h"

int provision_test = 0;
DWORD provision_test_scan_unprovisioned_time;
DWORD provision_test_connect_unprovisioned_time;
DWORD provision_test_provision_start_time;
DWORD provision_test_connect_provisioned_time;
DWORD provision_test_config_start_time;
DWORD provision_test_reset_time;
BOOL  provision_test_bScanning;

extern "C" uint8_t * wiced_bt_mesh_format_hci_header(uint16_t dst, uint16_t app_key_idx, uint8_t element_idx, uint8_t reliable, uint8_t send_segmented, uint8_t ttl, uint8_t retransmit_count, uint8_t retransmit_interval, uint8_t reply_timeout, uint16_t num_in_group, uint16_t * group_list, uint8_t * p_buffer, uint16_t len);
extern "C" wiced_bt_mesh_event_t * wiced_bt_mesh_event_from_hci_header(uint8_t * *p_buffer, uint16_t * len);
extern "C" uint16_t get_device_addr(const char *p_dev_name);
extern "C" uint16_t mesh_client_get_unicast_addr();

extern ClsStopWatch thesw;

// #define MESH_AUTOMATION_ENABLED TRUE
//#if defined(MESH_AUTOMATION_ENABLED) && (MESH_AUTOMATION_ENABLED == TRUE)
//#include "mesh_client_script.h"
//#endif

extern BOOL SendMessageToUDPServer(char* p_msg, UINT len);
#define WM_MESH_ADD_VENDOR_MODEL    (WM_USER + 111)
#define LOCAL_DEVICE_TTL                    63

static WCHAR *szDeviceType[] =
{
    L"Unknown",
    L"On/Off Client",
    L"Level Client",
    L"On/Off Server",
    L"Level Server",
    L"Dimmable Light",
    L"Power Outlet",
    L"HSL Light",
    L"CTL Light",
    L"XYL Light",
    L"Vendor Specific",
};

typedef struct
{
    int is_gatt_proxy;
    int is_friend;
    int is_relay;
    int send_net_beacon;
    int relay_xmit_count;
    int relay_xmit_interval;
    int default_ttl;
    int net_xmit_count;
    int net_xmit_interval;
    int publish_credential_flag;       ///< Value of the Friendship Credential Flag
    int publish_ttl;                   ///< Default TTL value for the outgoing messages
    int publish_retransmit_count;      ///< Number of retransmissions for each published message
    int publish_retransmit_interval;   ///< Interval in milliseconds between retransmissions
} device_config_params_t;

device_config_params_t DeviceConfig = { 1, 1, 1, 1, 3, 100, 8, 3, 100, 0, 8, 0, 500 };

void network_opened(uint8_t status);
/*extern "C" */ void unprovisioned_device(uint8_t *uuid, uint16_t oob, uint8_t *name_len, uint8_t name);
/*extern "C" */ void link_status(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt);
/*extern "C" */ void node_connect_status(uint8_t is_connected, char *p_device_name);
/*extern "C" */ void provision_status(uint8_t status, uint8_t *p_uuid);
/*extern "C" */ void database_changed(char *mesh_name);
/*extern "C" */ void component_info_status(uint8_t status, char *component_name, char *component_info);
/*extern "C" */ void onoff_status(const char *device_name, uint8_t present, uint8_t target, uint32_t remaining_time);
/*extern "C" */ void level_status(const char *device_name, int16_t present, int16_t target, uint32_t remaining_time);
/*extern "C" */ void lightness_status(const char *device_name, uint16_t present, uint16_t target, uint32_t remaining_time);
/*extern "C" */ void hsl_status(const char *device_name, uint16_t lightness, uint16_t hue, uint16_t saturation, uint32_t remaining_time);
/*extern "C" */ void ctl_status(const char *device_name, uint16_t present_lightness, uint16_t present_temperature, uint16_t target_lightness, uint16_t target_temperature, uint32_t remaining_time);
/*extern "C" */ void xyl_status(const char* device_name, uint16_t present_lightness, uint16_t x, uint16_t y, uint32_t remaining_time);
/*extern "C" */ void sensor_status(const char *device_name, int property_id, uint8_t value_len, uint8_t *value);
/*extern "C" */ void vendor_specific_data(const char *device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t ttl, uint8_t *p_data, uint16_t data_len);
/*extern "C" */ void fw_distribution_status(uint8_t state, uint8_t *p_data, uint32_t data_length);

#ifdef MESH_DFU_ENABLED
WCHAR *dfuMethods[] = {
    L"Proxy DFU to all",
    L"App DFU to all",
};
#endif

#define DISTRIBUTION_STATUS_TIMEOUT     10

extern wiced_bool_t mesh_adv_scanner_open();
extern void mesh_adv_scanner_close(void);
extern "C" void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data);

extern void lc_property_status(const char* device_name, int property_id, int value);
extern void lc_mode_status(const char* device_name, int mode);
extern void lc_occupancy_mode_status(const char* device_name, int mode);

char provisioner_uuid[50];

mesh_client_init_t mesh_client_init_callbacks =
{
    unprovisioned_device,
    provision_status,
    link_status,
    node_connect_status,
    database_changed,
    onoff_status,
    level_status,
    lightness_status,
    hsl_status,
    ctl_status,
    sensor_status,
    vendor_specific_data,
    xyl_status,
    lc_mode_status,
    lc_occupancy_mode_status,
    lc_property_status,
};


extern void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length);
extern void Log(WCHAR *fmt, ...);
extern int FwDownload(char *sHCDFileName);
void FwDownloadProcessEvent(LPBYTE p_data, DWORD len);
extern "C" void wiced_hci_process_data(uint16_t opcode, uint8_t *p_buffer, uint16_t len);

// CLightControl dialog

IMPLEMENT_DYNAMIC(CLightControl, CPropertyPage)

// Fills buffer by the full path to the file in the subfolder Infineon\\MeshClient of the local appdata folder.
// It creates a subfolder Infineon\\MeshClient in the local appdata folder if it doesn't exist
// Returns 1 on success. Returns 0 on error.
extern "C" int get_file_path_in_appdata_folder(const char* file, char* buffer, unsigned long bufferLen)
{
    int res = 1;
    PWSTR path = NULL;
    if (S_OK != SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path))
        res = 0;
    else if (0 == WideCharToMultiByte(CP_UTF8, 0, path, -1, buffer, bufferLen, NULL, FALSE))
        res = 0;
    else
    {
        strcat_s(buffer, bufferLen, "\\Infineon\\MeshClient\\");
        int iRes = SHCreateDirectoryExA(NULL, buffer, NULL);
        if (iRes != ERROR_SUCCESS && iRes != ERROR_ALREADY_EXISTS && iRes != ERROR_FILE_EXISTS)
            res = 0;
        else if (file != NULL && file[0] != 0)
            strcat_s(buffer, bufferLen, file);
    }
    // The calling process is responsible for freeing this resource (path) whether SHGetKnownFolderPath succeeds or not
    // If the parameter is NULL, the function CoTaskMemFree has no effect.
    CoTaskMemFree(path);
    return res;
}

// Writes content of the DeviceConfig into the file NetParameters.bin in the subfolder Infineon\\MeshClient of the local appdata folder.
void WriteDeviceConfig()
{
    char path[MAX_PATH];
    if (get_file_path_in_appdata_folder("NetParameters.bin", path, sizeof(path)))
    {
        FILE* fp = fopen(path, "wb");
        if (fp)
        {
            fwrite(&DeviceConfig, 1, sizeof(DeviceConfig), fp);
            fclose(fp);
        }
    }
}

CLightControl::CLightControl()
	: CPropertyPage(IDD_LIGHT_CONTROL)
{
    m_trace = NULL;
    m_szCurrentGroup[0] = 0;
    m_fw_download_active = FALSE;
    m_hHciEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_bScanning = FALSE;
    m_bConnected = FALSE;
    m_bConnecting = FALSE;
    m_received_evt[0] = 0;
    m_received_evt_len = 0;
    m_pPatch = 0;
    m_dwPatchSize = 0;
    m_event = 0;

    char path[MAX_PATH];
    if (get_file_path_in_appdata_folder("NetParameters.bin", path, sizeof(path)))
    {
        FILE* fp = fopen(path, "rb");
        if (fp)
        {
            fread(&DeviceConfig, 1, sizeof(DeviceConfig), fp);
            fclose(fp);
        }
    }
}

CLightControl::~CLightControl()
{
}

void CLightControl::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS, m_Progress);
}

BEGIN_MESSAGE_MAP(CLightControl, CPropertyPage)
    ON_WM_CLOSE()
    ON_CBN_SELCHANGE(IDC_COM_PORT, &CLightControl::OnCbnSelchangeComPort)
    ON_CBN_SELCHANGE(IDC_COM_BAUD, &CLightControl::OnCbnSelchangeComPort)
    ON_BN_CLICKED(IDC_CLEAR_TRACE, &CLightControl::OnBnClickedClearTrace)
    ON_BN_CLICKED(IDC_SCAN_UNPROVISIONED, &CLightControl::OnBnClickedScanUnprovisioned)
    ON_BN_CLICKED(IDC_DOWNLOAD, &CLightControl::OnBnClickedDownload)
    ON_BN_CLICKED(IDC_BROWSE, &CLightControl::OnBnClickedBrowse)
    ON_BN_CLICKED(IDC_PROVISION, &CLightControl::OnBnClickedProvision)
    ON_BN_CLICKED(IDC_NETWORK_CREATE, &CLightControl::OnBnClickedNetworkCreate)
    ON_BN_CLICKED(IDC_NETWORK_DELETE, &CLightControl::OnBnClickedNetworkDelete)
    ON_BN_CLICKED(IDC_NETWORK_OPEN, &CLightControl::OnBnClickedNetworkOpen)
    ON_BN_CLICKED(IDC_NETWORK_CLOSE, &CLightControl::OnBnClickedNetworkClose)
    ON_BN_CLICKED(IDC_GROUP_CREATE, &CLightControl::OnBnClickedGroupCreate)
    ON_BN_CLICKED(IDC_GROUP_DELETE, &CLightControl::OnBnClickedGroupDelete)
    ON_BN_CLICKED(IDC_NODE_RESET, &CLightControl::OnBnClickedNodeReset)
    ON_CBN_SELCHANGE(IDC_NETWORK, &CLightControl::OnSelchangeNetwork)
    ON_BN_CLICKED(IDC_CONFIGURE_NEW_NAME, &CLightControl::OnBnClickedConfigureNewName)
    ON_CBN_SELCHANGE(IDC_CURRENT_GROUP, &CLightControl::OnSelchangeCurrentGroup)
    ON_CBN_SELCHANGE(IDC_CONFIGURE_MOVE_DEVICE, &CLightControl::OnCbnSelchangeConfigureMoveDevice)
    ON_BN_CLICKED(IDC_CONFIGURE_MOVE, &CLightControl::OnBnClickedMoveToGroup)
    ON_BN_CLICKED(IDC_CONFIGURE_PUB, &CLightControl::OnBnClickedConfigurePub)
    ON_BN_CLICKED(IDC_CONNECTDISCONNECT, &CLightControl::OnBnClickedConnectdisconnect)
    ON_BN_CLICKED(IDC_IDENTIFY, &CLightControl::OnBnClickedIdentify)
    ON_BN_CLICKED(IDC_ON_OFF_GET, &CLightControl::OnBnClickedOnOffGet)
    ON_BN_CLICKED(IDC_ON_OFF_SET, &CLightControl::OnBnClickedOnOffSet)
    ON_BN_CLICKED(IDC_LEVEL_GET, &CLightControl::OnBnClickedLevelGet)
    ON_BN_CLICKED(IDC_LEVEL_SET, &CLightControl::OnBnClickedLevelSet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_GET, &CLightControl::OnBnClickedLightHslGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_SET, &CLightControl::OnBnClickedLightHslSet)
    ON_BN_CLICKED(IDC_VS_DATA, &CLightControl::OnBnClickedVsData)
    ON_BN_CLICKED(IDC_LIGHT_CTL_GET, &CLightControl::OnBnClickedLightCtlGet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_SET, &CLightControl::OnBnClickedLightCtlSet)
    ON_BN_CLICKED(IDC_LIGHTNESS_GET, &CLightControl::OnBnClickedLightnessGet)
    ON_BN_CLICKED(IDC_LIGHTNESS_SET, &CLightControl::OnBnClickedLightnessSet)
    ON_CBN_SELCHANGE(IDC_CONFIGURE_CONTROL_DEVICE, &CLightControl::OnCbnSelchangeConfigureControlDevice)
    ON_BN_CLICKED(IDC_NETWORK_IMPORT, &CLightControl::OnBnClickedNetworkImport)
    ON_BN_CLICKED(IDC_NETWORK_EXPORT, &CLightControl::OnBnClickedNetworkExport)
    ON_BN_CLICKED(IDC_BROWSE_DFU, &CLightControl::OnBnClickedBrowseDfu)
    ON_BN_CLICKED(IDC_DFU_START_STOP, &CLightControl::OnBnClickedDfuStartstop)
    ON_BN_CLICKED(IDC_DFU_PAUSE_RESUME, &CLightControl::OnBnClickedDfuPauseresume)
    ON_BN_CLICKED(IDC_DFU_GET_STATUS, &CLightControl::OnBnClickedDfuGetStatus)
    ON_BN_CLICKED(IDC_SENSOR_GET, &CLightControl::OnBnClickedSensorGet)
    ON_CBN_SELCHANGE(IDC_CONTROL_DEVICE, &CLightControl::OnCbnSelchangeControlDevice)
    ON_BN_CLICKED(IDC_SENSOR_CONFIGURE, &CLightControl::OnBnClickedSensorConfigure)
    ON_BN_CLICKED(IDC_GET_COMPONENT_INFO, &CLightControl::OnBnClickedGetComponentInfo)
    ON_BN_CLICKED(IDC_LC_CONFIGURE, &CLightControl::OnBnClickedLcConfigure)
    ON_BN_CLICKED(IDC_TRACE_CORE_SET, &CLightControl::OnBnClickedTraceCoreSet)
    ON_BN_CLICKED(IDC_TRACE_MODELS_SET, &CLightControl::OnBnClickedTraceModelsSet)
    ON_BN_CLICKED(IDC_RSSI_TEST_START, &CLightControl::OnBnClickedRssiTestStart)
    ON_MESSAGE(WM_MESH_ADD_VENDOR_MODEL, &CLightControl::OnAddVendorModel)

END_MESSAGE_MAP()

BOOL CLightControl::OnSetActive()
{
    CPropertyPage::OnSetActive();

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
    pSheet->m_active_page = 1;

    CComboBox *m_cbCom = (CComboBox *)GetDlgItem(IDC_COM_PORT);
    if (m_cbCom->GetCount() == 0)
    {
        WCHAR buf[20];
        for (int i = 0; i < 128 && aComPorts[i] != 0; i++)
        {
            wsprintf(buf, L"COM%d", aComPorts[i]);
            m_cbCom->SetItemData(m_cbCom->AddString(buf), aComPorts[i]);
        }

        wsprintf(buf, L"Host Mode");
        m_cbCom->SetItemData(m_cbCom->AddString(buf), 0);

        CComboBox *m_cbBaud = (CComboBox *)GetDlgItem(IDC_COM_BAUD);
        for (int i = 0; i < sizeof(as32BaudRate) / sizeof(as32BaudRate[0]); i++)
        {
            WCHAR acBaud[10];
            wsprintf(acBaud, L"%d", as32BaudRate[i]);

            m_cbBaud->SetItemData(m_cbBaud->AddString(acBaud), i);
        }
        m_cbBaud->SetCurSel(FindBaudRateIndex(3000000));

        if (m_ComHelper == NULL)
        {
            ComPort = m_cbCom->GetItemData(m_cbCom->GetCurSel());

            if (ComPort == 0)
            {
                m_ComHelper = new ComHelperHostMode(m_hWnd);
            }
            else
            {
                m_ComHelper = new ComHelper(m_hWnd);
            }
        }
    }

    if (ComPortSelected > 0)
        ((CComboBox *)GetDlgItem(IDC_COM_PORT))->SetCurSel(ComPortSelected);

    if (BaudRateSelected > 0)
        ((CComboBox *)GetDlgItem(IDC_COM_BAUD))->SetCurSel(BaudRateSelected);

    SetDlgItemHex(IDC_IDENTITY_DURATION, 1);

    CClientControlDlg *pMainDlg = &pSheet->pageMain;

    CString sHCDFileName = theApp.GetProfileString(L"LightControl", L"HCDFile", L"");
    SetDlgItemText(IDC_FILENAME, sHCDFileName);

    CString sStaticOobData = theApp.GetProfileString(L"LightControl", L"StaticOobData", L"");
    if (sStaticOobData == "")
        SetDlgItemText(IDC_OOB_DATA, L"965ca5c944b64d5786b47a29685c8bac");
    else
        SetDlgItemText(IDC_OOB_DATA, sStaticOobData);

    BOOL bUseStaticOobData = theApp.GetProfileInt(L"LightControl", L"UseStaticOobData", 0);
    ((CButton *)GetDlgItem(IDC_STATIC_OOB_DATA))->SetCheck(bUseStaticOobData);

    CString sFwManifestFile = theApp.GetProfileString(L"LightControl", L"FwManifestFile", L"");
    SetDlgItemText(IDC_FILENAME_DFU, sFwManifestFile);

    WCHAR szHostName[128];
    DWORD dw = 128;
    GetComputerName(szHostName, &dw);

    SetDlgItemText(IDC_PROVISIONER, szHostName);

    updateProvisionerUuid();

    ((CComboBox *)GetDlgItem(IDC_NETWORK))->ResetContent();

    char *p_networks = mesh_client_get_all_networks();
    char *p = p_networks;
    WCHAR szNetwork[80];
    int num_networks = 0;
    while (p != NULL && *p != NULL)
    {
        MultiByteToWideChar(CP_UTF8, 0, p, strlen(p) + 1, szNetwork, sizeof(szNetwork) / sizeof(WCHAR));
        ((CComboBox *)GetDlgItem(IDC_NETWORK))->AddString(szNetwork);
        p += strlen(p) + 1;
        num_networks++;
    }
    if (num_networks != 0)
        ((CComboBox *)GetDlgItem(IDC_NETWORK))->SetCurSel(0);

    ((CButton *)GetDlgItem(IDC_GATT_PROXY))->SetCheck(DeviceConfig.is_gatt_proxy);
    ((CButton *)GetDlgItem(IDC_FRIEND))->SetCheck(DeviceConfig.is_friend);
    ((CButton *)GetDlgItem(IDC_RELAY))->SetCheck(DeviceConfig.is_relay);
    ((CButton *)GetDlgItem(IDC_NET_BEACON))->SetCheck(DeviceConfig.send_net_beacon);
    SetDlgItemInt(IDC_RELAY_TRANSMIT_COUNT, DeviceConfig.relay_xmit_count);
    SetDlgItemInt(IDC_RELAY_TRANSMIT_INTERVAL, DeviceConfig.relay_xmit_interval);
    SetDlgItemInt(IDC_DEFAULT_TTL, DeviceConfig.default_ttl);
    SetDlgItemInt(IDC_NETWORK_TRANSMIT_COUNT, DeviceConfig.net_xmit_count);
    SetDlgItemInt(IDC_NETWORK_TRANSMIT_INTERVAL, DeviceConfig.net_xmit_interval);

    ((CComboBox *)GetDlgItem(IDC_MODEL_PUB_CREDENTIAL_FLAG))->SetCurSel(DeviceConfig.publish_credential_flag);
    SetDlgItemInt(IDC_PUBLISH_TTL, DeviceConfig.publish_ttl);
    SetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_COUNT, DeviceConfig.publish_retransmit_count);
    SetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_INTERVAL, DeviceConfig.publish_retransmit_interval);

    m_bVendorModelAdded = FALSE;

    SetDlgItemHex(IDC_RSSI_TEST_DST, 0xffff);
    SetDlgItemInt(IDC_RSSI_TEST_COUNT, 50);
    SetDlgItemInt(IDC_RSSI_TEST_INTERVAL, 100);

    mesh_client_init(&mesh_client_init_callbacks);
    free(p_networks);

#ifdef MESH_DFU_ENABLED
    m_dfuState = WICED_BT_MESH_DFU_STATE_INIT;
    m_bDfuStatus = FALSE;
    m_bDfuStarted = FALSE;
    m_bUploading = FALSE;

    CComboBox *pCb = (CComboBox *)GetDlgItem(IDC_DFU_METHOD);
    pCb->ShowWindow(SW_SHOW);
    for (int i = 0; i < ((sizeof(dfuMethods)) / sizeof(dfuMethods[0])); i++)
        pCb->AddString(dfuMethods[i]);
    pCb->SetCurSel(0);

    GetDlgItem(IDC_FILENAME_DFU)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_BROWSE_DFU)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_PROGRESS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_DFU_START_STOP)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_DFU_PAUSE_RESUME)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_DFU_PAUSE_RESUME)->EnableWindow(FALSE);
    GetDlgItem(IDC_DFU_GET_STATUS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_DFU_GET_STATUS)->EnableWindow(FALSE);
#endif

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CLightControl::OnClose()
{
    mesh_client_network_close();
    m_ComHelper->ClosePort();
    Sleep(1000);
    CPropertyPage::OnClose();
    // CDialogEx::OnClose();
}

void CLightControl::OnCancel()
{
    mesh_client_network_close();
    m_ComHelper->ClosePort();
    Sleep(1000);
    CPropertyPage::OnCancel();
    // CDialogEx::OnClose();
}

void CLightControl::OnCbnSelchangeComPort()
{
    CComboBox *m_cbCom = (CComboBox *)GetDlgItem(IDC_COM_PORT);

    ComPortSelected = m_cbCom->GetCurSel();
    BaudRateSelected = ((CComboBox *)GetDlgItem(IDC_COM_BAUD))->GetCurSel();

    ComPort = m_cbCom->GetItemData(m_cbCom->GetCurSel());
    int baud = GetBaudRateSelection();

    if (ComPort >= 0)
    {
        m_ComHelper->ClosePort();
        Sleep(1000);

        delete m_ComHelper;
        m_ComHelper = NULL;

        if (ComPort == 0)
        {
            m_ComHelper = new ComHelperHostMode(m_hWnd);
        }
        else
        {
            m_ComHelper = new ComHelper(m_hWnd);
        }

        m_ComHelper->OpenPort(ComPort, baud);
    }
}

int CLightControl::GetBaudRateSelection()
{
    CComboBox *m_cbBaud = (CComboBox *)GetDlgItem(IDC_COM_BAUD);
    int select = m_cbBaud->GetItemData(m_cbBaud->GetCurSel());

    if (select >= 0)
        return as32BaudRate[select];

    return as32BaudRate[0];
}

void CLightControl::OnBnClickedClearTrace()
{
    m_trace->ResetContent();
}

// CLightControl message handlers
void CLightControl::OnBnClickedDownload()
{
    char sHCDFileName[MAX_PATH] = { 0 };
    GetDlgItemTextA(m_hWnd, IDC_FILENAME, sHCDFileName, MAX_PATH);

    if (sHCDFileName[0] == 0)
    {
        Log(L"Specify valid configuration file and Address");
        return;
    }

    FILE *fHCD = NULL;
    LONG  nVeryFirstAddress = 0;

    WCHAR name[512] = { 0 };
    MultiByteToWideChar(CP_ACP, 0, (const char *)sHCDFileName, strlen(sHCDFileName), name, sizeof(name));
    BOOL rc = theApp.WriteProfileString(L"LightControl", L"HCDFile", name);

    m_fw_download_active = TRUE;
    rc = FwDownload(sHCDFileName);
    m_fw_download_active = FALSE;
}

void CLightControl::OnBnClickedBrowse()
{
    static TCHAR BASED_CODE szFilter[] = _T("HCD Files (*.hcd)|*.hcd|");

    CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
    if (dlgFile.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_FILENAME, dlgFile.GetPathName());
    }
}

DWORD CLightControl::GetHexValueInt(DWORD id)
{
    DWORD ret = 0;
    BYTE buf[32];
    DWORD len = GetHexValue(id, buf, sizeof(buf));
    for (DWORD i = 0; i<len; i++)
    {
        ret = (ret << 8) + buf[i];
    }
    return ret;
}

void CLightControl::SetDlgItemHex(DWORD id, DWORD val)
{
    WCHAR buf[10];
    wsprintf(buf, L"%x", val);
    SetDlgItemText(id, buf);
}

DWORD GetHexValueById(HWND hWnd, DWORD id, LPBYTE buf, DWORD buf_size)
{
    char szbuf[1300];
    char *psz = szbuf;
    BYTE *pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, buf_size);

    GetDlgItemTextA(hWnd, id, szbuf, sizeof(szbuf));
    if (strlen(szbuf) == 1)
    {
        szbuf[2] = 0;
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (strlen(szbuf) == 3)
    {
        szbuf[4] = 0;
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    for (DWORD i = 0; i < strlen(szbuf); i++)
    {
        if (isxdigit(psz[i]) && isxdigit(psz[i + 1]))
        {
            *pbuf++ = (ProcNibble(psz[i]) << 4) + ProcNibble(psz[i + 1]);
            res++;
            i++;
            if (res == buf_size)
                break;
        }
    }
    return res;
}

DWORD CLightControl::GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size)
{
    return GetHexValueById(m_hWnd, id, buf, buf_size);
}

void CLightControl::ProcessEvent(LPBYTE p_data, DWORD len)
{
    if (m_fw_download_active)
        FwDownloadProcessEvent(p_data, len);
    else
    {
        m_received_evt_len = len;
        memcpy(m_received_evt, p_data, len);
        SetEvent(m_hHciEvent);
    }
}

void CLightControl::ProcessData(INT port_num, DWORD opcode, LPBYTE p_data, DWORD len)
{
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    CLightControl* pDlg = &pSheet->pageLight;

    switch(opcode)
    {
    case HCI_CONTROL_EVENT_WICED_TRACE:
        if (len >= 2)
        {
            if ((len > 2) && (p_data[len - 2] == '\n'))
            {
                p_data[len - 2] = 0;
                len--;
            }
            TraceHciPkt(0, p_data, (USHORT)len);
        }
        break;

    case HCI_CONTROL_EVENT_HCI_TRACE:
        TraceHciPkt(p_data[0] + 1, &p_data[1], (USHORT)(len - 1));
        break;

    case HCI_CONTROL_MESH_EVENT_FW_DISTRIBUTION_UPLOAD_STATUS:
        pDlg->FwDistributionUploadStatus(p_data, len);
        break;

    case HCI_CONTROL_MESH_EVENT_RSSI_TEST_RESULT:
        pDlg->RssiTestResult(p_data, len);
        break;

    default:
        wiced_hci_process_data((uint16_t)opcode, p_data, (uint16_t)len);
        break;
    }
}

void CLightControl::ProcessUnprovisionedDevice(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len)
{
    WCHAR buf[180] = { 0 };
    WCHAR uuid[200] = { 0 };
    WCHAR szName[31] = { 0 };
    CComboBox *pCbUuid = (CComboBox *)GetDlgItem(IDC_PROVISION_UUID);

    for (int i = 0; i < 16; i++)
        wsprintf(&uuid[wcslen(uuid)], L"%02x ", p_uuid[i]);

    if (name_len != 0)
    {
        MultiByteToWideChar(CP_UTF8, 0, (char *)name, name_len, szName, sizeof(szName) / sizeof(WCHAR));
        szName[name_len] = 0;
        wcscat_s(uuid, sizeof(uuid) / 2, szName);
    }
    pCbUuid->SetCurSel(pCbUuid->AddString(uuid));

    wcscpy(buf, L"Unprovisioned Device UUID:");
    wcscat(buf, uuid);

    wsprintf(&buf[wcslen(buf)], L" OOB:%x", oob);

    m_trace->SetCurSel(m_trace->AddString(buf));

    if (m_bScanning && provision_test && p_uuid[12] == 0x11 && p_uuid[13] == 0x11 && p_uuid[14] == 0x11 && p_uuid[15] == 0x11)
    {
        PostMessage(WM_COMMAND, IDC_SCAN_UNPROVISIONED, 0);
        PostMessage(WM_COMMAND, IDC_PROVISION, 0);
    }
}

void CLightControl::LinkStatus(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt)
{
    WCHAR buf[180];

    wsprintf(&buf[0], L"Link Status:%d", is_connected);
    m_trace->SetCurSel(m_trace->AddString(buf));
    m_bConnected = is_connected;
    if (m_bConnected)
        SetDlgItemText(IDC_CONNECTDISCONNECT, L"Disconnect");
    else
        SetDlgItemText(IDC_CONNECTDISCONNECT, L"Connect");
}

void CLightControl::OnBnClickedScanUnprovisioned()
{
    if (!m_bScanning)
    {
        provision_test_scan_unprovisioned_time = GetTickCount();
        SetDlgItemText(IDC_SCAN_UNPROVISIONED, L"Stop Scanning");
        m_bScanning = TRUE;
        ((CComboBox *)GetDlgItem(IDC_PROVISION_UUID))->ResetContent();
    }
    else
    {
        SetDlgItemText(IDC_SCAN_UNPROVISIONED, L"Scan Unprovisioned");
        m_bScanning = FALSE;
    }
    WCHAR buf[128];
    wsprintf(buf, L"scan unprovisioned:%d", m_bScanning);
    m_trace->SetCurSel(m_trace->AddString(buf));
    mesh_client_scan_unprovisioned(m_bScanning, NULL);
}

void CLightControl::OnBnClickedProvision()
{
    uint8_t identify_duration = (BYTE)GetHexValueInt(IDC_IDENTITY_DURATION);
    DWORD num = 0;
    uint8_t uuid[16];
    num = GetHexValue(IDC_PROVISION_UUID, uuid, 16);

    BOOL is_static_oob_data = ((CButton*)GetDlgItem(IDC_STATIC_OOB_DATA))->GetCheck();
    uint8_t oob_data_len = 0;
    uint8_t oob_data[16];
    if (is_static_oob_data)
        oob_data_len = (uint8_t)GetHexValue(IDC_OOB_DATA, oob_data, 16);

    CString sStaticOobData;
    GetDlgItemText(IDC_OOB_DATA, sStaticOobData);
    theApp.WriteProfileString(L"LightControl", L"StaticOobData", sStaticOobData);
    theApp.WriteProfileInt(L"LightControl", L"UseStaticOobData", is_static_oob_data);

    char group_name[80];
    GetDlgItemTextA(m_hWnd, IDC_CURRENT_GROUP, group_name, sizeof(group_name));

    char node_name[80];
    GetDlgItemTextA(m_hWnd, IDC_PROVISION_UUID, node_name, sizeof(node_name));

    DeviceConfig.is_gatt_proxy = (BYTE)((CButton *)GetDlgItem(IDC_GATT_PROXY))->GetCheck();
    DeviceConfig.is_friend = (BYTE)((CButton *)GetDlgItem(IDC_FRIEND))->GetCheck();
    DeviceConfig.is_relay = (BYTE)((CButton *)GetDlgItem(IDC_RELAY))->GetCheck();
    DeviceConfig.send_net_beacon = (BYTE)((CButton *)GetDlgItem(IDC_NET_BEACON))->GetCheck();
    DeviceConfig.relay_xmit_count = GetDlgItemInt(IDC_RELAY_TRANSMIT_COUNT);
    DeviceConfig.relay_xmit_interval = GetDlgItemInt(IDC_RELAY_TRANSMIT_INTERVAL);
    DeviceConfig.default_ttl = GetDlgItemInt(IDC_DEFAULT_TTL);
    DeviceConfig.net_xmit_count = (BYTE)GetDlgItemInt(IDC_NETWORK_TRANSMIT_COUNT);
    DeviceConfig.net_xmit_interval = (USHORT)GetDlgItemInt(IDC_NETWORK_TRANSMIT_INTERVAL);

    DeviceConfig.publish_credential_flag = (BYTE)((CComboBox *)GetDlgItem(IDC_MODEL_PUB_CREDENTIAL_FLAG))->GetCurSel();
    DeviceConfig.publish_ttl = GetDlgItemInt(IDC_PUBLISH_TTL);
    DeviceConfig.publish_retransmit_count = GetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_COUNT);
    DeviceConfig.publish_retransmit_interval = GetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_INTERVAL);

    WriteDeviceConfig();

    mesh_client_set_device_config(NULL, DeviceConfig.is_gatt_proxy, DeviceConfig.is_friend, DeviceConfig.is_relay, DeviceConfig.send_net_beacon, DeviceConfig.relay_xmit_count, DeviceConfig.relay_xmit_interval, DeviceConfig.default_ttl, DeviceConfig.net_xmit_count, DeviceConfig.net_xmit_interval);
    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);

    if (!is_static_oob_data || (oob_data_len == 0))
        mesh_client_provision(node_name + (3 * num), group_name, uuid, identify_duration);
    else
        mesh_client_provision_with_oob(node_name + (3 * num), group_name, uuid, identify_duration, oob_data, oob_data_len);
}


void CLightControl::OnBnClickedNetworkCreate()
{
    char mesh_name[80], provisioner_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));
    GetDlgItemTextA(m_hWnd, IDC_PROVISIONER, provisioner_name, sizeof(provisioner_name));
    if (strlen(mesh_name) == 0)
        MessageBoxA(m_hWnd, "Provide mesh name and provisioner name", "Error", MB_ICONERROR);
    else if (mesh_client_network_exists(mesh_name))
        MessageBoxA(m_hWnd, mesh_name, "Network Already Exists", MB_ICONERROR);
    else
    {
        // try to update provisioner uuid if required.
        updateProvisionerUuid();

        int res = mesh_client_network_create(provisioner_name, provisioner_uuid, mesh_name);
        if (res == MESH_CLIENT_SUCCESS)
        {
            WCHAR s[80];
            MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, s, sizeof(s) / sizeof(WCHAR));
            Log(L"Network %s created\n", s);
            DisplayCurrentGroup();
        }
        else
        {
            Log(L"Failed to create network:%d\n", res);
        }
    }
}

void CLightControl::OnBnClickedNetworkDelete()
{
    char mesh_name[80], provisioner_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));
    GetDlgItemTextA(m_hWnd, IDC_PROVISIONER, provisioner_name, sizeof(provisioner_name));
    if (strlen(mesh_name) == 0)
        MessageBoxA(m_hWnd, "Provide mesh name and provisioner name", "Error", MB_ICONERROR);
    else if (!mesh_client_network_exists(mesh_name))
        MessageBoxA(m_hWnd, mesh_name, "Network Already Exists", MB_ICONERROR);
    else
    {
        int res = mesh_client_network_delete(provisioner_name, provisioner_uuid, mesh_name);
        if (res == MESH_CLIENT_SUCCESS)
        {
            WCHAR s[80];
            MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, s, sizeof(s) / sizeof(WCHAR));
            Log(L"Network %s deleted\n", s);

            // Clear the UUID after all mesh network deleted.
            char *p = mesh_client_get_all_networks();
            int num_networks = 0;
            while (p != NULL && *p != NULL)
            {
                p += strlen(p) + 1;
                num_networks++;
            }
            if (num_networks < 1)
            {
                WCHAR sProvisionerUuid[33] = { 0 };
                theApp.WriteProfileStringW(L"LightControl", L"ProvisionerUuid", sProvisionerUuid);
            }
        }
        else
        {
            Log(L"Failed to delete network:%d\n", res);
        }
    }
}

void CLightControl::OnBnClickedNetworkOpen()
{
    char mesh_name[80], provisioner_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));
    GetDlgItemTextA(m_hWnd, IDC_PROVISIONER, provisioner_name, sizeof(provisioner_name));
    if (mesh_client_network_open(provisioner_name, provisioner_uuid, mesh_name, network_opened) != MESH_CLIENT_SUCCESS)
    {
        MessageBoxA(m_hWnd, mesh_name, "Network Does Not Exists", MB_ICONERROR);
        return;
    }
    WCHAR szGroup[80];
    MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, szGroup, sizeof(szGroup) / sizeof(WCHAR));
    wcscpy(m_szCurrentGroup, szGroup);

    DisplayCurrentGroup();
}

void CLightControl::OnSelchangeCurrentGroup()
{
    CComboBox *p_current_group = (CComboBox *)GetDlgItem(IDC_CURRENT_GROUP);
    int sel = p_current_group->GetCurSel();
    if (sel < 0)
        return;

    p_current_group->GetLBText(sel, m_szCurrentGroup);
    DisplayCurrentGroup();
}

void CLightControl::OnBnClickedNetworkClose()
{
    mesh_client_network_close();
}

void CLightControl::DisplayCurrentGroup()
{
    CComboBox *p_network = (CComboBox *)GetDlgItem(IDC_NETWORK);
    CComboBox *p_current_group = (CComboBox *)GetDlgItem(IDC_CURRENT_GROUP);
    CComboBox *p_rename_devs = (CComboBox *)GetDlgItem(IDC_CONFIGURE_RENAME);
    CComboBox *p_move_devs = (CComboBox *)GetDlgItem(IDC_CONFIGURE_MOVE_DEVICE);
    CComboBox *p_move_groups = (CComboBox *)GetDlgItem(IDC_CONFIGURE_MOVE_TO_GROUP);
    CComboBox *p_configure_control_devs = (CComboBox *)GetDlgItem(IDC_CONFIGURE_CONTROL_DEVICE);
    CComboBox *p_configure_publish_to = (CComboBox *)GetDlgItem(IDC_CONFIGURE_PUBLISH_TO);
    CComboBox *p_target_devs_groups = (CComboBox *)GetDlgItem(IDC_CONTROL_DEVICE);

    int cur_group = p_current_group->GetCurSel();

    p_current_group->ResetContent();
    p_rename_devs->ResetContent();
    p_move_devs->ResetContent();
    p_move_groups->ResetContent();
    p_configure_control_devs->ResetContent();
    p_configure_publish_to->ResetContent();
    p_target_devs_groups->ResetContent();

    WCHAR szName[80] = { 0 };
    p_network->GetLBText(p_network->GetCurSel(), szName);
    p_current_group->AddString(szName);
    p_target_devs_groups->AddString(szName);
    p_move_groups->AddString(szName);

    p_configure_publish_to->AddString(L"none");
    p_configure_publish_to->AddString(L"all-nodes");
    p_configure_publish_to->AddString(L"all-proxies");
    p_configure_publish_to->AddString(L"all_friends");
    p_configure_publish_to->AddString(L"all-relays");
    p_configure_publish_to->AddString(L"this-device");

    char *p, *p1, *p2;
    char *p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
        p_current_group->AddString(szName);
        p_configure_publish_to->AddString(szName);
        p_target_devs_groups->AddString(szName);
        p_move_groups->AddString(szName);
        p_rename_devs->AddString(szName);

        char* p_groups1 = mesh_client_get_all_groups(p);
        for (p1 = p_groups1; p1 != NULL && *p1 != 0; p1 += (strlen(p1) + 1))
        {
            MultiByteToWideChar(CP_UTF8, 0, p1, -1, szName, sizeof(szName) / sizeof(WCHAR));
            p_current_group->AddString(szName);
            p_configure_publish_to->AddString(szName);
            p_target_devs_groups->AddString(szName);
            p_move_groups->AddString(szName);
            p_rename_devs->AddString(szName);

            char* p_groups2 = mesh_client_get_all_groups(p1);
            for (p2 = p_groups2; p2 != NULL && *p2 != 0; p2 += (strlen(p2) + 1))
            {
                MultiByteToWideChar(CP_UTF8, 0, p2, -1, szName, sizeof(szName) / sizeof(WCHAR));
                p_current_group->AddString(szName);
                p_configure_publish_to->AddString(szName);
                p_target_devs_groups->AddString(szName);
                p_move_groups->AddString(szName);
                p_rename_devs->AddString(szName);
            }
            free(p_groups2);
        }
        free(p_groups1);
    }
    free(p_groups);

    if ((cur_group >= 0) && (p_current_group->GetCount() >= cur_group))
        p_current_group->SetCurSel(cur_group);
    else if (p_current_group->GetCount() > 0)
        p_current_group->SetCurSel(0);

    // get groups and components for the current group
    char group_name[80];

    WideCharToMultiByte(CP_UTF8, 0, m_szCurrentGroup, -1, group_name, 80, NULL, FALSE);
    Log(L"Current Group: %s\n", m_szCurrentGroup);

    Log(L"Groups:\n");
    p_groups = mesh_client_get_all_groups(group_name);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
        Log(L"%s\n", szName);
    }
    free(p_groups);

    Log(L"Components:\n");
    char *p_components = mesh_client_get_group_components(group_name);
    for (p = p_components; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
        Log(L"%s\n", szName);

        p_rename_devs->AddString(szName);

        uint8_t component_type = mesh_client_get_component_type(p);
        switch (component_type)
        {
        case DEVICE_TYPE_GENERIC_ON_OFF_SERVER:
        case DEVICE_TYPE_GENERIC_LEVEL_SERVER:
        case DEVICE_TYPE_LIGHT_DIMMABLE:
        case DEVICE_TYPE_POWER_LEVEL_SERVER:
        case DEVICE_TYPE_POWER_ON_OFF_SERVER:
        case DEVICE_TYPE_LIGHT_HSL:
        case DEVICE_TYPE_LIGHT_CTL:
        case DEVICE_TYPE_LIGHT_XYL:
        case DEVICE_TYPE_SENSOR_SERVER:
        case DEVICE_TYPE_LOCATION_SERVER:
            p_move_devs->AddString(szName);
            p_configure_control_devs->AddString(szName);
            p_target_devs_groups->AddString(szName);
            p_configure_publish_to->AddString(szName);
            break;

        case DEVICE_TYPE_GENERIC_ON_OFF_CLIENT:
        case DEVICE_TYPE_GENERIC_LEVEL_CLIENT:
        case DEVICE_TYPE_SENSOR_CLIENT:
        case DEVICE_TYPE_LOCATION_CLIENT:
            p_target_devs_groups->AddString(szName);
            p_configure_control_devs->AddString(szName);
            p_configure_publish_to->AddString(szName);
            break;

        case DEVICE_TYPE_UNKNOWN:
        case DEVICE_TYPE_VENDOR_SPECIFIC:
            p_move_devs->AddString(szName);
            p_target_devs_groups->AddString(szName);
            p_configure_control_devs->AddString(szName);
            p_configure_publish_to->AddString(szName);
            break;
        }
    }
    free(p_components);
}

void CLightControl::OnBnClickedGroupCreate()
{
    int res;
    char mesh_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));
    char group_name[80], parent_group_name[80];
    GetDlgItemTextA(m_hWnd, IDC_GROUP_NAME, group_name, sizeof(group_name));
    GetDlgItemTextA(m_hWnd, IDC_CURRENT_GROUP, parent_group_name, sizeof(parent_group_name));

    SetDlgItemText(IDC_GROUP_NAME, L"");

    res = mesh_client_group_create(group_name, parent_group_name);
    if (res == MESH_CLIENT_SUCCESS)
    {
        WCHAR szGroup[80], szParent[80];
        MultiByteToWideChar(CP_UTF8, 0, group_name, strlen(group_name) + 1, szGroup, sizeof(szGroup) / sizeof(WCHAR));
        MultiByteToWideChar(CP_UTF8, 0, parent_group_name, strlen(parent_group_name) + 1, szParent, sizeof(szParent) / sizeof(WCHAR));
        Log(L"Group %s created in group %s\n", szGroup, szParent);
        DisplayCurrentGroup();
    }
    else
    {
        Log(L"Failed to create group:%d\n", res);
    }
}

void CLightControl::OnBnClickedGroupDelete()
{
    char group_name[80];
    GetDlgItemTextA(m_hWnd, IDC_CURRENT_GROUP, group_name, sizeof(group_name));
    mesh_client_group_delete(group_name);

    WCHAR szGroup[80];
    char mesh_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));
    MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, szGroup, sizeof(szGroup) / sizeof(WCHAR));
    wcscpy(m_szCurrentGroup, szGroup);

    DisplayCurrentGroup();
}

void CLightControl::ProvisionCompleted()
{
    DisplayCurrentGroup();
    if (provision_test)
    {
        CComboBox *p_target_devs_groups = (CComboBox *)GetDlgItem(IDC_CONTROL_DEVICE);
        p_target_devs_groups->SetCurSel(p_target_devs_groups->GetCount() - 1);
        PostMessage(WM_COMMAND, IDC_NODE_RESET, 0);
        return;
    }
}

void network_opened(uint8_t status)
{
    Log(L"Network opened");

    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
#ifndef NO_LIGHT_CONTROL
    if (pSheet->m_active_page == 1)
    {
        CLightControl* pDlg = &pSheet->pageLight;
        if (pDlg)
            pDlg->PostMessage(WM_MESH_ADD_VENDOR_MODEL, (WPARAM)0, (LPARAM)0);
    }
#endif
}

void unprovisioned_device(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len)
{
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
#ifndef NO_LIGHT_CONTROL
    if (pSheet->m_active_page == 1)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        pDlg->ProcessUnprovisionedDevice(p_uuid, oob, name, name_len);
    }
#endif
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)

    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script

    {
        tMESH_CLIENT_SCRIPT_UNPROVISIONED_DEVICE    unprovisioned_device = { 0 };
        memcpy(&unprovisioned_device.uuid, p_uuid, sizeof(UUID));
        if (name_len > 0 && name)
        {
            memcpy(&unprovisioned_device.name, name, name_len);
            unprovisioned_device.name_len = name_len;
        }
        unprovisioned_device.oob = oob;

        mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_UNPROVISIONED_DEVICE, &unprovisioned_device, sizeof(unprovisioned_device));
    }
#endif
}

/*
 * in general the application knows better when connection to the proxy is established or lost.
 * The only case when this function is called, when search for a node or a network times out.
 */
extern void link_status(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt)
{
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
#ifndef NO_LIGHT_CONTROL
    if (pSheet->m_active_page == 1)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        pDlg->LinkStatus(is_connected, conn_id, addr, is_over_gatt);
    }
#endif
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_CONNECT_STATUS connect_status;
    connect_status.is_connected = is_connected;
    connect_status.conn_id = conn_id;
    connect_status.addr = addr;
    connect_status.is_over_gatt = is_over_gatt;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_CONNECT_STATUS, &connect_status, sizeof(connect_status));

#endif
}

/*
 * Result of the componeent connect operation
 */
extern void node_connect_status(uint8_t status, char *p_device_name)
{
    WCHAR buf[512];
    WCHAR szDevName[80];
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
#ifndef NO_LIGHT_CONTROL
    if (pSheet->m_active_page == 1)
    {
        CLightControl *pDlg = &pSheet->pageLight;

        MultiByteToWideChar(CP_UTF8, 0, p_device_name, -1, szDevName, sizeof(szDevName)/sizeof(WCHAR));

        switch (status)
        {
        case MESH_CLIENT_NODE_CONNECTED:
            wsprintf(buf, L"Node %s connected continue OTA upgrade\n", szDevName);
            Log(buf);
            pDlg->OnOtaUpgradeContinue();
            break;

        case MESH_CLIENT_NODE_WARNING_UNREACHABLE:
            wsprintf(buf, L"Node %s failed to connect\n", szDevName);
            Log(buf);
            break;

        case MESH_CLIENT_NODE_ERROR_UNREACHABLE:
            wsprintf(buf, L"!!! Action Required Node %s unreachable\n", szDevName);
            Log(buf);
            break;
        }
    }
#endif
}

void provision_status(uint8_t status, uint8_t *p_uuid)
{
    WCHAR buf[512];
    char *p_devices = mesh_client_get_device_components(p_uuid);

    wsprintf(buf, L"Provision status:%d Device UUID: ", status);

    for (int i = 0; i < 16; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_uuid[i]);
    wcscat(buf, L"\n");

    Log(buf);

    if (status == MESH_CLIENT_PROVISION_STATUS_CONNECTING)
    {
        provision_test_connect_unprovisioned_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_PROVISIONING)
    {
        provision_test_provision_start_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_END)
    {
        provision_test_connect_provisioned_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_CONFIGURING)
    {
        provision_test_config_start_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_SUCCESS)
    {
        provision_test_reset_time = GetTickCount();
    }

    if (status != MESH_CLIENT_PROVISION_STATUS_SUCCESS && status != MESH_CLIENT_PROVISION_STATUS_FAILED)
        return;
    if (status == MESH_CLIENT_PROVISION_STATUS_SUCCESS)
    {
        for (char *p_component_name = p_devices; p_component_name != NULL && *p_component_name != 0; p_component_name += (strlen(p_component_name) + 1))
        {
            WCHAR szDevName[80];
            char *target_methods = mesh_client_get_target_methods(p_component_name);
            char *control_methods = mesh_client_get_control_methods(p_component_name);

            MultiByteToWideChar(CP_UTF8, 0, p_component_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
            wsprintf(buf, L"Name:%s", szDevName);
            Log(buf);
            if ((target_methods != NULL) && (target_methods[0] != 0))
            {
                wcscpy(buf, L"Can be controlled using: ");
                for (char *p = target_methods; *p != 0; p = p + strlen(p) + 1)
                {
                    MultiByteToWideChar(CP_UTF8, 0, p, -1, &buf[wcslen(buf)], sizeof(buf) / sizeof(WCHAR) - wcslen(buf));
                    wcscat(buf, L", ");
                }
                wcscat(buf, L"\n");
                Log(buf);
            }
            if ((control_methods != NULL) && (control_methods[0] != 0))
            {
                wcscpy(buf, L"Can control: ");
                for (char *p = control_methods; *p != 0; p = p + strlen(p) + 1)
                {
                    MultiByteToWideChar(CP_UTF8, 0, p, -1, &buf[wcslen(buf)], sizeof(buf) / sizeof(WCHAR) - wcslen(buf));
                    wcscat(buf, L", ");
                }
                wcscat(buf, L"\n");
                Log(buf);
            }
        }
    }
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
#ifndef NO_LIGHT_CONTROL
    if (pSheet->m_active_page == 1)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        pDlg->ProvisionCompleted();
    }
#endif
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_PROVISION_STATUS provision_status = { 0 };
    provision_status.status = status;
    if (p_uuid)
    {
        memcpy(&provision_status.uuid[0], p_uuid, sizeof(provision_status.uuid));
    }
    if (p_devices)
    {
        memcpy(&provision_status.name, p_devices, strlen(p_devices));
    }
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_PROVISION_STATUS, &provision_status, sizeof(provision_status));
#endif
    free(p_devices);
}

void database_changed(char *mesh_name)
{
    Log(L"database changed\n");
    // Update drop-down control "Move Device from"
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    if (pSheet)
    {
        CLightControl* pDlg = &pSheet->pageLight;
        if(pDlg)
            pDlg->OnCbnSelchangeConfigureMoveDevice();
    }
}

void onoff_status(const char *device_name, uint8_t present, uint8_t target, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName)/sizeof(WCHAR));
    Log(L"%s OnOff state:%d\n", szDevName, present);
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_ON_OFF_STATUS onoff_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&onoff_status.device_name, device_name, name_len);
    }
    onoff_status.present = present;
    onoff_status.target = target;
    onoff_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_ON_OFF_STATUS, &onoff_status, sizeof(onoff_status));
#endif
}

void level_status(const char *device_name, int16_t present, int16_t target, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
    Log(L"%s Level state:%d\n", szDevName, present);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_LEVEL_STATUS level_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&level_status.device_name, device_name, name_len);
    }
    level_status.present = present;
    level_status.target = target;
    level_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_LEVEL_STATUS, &level_status, sizeof(level_status));
#endif
}

void lightness_status(const char *device_name, uint16_t present, uint16_t target, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
    Log(L"%s Light:%d\n", szDevName, present);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_LIGHTNESS_STATUS lightness_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&lightness_status.device_name, device_name, name_len);
    }
    lightness_status.present = present;
    lightness_status.target = target;
    lightness_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_LIGHTNESS_STATUS, &lightness_status, sizeof(lightness_status));
#endif
}

void hsl_status(const char *device_name, uint16_t lightness, uint16_t hue, uint16_t saturation, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
    Log(L"%s Light:%d Hue:%d Sat:%d\n", szDevName, lightness, hue, saturation);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_HSL_STATUS hsl_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&hsl_status.device_name, device_name, name_len);
    }
    hsl_status.lightness = lightness;
    hsl_status.hue = hue;
    hsl_status.saturation = saturation;
    hsl_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_HSL_STATUS, &hsl_status, sizeof(hsl_status));
#endif
}

void ctl_status(const char *device_name, uint16_t present_lightness, uint16_t present_temperature, uint16_t target_lightness, uint16_t target_temperature, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
    Log(L"%s present Light/Temp:%d/%d target Light/Temp:%d/%d\n", szDevName, present_lightness, present_temperature, target_lightness, target_temperature);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_CTL_STATUS ctl_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&ctl_status.device_name, device_name, name_len);
    }
    ctl_status.present_lightness = present_lightness;
    ctl_status.present_temperature = present_temperature;
    ctl_status.target_lightness = target_lightness;
    ctl_status.target_temperature = target_temperature;
    ctl_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_CTL_STATUS, &ctl_status, sizeof(ctl_status));
#endif
}

void xyl_status(const char* device_name, uint16_t present_lightness, uint16_t x, uint16_t y, uint32_t remaining_time)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));
    Log(L"%s present Light:%d x/y:%d/%d\n", szDevName, present_lightness, x, y);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_XYL_STATUS xyl_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&xyl_status.device_name, device_name, name_len);
    }
    xyl_status.present_lightness = present_lightness;
    xyl_status.x = x;
    xyl_status.y = y;
    xyl_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_XYL_STATUS, &xyl_status, sizeof(xyl_status));
#endif
}


void sensor_status(const char *device_name, int property_id, uint8_t value_len, uint8_t *value)
{
    WCHAR szDevName[80];
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    MultiByteToWideChar(CP_UTF8, 0, device_name, -1, szDevName, sizeof(szDevName) / sizeof(WCHAR));

    WCHAR   msg[1002];
    if (property_id == WICED_BT_MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE)
        swprintf_s(msg, sizeof(msg) / 2, L"Sensor data from:%s Ambient Temperature %d degrees Celsius", szDevName, ((int8_t)value[0]) / 2);
    else if (property_id == WICED_BT_MESH_PROPERTY_PRESENCE_DETECTED)
        swprintf_s(msg, sizeof(msg) / 2, L"Sensor data from:%s Presense detected %s", szDevName, value[0] != 0 ? L"true" : L"false");
    else if (property_id == WICED_BT_MESH_PROPERTY_PRESENT_AMBIENT_LIGHT_LEVEL)
        swprintf_s(msg, sizeof(msg) / 2, L"Sensor data from:%s ambient light level %d", szDevName, value[0] + (value[1] << 8) + (value[2] << 16));
    else
    {
        swprintf_s(msg, sizeof(msg) / 2, L"Sensor data from %s Property ID:0x%x 0x: ", szDevName, property_id);
        for (int i = 0; i < value_len; i++)
        {
            int len = swprintf_s(&msg[wcslen(msg)], sizeof(msg) / 2 - wcslen(msg), L"%02x", value[i]);
        }
        wcscat(msg, L"\n");
    }
    Log(msg);
}

extern void RecordLatencyData(const char* device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t tx_hops, uint8_t rx_hops, int elapsed_time);

void vendor_specific_data(const char *device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t ttl, uint8_t *p_data, uint16_t data_len)
{
    uint8_t tx_hops = 0;
    uint8_t rx_hops = 0;
    uint16_t perf_data_len = data_len;
    WCHAR s[80];
    MultiByteToWideChar(CP_UTF8, 0, device_name, strlen(device_name) + 1, s, sizeof(s) / sizeof(WCHAR));

    DWORD roundtrip_time = p_data[data_len-4] + (p_data[data_len-3] << 8) + (p_data[data_len-2] << 16) + (p_data[data_len-1] << 24);

    WCHAR buf[300] = { 0 };
    int i;
    while (data_len != 0)
    {
        buf[0] = 0;
        for (i = 0; i < data_len && i < 32; i++)
            wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);

        data_len -= i;
        if (data_len != 0)
            Log(buf);
    }

    Log(buf);

    if (theApp.bMeshPerfMode)
    {
        // We expect a minimum of 5 bytes, TTL (1) in response from Mesh Perf application + elapsed time (4)
        if (perf_data_len < 5)
            return;
        tx_hops = LOCAL_DEVICE_TTL - (uint8_t)p_data[perf_data_len - 5];
        rx_hops = LOCAL_DEVICE_TTL - ttl;
        RecordLatencyData(device_name, company_id, model_id, opcode, tx_hops, rx_hops, (int)roundtrip_time);
        Log(L"RECV VS Data from %s company:%x model:%x opcode:%d tx_hops: %x, rx_hops: %x, roundtrip time: %d ms\n",
            s, company_id, model_id, opcode, tx_hops, rx_hops, roundtrip_time);
    }
    else
    {
        Log(L"RECV VS Data from %s company:%x model:%x opcode:%d roundtrip time: %d ms\n",
            s, company_id, model_id, opcode, roundtrip_time);
    }
}

void CLightControl::OnBnClickedNodeReset()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_reset_device(name);

    DisplayCurrentGroup();
}

void CLightControl::OnSelchangeNetwork()
{
    OnBnClickedNetworkClose();
}

void CLightControl::OnBnClickedConfigureNewName()
{
    CComboBox *p_rename_devs = (CComboBox *)GetDlgItem(IDC_CONFIGURE_RENAME);
    int sel = p_rename_devs->GetCurSel();
    if (sel < 0)
        return;

    char old_name[80];
    WCHAR szDevName[80];
    p_rename_devs->GetLBText(sel, szDevName);
    WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, old_name, 80, NULL, FALSE);

    char new_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NEW_NAME, new_name, sizeof(new_name) - 1);
    if (strlen(new_name) > 0)
    {
        SetDlgItemText(IDC_NEW_NAME, L"");

        mesh_client_rename(old_name, new_name);
        strcpy(old_name, new_name);
    }
    char group_name[80];
    GetDlgItemTextA(m_hWnd, IDC_CURRENT_GROUP, group_name, sizeof(group_name));

    DeviceConfig.is_gatt_proxy = (BYTE)((CButton *)GetDlgItem(IDC_GATT_PROXY))->GetCheck();
    DeviceConfig.is_friend = (BYTE)((CButton *)GetDlgItem(IDC_FRIEND))->GetCheck();
    DeviceConfig.is_relay = (BYTE)((CButton *)GetDlgItem(IDC_RELAY))->GetCheck();
    DeviceConfig.send_net_beacon = (BYTE)((CButton *)GetDlgItem(IDC_NET_BEACON))->GetCheck();
    DeviceConfig.relay_xmit_count = GetDlgItemInt(IDC_RELAY_TRANSMIT_COUNT);
    DeviceConfig.relay_xmit_interval = GetDlgItemInt(IDC_RELAY_TRANSMIT_INTERVAL);
    DeviceConfig.default_ttl = GetDlgItemInt(IDC_DEFAULT_TTL);
    DeviceConfig.net_xmit_count = (BYTE)GetDlgItemInt(IDC_NETWORK_TRANSMIT_COUNT);
    DeviceConfig.net_xmit_interval = (USHORT)GetDlgItemInt(IDC_NETWORK_TRANSMIT_INTERVAL);

    WriteDeviceConfig();

    mesh_client_set_device_config(old_name, DeviceConfig.is_gatt_proxy, DeviceConfig.is_friend, DeviceConfig.is_relay, DeviceConfig.send_net_beacon, DeviceConfig.relay_xmit_count, DeviceConfig.relay_xmit_interval, DeviceConfig.default_ttl, DeviceConfig.net_xmit_count, DeviceConfig.net_xmit_interval);
    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);

    DisplayCurrentGroup();
}

void CLightControl::OnBnClickedMoveToGroup()
{
    char device_name[80];
    char group_to_name[80] = { 0 };
    char group_from_name[80] = { 0 };
    WCHAR szDevName[80];
    WCHAR szGroupName[80];

    CComboBox *p_device = (CComboBox *)GetDlgItem(IDC_CONFIGURE_MOVE_DEVICE);
    int sel = p_device->GetCurSel();
    if (sel < 0)
        return;

    char mesh_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));

    p_device->GetLBText(sel, szDevName);
    WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, device_name, 80, NULL, FALSE);

    CComboBox *pGroup = (CComboBox *)GetDlgItem(IDC_CONFIGURE_MOVE_TO_GROUP);
    if ((sel = pGroup->GetCurSel()) >= 0)
    {
        pGroup->GetLBText(sel, szGroupName);
        WideCharToMultiByte(CP_UTF8, 0, szGroupName, -1, group_to_name, 80, NULL, FALSE);
    }
    pGroup = (CComboBox *)GetDlgItem(IDC_CONFIGURE_MOVE_FROM_GROUP);
    if ((sel = pGroup->GetCurSel()) >= 0)
    {
        pGroup->GetLBText(sel, szGroupName);
        WideCharToMultiByte(CP_UTF8, 0, szGroupName, -1, group_from_name, 80, NULL, FALSE);
    }

    // We might need to use default pub params
    WriteDeviceConfig();
    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);
    if (((group_from_name[0] == 0) || (strcmp(mesh_name, group_from_name) == 0)) && ((group_to_name[0] != 0) && (strcmp(mesh_name, group_to_name) != 0)))
        mesh_client_add_component_to_group(device_name, group_to_name);
    else if ((group_from_name[0] != 0) && (strcmp(mesh_name, group_from_name) != 0) && ((group_to_name[0] == 0) || (strcmp(mesh_name, group_to_name) == 0)))
        mesh_client_remove_component_from_group(device_name, group_from_name);
    else if ((group_from_name[0] != 0) && (strcmp(mesh_name, group_from_name) != 0) && (group_to_name[0] != 0) && (strcmp(mesh_name, group_to_name) != 0))
        mesh_client_move_component_to_group(device_name, group_from_name, group_to_name);
}

void CLightControl::OnCbnSelchangeConfigureControlDevice()
{
    CComboBox *p_device = (CComboBox *)GetDlgItem(IDC_CONFIGURE_CONTROL_DEVICE);
    int sel = p_device->GetCurSel();
    if (sel < 0)
        return;

    WCHAR szDevName[80];
    p_device->GetLBText(sel, szDevName);

    char device_name[80];
    WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, device_name, 80, NULL, FALSE);

    CComboBox *p_method = (CComboBox *)GetDlgItem(IDC_CONFIGURE_PUBLISH_METHOD);
    p_method->ResetContent();

    char *target_methods = mesh_client_get_target_methods(device_name);
    for (char *p = target_methods; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        WCHAR szPublishMethod[80];
        wcscpy(szPublishMethod, L"send \"");
        MultiByteToWideChar(CP_UTF8, 0, p, strlen(p) + 1, &szPublishMethod[wcslen(szPublishMethod)], sizeof(szPublishMethod) / sizeof(WCHAR) - 28);
        if (strncmp(p, MESH_CONTROL_METHOD_VENDOR, strlen(MESH_CONTROL_METHOD_VENDOR)) == 0)
            wcscat(szPublishMethod, L"\" data to");
        else
            wcscat(szPublishMethod, L"\" status to");
        p_method->AddString(szPublishMethod);
    }
    free(target_methods);

    char *control_methods = mesh_client_get_control_methods(device_name);
    for (char *p = control_methods; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        WCHAR szPublishMethod[80];
        wcscpy(szPublishMethod, L"control \"");
        MultiByteToWideChar(CP_UTF8, 0, p, strlen(p) + 1, &szPublishMethod[wcslen(szPublishMethod)], sizeof(szPublishMethod) / sizeof(WCHAR) - 24);
        wcscat(szPublishMethod, L"\" of");
        p_method->AddString(szPublishMethod);
    }
    free(control_methods);
    if (p_method->GetCount() != 0)
        p_method->SetCurSel(0);
}

BOOL is_component_in_group(char* component_name, char* group_name)
{
    char* p_components = mesh_client_get_group_components(group_name);
    for (char *p = p_components; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        if (strcmp(p, component_name) == 0)
            return TRUE;
    }
    return FALSE;
}

void CLightControl::OnCbnSelchangeConfigureMoveDevice()
{
    CComboBox* p_move_from_groups = (CComboBox*)GetDlgItem(IDC_CONFIGURE_MOVE_FROM_GROUP);
    p_move_from_groups->ResetContent();

    CComboBox* p_device = (CComboBox*)GetDlgItem(IDC_CONFIGURE_MOVE_DEVICE);
    int sel = p_device->GetCurSel();
    if (sel < 0)
        return;

    WCHAR szName[80] = { 0 };
    char device_name[80] = { 0 };
    p_device->GetLBText(sel, szName);
    WideCharToMultiByte(CP_UTF8, 0, szName, -1, device_name, 80, NULL, FALSE);

    char* p, * p1, * p2;
    char* p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        if (is_component_in_group(device_name, p))
        {
            MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
            p_move_from_groups->AddString(szName);
        }
        char* p_groups1 = mesh_client_get_all_groups(p);
        for (p1 = p_groups1; p1 != NULL && *p1 != 0; p1 += (strlen(p1) + 1))
        {
            if (is_component_in_group(device_name, p1))
            {
                MultiByteToWideChar(CP_UTF8, 0, p1, -1, szName, sizeof(szName) / sizeof(WCHAR));
                p_move_from_groups->AddString(szName);
            }
            char* p_groups2 = mesh_client_get_all_groups(p1);
            for (p2 = p_groups2; p2 != NULL && *p2 != 0; p2 += (strlen(p2) + 1))
            {
                if (is_component_in_group(device_name, p2))
                {
                    MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
                    p_move_from_groups->AddString(szName);
                }
            }
            free(p_groups2);
        }
        free(p_groups1);
    }
    free(p_groups);
}

void CLightControl::OnBnClickedConfigurePub()
{
    CComboBox *p_device = (CComboBox *)GetDlgItem(IDC_CONFIGURE_CONTROL_DEVICE);
    int sel = p_device->GetCurSel();
    if (sel < 0)
        return;

    WCHAR szDevName[80];
    p_device->GetLBText(sel, szDevName);

    CComboBox *p_method = (CComboBox *)GetDlgItem(IDC_CONFIGURE_PUBLISH_METHOD);
    if ((sel = p_method->GetCurSel()) < 0)
        return;

    WCHAR szPublishString[80], szPublishMethod[80];
    WCHAR *p;
    p_method->GetLBText(sel, szPublishString);
    uint8_t client_control;
    if (wcsncmp(szPublishString, L"control \"", wcslen(L"control \"")) == 0)
    {
        client_control = 1;
        p = &szPublishString[wcslen(L"control \"")];
    }
    else
    {
        client_control = 0;
        p = &szPublishString[wcslen(L"send \"")];
    }
    int i;
    for (i = 0; *p != '\"'; i++)
        szPublishMethod[i] = *p++;
    szPublishMethod[i] = 0;
    char publish_method[80];
    WideCharToMultiByte(CP_UTF8, 0, szPublishMethod, -1, publish_method, 80, NULL, FALSE);

    CComboBox *p_configure_publish_to = (CComboBox *)GetDlgItem(IDC_CONFIGURE_PUBLISH_TO);
    if ((sel = p_configure_publish_to->GetCurSel()) < 0)
        return;

    WCHAR szPublishToName[80];
    p_configure_publish_to->GetLBText(sel, szPublishToName);

    char device_name[80];
    char publish_to_name[80];
    WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, device_name, 80, NULL, FALSE);
    WideCharToMultiByte(CP_UTF8, 0, szPublishToName, -1, publish_to_name, 80, NULL, FALSE);

    // Get publication parameters from the dialog and tell Client that new parameters should be used
    int publish_credential_flag = (BYTE)((CComboBox *)GetDlgItem(IDC_MODEL_PUB_CREDENTIAL_FLAG))->GetCurSel();
    int publish_period = GetDlgItemInt(IDC_MODEL_PUB_PERIOD);
    int publish_ttl = GetDlgItemInt(IDC_PUBLISH_TTL);
    int publish_retransmit_count = GetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_COUNT);
    int publish_retransmit_interval = GetDlgItemInt(IDC_MODEL_PUB_RETRANSMIT_INTERVAL);

    mesh_client_set_publication_config(publish_credential_flag, publish_retransmit_count, publish_retransmit_interval, publish_ttl);
    mesh_client_configure_publication(device_name, client_control, publish_method, publish_to_name, publish_period);
}

bool isGroup(char* p_name)
{
    char* p;
    char* p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        if (strcmp(p_name, p) == NULL)
            return true;
    }
    return false;
}

void CLightControl::OnBnClickedOnOffGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_on_off_get(name);
}

void CLightControl::OnBnClickedOnOffSet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    int on_off = ((CComboBox *)GetDlgItem(IDC_ON_OFF_TARGET))->GetCurSel();
    if (on_off >= 0)
        mesh_client_on_off_set(name, on_off, !isGroup(name), DEFAULT_TRANSITION_TIME, 0);
}

void CLightControl::OnBnClickedLevelGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_level_get(name);
}

void CLightControl::OnBnClickedLevelSet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    short target_level = (short)GetDlgItemInt(IDC_LEVEL_TARGET);
    mesh_client_level_set(name, target_level, !isGroup(name), DEFAULT_TRANSITION_TIME, 0);
}

void CLightControl::OnBnClickedLightnessGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_lightness_get(name);
}


void CLightControl::OnBnClickedLightnessSet()
{
    int target_lightness = (int)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_lightness_set(name, target_lightness, !isGroup(name), DEFAULT_TRANSITION_TIME, 0);
}

void CLightControl::OnBnClickedLightHslGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_hsl_get(name);
}

void CLightControl::OnBnClickedLightHslSet()
{
    int target_lightness = (int)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    int target_hue = (int)GetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE);
    int target_saturation = (int)GetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE);
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_hsl_set(name, target_lightness, target_hue, target_saturation, !isGroup(name), DEFAULT_TRANSITION_TIME, 0);
}

void CLightControl::OnBnClickedLightCtlGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_ctl_get(name);
}

void CLightControl::OnBnClickedLightCtlSet()
{
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_temperature = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET);
    SHORT target_delta_uv = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_DELTA_UV_TARGET);
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_ctl_set(name, target_lightness, target_temperature, target_delta_uv, !isGroup(name), DEFAULT_TRANSITION_TIME, 0);
}

LRESULT CLightControl::OnAddVendorModel(WPARAM wparam, LPARAM lparam)
{
    {
        m_bVendorModelAdded = TRUE;

        uint8_t buffer[6];
        buffer[0] = 0xC1;               // Opcode 1  // Upper layer provided opcode
        buffer[0] = buffer[0] & ~0xC0;  // To make the vendor opcode spec compliant, turn off upper two bits ==> 0x01
                                        // Peer device will still receive 0xC1 from their access layer.
        buffer[1] = 0x31;
        buffer[2] = 0x01;

        buffer[3] = 0x02;               // Opcode 2
        buffer[4] = 0x31;
        buffer[5] = 0x01;

        // int mesh_client_add_vendor_model(uint16_t company_id, uint16_t model_id, uint8_t num_opcodes, uint8_t *buffer, uint16_t data_len);
        // CASE 1: When number of opcodes and data buffer is provided only messages that match both (opcode AND company_id) are received
        // mesh_client_add_vendor_model(0x131, 0x01, 0x02, buffer, (uint16_t)sizeof (buffer));

        // CASE 2: When number of opcodes is set to "0x0", messages for all opcodes for the company_id (0x131) are received
        mesh_client_add_vendor_model(0x131, 0x01, 0x0, buffer, (uint16_t)sizeof(buffer));

    }

    return S_OK;
}

void CLightControl::OnBnClickedVsData()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));

    BYTE buffer[400];
    DWORD len = GetHexValue(IDC_TC_NET_LEVEL_TRX_PDU, buffer, sizeof(buffer));

    mesh_client_vendor_data_set(name, 0x131, 0x01, 0x01, 0, buffer, (uint16_t)len);
}

void CLightControl::OnCbnSelchangeControlDevice()
{
    CComboBox *pSensors = (CComboBox *)GetDlgItem(IDC_SENSOR_TARGET);
    char name[80];
    ((CComboBox *)GetDlgItem(IDC_SENSOR_TARGET))->ResetContent();
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));

    int *property_ids = mesh_client_sensor_property_list_get(name);
    if (property_ids == NULL)
        return;
    int *p;

    for (p = property_ids; *p != 0; p++)
    {
        WCHAR szPropertyId[80];
        wsprintf(szPropertyId, L"%04X", *p);
        pSensors->SetItemData(pSensors->AddString(szPropertyId), *p);
    }
    free(property_ids);
    if (pSensors->GetCount() != 0)
        pSensors->SetCurSel(0);
}

void CLightControl::OnBnClickedSensorGet()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));

    int sel;
    CComboBox *pSensors = (CComboBox *)GetDlgItem(IDC_SENSOR_TARGET);
    if ((sel = pSensors->GetCurSel()) < 0)
        return;

    int property_id = (int)pSensors->GetItemData(sel);

    mesh_client_sensor_get(name, property_id);
}


void CLightControl::ProcessVendorSpecificData(LPBYTE p_data, DWORD len)
{
    DWORD i;
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT app_key_idx = (USHORT)p_data[2] + ((USHORT)p_data[3] << 8);
    BYTE   element_idx = p_data[4];
    p_data += 5;
    len -= 5;

    WCHAR buf[200];
    wsprintf(buf, L"VS Data from addr:%x element:%x app_key_idx:%x %d bytes:", src, app_key_idx, element_idx, len);
    m_trace->SetCurSel(m_trace->AddString(buf));

    while (len != 0)
    {
        buf[0] = 0;
        for (i = 0; i < len && i < 32; i++)
            wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);

        len -= i;
        if (len != 0)
            m_trace->SetCurSel(m_trace->AddString(buf));
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CLightControl::OnBnClickedIdentify()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_identify(name, 10);
}

void component_info_status_callback(uint8_t status, char *component_name, char *component_info)
{
    WCHAR szComponentName[80];
    WCHAR szComponentInfo[80];
    MultiByteToWideChar(CP_UTF8, 0, component_name, -1, szComponentName, sizeof(szComponentName) / sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, component_info, -1, szComponentInfo, sizeof(szComponentInfo) / sizeof(WCHAR));
    Log(L"Component Info status:%d from %s Info:%s\n", status, szComponentName, szComponentInfo);
}

void CLightControl::OnBnClickedGetComponentInfo()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
    mesh_client_get_component_info(name, component_info_status_callback);
}

void CLightControl::OnOtaUpgradeContinue()
{
#if 0
    int dfu_method = ((CComboBox *)GetDlgItem(IDC_DFU_METHOD))->GetCurSel();
    if (dfu_method != DFU_METHOD_APP_TO_DEVICE)
    {
        char name[80];
        GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
        EnterCriticalSection(&cs);
        mesh_client_dfu_start(dfu_method, name);
        LeaveCriticalSection(&cs);
        return;
    }
    // We are doing proprietary OTA Upgrad (app to device)
    // If Downloader object is created already
    if (m_pDownloader != NULL)
    {
        delete m_pDownloader;
        m_pDownloader = NULL;
    }
    CString sFilePath;
    GetDlgItemText(IDC_FILENAME, sFilePath);

    if (sFilePath.IsEmpty())
    {
        OnBnClickedBrowse();
        GetDlgItemText(IDC_FILENAME, sFilePath);
        if (sFilePath.IsEmpty())
            return;
    }
    FILE *fPatch;
    if (_wfopen_s(&fPatch, sFilePath, L"rb"))
    {
        MessageBox(L"Failed to open the patch file", L"Error", MB_OK);
        return;
    }

    // Load OTA FW file into memory
    fseek(fPatch, 0, SEEK_END);
    m_dwPatchSize = ftell(fPatch);
    rewind(fPatch);
    if (m_pPatch)
        free(m_pPatch);
    m_pPatch = (LPBYTE)malloc(m_dwPatchSize);

    m_dwPatchSize = (DWORD)fread(m_pPatch, 1, m_dwPatchSize, fPatch);
    fclose(fPatch);

    EnterCriticalSection(&cs);
    CBtWin10Interface *pWin10BtInterface = NULL;
    pWin10BtInterface = dynamic_cast<CBtWin10Interface *>(m_btInterface);

    if ((pWin10BtInterface == NULL) || !((CBtWin10Interface*)m_btInterface)->CheckForOTAServices())
    {
        LeaveCriticalSection(&cs);
        MessageBox(L"This device may not support OTA FW Upgrade. Select another device.", L"Error", MB_OK);

        if (m_pPatch != NULL)
        {
            free(m_pPatch);
            m_pPatch = NULL;
        }
        return;
    }
    LeaveCriticalSection(&cs);
    SetDlgItemText(IDC_OTA_UPGRADE_START, L"OTA Abort");

    // Create new downloader object
    m_pDownloader = new WSDownloader(m_btInterface, m_pPatch, m_dwPatchSize, m_hWnd);
    pWin10BtInterface->m_bConnected = TRUE;

    BTW_GATT_VALUE gatt_value;
    gatt_value.len = 2;
    gatt_value.value[0] = 3;
    gatt_value.value[1] = 0;

    EnterCriticalSection(&cs);
    if (m_btInterface != NULL)
    {
        guidSvcWSUpgrade = m_btInterface->m_bSecure ? GUID_OTA_SEC_FW_UPGRADE_SERVICE : GUID_OTA_FW_UPGRADE_SERVICE;
        pWin10BtInterface->SetDescriptorValue(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &gatt_value);
        pWin10BtInterface->RegisterNotification(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint);
    }
    LeaveCriticalSection(&cs);

    m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_CONNECTED);
#endif
}

LRESULT CLightControl::OnWsUpgradeCtrlPoint(WPARAM Instance, LPARAM lparam)
{
#if 0
    BTW_GATT_VALUE *pValue = (BTW_GATT_VALUE *)lparam;

    ods("OnWsUpgradeCtrlPoint: len:%d\n", pValue->len);
    if (pValue->len == 1)
    {
        switch (pValue->value[0])
        {
        case WICED_OTA_UPGRADE_STATUS_OK:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_OK);
            break;
        case WICED_OTA_UPGRADE_STATUS_CONTINUE:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_CONTINUE);
            break;
        default:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_FAILED);
            break;
        }
    }
    free(pValue);
#endif
    return S_OK;
}

LRESULT CLightControl::OnProgress(WPARAM state, LPARAM param)
{
#if 0
    static UINT total;
    if (state == WSDownloader::WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD)
    {
        total = (UINT)param;
        m_Progress.SetRange32(0, (int)param);
        SetDlgItemText(IDC_OTA_UPGRADE_START, L"Abort");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_DATA_TRANSFER)
    {
        m_Progress.SetPos((int)param);
        if (param == total)
        {
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_START_VERIFICATION);
        }
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_VERIFIED)
    {
        SetDlgItemText(IDC_OTA_UPGRADE_START, "OTA Start");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_ABORTED)
    {
        m_Progress.SetPos(total);
        SetDlgItemText(IDC_OTA_UPGRADE_START, "OTA Start");
    }
#endif
    return S_OK;
}

void CLightControl::OnBnClickedConnectdisconnect()
{
    WCHAR buf[128];

    if (!m_bConnected)
    {
        wsprintf(buf, L"Connecting to Proxy");
        m_trace->SetCurSel(m_trace->AddString(buf));
        mesh_client_connect_network(1, 7);
    }
    else
    {
        wsprintf(buf, L"Disconnecting from Proxy");
        m_trace->SetCurSel(m_trace->AddString(buf));
        mesh_client_disconnect_network();
    }
}

LRESULT CLightControl::OnMeshDeviceConnected(WPARAM Instance, LPARAM lparam)
{
    Log(L"OnMeshDeviceConnected:\n");
    mesh_client_connection_state_changed(1, 75);
    return S_OK;
}

LRESULT CLightControl::OnMeshDeviceDisconnected(WPARAM Instance, LPARAM lparam)
{
    Log(L"OnMeshDeviceDisconnected:\n");
    mesh_client_connection_state_changed(0, 0);
    return S_OK;
}

void CLightControl::OnBnClickedBrowseDfu()
{
    static TCHAR BASED_CODE szFilter[] = _T("JSON Files (*.json)|*.*|");

    CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, szFilter);
    if (dlgFile.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_FILENAME_DFU, dlgFile.GetPathName());
    }
}

void CLightControl::OnBnClickedNetworkImport()
{
    static TCHAR BASED_CODE szFilter[] = _T("JSON Files (*.json)|*.JSON|");

    CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, szFilter);
    if (dlgFile.DoModal() != IDOK)
        return;
    CString fileName = dlgFile.GetPathName();
    FILE *fJsonFile;
    if (_wfopen_s(&fJsonFile, fileName, L"rb"))
    {
        MessageBox(L"Failed to open the json file", L"Error", MB_OK);
        return;
    }

    // try to update provisioner uuid if required.
    updateProvisionerUuid();

    // Load OTA FW file into memory
    fseek(fJsonFile, 0, SEEK_END);
    size_t json_string_size = ftell(fJsonFile);
    rewind(fJsonFile);

    char *json_string = (char *)new BYTE[json_string_size+1];
    if (json_string == NULL)
        return;
    fread(json_string, 1, json_string_size, fJsonFile);
    json_string[json_string_size] = 0;
    fclose(fJsonFile);

    char *ifx_json_string = NULL;
    if (fileName.GetLength() > 4)
    {
        fileName.Insert(fileName.GetLength() - 4, L"ifx.");
        FILE *fIfxJsonFile;
        if (_wfopen_s(&fIfxJsonFile, fileName, L"rb") == 0)
        {
            fseek(fIfxJsonFile, 0, SEEK_END);
            json_string_size = ftell(fIfxJsonFile);
            rewind(fIfxJsonFile);

            ifx_json_string = new char[json_string_size+1];
            if (ifx_json_string != NULL)
            {
                fread(ifx_json_string, 1, json_string_size, fIfxJsonFile);
                ifx_json_string[json_string_size] = 0;
            }
            fclose(fIfxJsonFile);
        }
    }

    char provisioner_name[80];
    char *mesh_name;
    GetDlgItemTextA(m_hWnd, IDC_PROVISIONER, provisioner_name, sizeof(provisioner_name));
    if ((mesh_name = mesh_client_network_import(provisioner_name, provisioner_uuid, json_string, ifx_json_string, network_opened)) == NULL)
    {
        MessageBox(L"Failed to import json file", L"Error", MB_OK);
    }
    else
    {
        WCHAR s[80];
        MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, s, sizeof(s) / sizeof(WCHAR));
        Log(L"Network %s imported\n", s);

        ((CComboBox *)GetDlgItem(IDC_NETWORK))->ResetContent();

        char *p_networks = mesh_client_get_all_networks();
        char *p = p_networks;
        WCHAR szNetwork[80];
        int i = 0, sel = -1;
        while (p != NULL && *p != NULL)
        {
            MultiByteToWideChar(CP_UTF8, 0, p, strlen(p) + 1, szNetwork, sizeof(szNetwork) / sizeof(WCHAR));
            ((CComboBox *)GetDlgItem(IDC_NETWORK))->AddString(szNetwork);
            if (strcmp(p, mesh_name) == 0)
                sel = i;
            p += strlen(p) + 1;
            i++;
        }
        if (sel >= 0)
        {
            ((CComboBox *)GetDlgItem(IDC_NETWORK))->SetCurSel(sel);
        }
        WCHAR szGroup[80];
        MultiByteToWideChar(CP_UTF8, 0, mesh_name, strlen(mesh_name) + 1, szGroup, sizeof(szGroup) / sizeof(WCHAR));
        wcscpy(m_szCurrentGroup, szGroup);

        DisplayCurrentGroup();
    }
    delete[] json_string;
    if (ifx_json_string != NULL)
        delete[] ifx_json_string;
}

void CLightControl::OnBnClickedNetworkExport()
{
    char mesh_name[80];
    GetDlgItemTextA(m_hWnd, IDC_NETWORK, mesh_name, sizeof(mesh_name));

    CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, NULL);
    if (dlgFile.DoModal() != IDOK)
        return;

    char *json_string = mesh_client_network_export(mesh_name);
    FILE *fJsonFile;
    CString fileName = dlgFile.GetPathName();
    if (json_string != NULL)
    {
        if (_wfopen_s(&fJsonFile, fileName, L"w"))
        {
            MessageBox(L"Failed to open the json file", L"Error", MB_OK);
            return;
        }
        fwrite(json_string, 1, strlen(json_string), fJsonFile);
        fclose(fJsonFile);

        free(json_string);
    }
    strcat(mesh_name, ".ifx");
    json_string = mesh_client_network_export(mesh_name);
    if (json_string != NULL)
    {
        fileName.Insert(fileName.GetLength() - 4, L"ifx.");
        if (_wfopen_s(&fJsonFile, fileName, L"w"))
        {
            MessageBox(L"Failed to open the ifx.json file", L"Error", MB_OK);
            return;
        }
        fwrite(json_string, 1, strlen(json_string), fJsonFile);
        fclose(fJsonFile);

        free(json_string);
    }
}

void CLightControl::OnBnClickedSensorConfigure()
{
    CSensorConfig dlg;
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, dlg.component_name, sizeof(dlg.component_name));

    CComboBox *pSensors = (CComboBox *)GetDlgItem(IDC_SENSOR_TARGET);
    if (pSensors->GetCurSel() < 0)
        return;

    dlg.property_id = (USHORT)pSensors->GetItemData(pSensors->GetCurSel());

    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);
    INT_PTR nResponse = dlg.DoModal();
}

void CLightControl::OnBnClickedLcConfigure()
{
    CLightLcConfig dlg;
    GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, dlg.component_name, sizeof(dlg.component_name));
    if (mesh_client_is_light_controller(dlg.component_name))
        dlg.DoModal();
}

#ifdef MESH_DFU_ENABLED
#define MAX_TAG_NAME                                32
#define MAX_FILE_NAME                               256

extern "C"
{
    char skip_space(FILE* fp);
    int mesh_json_read_tag_name(FILE* fp, char* tagname, int len);
    int mesh_json_read_next_level_tag(FILE* fp, char* tagname, int len);
    int mesh_json_read_string(FILE* fp, char prefix, char* buffer, int len);
}

BOOL CLightControl::ReadDfuManifestFile(CString sFilePath)
{
    char tagname[MAX_TAG_NAME];
    char filename[MAX_FILE_NAME];
    char c1;
    FILE* fp;
    CString sPath;

    if (_wfopen_s(&fp, sFilePath, L"r"))
        return FALSE;

    sPath = sFilePath.Left(sFilePath.ReverseFind('\\') + 1);

    if (mesh_json_read_next_level_tag(fp, tagname, sizeof(tagname)) == 0)
        goto return_false;
    if (strcmp(tagname, "manifest") != 0)
        goto return_false;
    if (mesh_json_read_next_level_tag(fp, tagname, sizeof(tagname)) == 0)
        goto return_false;
    if (strcmp(tagname, "firmware") != 0)
        goto return_false;
    if (skip_space(fp) != '{')
        goto return_false;
    if (skip_space(fp) != '\"')
        goto return_false;
    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, MAX_TAG_NAME) == 0)
            break;
        if (skip_space(fp) != ':')
            goto return_false;
        c1 = skip_space(fp);
        if (strcmp(tagname, "firmware_file") == 0)
        {
            if (!mesh_json_read_string(fp, c1, filename, MAX_FILE_NAME))
                goto return_false;

            m_sDfuImageFilePath = sPath;
            m_sDfuImageFilePath.AppendFormat(L"%S", filename);
        }
        else if (strcmp(tagname, "metadata_file") == 0)
        {
            if (!mesh_json_read_string(fp, c1, filename, MAX_FILE_NAME))
                goto return_false;

            CString sMetaFilePath = sPath;
            sMetaFilePath.AppendFormat(L"%S", filename);

            FILE* fpMeta;
            if (_wfopen_s(&fpMeta, sMetaFilePath, L"rb"))
                goto return_false;

            int c = 0, i = 0;
            while (fread(&c, 1, 1, fpMeta) > 0)
            {
                m_DfuMetaData.data[i++] = (uint8_t)c;
            }
            m_DfuMetaData.len = (uint8_t)i;
            fclose(fpMeta);
        }
        else if (strcmp(tagname, "firmware_id") == 0)
        {
            if (!mesh_json_read_string(fp, c1, filename, MAX_FILE_NAME))
                goto return_false;
            char* p = filename;
            int i = 0;
            while ((size_t)(p - filename) < strlen(filename))
            {
                unsigned int value;
                sscanf(p, "%02x", &value);
                m_DfuFwId.fw_id[i++] = (uint8_t)value;
                p += 2;
            }
            m_DfuFwId.fw_id_len = i;
        }
        if (skip_space(fp) != ',')
            break;
        if (skip_space(fp) != '\"')
            break;
    }

    fclose(fp);
    return TRUE;

return_false:
    fclose(fp);
    return FALSE;
}
#endif

void CLightControl::OnBnClickedDfuStartstop()
{
    if (m_bDfuStarted)
    {
        OnDfuStop();
        SetDfuStarted(FALSE);
    }
    else if (OnDfuStart())
    {
        SetDfuStarted(TRUE);
    }
}

void CLightControl::SetDfuStarted(BOOL started)
{
    m_bDfuStarted = started;

    if (m_bDfuStarted)
    {
        SetDlgItemText(IDC_DFU_START_STOP, L"DFU Stop");
    }
    else
    {
        SetDlgItemText(IDC_DFU_START_STOP, L"DFU Start");
        GetDlgItem(IDC_DFU_PAUSE_RESUME)->EnableWindow(FALSE);
        GetDlgItem(IDC_DFU_GET_STATUS)->EnableWindow(FALSE);
    }
}

BOOL CLightControl::OnDfuStart()
{
    CString sFilePath;
    GetDlgItemText(IDC_FILENAME_DFU, sFilePath);

#ifdef MESH_DFU_ENABLED
    if (m_dfuState != WICED_BT_MESH_DFU_STATE_INIT && m_dfuState != WICED_BT_MESH_DFU_STATE_COMPLETE && m_dfuState != WICED_BT_MESH_DFU_STATE_FAILED)
    {
        Log(L"DFU already started\n");
        return FALSE;
    }
#endif

    if (sFilePath.IsEmpty())
    {
        OnBnClickedBrowseDfu();
        GetDlgItemText(IDC_FILENAME_DFU, sFilePath);
        if (sFilePath.IsEmpty())
            return FALSE;
    }

#ifdef MESH_DFU_ENABLED
    m_DfuMethod = ((CComboBox*)GetDlgItem(IDC_DFU_METHOD))->GetCurSel();

    if (m_DfuMethod == DFU_METHOD_PROXY_TO_ALL || m_DfuMethod == DFU_METHOD_APP_TO_ALL)
    {
        // DFU
        CString sFileExt = sFilePath.Right(5);
        if (sFileExt.CompareNoCase(CString(L".json")) != 0)
        {
            MessageBox(L"Please choose correct DFU manifest file (.json)", L"Error", MB_OK);
            return FALSE;
        }

        if (!ReadDfuManifestFile(sFilePath))
        {
            MessageBox(L"Failed to read from manifest file", L"Error", MB_OK);
            return FALSE;
        }

        m_Progress.SetRange32(0, 100);
        m_Progress.SetPos(0);

        m_dwPatchSize = GetDfuImageSize();
        m_dwPatchOffset = 0;

        // Start uploading firmware
        uint8_t buffer[512];
        LPBYTE p = buffer;

        *p++ = 0x7f;                            // TTL to use in a firmware image upload
        *p++ = 0;                               // timeout (2 bytes)
        *p++ = 0;                               // Actual timeout is 10*(Upload Timeout + 1). For upload over UART 10 seconds should be enough
        memset(p, 0, 8);                        // Blob ID
        p += 8;
        *p++ = m_dwPatchSize & 0xff;            // Firmware size in octets
        *p++ = (m_dwPatchSize >> 8) & 0xff;
        *p++ = (m_dwPatchSize >> 16) & 0xff;
        *p++ = (m_dwPatchSize >> 24) & 0xff;
        *p++ = m_DfuMetaData.len;               // Metadata length size in octets
        memcpy(p, m_DfuMetaData.data, m_DfuMetaData.len);
        p += m_DfuMetaData.len;
        memcpy(p, m_DfuFwId.fw_id, m_DfuFwId.fw_id_len);
        p += m_DfuFwId.fw_id_len;

        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_START, buffer, (DWORD)(p - buffer));
        m_bUploading = TRUE;
        Log(L"Start uploading\n");
    }
#endif
    return TRUE;
}

#define FIRMWARE_UPLOAD_BLOCK_SIZE  512

void CLightControl::FwDistributionUploadStatus(LPBYTE p_data, DWORD len)
{
#ifdef MESH_DFU_ENABLED
    uint8_t status = p_data[0];
    uint8_t phase = p_data[1];
    uint8_t distribution_status = p_data[2];
    uint8_t buffer[4 + FIRMWARE_UPLOAD_BLOCK_SIZE];
    uint8_t* p = buffer;
    DWORD dwBytes;

    if ((status == 0) && (m_dwPatchSize != 0) && m_bUploading)
    {
        if (m_dwPatchOffset == m_dwPatchSize)
        {
            m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_FINISH, &status, 1);
            m_dwPatchOffset = 0xFFFFFFFF;
            return;
        }
        else if (m_dwPatchOffset == 0xFFFFFFFF)
        {
            int result;
            uint16_t distributor_addr = 0;
            char name[80];
            GetDlgItemTextA(m_hWnd, IDC_CONTROL_DEVICE, name, sizeof(name));
            if (m_DfuMethod == DFU_METHOD_PROXY_TO_ALL)
                distributor_addr = get_device_addr(name);
            else if (m_DfuMethod == DFU_METHOD_APP_TO_ALL)
                distributor_addr = mesh_client_get_unicast_addr();
            m_bUploading = FALSE;
            //EnterCriticalSection(&cs);
            result = mesh_client_dfu_start(m_DfuFwId.fw_id, m_DfuFwId.fw_id_len, m_DfuMetaData.data, m_DfuMetaData.len, m_dwPatchSize, distributor_addr, fw_distribution_status);
            //LeaveCriticalSection(&cs);
            if (result != MESH_CLIENT_SUCCESS)
                SetDfuStarted(FALSE);
        }
        else
        {
            *p++ = m_dwPatchOffset & 0xff;
            *p++ = (m_dwPatchOffset >> 8) & 0xff;
            *p++ = (m_dwPatchOffset >> 16) & 0xff;
            *p++ = (m_dwPatchOffset >> 24) & 0xff;
            dwBytes = m_dwPatchOffset + FIRMWARE_UPLOAD_BLOCK_SIZE > m_dwPatchSize ? m_dwPatchSize - m_dwPatchOffset : FIRMWARE_UPLOAD_BLOCK_SIZE;
            GetDfuImageChunk(p, m_dwPatchOffset, (uint16_t)dwBytes);

            m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_DATA, buffer, dwBytes + 4);
            m_dwPatchOffset += dwBytes;
            m_Progress.SetPos(m_dwPatchOffset * 100 / m_dwPatchSize);
        }
    }
    else
    {
        if (m_bUploading)
        {
            Log(L"Upload failed\n");
            m_bUploading = FALSE;
        }
        SetDfuStarted(FALSE);
    }
#endif
}

void CLightControl::OnDfuStop()
{
#ifdef MESH_DFU_ENABLED
    if (m_bUploading)
    {
        uint8_t status = 1;
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_FINISH, &status, 1);
        m_bUploading = FALSE;
    }
    else
    {
        //EnterCriticalSection(&cs);
        mesh_client_dfu_stop();
        //LeaveCriticalSection(&cs);
    }

    if (m_bDfuStatus)
        OnBnClickedDfuGetStatus();

    m_dfuState = WICED_BT_MESH_DFU_STATE_INIT;
    Log(L"DFU stopped\n");
#endif
}

void CLightControl::OnBnClickedDfuPauseresume()
{
    static BOOL dfu_paused = FALSE;

#ifdef MESH_DFU_ENABLED
    //EnterCriticalSection(&cs);
    if (dfu_paused)
    {
        mesh_client_dfu_resume();
        dfu_paused = FALSE;
        SetDlgItemText(IDC_DFU_PAUSE_RESUME, L"DFU Pause");
    }
    else if (mesh_client_dfu_suspend() == MESH_CLIENT_SUCCESS)
    {
        dfu_paused = TRUE;
        SetDlgItemText(IDC_DFU_PAUSE_RESUME, L"DFU Resume");
    }
    //LeaveCriticalSection(&cs);
#endif
}

#ifdef MESH_DFU_ENABLED
void fw_distribution_status(uint8_t state, uint8_t *p_data, uint32_t data_length)
{
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    CLightControl* pDlg = &pSheet->pageLight;
    if (!pDlg)
        return;

    pDlg->OnDfuStatusCallback(state, p_data, data_length);
}
#endif

void CLightControl::OnBnClickedDfuGetStatus()
{
#ifdef MESH_DFU_ENABLED
    m_bDfuStatus = !m_bDfuStatus;

    //EnterCriticalSection(&cs);
    if (m_bDfuStatus)
        mesh_client_dfu_get_status(fw_distribution_status, DISTRIBUTION_STATUS_TIMEOUT);
    else
        mesh_client_dfu_get_status(NULL, 0);
    //LeaveCriticalSection(&cs);
    SetDlgItemText(IDC_DFU_GET_STATUS, m_bDfuStatus ? L"Stop DFU Status" : L"Get DFU Status");
#endif
}

#ifdef MESH_DFU_ENABLED
void CLightControl::OnDfuStatusCallback(uint8_t state, uint8_t *p_data, uint32_t data_length)
{
    int progress;
    uint16_t number_nodes, i;
    uint8_t *p;

    m_dfuState = state;

    switch (state)
    {
    case WICED_BT_MESH_DFU_STATE_VALIDATE_NODES:
        Log(L"DFU finding nodes\n");
        break;
    case WICED_BT_MESH_DFU_STATE_GET_DISTRIBUTOR:
        Log(L"DFU choosing Distributor\n");
        break;
    case WICED_BT_MESH_DFU_STATE_UPLOAD:
        if (p_data && data_length)
        {
            progress = (int)p_data[0];
            Log(L"DFU upload progress %d%%\n", progress);
            m_Progress.SetPos(progress / 2);
        }
        else
        {
            Log(L"DFU uploading firmware to the Distributor\n");
        }
        break;
    case WICED_BT_MESH_DFU_STATE_DISTRIBUTE:
        if (p_data && data_length)
        {
            number_nodes = (uint16_t)p_data[0] + ((uint16_t)p_data[1] << 8);
            if (number_nodes * 4 != data_length - 2)
            {
                Log(L"DFU bad distribution data length\n");
                break;
            }
            p = p_data + 2;
            progress = -1;
            for (i = 0; i < number_nodes; i++)
            {
                //Log(L"DFU distribution src:%02x%02x phase:%d progress:%d\n", p[1], p[0], p[2], p[3]);

                // Node data: 2 bytes address, 1 byte phase, 1 byte progress
                if (p[2] == WICED_BT_MESH_FW_UPDATE_PHASE_TRANSFER_ACTIVE)
                {
                    // Get first progress
                    if (progress == -1)
                        progress = (int)p[3];
                    // Get the lowest progress from all active nodes
                    else if (progress > (int)p[3])
                        progress = (int)p[3];
                }
                p += 4;
            }
            if (progress == -1)
                progress = 0;
            Log(L"DFU distribution progress %d%%\n", progress);

            m_Progress.SetPos(progress);
        }
        else
        {
            Log(L"DFU Distributor distributing firmware to nodes\n");
            GetDlgItem(IDC_DFU_PAUSE_RESUME)->EnableWindow(TRUE);
            GetDlgItem(IDC_DFU_GET_STATUS)->EnableWindow(TRUE);
            if (!m_bDfuStatus)
                OnBnClickedDfuGetStatus();
        }
        break;
    case WICED_BT_MESH_DFU_STATE_APPLY:
        Log(L"DFU applying firmware\n");
        break;
    case WICED_BT_MESH_DFU_STATE_SUSPENDED:
        Log(L"DFU paused\n");
        break;
    case WICED_BT_MESH_DFU_STATE_COMPLETE:
        Log(L"DFU completed\n");
        m_Progress.SetPos(100);
        if (p_data && data_length)
        {
            number_nodes = (uint16_t)p_data[0] + ((uint16_t)p_data[1] << 8);
            if (number_nodes * 4 != data_length - 2)
            {
                Log(L"DFU bad complete data length\n");
                break;
            }
            p = p_data + 2;
            for (i = 0; i < number_nodes; i++)
            {
                Log(L"Node %02x%02x DFU %s\n", p[1], p[0], p[2] == WICED_BT_MESH_FW_UPDATE_PHASE_APPLY_SUCCESS ? L"succeeded" : L"failed");
                p += 4;
            }
        }
        if (m_bDfuStatus)
            OnBnClickedDfuGetStatus();
        SetDfuStarted(FALSE);
        break;
    case WICED_BT_MESH_DFU_STATE_FAILED:
        Log(L"DFU failed\n");
        if (m_bDfuStatus)
            OnBnClickedDfuGetStatus();
        SetDfuStarted(FALSE);
        break;
    }
}

uint32_t CLightControl::GetDfuImageSize()
{
    uint32_t file_size;

    FILE *fPatch;
    if (_wfopen_s(&fPatch, m_sDfuImageFilePath, L"rb"))
        return 0;

    // Load OTA FW file into memory
    fseek(fPatch, 0, SEEK_END);
    file_size = (int)ftell(fPatch);
    fclose(fPatch);
    return file_size;
}

void CLightControl::GetDfuImageChunk(uint8_t *p_data, uint32_t offset, uint16_t data_len)
{
    FILE *fPatch;
    if (_wfopen_s(&fPatch, m_sDfuImageFilePath, L"rb"))
        return;

    // Load OTA FW file into memory
    fseek(fPatch, offset, SEEK_SET);
    fread(p_data, 1, data_len, fPatch);
    fclose(fPatch);
}

extern "C" uint32_t wiced_bt_get_fw_image_size(uint8_t partition)
{
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    CLightControl* pDlg = &pSheet->pageLight;
    return (pDlg != NULL) ? pDlg->GetDfuImageSize() : 0;
}

extern "C" void wiced_bt_get_fw_image_chunk(uint8_t partition, uint32_t offset, uint8_t *p_data, uint16_t data_len)
{
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    CLightControl* pDlg = &pSheet->pageLight;
    if (pDlg != NULL)
        pDlg->GetDfuImageChunk(p_data, offset, data_len);
}
#endif

extern "C" int mesh_client_set_device_config_UI_Ex(const char *device_name, int is_gatt_proxy, int is_friend, int is_relay, int beacon,
    int relay_xmit_count, int relay_xmit_interval, int default_ttl, int net_xmit_count, int net_xmit_interval)
{
    DeviceConfig.is_gatt_proxy = is_gatt_proxy;
    DeviceConfig.is_friend = is_friend;
    DeviceConfig.is_relay = is_relay;
    DeviceConfig.send_net_beacon = beacon;
    DeviceConfig.relay_xmit_count = relay_xmit_count;
    DeviceConfig.relay_xmit_interval = relay_xmit_interval;
    DeviceConfig.default_ttl = default_ttl;
    DeviceConfig.net_xmit_count = net_xmit_count;
    DeviceConfig.net_xmit_interval = net_xmit_interval;

    return 0;
}

extern "C" int mesh_client_set_publication_config_UI_Ex(const char *device_name, int device_type, int publish_credential_flag,
    int publish_retransmit_count, int publish_retransmit_interval, int publish_ttl)
{
    DeviceConfig.publish_credential_flag = publish_credential_flag;
    DeviceConfig.publish_ttl = publish_ttl;
    DeviceConfig.publish_retransmit_count = publish_retransmit_count;
    DeviceConfig.publish_retransmit_interval = publish_retransmit_interval;

    WriteDeviceConfig();

    return 0;
}

extern "C" int mesh_client_network_create_UI_Ex(const char *provisioner_name, const char *p_provisioner_uuid, char *mesh_name)
{
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    CLightControl* pDlg = &pSheet->pageLight;

    memset(provisioner_uuid, 0, sizeof(provisioner_uuid));
    memcpy(provisioner_uuid, p_provisioner_uuid, 32);

    WCHAR wcStr[128] = { 0 };
    MultiByteToWideChar(CP_UTF8, 0, (char*)mesh_name, strlen(mesh_name) + 1, wcStr, sizeof(wcStr) / sizeof(WCHAR));
    pDlg->SetDlgItemText(IDC_NETWORK, wcStr);

    memset(wcStr, 0, sizeof(wcStr));
    MultiByteToWideChar(CP_UTF8, 0, (char*)provisioner_name, strlen(provisioner_name) + 1, wcStr, sizeof(wcStr) / sizeof(WCHAR));
    pDlg->SetDlgItemText(IDC_PROVISIONER, wcStr);

    return 0;
}

extern "C" int mesh_client_scan_unprovisioned_UI_Ex(int start, const char*p_uuid)
{
    if (!provision_test_bScanning)
    {
        provision_test_scan_unprovisioned_time = GetTickCount();
        provision_test_bScanning = TRUE;
    }
    else
    {
        provision_test_bScanning = FALSE;
    }
    return 0;
}

static void mesh_clent_dlg_gen_uuid(BYTE *uuid)
{
    // Generate version 4 UUID (Random) per rfc4122:
    // - Set the two most significant bits(bits 6 and 7) of the
    //   clock_seq_hi_and_reserved to zero and one, respectively.
    // - Set the four most significant bits(bits 12 through 15) of the
    //   time_hi_and_version field to the 4 - bit version number.
    // - Set all the other bits to randomly(or pseudo - randomly) chosen values.
    *(UINT32 *)&uuid[0] = (UINT32)rand();
    *(UINT32 *)&uuid[4] = (UINT32)rand();
    *(UINT32 *)&uuid[8] = (UINT32)rand();
    *(UINT32 *)&uuid[12] = (UINT32)rand();
    // The version field is 4.
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    // The variant field is 10B
    uuid[8] = (uuid[8] & 0x3f) | 0x80;
}

void CLightControl::updateProvisionerUuid()
{
    CString sProvisionerUuid = theApp.GetProfileString(L"LightControl", L"ProvisionerUuid", L"");
    if (sProvisionerUuid == "")
    {
        BYTE uuid[16];
        mesh_clent_dlg_gen_uuid(uuid);
        WCHAR sProvisionerUuid[33] = { 0 };
        for (int i = 0; i < 16; i++)
            sprintf(&provisioner_uuid[i * 2], "%02X", uuid[i]);
        MultiByteToWideChar(CP_UTF8, 0, provisioner_uuid, 32, sProvisionerUuid, 32);
        theApp.WriteProfileStringW(L"LightControl", L"ProvisionerUuid", sProvisionerUuid);
    }
    else
    {
        WideCharToMultiByte(CP_UTF8, 0, sProvisionerUuid.GetBuffer(), -1, provisioner_uuid, 33, 0, FALSE);
    }
}

BOOL CLightControl::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);
    m_trace->AddString(L"NOTE:");
    m_trace->AddString(L"Use Baud rate of 3000000 for CYW920819EVB-02 board and 115200 for CYBT-213043-MESH board.");

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CLightControl::OnBnClickedTraceCoreSet()
{
    int core_level = ((CComboBox*)GetDlgItem(IDC_TRACE_CORE_LEVEL))->GetCurSel();
    DWORD core_modules = (DWORD)GetHexValueInt(IDC_TRACE_CORE_MODULES);
    if (core_level >= 0)
    {
        uint8_t buffer[5];
        uint8_t* p = buffer;
        *p++ = core_level & 0xff;
        *p++ = core_modules & 0xff;
        *p++ = (core_modules >> 8) & 0xff;
        *p++ = (core_modules >> 16) & 0xff;
        *p++ = (core_modules >> 24) & 0xff;
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TRACE_CORE_SET, buffer, 5);
    }
}


void CLightControl::OnBnClickedTraceModelsSet()
{
    int models_level = ((CComboBox*)GetDlgItem(IDC_TRACE_MODELS_LEVEL))->GetCurSel();
    if (models_level >= 0)
    {
        uint8_t buffer[1];
        uint8_t* p = buffer;
        *p++ = models_level & 0xff;
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TRACE_MODELS_SET, buffer, 1);
    }
}

void CLightControl::OnBnClickedRssiTestStart()
{
    DWORD dst = GetHexValueInt(IDC_RSSI_TEST_DST);
    DWORD count = GetDlgItemInt(IDC_RSSI_TEST_COUNT);
    DWORD interval = GetDlgItemInt(IDC_RSSI_TEST_INTERVAL);
    uint8_t buffer[5];
    uint8_t* p = buffer;
    *p++ = dst & 0xff;
    *p++ = (dst >> 8) & 0xff;
    *p++ = count & 0xff;
    *p++ = (count >> 8) & 0xff;
    if (interval < 100)
    {
        interval = 100;
        SetDlgItemInt(IDC_RSSI_TEST_INTERVAL, interval);
    }
    *p++ = (interval / 10) & 0xff;
    Log(L"Start RSSI test DST:%04x count:%d interval:%dms\n", dst, count, interval);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_RSSI_TEST_START, buffer, 5);
}

void CLightControl::RssiTestResult(LPBYTE p_data, DWORD len)
{
    uint16_t src = p_data[0] + (p_data[1] << 8);
    uint16_t report_addr = p_data[2] + (p_data[3] << 8);
    uint16_t rx_count = p_data[4] + (p_data[5] << 8);
    int8_t rx_rssi = p_data[6];

    Log(L"Test result %04x from %04x received:%d average rssi:%d\n", src, report_addr, rx_count, rx_rssi);
}
