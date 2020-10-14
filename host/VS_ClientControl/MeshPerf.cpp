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
// MeshPerf.cpp : implementation file
//

#include "stdafx.h"
#include "LightControl.h"
#include "ClientControlDlg.h"
#include "SensorConfig.h"
#include "LightLcConfig.h"
#include "afxdialogex.h"
#include "resource.h"
#include "wiced_mesh_client.h"
#include "hci_control_api.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_provision.h"
#include "wiced_bt_mesh_db.h"
#include "MeshPerformance.h"

ClsStopWatch thesw;

extern BOOL SendMessageToUDPServer(char* p_msg, UINT len);
extern void TraceHciPkt(BYTE type, BYTE* buffer, USHORT length);
extern void Log(WCHAR* fmt, ...);
extern "C" void wiced_hci_process_data(uint16_t opcode, uint8_t * p_buffer, uint16_t len);
extern DWORD GetHexValue(char* szbuf, LPBYTE buf, DWORD buf_size);

#define MESH_PERF_MAX_ITERATIONS 10

typedef struct
{
    char device_name[80];
    uint16_t company_id;
    uint16_t model_id;
    uint8_t opcode;
    uint8_t tx_hops; ///< Number of hops to reach destination
    uint8_t rx_hops; ///< Number of hops from destination
    int elapsed_time; ///< Total roundtrip time in milliseconds;
} latency_data_t;

typedef struct
{
    int test_id;
    int iteration;
    int max_iterations;
    latency_data_t* p_latency;
} perf_test_result_t;

perf_test_result_t test_result = { 0 };

void RecordLatencyData(const char* device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t tx_hops, uint8_t rx_hops, int elapsed_time)
{
    if ((test_result.iteration < test_result.max_iterations))
    {

        strcpy(test_result.p_latency[test_result.iteration].device_name, device_name);

        test_result.p_latency[test_result.iteration].company_id = company_id;
        test_result.p_latency[test_result.iteration].model_id = model_id;
        test_result.p_latency[test_result.iteration].opcode = opcode;
        test_result.p_latency[test_result.iteration].tx_hops = tx_hops;
        test_result.p_latency[test_result.iteration].rx_hops = rx_hops;
        test_result.p_latency[test_result.iteration].elapsed_time = elapsed_time;
        test_result.iteration++;
    }
}

// CMeshPerformance dialog
IMPLEMENT_DYNAMIC(CMeshPerformance, CPropertyPage)

CMeshPerformance::CMeshPerformance()
    : CPropertyPage(IDD_MESH_PERF)
{
    m_trace = NULL;
    m_timer_count = 0;
    m_max_iterations = MESH_PERF_MAX_ITERATIONS;
    m_bLocalDisableNtwkRetransmit = FALSE;
}

CMeshPerformance::~CMeshPerformance()
{
}

void CMeshPerformance::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMeshPerformance, CPropertyPage)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CLEAR_TRACE, &CMeshPerformance::OnBnClickedClearTrace)
    ON_BN_CLICKED(IDC_MESH_PERF_VS_DATA, &CMeshPerformance::OnBnClickedVsData)
    ON_BN_CLICKED(IDC_VS_PERF_TEST_START, &CMeshPerformance::OnBnClickedVsPerfTest)
    ON_BN_CLICKED(IDC_VS_PERF_TEST_STOP, &CMeshPerformance::OnBnClickedVsPerfTestStop)

    ON_BN_CLICKED(IDC_MESH_SET_ADV_TX_POWER, &CMeshPerformance::OnBnClickedMeshSetAdvTxPower)
    ON_CBN_SELCHANGE(IDC_PERF_CONTROL_DEVICE, &CMeshPerformance::OnCbnSelchangePerfControlDevice)
    ON_BN_CLICKED(IDC_VS_PERF_SHOW_RESULTS, &CMeshPerformance::OnBnClickedVsPerfShowResults)
    ON_BN_CLICKED(IDC_MESH_SET_NTWK_RETRANSMIT, &CMeshPerformance::OnBnClickedMeshSetNtwkRetransmit)
    ON_BN_CLICKED(IDC_MESH_SET_ADV_TX_POWER_ALL, &CMeshPerformance::OnBnClickedMeshSetAdvTxPowerAll)
    ON_BN_CLICKED(IDC_MESH_SET_NTWK_RETRANSMIT_ALL, &CMeshPerformance::OnBnClickedMeshSetNtwkRetransmitAll)
END_MESSAGE_MAP()

BOOL CMeshPerformance::OnSetActive()
{
    CPropertyPage::OnSetActive();
    m_trace = (CListBox*)GetDlgItem(IDC_TRACE);
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    pSheet->m_active_page = 1; // Leaving this as-is so lightcontrol can process all cmds/events

    SyncControlDevices();

    SetDlgItemText(IDC_MESH_PERF_VENDOR_DATA, L"123456");
    SetDlgItemText(IDC_NUM_ITERATIONS, L"10");
    SetDlgItemText(IDC_PERF_PACKET_LEN, L"6");

    ((CComboBox*)(GetDlgItem(IDC_PERF_CONTROL_DEVICE)))->SetCurSel(0);
    ((CComboBox*)(GetDlgItem(IDC_ADV_TX_POWER)))->SetCurSel(0);
    ((CComboBox*)(GetDlgItem(IDC_NTWK_RETRANSMIT)))->SetCurSel(0);

    return TRUE; // return TRUE unless you set the focus to a control
}

void CMeshPerformance::OnClose()
{
    CPropertyPage::OnClose();
}

void CMeshPerformance::OnCancel()
{
    CPropertyPage::OnCancel();
}

void CMeshPerformance::OnBnClickedClearTrace()
{
    m_trace->ResetContent();
}

DWORD CMeshPerformance::GetHexValueInt(DWORD id)
{
    DWORD ret = 0;
    BYTE buf[32];
    DWORD len = GetHexValue(id, buf, sizeof(buf));
    for (DWORD i = 0; i < len; i++)
    {
        ret = (ret << 8) + buf[i];
    }
    return ret;
}

void CMeshPerformance::SetDlgItemHex(DWORD id, DWORD val)
{
    WCHAR buf[10];
    wsprintf(buf, L"%x", val);
    SetDlgItemText(id, buf);
}

DWORD CMeshPerformance::GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size)
{
    char szbuf[1300];
    char* psz = szbuf;
    BYTE* pbuf = buf;
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
            if (res == buf_size)
                break;
        }
    }
    return res;
}

void CMeshPerformance::InitTestResults(int test_id)
{
    if (test_result.p_latency)
    {
        delete[] test_result.p_latency;
        test_result.p_latency = NULL;
    }

    test_result.p_latency = new latency_data_t[m_max_iterations];

    memset(test_result.p_latency, 0, sizeof(latency_data_t) * m_max_iterations);

    test_result.test_id = test_id;
    test_result.iteration = 0;
    test_result.max_iterations = m_max_iterations;
}

void CMeshPerformance::SendVendorData()
{
    char name[80];
    GetDlgItemTextA(m_hWnd, IDC_PERF_CONTROL_DEVICE, name, sizeof(name));

    BYTE buffer[400];
    DWORD len = GetHexValue(IDC_MESH_PERF_VENDOR_DATA, buffer, sizeof(buffer));

    mesh_client_vendor_data_set(name, 0x131, 0x01, 0x01, m_bLocalDisableNtwkRetransmit, buffer, (uint16_t)len);
}

void CMeshPerformance::OnBnClickedMeshSetNtwkRetransmit()
{
    char name[80] = { 0 };
    BYTE buffer[1];

    GetDlgItemTextA(m_hWnd, IDC_PERF_CONTROL_DEVICE, name, sizeof(name));

    CComboBox* p_ntwk_retransmit = (CComboBox*)(GetDlgItem(IDC_NTWK_RETRANSMIT));
    buffer[0] = (uint8_t)p_ntwk_retransmit->GetCurSel();

    CComboBox* p_device = (CComboBox*)(GetDlgItem(IDC_PERF_CONTROL_DEVICE));
    int selected_device = (uint8_t)p_device->GetCurSel();

    if (selected_device == 0)
    {
        m_bLocalDisableNtwkRetransmit = buffer[0];
        Log(L"Local Device: Disabled Network Retransmission: %d", m_bLocalDisableNtwkRetransmit);
    }
    else
    {
        Log(L"%S: Disabled Network Retransmission: %d", name, buffer[0]);
        // Opcode 0x04 to Disable Network retransmit
        mesh_client_vendor_data_set(name, 0x131, 0x01, 0x04, 0, buffer, 1);
    }
}

void CMeshPerformance::OnBnClickedMeshSetNtwkRetransmitAll()
{
    char name[80] = { 0 };
    BYTE buffer[1];

    //GetDlgItemTextA(m_hWnd, IDC_PERF_CONTROL_DEVICE, name, sizeof(name));

    CComboBox* p_ntwk_retransmit = (CComboBox*)(GetDlgItem(IDC_NTWK_RETRANSMIT));
    buffer[0] = (uint8_t)p_ntwk_retransmit->GetCurSel();

    CComboBox* p_device = (CComboBox*)(GetDlgItem(IDC_PERF_CONTROL_DEVICE));
    int selected_device = (uint8_t)p_device->GetCurSel();
    int dev_count = p_device->GetCount();

    for (int i = 0; i < dev_count; i++)
    {
        if (i == 0)
        {
            m_bLocalDisableNtwkRetransmit = buffer[0];
            Log(L"Local Device: Disabled Network Retransmission: %d", m_bLocalDisableNtwkRetransmit);
        }
        else
        {
            WCHAR szDevName[80];
            memset(name, 0, sizeof(name));
            p_device->GetLBText(i, szDevName);
            WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, name, 80, NULL, FALSE);
            Log(L"%S: Disabled Network Retransmission: %d", name, buffer[0]);
            // Opcode 0x04 to Disable Network retransmit
            mesh_client_vendor_data_set(name, 0x131, 0x01, 0x04, 0, buffer, 1);
            Sleep(500);
        }
    }
}

void CMeshPerformance::OnBnClickedVsData()
{
    m_bPerfTestRunning = TRUE;
    m_max_iterations = 1;
    InitTestResults(1);

    SendVendorData();
}

// Set ADV TX POWER of a destination node
void CMeshPerformance::OnBnClickedMeshSetAdvTxPower()
{
    char name[80] = { 0 };
    BYTE buffer[1];

    GetDlgItemTextA(m_hWnd, IDC_PERF_CONTROL_DEVICE, name, sizeof(name));

    CComboBox* p_tx_power = (CComboBox*)(GetDlgItem(IDC_ADV_TX_POWER));
    buffer[0] = (uint8_t)p_tx_power->GetCurSel();

    CComboBox* p_device = (CComboBox*)(GetDlgItem(IDC_PERF_CONTROL_DEVICE));
    int selected_device = (uint8_t)p_device->GetCurSel();

    if (selected_device == 0)
    {
        mesh_client_core_adv_tx_power_set(buffer[0]);
        Log(L"Local Device: Tx ADV power set to: %d", buffer[0]);
    }
    else
    {
        // Opcode 0x03 to set ADV TX POWER
        mesh_client_vendor_data_set(name, 0x131, 0x01, 0x03, 0, buffer, 1);
    }
}

void CMeshPerformance::OnBnClickedMeshSetAdvTxPowerAll()
{
    char name[80] = { 0 };
    BYTE buffer[1];

    CComboBox* p_tx_power = (CComboBox*)(GetDlgItem(IDC_ADV_TX_POWER));
    buffer[0] = (uint8_t)p_tx_power->GetCurSel();

    CComboBox* p_device = (CComboBox*)(GetDlgItem(IDC_PERF_CONTROL_DEVICE));
    int selected_device = (uint8_t)p_device->GetCurSel();
    int dev_count = p_device->GetCount();

    for (int i = 0; i < dev_count; i++)
    {
        if (i == 0)
        {
            mesh_client_core_adv_tx_power_set(buffer[0]);
            Log(L"Local Device: Tx ADV power set to: %d", buffer[0]);
        }
        else
        {
            WCHAR szDevName[80];
            memset(name, 0, sizeof(name));
            p_device->GetLBText(i, szDevName);
            WideCharToMultiByte(CP_UTF8, 0, szDevName, -1, name, 80, NULL, FALSE);
            // Opcode 0x03 to set ADV TX POWER
            mesh_client_vendor_data_set(name, 0x131, 0x01, 0x03, 0, buffer, 1);
            Sleep(500);
        }
    }
}


void CMeshPerformance::OnBnClickedVsPerfTest()
{
    char name[80] = { 0 };
    Log(L"\n*********Start Test...*********\n");

    int packet_len = GetDlgItemInt(IDC_PERF_PACKET_LEN);
    // payload lengths above 11 cause segmentation and we need more than one mesh packets
    if (packet_len > 0 && packet_len <= 20)
    {
        m_packet_len = packet_len;

        memset(name, 0, sizeof(name));
        for (int i = 1; i <= 2 * m_packet_len; i++)
            sprintf(&name[strlen(name)], "%d", i % 10);
    }
    else
    {
        m_packet_len = 3; // default for testing
        strcpy(name, "123456");
    }

    SetDlgItemTextA(m_hWnd, IDC_MESH_PERF_VENDOR_DATA, name);

    int num_iterations = GetDlgItemInt(IDC_NUM_ITERATIONS);
    if (num_iterations > 0 && num_iterations <= 50)
        m_max_iterations = num_iterations;
    else
        m_max_iterations = MESH_PERF_MAX_ITERATIONS;

    InitTestResults(2);

    // Set Timer and on timer trigger OnBnClickedVsData to send data
    // After m_max_iterations times stop the timer
    m_timer_count = 0;
    m_bPerfTestRunning = TRUE;
    SetTimer(IDT_SEND_VENDOR_DATA, SEND_VENDOR_DATA_INTERVAL, NULL);

    //// For testing only.
    //char fieldval[] = "Starting Test...";
    //SendMessageToUDPServer((char*)fieldval, strlen(fieldval));
}

void CMeshPerformance::OnBnClickedVsPerfTestStop()
{
    m_timer_count = 0;
    KillTimer(IDT_SEND_VENDOR_DATA);
    Log(L"\n*********End Test...*********\n");
    m_bPerfTestRunning = FALSE;
}

BOOL CMeshPerformance::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_trace = (CListBox*)GetDlgItem(IDC_TRACE);
    m_trace->AddString(L"NOTE:");
    m_trace->AddString(L"1. Create a network, open and provision as Mesh nodes in the test in the Light Control tab ");
    m_trace->AddString(L"2. Use the options available on this screen to run performance tests");

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CMeshPerformance::OnTimer(UINT_PTR nIDEvent)
{
    CPropertyPage::OnTimer(nIDEvent);

    switch (nIDEvent)
    {
    case IDT_SEND_VENDOR_DATA:

        m_timer_count++;

        if (m_timer_count > m_max_iterations)
        {
            m_timer_count = 0;
            KillTimer(IDT_SEND_VENDOR_DATA);
            Log(L"\n*********End Test...*********\n");

            m_bPerfTestRunning = FALSE;
        }
        else
        {
            Log(L"*********Send attempt: %d *********\n", m_timer_count);
            SendVendorData();
        }
        break;

    default:
        break;
    }
}

// Synchronize the currently selected device between mesh performance and light control tabs
void CMeshPerformance::OnCbnSelchangePerfControlDevice()
{

}

// Synchronize the currently selected device between mesh performance and light control tabs
void CMeshPerformance::SyncControlDevices()
{
    WCHAR szName[80] = { 0 };

    CComboBox* p_perf_ctl_devices = (CComboBox*)GetDlgItem(IDC_PERF_CONTROL_DEVICE);
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    if (pSheet->m_active_page == 1 && theApp.bMeshPerfMode)
    {
        CLightControl* pDlg = &pSheet->pageLight;
        if (pDlg)
        {
            CComboBox* p_target_devs_groups = (CComboBox*)(pDlg->GetDlgItem(IDC_CONTROL_DEVICE));

            p_perf_ctl_devices->ResetContent();

            p_perf_ctl_devices->AddString(L"Local Device");

            int count = p_target_devs_groups->GetCount();

            for (int i = 0; i < count; i++)
            {
                memset(szName, 0, sizeof(szName));
                p_target_devs_groups->GetLBText(i, szName);
                p_perf_ctl_devices->AddString(szName);
            }
        }
    }
}

void CMeshPerformance::OnBnClickedVsPerfShowResults()
{
    char msg[512] = { 0 };
    char strfilename[128] = { 0 };
    latency_data_t* p_lat_data = NULL;
    FILE* fp;
    SYSTEMTIME st;
    GetLocalTime(&st);

    int len = sprintf(strfilename, "test_results_%02d_%02d_%02d_%02d_%02d_%02d.txt", st.wMonth, st.wDay, st.wHour, st.wMinute, m_max_iterations, m_packet_len);
    fopen_s(&fp, strfilename, "w");

    if (!fp)
        return;

    memset(msg, 0, sizeof(msg));
    sprintf(msg, "Mesh Performance - Reliability and Latency Test Results \n");
    sprintf(&msg[strlen(msg)], "---------------------------------------------------------------------\n");
    sprintf(&msg[strlen(msg)], "Number of iterations: %d\n", m_max_iterations);
    sprintf(&msg[strlen(msg)], "Packet size: %d\n", m_packet_len);
    sprintf(&msg[strlen(msg)], "#\tDevice_name\tcompany_id\tmodel_id\topcode\ttx_hops\trx_hops\troundtrip_time\n");
    sprintf(&msg[strlen(msg)], "---------------------------------------------------------------------\n");
    fputs(msg, fp);

    for (int i = 0; i < m_max_iterations; i++)
    {
        memset(msg, 0, sizeof(msg));
        p_lat_data = &test_result.p_latency[i];

        sprintf(msg, "%d\t%s\t%x\t%x\t%d\t%x\t%d\t%d\n", i + 1, p_lat_data->device_name, p_lat_data->company_id,
            p_lat_data->model_id, p_lat_data->opcode, p_lat_data->tx_hops, p_lat_data->rx_hops, p_lat_data->elapsed_time);

        fputs(msg, fp);
    }

    if (fp)
        fclose(fp);
}
