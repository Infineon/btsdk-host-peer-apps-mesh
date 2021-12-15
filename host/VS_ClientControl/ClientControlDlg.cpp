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

// ClientControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "atlbase.h"
#include "atlstr.h"
#include "comutil.h"
#include "ClientControl.h"
#include "ClientControlDlg.h"
#include "afxdialogex.h"
#include "ClientControlDlg.h"
#include "SchedulerAdvanced.h"
#include "MeshConfig.h"
#include "LightControl.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_mesh_client.h"
#include "hci_control_api.h"
#include "MeshPerf.h"
#ifdef DIRECTED_FORWARDING_SUPPORTED
#include "DirectedForwarding.h"
#endif

ComHelper *m_ComHelper;
ComHelper* m_ComHelper2 = NULL;

BYTE world_day_of_week[7] = { 1 << 6, 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };

#ifndef assert
#define assert
#endif

char log_filename[MAX_PATH] = { 0 };

extern BOOL SendMessageToUDPServer(char* p_msg, UINT len);

void tai_to_utc_local_time(ULONGLONG tai_seconds, int* year, int* month, int* day, int* hour, int* minute, int* second);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WCHAR *TestSelect[] =
{
    L"OnOff",
    L"Level",
    L"Default Transition Time",
    L"Power OnOff",
    L"Power Level",
    L"Battery",
    L"Location",
    L"Property",
    L"Sensor",
    L"Time",
    L"Scene",
    L"Scheduler",
    L"Light Lightness",
    L"Light CTL",
    L"Light HSL",
    L"Light xyL",
    L"Light LC",
    L"Other",
};

#define DESCRIPTOR_GET 0
#define SENSOR_GET     1
#define SERIES_GET     2
#define COLUMN_GET     3
#define CADENCE_GET    4
#define CADENCE_SET    5
#define SETTINGS_GET   6
#define SETTING_GET    7
#define SETTING_SET    8
#define SENSOR_STATUS  9

#define TEMPERATURE_SETTING_PROP_ID 0x2AC9
#define UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT 0x2A1C
#define UUID_CHARACTERISTIC_TEMPERATURE_TYPE  0x2A1D

LightLcProp lightLcProp[] =
{
    { L"Time Occupancy Delay (0x3a)", 0x3a, 3 },
    { L"Time Fade On (0x37)", 0x37, 3 },
    { L"Time Run On (0x3c)", 0x3c, 3 },
    { L"Time Fade (0x36)", 0x36, 3 },
    { L"Time Prolong (0x3b)", 0x3b, 3 },
    { L"Time Fade Standby Auto (0x38)", 0x38, 3 },
    { L"Time Fade Standby Manual (0x39)", 0x39, 3 },
    { L"Lightness On (0x2e)", 0x2e, 2 },
    { L"Lightness Prolong (0x2f)", 0x2f, 2 },
    { L"Lightness Standby (0x30)", 0x39, 2 },
    { L"Ambient LuxLevel On (0x2b)", 0x2b, 2 },
    { L"Ambient LuxLevel Prolong (0x2c)", 0x2c, 2 },
    { L"Ambient LuxLevel Standby (0x2d)", 0x2d, 2 },
    { L"Regulator Kiu (0x33)", 0x33, 4 },
    { L"Regulator Kid (0x32)", 0x32, 4 },
    { L"Regulator Kpu (0x35)", 0x35, 4 },
    { L"Regulator Kpd (0x34)", 0x34, 4 },
    { L"Regulator Accuracy (0x31)", 0x31, 1 },
};
int numLightLcProps = sizeof(lightLcProp) / sizeof(lightLcProp[0]);

extern "C" uint8_t *wiced_bt_mesh_format_hci_header(uint16_t dst, uint16_t app_key_idx, uint8_t element_idx, uint8_t reliable, uint8_t send_segmented, uint8_t ttl, uint8_t retransmit_count, uint8_t retransmit_interval, uint8_t reply_timeout, uint8_t *p_buffer, uint16_t len);

char* szRegKeyPrefix = "Software\\Cypress\\";
char *szAppName = "HciControlMesh";
CComboBox *sensorComMsg;
CComboBox *propIdComMsg;
CComboBox *settingPropIdComMsg;
UINT16 property_id = WICED_BT_MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE;
UINT16 setting_property_id = TEMPERATURE_SETTING_PROP_ID;
UINT8 prop_value_len = 2;
int ComPort = 0;
int ComPort2 = 0;
int BaudRate = 0;
int SendDataTime = 10;      // by default in auto mode send data for 10 seconds
int AutoNodes = 0;
IMPLEMENT_DYNCREATE(CClientControlDlg, CPropertyPage)

// CClientControlDlg dialog
CClientControlDlg::CClientControlDlg(CWnd* pParent /*=NULL*/)
    : CPropertyPage(CClientControlDlg::IDD, 102) // CDialogEx(CClientControlDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_month_selection = 0;
    m_day_of_week_selection = 0;
    m_year = 0;
    m_day = 0;
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_trace = NULL;
}

CClientControlDlg::~CClientControlDlg()
{
    delete m_ComHelper;
}

void CClientControlDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX); //CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClientControlDlg, CPropertyPage) // CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_CBN_SELCHANGE(IDC_COM_PORT, &CClientControlDlg::OnCbnSelchangeComPort)
    ON_CBN_SELCHANGE(IDC_COM_BAUD, &CClientControlDlg::OnCbnSelchangeComPort)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BATTERY_LEVEL_GET, &CClientControlDlg::OnBnClickedBatteryLevelGet)
    ON_BN_CLICKED(IDC_BATTERY_LEVEL_SET, &CClientControlDlg::OnBnClickedBatteryLevelSet)
    ON_BN_CLICKED(IDC_LOCATION_GET, &CClientControlDlg::OnBnClickedLocationGet)
    ON_BN_CLICKED(IDC_LOCATION_SET, &CClientControlDlg::OnBnClickedLocationSet)
    ON_BN_CLICKED(IDC_ON_OFF_GET, &CClientControlDlg::OnBnClickedOnOffGet)
    ON_BN_CLICKED(IDC_ON_OFF_SET, &CClientControlDlg::OnBnClickedOnOffSet)
    ON_BN_CLICKED(IDC_USE_PUBLICATION_INFO, &CClientControlDlg::OnBnClickedUsePublicationInfo)
    ON_BN_CLICKED(IDC_LEVEL_GET, &CClientControlDlg::OnBnClickedLevelGet)
    ON_BN_CLICKED(IDC_LEVEL_SET, &CClientControlDlg::OnBnClickedLevelSet)
    ON_BN_CLICKED(IDC_DELTA_SET, &CClientControlDlg::OnBnClickedDeltaSet)
    ON_BN_CLICKED(IDC_MOVE_SET, &CClientControlDlg::OnBnClickedMoveSet)
    ON_BN_CLICKED(IDC_DEFAULT_TRANSITION_TIME_GET, &CClientControlDlg::OnBnClickedDefaultTransitionTimeGet)
    ON_BN_CLICKED(IDC_DEFAULT_TRANSITION_TIME_SET, &CClientControlDlg::OnBnClickedDefaultTransitionTimeSet)
    ON_BN_CLICKED(IDC_POWER_ON_OFF_GET, &CClientControlDlg::OnBnClickedPowerOnOffGet)
    ON_BN_CLICKED(IDC_POWER_ON_OFF_SET, &CClientControlDlg::OnBnClickedPowerOnOffSet)
    ON_BN_CLICKED(IDC_TC_NET_LEVEL_TRX_SEND, &CClientControlDlg::OnBnClickedTcNetLevelTrxSend)
    ON_BN_CLICKED(IDC_TC_TRANSP_LEVEL_SEND, &CClientControlDlg::OnBnClickedTcTranspLevelSend)
    ON_BN_CLICKED(IDC_TC_IV_UPDATE_TRANSIT, &CClientControlDlg::OnBnClickedTcIvUpdateTransit)
    ON_BN_CLICKED(IDC_PROPERTIES_GET, &CClientControlDlg::OnBnClickedPropertiesGet)
    ON_BN_CLICKED(IDC_PROPERTY_GET, &CClientControlDlg::OnBnClickedPropertyGet)
    ON_BN_CLICKED(IDC_PROPERTY_SET, &CClientControlDlg::OnBnClickedPropertySet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_GET, &CClientControlDlg::OnBnClickedLightLightnessGet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_SET, &CClientControlDlg::OnBnClickedLightLightnessSet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_LAST_GET, &CClientControlDlg::OnBnClickedLightLightnessLastGet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_DEFAULT_GET, &CClientControlDlg::OnBnClickedLightLightnessDefaultGet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_DEFAULT_SET, &CClientControlDlg::OnBnClickedLightLightnessDefaultSet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_RANGE_GET, &CClientControlDlg::OnBnClickedLightLightnessRangeGet)
    ON_BN_CLICKED(IDC_LIGHT_LIGHTNESS_RANGE_SET, &CClientControlDlg::OnBnClickedLightLightnessRangeSet)
    ON_BN_CLICKED(IDC_TC_LPN_FRND_CLEAR, &CClientControlDlg::OnBnClickedTcLpnFrndClear)
    ON_CBN_SELCHANGE(IDC_SENSOR_MSG, &CClientControlDlg::OnCbnSelchangeSensorMsg)
    ON_BN_CLICKED(IDC_SENSOR_MSG_SEND, &CClientControlDlg::OnBnClickedSensorMsgSend)
    ON_BN_CLICKED(IDC_LIGHT_HSL_GET, &CClientControlDlg::OnBnClickedLightHslGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_SET, &CClientControlDlg::OnBnClickedLightHslSet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_HUE_GET, &CClientControlDlg::OnBnClickedLightHslHueGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_HUE_SET, &CClientControlDlg::OnBnClickedLightHslHueSet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_SATURATION_GET, &CClientControlDlg::OnBnClickedLightHslSaturationGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_SATURATION_SET, &CClientControlDlg::OnBnClickedLightHslSaturationSet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_DEFAULT_GET, &CClientControlDlg::OnBnClickedLightHslDefaultGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_DEFAULT_SET, &CClientControlDlg::OnBnClickedLightHslDefaultSet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_TARGET_GET, &CClientControlDlg::OnBnClickedLightHslTargetGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_RANGE_GET, &CClientControlDlg::OnBnClickedLightHslRangeGet)
    ON_BN_CLICKED(IDC_LIGHT_HSL_RANGE_SET, &CClientControlDlg::OnBnClickedLightHslRangeSet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_GET, &CClientControlDlg::OnBnClickedLightCtlGet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_SET, &CClientControlDlg::OnBnClickedLightCtlSet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_TEMPERATURE_GET, &CClientControlDlg::OnBnClickedLightCtlTemperatureGet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_TEMPERATURE_SET, &CClientControlDlg::OnBnClickedLightCtlTemperatureSet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_DEFAULT_GET, &CClientControlDlg::OnBnClickedLightCtlDefaultGet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_DEFAULT_SET, &CClientControlDlg::OnBnClickedLightCtlDefaultSet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_TEMPERATURE_RANGE_GET, &CClientControlDlg::OnBnClickedLightCtlTemperatureRangeGet)
    ON_BN_CLICKED(IDC_LIGHT_CTL_TEMPERATURE_RANGE_SET, &CClientControlDlg::OnBnClickedLightCtlTemperatureRangeSet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_GET, &CClientControlDlg::OnBnClickedLightXylGet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_SET, &CClientControlDlg::OnBnClickedLightXylSet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_DEFAULT_GET, &CClientControlDlg::OnBnClickedLightXylDefaultGet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_DEFAULT_SET, &CClientControlDlg::OnBnClickedLightXylDefaultSet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_RANGE_GET, &CClientControlDlg::OnBnClickedLightXylRangeGet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_RANGE_SET, &CClientControlDlg::OnBnClickedLightXylRangeSet)
    ON_BN_CLICKED(IDC_LIGHT_XYL_TARGET_GET, &CClientControlDlg::OnBnClickedLightXylTargetGet)
    ON_BN_CLICKED(IDC_VS_DATA, &CClientControlDlg::OnBnClickedVsData)
    ON_BN_CLICKED(IDC_TC_CLEAR_RPL, &CClientControlDlg::OnBnClickedTcClearRpl)
    ON_CBN_SELCHANGE(IDC_TEST_SELECTION, &CClientControlDlg::OnCbnSelchangeTestSelection)
    ON_BN_CLICKED(IDC_USE_DEFAULT_TRANS_TIME_SEND, &CClientControlDlg::OnBnClickedUseDefaultTransTimeSend)
    ON_BN_CLICKED(IDC_TC_IV_UPDATE_SET_TEST_MODE, &CClientControlDlg::OnBnClickedTcIvUpdateSetTestMode)
    ON_BN_CLICKED(IDC_TC_IV_UPDATE_SET_RECOVERY_MODE, &CClientControlDlg::OnBnClickedTcIvUpdateSetRecoveryMode)
    ON_BN_CLICKED(IDC_LOCATION_LOCAL_GET, &CClientControlDlg::OnBnClickedLocationLocalGet)
    ON_BN_CLICKED(IDC_LOCATION_LOCAL_SET, &CClientControlDlg::OnBnClickedLocationLocalSet)
    ON_BN_CLICKED(IDC_LIGHT_LC_MODE_GET, &CClientControlDlg::OnBnClickedLightLcModeGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_MODE_SET, &CClientControlDlg::OnBnClickedLightLcModeSet)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_MODE_GET, &CClientControlDlg::OnBnClickedLightLcOccupancyModeGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_MODE_SET, &CClientControlDlg::OnBnClickedLightLcOccupancyModeSet)
    ON_BN_CLICKED(IDC_LIGHT_LC_ON_OFF_GET, &CClientControlDlg::OnBnClickedLightLcOnOffGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_ON_OFF_SET, &CClientControlDlg::OnBnClickedLightLcOnOffSet)
    ON_BN_CLICKED(IDC_LIGHT_LC_PROPERTY_GET, &CClientControlDlg::OnBnClickedLightLcPropertyGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_PROPERTY_SET, &CClientControlDlg::OnBnClickedLightLcPropertySet)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_SET, &CClientControlDlg::OnBnClickedLightLcOccupancySet)
    ON_BN_CLICKED(IDC_SCENE_REGISTER_GET, &CClientControlDlg::OnBnClickedSceneRegisterGet)
    ON_BN_CLICKED(IDC_SCENE_RECALL, &CClientControlDlg::OnBnClickedSceneRecall)
    ON_BN_CLICKED(IDC_SCENE_GET, &CClientControlDlg::OnBnClickedSceneGet)
    ON_BN_CLICKED(IDC_SCENE_STORE, &CClientControlDlg::OnBnClickedSceneStore)
    ON_BN_CLICKED(IDC_SCENE_DELETE, &CClientControlDlg::OnBnClickedSceneDelete)
    ON_BN_CLICKED(IDC_SCHEDULER_REGISTER_GET, &CClientControlDlg::OnBnClickedSchedulerRegisterGet)
    ON_BN_CLICKED(IDC_SCHEDULER_ACTION_GET, &CClientControlDlg::OnBnClickedSchedulerActionGet)
    ON_BN_CLICKED(IDC_SCHEDULER_ACTION_SET, &CClientControlDlg::OnBnClickedSchedulerActionSet)
    ON_BN_CLICKED(IDC_SCHEDULER_ADVANCED, &CClientControlDlg::OnBnClickedSchedulerAdvanced)
    ON_BN_CLICKED(IDC_TIME_GET, &CClientControlDlg::OnBnClickedTimeGet)
    ON_BN_CLICKED(IDC_TC_HEALTH_FAULTS_SET, &CClientControlDlg::OnBnClickedTcHealthFaultsSet)
    ON_BN_CLICKED(IDC_TIME_SET, &CClientControlDlg::OnBnClickedTimeSet)
    ON_BN_CLICKED(IDC_TIMEZONE_GET, &CClientControlDlg::OnBnClickedTimezoneGet)
    ON_BN_CLICKED(IDC_TIMEZONE_SET, &CClientControlDlg::OnBnClickedTimezoneSet)
    ON_BN_CLICKED(IDC_TAI_UTC_DELTA_GET, &CClientControlDlg::OnBnClickedTaiUtcDeltaGet)
    ON_BN_CLICKED(IDC_TAI_UTC_DELTA_SET, &CClientControlDlg::OnBnClickedTaiUtcDeltaSet)
    ON_BN_CLICKED(IDC_TIME_AUTHORITY_GET, &CClientControlDlg::OnBnClickedTimeAuthorityGet)
    ON_BN_CLICKED(IDC_TIME_AUTHORITY_SET, &CClientControlDlg::OnBnClickedTimeAuthoritySet)
    ON_BN_CLICKED(IDC_TC_CFG_IDENTITY, &CClientControlDlg::OnBnClickedTcCfgIdentity)
    ON_BN_CLICKED(IDC_TC_ACCESS_PDU, &CClientControlDlg::OnBnClickedTcAccessPdu)
    ON_BN_CLICKED(IDC_TC_LPN_SEND_SUBS_ADD, &CClientControlDlg::OnBnClickedTcLpnSendSubsAdd)
    ON_BN_CLICKED(IDC_TC_LPN_SEND_SUBS_DEL, &CClientControlDlg::OnBnClickedTcLpnSendSubsDel)
END_MESSAGE_MAP()

BYTE ProcNibble(char n)
{
    if ((n >= '0') && (n <= '9'))
    {
        n -= '0';
    }
    else if ((n >= 'A') && (n <= 'F'))
    {
        n = ((n - 'A') + 10);
    }
    else if ((n >= 'a') && (n <= 'f'))
    {
        n = ((n - 'a') + 10);
    }
    else
    {
        n = (char)0xff;
    }
    return (n);
}

// CClientControlDlg message handlers
DWORD GetHexValue(char *szbuf, LPBYTE buf, DWORD buf_size)
{
    BYTE *pbuf = buf;
    DWORD res = 0;

    // remove leading white space
    while (*szbuf != 0 && (BYTE)*szbuf <= 0x20) szbuf++;
    DWORD len = (DWORD)strlen(szbuf);
    // remove terminating white space
    while (len > 0 && (BYTE)szbuf[len - 1] <= 0x20) len--;

    memset(buf, 0, buf_size);

    if (len == 1)
    {
        szbuf[2] = 0;
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (len == 3)
    {
        szbuf[4] = 0;
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (len == 5)
    {
        szbuf[6] = 0;
        szbuf[5] = szbuf[4];
        szbuf[4] = szbuf[3];
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    for (DWORD i = 0; (i < len) && (res < buf_size); i++)
    {
        if (isxdigit(szbuf[i]) && isxdigit(szbuf[i + 1]))
        {
            *pbuf++ = (ProcNibble((char)szbuf[i]) << 4) + ProcNibble((char)szbuf[i + 1]);
            res++;
            i++;
        }
    }
    return res;
}

DWORD CClientControlDlg::GetHexValueInt(DWORD id)
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


BOOL CClientControlDlg::OnSetActive()
{
    static BOOL initialized = FALSE;
    CPropertyPage::OnSetActive();

    ((CClientDialog *)theApp.m_pMainWnd)->m_active_page = idxPageMain;

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);

    CComboBox *m_cbCom = (CComboBox *)GetDlgItem(IDC_COM_PORT);
    if (m_cbCom->GetCount() == 0)
    {
        int com_port, baud_rate;
        for (com_port = 0; com_port < 128 && aComPorts[com_port] != 0; com_port++)
        {
            WCHAR buf[20];
            wsprintf(buf, L"COM%d", aComPorts[com_port]);
            m_cbCom->SetItemData(m_cbCom->AddString(buf), aComPorts[com_port]);
        }

        CComboBox *m_cbBaud = (CComboBox *)GetDlgItem(IDC_COM_BAUD);
        for (baud_rate = 0; baud_rate < sizeof(as32BaudRate) / sizeof(as32BaudRate[0]); baud_rate++)
        {
            WCHAR acBaud[10];
            wsprintf(acBaud, L"%d", as32BaudRate[baud_rate]);

            m_cbBaud->SetItemData(m_cbBaud->AddString(acBaud), baud_rate);
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

    if (!initialized)
    {
        initialized = TRUE;

        CComboBox *pFloor = (CComboBox *)GetDlgItem(IDC_FLOOR);
        pFloor->ResetContent();
        pFloor->AddString(L"-20 or lower");
        WCHAR buf[10];
        for (int i = -19; i < 232; i++)
        {
            wsprintf(buf, L"%d", i);
            pFloor->AddString(buf);
        }
        pFloor->AddString(L"232 or higher");
        pFloor->AddString(L"Ground Floor 0");
        pFloor->AddString(L"Ground Floor 1");
        pFloor->AddString(L"Not configured");

        CComboBox *pLightLcProp = (CComboBox *)GetDlgItem(IDC_LIGHT_LC_PROPERTY);
        pLightLcProp->ResetContent();
        for (int i = 0; i < sizeof(lightLcProp) / sizeof(lightLcProp[0]); i++)
            pLightLcProp->AddString(lightLcProp[i].PropName);

        ((CComboBox *)GetDlgItem(IDC_ON_OFF_TARGET))->SetCurSel(0);
        ((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->SetCheck(0);
        OnBnClickedUsePublicationInfo();

        ((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->SetCheck(1);

        SetDlgItemInt(IDC_DST, 1);
        SetDlgItemInt(IDC_APP_KEY_IDX, 0);
        SetDlgItemInt(IDC_TRANSITION_TIME, 20000, 0);
        SetDlgItemInt(IDC_DELAY, 25, 0);
        ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->SetCheck(1);

        GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(0);
        GetDlgItem(IDC_DELAY)->EnableWindow(0);

        ((CComboBox *)GetDlgItem(IDC_PROPERTY_TYPE))->SetCurSel(3);
        SetDlgItemText(IDC_TC_NET_LEVEL_TRX_DST, L"0001");
        SetDlgItemText(IDC_TC_NET_LEVEL_TRX_TTL, L"00");
        SetDlgItemText(IDC_TC_NET_LEVEL_TRX_PDU, L"05 00 00 50 73 20 0F 0F 0F 0F 0F 0F 0F 0F 0F 0F");
        SetDlgItemText(IDC_TC_PVNR_ADDR, L"1202");
        SetDlgItemText(IDC_TC_HEALTH_FAULTS, L"01 02 03");

        sensorComMsg = (CComboBox *)GetDlgItem(IDC_SENSOR_MSG);
        sensorComMsg->AddString(L"DESCRIPTOR GET");
        sensorComMsg->AddString(L"SENSOR GET");
        sensorComMsg->AddString(L"SERIES GET");
        sensorComMsg->AddString(L"COLUMN GET");
        sensorComMsg->AddString(L"CADENCE GET");
        sensorComMsg->AddString(L"CADENCE SET");
        sensorComMsg->AddString(L"SETTINGS GET");
        sensorComMsg->AddString(L"SETTING GET");
        sensorComMsg->AddString(L"SETTING SET");
        sensorComMsg->AddString(L"SENSOR STATUS");
        ((CComboBox *)GetDlgItem(IDC_SENSOR_MSG))->SetCurSel(0);

        propIdComMsg = (CComboBox *)GetDlgItem(IDC_SENSOR_PROP_ID);
        propIdComMsg->AddString(L"TEMP_MEASUREMENT");
        propIdComMsg->AddString(L"NO_PROPERTY_ID");
        propIdComMsg->AddString(L"TEMP_TYPE");
        ((CComboBox *)GetDlgItem(IDC_SENSOR_PROP_ID))->SetCurSel(0);

        settingPropIdComMsg = (CComboBox *)GetDlgItem(IDC_SENSOR_SETTING_PROP_ID);
        settingPropIdComMsg->AddString(L"TEMP_SETTING_PROP_ID");
        settingPropIdComMsg->AddString(L"NO_SETTING_PROPERTY_ID");

        ((CComboBox *)GetDlgItem(IDC_SENSOR_SETTING_PROP_ID))->SetCurSel(0);
        SetDlgItemInt(IDC_SCHEDULAR_ACTION_NUMBER, 1);

        CComboBox *pTestSelect = (CComboBox *)GetDlgItem(IDC_TEST_SELECTION);
        pTestSelect->ResetContent();
        for (int i = 0; i < sizeof(TestSelect) / sizeof(TestSelect[0]); i++)
            pTestSelect->SetItemData(pTestSelect->AddString(TestSelect[i]), (DWORD_PTR)i);

        pTestSelect->SetCurSel(0);
        OnCbnSelchangeTestSelection();

        ((CComboBox *)GetDlgItem(IDC_TIME_AUTHORITY))->SetCurSel(0);
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    CPropertyPage::OnSysCommand(nID, lParam);
    // CDialogEx::OnSysCommand(nID, lParam);
}

void CClientControlDlg::DisableAll()
{
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_CURRENT)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_LEVEL)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_TO_DISCHARGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_TO_CHARGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HUE_RANGE_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HUE_RANGE_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCAL_NORTH)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCAL_EAST)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCAL_ALTITUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_UPDATE_TIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_PRECISION)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_ID)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_VALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_ACCESS)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_SATURATION_RANGE_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_SATURATION_RANGE_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_PRESENSE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_INDICATOR)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_CHARGING)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_SERVICABILITY)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLOOR)->EnableWindow(FALSE);
    GetDlgItem(IDC_ON_OFF_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_POWER_ON_OFF_STATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_TYPE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_LEVEL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCATION_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_ON_OFF_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_MOBILE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BATTERY_LEVEL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCATION_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCATION_LOCAL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LOCATION_LOCAL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_GLOBAL_ALTITUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_GLOBAL_LONGITUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_GLOBAL_LATITUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LEVEL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_ON_OFF_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELAY)->EnableWindow(FALSE);
    GetDlgItem(IDC_LEVEL_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LEVEL_CURRENT)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_LEVEL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LEVEL_DELTA)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELTA_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELTA_CONTINUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_MOVE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_POWER_ON_OFF_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_POWER_ON_OFF_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTIES_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROPERTY_STATUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_REGISTER_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_RECALL)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_STORE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_DELETE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_NUMBER)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHEDULER_DAY)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHEDULER_TIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_SUBSECONDS)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_UNCERTAINTY)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIMEZONE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIMEZONE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_ZONE_OFFSET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TAI_UTC_DELTA_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TAI_UTC_DELTA_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_TAI_UTC_DELTA)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_AUTHORITY)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_AUTHORITY_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_AUTHORITY_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCENE_NUMBER)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCHEDULER_REGISTER_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCHEDULER_ACTION_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCHEDULER_ACTION_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCHEDULAR_ACTION_NUMBER)->EnableWindow(FALSE);
    GetDlgItem(IDC_ACTION)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LAST_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_VALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_VALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_TARGET_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_DEFAULT_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_DEFAULT_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_RANGE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_HSL_RANGE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_DEFAULT_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_DEFAULT_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_CTL_DELTA_UV_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_X_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_X_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_Y_MAX)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_Y_MIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_Y_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_X_TARGET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_DEFAULT_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_DEFAULT_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_RANGE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_RANGE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_TARGET_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_XYL_TARGET_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_MODE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_MODE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_MODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_VALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_GET)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_TRIG_TYPE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_RAW_VAL_X)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_RAW_VAL_X1)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_RAW_VAL_X2)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_SETTING_VAL)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_FS_CAD_DIV)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_TRIG_DWN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_TRIG_DEL_UP)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_MIN_INT)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_FST_CAD_HIGH)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_CL_FST_CAD_LOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_PROP_ID)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_SETTING_PROP_ID)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_PROP_VAL_LEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_MSG)->EnableWindow(FALSE);
    GetDlgItem(IDC_SENSOR_MSG_SEND)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_CTL)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_SEND)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_TTL)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_PDU)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_DST)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_TRANSP_LEVEL_SZMIC)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_TRANSP_LEVEL_SEND)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_IV_UPDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_IV_UPDATE_TRANSIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_IV_UPDATE_SET_TEST_MODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_IV_UPDATE_SET_RECOVERY_MODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_LPN_FRND_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_PVNR_ADDR)->EnableWindow(FALSE);
    GetDlgItem(IDC_VS_DATA)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_CLEAR_RPL)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_HEALTH_FAULTS)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_HEALTH_FAULTS_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_TC_CFG_IDENTITY)->EnableWindow(FALSE);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientControlDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CPropertyPage::OnPaint();
        // CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClientControlDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void HandleWicedEvent(int com_port, DWORD opcode, DWORD len, BYTE *p_data)
{
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
    if (pSheet == NULL)
        return;

    if (pSheet->m_active_page == idxPageConnectedMesh)
    {
        ConnectedMesh  *pDlg = &pSheet->pageConnectedMesh;
        pDlg->ProcessData(com_port, opcode, p_data, len);
        return;
    }
#ifdef DIRECTED_FORWARDING_SUPPORTED
    else if (pSheet->m_active_page == idxPageDirectedForwarding)
    {
        CDirectedForwarding*pDlg = &pSheet->pageDirectedForwarding;
        pDlg->ProcessData(com_port, opcode, p_data, len);
        return;
    }
#endif
    else if (pSheet->m_active_page == idxPageConfig)
    {
        CConfig *pDlg = &pSheet->pageConfig;
        pDlg->ProcessData(opcode, p_data, len);
        return;
    }
    else if (pSheet->m_active_page == idxPageMain)
    {
        CClientControlDlg *pDlg = &pSheet->pageMain;
        pDlg->ProcessData(com_port, opcode, p_data, len);
        return;
    }
    else if (pSheet->m_active_page == idxPageLight)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        pDlg->ProcessData(com_port, opcode, p_data, len);
        return;
    }
}

void HandleHciEvent(BYTE *p_data, DWORD len)
{
    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
    if (pSheet == NULL)
        return;

    if (pSheet->m_active_page == idxPageConfig)
    {
        CConfig *pDlg = &pSheet->pageConfig;
        pDlg->ProcessEvent(p_data, len);
        return;
    }
    else if (pSheet->m_active_page == idxPageMain)
    {
        CClientControlDlg *pDlg = &pSheet->pageMain;
        pDlg->ProcessEvent(p_data, len);
        return;
    }
    else if (pSheet->m_active_page == idxPageLight)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        pDlg->ProcessEvent(p_data, len);
        return;
    }
}

#include <WinSock2.h>
SOCKET log_sock = INVALID_SOCKET;
extern int host_mode_instance;

// mapping between wiced trace types and spy trace types (evt, cmd, rx data, tx data)
static int wiced_trace_to_spy_trace[] = { 0, 4, 3, 6, 7 };

void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length)
{
    SOCKADDR_IN socket_addr;
    static int initialized = FALSE;
    BYTE buf[1100];
    USHORT offset = 0;
    USHORT *p = (USHORT*)buf;

    if (!initialized)
    {
        initialized = TRUE;

        WSADATA wsaData;
        int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
        if (err != 0)
            return;
        log_sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (log_sock == INVALID_SOCKET)
            return;

        memset(&socket_addr, 0, sizeof(socket_addr));
        socket_addr.sin_family = AF_INET;
        socket_addr.sin_addr.s_addr = ADDR_ANY;
        socket_addr.sin_port = 0;

        err = bind(log_sock, (SOCKADDR *)&socket_addr, sizeof(socket_addr));
        if (err != 0)
        {
            closesocket(log_sock);
            log_sock = INVALID_SOCKET;
            return;
        }
    }
    if (log_sock == INVALID_SOCKET)
        return;

    if (length > 1024)
        length = 1024;

    *p++ = wiced_trace_to_spy_trace[type];
    *p++ = length;
    *p++ = 0;
    *p++ = 1;
    memcpy(p, buffer, length);

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = ntohl(0x7f000001);
    socket_addr.sin_port = 9876 + host_mode_instance;

    length = sendto(log_sock, (const char *)buf, length + 8, 0, (SOCKADDR *)&socket_addr, sizeof(SOCKADDR_IN));
}

void CClientControlDlg::ProcessEvent(LPBYTE p_data, DWORD len)
{
}

void CClientControlDlg::ProcessData(INT port_num, DWORD opcode, LPBYTE p_data, DWORD len)
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
    case HCI_CONTROL_EVENT_DEVICE_STARTED:
        m_trace->SetCurSel(m_trace->AddString(L"Device Started"));
        break;
    case HCI_CONTROL_EVENT_PAIRING_COMPLETE:
        wsprintf(trace, L"Pairing complete:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_EVENT_ENCRYPTION_CHANGED:
        wsprintf(trace, L"Encryption changed:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_COMMAND_STATUS:
        wsprintf(trace, L"Mesh Command Status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_CORE_SEQ_CHANGED:
    case HCI_CONTROL_MESH_EVENT_TX_COMPLETE:
        break;
    case HCI_CONTROL_MESH_EVENT_ONOFF_SET:
        ProcessOnOffSet(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_ONOFF_STATUS:
        ProcessOnOffStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LEVEL_STATUS:
        ProcessLevelStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_DEF_TRANS_TIME_STATUS:
        ProcessDefaultTransitionTimeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_POWER_ONOFF_STATUS:
        ProcessPowerOnOffStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_POWER_LEVEL_STATUS:
        ProcessPowerLevelStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_POWER_LEVEL_LAST_STATUS:
        ProcessPowerLevelLastStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_POWER_LEVEL_DEFAULT_STATUS:
        ProcessPowerLevelDefaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_POWER_LEVEL_RANGE_STATUS:
        ProcessPowerLevelRangeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_BATTERY_STATUS:
        m_trace->SetCurSel(m_trace->AddString(L"Battery client status"));
        ProcessBatteryStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LOCATION_LOCAL_SET:
        m_trace->SetCurSel(m_trace->AddString(L"Location server local set"));
        ProcessLocationLocalSet(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LOCATION_GLOBAL_SET:
        m_trace->SetCurSel(m_trace->AddString(L"Location server global set"));
        ProcessLocationGlobalSet(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LOCATION_GLOBAL_STATUS:
        m_trace->SetCurSel(m_trace->AddString(L"Global Location changed"));
        ProcessLocationGlobalSet(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LOCATION_LOCAL_STATUS:
        m_trace->SetCurSel(m_trace->AddString(L"Local Location changed"));
        ProcessLocationLocalSet(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROPERTIES_STATUS:
        ProcessPropertiesStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROPERTY_STATUS:
        ProcessPropertyStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SCENE_REGISTER_STATUS:
        ProcessSceneRegisterStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SCENE_STATUS:
        ProcessSceneStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SCHEDULER_STATUS:
        ProcessSchedulerStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SCHEDULER_ACTION_STATUS:
        ProcessSchedulerActionStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_TIME_STATUS:
        ProcessTimeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_TIME_ZONE_STATUS:
        ProcessTimeZoneStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_TIME_TAI_UTC_DELTA_STATUS:
        ProcessTimeTaiDeltaStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_TIME_ROLE_STATUS:
        ProcessTimeRoleStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_STATUS:
        ProcessLightnessStatus(FALSE, p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_LINEAR_STATUS:
        ProcessLightnessStatus(TRUE, p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_LAST_STATUS:
        ProcessLightnessLastStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_DEFAULT_STATUS:
        ProcessLightnessDefaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_RANGE_STATUS:
        ProcessLightnessRangeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_DESCRIPTOR_STATUS:
        ProcessSensorDescriptorStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_STATUS:
        ProcessSensorStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_COLUMN_STATUS:
        wsprintf(trace, L"column status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_CADENCE_STATUS:
        wsprintf(trace, L"cadence status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_SETTINGS_STATUS:
        wsprintf(trace, L"settings status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_SETTING_STATUS:
        wsprintf(trace, L"setting status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_SERIES_STATUS:
        wsprintf(trace, L"series status:%d", p_data[0]);
        m_trace->SetCurSel(m_trace->AddString(trace));
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_CTL_STATUS:
        ProcessLightCtlStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_STATUS:
        ProcessLightCtlTemperatureStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_RANGE_STATUS:
        ProcessLightCtlTemperatureRangeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_CTL_DEFAULT_STATUS:
        ProcessLightCtlDefaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_STATUS:
        ProcessLightHslStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_TARGET_STATUS:
        ProcessLightHslTargetStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_HUE_STATUS:
        ProcessLightHslHueStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_SATURATION_STATUS:
        ProcessLightHslSaturationStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_RANGE_STATUS:
        ProcessLightHslRangeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_DEFAULT_STATUS:
        ProcessLightHslDefaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_XYL_STATUS:
        ProcessLightXylStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_XYL_TARGET_STATUS:
        ProcessLightXylTargetStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_XYL_RANGE_STATUS:
        ProcessLightXylRangeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_XYL_DEFAULT_STATUS:
        ProcessLightXylDefaultStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LC_MODE_CLIENT_STATUS:
        ProcessLightLcModeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LC_OCCUPANCY_MODE_CLIENT_STATUS:
        ProcessLightLcOccupancyModeStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LC_ONOFF_CLIENT_STATUS:
        ProcessLightLcLightOnOffStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LC_PROPERTY_CLIENT_STATUS:
        ProcessLightLcPropertyStatus(p_data, len);
        break;
    case HCI_CONTROL_MESH_EVENT_VENDOR_DATA:
        ProcessVendorSpecificData(p_data, len);
        break;
    default:
        wsprintf(trace, L"Rcvd Unknown Op Code: 0x%04x", opcode);
        m_trace->SetCurSel(m_trace->AddString(trace));
    }
}

int FindBaudRateIndex(int baud)
{
    int index = 0;

    for (; index < sizeof(as32BaudRate) / sizeof(as32BaudRate[0]); index++)
        if (as32BaudRate[index] == baud)
            return index;

    return -1;
}

int CClientControlDlg::GetBaudRateSelection()
{
    CComboBox *m_cbBaud = (CComboBox *)GetDlgItem(IDC_COM_BAUD);
    int select = m_cbBaud->GetItemData(m_cbBaud->GetCurSel());

    if (select >= 0 && select < sizeof(as32BaudRate))
        return as32BaudRate[select];

    return as32BaudRate[0];
}

void CClientControlDlg::OnCbnSelchangeComPort()
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

DWORD CClientControlDlg::GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size)
{
    char szbuf[1300];
    char *psz = szbuf;
    BYTE *pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, buf_size);

    GetDlgItemTextA(m_hWnd, id, szbuf, sizeof(szbuf));
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
        }
    }
    return res;
}

DWORD CClientControlDlg::GetHandle(DWORD id)
{
    BYTE buf[2];
    int num_digits = GetHexValue(id, buf, 2);
    if (num_digits == 2)
        return (buf[0] << 8) + buf[1];
    else
        return buf[0];
}

void Log(WCHAR *fmt, ...)
{
    WCHAR   msg[1002];

    va_list cur_arg;
    SYSTEMTIME st;
    GetLocalTime(&st);

    memset(msg, 0, sizeof(msg));

    int len = swprintf_s(msg, 1002, L"%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    va_start(cur_arg, fmt);
    vswprintf(&msg[wcslen(msg)], (sizeof(msg) / sizeof(WCHAR)) - wcslen(msg), fmt, cur_arg);
    va_end(cur_arg);

    if (strlen(log_filename))
    {
        FILE* fp;
        fopen_s(&fp, log_filename, "a");
        if (fp)
        {
            fputws(msg, fp);
            fputws(L"\n", fp);
            fclose(fp);
        }
    }

    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;
    if (pSheet == NULL)
        return;

    if (pSheet->m_active_page == idxPageConnectedMesh)
    {
        ConnectedMesh* pDlg = &pSheet->pageConnectedMesh;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
#ifdef DIRECTED_FORWARDING_SUPPORTED
    else if (pSheet->m_active_page == idxPageDirectedForwarding)
    {
        CDirectedForwarding* pDlg = &pSheet->pageDirectedForwarding;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
#endif
    else if (pSheet->m_active_page == idxPageConfig)
    {
        CConfig *pDlg = &pSheet->pageConfig;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
    else if (pSheet->m_active_page == idxPageMain)
    {
        CClientControlDlg *pDlg = &pSheet->pageMain;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
    else if (pSheet->m_active_page == idxPageLight)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
    else if (pSheet->m_active_page == idxPageMeshPerf)
    {
        CMeshPerformance* pDlg = &pSheet->pageMeshPerf;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(msg));
        return;
    }
}

void LogFile(WCHAR* fmt, ...)
{
    WCHAR   msg[1002];

    va_list cur_arg;

    if (strlen(log_filename))
    {
        FILE* fp;
        fopen_s(&fp, log_filename, "a");
        if (fp)
        {
            SYSTEMTIME st;
            GetLocalTime(&st);

            memset(msg, 0, sizeof(msg));

            int len = swprintf_s(msg, 1002, L"%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

            va_start(cur_arg, fmt);
            vswprintf(&msg[wcslen(msg)], (sizeof(msg) / sizeof(WCHAR)) - wcslen(msg), fmt, cur_arg);
            va_end(cur_arg);

            wcscat(msg, L"\n");

            fputws(msg, fp);
            fclose(fp);
        }
    }
}

extern "C" void Log(char *fmt, ...)
{
    char   msg[1002];
    WCHAR  wmsg[2004];

    memset(msg, 0, sizeof(msg));

    va_list marker = NULL;
    va_start(marker, fmt);

    SYSTEMTIME st;
    GetLocalTime(&st);

    int len = sprintf_s(msg, sizeof(msg), "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    vsnprintf_s(&msg[len], sizeof(msg) - len, _TRUNCATE, fmt, marker);
    va_end(marker);

    MultiByteToWideChar(CP_UTF8, 0, msg, -1, wmsg, sizeof(wmsg) / 2);

    if (strlen(log_filename))
    {
        FILE* fp;
        fopen_s(&fp, log_filename, "a");
        wcscat(wmsg, L"\n");
        if (fp)
        {
            fputws(wmsg, fp);
            fclose(fp);
        }
    }

    CClientDialog *pSheet = (CClientDialog *)theApp.m_pMainWnd;

    if (pSheet->m_active_page == idxPageConnectedMesh)
    {
        ConnectedMesh* pDlg = &pSheet->pageConnectedMesh;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
#ifdef DIRECTED_FORWARDING_SUPPORTED
    else if (pSheet->m_active_page == idxPageDirectedForwarding)
    {
        CDirectedForwarding* pDlg = &pSheet->pageDirectedForwarding;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
#endif
    else if (pSheet->m_active_page == idxPageConfig)
    {
        CConfig *pDlg = &pSheet->pageConfig;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
    else if (pSheet->m_active_page == idxPageMain)
    {
        CClientControlDlg *pDlg = &pSheet->pageMain;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
    else if (pSheet->m_active_page == idxPageLight)
    {
        CLightControl *pDlg = &pSheet->pageLight;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
    else if (pSheet->m_active_page == idxPageMeshPerf)
    {
        CMeshPerformance* pDlg = &pSheet->pageMeshPerf;
        if (pDlg && pDlg->m_hWnd && ::IsWindow(pDlg->m_hWnd) && ::IsWindow(pDlg->m_trace->m_hWnd))
            pDlg->m_trace->SetCurSel(pDlg->m_trace->AddString(wmsg));
        return;
    }
}

extern "C" void ods(char* fmt_str, ...)
{
    char buf[1000] = { 0 };
    va_list marker = NULL;

    va_start(marker, fmt_str);

    SYSTEMTIME st;
    GetLocalTime(&st);

    int len = sprintf_s(buf, sizeof(buf), "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    vsnprintf_s(&buf[len], sizeof(buf) - len, _TRUNCATE, fmt_str, marker);
    va_end(marker);

    if (buf[strlen(buf) - 1] != '\n')
        strcat_s(buf, sizeof(buf), "\n");
    OutputDebugStringA(buf);
}

void CClientControlDlg::OnClose()
{
    m_ComHelper->ClosePort();
    Sleep(2000);
    CPropertyPage::OnClose();
    // CDialogEx::OnClose();
}

#if 0
#include "wiced.h"
#include "wiced_bt_event.h"
#include "wiced_bt_mesh_event.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_models.h"
//#include "wiced_bt_mesh_core.h"

wiced_result_t wiced_bt_mesh_core_send(wiced_bt_mesh_event_t *p_event, const uint8_t* params, uint16_t params_len, wiced_bt_mesh_core_send_complete_callback_t complete_callback)
{
    CClientControlDlg *pDlg = (CClientControlDlg *)theApp.GetMainWnd();
    return (wiced_result_t)pDlg->WicedBtMeshCoreSend(p_event, params, params_len);
}

void wiced_bt_mesh_core_cancel_send(wiced_bt_mesh_event_t *p_event)
{
}

extern "C" wiced_result_t wiced_bt_mesh_core_get_publication(wiced_bt_mesh_event_t *p_event)
{
    return WICED_ERROR;
}

extern "C" wiced_bool_t foundation_find_appkey_(uint16_t appkey_global_idx, uint8_t *p_app_idx, uint8_t *p_net_idx, uint16_t *p_netkey_global_idx)
{
    return WICED_FALSE;
}

BOOL CClientControlDlg::WicedBtMeshCoreSend(void *pc, const unsigned char* params, unsigned short params_len)
{
    wiced_bt_mesh_event_t *p_event = (wiced_bt_mesh_event_t *)pc;
    USHORT dst = 0, app_key_idx = 0;

    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = buffer;
    *p++ = dst & 0xff;
    *p++ = (dst >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    *p++ = 0;  // element_idx
    *p++ = p_event->opcode & 0xff;
    *p++ = (p_event->opcode >> 8) & 0xff;
    memcpy(p, params, params_len);

    WCHAR buf[200];
    wsprintf(buf, L"Send opcode:%04x", p_event->opcode);
    m_trace->SetCurSel(m_trace->AddString(buf));
    // m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_MESSAGE, buffer, (DWORD)(p - buffer));
    return TRUE;
}
#endif

void CClientControlDlg::OnBnClickedBatteryLevelGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Battery Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_BATTERY_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSceneRegisterGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Scene Register Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCENE_REGISTER_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSceneRecall()
{
    WCHAR buf[40];
    UINT16 scene = GetDlgItemInt(IDC_SCENE_NUMBER);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);
    wsprintf(buf, L"Scene Recall:%d", scene);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = scene & 0xff;
    *p++ = (scene >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCENE_RECALL, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSceneGet()
{
    WCHAR buf[40];
    UINT16 scene = GetDlgItemInt(IDC_SCENE_NUMBER);
    wsprintf(buf, L"Scene Get:%d", scene);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCENE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSceneStore()
{
    WCHAR buf[40];
    UINT16 scene = GetDlgItemInt(IDC_SCENE_NUMBER);
    wsprintf(buf, L"Scene Store:%d", scene);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = scene & 0xff;
    *p++ = (scene >> 8) & 0xff;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCENE_STORE, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSceneDelete()
{
    WCHAR buf[40];
    UINT16 scene = GetDlgItemInt(IDC_SCENE_NUMBER);
    wsprintf(buf, L"Scene Delete:%d", scene);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = scene & 0xff;
    *p++ = (scene >> 8) & 0xff;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCENE_DELETE, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedBatteryLevelSet()
{
    BYTE battery_level = (BYTE) GetDlgItemInt(IDC_BATTERY_LEVEL);
    DWORD time_to_discharge = (DWORD)GetDlgItemInt(IDC_TIME_TO_DISCHARGE);
    DWORD time_to_charge = (DWORD)GetDlgItemInt(IDC_TIME_TO_CHARGE);
    int presence = ((CComboBox *)GetDlgItem(IDC_BATTERY_PRESENSE))->GetCurSel();
    int indicator = ((CComboBox *)GetDlgItem(IDC_BATTERY_INDICATOR))->GetCurSel();
    int charging = ((CComboBox *)GetDlgItem(IDC_BATTERY_CHARGING))->GetCurSel();
    int servicability = ((CComboBox *)GetDlgItem(IDC_BATTERY_SERVICABILITY))->GetCurSel();

    BYTE buffer[128];
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = battery_level;
    *p++ = time_to_discharge & 0xff;
    *p++ = (time_to_discharge >> 8) & 0xff;
    *p++ = (time_to_discharge >> 16) & 0xff;
    *p++ = time_to_charge & 0xff;
    *p++ = (time_to_charge >> 8) & 0xff;
    *p++ = (time_to_charge >> 16) & 0xff;
    *p++ = presence - 1;
    *p++ = indicator - 1;
    *p++ = charging - 1;
    *p++ = servicability - 1;

    m_trace->SetCurSel(m_trace->AddString(L"Sent Battery server state"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_BATTERY_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::ProcessBatteryStatus(LPBYTE p_data, DWORD len)
{
    if (len < 8)
    {
        Log(L"ProcessBatteryStatus %d", len);
        return;
    }
    SetDlgItemInt(IDC_BATTERY_LEVEL, p_data[0], FALSE);
    SetDlgItemInt(IDC_TIME_TO_DISCHARGE, p_data[1] + (p_data[2] << 8) + (p_data[3] << 16));
    SetDlgItemInt(IDC_TIME_TO_CHARGE, p_data[4] + (p_data[5] << 8) + (p_data[6] << 16));
    ((CComboBox *)GetDlgItem(IDC_BATTERY_PRESENSE))->SetCurSel(p_data[7]);
    ((CComboBox *)GetDlgItem(IDC_BATTERY_CHARGING))->SetCurSel(p_data[8]);
    ((CComboBox *)GetDlgItem(IDC_BATTERY_INDICATOR))->SetCurSel(p_data[9]);
    ((CComboBox *)GetDlgItem(IDC_BATTERY_SERVICABILITY))->SetCurSel(p_data[10]);
}

void CClientControlDlg::OnBnClickedLocationGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Location Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_GET, buffer, (DWORD)(p - buffer));
}

#define LOCATION_SEND_GLOBAL    1
#define LOCATION_SEND_LOCAL     2
#define LOCATION_SEND_BOTH      (LOCATION_SEND_GLOBAL | LOCATION_SEND_LOCAL)

void CClientControlDlg::OnBnClickedLocationSet()
{
    ReadValuesSendMsg(LOCATION_SEND_GLOBAL);
}

void CClientControlDlg::OnBnClickedLocationLocalGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Location Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLocationLocalSet()
{
    ReadValuesSendMsg(LOCATION_SEND_LOCAL);
}

void CClientControlDlg::ReadValuesSendMsg(BYTE local_global)
{
    DWORD global_latitude, global_longitude;
    DWORD global_altitude, local_north, local_east, local_altitude;
    BYTE floor_number;

    WCHAR buffer[80];
    GetDlgItemText(IDC_GLOBAL_LATITUDE, buffer, sizeof(buffer)/2);
    if (buffer[0] == 0)
    {
        global_latitude = 0x8000000;
    }
    else
    {
        //float f;
        //swscanf_s(buffer, L"%f", &f);
        //if ((f < -90.) || (f > 90.))
        //{
        //    f = 0.;
        //    //GetDlgItem(IDC_GLOBAL_LATITUDE)->SetFocus();
        //    // return;
        //}
        // global_latitude = (DWORD)floor((f / 90) * 0x7fffffff);
        global_latitude = GetDlgItemInt(IDC_GLOBAL_LATITUDE, NULL, 0);
    }
    GetDlgItemText(IDC_GLOBAL_LONGITUDE, buffer, sizeof(buffer)/2);
    if (buffer[0] == 0)
    {
        global_longitude = 0x8000000;
    }
    else
    {
        //float f;
        //swscanf_s(buffer, L"%f", &f);
        //if ((f < -180.) || (f > 180.))
        //{
        //    f = 0;
        //    // GetDlgItem(IDC_GLOBAL_LONGITUDE)->SetFocus();
        //    // return;
        //}
        //global_longitude = (DWORD)floor((f / 180) * 0x7fffffff);
        global_longitude = GetDlgItemInt(IDC_GLOBAL_LONGITUDE, NULL, 0);
    }
    GetDlgItemText(IDC_GLOBAL_ALTITUDE, buffer, sizeof(buffer)/2);
    if (buffer[0] == 0)
    {
        global_altitude = 0x7FFF;
    }
    else
    {
        int altitude = GetDlgItemInt(IDC_GLOBAL_ALTITUDE);
        if (altitude < -32768)
        {
            altitude = 0;
            // GetDlgItem(IDC_GLOBAL_ALTITUDE)->SetFocus();
            // return;
        }
        if (altitude > 32766)
        {
            global_altitude = 0x7FFE;
        }
        else
        {
            global_altitude = (USHORT)altitude;
        }
    }
    GetDlgItemText(IDC_LOCAL_NORTH, buffer, sizeof(buffer)/2);
    if (buffer[0] == 0)
    {
        local_north = 0x8000;
    }
    else
    {
        //int ii = GetDlgItemInt(IDC_LOCAL_NORTH);
        //if ((ii < -32767) || (ii > 32767))
        //{
        //    ii = 0;
        //    // GetDlgItem(IDC_LOCAL_NORTH)->SetFocus();
        //    // return;
        //}
        //else
        //{
        //    local_north = (USHORT)ii;
        //}
        local_north = GetDlgItemInt(IDC_LOCAL_NORTH);
    }
    GetDlgItemText(IDC_LOCAL_EAST, buffer, sizeof(buffer)/2);
    if (buffer[0] == 0)
    {
        local_east = 0x8000;
    }
    else
    {
        //int ii = GetDlgItemInt(IDC_LOCAL_EAST);
        //if ((ii < -32767) || (ii > 32767))
        //{
        //    ii = 0;
        //    // GetDlgItem(IDC_LOCAL_EAST)->SetFocus();
        //    // return;
        //}
        //else
        //{
        //    local_east = (USHORT)ii;
        //}
        local_east = GetDlgItemInt(IDC_LOCAL_EAST);
    }
    GetDlgItemText(IDC_LOCAL_ALTITUDE, buffer, sizeof(buffer));
    if (buffer[0] == 0)
    {
        local_altitude = 0x8000;
    }
    else
    {
        //int ii = GetDlgItemInt(IDC_LOCAL_ALTITUDE);
        //if ((ii < -32767) || (ii > 32767))
        //{
        //    ii = 0;
        //    // GetDlgItem(IDC_LOCAL_ALTITUDE)->SetFocus();
        //    // return;
        //}
        //else
        //{
        //    local_altitude = (USHORT)ii;
        //}
        local_altitude = GetDlgItemInt(IDC_LOCAL_ALTITUDE);
    }
    floor_number = (BYTE)((CComboBox *)GetDlgItem(IDC_FLOOR))->GetCurSel();

    BYTE is_mobile = (BYTE)((CButton *)GetDlgItem(IDC_MOBILE))->GetCheck();

    FLOAT two_power_x_minus_three[] = {0.125, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

    BYTE update_time;
    GetDlgItemText(IDC_UPDATE_TIME, buffer, sizeof(buffer));
    if (buffer[0] == 0)
    {
        update_time = 0;
    }
    else
    {
        // float f;
        // swscanf_s(buffer, L"%f", &f);

        // for (update_time = 0; update_time <= 15; update_time++)
        //{
        //    if (f <= two_power_x_minus_three[update_time])
        //        break;
        //}
        update_time = GetDlgItemInt(IDC_UPDATE_TIME);
    }

    BYTE precision;
    //GetDlgItemText(IDC_PRECISION, buffer, sizeof(buffer));
    //if (buffer[0] == 0)
    //{
    //    precision = 0;
    //}
    //else
    //{
    //    float f;
    //    swscanf_s(buffer, L"%f", &f);
    //
    //    for (precision = 0; precision <= 15; precision++)
    //    {
    //        if (f <= two_power_x_minus_three[precision])
    //            break;
    //    }
    //}
    precision = GetDlgItemInt(IDC_PRECISION);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE msg[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, msg, sizeof(msg));
    if (local_global & LOCATION_SEND_GLOBAL)
    {
        *p++ = global_latitude & 0xff;
        *p++ = (global_latitude >> 8) & 0xff;
        *p++ = (global_latitude >> 16) & 0xff;
        *p++ = (global_latitude >> 24) & 0xff;
        *p++ = global_longitude & 0xff;
        *p++ = (global_longitude >> 8) & 0xff;
        *p++ = (global_longitude >> 16) & 0xff;
        *p++ = (global_longitude >> 24) & 0xff;
        *p++ = global_altitude & 0xff;
        *p++ = (global_altitude >> 8) & 0xff;
        m_trace->SetCurSel(m_trace->AddString(L"Sent Location Global Set"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_SET, msg, (DWORD)(p - msg));
    }
    if (local_global & LOCATION_SEND_LOCAL)
    {
        *p++ = local_north & 0xff;
        *p++ = (local_north >> 8) & 0xff;
        *p++ = local_east & 0xff;
        *p++ = (local_east >> 8) & 0xff;
        *p++ = local_altitude & 0xff;
        *p++ = (local_altitude >> 8) & 0xff;
        *p++ = floor_number;
        *p++ = is_mobile;
        *p++ = update_time;
        *p++ = precision;
        m_trace->SetCurSel(m_trace->AddString(L"Sent Location Local Status"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_SET, msg, (DWORD)(p - msg));
    }
}

void CClientControlDlg::ProcessLocationGlobalSet(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    int global_latitude = p_data[0] + (p_data[1] << 8) + (p_data[2] << 16) + (p_data[3] << 24);
    float Latitude = (float)global_latitude / 0x7fffffff * 90;
        p_data += 4;
    if (global_latitude == 0x80000000)
        SetDlgItemText(IDC_GLOBAL_LATITUDE, L"");
    else
    {
        // float f = (float)global_latitude / 0x7fffffff * 90;
        // wsprintf(buffer, L"%f", f);
        // SetDlgItemText(IDC_GLOBAL_LATITUDE, buffer);
        SetDlgItemInt(IDC_GLOBAL_LATITUDE, global_latitude, 0);
    }

    int global_longitude = p_data[0] + (p_data[1] << 8) + (p_data[2] << 16) + (p_data[3] << 24);
    float Longitude = (float)global_longitude / 0x7fffffff * 180;
    p_data += 4;
    if (global_longitude == 0x80000000)
        SetDlgItemText(IDC_GLOBAL_LONGITUDE, L"");
    else
    {
        //float f = (float)global_longitude / 0x7fffffff * 180;
        //wsprintf(buffer, L"%f", f);
        // wsprintf(buffer, L"%x", global_longitude);
        // SetDlgItemText(IDC_GLOBAL_LONGITUDE, buffer);
        SetDlgItemInt(IDC_GLOBAL_LONGITUDE, global_longitude, 0);
    }

    USHORT global_altitude = p_data[0] + (p_data[1] << 8);
    if (global_altitude == 0x7FFF)
        SetDlgItemText(IDC_GLOBAL_ALTITUDE, L"");
    else
        SetDlgItemInt(IDC_GLOBAL_ALTITUDE, global_altitude);

    WCHAR buf[180];
    char buf1[100];
    sprintf(buf1, "Latitude %f(%d) Longitude %f(%d)", Latitude, global_latitude, Longitude, global_longitude);
    MultiByteToWideChar(CP_ACP, 0, (const char *)buf1, -1, buf, sizeof(buf) / sizeof(WCHAR));

    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLocationLocalSet(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    USHORT local_north = p_data[0] + (p_data[1] << 8);
    p_data += 2;
    if (local_north == 0x8000)
        SetDlgItemText(IDC_LOCAL_NORTH, L"");
    else
        SetDlgItemInt(IDC_LOCAL_NORTH, local_north);

    USHORT local_east = p_data[0] + (p_data[1] << 8);
    p_data += 2;
    if (local_east == 0x8000)
        SetDlgItemText(IDC_LOCAL_EAST, L"");
    else
        SetDlgItemInt(IDC_LOCAL_EAST, local_east);

    USHORT local_altitude = p_data[0] + (p_data[1] << 8);
    p_data += 2;
    if (local_altitude == 0x8000)
        SetDlgItemText(IDC_LOCAL_ALTITUDE, L"");
    else
        SetDlgItemInt(IDC_LOCAL_ALTITUDE, local_altitude);

    BYTE floor_number = *p_data++;
    ((CComboBox *)GetDlgItem(IDC_FLOOR))->SetCurSel(floor_number);

    BYTE is_mobile = *p_data++;
    ((CButton *)GetDlgItem(IDC_MOBILE))->SetCheck(is_mobile);

    FLOAT two_power_x_minus_three[] = { 0.125, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

    BYTE update_time = *p_data++;
    // wsprintf(buffer, L"%f", two_power_x_minus_three[update_time]);
    SetDlgItemInt(IDC_UPDATE_TIME, update_time);

    BYTE precision = *p_data++;
    // wsprintf(buffer, L"%f", two_power_x_minus_three[precision]);
    SetDlgItemInt(IDC_PRECISION, precision);
}

void CClientControlDlg::ProcessPropertiesStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180] = { 0 };
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Properties from:%x appKey:%x element:%d Type:%d ", src, app_key_idx, element_idx, *p_data++);
    for (len = len - 14; len > 0; len -= 2, p_data += 2)
    {
        wsprintf(&buf[wcslen(buf)], L"%04x ", p_data[0] + (p_data[1] << 8));
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessPropertyStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;
    len -= 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Property from:%x appKey:%x element:%d Type:%d Access:%d ID:%04x ", src, app_key_idx, element_idx, p_data[0], p_data[1], p_data[2] + (p_data[3] << 8));
    p_data += 4;
    len -= 4;
    for (DWORD i = 0; i < len; i++)
    {
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessSceneRegisterStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;
    len -= 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Scene Register from:%x appKey:%x element:%d Status:%d current scene:%d scenes ", src, app_key_idx, element_idx, p_data[0], p_data[1] + (p_data[2] << 8));

    for (len = len - 3, p_data = p_data + 3; len > 0; len -= 2, p_data += 2)
    {
        wsprintf(&buf[wcslen(buf)], L"%04x ", p_data[0] + (p_data[1] << 8));
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessSceneStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Scene from:%x appKey:%x element:%d Status:%d current:%d target:%d remaining_time:%d", src, app_key_idx, element_idx,
        p_data[0], p_data[1] + (p_data[2] << 8), p_data[3] + (p_data[4] << 8),
        p_data[5] + (p_data[6] << 8) + (p_data[7] << 16) + (p_data[8] << 24));
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessSchedulerStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Scheduler Status from:%x appKey:%x element:%d schedules ", src, app_key_idx, element_idx);

    USHORT schedules = p_data[0] + (p_data[1] << 8);
    for (int i = 0; i < 16; i++)
    {
        if ((schedules & (1 << i)) != 0)
            wsprintf(&buf[wcslen(buf)], L"%04x ", i);
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessSchedulerActionStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Scheduler action from:%x appKey:%x element:%d #:%d year:%d month:%d day:%d hour:%d min:%d second:%d day of week:%d action:%d time:%d scene:%d\n", src, app_key_idx, element_idx,
        p_data[0], p_data[1], p_data[2] + (p_data[3] << 8), p_data[4], p_data[5], p_data[6], p_data[7], p_data[8], p_data[9],
        p_data[10] + (p_data[11] << 8) + (p_data[12] << 16) + (p_data[13] << 24), p_data[14] + (p_data[15] << 8));
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessTimeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[300];
    ULONGLONG tai_seconds = (ULONGLONG)p_data[0] + ((ULONGLONG)p_data[1] << 8) + ((ULONGLONG)p_data[2] << 16) + ((ULONGLONG)p_data[3] << 24) + ((ULONGLONG)p_data[4] << 32);
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Time from:%x appKey:%x element:%d TAI seconds:%I64u(%I64x) subseconds:%d uncertainty:%d authority:%d tai_utc_delta:%d time_zone_offset:%d\n", src, app_key_idx, element_idx,
        tai_seconds, tai_seconds, p_data[5], p_data[6], p_data[7], p_data[8] + (p_data[9] << 8), p_data[10]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessTimeZoneStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    ULONGLONG tai_seconds = (ULONGLONG)p_data[2] + ((ULONGLONG)p_data[3] << 8) + ((ULONGLONG)p_data[4] << 16) + ((ULONGLONG)p_data[5] << 24) + ((ULONGLONG)p_data[6] << 32);
    wsprintf(buf, L"Time Zone Offset from:%x appKey:%x element:%d Current:%d New:%d TAI seconds of change:%I64u(%I64x)\n", src, app_key_idx, element_idx,
        p_data[0], p_data[1], tai_seconds, tai_seconds);

    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessTimeTaiDeltaStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    ULONGLONG tai_seconds = (ULONGLONG)p_data[4] + ((ULONGLONG)p_data[5] << 8) + ((ULONGLONG)p_data[6] << 16) + ((ULONGLONG)p_data[7] << 24) + ((ULONGLONG)p_data[8] << 32);
    wsprintf(buf, L"Time Zone Offset from:%x appKey:%x element:%d Current:%d New:%d TAI seconds of delta change:%I64u(%I64x)\n", src, app_key_idx, element_idx,
        p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8), tai_seconds, tai_seconds);

    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessTimeRoleStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"Time Role Status from:%x appKey:%x element:%d role:%d\n", src, app_key_idx, element_idx, p_data[0]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::OnBnClickedOnOffGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send OnOff Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_ONOFF_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedOnOffSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE target_state = (BYTE)((CComboBox *)GetDlgItem(IDC_ON_OFF_TARGET))->GetCurSel();
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_state;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send OnOff Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_ONOFF_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::ProcessOnOffSet(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[180];
    memset(buf, 0, sizeof(buf));
    wsprintf(buf, L"OnOff Set from:%x appKey:%x element:%d Target:%d Transition Time:%d Delay:%d", src, app_key_idx, element_idx,
        p_data[0], p_data[1] + (p_data[2] << 8) + (p_data[3] << 16) + (p_data[4] << 24), p_data[5] + (p_data[6] << 8));
}

void CClientControlDlg::ProcessOnOffStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE current_state = *p_data++;
    BYTE target_state = *p_data++;
    ((CComboBox *)GetDlgItem(IDC_ON_OFF_TARGET))->SetCurSel(target_state);
    DWORD remaining_time = p_data[0] + (p_data[1] << 8) + (p_data[2] << 16) + (p_data[3] << 24);
    wsprintf(buf, L"OnOff Status from:%x appKey:%x element:%d state:%d target:%d remaining time:%d", src, app_key_idx, element_idx, current_state, target_state, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightnessStatus(BOOL is_linear, LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT actual = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_CURRENT, actual, 0);
    USHORT target = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET, target, 0);
    DWORD remaining_time = p_data[4] + (p_data[5] << 8) + (p_data[6] << 16) + (p_data[7] << 24);
    if (is_linear)
        wsprintf(buf, L"Light Lightness Linear Status from:%x appKey:%x element:%d present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, actual, target, remaining_time);
    else
        wsprintf(buf, L"Light Lightness Actual Status from:%x appKey:%x element:%d present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, actual, target, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightnessLastStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    wsprintf(buf, L"Light Lightness Last Status from:%x appKey:%x element:%d lightness:%d", src, app_key_idx, element_idx, lightness);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightnessDefaultStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT, lightness, FALSE);
    wsprintf(buf, L"Light Lightness Default Status from:%x appKey:%x element:%d lightness:%d", src, app_key_idx, element_idx, lightness);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightnessRangeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE status = p_data[0];
    USHORT min = p_data[1] + (p_data[2] << 8);
    USHORT max = p_data[3] + (p_data[4] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MIN, min, FALSE);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MAX, max, FALSE);
    wsprintf(buf, L"Light Lightness Range Status from:%x appKey:%x element:%d status:%d min:%d max:%d", src, app_key_idx, element_idx, status, min, max);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessSensorDescriptorStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[380];

    UINT8 no_descriptors = p_data[0];
    int i;
    USHORT prop_id;
    USHORT pos_tol;
    USHORT neg_tol;
    UINT8  sampling_function;
    BYTE   measurement_period;
    BYTE   update_interval;
    int j = 1;
    for (i = 0; i < no_descriptors; i++)
    {
        prop_id = p_data[j] + (p_data[j + 1] << 8);
        pos_tol = p_data[j + 2] + (p_data[j + 3] << 8);
        neg_tol = p_data[j + 4] + (p_data[j + 5] << 8);
        sampling_function = p_data[j + 6];
        measurement_period = p_data[j + 7];
        update_interval = p_data[j + 8];
        j += 9;
        wsprintf(buf, L"Descriptor Status prop_id:%04X pos_tol:%d neg_tol:%d sampling_function:%d measurement_period:%d update_interval:%d", prop_id, pos_tol, neg_tol, sampling_function, measurement_period, update_interval);
        m_trace->SetCurSel(m_trace->AddString(buf));
    }
}

void CClientControlDlg::ProcessSensorStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200] = { 0 };
    wsprintf(buf, L"sensor status from:%x appKey:%x element:%d property_id:0x%04x len:%d ", src, app_key_idx, element_idx, p_data[0] + (p_data[1] << 8), p_data[2]);
    for (int i = 3; i < 3 + p_data[2] && i < 35; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", p_data[i]);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessVendorSpecificData(LPBYTE p_data, DWORD len)
{
    DWORD i;
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;
    len -= 13;

    WCHAR buf[1300];
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

void CClientControlDlg::OnBnClickedUsePublicationInfo()
{
    BOOL use_publication_info = (BYTE)((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck();
    GetDlgItem(IDC_DST)->EnableWindow(!use_publication_info);
    GetDlgItem(IDC_APP_KEY_IDX)->EnableWindow(!use_publication_info);
}

void CClientControlDlg::ProcessLightCtlStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT present_lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_CURRENT, present_lightness, 0);
    USHORT present_temperature = p_data[2] + (p_data[3] << 8);
    USHORT target_lightness = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET, target_lightness, 0);
    USHORT target_temperature = p_data[6] + (p_data[7] << 8);
    SetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET, target_temperature, 0);
    DWORD remaining_time = p_data[8] + (p_data[9] << 8) + (p_data[10] << 16) + (p_data[11] << 24);
    wsprintf(buf, L"Light CTL Status from:%x appKey:%x element:%d Lightness present:%d target:%d Temperature present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, present_lightness, target_lightness, present_temperature, target_temperature, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightCtlTemperatureStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT present_temperature = p_data[0] + (p_data[1] << 8);
    USHORT present_delta_uv = p_data[2] + (p_data[3] << 8);
    USHORT target_temperature = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET, target_temperature, 0);
    USHORT target_delta_uv = p_data[6] + (p_data[7] << 8);
    SetDlgItemInt(IDC_LIGHT_CTL_DELTA_UV_TARGET, target_delta_uv, 0);
    DWORD remaining_time = p_data[8] + (p_data[9] << 8) + (p_data[10] << 16) + (p_data[11] << 24);
    wsprintf(buf, L"Light CTL Status from:%x appKey:%x element:%d Temperature  present:%d target:%d Delta UV present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, present_temperature, target_temperature, present_delta_uv, target_delta_uv, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightCtlTemperatureRangeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE status = *p_data++;
    USHORT range_min = p_data[0] + (p_data[1] << 8);
    USHORT range_max = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MIN, range_min, 0);
    SetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MAX, range_max, 0);
    wsprintf(buf, L"Light CTL Status from:%x appKey:%x element:%d Temperature Range status:%d min:%d max:%d", src, app_key_idx, element_idx, status, range_min, range_max);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightCtlDefaultStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT, lightness, 0);
    USHORT temperature = p_data[2] + (p_data[3] << 8);
    USHORT delta_uv = p_data[4] + (p_data[5] << 8);
    wsprintf(buf, L"Light CTL Default Status from:%x appKey:%x element:%d lightness:%d temperature:%d delta_uv:%d", src, app_key_idx, element_idx, lightness, temperature, delta_uv);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_CURRENT, lightness, 0);
    USHORT hue = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE, hue, 0);
    USHORT saturation = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE, saturation, 0);
    DWORD remaining_time = p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24);
    wsprintf(buf, L"Light HSL Status from:%x appKey:%x element:%d Present lightness:%d hue:%d saturation:%d remaining time:%d", src, app_key_idx, element_idx, lightness, hue, saturation, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslTargetStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET, lightness, 0);
    USHORT hue = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE, hue, 0);
    USHORT saturation = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE, saturation, 0);
    DWORD remaining_time = p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24);
    wsprintf(buf, L"Light HSL Status from:%x appKey:%x element:%d Target lightness:%d hue:%d saturation:%d remaining time:%d", src, app_key_idx, element_idx, lightness, hue, saturation, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslHueStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT present_hue = p_data[0] + (p_data[1] << 8);
    USHORT target_hue = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE, target_hue, 0);
    DWORD remaining_time = p_data[4] + (p_data[5] << 8) + (p_data[6] << 16) + (p_data[7] << 24);
    wsprintf(buf, L"Light HSL Status from:%x appKey:%x element:%d Hue present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, present_hue, target_hue, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslSaturationStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT present_saturation = p_data[0] + (p_data[1] << 8);
    USHORT target_saturation = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE, target_saturation, 0);
    DWORD remaining_time = p_data[4] + (p_data[5] << 8) + (p_data[6] << 16) + (p_data[7] << 24);
    wsprintf(buf, L"Light HSL Status from:%x appKey:%x element:%d Saturation present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, present_saturation, target_saturation, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslRangeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE status = *p_data++;
    USHORT hue_range_min = p_data[0] + (p_data[1] << 8);
    USHORT hue_range_max = p_data[2] + (p_data[3] << 8);
    USHORT saturation_range_min = p_data[0] + (p_data[1] << 8);
    USHORT saturation_range_max = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_HUE_RANGE_MIN, hue_range_min, 0);
    SetDlgItemInt(IDC_LIGHT_HUE_RANGE_MAX, hue_range_max, 0);
    SetDlgItemInt(IDC_LIGHT_SATURATION_RANGE_MIN, saturation_range_min, 0);
    SetDlgItemInt(IDC_LIGHT_SATURATION_RANGE_MAX, saturation_range_max, 0);
    wsprintf(buf, L"Light HSL Status Range from:%x appKey:%x element:%d status:%d Hue min:%d max:%d Saturation min:%d max:%d", src, app_key_idx, element_idx, status, hue_range_min, hue_range_max, saturation_range_min, saturation_range_max);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightHslDefaultStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT, lightness, 0);
    USHORT hue = p_data[2] + (p_data[3] << 8);
    USHORT saturation = p_data[4] + (p_data[5] << 8);
    wsprintf(buf, L"Light HSL Default Status from:%x appKey:%x element:%d lightness:%d hue:%d saturation:%d", src, app_key_idx, element_idx, lightness, hue, saturation);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightXylStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_CURRENT, lightness, 0);
    USHORT x = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_XYL_X_TARGET, x, 0);
    USHORT y = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_XYL_Y_TARGET, y, 0);
    DWORD remaining_time = p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24);
    wsprintf(buf, L"Light xyL Status from:%x appKey:%x element:%d Present xyL lightness:%d xyL x:%d xyL y:%d remaining time:%d", src, app_key_idx, element_idx, lightness, x, y, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightXylTargetStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET, lightness, 0);
    USHORT x = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_XYL_X_TARGET, x, 0);
    USHORT y = p_data[4] + (p_data[5] << 8);
    SetDlgItemInt(IDC_LIGHT_XYL_Y_TARGET, y, 0);
    DWORD remaining_time = p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24);
    wsprintf(buf, L"Light xyL Status from:%x appKey:%x element:%d Target xyL lightness:%d xyL x:%d xyL y:%d remaining time:%d", src, app_key_idx, element_idx, lightness, x, y, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightXylRangeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE status = *p_data++;
    USHORT x_range_min = p_data[0] + (p_data[1] << 8);
    USHORT x_range_max = p_data[2] + (p_data[3] << 8);
    USHORT y_range_min = p_data[0] + (p_data[1] << 8);
    USHORT y_range_max = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LIGHT_XYL_X_MIN, x_range_min, 0);
    SetDlgItemInt(IDC_LIGHT_XYL_X_MAX, x_range_max, 0);
    SetDlgItemInt(IDC_LIGHT_XYL_Y_MIN, y_range_min, 0);
    SetDlgItemInt(IDC_LIGHT_XYL_Y_MAX, y_range_max, 0);
    wsprintf(buf, L"Light xyL Status from:%x appKey:%x element:%d Range status:%d x min:%d max:%d y min:%d max:%d", src, app_key_idx, element_idx, status, x_range_min, x_range_max, y_range_min, y_range_max);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightXylDefaultStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT lightness = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT, lightness, 0);
    USHORT x = p_data[2] + (p_data[3] << 8);
    USHORT y = p_data[4] + (p_data[5] << 8);
    wsprintf(buf, L"Light xyL Default Status from:%x appKey:%x element:%d xyL lightness:%d xyL x:%d xyL y:%d", src, app_key_idx, element_idx, lightness, x, y);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightLcModeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE mode = *p_data++;
    wsprintf(buf, L"Light LC from:%x appKey:%x element:%d Mode:%d", src, app_key_idx, element_idx, mode);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightLcOccupancyModeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE mode = *p_data++;
    wsprintf(buf, L"Light LC from:%x appKey:%x element:%d Occupancy Mode:%d", src, app_key_idx, element_idx, mode);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightLcLightOnOffStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE present = p_data[0];
    BYTE target = p_data[1];
    DWORD remaining_time = p_data[2] + (p_data[3] << 8) + (p_data[4] << 16) + (p_data[5] << 24);
    wsprintf(buf, L"Light LC OnOff Status from:%x appKey:%x element:%d Present:%d Target:%d remaining time:%d", src, app_key_idx, element_idx, present, target, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessLightLcPropertyStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;
    len -= 13;

    WCHAR buf[200];
    USHORT id = p_data[0] + (p_data[1] << 8);
    DWORD value = 0;
    if (len == 3)
        value = p_data[2];
    else if (len == 4)
        value = p_data[2] + (p_data[3] << 8);
    else if (len == 5)
        value = p_data[2] + (p_data[3] << 8) + (p_data[4] << 16);
    else if (len == 6)
        value = p_data[2] + (p_data[3] << 8) + (p_data[4] << 16) + (p_data[5] << 24);
    wsprintf(buf, L"Light LC Property Status from:%x appKey:%x element:%d ID:0x%x Value:0x%x", src, app_key_idx, element_idx, id, value);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::OnBnClickedLevelGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Level Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LEVEL_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLevelSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_level = (SHORT)GetDlgItemInt(IDC_LEVEL_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_level & 0xff;
    *p++ = (target_level >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Level Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LEVEL_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedDeltaSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT delta_level = (SHORT)GetDlgItemInt(IDC_LEVEL_DELTA);
    BYTE  delta_continue = (BYTE)((CButton *)GetDlgItem(IDC_DELTA_CONTINUE))->GetCheck();
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = delta_level & 0xff;
    *p++ = (delta_level >> 8) & 0xff;
    *p++ = (delta_level >> 16) & 0xff;
    *p++ = (delta_level >> 24) & 0xff;
    *p++ = delta_continue;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Delta Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LEVEL_DELTA_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedMoveSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT delta_level = (SHORT)GetDlgItemInt(IDC_LEVEL_DELTA);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = delta_level & 0xff;
    *p++ = (delta_level >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Move Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LEVEL_MOVE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::ProcessLevelStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    SHORT present_level = p_data[0] + (p_data[1] << 8);
    SetDlgItemInt(IDC_LEVEL_CURRENT, present_level);
    SHORT target_level = p_data[2] + (p_data[3] << 8);
    SetDlgItemInt(IDC_LEVEL_TARGET, target_level);
    DWORD remaining_time = p_data[4] + (p_data[5] << 8) + (p_data[6] << 16) + (p_data[7] << 24);
    wsprintf(buf, L"Level Status from:%x appKey:%x element:%d present:%d target:%d remaining time:%d", src, app_key_idx, element_idx, present_level, target_level, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::OnBnClickedDefaultTransitionTimeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Default Transition Time Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_DEF_TRANS_TIME_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedDefaultTransitionTimeSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    DWORD transition_time = GetDlgItemInt(IDC_DEFAULT_TRANSITION_TIME);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Default Transition Time Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_DEF_TRANS_TIME_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::ProcessDefaultTransitionTimeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    USHORT company_id = p_data[8] + (p_data[9] << 8);
    USHORT opcode = p_data[10] + (p_data[11] << 8);
    p_data += 12;

    WCHAR buf[200];
    DWORD transition_time = p_data[0] + (p_data[1] << 8) + (p_data[2] << 16) + (p_data[3] << 24);
    SetDlgItemInt(IDC_TRANSITION_TIME, transition_time);
    wsprintf(buf, L"Default Transition Time Status from:%x appKey:%x element:%d transition time:%d", src, app_key_idx, element_idx, transition_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}


void CClientControlDlg::OnBnClickedPowerOnOffGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send OnPowerUp Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_ONPOWERUP_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedPowerOnOffSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE on_power_up_state = (BYTE)((CComboBox *)GetDlgItem(IDC_POWER_ON_OFF_STATE))->GetCurSel();

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = on_power_up_state;

    m_trace->SetCurSel(m_trace->AddString(L"Send OnPowerUp Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_ONPOWERUP_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::ProcessPowerOnOffStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE power_on_off_state = *p_data++;
    ((CComboBox *)GetDlgItem(IDC_POWER_ON_OFF_STATE))->SetCurSel(power_on_off_state);
    wsprintf(buf, L"Power OnPowerUp Status from:%x appKey:%x element:%d state:%d", src, app_key_idx, element_idx, power_on_off_state);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessPowerLevelStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT level1 = p_data[0] + (p_data[1] << 8);
    USHORT level2 = p_data[2] + (p_data[3] << 8);
    DWORD remaining_time = p_data[4] + (p_data[5] << 8) + (p_data[6] << 16) + (p_data[7] << 24);
    wsprintf(buf, L"Power Level Status from:%x appKey:%x element:%d Present:%d Target:%d remaining time:%d", src, app_key_idx, element_idx, level1, level2, remaining_time);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessPowerLevelDefaultStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT level = p_data[0] + (p_data[1] << 8);
    wsprintf(buf, L"Power Level Default from:%x appKey:%x element:%d Level:%d", src, app_key_idx, element_idx, level);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessPowerLevelLastStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    USHORT level = p_data[0] + (p_data[1] << 8);
    wsprintf(buf, L"Power Level Last from:%x appKey:%x element:%d level:%d", src, app_key_idx, element_idx, level);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::ProcessPowerLevelRangeStatus(LPBYTE p_data, DWORD len)
{
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT dst = p_data[2] + (p_data[3] << 8);
    USHORT app_key_idx = p_data[4] + (p_data[5] << 8);
    BYTE   element_idx = p_data[6];
    BYTE   rssi = p_data[7];
    BYTE   ttl = p_data[8];
    USHORT company_id = p_data[9] + (p_data[10] << 8);
    USHORT opcode = p_data[11] + (p_data[12] << 8);
    p_data += 13;

    WCHAR buf[200];
    BYTE status = p_data[0];
    USHORT min = p_data[1] + (p_data[2] << 8);
    USHORT max = p_data[3] + (p_data[4] << 8);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MIN, min, FALSE);
    SetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MAX, max, FALSE);
    wsprintf(buf, L"Power Level Range from:%x appKey:%x element:%d Status %d min:%d max:%d", src, app_key_idx, element_idx, status, min, max);
    m_trace->SetCurSel(m_trace->AddString(buf));
}

void CClientControlDlg::OnBnClickedPowerLevelGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_level = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_level & 0xff;
    *p++ = (target_level >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelLastGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Last Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_LAST_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelDefaultGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Default Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedPowerLevelDefaultSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT default_level = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = default_level & 0xff;
    *p++ = (default_level >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Default Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelRangeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Range Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelRangeSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT range_min = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MIN);
    SHORT range_max = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MAX);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = range_min & 0xff;
    *p++ = (range_min >> 8) & 0xff;
    *p++ = range_max & 0xff;
    *p++ = (range_max >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Power Level Range Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPowerLevelStatus()
{
    SHORT current_state = GetDlgItemInt(IDC_POWER_LEVEL_CURRENT);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 0, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = current_state & 0xff;
    *p++ = (current_state >> 8) & 0xff;
    *p++ = 0;   // remaining time is currently 0
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;

    m_trace->SetCurSel(m_trace->AddString(L"Send Level Status"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedTcNetLevelTrxSend()
{
    m_trace->SetCurSel(m_trace->AddString(L"Network Layer: Transmit Adv Msg"));
    BYTE    ctl = ((CButton *)GetDlgItem(IDC_TC_NET_LEVEL_TRX_CTL))->GetCheck() != 0 ? 1 : 0;
    BYTE    ttl = (BYTE)GetHexValueInt(IDC_TC_NET_LEVEL_TRX_TTL);
    USHORT  dst = (USHORT)GetHexValueInt(IDC_TC_NET_LEVEL_TRX_DST);
    BYTE    buffer[20];
    LPBYTE  p = buffer;
    *p++ = ctl;
    *p++ = ttl;
    *p++ = (dst >> 8) & 0xff;
    *p++ = dst & 0xff;
    p += GetHexValue(IDC_TC_NET_LEVEL_TRX_PDU, p, 16);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_NETWORK_LAYER_TRNSMIT, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedTcTranspLevelSend()
{
    BYTE    ctl = ((CButton *)GetDlgItem(IDC_TC_NET_LEVEL_TRX_CTL))->GetCheck() != 0 ? 1 : 0;
    BYTE    ttl = (BYTE)GetHexValueInt(IDC_TC_NET_LEVEL_TRX_TTL);
    BYTE    szmic = ((CButton *)GetDlgItem(IDC_TC_TRANSP_LEVEL_SZMIC))->GetCheck() != 0 ? 1 : 0;
    BYTE    buffer[20 + 380] = { 0 };
    LPBYTE  p = buffer;

    *p++ = ctl;
    *p++ = ttl;
    *p = (BYTE)GetHexValue(IDC_TC_NET_LEVEL_TRX_DST, p+1, 16);
    if (*p != 2 && *p != 16)
    {
        m_trace->SetCurSel(m_trace->AddString(L"Transport Layer: Invalid DST length. Should be 2 or 16"));
        return;
    }
    p += 1 + 16;
    *p++ = szmic;
    p += GetHexValue(IDC_TC_NET_LEVEL_TRX_PDU, p, 380);
    m_trace->SetCurSel(m_trace->AddString(L"Transport Layer: Transmit Adv Msg"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_TRANSPORT_LAYER_TRNSMIT, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedTcIvUpdateTransit()
{
    m_trace->SetCurSel(m_trace->AddString(L"IV UPDATE Transit Signal"));
    BYTE    in_progress = ((CButton *)GetDlgItem(IDC_TC_IV_UPDATE))->GetCheck() != 0 ? 1 : 0;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_IVUPDATE_SIGNAL_TRNSIT, &in_progress, 1);
}


void CClientControlDlg::OnBnClickedPropertiesGet()
{
    WCHAR buf[40];
    BYTE type = ((CComboBox *)GetDlgItem(IDC_PROPERTY_TYPE))->GetCurSel();
    USHORT starting_id = (USHORT)GetHandle(IDC_PROPERTY_ID);
    wsprintf(buf, L"Properties Type:%d Get starting ID:%x", type, starting_id);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = (BYTE)type;
    // Starting ID is valid for Client Properties Only
    if (type == 0)
    {
        *p++ = starting_id & 0xff;
        *p++ = (starting_id >> 8) & 0xff;
    }
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROPERTIES_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPropertyGet()
{
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    USHORT id = (USHORT)GetHexValueInt(IDC_PROPERTY_ID);
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 16) // Light LC
    {
        *p++ = id & 0xff;
        *p++ = (id >> 8) & 0xff;
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_GET, buffer, (DWORD)(p - buffer));
        return;
    }
    BYTE type = ((CComboBox *)GetDlgItem(IDC_PROPERTY_TYPE))->GetCurSel();
    *p++ = type;
    *p++ = id & 0xff;
    *p++ = (id >> 8) & 0xff;
    WCHAR buf[40];
    wsprintf(buf, L"Property Type:%d Get ID:%x", type, id);
    m_trace->SetCurSel(m_trace->AddString(buf));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROPERTY_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedPropertySet()
{
    WCHAR buf[40];
    USHORT id = (USHORT)GetHexValueInt(IDC_PROPERTY_ID);
    USHORT dst = 0, app_key_idx = 0;
    BYTE type = ((CComboBox *)GetDlgItem(IDC_PROPERTY_TYPE))->GetCurSel();
    BYTE access = ((CComboBox *)GetDlgItem(IDC_PROPERTY_ACCESS))->GetCurSel();
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = type;
    *p++ = id & 0xff;
    *p++ = (id >> 8) & 0xff;
    *p++ = access;
    p += GetHexValue(IDC_PROPERTY_VALUE, p, sizeof(buffer) - (p - buffer));
    wsprintf(buf, L"Property Type:%d Set ID:%x", type, id);
    m_trace->SetCurSel(m_trace->AddString(buf));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_PROPERTY_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLightnessGet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelGet();
        return;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    int linear = ((CButton *)GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR))->GetCheck();
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(linear ? HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LINEAR_GET : HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLightnessSet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelSet();
        return;
    }
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_level = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);
    int linear = ((CButton *)GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR))->GetCheck();

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_level & 0xff;
    *p++ = (target_level >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Set"));
    m_ComHelper->SendWicedCommand(linear ? HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LINEAR_SET :HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLightnessLastGet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelLastGet();
        return;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Last Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LAST_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLightnessDefaultGet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelDefaultGet();
        return;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Default Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_DEFAULT_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLightnessDefaultSet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelDefaultSet();
        return;
    }
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT default_level = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_DEFAULT);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = default_level & 0xff;
    *p++ = (default_level >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Default Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_DEFAULT_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLightnessRangeGet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelRangeGet();
        return;
    }
    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Range Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_RANGE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLightnessRangeSet()
{
    if (((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel() == 4) // power level
    {
        OnBnClickedPowerLevelRangeSet();
        return;
    }
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT range_min = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MIN);
    SHORT range_max = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_RANGE_MAX);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = range_min & 0xff;
    *p++ = (range_min >> 8) & 0xff;
    *p++ = range_max & 0xff;
    *p++ = (range_max >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light Lightness Range Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_RANGE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTcLpnFrndClear()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send LPN Friend Clear"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_LOW_POWER_SEND_FRIEND_CLEAR, NULL, 0);
}

void CClientControlDlg::OnCbnSelchangeSensorMsg()
{
    // TODO: Add your control notification handler code here
}

void CClientControlDlg::OnBnClickedSensorMsgSend()
{
    USHORT dst = 0, app_key_idx = 0;
    BYTE buffer[128];
    CString propIdStr;
    CString rawValx1Str;
    CString rawValx2Str;
    CString rawValxStr;
    CString settingPropIdStr;
    CString propvalStr;
    UINT16  u16;
    UINT32  u32;

    int currentSelectedCommand = sensorComMsg->GetCurSel();
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();

    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    ((CEdit *)GetDlgItem(IDC_SENSOR_PROP_ID))->GetWindowTextW(propIdStr);

    switch (currentSelectedCommand)
    {
    case DESCRIPTOR_GET:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        m_trace->SetCurSel(m_trace->AddString(L"Descriptor Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_DESCRIPTOR_GET, buffer, (DWORD)(p - buffer));
        break;

    case SENSOR_GET:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
            m_trace->SetCurSel(m_trace->AddString(L"Sensor Get"));
            m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_GET, buffer, (DWORD)(p - buffer));
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Sensor Get"));
            m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_GET, buffer, (DWORD)(p - buffer));
            return;
        }
        break;

    case SERIES_GET:
        ((CEdit *)GetDlgItem(IDC_SENSOR_RAW_VAL_X1))->GetWindowTextW(rawValx1Str);
        ((CEdit *)GetDlgItem(IDC_SENSOR_CL_RAW_VAL_X2))->GetWindowTextW(rawValx2Str);

        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        ((CEdit *)GetDlgItem(IDC_SENSOR_PROP_VAL_LEN))->GetWindowTextW(propvalStr);
        if (!propvalStr.IsEmpty())
        {
            prop_value_len = (UINT8)GetHexValueInt(IDC_SENSOR_PROP_VAL_LEN);
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId val len"));
            return;
        }
        if (!rawValx1Str.IsEmpty() && !rawValx1Str.IsEmpty())
        {
           //set values
            *p++ = prop_value_len;
            p += GetHexValue(IDC_SENSOR_RAW_VAL_X1, p, prop_value_len);
            p += GetHexValue(IDC_SENSOR_CL_RAW_VAL_X2, p, prop_value_len);

        }
        m_trace->SetCurSel(m_trace->AddString(L"Series Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_SERIES_GET, buffer, (DWORD)(p - buffer));
        break;

    case COLUMN_GET:
        ((CEdit *)GetDlgItem(IDC_SENSOR_RAW_VAL_X))->GetWindowTextW(rawValxStr);

        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }

        ((CEdit *)GetDlgItem(IDC_SENSOR_PROP_VAL_LEN))->GetWindowTextW(propvalStr);
        if (!propvalStr.IsEmpty())
        {
            prop_value_len = (UINT8)GetHexValueInt(IDC_SENSOR_PROP_VAL_LEN);
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId val len"));
            return;
        }
        *p++ = prop_value_len;

        if (!rawValxStr.IsEmpty() && !rawValxStr.IsEmpty())
        {
            //set values
            p += GetHexValue(IDC_SENSOR_RAW_VAL_X, p, prop_value_len);
        }
        m_trace->SetCurSel(m_trace->AddString(L"Column Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_COLUMN_GET, buffer, (DWORD)(p - buffer));
        break;

    case CADENCE_GET:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        m_trace->SetCurSel(m_trace->AddString(L"Cadence Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_GET, buffer, (DWORD)(p - buffer));
        break;

    case SETTINGS_GET:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        m_trace->SetCurSel(m_trace->AddString(L"Settings Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTINGS_GET, buffer, (DWORD)(p - buffer));
        break;

    case SETTING_GET :
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }

        ((CEdit *)GetDlgItem(IDC_SENSOR_SETTING_PROP_ID))->GetWindowTextW(settingPropIdStr);
        if (!settingPropIdStr.IsEmpty())
        {
            setting_property_id = (USHORT)GetHexValueInt(IDC_SENSOR_SETTING_PROP_ID);
            *p++ = setting_property_id & 0xff;
            *p++ = (setting_property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set Setting PropertyId"));
            return;
        }

        m_trace->SetCurSel(m_trace->AddString(L"Setting Get"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_GET, buffer, (DWORD)(p - buffer));
        break;

    case SETTING_SET:
        ((CEdit *)GetDlgItem(IDC_SENSOR_SETTING_VAL))->GetWindowTextW(rawValxStr);

        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        ((CEdit *)GetDlgItem(IDC_SENSOR_SETTING_PROP_ID))->GetWindowTextW(settingPropIdStr);
        if (!settingPropIdStr.IsEmpty())
        {
            setting_property_id = (USHORT)GetHexValueInt(IDC_SENSOR_SETTING_PROP_ID);
            *p++ = setting_property_id & 0xff;
            *p++ = (setting_property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set Setting PropertyId"));
            return;
        }
        ((CEdit *)GetDlgItem(IDC_SENSOR_PROP_VAL_LEN))->GetWindowTextW(propvalStr);
        if (!propvalStr.IsEmpty())
        {
            prop_value_len = (UINT8)GetHexValueInt(IDC_SENSOR_PROP_VAL_LEN);
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId val len"));
            return;
        }

        if (!rawValxStr.IsEmpty() && !rawValxStr.IsEmpty())
        {
            *p++ = prop_value_len;
            //set values
            m_trace->SetCurSel(m_trace->AddString(L"Setting Set"));
            p += GetHexValue(IDC_SENSOR_SETTING_VAL, p, prop_value_len);
        }
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_SET, buffer, (DWORD)(p - buffer));
        break;

    case CADENCE_SET:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        GetDlgItem(IDC_SENSOR_PROP_VAL_LEN)->GetWindowTextW(propvalStr);
        if (!propvalStr.IsEmpty())
        {
            prop_value_len = (UINT8)GetHexValueInt(IDC_SENSOR_PROP_VAL_LEN);
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId val len"));
            return;
        }

        *p++ = prop_value_len;

        u16 = (UINT16)GetHexValueInt(IDC_SENSOR_FS_CAD_DIV);
        *p++ = u16 & 0xff;
        *p++ = (u16 >> 8) & 0xff;

        if (((CButton *)GetDlgItem(IDC_SENSOR_CL_TRIG_TYPE))->GetCheck())
        {
            *p++ = 1;
            GetHexValue(IDC_SENSOR_CL_TRIG_DWN, p, 2);
            p += 2;
            GetHexValue(IDC_SENSOR_CL_TRIG_DEL_UP, p, 2);
            p += 2;
        }
        else
        {
            *p++ = 0;
            GetHexValue(IDC_SENSOR_CL_TRIG_DWN, p, prop_value_len);
            p += prop_value_len;
            GetHexValue(IDC_SENSOR_CL_TRIG_DEL_UP, p, prop_value_len);
            p += prop_value_len;
        }
        u32 = GetHexValueInt(IDC_SENSOR_CL_MIN_INT);
        *p++ = u32 & 0xff;
        *p++ = (u32 >> 8) & 0xff;
        *p++ = (u32 >> 16) & 0xff;
        *p++ = (u32 >> 24) & 0xff;

        p += GetHexValue(IDC_SENSOR_CL_FST_CAD_LOW, p, prop_value_len);
        p += GetHexValue(IDC_SENSOR_CL_FST_CAD_HIGH, p, prop_value_len);

        m_trace->SetCurSel(m_trace->AddString(L"Cadence Set"));
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_SET, buffer, (DWORD)(p - buffer));
        break;

    case SENSOR_STATUS:
        if (!propIdStr.IsEmpty())
        {
            property_id = (USHORT)GetHexValueInt(IDC_SENSOR_PROP_ID);
            *p++ = property_id & 0xff;
            *p++ = (property_id >> 8) & 0xff;
        }
        else
        {
            m_trace->SetCurSel(m_trace->AddString(L"Please set PropertyId"));
            return;
        }
        *p++ = 0x01 & 0xff;
        *p++ = 0x00;
        prop_value_len = GetDlgItemInt(IDC_SENSOR_PROP_VAL_LEN);
        *p++ = prop_value_len;
        WCHAR buf[100];
        wsprintf(buf, L"Sensor Set Prop ID:%04x Value:%d", property_id, prop_value_len);
        m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SENSOR_SET, buffer, (DWORD)(p - buffer));
        break;
    }
}

void CClientControlDlg::OnBnClickedLightHslGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_hue = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE);
    SHORT target_saturation = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_hue & 0xff;
    *p++ = (target_hue >> 8) & 0xff;
    *p++ = target_saturation & 0xff;
    *p++ = (target_saturation >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslHueGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Hue Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslHueSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_hue = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_hue & 0xff;
    *p++ = (target_hue >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Hue Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslSaturationGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Saturation Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslSaturationSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_saturation = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_saturation & 0xff;
    *p++ = (target_saturation >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Saturation Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightHslTargetGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Target Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_TARGET_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightHslDefaultGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Default Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslDefaultSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_hue = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_HUE_VALUE);
    SHORT target_saturation = (SHORT)GetDlgItemInt(IDC_LIGHT_HSL_SATURATION_VALUE);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_hue & 0xff;
    *p++ = (target_hue >> 8) & 0xff;
    *p++ = target_saturation & 0xff;
    *p++ = (target_saturation >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Default Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslRangeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Client Range Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightHslRangeSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT hue_min = (SHORT)GetDlgItemInt(IDC_LIGHT_HUE_RANGE_MIN);
    SHORT hue_max = (SHORT)GetDlgItemInt(IDC_LIGHT_HUE_RANGE_MAX);
    SHORT saturation_min = (SHORT)GetDlgItemInt(IDC_LIGHT_SATURATION_RANGE_MIN);
    SHORT saturation_max = (SHORT)GetDlgItemInt(IDC_LIGHT_SATURATION_RANGE_MAX);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = hue_min & 0xff;
    *p++ = (hue_min >> 8) & 0xff;
    *p++ = hue_max & 0xff;
    *p++ = (hue_max >> 8) & 0xff;
    *p++ = saturation_min & 0xff;
    *p++ = (saturation_min >> 8) & 0xff;
    *p++ = saturation_max & 0xff;
    *p++ = (saturation_max >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light HSL Range Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightCtlGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_temperature = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET);
    SHORT target_delta_uv = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_DELTA_UV_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_temperature & 0xff;
    *p++ = (target_temperature >> 8) & 0xff;
    *p++ = target_delta_uv & 0xff;
    *p++ = (target_delta_uv >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightCtlTemperatureGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Client Temperature Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlTemperatureSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_temperature = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET);
    SHORT target_delta_uv = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_DELTA_UV_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_temperature & 0xff;
    *p++ = (target_temperature >> 8) & 0xff;
    *p++ = target_delta_uv & 0xff;
    *p++ = (target_delta_uv >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Temperature Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlDefaultGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Client Default Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_DEFAULT_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlDefaultSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_temperature = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_TARGET);
    SHORT target_delta_uv = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_DELTA_UV_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_temperature & 0xff;
    *p++ = (target_temperature >> 8) & 0xff;
    *p++ = target_delta_uv & 0xff;
    *p++ = (target_delta_uv >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Default Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_DEFAULT_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlTemperatureRangeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Client Temperature Range Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_RANGE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightCtlTemperatureRangeSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT range_min = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MIN);
    SHORT range_max = (SHORT)GetDlgItemInt(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MAX);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = range_min & 0xff;
    *p++ = (range_min >> 8) & 0xff;
    *p++ = range_max & 0xff;
    *p++ = (range_max >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light CTL Temperature Range Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_RANGE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightXylGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightXylSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_x = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_X_TARGET);
    SHORT target_y = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_Y_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_x & 0xff;
    *p++ = (target_x >> 8) & 0xff;
    *p++ = target_y & 0xff;
    *p++ = (target_y >> 8) & 0xff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightXylStatus()
{
    // TODO: Add your control notification handler code here
}

void CClientControlDlg::OnBnClickedLightXylTargetGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Default Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_TARGET_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightXylDefaultGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Default Client Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightXylDefaultSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT target_lightness = (SHORT)GetDlgItemInt(IDC_LIGHT_LIGHTNESS_TARGET);
    SHORT target_x = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_X_TARGET);
    SHORT target_y = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_Y_TARGET);
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = target_lightness & 0xff;
    *p++ = (target_lightness >> 8) & 0xff;
    *p++ = target_x & 0xff;
    *p++ = (target_x >> 8) & 0xff;
    *p++ = target_y & 0xff;
    *p++ = (target_y >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Default Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_SET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightXylRangeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Client Range Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightXylRangeSet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    SHORT x_min = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_X_MIN);
    SHORT x_max = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_X_MAX);
    SHORT y_min = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_Y_MIN);
    SHORT y_max = (SHORT)GetDlgItemInt(IDC_LIGHT_XYL_Y_MAX);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = x_min & 0xff;
    *p++ = (x_min >> 8) & 0xff;
    *p++ = x_max & 0xff;
    *p++ = (x_max >> 8) & 0xff;
    *p++ = y_min & 0xff;
    *p++ = (y_min >> 8) & 0xff;
    *p++ = y_max & 0xff;
    *p++ = (y_max >> 8) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Light xyL Range Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcModeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Mode Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcModeSet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Mode Set"));

    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE mode = ((CComboBox *)GetDlgItem(IDC_LIGHT_LC_MODE))->GetCurSel();

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = mode;

    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcOccupancyModeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Occupancy Mode Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLcOccupancyModeSet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Occupancy Mode Set"));

    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE mode = ((CComboBox *)GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE))->GetCurSel();

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = mode;

    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcOccupancySet()
{
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE mode = ((CComboBox *)GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE))->GetCurSel();
    USHORT dst = 0, app_key_idx = 0;
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcOnOffGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC OnOff Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLcOnOffSet()
{
    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    DWORD delay = GetDlgItemInt(IDC_DELAY);
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC OnOff Set"));

    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    BYTE onoff = ((CComboBox *)GetDlgItem(IDC_LIGHT_LC_ON_OFF))->GetCurSel();

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = onoff;
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;
    *p++ = delay & 0xff;
    *p++ = (delay >> 8) & 0xff;

    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedLightLcPropertyGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Property Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    USHORT id = lightLcProp[((CComboBox *)GetDlgItem(IDC_LIGHT_LC_PROPERTY))->GetCurSel()].PropId;
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = id & 0xff;
    *p++ = (id >> 8) & 0xff;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedLightLcPropertySet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Send Light LC Property Set"));
    BYTE reliable = (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck();
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    USHORT id = lightLcProp[((CComboBox *)GetDlgItem(IDC_LIGHT_LC_PROPERTY))->GetCurSel()].PropId;
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, reliable, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = id & 0xff;
    *p++ = (id >> 8) & 0xff;
    DWORD value = GetDlgItemInt(IDC_LIGHT_LC_PROPERTY_VALUE, NULL, 0);
    int len = lightLcProp[((CComboBox *)GetDlgItem(IDC_LIGHT_LC_PROPERTY))->GetCurSel()].len;
    if (len == 1)
        *p++ = value & 0xff;
    else if (len == 2)
    {
        *p++ = (value >> 8) & 0xff;
        *p++ = value & 0xff;
    }
    else if (len == 3)
    {
        *p++ = (value >> 16) & 0xff;
        *p++ = (value >> 8) & 0xff;
        *p++ = value & 0xff;
    }
    else if (len == 4)
    {
        *p++ = (value >> 24) & 0xff;
        *p++ = (value >> 16) & 0xff;
        *p++ = (value >> 8) & 0xff;
        *p++ = value & 0xff;
    }
    // p += GetHexValue(IDC_LIGHT_LC_PROPERTY_VALUE, p, sizeof(buffer) - 7);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedVsData()
{
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[400];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 0, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));

    DWORD data_len = GetHexValue(IDC_TC_NET_LEVEL_TRX_PDU, p, (DWORD)(&buffer[sizeof(buffer)] - p));

    USHORT  company_id = 0x131;
    USHORT  model_id = 0x01;
    BYTE    opcode = 0x01;

    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    *p++ = model_id & 0xff;
    *p++ = (model_id >> 8) & 0xff;
    *p++ = opcode & 0xff;

    WCHAR buf[100];
    wsprintf(buf, L"Send VS Data to addr:%x app_key_idx:%x company_id:%04x model:%04x opcode:%d %d bytes:", dst, app_key_idx, company_id, model_id, opcode, data_len);
    m_trace->SetCurSel(m_trace->AddString(buf));
    DWORD len = data_len;
    DWORD i;
    while (len != 0)
    {
        buf[0] = 0;
        for (i = 0; i < len && i < 32; i++)
            wsprintf(&buf[wcslen(buf)], L"%02x ", p[i]);

        len -= i;
        if (len != 0)
            m_trace->SetCurSel(m_trace->AddString(buf));
    }
    m_trace->SetCurSel(m_trace->AddString(buf));
    p += data_len;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_VENDOR_DATA, (BYTE*)&buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTcClearRpl()
{
    m_trace->SetCurSel(m_trace->AddString(L"Transport Layer: Clear Replay Protection List"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_CLEAR_REPLAY_PROT_LIST, NULL, 0);
}


void CClientControlDlg::OnCbnSelchangeTestSelection()
{
    int sel = ((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel();
    int i = (int)((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetItemData(((CComboBox *)GetDlgItem(IDC_TEST_SELECTION))->GetCurSel());
    DisableAll();
    switch (sel)
    {
    case 0:
        EnableOnOff();
        break;

    case 1:
        EnableLevel();
        break;

    case 2:
        EnableDefaultTransitionTime();
        break;

    case 3:
        EnablePowerOnOff();
        break;

    case 4:
        EnablePowerLevel();
        break;

    case 5:
        EnableBattery();
        break;

    case 6:
        EnableLocation();
        break;

    case 7:
        EnableProperty();
        break;

    case 8:
        EnableSensor();
        break;

    case 9:
        EnableTime();
        break;

    case 10:
        EnableScene();
        break;

    case 11:
        EnableScheduler();
        break;

    case 12:
        EnableLightLightness();
        break;

    case 13:
        EnableLightCTL();
        break;

    case 14:
        EnableLightHSL();
        break;

    case 15:
        EnableLightxyL();
        break;

    case 16:
        EnableLightLC();
        break;

    case 17:
        EnableOther();
        break;
    }
}

void CClientControlDlg::EnableOnOff()
{
    EnableDefaultTransitionTime();
    GetDlgItem(IDC_ON_OFF_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_ON_OFF_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_ON_OFF_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
    GetDlgItem(IDC_DELAY)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
}

void CClientControlDlg::EnableLevel()
{
    EnableDefaultTransitionTime();
    GetDlgItem(IDC_LEVEL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LEVEL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LEVEL_DELTA)->EnableWindow(TRUE);
    GetDlgItem(IDC_DELTA_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_DELTA_CONTINUE)->EnableWindow(TRUE);
    GetDlgItem(IDC_MOVE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LEVEL_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
    GetDlgItem(IDC_DELAY)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
}

void CClientControlDlg::EnableDefaultTransitionTime()
{
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_DEFAULT_TRANSITION_TIME)->EnableWindow(TRUE);
}

void CClientControlDlg::EnablePowerOnOff()
{
    EnableOnOff();
    GetDlgItem(IDC_POWER_ON_OFF_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_POWER_ON_OFF_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_POWER_ON_OFF_STATE)->EnableWindow(TRUE);
}

void CClientControlDlg::EnablePowerLevel()
{
    EnableLevel();
    EnablePowerOnOff();
    SetDlgItemText(IDC_STATIC_LIGHTNESS_POWER_LEVEL, L"Power Level");
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LAST_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
    GetDlgItem(IDC_DELAY)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
}

void CClientControlDlg::EnableBattery()
{
    GetDlgItem(IDC_BATTERY_LEVEL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_LEVEL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_LEVEL)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_TO_DISCHARGE)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_TO_CHARGE)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_PRESENSE)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_INDICATOR)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_CHARGING)->EnableWindow(TRUE);
    GetDlgItem(IDC_BATTERY_SERVICABILITY)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableLocation()
{
    GetDlgItem(IDC_LOCATION_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCATION_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCATION_LOCAL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCATION_LOCAL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_GLOBAL_ALTITUDE)->EnableWindow(TRUE);
    GetDlgItem(IDC_GLOBAL_LONGITUDE)->EnableWindow(TRUE);
    GetDlgItem(IDC_GLOBAL_LATITUDE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCAL_NORTH)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCAL_EAST)->EnableWindow(TRUE);
    GetDlgItem(IDC_LOCAL_ALTITUDE)->EnableWindow(TRUE);
    GetDlgItem(IDC_FLOOR)->EnableWindow(TRUE);
    GetDlgItem(IDC_MOBILE)->EnableWindow(TRUE);
    GetDlgItem(IDC_UPDATE_TIME)->EnableWindow(TRUE);
    GetDlgItem(IDC_PRECISION)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableProperty()
{
    GetDlgItem(IDC_PROPERTIES_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_TYPE)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_ID)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_VALUE)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROPERTY_ACCESS)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableSensor()
{
    GetDlgItem(IDC_SENSOR_CL_TRIG_TYPE)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_RAW_VAL_X)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_RAW_VAL_X1)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_RAW_VAL_X2)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_SETTING_VAL)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_FS_CAD_DIV)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_TRIG_DWN)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_TRIG_DEL_UP)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_MIN_INT)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_FST_CAD_HIGH)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_CL_FST_CAD_LOW)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_PROP_ID)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_SETTING_PROP_ID)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_PROP_VAL_LEN)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_MSG)->EnableWindow(TRUE);
    GetDlgItem(IDC_SENSOR_MSG_SEND)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableTime()
{
    GetDlgItem(IDC_SHEDULER_DAY)->EnableWindow(TRUE);
    GetDlgItem(IDC_SHEDULER_TIME)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_SUBSECONDS)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_UNCERTAINTY)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIMEZONE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIMEZONE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_ZONE_OFFSET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_AUTHORITY)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_AUTHORITY_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_AUTHORITY_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TAI_UTC_DELTA_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TAI_UTC_DELTA_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_TAI_UTC_DELTA)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableScene()
{
    GetDlgItem(IDC_SCENE_REGISTER_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCENE_RECALL)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCENE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCENE_STORE)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCENE_DELETE)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCENE_NUMBER)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableScheduler()
{
    GetDlgItem(IDC_SCENE_NUMBER)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCHEDULER_REGISTER_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCHEDULER_ACTION_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCHEDULER_ACTION_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_SHEDULER_DAY)->EnableWindow(TRUE);
    GetDlgItem(IDC_SHEDULER_TIME)->EnableWindow(TRUE);
    GetDlgItem(IDC_SCHEDULAR_ACTION_NUMBER)->EnableWindow(TRUE);
    GetDlgItem(IDC_ACTION)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableLightLightness()
{
    EnablePowerOnOff();
    EnableLevel();
    SetDlgItemText(IDC_STATIC_LIGHTNESS_POWER_LEVEL, L"Lightness");
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LAST_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_LINEAR)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_DEFAULT)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_RANGE_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
    GetDlgItem(IDC_DELAY)->EnableWindow(((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() == 0);
}

void CClientControlDlg::EnableLightCTL()
{
    EnableLightLightness();
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_DELTA_UV_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_DEFAULT_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_DEFAULT_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_CTL_TEMPERATURE_RANGE_SET)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableLightHSL()
{
    EnableLightLightness();
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_VALUE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_VALUE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_HUE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_SATURATION_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_TARGET_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HUE_RANGE_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HUE_RANGE_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_SATURATION_RANGE_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_SATURATION_RANGE_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_DEFAULT_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_DEFAULT_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_RANGE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_HSL_RANGE_SET)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableLightxyL()
{
    EnableLightLightness();
    GetDlgItem(IDC_LIGHT_LIGHTNESS_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_X_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_X_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_Y_MAX)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_Y_MIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_Y_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_X_TARGET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_DEFAULT_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_DEFAULT_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_RANGE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_RANGE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_XYL_TARGET_GET)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableLightLC()
{
    EnableLightLightness();
    GetDlgItem(IDC_LIGHT_LC_MODE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_MODE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_MODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_OCCUPANCY_MODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_ON_OFF)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_VALUE)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_GET)->EnableWindow(TRUE);
    GetDlgItem(IDC_LIGHT_LC_PROPERTY_SET)->EnableWindow(TRUE);
}

void CClientControlDlg::EnableOther()
{
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_CTL)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_TTL)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_PDU)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_NET_LEVEL_TRX_DST)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_TRANSP_LEVEL_SZMIC)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_TRANSP_LEVEL_SEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_IV_UPDATE)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_IV_UPDATE_TRANSIT)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_IV_UPDATE_SET_TEST_MODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_IV_UPDATE_SET_RECOVERY_MODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_LPN_FRND_CLEAR)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_PVNR_ADDR)->EnableWindow(TRUE);
    GetDlgItem(IDC_VS_DATA)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_CLEAR_RPL)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_HEALTH_FAULTS)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_HEALTH_FAULTS_SET)->EnableWindow(TRUE);
    GetDlgItem(IDC_TC_CFG_IDENTITY)->EnableWindow(TRUE);
}


void CClientControlDlg::OnBnClickedUseDefaultTransTimeSend()
{
    BOOL use_default_transition_time = (BYTE)((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck();
    GetDlgItem(IDC_TRANSITION_TIME)->EnableWindow(!use_default_transition_time);
    GetDlgItem(IDC_DELAY)->EnableWindow(!use_default_transition_time);
}


void CClientControlDlg::OnBnClickedTcIvUpdateSetTestMode()
{
    m_trace->SetCurSel(m_trace->AddString(L"IV UPDATE Set Test Mode"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_SET_IV_UPDATE_TEST_MODE, NULL, 0);
}


void CClientControlDlg::OnBnClickedTcIvUpdateSetRecoveryMode()
{
    m_trace->SetCurSel(m_trace->AddString(L"IV UPDATE Set Recovery state"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_SET_IV_RECOVERY_STATE, NULL, 0);
}

void CClientControlDlg::OnBnClickedTcHealthFaultsSet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Health: Set Faults"));
    BYTE    test_id = 0;// (BYTE)GetHexValueInt(IDC_TC_HEALTH_TEST_ID);
    USHORT  company_id = 0x131;// (USHORT)GetHexValueInt(IDC_TC_HEALTH_COMPANY_ID);
    BYTE    buffer[20];
    LPBYTE  p = buffer;
    *p++ = test_id;
    *p++ = company_id & 0xff;
    *p++ = (company_id >> 8) & 0xff;
    p += GetHexValue(IDC_TC_HEALTH_FAULTS, p, 16);
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_HEALTH_SET_FAULTS, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSchedulerRegisterGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Scene Register Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCHEDULER_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSchedulerActionGet()
{
    WCHAR buf[40];
    BYTE index = (BYTE)GetDlgItemInt(IDC_SCHEDULAR_ACTION_NUMBER);
    if (index > 15)
    {
        m_trace->SetCurSel(m_trace->AddString(L"Scheduler entry should be 0-15"));
        return;
    }
    wsprintf(buf, L"Scheduler Action Get:%d", index);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = index;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_GET, buffer, (DWORD)(p - buffer));
}


void CClientControlDlg::OnBnClickedSchedulerActionSet()
{
    WCHAR buf[40];
    UINT16 index = GetDlgItemInt(IDC_SCHEDULAR_ACTION_NUMBER);
    if (index > 15)
    {
        m_trace->SetCurSel(m_trace->AddString(L"Scheduler entry should be 0-15"));
        return;
    }
    wsprintf(buf, L"Scheduler Action Set:%d", index);
    m_trace->SetCurSel(m_trace->AddString(buf));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, (BYTE)((CButton *)GetDlgItem(IDC_RELIABLE_SEND))->GetCheck(), 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));

    *p++ = GetDlgItemInt(IDC_SCHEDULAR_ACTION_NUMBER);

    CTime time;
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_DAY))->GetTime(time);
    if (m_year == 0)
        *p++ = time.GetYear() - 2000;
    else
        *p++ = 0x64;

    USHORT month = 0;
    month = (m_month_selection == 0) ? 1 << (time.GetMonth() - 1) : m_month_selection;

    *p++ = (month & 0xff);
    *p++ = (month >> 8) & 0xff;

    if (m_day == 0)
        *p++ = time.GetDay();
    else
        *p++ = 0;

    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_TIME))->GetTime(time);
    if (m_hour == 0)
        *p++ = time.GetHour();
    else if (m_hour == 1)
        *p++ = 0x18;
    else if (m_hour == 2)
        *p++ = 0x19;

    if (m_minute == 0)
        *p++ = time.GetMinute();
    else if (m_minute == 1)
        *p++ = 0x3C;
    else if (m_minute == 2)
        *p++ = 0x3D;
    else if (m_minute == 3)
        *p++ = 0x3E;
    else if (m_minute == 4)
        *p++ = 0x3F;

    if (m_second == 0)
        *p++ = time.GetSecond();
    else if (m_second == 1)
        *p++ = 0x3C;
    else if (m_second == 2)
        *p++ = 0x3D;
    else if (m_second == 3)
        *p++ = 0x3E;
    else if (m_second == 4)
        *p++ = 0x3F;

    BYTE a = (m_day_of_week_selection == 0) ? world_day_of_week[time.GetDayOfWeek() - 1] : m_day_of_week_selection;
    *p++ = m_day_of_week_selection;

    *p++ = ((CComboBox *)GetDlgItem(IDC_ACTION))->GetCurSel();

    DWORD transition_time = ((CButton *)GetDlgItem(IDC_USE_DEFAULT_TRANS_TIME_SEND))->GetCheck() ? 0xffffffff : GetDlgItemInt(IDC_TRANSITION_TIME);
    *p++ = transition_time & 0xff;
    *p++ = (transition_time >> 8) & 0xff;
    *p++ = (transition_time >> 16) & 0xff;
    *p++ = (transition_time >> 24) & 0xff;

    UINT16 scene = GetDlgItemInt(IDC_SCENE_NUMBER);
    *p++ = scene & 0xff;
    *p++ = (scene >> 8) & 0xff;
#if 0
    BYTE buffer1[100];
    memset(buffer1, 0, 100);
    p = buffer1;

#include <pshpack1.h>
    typedef struct
    {
        BYTE action_number;                          /**< zero based entry number */
        BYTE year;                                   /**< scheduled year for the action, or 0 if action needs to happen every year */
        USHORT month;                                 /**< Bit field of the months for the action */
#define WICED_BT_MESH_SCHEDULER_EVERY_DAY           0x00
        BYTE  day;                                   /**< Scheduled day of the month, or 0 to repeat every day */
#define WICED_BT_MESH_SCHEDULER_EVERY_HOUR          0x18
#define WICED_BT_MESH_SCHEDULER_RANDOM_HOUR         0x19
        BYTE  hour;                                  /**< Scheduled hour for the action */
#define WICED_BT_MESH_SCHEDULER_EVERY_MINUTE        0x3C
#define WICED_BT_MESH_SCHEDULER_EVERY_15_MINUTES    0x3D
#define WICED_BT_MESH_SCHEDULER_EVERY_20_MINUTES    0x3E
#define WICED_BT_MESH_SCHEDULER_RANDOM_MINUTE       0x3F
        BYTE  minute;                                /**< Scheduled hour for the action */
#define WICED_BT_MESH_SCHEDULER_EVERY_SECOND        0x3C
#define WICED_BT_MESH_SCHEDULER_EVERY_15_SECONDS    0x3D
#define WICED_BT_MESH_SCHEDULER_EVERY_20_SECONDS    0x3E
#define WICED_BT_MESH_SCHEDULER_RANDOM_SECOND       0x3F
        BYTE  second;                                /**< Scheduled hour for the action */
#define WICED_BT_MESH_SCHEDULER_ACTION_TURN_OFF     0
#define WICED_BT_MESH_SCHEDULER_ACTION_TURN_ON      1
#define WICED_BT_MESH_SCHEDULER_ACTION_SCENE_RECALL 2
#define WICED_BT_MESH_SCHEDULER_ACTION_TURN_NONE    0x0f
        BYTE  day_of_week;                           /**< Bit field of the days of week when the action should happen */
        BYTE  action;
        ULONG transition_time;                       /**< Transition time to turn on/off or to transition to a scene */
        USHORT scene_number;                          /**< Scene number to transition to */
    } wiced_bt_mesh_scheduler_action_data_t;
#include <poppack.h>

#define UINT32_TO_STREAM(p, u32) {*(p)++ = (UINT8)(u32); *(p)++ = (UINT8)((u32) >> 8); *(p)++ = (UINT8)((u32) >> 16); *(p)++ = (UINT8)((u32) >> 24);}
#define UINT24_TO_STREAM(p, u24) {*(p)++ = (UINT8)(u24); *(p)++ = (UINT8)((u24) >> 8); *(p)++ = (UINT8)((u24) >> 16);}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (UINT8)(u16); *(p)++ = (UINT8)((u16) >> 8);}
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (UINT8)(u8);}

    wiced_bt_mesh_scheduler_action_data_t *p_data = (wiced_bt_mesh_scheduler_action_data_t *)&buffer[5];

    UINT8_TO_STREAM(p, (p_data->action_number & 0x0f) + ((p_data->year & 0x0f) << 4));     // 4 bits of action number and 4 bits of year
    UINT8_TO_STREAM(p, ((p_data->year >> 4) & 0x07) + ((p_data->month & 0x1f) << 3));     // 3 bits of year and 5 bits of month
    UINT8_TO_STREAM(p, ((p_data->month >> 5) & 0x7f) + ((p_data->day & 0x01) << 7));     // 7 bits of month and 1 bit of day
    UINT8_TO_STREAM(p, ((p_data->day >> 1) & 0x0f) + ((p_data->hour & 0x0f) << 4));     // 4 bits of day and 4 bits of hour
    UINT8_TO_STREAM(p, ((p_data->hour >> 4) & 0x01) + (p_data->minute << 1) + ((p_data->second & 0x01) << 7)); // 4 bits of hour and 6 bits of minutes and 1 of second
    UINT8_TO_STREAM(p, ((p_data->second >> 1) & 0x1f) + ((p_data->day_of_week & 0x07) << 5));     // 5 bits of second and 3 bits of day of week
    UINT8_TO_STREAM(p, ((p_data->day_of_week >> 3) & 0x0f) + ((p_data->action & 0x0f) << 4));     // 4 bits of day of week and 4 bits of action
    UINT8_TO_STREAM(p, 0);
    UINT16_TO_STREAM(p, p_data->scene_number);

    wiced_bt_mesh_scheduler_action_data_t status_data;

    status_data.action_number = buffer1[0] & 0x0f;

    status_data.year = ((buffer1[0] >> 4) & 0x0f) + ((buffer1[1] & 0x07) << 4);
    status_data.month = ((buffer1[1] >> 3) & 0x1f) + ((buffer1[2] & 0x7f) << 5);
    status_data.day = ((buffer1[2] >> 7) & 0x01) + ((buffer1[3] & 0x0f) << 1);
    status_data.hour = ((buffer1[3] >> 4) & 0x0f) + ((buffer1[4] & 0x01) << 4);
    status_data.minute = ((buffer1[4] >> 1) & 0x3f);
    status_data.second = ((buffer1[4] >> 7) & 0x01) + ((buffer1[5] & 0x1f) << 1);
    status_data.day_of_week = ((buffer1[5] >> 5) & 0x07) + ((buffer1[6] & 0x0f) << 3);
    status_data.action = ((buffer1[6] >> 4) & 0x0f);
    status_data.transition_time = (buffer1[7]);
    status_data.scene_number = buffer1[8] + (buffer1[9] << 8);
#endif
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedSchedulerAdvanced()
{
    CTime time;
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_DAY))->GetTime(time);

    CSchedulerAdvanced dlg;
    BYTE month = time.GetMonth();
    dlg.m_month_selection = 1 << (time.GetMonth() - 1);

    BYTE day_of_week = time.GetDayOfWeek();
    dlg.m_day_of_week_selection = world_day_of_week[day_of_week - 1];
    dlg.m_year = 0;
    dlg.m_day = 0;
    dlg.m_minute = 0;
    dlg.m_second = 0;
    if (dlg.DoModal() == IDOK)
    {
        m_month_selection = dlg.m_month_selection;
        m_day_of_week_selection = dlg.m_day_of_week_selection;
        m_year = dlg.m_year;
        m_day = dlg.m_day;
        m_hour = dlg.m_hour;
        m_minute = dlg.m_minute;
        m_second = dlg.m_second;
    }
}


void CClientControlDlg::OnBnClickedTimeGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Time Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_GET, buffer, (DWORD)(p - buffer));
}

/*
* helper function that converts local time to tai
*/
ULONGLONG local_to_tai_seconds(int year, int month, int day, int hour, int minute, int second)
{
    ULONGLONG l;
    int  f, d, b, z, v, a, k;
    f = 0;

    // Let E be TAI - UTC Delta Current if T < TAI of Delta Change, and TAI - UTC Delta New if T >= TAI of Delta Change.
    if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
    {
        //it is a leap year
        a = 1;
    }
    else
    {
        a = 2;
    }
    if (month <= 2)
    {
        //January or February
        k = 0;
    }
    else
    {
        k = a;
    }
    z = year - 1;
    v = (int)((day * 12 + (int)((367 * month - 362))) / 12) - k - 1;
    b = v + (int)(z / 400) - (int)(z / 100) + (int)(z / 4) + (365 * z);
    d = b - 730119;
    l = (ULONGLONG)second + (ULONGLONG)d * 86400 + (ULONGLONG)hour * 3600 + (ULONGLONG)minute * 60 + f;

#if 0
    // time zone has to be subracted
    tai_seconds = l - (int)(time_zone_offset_current - 64) * 60;

    //subract TAI UTC Delta Current or TAI UTC Delta New
    // TO DO : check this logic
    if (mesh_time->utc_delta.tai_of_delta_change == 0)
    {
        tai_seconds = tai_seconds - mesh_time->utc_delta.tai_utc_delta_current;
    }
    else
    {
        if (tai_seconds < mesh_time->utc_delta.tai_of_delta_change)
            tai_seconds = tai_seconds - mesh_time->utc_delta.tai_utc_delta_current;

        if (tai_seconds >= mesh_time->utc_delta.tai_of_delta_change)
            tai_seconds = tai_seconds - mesh_time->utc_delta.tai_utc_delta_new;
    }
#endif

    return l;
}


void CClientControlDlg::OnBnClickedTimeSet()
{
    CTime time_day, time_seconds;
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_DAY))->GetTime(time_day);
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_TIME))->GetTime(time_seconds);
    int time_zone_offset_current = GetDlgItemInt(IDC_TIME_ZONE_OFFSET, NULL, 1);

    ULONGLONG tai_time = local_to_tai_seconds(time_day.GetYear(), time_day.GetMonth(), time_day.GetDay(), time_seconds.GetHour(), time_seconds.GetMinute(), time_seconds.GetSecond());
    int year, month, day, hour, minute, second;

    tai_to_utc_local_time(tai_time, &year, &month, &day, &hour, &minute, &second);

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton*)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = tai_time & 0xff;
    *p++ = (tai_time >> 8) & 0xff;
    *p++ = (tai_time >> 16) & 0xff;
    *p++ = (tai_time >> 24) & 0xff;
    *p++ = (tai_time >> 32) & 0xff;
    *p++ = GetDlgItemInt(IDC_TIME_SUBSECONDS) & 0xff;
    *p++ = GetDlgItemInt(IDC_TIME_UNCERTAINTY) & 0xff;
    *p++ = ((CComboBox *)GetDlgItem(IDC_TIME_AUTHORITY))->GetCurSel();
    *p++ = (USHORT)GetDlgItemInt(IDC_TIME_TAI_UTC_DELTA, NULL, FALSE) & 0xff;
    *p++ = ((USHORT)GetDlgItemInt(IDC_TIME_TAI_UTC_DELTA, NULL, FALSE) >> 8) & 0xff;
    *p++ = GetDlgItemInt(IDC_TIME_ZONE_OFFSET) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Time Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTimezoneGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Time Zone Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_ZONE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTimezoneSet()
{
    CTime time_day, time_seconds;
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_DAY))->GetTime(time_day);
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_TIME))->GetTime(time_seconds);
    int time_zone_offset_current = GetDlgItemInt(IDC_TIME_ZONE_OFFSET, NULL, 1);
    ULONGLONG tai_time = local_to_tai_seconds(time_day.GetYear(), time_day.GetMonth(), time_day.GetDay(), time_seconds.GetHour(), time_seconds.GetMinute(), time_seconds.GetSecond());

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton*)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = time_zone_offset_current & 0xff;
    *p++ = tai_time & 0xff;
    *p++ = (tai_time >> 8) & 0xff;
    *p++ = (tai_time >> 16) & 0xff;
    *p++ = (tai_time >> 24) & 0xff;
    *p++ = (tai_time >> 32) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Time Zone Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_ZONE_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTaiUtcDeltaGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Time TAI-UTC Delta Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTaiUtcDeltaSet()
{
    CTime time_day, time_seconds;
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_DAY))->GetTime(time_day);
    ((CDateTimeCtrl *)GetDlgItem(IDC_SHEDULER_TIME))->GetTime(time_seconds);
    USHORT delta = GetDlgItemInt(IDC_TIME_TAI_UTC_DELTA);
    ULONGLONG tai_time = local_to_tai_seconds(time_day.GetYear(), time_day.GetMonth(), time_day.GetDay(), time_seconds.GetHour(), time_seconds.GetMinute(), time_seconds.GetSecond());

    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton*)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = delta & 0xff;
    *p++ = (delta >> 8) & 0xff;
    *p++ = tai_time & 0xff;
    *p++ = (tai_time >> 8) & 0xff;
    *p++ = (tai_time >> 16) & 0xff;
    *p++ = (tai_time >> 24) & 0xff;
    *p++ = (tai_time >> 32) & 0xff;

    m_trace->SetCurSel(m_trace->AddString(L"Send Time UTC Delta Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_SET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTimeAuthorityGet()
{
    m_trace->SetCurSel(m_trace->AddString(L"Time Authority Get"));
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_ROLE_GET, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTimeAuthoritySet()
{
    USHORT dst = 0, app_key_idx = 0;
    if (!((CButton*)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
    {
        dst = (USHORT)GetHexValueInt(IDC_DST);
        app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    }
    BYTE buffer[128];
    LPBYTE p = wiced_bt_mesh_format_hci_header(dst, app_key_idx, 0, 1, 0, USE_CONFIGURED_DEFAULT_TTL, 0, 0, 0, buffer, sizeof(buffer));
    *p++ = ((CComboBox *)GetDlgItem(IDC_TIME_AUTHORITY))->GetCurSel();

    m_trace->SetCurSel(m_trace->AddString(L"Send Time Authority Set"));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_TIME_ROLE_SET, buffer, (DWORD)(p - buffer));
}

#if 1
/*
* helper function that computes the local time
* time zone is already taken care by this function
*/
void tai_to_utc_local_time(ULONGLONG tai_seconds, int *year, int *month, int *day, int *hour, int *minute, int *second)
{
    ULONGLONG l;
    int  e, f, d, h, s, m, b, q, c, x, z, v, a, j, k;
    int H;
    int tai_of_delta_change = 0;
    int tai_utc_delta_current = 0;
    int tai_utc_delta_new = 0;
    int time_zone_offset_current = 0;

    if (tai_of_delta_change == 0)
    {
        e = (int)(tai_utc_delta_current - 255);
        f = 0;
    }
    else
    {
        if (tai_seconds < tai_of_delta_change)
            e = (int)(tai_utc_delta_current - 255);
        if (tai_seconds >= tai_of_delta_change)
            e = (int)(tai_of_delta_change - 255);
        if ((tai_seconds + 1 == tai_of_delta_change) &&
            (tai_utc_delta_current < tai_utc_delta_new))
            f = 1;
        else
            f = 0;
    }

    l = tai_seconds - e - f;
    // add the time zone offset
    l = l + ((ULONGLONG)time_zone_offset_current - 64) * 60;
    d = (int)(l / 86400);
    h = (int)((l - (ULONGLONG)d * 86400) / 3600);
    m = (int)((l - (ULONGLONG)d * 86400 - (ULONGLONG)h * 3600) / 60);
    s = (uint32_t)(l - (ULONGLONG)d * 86400 - (ULONGLONG)h * 3600 - (ULONGLONG)m * 60 + f);
    *hour = h;
    *minute = m;
    *second = s;
    // The time of day is H:M:S and D is the number of days since 2000-01-01. Note that F will only equal 1 at a positive leap second; if F = 1 and S is not 60, the value of TAI of Delta Change is wrong.
    // Converting D to a date is then done as follows.
    b = d + 730119;
    q = b % 146097;
    c = (int)(q / 36524);
    H = q % 36524;
    x = (int)((H % 1461) / 365);
    * year = ((int)(b / 146097) * 400 + c * 100 + (int)(H / 1461) * 4 + x + (!((c == 4) || (x == 4)) ? 1 : 0));
    z = *year - 1;
    v = b - 365 * z - (int)(z / 4) + (int)(z / 100) - (int)(z / 400);

    if ((*year % 4 == 0) && ((*year % 100 != 0) || (*year % 400 == 0)))
    {
        //it is a leap year
        a = 1;
    }
    else
    {
        a = 2;
    }

    if ((v + a) < 61)
    {
        j = 0;
    }
    else
    {
        j = a;
    }

    *month = (int)(((v + j) * 12 + 373) / 367);

    if (*month <= 2)
    {
        //January or February
        k = 0;
    }
    else
    {
        k = a;
    }
    *day = v + k + 1 - (int)((367 * *month - 362) / 12);
}
#endif

void CClientControlDlg::OnBnClickedTcCfgIdentity()
{
    m_trace->SetCurSel(m_trace->AddString(L"Config: Begin advert Node Identity"));
    // Set node identity for all networks for 60 seconds
    // params:=<action><type>; <action>=1 means set; <type>=1 means NODE IDENTITY USER
    BYTE params[] = { 1, 1 };
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_CFG_ADV_IDENTITY, params, (DWORD)sizeof(params));
}


void CClientControlDlg::OnBnClickedTcAccessPdu()
{
    m_trace->SetCurSel(m_trace->AddString(L"Access PDU:"));
    BYTE    ttl = (BYTE)GetHexValueInt(IDC_TC_NET_LEVEL_TRX_TTL);
    USHORT  app_key_idx = (USHORT)GetHexValueInt(IDC_APP_KEY_IDX);
    USHORT  src = (USHORT)GetHexValueInt(IDC_TC_PVNR_ADDR);
    USHORT  dst = (USHORT)GetHexValueInt(IDC_TC_NET_LEVEL_TRX_DST);
    BYTE    buffer[128];
    LPBYTE  p = buffer;
    *p++ = ttl;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    *p++ = 0;  // element_idx
    *p++ = src & 0xff;
    *p++ = (src >> 8) & 0xff;
    *p++ = dst & 0xff;
    *p++ = (dst >> 8) & 0xff;
    p += GetHexValue(IDC_TC_NET_LEVEL_TRX_PDU, p, (DWORD)(&buffer[sizeof(buffer)] - p));
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_ACCESS_PDU, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTcLpnSendSubsUpdt(BOOL add)
{
    m_trace->SetCurSel(m_trace->AddString(L"LPN: Send Subscription Add or Remove"));
    USHORT  addr = (USHORT)GetHexValueInt(IDC_TC_PVNR_ADDR);
    BYTE    buffer[128];
    LPBYTE  p = buffer;
    *p++ = add ? 1 : 0;
    *p++ = addr & 0xff;
    *p++ = (addr >> 8) & 0xff;
    m_ComHelper->SendWicedCommand(HCI_CONTROL_MESH_COMMAND_CORE_SEND_SUBS_UPDT, buffer, (DWORD)(p - buffer));
}

void CClientControlDlg::OnBnClickedTcLpnSendSubsAdd()
{
    OnBnClickedTcLpnSendSubsUpdt(TRUE);
}

void CClientControlDlg::OnBnClickedTcLpnSendSubsDel()
{
    OnBnClickedTcLpnSendSubsUpdt(FALSE);
}

USHORT CClientControlDlg::GetDst()
{
    USHORT dst = 0;
    if (!((CButton *)GetDlgItem(IDC_USE_PUBLICATION_INFO))->GetCheck())
        dst = (USHORT)GetHexValueInt(IDC_DST);
    return dst;
}

BOOL CClientControlDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_trace = (CListBox *)GetDlgItem(IDC_TRACE);

    m_trace->AddString(L"NOTE:");
    m_trace->AddString(L"Use Baud rate of 3000000 for CYW920819EVB-02 board and 115200 for CYBT-213043-MESH board.");

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
