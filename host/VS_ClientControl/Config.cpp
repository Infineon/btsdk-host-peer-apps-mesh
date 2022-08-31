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
// Config.cpp : implementation file
//

#include "stdafx.h"
#include "MeshConfig.h"
#include "ClientControlDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "hci_control_api.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_event.h"
#include "wiced_bt_mesh_provision.h"
#include "wiced_mesh_client.h"

//#define MIBLE
#ifdef MIBLE
#include "mible_mesh_api.h"
extern "C" void wiced_hci_process_data(uint16_t opcode, uint8_t *p_buffer, uint16_t len);
#endif

BYTE dev_key[16] = { 0 };
BYTE configured_dev_key[16] = { 0 };
BYTE pub_key_type;

// CConfig dialog
#if defined (CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
#include "mesh_bt_cert.h"



typedef struct
{
    uint16_t    extensions;                                             /* Bitmask indicating the provisioning extensions supported by the device */
    uint16_t    list[WICED_BT_MESH_PROVISIONING_RECORD_ID_MAX_SIZE];    /* Lists the Record IDs of the provisioning records stored on the device */
    uint16_t    size;                                                   /* sizes of the record data in the list */
} mesh_cbp_provisioning_list_t;

#pragma pack(1)
typedef struct
{
    uint8_t                                                 status;
    wiced_bt_mesh_provision_device_record_fragment_data_t   response;
    uint8_t                                                 data[WICED_BT_MESH_PROVISIONING_RECORD_BUFF_MAX_SIZE];
    uint16_t                                                size;
} mesh_cbp_provisioning_record_t;
#pragma pack()
#endif




#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_PUB_KEY   1   ///< Provisioner: Enter public key()
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_OUTPUT    2   /**< Provisioner: Enter output OOB value(size, action) displayed on provisioning node */
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_STATIC    3   ///< Provisioner: Enter static OOB value(size)
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_INPUT     4   /**< Provisioning node: Enter input OOB value(size, action) displayed on provisioner */
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_INPUT   5   ///< Provisioner: Select and display input OOB value(size, action)
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_OUTPUT  6   ///< Provisioning node: Select and display output OOB value(size, action)
#define WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_STOP    7   ///< Provisioner and Provisioning node: Stop displaying OOB value

#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_BLINK                 0x00  /**< Blink */
#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_BEEP                  0x01  /**< Beep */
#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_VIBRATE               0x02  /**< Vibrate */
#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_DISP_NUM              0x03  /**< Output Numeric */
#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_DISP_ALPH             0x04  /**< Output Alphanumeric */
#define WICED_BT_MESH_PROVISION_OUT_OOB_ACT_MAX                   0x15  /**< Max number of supported actions */

#define WICED_BT_MESH_PROVISION_IN_OOB_ACT_PUSH                   0x00  /**< Push */
#define WICED_BT_MESH_PROVISION_IN_OOB_ACT_TWIST                  0x01  /**< Twist */
#define WICED_BT_MESH_PROVISION_IN_OOB_ACT_ENTER_NUM              0x02  /**< Input Number */
#define WICED_BT_MESH_PROVISION_IN_OOB_ACT_ENTER_STR              0x03  /**< Input Alphanumeric */
#define WICED_BT_MESH_PROVISION_IN_OOB_ACT_MAX                    0x15  /**< Max number of supported actions */

static WCHAR* wchOobType[] =
{
    L"invalid",
    L"Enter Pub Key",
    L"invalid",
    L"Enter Static OOB",
    L"Enter Input",
    L"Display Input",
    L"Display Output",
    L"Display stop",
    L"invalid",
};

static WCHAR* wchOutputOobAction[] =
{
    L"Blink",
    L"Beep",
    L"Vibrate",
    L"Display Number",
    L"Display Alphanumeric",
    L"Invalid",
};

static WCHAR* wchInputOobAction[] =
{
    L"Push",
    L"Twist",
    L"Enter Number",
    L"Enter String",
    L"Invalid",
};

extern void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length);
extern "C" uint8_t *wiced_bt_mesh_format_hci_header(uint16_t dst, uint16_t app_key_idx, uint8_t element_idx, uint8_t reliable, uint8_t send_segmented, uint8_t ttl, uint8_t retransmit_count, uint8_t retransmit_interval, uint8_t reply_timeout, uint8_t *p_buffer, uint16_t len);
extern "C" wiced_bt_mesh_event_t *wiced_bt_mesh_event_from_hci_header(uint8_t **p_buffer, uint16_t *len);

// COutputOob dialog
class COutputOob : public CDialogEx
{
    DECLARE_DYNAMIC(COutputOob)

public:
    COutputOob(CWnd* pParent = NULL);   // standard constructor
    virtual ~COutputOob();
    BYTE m_oob_type;
    BYTE m_oob_size;
    BYTE m_oob_action;
    char m_output_value[17];

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_OOB_OUTPUT };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void OnBnClickedOk();

    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(COutputOob, CDialogEx)

COutputOob::COutputOob(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_OOB_OUTPUT, pParent)
{
    m_oob_type = 0;
    m_oob_size = 0;
    m_oob_action = 0;
    for (int i = 0; i < 17; ++i)
    {
        m_output_value[i] = 0;
    }
}

BOOL COutputOob::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetDlgItemText(IDC_STATIC_OOB, L"Input value presented by the device");
    return TRUE;  // return TRUE  unless you set the focus to a control
}

COutputOob::~COutputOob()
{
}

void COutputOob::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COutputOob, CDialogEx)
    ON_BN_CLICKED(IDOK, &COutputOob::OnBnClickedOk)
END_MESSAGE_MAP()

void COutputOob::OnBnClickedOk()
{
    GetDlgItemTextA(m_hWnd, IDC_OOB_NUMERIC, m_output_value, 17);
    CDialogEx::OnOK();
}

IMPLEMENT_DYNAMIC(CConfig, CPropertyPage)

BYTE auth_action;
extern BYTE dev_key[16];
extern BYTE pub_key_type;

CConfig::CConfig(CWnd* pParent /*=NULL*/)
	: CPropertyPage(IDD_CONFIGURATION, 103)
{
    p_local_composition_data = NULL;
    local_composition_data_len = 0;
    p_remote_composition_data = NULL;
    remote_composition_data_len = 0;
}

CConfig::~CConfig()
{
}

extern void Log(WCHAR *fmt, ...);

BOOL CConfig::OnSetActive()
{
    CPropertyPage::OnSetActive();

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
    ((CClientDialog*)theApp.m_pMainWnd)->m_active_page = idxPageConfig;

    CComboBox *m_cbCom = (CComboBox *)GetDlgItem(IDC_COM_PORT);
    if (m_cbCom->GetCount() == 0)
    {
        for (int i = 0; i < 128 && aComPorts[i] != 0; i++)
        {
            WCHAR buf[20];
            wsprintf(buf, L"COM%d", aComPorts[i]);
            m_cbCom->SetItemData(m_cbCom->AddString(buf), aComPorts[i]);
        }

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
            m_ComHelper = new ComHelper(m_hWnd);
        }
    }

    if (ComPortSelected > 0)
        ((CComboBox *)GetDlgItem(IDC_COM_PORT))->SetCurSel(ComPortSelected);

    if (BaudRateSelected > 0)
        ((CComboBox *)GetDlgItem(IDC_COM_BAUD))->SetCurSel(BaudRateSelected);

    SetDlgItemHex(IDC_LOCAL_ADDR, 2);
    SetDlgItemText(IDC_NET_KEY, L"00112233445566778899aabbccddeeff");
    SetDlgItemHex(IDC_NET_KEY_IDX, 0);
    SetDlgItemText(IDC_IV_INDEX, L"00000000");

    SetDlgItemText(IDC_PROVISION_UUID, L"");

    SetDlgItemHex(IDC_DST, 1);
    SetDlgItemHex(IDC_APP_KEY_IDX, 0);
    ((CButton *)(GetDlgItem(IDC_RELIABLE_SEND)))->SetCheck(1);

    SetDlgItemHex(IDC_HEARTBEAT_SUBSCRIPTION_SRC, 1);
    SetDlgItemHex(IDC_HEARTBEAT_SUBSCRIPTION_DST, 2);
    SetDlgItemHex(IDC_HEARTBEAT_SUBSCRIPTION_PERIOD, 0x10000);

    SetDlgItemHex(IDC_HEARTBEAT_PUBLICATION_DST, 0x02);
    SetDlgItemHex(IDC_HEARTBEAT_PUBLICATION_COUNT, 0x10000);
    SetDlgItemHex(IDC_HEARTBEAT_PUBLICATION_PERIOD, 0x3a98);
    SetDlgItemHex(IDC_HEARTBEAT_PUBLICATION_NET_KEY_IDX, 0);
    SetDlgItemHex(IDC_HEARTBEAT_PUBLICATION_TTL, 0x7f);

    ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_PROXY)))->SetCheck(0);
    ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_RELAY)))->SetCheck(0);
    ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_FRIEND)))->SetCheck(0);
    ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_LOW_POWER_MODE)))->SetCheck(0);

    SetDlgItemHex(IDC_COMPOSITION_DATA_PAGE, 1);
    SetDlgItemHex(IDC_NETWORK_TRANSMIT_COUNT, 2);
    SetDlgItemHex(IDC_NETWORK_TRANSMIT_INTERVAL, 0x64);
    ((CComboBox *)(GetDlgItem(IDC_BEACON_STATE)))->SetCurSel(0);
    SetDlgItemHex(IDC_DEFAULT_TTL, 127);
    ((CComboBox *)(GetDlgItem(IDC_GATT_PROXY_STATE)))->SetCurSel(0);
    ((CComboBox *)(GetDlgItem(IDC_RELAY_STATE)))->SetCurSel(0);
    SetDlgItemHex(IDC_RELAY_TRANSMIT_COUNT, 3);
    SetDlgItemHex(IDC_RELAY_TRANSMIT_INTERVAL, 0x64);
    ((CComboBox *)(GetDlgItem(IDC_FRIEND_STATE)))->SetCurSel(0);

    SetDlgItemHex(IDC_MODEL_PUB_ELEMENT_ADDR, 0);
    SetDlgItemHex(IDC_MODEL_PUB_PUBLISH_ADDR, 2);
    SetDlgItemHex(IDC_MODEL_PUB_APP_KEY_IDX, 0);
    ((CComboBox *)(GetDlgItem(IDC_MODEL_PUB_CREDENTIAL_FLAG)))->SetCurSel(0);
    SetDlgItemHex(IDC_PUBLISH_TTL, 0x3f);
    SetDlgItemHex(IDC_MODEL_PUB_PERIOD, 0x3a98);
    SetDlgItemHex(IDC_MODEL_PUB_RETRANSMIT_COUNT, 0);
    SetDlgItemHex(IDC_MODEL_PUB_RETRANSMIT_INTERVAL, 0);
    SetDlgItemHex(IDC_MODEL_PUB_COMP_ID, 0xFFFF);
    SetDlgItemHex(IDC_MODEL_PUB_MODEL_ID, 0x1001);
    SetDlgItemText(IDC_MODEL_PUB_VIRTUAL_ADDR, L"00112233445566778899aabbccddeeff");

    SetDlgItemHex(IDC_MODEL_SUB_ELEMENT_ADDR, 1);
    SetDlgItemHex(IDC_MODEL_SUB_ADDR, 2);
    SetDlgItemHex(IDC_MODEL_SUB_COMP_ID, 0xFFFF);
    SetDlgItemHex(IDC_MODEL_SUB_MODEL_ID, 0x1001);
    SetDlgItemText(IDC_MODEL_SUB_VIRTUAL_ADDR, L"00112233445566778899aabbccddeeff");

    SetDlgItemHex(IDC_NETKEY_IDX, 0);
    SetDlgItemHex(IDC_APPKEY_IDX, 0);
    SetDlgItemHex(IDC_COMP_ID, 0xFFFF);
    SetDlgItemHex(IDC_MODEL_ID, 0x1001);
    SetDlgItemHex(IDC_MODEL_BIND_ELEMENT_ADDR, 0x1);
    SetDlgItemText(IDC_NETKEY, L"00112233445566778899aabbccddeeff");
    SetDlgItemText(IDC_APPKEY, L"00112233445566778899aabbccddeeff");

    ((CComboBox *)(GetDlgItem(IDC_NODE_IDENTITY_STATE)))->SetCurSel(0);
    SetDlgItemHex(IDC_NODE_IDENTITY_NETKEY_IDX, 0);

    SetDlgItemHex(IDC_HEALTH_FAULT_COMPANY_ID, 0x131);
    SetDlgItemHex(IDC_HEALTH_FAULT_TEST_ID, 0);
    SetDlgItemHex(IDC_HEALTH_PERIOD, 1);
    SetDlgItemHex(IDC_HEALTH_ATTENTION, 0);

    SetDlgItemHex(IDC_IDENTITY_DURATION, 1);

    SetDlgItemText(IDC_DEVICE_PUB_KEY, L"F465E43FF23D3F1B9DC7DFC04DA8758184DBC966204796ECCF0D6CF5E16500CC0201D048BCBBD899EEEFC424164E33C201C2B010CA6B4D43A8A155CAD8ECB279");
    SetDlgItemText(IDC_BD_ADDR, L"001bdc08e4e8");
    SetDlgItemInt(IDC_BD_ADDR_TYPE, 0);

    // If static OOB data is requered in the profile then set 1(static OOB) in the IDC_AUTH_METOD. Otherwise set 0(No OOB)
    BOOL bUseStaticOobData = theApp.GetProfileInt(L"LightControl", L"UseStaticOobData", 0);
    ((CComboBox*)GetDlgItem(IDC_AUTH_METHOD))->SetCurSel(bUseStaticOobData ? 1 : 0);
    // Reset drop-down-box IDC_AUTH_ACTION
    ((CComboBox*)GetDlgItem(IDC_AUTH_ACTION))->ResetContent();

    // Fill Static OOB Data with the same value as in the LightControl tab
    CString sStaticOobData = theApp.GetProfileString(L"LightControl", L"StaticOobData", L"");
    if (sStaticOobData == "")
        SetDlgItemText(IDC_OOB_DATA_CFG, L"965ca5c944b64d5786b47a29685c8bac");
    else
        SetDlgItemText(IDC_OOB_DATA_CFG, sStaticOobData);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CConfig::ProcessEvent(LPBYTE p_data, DWORD len)
{
}


void CConfig::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}

BOOL CConfig::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);
    m_trace->AddString(L"NOTE:");
    m_trace->AddString(L"Use Baud rate of 3000000 for CYW920819EVB-02 board and 115200 for CYBT-213043-MESH board.");

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CConfig, CPropertyPage)
    ON_CBN_SELCHANGE(IDC_COM_PORT, &CConfig::OnCbnSelchangeComPort)
    ON_CBN_SELCHANGE(IDC_COM_BAUD, &CConfig::OnCbnSelchangeComPort)
    ON_BN_CLICKED(IDC_HEARTBEAT_SUBSCRIPTION_SET, &CConfig::OnBnClickedHeartbeatSubscriptionSet)
    ON_BN_CLICKED(IDC_HEARTBEAT_PUBLICATION_SET, &CConfig::OnBnClickedHeartbeatPublicationSet)
    ON_BN_CLICKED(IDC_NETWORK_TRANSMIT_INTERVAL_GET, &CConfig::OnBnClickedNetworkTransmitIntervalGet)
    ON_BN_CLICKED(IDC_NETWORK_TRANSMIT_INTERVAL_SET, &CConfig::OnBnClickedNetworkTransmitIntervalSet)
    ON_BN_CLICKED(IDC_NODE_RESET, &CConfig::OnBnClickedNodeReset)
    ON_BN_CLICKED(IDC_CONFIG_DATA_GET, &CConfig::OnBnClickedConfigDataGet)
    ON_BN_CLICKED(IDC_BEACON_GET, &CConfig::OnBnClickedBeaconGet)
    ON_BN_CLICKED(IDC_BEACON_SET, &CConfig::OnBnClickedBeaconSet)
    ON_BN_CLICKED(IDC_DEFAULT_TTL_GET, &CConfig::OnBnClickedDefaultTtlGet)
    ON_BN_CLICKED(IDC_DEFAULT_TTL_SET, &CConfig::OnBnClickedDefaultTtlSet)
    ON_BN_CLICKED(IDC_GATT_PROXY_GET, &CConfig::OnBnClickedGattProxyGet)
    ON_BN_CLICKED(IDC_GATT_PROXY_SET, &CConfig::OnBnClickedGattProxySet)
    ON_BN_CLICKED(IDC_RELAY_GET, &CConfig::OnBnClickedRelayGet)
    ON_BN_CLICKED(IDC_RELAY_SET, &CConfig::OnBnClickedRelaySet)
    ON_BN_CLICKED(IDC_FRIEND_GET, &CConfig::OnBnClickedFriendGet)
    ON_BN_CLICKED(IDC_FRIEND_SET, &CConfig::OnBnClickedFriendSet)
    ON_BN_CLICKED(IDC_MODEL_PUB_SET, &CConfig::OnBnClickedModelPubSet)
    ON_BN_CLICKED(IDC_MODEL_PUB_GET, &CConfig::OnBnClickedModelPubGet)
    ON_BN_CLICKED(IDC_MODEL_SUBSCRIPTION_ADD, &CConfig::OnBnClickedModelSubscriptionAdd)
    ON_BN_CLICKED(IDC_MODEL_SUBSCRIPTION_DELETE, &CConfig::OnBnClickedModelSubscriptionDelete)
    ON_BN_CLICKED(IDC_MODEL_SUBSCRIPTION_OVERWRITE, &CConfig::OnBnClickedModelSubscriptionOverwrite)
    ON_BN_CLICKED(IDC_MODEL_SUBSCRIPTION_DELETE_ALL, &CConfig::OnBnClickedModelSubscriptionDeleteAll)
    ON_BN_CLICKED(IDC_SUBSCRIPTION_GET, &CConfig::OnBnClickedSubscriptionGet)
    ON_BN_CLICKED(IDC_NETKEY_ADD, &CConfig::OnBnClickedNetkeyAdd)
    ON_BN_CLICKED(IDC_NETKEY_DELETE, &CConfig::OnBnClickedNetkeyDelete)
    ON_BN_CLICKED(IDC_NETKEY_UPDATE, &CConfig::OnBnClickedNetkeyUpdate)
    ON_BN_CLICKED(IDC_NETKEY_GET, &CConfig::OnBnClickedNetkeyGet)
    ON_BN_CLICKED(IDC_APPKEY_ADD, &CConfig::OnBnClickedAppkeyAdd)
    ON_BN_CLICKED(IDC_APPKEY_DELETE, &CConfig::OnBnClickedAppkeyDelete)
    ON_BN_CLICKED(IDC_APPKEY_UPDATE, &CConfig::OnBnClickedAppkeyUpdate)
    ON_BN_CLICKED(IDC_APPKEY_GET, &CConfig::OnBnClickedAppkeyGet)
    ON_BN_CLICKED(IDC_MODEL_APP_BIND, &CConfig::OnBnClickedModelAppBind)
    ON_BN_CLICKED(IDC_MODEL_APP_UNBIND, &CConfig::OnBnClickedModelAppUnbind)
    ON_BN_CLICKED(IDC_MODEL_APP_GET, &CConfig::OnBnClickedModelAppGet)
    ON_BN_CLICKED(IDC_NODE_IDENTITY_GET, &CConfig::OnBnClickedNodeIdentityGet)
    ON_BN_CLICKED(IDC_NODE_IDENTITY_SET, &CConfig::OnBnClickedNodeIdentitySet)
    ON_BN_CLICKED(IDC_HEALTH_FAULT_GET, &CConfig::OnBnClickedHealthFaultGet)
    ON_BN_CLICKED(IDC_HEALTH_FAULT_CLEAR, &CConfig::OnBnClickedHealthFaultClear)
    ON_BN_CLICKED(IDC_HEALTH_FAULT_TEST, &CConfig::OnBnClickedHealthFaultTest)
    ON_BN_CLICKED(IDC_HEALTH_PERIOD_GET, &CConfig::OnBnClickedHealthPeriodGet)
    ON_BN_CLICKED(IDC_HEALTH_PERIOD_SET, &CConfig::OnBnClickedHealthPeriodSet)
    ON_BN_CLICKED(IDC_HEALTH_ATTENTION_GET, &CConfig::OnBnClickedHealthAttentionGet)
    ON_BN_CLICKED(IDC_HEALTH_ATTENTION_SET, &CConfig::OnBnClickedHealthAttentionSet)
    ON_BN_CLICKED(IDC_PROVISION_CONNECT, &CConfig::OnBnClickedProvisionConnect)
    ON_BN_CLICKED(IDC_LOCAL_SET, &CConfig::OnBnClickedLocalSet)
    ON_BN_CLICKED(IDC_SCAN_UNPROVISIONED, &CConfig::OnBnClickedScanUnprovisioned)
    ON_BN_CLICKED(IDC_PROVISION_DISCONNECT, &CConfig::OnBnClickedProvisionDisconnect)
    ON_BN_CLICKED(IDC_PROVISION_START, &CConfig::OnBnClickedProvisionStart)
    ON_CBN_SELCHANGE(IDC_AUTH_METHOD, &CConfig::OnCbnSelchangeAuthMethod)
    ON_BN_CLICKED(IDC_HEARTBEAT_PUBLICATION_GET, &CConfig::OnBnClickedHeartbeatPublicationGet)
    ON_BN_CLICKED(IDC_HEARTBEAT_SUBSCRIPTION_GET, &CConfig::OnBnClickedHeartbeatSubscriptionGet)
    ON_BN_CLICKED(IDC_LPN_POLL_TIMEOUT_GET, &CConfig::OnBnClickedLpnPollTimeoutGet)
    ON_BN_CLICKED(IDC_KEY_REFRESH_PHASE_GET, &CConfig::OnBnClickedKeyRefreshPhaseGet)
    ON_BN_CLICKED(IDC_KEY_REFRESH_PHASE_SET, &CConfig::OnBnClickedKeyRefreshPhaseSet)
    ON_BN_CLICKED(IDC_CONNECT_PROXY, &CConfig::OnBnClickedConnectProxy)
    ON_BN_CLICKED(IDC_FIND_PROXY, &CConfig::OnBnClickedFindProxy)
    ON_BN_CLICKED(IDC_DISCONNECT_PROXY, &CConfig::OnBnClickedDisconnectProxy)
    ON_BN_CLICKED(IDC_PROXY_FILTER_TYPE_SET, &CConfig::OnBnClickedProxyFilterTypeSet)
    ON_BN_CLICKED(IDC_PROXY_FILTER_ADDR_ADD, &CConfig::OnBnClickedProxyFilterAddrAdd)
    ON_BN_CLICKED(IDC_PROXY_FILTER_ADDR_DELETE, &CConfig::OnBnClickedProxyFilterAddrDelete)
    ON_BN_CLICKED(IDC_NODE_GATT_CONNECT, &CConfig::OnBnClickedNodeGattConnect)
    ON_BN_CLICKED(IDC_SCAN_EXTENDED, &CConfig::OnBnClickedScanExtended)
END_MESSAGE_MAP()
extern BYTE ProcNibble(char n);
extern DWORD GetHexValue(char *szbuf, LPBYTE buf, DWORD buf_size);
extern DWORD GetHexValueById(HWND hWnd, DWORD id, LPBYTE buf, DWORD buf_size);

void CConfig::OnCbnSelchangeComPort()
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

        m_ComHelper->OpenPort(ComPort, baud);
    }
}

int CConfig::GetBaudRateSelection()
{
    CComboBox *m_cbBaud = (CComboBox *)GetDlgItem(IDC_COM_BAUD);
    int select = m_cbBaud->GetItemData(m_cbBaud->GetCurSel());

    if (select >= 0)
        return as32BaudRate[select];

    return as32BaudRate[0];
}

void CConfig::ProcessData(DWORD opcode, LPBYTE p_data, DWORD len)
{
    WCHAR   trace[1024];
    switch (opcode)
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
        //MultiByteToWideChar(CP_ACP, 0, (const char *)p_data, len, trace, sizeof(trace) / sizeof(WCHAR));
        //m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_EVENT_HCI_TRACE:
        TraceHciPkt(p_data[0] + 1, &p_data[1], (USHORT)(len - 1));
        break;
    case HCI_CONTROL_MESH_EVENT_COMMAND_STATUS:
        wsprintf(trace, L"Mesh Command Status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
#ifdef MIBLE
    default:
        wiced_hci_process_data((uint16_t)opcode, p_data, (uint16_t)len);
        break;
#else
    case HCI_CONTROL_MESH_EVENT_CORE_SEQ_CHANGED:
    case HCI_CONTROL_MESH_EVENT_TX_COMPLETE:
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_CAPABILITIES_STATUS:
        ProcessScanCapabilitiesStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_STATUS:
        ProcessScanStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_REPORT:
        ProcessScanReport(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_EXTENDED_REPORT:
        ProcessExtendedScanReport(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROXY_DEVICE_NETWORK_DATA:
        ProcessProxyDeviceNetworkData(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_END:
        ProcessProvisionEnd(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_LINK_REPORT:
        ProcessProvisionLinkReport(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_DEVICE_CAPABILITIES:
        ProcessProvisionDeviceCapabilities(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_OOB_DATA:
        ProcessProvisionOobDataRequest(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROXY_CONNECTION_STATUS:
        ProcessProxyConnectionStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NODE_RESET_STATUS:
        ProcessNodeResetStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_COMPOSITION_DATA_STATUS:
        ProcessCompositionDataStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_FRIEND_STATUS:
        ProcessFriendStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_KEY_REFRESH_PHASE_STATUS:
        ProcessKeyRefreshPhaseStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_GATT_PROXY_STATUS:
        ProcessGattProxyStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_RELAY_STATUS:
        ProcessRelayStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_DEFAULT_TTL_STATUS:
        ProcessDefaultTtlStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_BEACON_STATUS:
        ProcessBeaconStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_PUBLICATION_STATUS:
        ProcessModelPublicationStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_STATUS:
        ProcessModelSubscriptionStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_LIST:
        ProcessModelSubscriptionList(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NETKEY_STATUS:
        ProcessNetKeyStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NETKEY_LIST:
        ProcessNetKeyList(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_APPKEY_STATUS:
        ProcessAppKeyStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_APPKEY_LIST:
        ProcessAppKeyList(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_APP_BIND_STATUS:
        ProcessModelAppStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_APP_LIST:
        ProcessModelAppList(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NODE_IDENTITY_STATUS:
        ProcessNodeIdentityStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEARTBEAT_SUBSCRIPTION_STATUS:
        ProcessHearbeatSubscriptionStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEARTBEAT_PUBLICATION_STATUS:
        ProcessHearbeatPublicationStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NETWORK_TRANSMIT_PARAMS_STATUS:
        ProcessNetworkTransmitParamsStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEALTH_CURRENT_STATUS:
        ProcessHealthCurrentStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEALTH_FAULT_STATUS:
        ProcessHealthFaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEALTH_PERIOD_STATUS:
        ProcessHealthPeriodStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_HEALTH_ATTENTION_STATUS:
        ProcessHealthAttentionStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LPN_POLL_TIMEOUT_STATUS:
        ProcessLpnPollTimeoutStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROXY_FILTER_STATUS:
        ProcessProxyFilterStatus(p_data, len);
        break;
    default:
        wsprintf(trace, L"Rcvd Unknown Op Code: 0x%04x", opcode);
        m_trace->SetCurSel(m_trace->AddString(trace));
#endif
    }
}

void CConfig::ProcessScanCapabilitiesStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[180];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Scan Info Status from %04x max items:%d", p_event->src, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::ProcessScanStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[180];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Scan Status from %04x status:%d state:%d max items:%d timeout:%d", p_event->src, p_data[0], p_data[1], p_data[2], p_data[3]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::ProcessScanReport(LPBYTE p_data, DWORD len)
{
    WCHAR buf[180];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    WCHAR uuid[50] = { 0 };
    uint16_t provisioner_addr = p_event->src;
    int8_t   rssi = p_data[0];
    for (int i = 1; i < 17; i++)
        wsprintf(&uuid[wcslen(uuid)], L"%02x ", p_data[i]);
    SetDlgItemText(IDC_PROVISION_UUID, uuid);
    uint16_t oob = p_data[17] + (p_data[18] << 8);
    uint32_t uri_hash = p_data[19] + ((uint32_t)p_data[20] << 8) + ((uint32_t)p_data[21] << 16) + ((uint32_t)p_data[22] << 24);

    wsprintf(buf, L"From %04x RSSI:%d Unprovisioned Device UUID:", provisioner_addr, rssi);
    for (int i = 1; i < 17; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);

    wsprintf(&buf[wcslen(buf)], L"OOB:%x URI hash:%x", oob, uri_hash);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::ProcessExtendedScanReport(LPBYTE p_data, DWORD len)
{
    WCHAR buf[180];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    uint16_t provisioner_addr = p_event->src;
    uint8_t  status = p_data[0];

    wsprintf(buf, L"From %04x status:%d Unprovisioned Device UUID:", provisioner_addr, status);
    for (int i = 1; i < 17; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);

    m_trace->SetCurSel(m_trace->AddString(buf));

    p_data += 17;

    while (p_data[0] != 0)
    {
        int len = p_data[1];
        wsprintf(buf, L"ADV %02x Len:%d ", p_data[1], p_data[0]);
        for (int i = 0; i < p_data[1]; i++)
            wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i + 2]);

        m_trace->SetCurSel(m_trace->AddString(buf));
        p_data += p_data[0] + 1;
    }
}

void CConfig::ProcessProxyDeviceNetworkData(LPBYTE p_data, DWORD len)
{
    WCHAR buf[180] = { 0 };
    wsprintf(buf, L"Proxy Device BD_ADDR:%02x%02x%02x%02x%02x%02x Type:%d NetKeyIdx:%x RSSI:%d",
        p_data[5], p_data[4], p_data[3], p_data[2], p_data[1], p_data[0], p_data[6], p_data[8] + (p_data[9] << 8), p_data[7]);

    m_trace->SetCurSel(m_trace->AddString(buf));

    wsprintf(buf, L"%02x%02x%02x%02x%02x%02x", p_data[5], p_data[4], p_data[3], p_data[2], p_data[1], p_data[0]);
    SetDlgItemText(IDC_BD_ADDR, buf);
    SetDlgItemInt(IDC_BD_ADDR_TYPE, p_data[6]);
}

void CConfig::ProcessProvisionEnd(LPBYTE p_data, DWORD len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    free(p_event);
    WCHAR buf[80];
    USHORT provisioner_addr = p_data[0] + ((USHORT)p_data[1] << 8);
    USHORT addr = p_data[2] + ((USHORT)p_data[3] << 8);
    USHORT net_key_idx = p_data[4] + ((USHORT)p_data[5] << 8);
    wsprintf(buf, L"Provision: End from:%x Addr:%x net_key_idx:%x result:%x", provisioner_addr, addr, net_key_idx, p_data[6]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    memcpy(dev_key, &p_data[7], 16);
    wcscpy(buf, L"DevKey: ");
    for (int i = 0; i < 16; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", dev_key[i]);
    m_trace->SetCurSel(m_trace->AddString(buf));

    wiced_bt_mesh_set_dev_key_data_t data;
    data.dst = addr;
    memcpy(data.dev_key, dev_key, 16);
    data.net_key_idx = net_key_idx;
    wiced_bt_mesh_provision_set_dev_key(&data);
}

void CConfig::ProcessProvisionLinkReport(LPBYTE p_data, DWORD len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    WCHAR buf[160];
    wsprintf(buf, L"Provision: Link Report from:%04x status:%d remote provisioner state:%d reason:%d over_gatt:%d", p_event->src, p_data[0], p_data[1], p_data[2], p_data[3]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessProvisionDeviceCapabilities(LPBYTE p_data, DWORD len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    free(p_event);
    WCHAR buf[280];
    USHORT provisioner_addr = p_data[0] + ((USHORT)p_data[1] << 8);
    BYTE elements_num = p_data[2];
    USHORT algorithms = p_data[3] + ((USHORT)p_data[4] << 8);
    pub_key_type = p_data[5];
    BYTE static_oob_type = p_data[6];
    BYTE output_oob_size = p_data[7];
    USHORT output_oob_action = p_data[8] + ((USHORT)p_data[9] << 8);
    BYTE input_oob_size = p_data[10];
    USHORT input_oob_action = p_data[11] + ((USHORT)p_data[12] << 8);
    wsprintf(buf, L"Provision: Device Capabilities from:%x Num elements:%x algo:%x pub key type:%d oob type:%d oob size:%d oob_action:%d input oob size:%d action:%d",
        provisioner_addr, elements_num, algorithms, pub_key_type, static_oob_type, output_oob_size, output_oob_action, input_oob_size, input_oob_action);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::ProcessProvisionOobDataRequest(LPBYTE p_data, DWORD len)
{
    wiced_bt_mesh_event_t* p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t*)&len);
    if (p_event == NULL)
        return;
    free(p_event);
    WCHAR buf[280];
    USHORT provisioner_addr = p_data[0] + ((USHORT)p_data[1] << 8);
    BYTE static_oob_type = p_data[2];
    BYTE oob_size = p_data[3];
    BYTE oob_action = p_data[4];
    BYTE oob_data_size;
    BYTE oob_data[64];

    if ((static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_OUTPUT) && (oob_action < 5))
        wsprintf(buf, L"Provision: Get OOB Data oob type:%s oob size:%d oob_action:%s", wchOobType[static_oob_type], oob_size, wchOutputOobAction[oob_action]);
    else if ((static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_INPUT) && (oob_action < 4))
        wsprintf(buf, L"Provision: Get OOB Data oob type:%s oob size:%d oob_action:%s", wchOobType[static_oob_type], oob_size, wchInputOobAction[oob_action]);
    else
        wsprintf(buf, L"Provision: Get OOB Data oob type:%s oob size:%d oob_action:%d", wchOobType[static_oob_type], oob_size, oob_action);

    m_trace->SetCurSel(m_trace->AddString(buf));

    if (static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_STOP)
        return;

    if (static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_OUTPUT)
    {
        wsprintf(buf, L"OOB Data ");
        for (int i = 5; i < 5 + oob_size; i++)
        {
            wsprintf(&buf[wcslen(buf)], L"%d ", p_data[i]);
        }
        m_trace->SetCurSel(m_trace->AddString(buf));
        return;
    }

    if (static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_STATIC)
    {
        // Get static OOB data from the UI
        BYTE auth_method = ((CComboBox*)GetDlgItem(IDC_AUTH_METHOD))->GetCurSel();
        oob_data_size = 0;
        if(auth_method == 1)
            oob_data_size = (uint8_t)GetHexValueById(m_hWnd, IDC_OOB_DATA_CFG, oob_data, 16);
    }
    else if (static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_DISPLAY_INPUT)
    {
        oob_data_size = 1;
        oob_data[0] = rand() & 0x07;
        wsprintf(buf, L"Provision: OOB input %d", oob_data[0]);
        if (auth_action == 3)
        {
            // alphanumeric
            oob_data[0] = 0x30 + oob_data[0];
        }
        m_trace->SetCurSel(m_trace->AddString(buf));
    }
    else if (static_oob_type == WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_PUB_KEY)
    {
        char szbuf[130];
        GetDlgItemTextA(m_hWnd, IDC_DEVICE_PUB_KEY, szbuf, 130);
        DWORD len = GetHexValue(szbuf, oob_data, 64);
        if (len != 64)
        {
            MessageBox(L"Public Key should be 64 octets in length");
            return;
        }
        oob_data_size = 64;
    }
    else
    {
        COutputOob dlg;
        dlg.m_oob_type = static_oob_type;
        dlg.m_oob_size = oob_size;
        dlg.m_oob_action = oob_action;

        if (dlg.DoModal() != IDOK)
            return;

        oob_data_size = (BYTE)((strlen(dlg.m_output_value) + 1) / 2);
        DWORD val;
        for (int i = 0; i < oob_data_size; i++)
        {
            sscanf(&dlg.m_output_value[2 * i], "%02d", &val);
            oob_data[i] = val & 0xff;
        }
        wsprintf(buf, L"Provision: OOB data: %d", oob_data[0]);
        m_trace->SetCurSel(m_trace->AddString(buf));
    }
    USHORT dst = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, dst, 0xffff);

    wiced_bt_mesh_provision_client_set_oob(p_event, oob_data, oob_data_size);
}

void CConfig::ProcessProxyConnectionStatus(LPBYTE p_data, DWORD len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    free(p_event);
    WCHAR buf[80];
    wsprintf(buf, L"Proxy connection status:%d", p_data[4]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}


void CConfig::ProcessNodeResetStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Node Reset Status from:%x", p_event->src);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

BYTE GetNumElements(BYTE *p_data, USHORT len)
{
    BYTE elem_idx = 0;
    while (len)
    {
        if (len < 4)
            return elem_idx;

        USHORT location = p_data[0] + (p_data[1] << 8);
        BYTE num_models = p_data[2];
        BYTE num_vs_models = p_data[3];
        p_data += 4;
        len -= 4;

        if (len < 2 * num_models + 4 * num_vs_models)
            return elem_idx;

        len    -= (2 * num_models + 4 * num_vs_models);
        p_data += (2 * num_models + 4 * num_vs_models);

        elem_idx++;
    }
    return elem_idx;
}

void CConfig::ProcessCompositionDataStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[300];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Composition Data from:%x Page:%d Len:%d ", p_event->src, p_data[0], len - 1);
    m_trace->SetCurSel(m_trace->AddString(buf));
    for (DWORD i = 1; i < len; i++)
    {
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    p_data += 1;
    len -= 1;

    if (m_state == STATE_LOCAL_GET_COMPOSITION_DATA)
    {
        if (p_local_composition_data)
            free(p_local_composition_data);
        p_local_composition_data = (BYTE *)malloc(len);
        local_composition_data_len = (USHORT)len;
        memcpy(p_local_composition_data, p_data, len);
    }
    else if (m_state == STATE_REMOTE_GET_COMPOSITION_DATA)
    {
        if (p_remote_composition_data)
            free(p_remote_composition_data);
        p_remote_composition_data = (BYTE *)malloc(len);
        remote_composition_data_len = (USHORT)len;
        memcpy(p_remote_composition_data, p_data, len);
    }

    wsprintf(buf, L"Compony ID %04x PID:%04x VID:%x CRPL:%04x Features:%04x\n",
        p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8), p_data[4] + (p_data[5] << 8), p_data[6] + (p_data[7] << 8), p_data[8] + (p_data[9] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    p_data += 10;
    len -= 10;

    BYTE num_elements = GetNumElements(p_data, (USHORT)len);

    for (BYTE elem_idx = 0; elem_idx < num_elements; elem_idx++)
    {
        USHORT location = p_data[0] + (p_data[1] << 8);
        BYTE num_models = p_data[2];
        BYTE num_vs_models = p_data[3];
        p_data += 4;
        len -= 4;

        wsprintf(buf, L"element:%d loc %04x num models:%d num_vs_models:%d", elem_idx, location, num_models, num_vs_models);
        m_trace->SetCurSel(m_trace->AddString(buf));
        if (len == 0)
            break;

        buf[0] = 0;
        for (BYTE i = 0; i < num_models && len >= 2; i++)
        {
            wsprintf(&buf[wcslen(buf)], L"%04x ", p_data[0] + (p_data[1] << 8));
            p_data += 2;
            len -= 2;
        }
        m_trace->SetCurSel(m_trace->AddString(buf));

        if (len == 0)
            break;

        if (num_vs_models)
        {
            buf[0] = 0;
            for (BYTE i = 0; i < num_vs_models && len >= 4; i++)
            {
                wsprintf(&buf[wcslen(buf)], L"%04x:%04x ", (p_data[0] << 8) + p_data[1], (p_data[2] << 8) + p_data[3]);
                p_data += 4;
                len -= 4;
            }
            m_trace->SetCurSel(m_trace->AddString(buf));
        }
    }
    free(p_event);
}

void CConfig::ProcessFriendStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Friend Status from:%x state:%d", p_event->src, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessKeyRefreshPhaseStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    BYTE status = p_data[0];
    USHORT net_key_inx = p_data[1] + ((USHORT)p_data[2] << 8);
    wsprintf(buf, L"Key Refresh Phase Status from:%x Status:%d NetKeyIdx:%x state:%d", p_event->src, status, net_key_inx, p_data[3]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessGattProxyStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"GATT Proxy Status from:%x state:%d", p_event->src, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessRelayStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Relay Status from:%x state:%d count:%d interval:%d", p_event->src, p_data[0], p_data[1], p_data[2] + ((USHORT)p_data[3] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessDefaultTtlStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Default TTL Status from:%x TTL:%d", p_event->src, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessNodeIdentityStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Node Identity Status from:%x status:%d net key idx:%d identity:%d", p_event->src, p_data[0], p_data[1] + ((USHORT)p_data[2] << 8), p_data[3]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessBeaconStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Beacon Status from:%x state:%d\n", p_event->src, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessModelPublicationStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Model Pub Status from:%x status:%d ElementAddr:%x Model ID: %x PublishAddr:%x app_key_idx:%x cred_flag:%d TTL:%d Period:%d Retransmit Count/Interval %d/%d",
        p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8),
        p_data[3] + ((ULONG)p_data[4] << 8) + ((ULONG)p_data[5] << 16) + ((ULONG)p_data[6] << 24),
        p_data[7] + ((ULONG)p_data[8] << 8), p_data[9] + ((ULONG)p_data[10] << 8), p_data[11], p_data[12],
        p_data[13] + ((ULONG)p_data[14] << 8) + ((ULONG)p_data[15] << 16) + ((ULONG)p_data[16] << 24),
        p_data[17], p_data[18] + ((ULONG)p_data[19] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessModelSubscriptionStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Model Sub Status from:%x status:%d Element Addr:%x Comp ID:%x Model ID:%x Addr:%x", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8),
        p_data[3] + ((USHORT)p_data[4] << 8), (USHORT)p_data[5] + ((USHORT)p_data[6] << 8),
        p_data[7] + ((USHORT)p_data[8] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessModelSubscriptionList(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Model Sub List from:%x status:%d Element Addr:%x Company:%x Model ID:%x Addresses:", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8),
        p_data[3] + ((USHORT)p_data[4] << 8), (USHORT)p_data[5] + ((USHORT)p_data[6] << 8));
    len -= 7;
    p_data += 7;
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data += 2;
        len -= 2;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessNetKeyStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"NetKey Status from:%x status:%d NetKey Index:%x", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessNetKeyList(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"NetKey List from:%x Addresses:", p_event->src);
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data += 2;
        len -= 2;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessAppKeyStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"AppKey Status from:%x status:%d NetKey Index:%x ApptKey Index:%x", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8), p_data[3] + ((ULONG)p_data[4] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessAppKeyList(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"AppKey List from:%x status:%x NetKeyIdx:%x Addresses:", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8));
    len -= 3;
    p_data += 3;
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data += 2;
        len -= 2;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessModelAppStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Model App Bind Status from:%x status:%d Element Addr:%x Company:%x Model ID:%x ApptKey Index:%x", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8),
        p_data[3] + ((USHORT)p_data[4] << 8), (USHORT)p_data[5] + ((USHORT)p_data[6] << 8),
        p_data[7] + ((ULONG)p_data[8] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessModelAppList(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Model App List from:%x status:%x Element Addr:%x Company:%x Model ID:%x App Keys: ", p_event->src, p_data[0], p_data[1] + ((ULONG)p_data[2] << 8),
        p_data[3] + ((USHORT)p_data[4] << 8), (USHORT)p_data[5] + ((USHORT)p_data[6] << 8));
    len -= 7;
    p_data += 7;
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data += 2;
        len -= 2;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHearbeatSubscriptionStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"subs status src:%x status:%d subs src/dst:%x/%x period:%d count:%d hops min/max:%d/%d\n",
        p_event->src, p_data[0], p_data[1] + ((USHORT)p_data[2] << 8), p_data[3] + ((USHORT)p_data[4] << 8),
        p_data[5] + ((ULONG)p_data[6] << 8) + ((ULONG)p_data[7] << 16) + ((ULONG)p_data[8] << 24),
        p_data[9] + ((USHORT)p_data[10] << 8), p_data[11], p_data[12]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHearbeatPublicationStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[200];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"pubs status src:%x status:%d pubs dst:%x period:%d count:%d ttl:%d relay:%d proxy:%d friend:%d low power:%d net_key_idx:%d\n",
        p_event->src, p_data[0], p_data[1] + ((USHORT)p_data[2] << 8),
        p_data[3] + ((ULONG)p_data[4] << 8) + ((ULONG)p_data[5] << 16) + ((ULONG)p_data[6] << 24),
        p_data[7] + ((USHORT)p_data[8] << 8), p_data[11],
        p_data[12], p_data[13], p_data[14], p_data[15], p_data[16], p_data[17] + ((USHORT)p_data[18] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessNetworkTransmitParamsStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    wsprintf(buf, L"Network Transmit Params Status from:%x count:%d interval:%d", p_event->src, p_data[0],
        p_data[1] + ((ULONG)p_data[2] << 8) + ((ULONG)p_data[3] << 16) + ((ULONG)p_data[4] << 24));
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHealthCurrentStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    USHORT app_key_idx = p_data[0] + ((USHORT)p_data[1] << 8);
    wsprintf(buf, L"Health Current Status from:%x AppKeyIdx:%x Test ID:%d CompanyID:%x Faults: ", p_event->src, app_key_idx, p_data[2], p_data[3] + ((USHORT)p_data[4] << 8));
    len -= 5;
    p_data += 5;
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data ++;
        len --;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHealthFaultStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    USHORT app_key_idx = p_data[0] + ((USHORT)p_data[1] << 8);
    wsprintf(buf, L"Health Fault Status from:%x AppKeyIdx:%x Test ID:%d CompanyID:%x Faults: ", p_event->src, app_key_idx, p_data[2], p_data[3] + ((USHORT)p_data[4] << 8));
    len -= 5;
    p_data += 5;
    while (len)
    {
        wsprintf(&buf[wcslen(buf)], L"0x%04x ", p_data[0] + ((USHORT)p_data[1] << 8));
        p_data++;
        len--;
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHealthPeriodStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    USHORT app_key_idx = p_data[0] + ((USHORT)p_data[1] << 8);
    wsprintf(buf, L"Health Period Status from:%x AppKeyIdx:%x Divisor:%d", p_event->src, app_key_idx, p_data[2]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessHealthAttentionStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    USHORT app_key_idx = p_data[0] + ((USHORT)p_data[1] << 8);
    wsprintf(buf, L"Health Attention Status from:%x AppKeyIdx:%x Timer:%d", p_event->src, app_key_idx, p_data[2]);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessLpnPollTimeoutStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, (uint16_t *)&len);
    if (p_event == NULL)
        return;
    USHORT lpn_addr = p_data[0] + ((USHORT)p_data[1] << 8);
    USHORT poll_timeout = p_data[2] + ((ULONG)p_data[3] << 8) + ((ULONG)p_data[4] << 16) + ((ULONG)p_data[5] << 24);
    wsprintf(buf, L"LPN Poll Timeout Status from:%x Addr:%x PollTimeout:%d", p_event->src, lpn_addr, poll_timeout);
    m_trace->SetCurSel(m_trace->AddString(buf));
    free(p_event);
}

void CConfig::ProcessProxyFilterStatus(LPBYTE p_data, DWORD len)
{
    WCHAR buf[80];
    BYTE  type = p_data[0];
    USHORT list_size = p_data[1] + ((USHORT)p_data[2] << 8);
    wsprintf(buf, L"Proxy Filter Status Type:%x List Size:%d", type, list_size);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::SetDlgItemHex(DWORD id, DWORD val)
{
    WCHAR buf[10];
    wsprintf(buf, L"%x", val);
    SetDlgItemText(id, buf);
}

// CConfig message handlers
DWORD CConfig::GetHexValueInt(DWORD id)
{
    DWORD ret = 0;
    BYTE buf[32];
    char szbuf[100];
    char *psz = szbuf;
    BYTE *pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, 32);

    GetDlgItemTextA(m_hWnd, id, szbuf, 100);

    DWORD len = GetHexValue(szbuf, buf, sizeof(buf));
    for (DWORD i = 0; i<len; i++)
    {
        ret = (ret << 8) + buf[i];
    }
    return ret;
}

#ifdef MIBLE
extern "C" int mible_mesh_event_cb(mible_mesh_event_t event, void *data)
{
    Log(L"mible_mesh_event_cb:%d\n", event);
    return 0;
}

uint32_t sig_model_list[] = { 0x1101, 0x1301, 0x1306, 0x1307 };
#endif

void CConfig::OnBnClickedLocalSet()
{
    USHORT addr = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NET_KEY_IDX);
    ULONG  iv_idx = (USHORT)GetHexValueInt(IDC_IV_INDEX);
    BYTE   key_refresh = (BYTE)((CButton *)GetDlgItem(IDC_KEY_REFRESH_PHASE2))->GetCheck();
    BYTE   iv_update = (BYTE)((CButton *)GetDlgItem(IDC_IV_UPDATE))->GetCheck();
    BYTE   net_key[16];
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_NET_KEY, szbuf, 100);
    DWORD len = GetHexValue(szbuf, net_key, 16);

#ifdef MIBLE
    mible_mesh_gateway_info_t info;
    info.unicast_address = 0x101;
    info.netkey_index = 0xaa;
    info.appkey_index = 0xbb;
    info.default_ttl = 11;
    info.flags = 0;
    info.group_address = 0xc000;
    info.iv_index = 300;
    info.max_num_appkey = 8;
    info.max_num_netkey = 4;
    memset(info.primary_netkey, 0x11, 16);
    memset(info.primary_appkey, 0x22, 16);
    info.replay_list_size = 200;
    info.sig_model_db.model_num = sizeof(sig_model_list) / sizeof(sig_model_list[0]);
    info.sig_model_db.model_list = sig_model_list;
    info.vendor_model_db.model_num = 0;

    mible_mesh_init_stack(mible_mesh_event_cb, NULL);

    mible_mesh_load_config(&info);
#else
    wiced_bt_mesh_local_device_set_data_t set;
    memset(&set, 0, sizeof(set));

    set.addr = addr;
    //memcpy(set.dev_key, node->device_key, 16);
    memcpy(set.network_key, net_key, 16);
    set.net_key_idx = net_key_idx;
    set.key_refresh = key_refresh;

//    download_iv(&set.iv_idx, &set.iv_update);

    m_trace->SetCurSel(m_trace->AddString(L"Set Local Device"));
//    Log("Set Local Device addr:0x%04x net_key_idx:%04x iv_idx:%d key_refresh:%d iv_updata:%d", set.addr, set.net_key_idx, set.iv_idx, set.key_refresh, set.iv_update);

    wiced_bt_mesh_provision_local_device_set(&set);
#endif
}

void CConfig::OnBnClickedScanUnprovisioned()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, dst, 0xffff);
    BYTE uuid[16];
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_PROVISION_UUID, szbuf, 100);
    DWORD len = GetHexValue(szbuf, uuid, 16);

    static BYTE scanning = FALSE;
    if (!scanning)
    {
        wiced_bt_mesh_provision_scan_start_data_t data;
        memset(&data, 0, sizeof(wiced_bt_mesh_provision_scan_start_data_t));
        if (len == 16)
        {
            memcpy(data.uuid, uuid, sizeof(data.uuid));
            data.scanned_items_limit = 1;
            data.scan_single_uuid = 1;
            data.timeout = 10;
        }
        else
        {
            data.scanned_items_limit = 10;
            data.timeout = 10;
        }
        SetDlgItemText(IDC_SCAN_UNPROVISIONED, L"Stop Scanning");
        scanning = TRUE;
        wiced_bt_mesh_provision_scan_start(p_event, &data);
    }
    else
    {
        SetDlgItemText(IDC_SCAN_UNPROVISIONED, L"Scan Unprovisioned");
        scanning = FALSE;
        wiced_bt_mesh_provision_scan_stop(p_event);
    }
    WCHAR buf[128];
    wsprintf(buf, L"scan unprovisioned:%d", scanning);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CConfig::OnBnClickedScanExtended()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, dst, 0xffff);
    BYTE uuid[16];
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_PROVISION_UUID, szbuf, 100);
    DWORD len = GetHexValue(szbuf, uuid, 16);

    wiced_bt_mesh_provision_scan_extended_start_t data;
    memset(&data, 0, sizeof(data));

    GetDlgItemTextA(m_hWnd, IDC_ADV_FILTER, szbuf, 100);
    data.num_ad_filters = (uint8_t)GetHexValue(szbuf, data.ad_filter_types, 7);
    if (len == 16)
    {
        data.uuid_present = 1;
        memcpy(data.uuid, uuid, sizeof(uuid));
        data.timeout = 5;
    }
    wiced_bt_mesh_provision_scan_extended_start(p_event, &data);
}

void CConfig::OnBnClickedProvisionConnect()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, dst, 0xffff);
    BYTE    identify_duration = (BYTE)GetHexValueInt(IDC_IDENTITY_DURATION);
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_PROVISION_UUID, szbuf, 100);
    BYTE    use_gatt = (BYTE)((CButton *)GetDlgItem(IDC_USE_GATT))->GetCheck();

    wiced_bt_mesh_provision_connect_data_t data;
    DWORD len = GetHexValue(szbuf, data.uuid, 16);
    data.identify_duration = identify_duration;
    data.procedure = WICED_BT_MESH_PROVISION_PROCEDURE_PROVISION;
    WCHAR buf[128];
    wsprintf(buf, L"Provision: addr:%x", dst);
    m_trace->SetCurSel(m_trace->AddString(buf));

    wiced_bt_mesh_provision_connect(p_event, &data, use_gatt);
}

void CConfig::OnBnClickedProvisionDisconnect()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, dst, 0xffff);

    WCHAR buf[128];
    wsprintf(buf, L"Provision: Disconnect");
    m_trace->SetCurSel(m_trace->AddString(buf));

    wiced_bt_mesh_provision_disconnect(p_event);
}

void CConfig::OnBnClickedProvisionStart()
{
    USHORT local_addr = (USHORT)GetHexValueInt(IDC_LOCAL_ADDR);
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, 0, local_addr, 0xffff);
    BYTE algorithm = 0;
    BYTE auth_method = ((CComboBox *)GetDlgItem(IDC_AUTH_METHOD))->GetCurSel();
    auth_action = 0;
    BYTE auth_size = 0;
    if ((auth_method == 2) || (auth_method == 3)) // authentication with Output OOB
    {
        auth_action = ((CComboBox *)GetDlgItem(IDC_AUTH_ACTION))->GetCurSel();
        auth_size = GetDlgItemInt(IDC_AUTH_SIZE);
        if ((auth_size == 0) || (auth_size > 8))
        {
            MessageBox(L"Auth size should be 1-8");
            return;
        }
    }

    // Save static OOB data in the user profile. Also save UseStaticOobData depending on auth_method
    CString sStaticOobData;
    GetDlgItemText(IDC_OOB_DATA_CFG, sStaticOobData);
    theApp.WriteProfileString(L"LightControl", L"StaticOobData", sStaticOobData);
    theApp.WriteProfileInt(L"LightControl", L"UseStaticOobData", auth_method == 1);

    wiced_bt_mesh_provision_start_data_t data;
    data.addr = dst;
    data.algorithm = algorithm;
    data.public_key_type = pub_key_type;    // use value passed in the capabilities
    data.auth_method = auth_method;
    data.auth_action = auth_action;
    data.auth_size = auth_size;
    data.net_key_idx = 0;       //ToDo: add UI to select net key index from the database. For now use primary net key (0)

    m_trace->SetCurSel(m_trace->AddString(L"Provision: Start"));
    wiced_bt_mesh_provision_start(p_event, &data);
}

void CConfig::OnCbnSelchangeAuthMethod()
{
    CComboBox *pCb = (CComboBox *)GetDlgItem(IDC_AUTH_ACTION);
    pCb->ResetContent();
    BYTE auth_method = ((CComboBox *)GetDlgItem(IDC_AUTH_METHOD))->GetCurSel();
    if (auth_method == 2)
    {
        pCb->AddString(L"Blink");
        pCb->AddString(L"Beep");
        pCb->AddString(L"Vibrate");
        pCb->AddString(L"Output Numeric");
        pCb->AddString(L"Output Alphanumeric");
        pCb->SetCurSel(0);
    }
    else if (auth_method == 3)
    {
        pCb->AddString(L"Push");
        pCb->AddString(L"Twist");
        pCb->AddString(L"Input Numeric");
        pCb->AddString(L"Input Alphanumeric");
        pCb->SetCurSel(0);
    }
}

void CConfig::OnBnClickedHeartbeatSubscriptionGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    BYTE    buffer[128];

    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Heartbeat Subscription Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHeartbeatSubscriptionSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT subsription_src = (USHORT)GetHexValueInt(IDC_HEARTBEAT_SUBSCRIPTION_SRC);
    USHORT subsription_dst = (USHORT)GetHexValueInt(IDC_HEARTBEAT_SUBSCRIPTION_DST);
    ULONG subsription_period = (ULONG)GetHexValueInt(IDC_HEARTBEAT_SUBSCRIPTION_PERIOD);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = subsription_src & 0xff;
    *p++ = (subsription_src >> 8) & 0xff;
    *p++ = subsription_dst & 0xff;
    *p++ = (subsription_dst >> 8) & 0xff;
    *p++ = subsription_period & 0xff;
    *p++ = (subsription_period >> 8) & 0xff;
    *p++ = (subsription_period >> 16) & 0xff;
    *p++ = (subsription_period >> 24) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Heartbeat Subscription Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHeartbeatPublicationGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Heartbeat Publication Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHeartbeatPublicationSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT publication_dst = (USHORT)GetHexValueInt(IDC_HEARTBEAT_PUBLICATION_DST);
    ULONG  publication_count = GetHexValueInt(IDC_HEARTBEAT_PUBLICATION_COUNT);
    ULONG  publication_period = GetHexValueInt(IDC_HEARTBEAT_PUBLICATION_PERIOD);
    BYTE   publication_feature_proxy = ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_PROXY)))->GetCheck();
    BYTE   publication_feature_relay = ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_RELAY)))->GetCheck();
    BYTE   publication_feature_friend = ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_FRIEND)))->GetCheck();
    BYTE   publication_feature_low_power_mode = ((CButton *)(GetDlgItem(IDC_HEARTBEAT_PUB_LOW_POWER_MODE)))->GetCheck();
    USHORT publication_net_key_idx = (USHORT)GetHexValueInt(IDC_HEARTBEAT_PUBLICATION_NET_KEY_IDX);
    BYTE   publication_ttl = (BYTE)GetHexValueInt(IDC_HEARTBEAT_PUBLICATION_TTL);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = publication_dst & 0xff;
    *p++ = (publication_dst >> 8) & 0xff;
    *p++ = publication_count & 0xff;
    *p++ = (publication_count >> 8) & 0xff;
    *p++ = (publication_count >> 16) & 0xff;
    *p++ = (publication_count >> 24) & 0xff;
    *p++ = publication_period & 0xff;
    *p++ = (publication_period >> 8) & 0xff;
    *p++ = (publication_period >> 16) & 0xff;
    *p++ = (publication_period >> 24) & 0xff;
    *p++ = publication_ttl;
    *p++ = publication_feature_relay;
    *p++ = publication_feature_proxy;
    *p++ = publication_feature_friend;
    *p++ = publication_feature_low_power_mode;
    *p++ = publication_net_key_idx & 0xff;
    *p++ = (publication_net_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Heartbeat Publication Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetworkTransmitIntervalGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Network Transmit Interval Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetworkTransmitIntervalSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE network_transmit_count = (BYTE)GetHexValueInt(IDC_NETWORK_TRANSMIT_COUNT);
    USHORT network_transmit_interval = (USHORT)GetHexValueInt(IDC_NETWORK_TRANSMIT_INTERVAL);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = network_transmit_count & 0xff;
    *p++ = network_transmit_interval & 0xff;
    *p++ = (network_transmit_interval >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Network Transmit Interval Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNodeReset()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));

    m_trace->SetCurSel(m_trace->AddString(L"Node Reset"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_RESET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedConfigDataGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE composition_data_page = (BYTE)GetHexValueInt(IDC_COMPOSITION_DATA_PAGE);
    m_trace->SetCurSel(m_trace->AddString(L"Composition Data Get"));
    SendGetCompositionData(dst, composition_data_page);
}

void CConfig::OnBnClickedBeaconGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Beacon Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedBeaconSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE beacon_state = (BYTE)((CComboBox *)(GetDlgItem(IDC_BEACON_STATE)))->GetCurSel();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = beacon_state & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Beacon Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedDefaultTtlGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Default TTL Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedDefaultTtlSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE default_ttl = (BYTE)GetHexValueInt(IDC_DEFAULT_TTL);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = default_ttl & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Default TTL Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedGattProxyGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"GATT Proxy Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedGattProxySet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE gatt_proxy_state = (BYTE)((CComboBox *)(GetDlgItem(IDC_GATT_PROXY_STATE)))->GetCurSel();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = gatt_proxy_state & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"GATT Proxy Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedRelayGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));

    m_trace->SetCurSel(m_trace->AddString(L"Relay Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedRelaySet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE relay_state = (BYTE)((CComboBox *)(GetDlgItem(IDC_RELAY_STATE)))->GetCurSel();
    BYTE relay_retransmit_count = (BYTE)GetHexValueInt(IDC_RELAY_TRANSMIT_COUNT);
    USHORT relay_retransmit_interval = (USHORT)GetHexValueInt(IDC_RELAY_TRANSMIT_INTERVAL);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = relay_state & 0xff;
    *p++ = relay_retransmit_count & 0xff;
    *p++ = relay_retransmit_interval & 0xff;
    *p++ = (relay_retransmit_interval >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Relay Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedFriendGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));

    m_trace->SetCurSel(m_trace->AddString(L"Friend Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedFriendSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    BYTE friend_state = (BYTE)((CComboBox *)(GetDlgItem(IDC_FRIEND_STATE)))->GetCurSel();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = friend_state & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Friend Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedKeyRefreshPhaseGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NET_KEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Key Refresh Phase Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedKeyRefreshPhaseSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NET_KEY_IDX);
    BYTE phase = (BYTE)GetHexValueInt(IDC_KEY_REFRESH_PHASE);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    *p++ = phase & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Key Refresh Phase Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelPubGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_pub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_PUB_ELEMENT_ADDR);
    USHORT model_pub_company_id = (USHORT)GetHexValueInt(IDC_MODEL_PUB_COMP_ID);
    USHORT model_pub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_PUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_pub_element_addr & 0xff;
    *p++ = (model_pub_element_addr >> 8) & 0xff;
    *p++ = model_pub_company_id & 0xff;
    *p++ = (model_pub_company_id >> 8) & 0xff;
    *p++ = model_pub_model_id & 0xff;
    *p++ = (model_pub_model_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model Publication Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelPubSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_pub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_PUB_ELEMENT_ADDR);
    USHORT model_pub_company_id = (USHORT)GetHexValueInt(IDC_MODEL_PUB_COMP_ID);
    USHORT model_pub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_PUB_MODEL_ID);
    ULONG  model_pub_period = GetHexValueInt(IDC_MODEL_PUB_PERIOD);
    USHORT model_pub_app_key_idx = (USHORT)GetHexValueInt(IDC_MODEL_PUB_APP_KEY_IDX);
    BYTE   model_pub_credential_flag = (BYTE)((CComboBox *)(GetDlgItem(IDC_MODEL_PUB_CREDENTIAL_FLAG)))->GetCurSel();
    BYTE   model_pub_ttl = (BYTE)GetHexValueInt(IDC_PUBLISH_TTL);
    BYTE   model_pub_retransmit_count = (BYTE)GetHexValueInt(IDC_MODEL_PUB_RETRANSMIT_COUNT);
    USHORT model_pub_retransmit_interval = (USHORT)GetHexValueInt(IDC_MODEL_PUB_RETRANSMIT_INTERVAL);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_pub_element_addr & 0xff;
    *p++ = (model_pub_element_addr >> 8) & 0xff;
    *p++ = model_pub_company_id & 0xff;
    *p++ = (model_pub_company_id >> 8) & 0xff;
    *p++ = model_pub_model_id & 0xff;
    *p++ = (model_pub_model_id >> 8) & 0xff;
    if (!((CButton *)GetDlgItem(IDC_USE_VIRTUAL_ADDR))->GetCheck())
    {
        USHORT model_pub_publish_addr = (USHORT)GetHexValueInt(IDC_MODEL_PUB_PUBLISH_ADDR);
        *p++ = model_pub_publish_addr & 0xff;
        *p++ = (model_pub_publish_addr >> 8) & 0xff;
        memset(p, 0, 14);
        p += 14;
    }
    else
    {
        memset(p, 0, 16);
        char szbuf[100];
        GetDlgItemTextA(m_hWnd, IDC_MODEL_PUB_VIRTUAL_ADDR, szbuf, 100);
        DWORD len = GetHexValue(szbuf, p, 16);
        p += 16;
    }
    *p++ = model_pub_app_key_idx & 0xff;
    *p++ = (model_pub_app_key_idx >> 8) & 0xff;
    *p++ = model_pub_credential_flag;
    *p++ = model_pub_ttl;
    *p++ = model_pub_period & 0xff;
    *p++ = (model_pub_period >> 8) & 0xff;
    *p++ = (model_pub_period >> 16) & 0xff;
    *p++ = (model_pub_period >> 24) & 0xff;
    *p++ = model_pub_retransmit_count;
    *p++ = model_pub_retransmit_interval & 0xff;
    *p++ = (model_pub_retransmit_interval >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model Publication Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelSubscriptionAdd()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_sub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ELEMENT_ADDR);
    USHORT model_sub_comp_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_COMP_ID);
    USHORT model_sub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_sub_element_addr & 0xff;
    *p++ = (model_sub_element_addr >> 8) & 0xff;
    *p++ = model_sub_comp_id & 0xff;
    *p++ = (model_sub_comp_id >> 8) & 0xff;
    *p++ = model_sub_model_id & 0xff;
    *p++ = (model_sub_model_id >> 8) & 0xff;
    if (!((CButton *)GetDlgItem(IDC_USE_VIRTUAL_ADDR2))->GetCheck())
    {
        USHORT model_sub_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ADDR);
        *p++ = model_sub_addr & 0xff;
        *p++ = (model_sub_addr >> 8) & 0xff;
        memset(p, 0, 14);
        p += 14;
    }
    else
    {
        memset(p, 0, 16);
        char szbuf[100];
        GetDlgItemTextA(m_hWnd, IDC_MODEL_SUB_VIRTUAL_ADDR, szbuf, 100);
        DWORD len = GetHexValue(szbuf, p, 16);
        p += 16;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Model Subscription Add"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_ADD, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelSubscriptionDelete()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_sub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ELEMENT_ADDR);
    USHORT model_sub_comp_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_COMP_ID);
    USHORT model_sub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_sub_element_addr & 0xff;
    *p++ = (model_sub_element_addr >> 8) & 0xff;
    *p++ = model_sub_comp_id & 0xff;
    *p++ = (model_sub_comp_id >> 8) & 0xff;
    *p++ = model_sub_model_id & 0xff;
    *p++ = (model_sub_model_id >> 8) & 0xff;
    if (!((CButton *)GetDlgItem(IDC_USE_VIRTUAL_ADDR2))->GetCheck())
    {
        USHORT model_sub_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ADDR);
        *p++ = model_sub_addr & 0xff;
        *p++ = (model_sub_addr >> 8) & 0xff;
        memset(p, 0, 14);
        p += 14;
    }
    else
    {
        memset(p, 0, 16);
        char szbuf[100];
        GetDlgItemTextA(m_hWnd, IDC_MODEL_SUB_VIRTUAL_ADDR, szbuf, 100);
        DWORD len = GetHexValue(szbuf, p, 16);
        p += 16;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Model Subscription Delete"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelSubscriptionOverwrite()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_sub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ELEMENT_ADDR);
    USHORT model_sub_comp_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_COMP_ID);
    USHORT model_sub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_sub_element_addr & 0xff;
    *p++ = (model_sub_element_addr >> 8) & 0xff;
    *p++ = model_sub_comp_id & 0xff;
    *p++ = (model_sub_comp_id >> 8) & 0xff;
    *p++ = model_sub_model_id & 0xff;
    *p++ = (model_sub_model_id >> 8) & 0xff;
    if (!((CButton *)GetDlgItem(IDC_USE_VIRTUAL_ADDR2))->GetCheck())
    {
        USHORT model_sub_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ADDR);
        *p++ = model_sub_addr & 0xff;
        *p++ = (model_sub_addr >> 8) & 0xff;
        memset(p, 0, 14);
        p += 14;
    }
    else
    {
        memset(p, 0, 16);
        char szbuf[100];
        GetDlgItemTextA(m_hWnd, IDC_MODEL_SUB_VIRTUAL_ADDR, szbuf, 100);
        DWORD len = GetHexValue(szbuf, p, 16);
        p += 16;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Model Subscription Overwrite"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelSubscriptionDeleteAll()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_sub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ELEMENT_ADDR);
    USHORT model_sub_comp_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_COMP_ID);
    USHORT model_sub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_sub_element_addr & 0xff;
    *p++ = (model_sub_element_addr >> 8) & 0xff;
    *p++ = model_sub_comp_id & 0xff;
    *p++ = (model_sub_comp_id >> 8) & 0xff;
    *p++ = model_sub_model_id & 0xff;
    *p++ = (model_sub_model_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model Subscription Delete All"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedSubscriptionGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT model_sub_element_addr = (USHORT)GetHexValueInt(IDC_MODEL_SUB_ELEMENT_ADDR);
    USHORT model_sub_comp_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_COMP_ID);
    USHORT model_sub_model_id = (USHORT)GetHexValueInt(IDC_MODEL_SUB_MODEL_ID);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = model_sub_element_addr & 0xff;
    *p++ = (model_sub_element_addr >> 8) & 0xff;
    *p++ = model_sub_comp_id & 0xff;
    *p++ = (model_sub_comp_id >> 8) & 0xff;
    *p++ = model_sub_model_id & 0xff;
    *p++ = (model_sub_model_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model Subscription Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetkeyAdd()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    memset(p, 0, 16);
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_NETKEY, szbuf, 100);
    DWORD len = GetHexValue(szbuf, p, 16);
    p += 16;
    m_trace->SetCurSel(m_trace->AddString(L"NetKey Add"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_ADD, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetkeyDelete()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"NetKey Delete"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_DELETE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetkeyUpdate()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    memset(p, 0, 16);
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_NETKEY, szbuf, 100);
    DWORD len = GetHexValue(szbuf, p, 16);
    p += 16;
    m_trace->SetCurSel(m_trace->AddString(L"NetKey Add"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_UPDATE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNetkeyGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"NetKey Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedAppkeyAdd()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APPKEY_IDX);
    BYTE key[16];
    memset(key, 0, 16);
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_APPKEY, szbuf, 100);
    DWORD len = GetHexValue(szbuf, key, 16);
    m_trace->SetCurSel(m_trace->AddString(L"AppKey Add"));
    SendAddAppKey(dst, net_key_idx, app_key_idx, key);
}

void CConfig::OnBnClickedAppkeyDelete()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APPKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"AppKey Delete"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_DELETE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedAppkeyUpdate()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APPKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    memset(p, 0, 16);
    char szbuf[100];
    GetDlgItemTextA(m_hWnd, IDC_APPKEY, szbuf, 100);
    DWORD len = GetHexValue(szbuf, p, 16);
    p += 16;
    m_trace->SetCurSel(m_trace->AddString(L"AppKey Update"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_UPDATE, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedAppkeyGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"AppKey Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelAppBind()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT element_addr = (USHORT)GetHexValueInt(IDC_MODEL_BIND_ELEMENT_ADDR);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_COMP_ID);
    USHORT model_id = (USHORT)GetHexValueInt(IDC_MODEL_ID);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = element_addr & 0xff;
    *p++ = (element_addr >> 8) & 0xff;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    *p++ = model_id & 0xff;
    *p++ = (model_id >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model App Bind"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelAppUnbind()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT element_addr = (USHORT)GetHexValueInt(IDC_MODEL_BIND_ELEMENT_ADDR);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_COMP_ID);
    USHORT model_id = (USHORT)GetHexValueInt(IDC_MODEL_ID);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = element_addr & 0xff;
    *p++ = (element_addr >> 8) & 0xff;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    *p++ = model_id & 0xff;
    *p++ = (model_id >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model App Unbind"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_UNBIND, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedModelAppGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT element_addr = (USHORT)GetHexValueInt(IDC_MODEL_BIND_ELEMENT_ADDR);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_COMP_ID);
    USHORT model_id = (USHORT)GetHexValueInt(IDC_MODEL_ID);
    USHORT app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = element_addr & 0xff;
    *p++ = (element_addr >> 8) & 0xff;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    *p++ = model_id & 0xff;
    *p++ = (model_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Model App Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNodeIdentityGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NODE_IDENTITY_NETKEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Node Identity Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedNodeIdentitySet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT net_key_idx = (USHORT)GetHexValueInt(IDC_NODE_IDENTITY_NETKEY_IDX);
    BYTE node_identity_state = (BYTE)((CComboBox *)(GetDlgItem(IDC_NODE_IDENTITY_STATE)))->GetCurSel();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    *p++ = node_identity_state;
    m_trace->SetCurSel(m_trace->AddString(L"Node Identity Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedLpnPollTimeoutGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT lpn_addr = (USHORT)GetHexValueInt(IDC_LPN_ADDR);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = lpn_addr & 0xff;
    *p++ = (lpn_addr >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"LPN Poll Timeout Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_LPN_POLL_TIMEOUT_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthFaultGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_HEALTH_FAULT_COMPANY_ID);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Health Fault Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthFaultClear()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_HEALTH_FAULT_COMPANY_ID);
    BYTE   reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Health Fault Clear"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_CLEAR, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthFaultTest()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);

    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    USHORT company_id = (USHORT)GetHexValueInt(IDC_HEALTH_FAULT_COMPANY_ID);
    BYTE   test_id = (BYTE)GetHexValueInt(IDC_HEALTH_FAULT_TEST_ID);
    BYTE   reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = test_id;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    m_trace->SetCurSel(m_trace->AddString(L"Health Fault Test"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_TEST, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthPeriodGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Health Period Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthPeriodSet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    BYTE   period = (BYTE)GetHexValueInt(IDC_HEALTH_PERIOD);
    BYTE   reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = period;
    m_trace->SetCurSel(m_trace->AddString(L"Health Period Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthAttentionGet()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_trace->SetCurSel(m_trace->AddString(L"Attention Get"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_GET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedHealthAttentionSet()
{
    BYTE   attention = (BYTE)GetHexValueInt(IDC_HEALTH_ATTENTION);
    BYTE   reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    USHORT app_key_idx = GetDlgItemInt(IDC_APP_KEY_IDX);
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = attention;
    m_trace->SetCurSel(m_trace->AddString(L"Attention Timer Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedFindProxy()
{
    static BYTE scanning = FALSE;
    if (!scanning)
    {
        SetDlgItemText(IDC_FIND_PROXY, L"Stop Scanning");
        scanning = TRUE;
    }
    else
    {
        SetDlgItemText(IDC_FIND_PROXY, L"Find Proxy");
        scanning = FALSE;
    }
    WCHAR buf[128];
    wsprintf(buf, L"Find proxy:%d", scanning);
    m_trace->SetCurSel(m_trace->AddString(buf));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SEARCH_PROXY, &scanning, 1);
}

void CConfig::OnBnClickedConnectProxy()
{
    BYTE    buffer[128];
    LPBYTE  p = buffer;
    *p++ = 10; // 10 seconds scan duration
#if 0
    memset(p, 0, 6);
    char szbuf[100];
    BYTE bd_addr[6];
    GetDlgItemTextA(m_hWnd, IDC_BD_ADDR, szbuf, 100);
    DWORD len = GetHexValue(szbuf, bd_addr, 6);
    for (int i = 0; i < 6; i++)
        *p++ = bd_addr[5 - i];
    *p++ = GetDlgItemInt(IDC_BD_ADDR_TYPE);
#endif
    m_trace->SetCurSel(m_trace->AddString(L"Proxy: Connect"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_CONNECT, buffer, (DWORD)(p - buffer));
}


void CConfig::OnBnClickedDisconnectProxy()
{
    m_trace->SetCurSel(m_trace->AddString(L"Proxy: Disconnect"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_DISCONNECT, NULL, 0);
}

void CConfig::OnBnClickedNodeGattConnect()
{
    USHORT dst = (USHORT)GetHexValueInt(IDC_DST);
    BYTE    buffer[128];
    LPBYTE  p = buffer;
    *p++ = dst & 0xff;
    *p++ = (dst >> 8) & 0xff;
    *p++ = 10; // 10 seconds scan duration
    m_trace->SetCurSel(m_trace->AddString(L"Node GATT: Connect"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_CONNECT, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedProxyFilterTypeSet()
{
    BYTE type = (BYTE)GetHexValueInt(IDC_PROXY_FILTER_TYPE);
    BYTE    buffer[128];
    // special case for filter dst = 0
    LPBYTE p =wiced_bt_mesh_format_hci_header(0, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    WCHAR buf[128];
    wsprintf(buf, L"Proxy: Filter Type:%x", type);
    m_trace->SetCurSel(m_trace->AddString(buf));
    *p++ = (BYTE)type;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_TYPE_SET, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedProxyFilterAddrAdd()
{
    USHORT  addr = (USHORT)GetHexValueInt(IDC_PROXY_FILTER_ADDR);
    BYTE    buffer[128];
    // special case for filter dst = 0
    LPBYTE p =wiced_bt_mesh_format_hci_header(0, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    WCHAR buf[128];
    wsprintf(buf, L"Proxy: Filter Add:%x", addr);
    m_trace->SetCurSel(m_trace->AddString(buf));
    *p++ = (BYTE)addr;
    *p++ = (BYTE)(addr >> 8);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD, buffer, (DWORD)(p - buffer));
}

void CConfig::OnBnClickedProxyFilterAddrDelete()
{
    USHORT  addr = (USHORT)GetHexValueInt(IDC_PROXY_FILTER_ADDR);
    BYTE    buffer[128];
    // special case for filter dst = 0
    LPBYTE p =wiced_bt_mesh_format_hci_header(0, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    WCHAR buf[128];
    wsprintf(buf, L"Proxy: Filter Add:%x", addr);
    m_trace->SetCurSel(m_trace->AddString(buf));
    *p++ = (BYTE)addr;
    *p++ = (BYTE)(addr >> 8);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_DELETE, buffer, (DWORD)(p - buffer));
}

void SendGetCompositionData(USHORT dst, BYTE page)
{
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = page;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_COMPOSITION_DATA_GET, buffer, (DWORD)(p - buffer));
}

void SendAddAppKey(USHORT dst, USHORT net_key_idx, USHORT app_key_idx, BYTE *p_key)
{
    BYTE    buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = net_key_idx & 0xff;
    *p++ = (net_key_idx >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    memcpy(p, p_key, 16);
    p += 16;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_ADD, buffer, (DWORD)(p - buffer));
}

void SendBind(USHORT dst, USHORT element_addr, USHORT company_id, USHORT model_id, USHORT app_key_idx)
{
    BYTE   buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, 0xFFFF, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = element_addr & 0xff;
    *p++ = (element_addr >> 8) & 0xff;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    *p++ = model_id & 0xff;
    *p++ = (model_id >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;

    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND, buffer, (DWORD)(p - buffer));
}

BOOL CheckCompositionData(BYTE type, BYTE *p_composition_data, USHORT len)
{
    return TRUE;
}
