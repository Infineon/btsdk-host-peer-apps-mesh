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
#pragma once

#include "ClientControl.h"
#include "ControlComm.h"

#define IDT_SEND_VENDOR_DATA 2
#define SEND_VENDOR_DATA_INTERVAL 4000

// CMeshPerformance dialog

class CMeshPerformance : public CPropertyPage
{
    DECLARE_DYNAMIC(CMeshPerformance)

public:
    CMeshPerformance();
    virtual ~CMeshPerformance();
    void OnCancel();
    void SetDlgItemHex(DWORD id, DWORD val);
    DWORD GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size);
    DWORD GetHexValueInt(DWORD id);

    CListBox* m_trace;

    BOOL m_bConnected;
    BOOL m_bPerfTestRunning;
    BOOL m_bLocalDisableNtwkRetransmit;

    void DisplayCurrentGroup();

    void ProcessVendorSpecificData(LPBYTE p_data, DWORD len);
    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MESH_PERF };
#endif


protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnSetActive();
    afx_msg void OnClose();
    afx_msg void OnBnClickedClearTrace();

    void SendVendorData();
    void OnBnClickedVsData();

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedVsPerfTest();
    afx_msg void OnBnClickedVsPerfTestStop();

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    LRESULT OnAddVendorModel(WPARAM wparam, LPARAM lparam);

    int m_timer_count;
    int m_max_iterations;
    int m_packet_len;
    afx_msg void OnBnClickedMeshSetAdvTxPower();
    afx_msg void OnCbnSelchangePerfControlDevice();
    afx_msg void OnBnClickedVsPerfShowResults();

    void SyncControlDevices();

    void InitTestResults(int test_id);
    afx_msg void OnBnClickedMeshSetNtwkRetransmit();
    afx_msg void OnBnClickedMeshSetAdvTxPowerAll();
    afx_msg void OnBnClickedMeshSetNtwkRetransmitAll();
};

extern CClientControlApp theApp;
