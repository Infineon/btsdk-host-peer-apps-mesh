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
// CLightLcConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MeshClient.h"
#include "LightLcConfig.h"
#include "afxdialogex.h"
#include "wiced_mesh_client.h"

extern "C" CRITICAL_SECTION cs;
extern DWORD GetHexValue(char* szbuf, LPBYTE buf, DWORD buf_size);

CLightLcConfig* pDlg = NULL;

typedef struct
{
    WCHAR* PropName;
    USHORT PropId;
    int len;
} LightLcProp;

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


// CLightLcConfig dialog

IMPLEMENT_DYNAMIC(CLightLcConfig, CDialogEx)

CLightLcConfig::CLightLcConfig(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LC_CONFIG, pParent)
{
    pDlg = this;
}

CLightLcConfig::~CLightLcConfig()
{
    pDlg = NULL;
}

BOOL CLightLcConfig::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CComboBox* pLightLcProp = (CComboBox*)GetDlgItem(IDC_LIGHT_LC_PROPERTY);
    pLightLcProp->ResetContent();
    for (int i = 0; i < sizeof(lightLcProp) / sizeof(lightLcProp[0]); i++)
        pLightLcProp->AddString(lightLcProp[i].PropName);
    pLightLcProp->SetCurSel(0);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightLcConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLightLcConfig, CDialogEx)
    ON_BN_CLICKED(IDC_LIGHT_LC_PROPERTY_GET, &CLightLcConfig::OnBnClickedLightLcPropertyGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_PROPERTY_SET, &CLightLcConfig::OnBnClickedLightLcPropertySet)
    ON_BN_CLICKED(IDC_LIGHT_LC_MODE_GET, &CLightLcConfig::OnBnClickedLightLcModeGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_MODE_GET, &CLightLcConfig::OnBnClickedLightLcOccupancyModeGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_ON_OFF_GET, &CLightLcConfig::OnBnClickedLightLcOnOffGet)
    ON_BN_CLICKED(IDC_LIGHT_LC_MODE_SET_ON, &CLightLcConfig::OnBnClickedLightLcModeSetOn)
    ON_BN_CLICKED(IDC_LIGHT_LC_MODE_SET_OFF, &CLightLcConfig::OnBnClickedLightLcModeSetOff)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_MODE_SET_ON, &CLightLcConfig::OnBnClickedLightLcOccupancyModeSetOn)
    ON_BN_CLICKED(IDC_LIGHT_LC_OCCUPANCY_MODE_SET_OFF, &CLightLcConfig::OnBnClickedLightLcOccupancyModeSetOff)
    ON_BN_CLICKED(IDC_LIGHT_LC_SET_ON, &CLightLcConfig::OnBnClickedLightLcSetOn)
    ON_BN_CLICKED(IDC_LIGHT_LC_SET_OFF, &CLightLcConfig::OnBnClickedLightLcSetOff)
END_MESSAGE_MAP()


// CLightLcConfig message handlers
void property_status_callback(const char* device_name, int property_id, int value)
{
    if (pDlg != NULL)
        pDlg->PropertyStatus(property_id, value);
}

void CLightLcConfig::PropertyStatus(int property_id, int value)
{
    CComboBox* pLightLcProp = (CComboBox*)GetDlgItem(IDC_LIGHT_LC_PROPERTY);
    for (int i = 0; i < sizeof(lightLcProp) / sizeof(lightLcProp[0]); i++)
    {
        if (lightLcProp[i].PropId == property_id)
        {
            pLightLcProp->SetCurSel(i);
            SetDlgItemInt(IDC_LIGHT_LC_PROPERTY_VALUE, value, 0);
        }
    }
}

void CLightLcConfig::OnBnClickedLightLcPropertyGet()
{
    CComboBox* pLightLcProp = (CComboBox*)GetDlgItem(IDC_LIGHT_LC_PROPERTY);
    int sel = pLightLcProp->GetCurSel();
    if (sel < 0)
        return;
    int property_id = lightLcProp[sel].PropId;
    EnterCriticalSection(&cs);
    mesh_client_light_lc_property_get(component_name, property_id, &property_status_callback);
    LeaveCriticalSection(&cs);
}


void CLightLcConfig::OnBnClickedLightLcPropertySet()
{
    CComboBox* pLightLcProp = (CComboBox*)GetDlgItem(IDC_LIGHT_LC_PROPERTY);
    int sel = pLightLcProp->GetCurSel();
    if (sel < 0)
        return;
    int property_id = lightLcProp[sel].PropId;
    int value = GetDlgItemInt(IDC_LIGHT_LC_PROPERTY_VALUE, 0, 0);
    EnterCriticalSection(&cs);
    mesh_client_light_lc_property_set(component_name, property_id, value, &property_status_callback);
    LeaveCriticalSection(&cs);
}

void lc_mode_status_callback(const char* device_name, int mode)
{
}

void CLightLcConfig::OnBnClickedLightLcModeGet()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_mode_get(component_name, &lc_mode_status_callback);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcModeSetOn()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_mode_set(component_name, 1, &lc_mode_status_callback);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcModeSetOff()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_mode_set(component_name, 0, &lc_mode_status_callback);
    LeaveCriticalSection(&cs);
}

void lc_occupancy_mode_status_callback(const char* device_name, int mode)
{
}

void CLightLcConfig::OnBnClickedLightLcOccupancyModeGet()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_occupancy_mode_get(component_name, &lc_occupancy_mode_status_callback);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcOccupancyModeSetOn()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_occupancy_mode_set(component_name, 1, &lc_occupancy_mode_status_callback);
    LeaveCriticalSection(&cs);
}


void CLightLcConfig::OnBnClickedLightLcOccupancyModeSetOff()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_occupancy_mode_set(component_name, 0, &lc_occupancy_mode_status_callback);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcOnOffGet()
{
    EnterCriticalSection(&cs);
    mesh_client_on_off_get(component_name);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcSetOn()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_on_off_set(component_name, 1, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
    LeaveCriticalSection(&cs);
}

void CLightLcConfig::OnBnClickedLightLcSetOff()
{
    EnterCriticalSection(&cs);
    mesh_client_light_lc_on_off_set(component_name, 0, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
    LeaveCriticalSection(&cs);
}
