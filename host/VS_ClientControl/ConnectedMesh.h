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
#include <list>

// ConnectedMesh dialog
struct ProvisionedNode
{
    BYTE   addr;
    UINT   packetsTx;
    UINT   packetsRx;
    UINT   bytesTx;
    UINT   bytesRx;
    UINT   bpsRx;
    UINT   bpsTx;
};

struct FirmwareID
{
    BYTE   length;
#define MAX_FIRMWARE_ID_LEN   20
    BYTE   id[MAX_FIRMWARE_ID_LEN];
};

struct FirmwareUpdateNode
{
    BYTE   addr;
    BYTE   state;
};

#define MAX_RCVD_PACKET_DELAY   3

class ConnectedMesh : public CPropertyPage
{
    DECLARE_DYNAMIC(ConnectedMesh)

public:
    ConnectedMesh();
    virtual ~ConnectedMesh();
    void OnCancel();
    void SendData();

    CListBox* m_trace;
#define AUTO_STATE_IDLE                 0
#define AUTO_STATE_PROVISIONING         1
#define AUTO_STATE_SWITCHING_OPERATING  2
#define AUTO_STATE_WAITING_CONN_UP      3
#define AUTO_STATE_WAITING_SEND_DATA    4
#define AUTO_STATE_SEND_DATA            5
#define AUTO_STATE_STOPPING_SEND_DATA   6
#define AUTO_STATE_RESETTING            7
#define AUTO_STATE_WAIT_RESTART         8
    int m_autoState;
    int m_numNodes;
    int m_numPendingNodes;
    BOOL m_sendingData;
    // std::list<ProvisionedNode> m_provisioned_nodes;
#define MAX_NODES   24
    ProvisionedNode m_provisioned_nodes[MAX_NODES];
    BOOL m_bProvisioning;
    BOOL m_bStarting;
    UINT startTime;

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CONN_MESH };
#endif


protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    int GetBaudRateSelection();

public:
    virtual BOOL OnSetActive();
    afx_msg void OnClose();
    afx_msg void OnBnClickedClearTrace();

    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeComPort();
    afx_msg void OnBnClickedBecomeProvisioner();
    afx_msg void OnBnClickedGetNodeList();
    afx_msg void OnBnClickedGetConnStatus1();
    afx_msg void OnBnClickedRunPingTest();
    afx_msg void OnBnClickedEndProvisioning();

    DWORD GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size);
    void  ProcessData(INT port_num, DWORD opcode, LPBYTE p_data, DWORD len);

    afx_msg void OnBnClickedConnMeshStartData();
    afx_msg void OnBnClickedConnMeshFactoryReset();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedConnMeshReset();
    afx_msg void OnBnClickedConnMeshGetDataStats();
    afx_msg void OnBnClickedConnMeshIdentify();
    afx_msg void OnBnClickedConnMeshGetRssi ();
    afx_msg void OnBnClickedConnMeshUpdateBrowse();
    afx_msg void OnBnClickedConnMeshUpdateStartStop();
    afx_msg void OnBnClickedConnMeshGetNodesInfo();

private:
#define UPDATE_STATE_IDLE           0       // Idle
#define UPDATE_STATE_STARTING       1       // Update starting
#define UPDATE_STATE_UPDATING       2       // Sending image to nodes
#define UPDATE_STATE_APPLYING       3       // Nodes applying to new firmware image
    UINT8 m_nUpdateState;
    CString m_sFirmwareFilePath;
    UINT m_nFirmwareSize;
    LPBYTE m_pFirmwareImage;
    LPBYTE m_pDataSendPtr;
    FirmwareID m_FirmwareId;
    UINT8 m_nUpdateMtu;
    FirmwareUpdateNode m_UpdateNodeList[MAX_NODES];
    int m_nUpdateNodes;
    int m_nTotalNodes;
    int m_nFactoryReset;

    void SetUpdateState(UINT8 state);
    BOOL OnUpdateStart();
    void OnUpdateStop();
    int SendFirmwareData();
    BOOL ReadFirmwareManifestFile(CString sFilePath);
};

extern CClientControlApp theApp;
