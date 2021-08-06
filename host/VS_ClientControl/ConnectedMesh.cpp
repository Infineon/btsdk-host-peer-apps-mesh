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
// ConnectedMesh.cpp : implementation file
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
#include "ConnectedMesh.h"
#include <mmsystem.h>

extern BOOL SendMessageToUDPServer(char* p_msg, UINT len);
extern void TraceHciPkt(BYTE type, BYTE* buffer, USHORT length);
extern void Log(WCHAR* fmt, ...);
extern void LogFile(WCHAR* fmt, ...);
extern "C" void wiced_hci_process_data(uint16_t opcode, uint8_t * p_buffer, uint16_t len);
extern DWORD GetHexValue(char* szbuf, LPBYTE buf, DWORD buf_size);

CRITICAL_SECTION cs_ui;

MMRESULT timerid = 0;

static int data_len_down = 25;
static int battery_len_up = 50;


// ConnectedMesh dialog
IMPLEMENT_DYNAMIC(ConnectedMesh, CPropertyPage)

ConnectedMesh::ConnectedMesh()
    : CPropertyPage(IDD_CONN_MESH)
{
    m_trace = NULL;
    m_bProvisioning = FALSE;
    m_sendingData = FALSE;
    InitializeCriticalSection(&cs_ui);
}

ConnectedMesh::~ConnectedMesh()
{
}

void ConnectedMesh::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ConnectedMesh, CPropertyPage)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CLEAR_TRACE, &ConnectedMesh::OnBnClickedClearTrace)
    ON_CBN_SELCHANGE(IDC_COM_PORT, &ConnectedMesh::OnCbnSelchangeComPort)
    ON_CBN_SELCHANGE(IDC_COM_BAUD, &ConnectedMesh::OnCbnSelchangeComPort)

    ON_BN_CLICKED(IDC_CONN_MESH_BECOME_PROVISIONER, &ConnectedMesh::OnBnClickedBecomeProvisioner)
    ON_BN_CLICKED(IDC_CONN_MESH_END_PROVISIONING, &ConnectedMesh::OnBnClickedEndProvisioning)
    ON_BN_CLICKED(IDC_CONN_MESH_GET_NODE_LIST, &ConnectedMesh::OnBnClickedGetNodeList)
    ON_BN_CLICKED(IDC_CONN_MESH_SEND_DATA, &ConnectedMesh::OnBnClickedSendData)
    ON_BN_CLICKED(IDC_CONN_MESH_GET_CONN_STATUS1, &ConnectedMesh::OnBnClickedGetConnStatus1)
    ON_BN_CLICKED(IDC_CONN_MESH_RUN_PING_TEST, &ConnectedMesh::OnBnClickedRunPingTest)
    ON_BN_CLICKED(IDC_CONN_MESH_START_DATA, &ConnectedMesh::OnBnClickedConnMeshStartData)
    ON_BN_CLICKED(IDC_CONN_MESH_FACTORY_RESET, &ConnectedMesh::OnBnClickedConnMeshFactoryReset)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CONN_MESH_RESET, &ConnectedMesh::OnBnClickedConnMeshReset)
    ON_BN_CLICKED(IDC_CONN_MESH_GET_DATA_STATS, &ConnectedMesh::OnBnClickedConnMeshGetDataStats)
    ON_BN_CLICKED(IDC_CONN_MESH_IDENTIFY, &ConnectedMesh::OnBnClickedConnMeshIdentify)
END_MESSAGE_MAP()

BOOL ConnectedMesh::OnSetActive()
{
    CPropertyPage::OnSetActive();
    m_trace = (CListBox*)GetDlgItem(IDC_TRACE);
    CClientDialog* pSheet = (CClientDialog*)theApp.m_pMainWnd;
    ((CClientDialog*)theApp.m_pMainWnd)->m_active_page = idxPageConnectedMesh;

    CComboBox* m_cbCom = (CComboBox*)GetDlgItem(IDC_COM_PORT);
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

        CComboBox* m_cbBaud = (CComboBox*)GetDlgItem(IDC_COM_BAUD);
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
        ((CComboBox*)GetDlgItem(IDC_COM_PORT))->SetCurSel(ComPortSelected);

    if (BaudRateSelected > 0)
        ((CComboBox*)GetDlgItem(IDC_COM_BAUD))->SetCurSel(BaudRateSelected);

    SetDlgItemText(IDC_CONN_MESH_APP_DATA, L"123456");

    SetDlgItemText(IDC_CONN_MESH_START_DATA, m_sendingData ? L"Stop Sending Data" : L"Start Sending Data");

    SetDlgItemInt (IDC_CONN_MESH_HOST_DATA_LEN, data_len_down);
    SetDlgItemInt (IDC_CONN_MESH_BATTERY_DATA_LEN, battery_len_up);

    CComboBox* cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
    cbNodeAddr->ResetContent();
    cbNodeAddr->SetItemData(cbNodeAddr->AddString(L"0x0001"), 1);
    cbNodeAddr->SetCurSel(0);
    return TRUE; // return TRUE unless you set the focus to a control

}

void ConnectedMesh::OnClose()
{
    CPropertyPage::OnClose();
}

void ConnectedMesh::OnCancel()
{
    CPropertyPage::OnCancel();
}

void ConnectedMesh::OnCbnSelchangeComPort()
{
    CComboBox* m_cbCom = (CComboBox*)GetDlgItem(IDC_COM_PORT);

    ComPortSelected = m_cbCom->GetCurSel();
    BaudRateSelected = ((CComboBox*)GetDlgItem(IDC_COM_BAUD))->GetCurSel();

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

int ConnectedMesh::GetBaudRateSelection()
{
    CComboBox* m_cbBaud = (CComboBox*)GetDlgItem(IDC_COM_BAUD);
    int select = m_cbBaud->GetItemData(m_cbBaud->GetCurSel());

    if (select >= 0)
        return as32BaudRate[select];

    return as32BaudRate[0];
}

void ConnectedMesh::OnBnClickedClearTrace()
{
    m_trace->ResetContent();
}

void ConnectedMesh::OnBnClickedBecomeProvisioner()
{
    memset(m_provisioned_nodes, 0, sizeof(m_provisioned_nodes));
    CComboBox* cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
    ((CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR))->ResetContent();
    cbNodeAddr->SetItemData(cbNodeAddr->AddString(L"0x0001"), 1);
    cbNodeAddr->SetCurSel(0);
    Log(L"\n[1]  ********* Sending 'Become Provisioner'...*********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_BECOME_PROVISIONER, NULL, 0);
}

void ConnectedMesh::OnBnClickedEndProvisioning()
{
    Log(L"\n[1]  ********* Sending 'End Provisioning'...*********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_END_PROVISIONING, NULL, 0);
}

void ConnectedMesh::OnBnClickedGetNodeList()
{
    Log(L"\n[1]  *********  Getting Node List  *********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_GET_NODE_LIST, NULL, 0);
}

void ConnectedMesh::OnBnClickedGetConnStatus1()
{
    Log(L"\n[1]  *********  Getting Connection Status  *********\n");
    m_ComHelper->SendWicedCommand (HCI_CONTROL_CONN_MESH_COMMAND_GET_CONN_STATUS, NULL, 0);
}

void ConnectedMesh::OnBnClickedRunPingTest()
{
    Log(L"\n[1]  *********  Starting Ping Test  *********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_PING_ALL_NODES, NULL, 0);
}

static void CALLBACK _TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ConnectedMesh* pDlg = (ConnectedMesh*)dwUser;
    pDlg->SendData();
}

void ConnectedMesh::SendData()
{
    WCHAR buff[300];
    static UINT seq = 0;
    int i = 0;

    if (!m_sendingData)
        KillTimer(1);
    else
    {
        uint8_t buffer[1024];
        if (m_bStarting)
        {
            m_bStarting = FALSE;
            startTime = GetTickCount();
        }
        // SendMessage(WM_SETREDRAW, false, 0);
        uint16_t total_len_down = 0;
        memset(buffer, seq & 0xFF, sizeof(buffer));
        for (int i = 0; i < MAX_NODES; i++)
        {
            if ( (m_provisioned_nodes[i].addr != 0) && (data_len_down != 0) )
            {
                uint8_t* p = &buffer[total_len_down];
                UINT16_TO_STREAM(p, m_provisioned_nodes[i].addr);
                UINT8_TO_STREAM(p, data_len_down);
                p += data_len_down;
                LogFile(L"Send  App Data   Dst: 0x%04x  Data Len: %d  Data: %02x %02x %02x %02x %02x %02x %02x", m_provisioned_nodes[i].addr, data_len_down, p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
                m_provisioned_nodes[i].bytesTx += data_len_down;
                m_provisioned_nodes[i].packetsTx++;
                total_len_down += (3 + data_len_down);
                if (total_len_down + 3 + data_len_down > 1024)
                {
                    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_SEND_APP_DATA, buffer, total_len_down);
                    total_len_down = 0;
                }
            }
        }
        if (total_len_down != 0)
            m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_SEND_APP_DATA, buffer, total_len_down);
        // SendMessage(WM_SETREDRAW, true, 0);
        seq++;
        if ((seq % 10) == 0)
        {
            EnterCriticalSection(&cs_ui);
            UINT timePassed = GetTickCount() - startTime;
            for (int i = 0; i < MAX_NODES; i++)
            {
                if (m_provisioned_nodes[i].addr == 0)
                    continue;
                m_provisioned_nodes[i].bpsRx = (timePassed != 0) ? (((m_provisioned_nodes[i].bytesRx * 1000) + (timePassed / 2)) / timePassed) : 0;
                m_provisioned_nodes[i].bpsTx = (timePassed != 0) ? (((m_provisioned_nodes[i].bytesTx * 1000) + (timePassed / 2)) / timePassed) : 0;
                wsprintf(buff, L"0x%04x\n%d/%d\n%d/%d\n%d/%d", m_provisioned_nodes[i].addr, m_provisioned_nodes[i].packetsTx, m_provisioned_nodes[i].packetsRx, m_provisioned_nodes[i].bytesTx, m_provisioned_nodes[i].bytesRx, m_provisioned_nodes[i].bpsTx, m_provisioned_nodes[i].bpsRx);
                SetDlgItemText(IDC_CONN_MESH_TX_RX_DATA_1 + i, buff);
            }
            LeaveCriticalSection(&cs_ui);
        }
    }
}

void ConnectedMesh::OnBnClickedConnMeshStartData()
{
    uint8_t buffer[1];        // Space for a parameter if needed

    if (!m_sendingData)
    {
        Log (L"\n[1]  *********  Start Data Test  *********\n");

        data_len_down = GetDlgItemInt (IDC_CONN_MESH_HOST_DATA_LEN);
        battery_len_up = GetDlgItemInt (IDC_CONN_MESH_BATTERY_DATA_LEN);

        buffer[0] = (uint8_t)battery_len_up;

        for (int i = 0; i < MAX_NODES; i++)
        {
            if (m_provisioned_nodes[i].addr == 0)
                continue;

            Log(L"%04x\n", m_provisioned_nodes[i].addr);
            m_provisioned_nodes[i].packetsTx = 0;
            m_provisioned_nodes[i].packetsRx = 0;
            m_provisioned_nodes[i].bytesTx = 0;
            m_provisioned_nodes[i].bytesRx = 0;
            m_provisioned_nodes[i].bpsRx = 0;
            m_provisioned_nodes[i].bpsTx = 0;
        }
        m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_START_STOP_DATA, buffer, 1);
        timeBeginPeriod(1);     // request 1ms accuracy
        timerid = timeSetEvent(100, 0, _TimerProc, (DWORD_PTR)this, TIME_PERIODIC);

        // SetTimer(1, 100, NULL);
        m_bStarting = TRUE;
        m_sendingData = TRUE;
        SetDlgItemText(IDC_CONN_MESH_START_DATA, L"Stop Sending Data");
    }
    else
    {
        Log(L"\n[1]  *********  Stop Data Test  *********\n");
        buffer[0] = 0;

        if (timerid != 0)
        {
            timeKillEvent(timerid);
            timeEndPeriod(1);
            timerid = 0;
        }

        // Allow time for forward data to get out of the queues.
        Sleep (1000);

        m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_START_STOP_DATA, buffer, 1);
        // KillTimer(1);
        m_sendingData = FALSE;
        SetDlgItemText(IDC_CONN_MESH_START_DATA, L"Start Sending Data");
    }
}

void ConnectedMesh::OnBnClickedConnMeshFactoryReset()
{
    memset(m_provisioned_nodes, 0, sizeof(m_provisioned_nodes));
    CComboBox* cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
    ((CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR))->ResetContent();
    cbNodeAddr->SetItemData(cbNodeAddr->AddString(L"0x0001"), 1);
    cbNodeAddr->SetCurSel(0);
    Log(L"\n[1]  *********  Factory Reset  *********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_FACTORY_RESET, NULL, 0);
}

void ConnectedMesh::OnBnClickedConnMeshReset()
{
    Log(L"\n[1]  *********  Reset  *********\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_RESET, NULL, 0);
}

BOOL ConnectedMesh::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_trace = (CListBox*)GetDlgItem(IDC_TRACE);
    m_trace->AddString(L"Click 'Become Provisioner' to init device 1 as a provisioner. When node is");
    m_trace->AddString(L"provisioned and configured, click 'Advertise' and 'Connect' and then 'Send Data'");

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void ConnectedMesh::OnBnClickedSendData()
{
    BYTE buffer[400];
    DWORD len = GetHexValue(IDC_CONN_MESH_APP_DATA, buffer, sizeof(buffer));

    if (m_ComHelper2 != NULL)
    {
        Log(L"\n[2]  *********  Sending Data  *********\n");

        m_ComHelper2->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_SEND_APP_DATA, buffer, len);
    }
    else
        Log(L"\n*********  ERROR - no connection on [2] *********\n");
}

DWORD ConnectedMesh::GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size)
{
    char szbuf[1300];
    char* psz = szbuf;
    BYTE* pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, buf_size);

    GetDlgItemTextA (m_hWnd, id, szbuf, sizeof(szbuf));
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

void ConnectedMesh::ProcessData(int port_num, DWORD opcode, LPBYTE p_data, DWORD len)
{
    LPBYTE  p = p_data;
    UINT16  src, node;
    int     xx, yy, num_nodes;
    WCHAR   buff[1000];
    uint32_t tx_delay[MAX_RCVD_PACKET_DELAY];
    uint32_t rx_delay[MAX_RCVD_PACKET_DELAY];
    UINT8   connected;
    UINT32  ping_rtt;
    CComboBox* cbNodeAddr;
    int i = 0;

    switch (opcode)
    {
    case HCI_CONTROL_CONN_MESH_EVENT_NODE_PROVISIONED:
        STREAM_TO_UINT16(node, p);
        Log(L"New Node Provisioned, Address: 0x%04x", node);
        if (node != 1)
        {
            wsprintf(buff, L"0x%04x", node);
            cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
            cbNodeAddr->SetItemData(cbNodeAddr->AddString(buff), node);
            for (i = 0; i < MAX_NODES; i++)
            {
                if (m_provisioned_nodes[i].addr == 0)
                {
                    m_provisioned_nodes[i].addr = node;
                    break;
                }
            }
        }
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_OP_STATE_CHANGED:
        STREAM_TO_UINT16(node, p);
        Log(L"Operating state changed, Address: 0x%04x", node);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_RESET_COMPLETE:
        STREAM_TO_UINT16(node, p);
        Log(L"Reset complete, Address: 0x%04x", node);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_CONN_UP:
        STREAM_TO_UINT16(node, p);
        Log(L"Connection up, Address: 0x%04x", node);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_CONN_DOWN:
        STREAM_TO_UINT16(node, p);
        Log(L"Connection down, Address: 0x%04x reason:%d", node, p[0]);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_APP_DATA:
        EnterCriticalSection(&cs_ui);
        {
            int total_len = (int)len;
#if 0
            static int prev_rcvd_len = 0;
            if (total_len != prev_rcvd_len)
            {
                int i;
                prev_rcvd_len = total_len;
                LogFile(L"Len:%d", total_len);
                WCHAR* p_buf = (WCHAR*)malloc(2 * (total_len * 3 + 2));
                if (p_buf != 0)
                {
                    for (i = 0; i < total_len; i++)
                    {
                        wsprintf(&p_buf[i * 3], L"%02x ", p[i]);
                    }
                    p_buf[i * 3] = 0;
                    LogFile(p_buf);
                    free(p_buf);
                }
            }
#endif
            while (total_len > 0)
            {
                uint8_t msg_len;
                STREAM_TO_UINT16(src, p);
                STREAM_TO_UINT8(msg_len, p);
                LogFile(L"Rcvd  App Data  Rem Len: %d  Src: 0x%04x  Data Len: %d  Data: %02x %02x %02x %02x %02x %02x %02x", total_len, src, msg_len, p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
                // SendMessage(WM_SETREDRAW, false, 0);
                for (i = 0; i < MAX_NODES; i++)
                {
                    if (m_provisioned_nodes[i].addr == src)
                    {
                        m_provisioned_nodes[i].bytesRx += msg_len;
                        m_provisioned_nodes[i].packetsRx++;
                        break;
                    }
                }
                p += msg_len;
                total_len -= (msg_len + 3);
            }
        }
        // SendMessage(WM_SETREDRAW, true, 0);
        LeaveCriticalSection(&cs_ui);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_NODE_LIST:
        memset(m_provisioned_nodes, 0, sizeof(m_provisioned_nodes));
        cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
        cbNodeAddr->ResetContent();
        cbNodeAddr->SetItemData(cbNodeAddr->AddString(L"0x0001"), 1);
        cbNodeAddr->SetCurSel(0);
        num_nodes = len / 2;
        buff[0] = 0;
        for (xx = yy = 0; xx < num_nodes; xx++)
        {
            STREAM_TO_UINT16(node, p);
            WCHAR buff1[10];
            wsprintf(buff1, L"0x%04x", node);
            cbNodeAddr->SetItemData(cbNodeAddr->AddString(buff1), node);
            yy += wsprintf (&buff[yy], L"0x%04x  ", node);
            for (i = 0; i < MAX_NODES; i++)
            {
                if (m_provisioned_nodes[i].addr == 0)
                {
                    m_provisioned_nodes[i].addr = node;
                    break;
                }
            }
        }
        Log (L"Num Nodes: %d   Addresses: %s", num_nodes, buff);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_CONN_STATUS:
        STREAM_TO_UINT16 (node, p);
        len -= 2;
        num_nodes = len / 3;
        Log(L"Node: 0x%04x  Num Conns: %d", node, num_nodes);
        memset(m_provisioned_nodes, 0, sizeof(m_provisioned_nodes));
        cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
        cbNodeAddr->ResetContent();
        cbNodeAddr->SetItemData(cbNodeAddr->AddString(L"0x0001"), 1);
        cbNodeAddr->SetCurSel(0);

        for (xx = 0; xx < num_nodes; xx++)
        {
            STREAM_TO_UINT16(node, p);
            STREAM_TO_UINT8(connected, p);
            Log(L"                    Node: 0x%04x   Connected: %s", node, connected ? L"Yes" : L"No");
            WCHAR buff[10];
            wsprintf(buff, L"0x%04x", node);
            cbNodeAddr->SetItemData(cbNodeAddr->AddString(buff), node);

            for (i = 0; i < MAX_NODES; i++)
            {
                if (m_provisioned_nodes[i].addr == 0)
                {
                    m_provisioned_nodes[i].addr = node;
                    break;
                }
            }
        }
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_PING_RESULT:
        STREAM_TO_UINT16(node, p);
        STREAM_TO_UINT32(ping_rtt, p);

        if (ping_rtt != 0xFFFFFFFF)
            Log(L"Got Ping Response from Node: 0x%04x   RTT: %d ms\n", node, ping_rtt);
        else
            Log(L"Ping TIMEOUT from Node: 0x%04x\n", node);
        break;

    case HCI_CONTROL_CONN_MESH_EVENT_DATA_STATS:
        STREAM_TO_UINT16(node, p);
        for (int i = 0; i < MAX_RCVD_PACKET_DELAY; i++)
            STREAM_TO_UINT32(tx_delay[i], p);
        for (int i = 0; i < MAX_RCVD_PACKET_DELAY; i++)
            STREAM_TO_UINT32(rx_delay[i], p);

        Log (L"Node: 0x%04x      Rcvd at node:      %10u        %10u        %10u                Rcvd at central:      %10u        %10u        %10u\n", node,
            tx_delay[0], tx_delay[1], tx_delay[2], rx_delay[0], rx_delay[1], rx_delay[2]);
        break;

    case HCI_CONTROL_EVENT_WICED_TRACE:
        if (len >= 2)
        {
            if ((len > 2) && (p_data[len - 2] == '\n'))
            {
                p_data[len - 2] = 0;
                len--;
            }
            TraceHciPkt (0, p_data, (USHORT)len);
        }
        //MultiByteToWideChar(CP_ACP, 0, (const char *)p_data, len, trace, sizeof(trace) / sizeof(WCHAR));
        //m_trace->SetCurSel(m_trace->AddString(trace));
        break;

    case HCI_CONTROL_EVENT_HCI_TRACE:
        TraceHciPkt (p_data[0] + 1, &p_data[1], (USHORT)(len - 1));
        break;

    default:
        wiced_hci_process_data((uint16_t)opcode, p_data, (uint16_t)len);
        break;
    }
}


void ConnectedMesh::OnTimer(UINT_PTR nIDEvent)
{
    static USHORT seq = 0;
    if (!m_sendingData)
        KillTimer(1);
    else
    {
        uint8_t buffer[100];

        memset(buffer, seq, sizeof(buffer));
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (m_provisioned_nodes[i].addr != 0)
            {
                uint8_t* p = buffer;
                UINT16_TO_STREAM(p, m_provisioned_nodes[i].addr);
                Log(L"Send  App Data   Dst: 0x%04x  Data Len: %d  Data: %02x %02x %02x %02x %02x %02x %02x", m_provisioned_nodes[i].addr, data_len_down, p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
                m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_SEND_APP_DATA, buffer, data_len_down);
            }
        }
        seq++;
    }
    CPropertyPage::OnTimer(nIDEvent);
}

void ConnectedMesh::OnBnClickedConnMeshGetDataStats()
{
    Log(L" ");
    Log(L"     Transmission delays                 < 100ms       < 200ms        > 200ms                                             < 100ms       < 200ms        > 200ms\n");
    m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_GET_STATS, NULL, 0);
}


void ConnectedMesh::OnBnClickedConnMeshIdentify()
{
    CComboBox* cbNodeAddr = (CComboBox*)GetDlgItem(IDC_CONN_MESH_NODE_ADDR);
    int sel = cbNodeAddr->GetCurSel();
    if (sel >= 0)
    {
        uint16_t node = (uint16_t)cbNodeAddr->GetItemData(sel);
        uint8_t buffer[100], *p = buffer;
        UINT16_TO_STREAM(p, node);
        Log(L"\n[1]  *********  Sending Identify Node 0x%04x  *********\n", node);
        m_ComHelper->SendWicedCommand(HCI_CONTROL_CONN_MESH_COMMAND_IDENTIFY, buffer, (uint16_t)(p - buffer));
    }
}
