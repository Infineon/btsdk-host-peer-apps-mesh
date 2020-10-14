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
// SchedulerAdvanced.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerAdvanced.h"
#include "afxdialogex.h"
#include "resource.h"

// CSchedulerAdvanced dialog

IMPLEMENT_DYNAMIC(CSchedulerAdvanced, CDialogEx)

CSchedulerAdvanced::CSchedulerAdvanced(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCHEDULER_ADVANCED, pParent)
{
}

CSchedulerAdvanced::~CSchedulerAdvanced()
{
}

BOOL CSchedulerAdvanced::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    ((CComboBox*)GetDlgItem(IDC_SCHEDULER_YEAR))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_SCHEDULER_DAY))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_SCHEDULER_HOUR))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_SCHEDULER_MINUTE))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_SCHEDULER_SECOND))->SetCurSel(0);

    for (int i = 0; i < 7; i++)
    {
        if (m_day_of_week_selection & (1 << i))
            ((CButton *)GetDlgItem(IDC_MONDAY + i))->SetCheck(1);
    }
    for (int i = 0; i < 12; i++)
    {
        if (m_month_selection & (1 << i))
            ((CButton *)GetDlgItem(IDC_JANUARY + i))->SetCheck(1);
    }
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CSchedulerAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSchedulerAdvanced, CDialogEx)
    ON_BN_CLICKED(IDOK, &CSchedulerAdvanced::OnBnClickedOk)
END_MESSAGE_MAP()


// CSchedulerAdvanced message handlers


void CSchedulerAdvanced::OnBnClickedOk()
{
    for (int i = 0; i < 7; i++)
    {
        if (((CButton *)GetDlgItem(IDC_MONDAY + i))->GetCheck())
            m_day_of_week_selection |= (1 << i);
    }
    for (int i = 0; i < 12; i++)
    {
        if (((CButton *)GetDlgItem(IDC_JANUARY + i))->GetCheck())
            m_month_selection |= (1 << i);
    }
    m_year = ((CComboBox*)GetDlgItem(IDC_SCHEDULER_YEAR))->GetCurSel();
    m_day = ((CComboBox*)GetDlgItem(IDC_SCHEDULER_DAY))->GetCurSel();
    m_hour = ((CComboBox*)GetDlgItem(IDC_SCHEDULER_HOUR))->GetCurSel();
    m_minute = ((CComboBox*)GetDlgItem(IDC_SCHEDULER_MINUTE))->GetCurSel();
    m_second = ((CComboBox*)GetDlgItem(IDC_SCHEDULER_SECOND))->GetCurSel();
    CDialogEx::OnOK();
}
