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
#pragma once

#include "ClientControl.h"
#include "ControlComm.h"

// CConfig dialog

class CConfig : public CPropertyPage // public CDialogEx
{
	DECLARE_DYNAMIC(CConfig)

public:
	CConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfig();
    DWORD GetHexValueInt(DWORD id);
    void SetDlgItemHex(DWORD id, DWORD val);
    void ProcessData(DWORD opcode, LPBYTE p_data, DWORD len);
    void ProcessEvent(LPBYTE p_data, DWORD len);

    CListBox *m_trace;

    HANDLE m_event;
#define STATE_IDLE                          0
#define STATE_LOCAL_GET_COMPOSITION_DATA    1
#define STATE_LOCAL_ADD_APPLICATION_KEY     2
#define STATE_LOCAL_BIND_MODELS             3
#define STATE_REMOTE_GET_COMPOSITION_DATA   4
#define STATE_REMOTE_ADD_APPLICATION_KEY    5
#define STATE_REMOTE_BIND_MODELS            6
    int m_state;

    BYTE *p_local_composition_data;
    USHORT local_composition_data_len;
    BYTE *p_remote_composition_data;
    USHORT remote_composition_data_len;

    // Dialog Data
    void ProcessScanCapabilitiesStatus(LPBYTE p_data, DWORD len);
    void ProcessScanStatus(LPBYTE p_data, DWORD len);
    void ProcessScanReport(LPBYTE p_data, DWORD len);
    void ProcessExtendedScanReport(LPBYTE p_data, DWORD len);
    void ProcessProxyDeviceNetworkData(LPBYTE p_data, DWORD len);
    void ProcessProvisionEnd(LPBYTE p_data, DWORD len);
    void ProcessProvisionLinkReport(LPBYTE p_data, DWORD len);
    void ProcessProvisionDeviceCapabilities(LPBYTE p_data, DWORD len);
    void ProcessProvisionOobDataRequest(LPBYTE p_data, DWORD len);
    void ProcessProxyConnectionStatus(LPBYTE p_data, DWORD len);
    void ProcessNodeResetStatus(LPBYTE p_data, DWORD len);
    void ProcessCompositionDataStatus(LPBYTE p_data, DWORD len);
    void ProcessFriendStatus(LPBYTE p_data, DWORD len);
    void ProcessKeyRefreshPhaseStatus(LPBYTE p_data, DWORD len);
    void ProcessGattProxyStatus(LPBYTE p_data, DWORD len);
    void ProcessRelayStatus(LPBYTE p_data, DWORD len);
    void ProcessDefaultTtlStatus(LPBYTE p_data, DWORD len);
    void ProcessBeaconStatus(LPBYTE p_data, DWORD len);
    void ProcessModelPublicationStatus(LPBYTE p_data, DWORD len);
    void ProcessModelSubscriptionStatus(LPBYTE p_data, DWORD len);
    void ProcessModelSubscriptionList(LPBYTE p_data, DWORD len);
    void ProcessNetKeyStatus(LPBYTE p_data, DWORD len);
    void ProcessNetKeyList(LPBYTE p_data, DWORD len);
    void ProcessAppKeyStatus(LPBYTE p_data, DWORD len);
    void ProcessAppKeyList(LPBYTE p_data, DWORD len);
    void ProcessModelAppStatus(LPBYTE p_data, DWORD len);
    void ProcessModelAppList(LPBYTE p_data, DWORD len);
    void ProcessNodeIdentityStatus(LPBYTE p_data, DWORD len);
    void ProcessHearbeatSubscriptionStatus(LPBYTE p_data, DWORD len);
    void ProcessHearbeatPublicationStatus(LPBYTE p_data, DWORD len);
    void ProcessNetworkTransmitParamsStatus(LPBYTE p_data, DWORD len);
    void ProcessHealthCurrentStatus(LPBYTE p_data, DWORD len);
    void ProcessHealthFaultStatus(LPBYTE p_data, DWORD len);
    void ProcessHealthPeriodStatus(LPBYTE p_data, DWORD len);
    void ProcessHealthAttentionStatus(LPBYTE p_data, DWORD len);
    void ProcessLpnPollTimeoutStatus(LPBYTE p_data, DWORD len);
    void ProcessProxyFilterStatus(LPBYTE p_data, DWORD len);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONFIGURATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    int GetBaudRateSelection();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedHeartbeatSubscriptionSet();
    afx_msg void OnCbnSelchangeComPort();
    virtual BOOL OnSetActive();
    afx_msg void OnBnClickedHeartbeatPublicationSet();
    afx_msg void OnBnClickedNetworkTransmitIntervalGet();
    afx_msg void OnBnClickedNetworkTransmitIntervalSet();
    afx_msg void OnBnClickedNodeReset();
    afx_msg void OnBnClickedConfigDataGet();
    afx_msg void OnBnClickedBeaconGet();
    afx_msg void OnBnClickedBeaconSet();
    afx_msg void OnBnClickedDefaultTtlGet();
    afx_msg void OnBnClickedDefaultTtlSet();
    afx_msg void OnBnClickedGattProxyGet();
    afx_msg void OnBnClickedGattProxySet();
    afx_msg void OnBnClickedRelayGet();
    afx_msg void OnBnClickedRelaySet();
    afx_msg void OnBnClickedFriendGet();
    afx_msg void OnBnClickedFriendSet();
    afx_msg void OnBnClickedModelPubSet();
    afx_msg void OnBnClickedModelPubGet();
    afx_msg void OnBnClickedModelSubscriptionAdd();
    afx_msg void OnBnClickedModelSubscriptionDelete();
    afx_msg void OnBnClickedModelSubscriptionOverwrite();
    afx_msg void OnBnClickedModelSubscriptionDeleteAll();
    afx_msg void OnBnClickedSubscriptionGet();
    afx_msg void OnBnClickedNetkeyAdd();
    afx_msg void OnBnClickedNetkeyDelete();
    afx_msg void OnBnClickedNetkeyUpdate();
    afx_msg void OnBnClickedNetkeyGet();
    afx_msg void OnBnClickedAppkeyAdd();
    afx_msg void OnBnClickedAppkeyDelete();
    afx_msg void OnBnClickedAppkeyUpdate();
    afx_msg void OnBnClickedAppkeyGet();
    afx_msg void OnBnClickedModelAppBind();
    afx_msg void OnBnClickedModelAppUnbind();
    afx_msg void OnBnClickedModelAppGet();
    afx_msg void OnBnClickedNodeIdentityGet();
    afx_msg void OnBnClickedNodeIdentitySet();
    afx_msg void OnBnClickedHealthFaultGet();
    afx_msg void OnBnClickedHealthFaultClear();
    afx_msg void OnBnClickedHealthFaultTest();
    afx_msg void OnBnClickedHealthPeriodGet();
    afx_msg void OnBnClickedHealthPeriodSet();
    afx_msg void OnBnClickedHealthAttentionGet();
    afx_msg void OnBnClickedHealthAttentionSet();
    afx_msg void OnBnClickedProvisionConnect();
    afx_msg void OnBnClickedLocalSet();
    afx_msg void OnBnClickedScanUnprovisioned();
    afx_msg void OnBnClickedProvisionDisconnect();
    afx_msg void OnBnClickedProvisionStart();
    afx_msg void OnCbnSelchangeAuthMethod();
    afx_msg void OnBnClickedHeartbeatPublicationGet();
    afx_msg void OnBnClickedHeartbeatSubscriptionGet();
    afx_msg void OnBnClickedLpnPollTimeoutGet();
    afx_msg void OnBnClickedKeyRefreshPhaseGet();
    afx_msg void OnBnClickedKeyRefreshPhaseSet();
    afx_msg void OnBnClickedConnectProxy();
    afx_msg void OnBnClickedFindProxy();
    afx_msg void OnBnClickedDisconnectProxy();
    afx_msg void OnBnClickedProxyFilterTypeSet();
    afx_msg void OnBnClickedProxyFilterAddrAdd();
    afx_msg void OnBnClickedProxyFilterAddrDelete();
    afx_msg void OnBnClickedNodeGattConnect();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedScanExtended();
};

extern CClientControlApp theApp;
