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
* SensorConfig.cpp : implementation file
*
*/

#include "stdafx.h"
#include "afxdialogex.h"
#include <setupapi.h>
#include "SensorConfig.h"
#include "wiced_mesh_client.h"

extern "C" CRITICAL_SECTION cs;

// CSensorConfig dialog
CSensorConfig::CSensorConfig(CWnd* pParent /*=NULL*/)
    : CDialogEx(CSensorConfig::IDD, pParent)
{
}

CSensorConfig::~CSensorConfig()
{
}

BOOL CSensorConfig::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CComboBox *p_configure_publish_to = (CComboBox *)GetDlgItem(IDC_PUBLISH_TO);
    p_configure_publish_to->ResetContent();
    p_configure_publish_to->AddString(L"none");
    p_configure_publish_to->AddString(L"all-nodes");
    p_configure_publish_to->AddString(L"all-proxies");
    p_configure_publish_to->AddString(L"all_friends");
    p_configure_publish_to->AddString(L"all-relays");
    p_configure_publish_to->SetCurSel(0);

    SetDlgItemText(IDC_PERIOD, L"10000");

    WCHAR szName[80] = { 0 };
    char *p;
    char *p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        MultiByteToWideChar(CP_UTF8, 0, p, -1, szName, sizeof(szName) / sizeof(WCHAR));
        p_configure_publish_to->AddString(szName);
    }
    free(p_groups);

    ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(0);
    ((CComboBox *)GetDlgItem(IDC_INSIDE_OUTSIDE))->SetCurSel(0);
    ((CComboBox *)GetDlgItem(IDC_INSIDE_OUTSIDE))->SetCurSel(0);

    const char *name = mesh_client_get_publication_target(component_name, FALSE, "SENSOR");
    int period = mesh_client_get_publication_period(component_name, FALSE, "SENSOR");

    WCHAR szSelName[80] = { 0 };
    MultiByteToWideChar(CP_UTF8, 0, name, -1, szSelName, sizeof(szSelName) / sizeof(WCHAR));
    for (int sel = 0; sel < p_configure_publish_to->GetCount(); sel++)
    {
        p_configure_publish_to->GetLBText(sel, szName);
        if (wcscmp(szSelName, szName) == 0)
        {
            p_configure_publish_to->SetCurSel(sel);
            break;
        }
    }
    if (period == 0)
    {
        ((CButton *)GetDlgItem(IDC_PUBLISH_PERIODICALLY))->SetCheck(0);
        ((CButton *)GetDlgItem(IDC_PUBLISH_FASTER))->SetCheck(0);
    }
    else
    {
        ((CButton *)GetDlgItem(IDC_PUBLISH_PERIODICALLY))->SetCheck(1);
        SetDlgItemInt(IDC_PERIOD, period, FALSE);
    }

    uint16_t fast_cadence_period_divisor;
    wiced_bool_t trigger_type;
    uint32_t trigger_delta_down, trigger_delta_up;
    uint32_t min_interval, fast_cadence_low, fast_cadence_high;

    if (mesh_client_sensor_cadence_get(component_name, property_id, &fast_cadence_period_divisor, &trigger_type,
        &trigger_delta_down, &trigger_delta_up, &min_interval, &fast_cadence_low, &fast_cadence_high) != MESH_CLIENT_SUCCESS)
    {
        ((CButton *)GetDlgItem(IDC_PUBLISH_FASTER))->SetCheck(FALSE);
        ((CButton *)GetDlgItem(IDC_TRIGGER_PUB))->SetCheck(FALSE);
    }
    else
    {
        if (fast_cadence_period_divisor > 1)
        {
            ((CButton *)GetDlgItem(IDC_PUBLISH_FASTER))->SetCheck(TRUE);
            if (fast_cadence_period_divisor < 4)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(0);
            else if (fast_cadence_period_divisor < 8)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(1);
            else if (fast_cadence_period_divisor < 16)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(2);
            else if (fast_cadence_period_divisor < 32)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(3);
            else if (fast_cadence_period_divisor < 64)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(4);
            else if (fast_cadence_period_divisor < 128)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(5);
            else if (fast_cadence_period_divisor < 256)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(6);
            else if (fast_cadence_period_divisor < 512)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(7);
            else if (fast_cadence_period_divisor < 1024)
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(8);
            else
                ((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->SetCurSel(9);

            if (fast_cadence_low > fast_cadence_high)
            {
                ((CComboBox *)GetDlgItem(IDC_INSIDE_OUTSIDE))->SetCurSel(1);
                SetDlgItemInt(IDC_FAST_CADENCE_LOW, fast_cadence_high);
                SetDlgItemInt(IDC_FAST_CADENCE_HIGH, fast_cadence_low);
            }
            else
            {
                ((CComboBox *)GetDlgItem(IDC_INSIDE_OUTSIDE))->SetCurSel(0);
                SetDlgItemInt(IDC_FAST_CADENCE_LOW, fast_cadence_low);
                SetDlgItemInt(IDC_FAST_CADENCE_HIGH, fast_cadence_high);
            }
        }
        if ((trigger_delta_down != 0) || (trigger_delta_up != 0))
        {
            ((CButton *)GetDlgItem(IDC_TRIGGER_PUB))->SetCheck(TRUE);
            SetDlgItemInt(IDC_TRIGGER_UP, trigger_delta_up, FALSE);
            SetDlgItemInt(IDC_TRIGGER_DOWN, trigger_delta_down, FALSE);
        }
        ((CComboBox *)GetDlgItem(IDC_TRIGGER_TYPE))->SetCurSel(trigger_type);
        SetDlgItemInt(IDC_MIN_INTERVAL, min_interval / 1000, FALSE);
    }
    DisplayControls();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSensorConfig::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSensorConfig, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()

    ON_BN_CLICKED(IDC_PUBLISH_PERIODICALLY, &CSensorConfig::OnBnClickedPublishPeriodically)
    ON_BN_CLICKED(IDC_PUBLISH_FASTER, &CSensorConfig::OnBnClickedPublishFaster)
    ON_BN_CLICKED(IDC_PUBLISH_PERIODICALLY3, &CSensorConfig::OnBnClickedPublishPeriodically3)
    ON_CBN_SELCHANGE(IDC_PUBLISH_TO, &CSensorConfig::OnCbnSelchangePublishTo)
    ON_BN_CLICKED(IDOK, &CSensorConfig::OnBnClickedOk)
    ON_BN_CLICKED(IDC_PUBLICATION, &CSensorConfig::OnBnClickedPublication)
    ON_BN_CLICKED(IDC_CADENCE, &CSensorConfig::OnBnClickedCadence)
END_MESSAGE_MAP()

void CSensorConfig::OnClose()
{
    CDialogEx::OnClose();
}

void CSensorConfig::OnCancel()
{
    CDialogEx::OnCancel();
}

void CSensorConfig::DisplayControls()
{
    int sel = ((CComboBox *)GetDlgItem(IDC_PUBLISH_TO))->GetCurSel();
    BOOL bSendPeriodic = ((CButton *)GetDlgItem(IDC_PUBLISH_PERIODICALLY))->GetCheck();
    BOOL bFastCadence = ((CButton *)GetDlgItem(IDC_PUBLISH_FASTER))->GetCheck();
    BOOL bTriggerPub = ((CButton *)GetDlgItem(IDC_TRIGGER_PUB))->GetCheck();

    GetDlgItem(IDC_PUBLISH_PERIODICALLY)->EnableWindow(sel != 0);
    GetDlgItem(IDC_CADENCE)->EnableWindow(sel != 0);
    GetDlgItem(IDC_PERIOD)->EnableWindow((sel != 0) && bSendPeriodic);
    GetDlgItem(IDC_PUBLISH_FASTER)->EnableWindow((sel != 0) && bSendPeriodic);
    GetDlgItem(IDC_FAST_CADENCE_LOW)->EnableWindow((sel != 0) && bSendPeriodic && bFastCadence);
    GetDlgItem(IDC_FAST_CADENCE_HIGH)->EnableWindow((sel != 0) && bSendPeriodic && bFastCadence);
    GetDlgItem(IDC_TRIGGER_PUB)->EnableWindow((sel != 0));
    GetDlgItem(IDC_TRIGGER_UP)->EnableWindow((sel != 0) && bTriggerPub);
    GetDlgItem(IDC_TRIGGER_DOWN)->EnableWindow((sel != 0) && bTriggerPub);
}

void CSensorConfig::OnBnClickedPublishPeriodically()
{
    DisplayControls();
}

void CSensorConfig::OnBnClickedPublishFaster()
{
    DisplayControls();
}

void CSensorConfig::OnBnClickedPublishPeriodically3()
{
    DisplayControls();
}

void CSensorConfig::OnCbnSelchangePublishTo()
{
    DisplayControls();
}

extern "C" int mesh_client_sensor_cadence_get(const char *device_name, int property_id,
    uint16_t *fast_cadence_period_divisor, wiced_bool_t *trigger_type,
    uint32_t *trigger_delta_down, uint32_t *trigger_delta_up,
    uint32_t *min_interval, uint32_t *fast_cadence_low,
    uint32_t *fast_cadence_high);

void CSensorConfig::OnBnClickedOk()
{
   CDialogEx::OnOK();
}


void CSensorConfig::OnBnClickedPublication()
{
    BOOL bSendPeriodic = ((CButton *)GetDlgItem(IDC_PUBLISH_PERIODICALLY))->GetCheck();

    const char *publish_to_name = mesh_client_get_publication_target(component_name, FALSE, "SENSOR");
    int current_period = mesh_client_get_publication_period(component_name, FALSE, "SENSOR");

    int new_period = 0;
    if (bSendPeriodic)
        new_period = GetDlgItemInt(IDC_PERIOD);

    CComboBox *p_configure_publish_to = (CComboBox *)GetDlgItem(IDC_PUBLISH_TO);
    WCHAR szName[80];
    p_configure_publish_to->GetLBText(p_configure_publish_to->GetCurSel(), szName);
    char new_name[80] = { 0 };
    WideCharToMultiByte(CP_UTF8, 0, szName, -1, new_name, 79, 0, FALSE);

    if ((current_period != new_period) || (strcmp(publish_to_name, new_name) != 0))
    {
        EnterCriticalSection(&cs);
        mesh_client_configure_publication(component_name, 0, "SENSOR", new_name, new_period);
        LeaveCriticalSection(&cs);
    }
}


void CSensorConfig::OnBnClickedCadence()
{
    BOOL bFastCadence = ((CButton *)GetDlgItem(IDC_PUBLISH_FASTER))->GetCheck();
    BOOL bTriggerPub = ((CButton *)GetDlgItem(IDC_TRIGGER_PUB))->GetCheck();

    const char *publish_to_name = mesh_client_get_publication_target(component_name, FALSE, "SENSOR");
    int current_period = mesh_client_get_publication_period(component_name, FALSE, "SENSOR");

    CComboBox *p_configure_publish_to = (CComboBox *)GetDlgItem(IDC_PUBLISH_TO);
    WCHAR szName[80];
    p_configure_publish_to->GetLBText(p_configure_publish_to->GetCurSel(), szName);
    char new_name[80] = { 0 };
    WideCharToMultiByte(CP_UTF8, 0, szName, -1, new_name, 79, 0, FALSE);

    int new_fast_cadence_period_divisor = 1;
    uint32_t new_fast_cadence_low = 0;
    uint32_t new_fast_cadence_high = 0;

    if (bFastCadence)
    {
        new_fast_cadence_period_divisor = 1 << (((CComboBox *)GetDlgItem(IDC_FAST_CADENCE_DIVISOR))->GetCurSel() + 1);
        int low = GetDlgItemInt(IDC_FAST_CADENCE_LOW, 0, FALSE);
        int high = GetDlgItemInt(IDC_FAST_CADENCE_HIGH, 0, FALSE);
        BOOL bInside = ((CComboBox *)GetDlgItem(IDC_INSIDE_OUTSIDE))->GetCurSel() == 1;
        new_fast_cadence_low = bInside ? low : high;
        new_fast_cadence_high = bInside ? high : low;
    }

    uint32_t new_trigger_delta_up = 0;
    uint32_t new_trigger_delta_down = 0;
    uint8_t  new_trigger_type = 0;

    if (bTriggerPub)
    {
        new_trigger_delta_up = GetDlgItemInt(IDC_TRIGGER_UP, 0, FALSE);
        new_trigger_delta_down = GetDlgItemInt(IDC_TRIGGER_DOWN, 0, FALSE);
    }
    new_trigger_type = ((CComboBox *)GetDlgItem(IDC_TRIGGER_TYPE))->GetCurSel() == 1;

    uint32_t min_interval = GetDlgItemInt(IDC_MIN_INTERVAL, 0, FALSE) * 1000;
    EnterCriticalSection(&cs);
    mesh_client_sensor_cadence_set(component_name, property_id, new_fast_cadence_period_divisor, new_trigger_type,
        new_trigger_delta_down, new_trigger_delta_up, min_interval, new_fast_cadence_low, new_fast_cadence_high);
    LeaveCriticalSection(&cs);
}
