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
#include "stdafx.h"
#include <string>
using namespace std;

typedef enum _state
{
    STATE_IDLE,
    STATE_LOCAL_GET_COMPOSITION_DATA,
    STATE_LOCAL_ADD_APPLICATION_KEY,
    STATE_LOCAL_BIND_MODELS,
    STATE_REMOTE_GET_COMPOSITION_DATA,
    STATE_REMOTE_ADD_APPLICATION_KEY,
    STATE_REMOTE_BIND_MODELS
} MESH_STATE;

class CEventHandler
{
public:
    CEventHandler();
    virtual void ProcessData(DWORD opcode, LPBYTE p_data, DWORD len);
    void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length);

protected:
    SOCKET log_sock = INVALID_SOCKET;

private:
    MESH_STATE m_state;
    string p_local_composition_data;
    string p_remote_composition_data;
    static int initialized;
//    void ProcessCoreProvisionEnd(LPBYTE p_data, DWORD len);
//    void ProcessDescriptorStatus(LPBYTE p_data, DWORD len);
    void ProcessScanCapabilitiesStatus(LPBYTE p_data, DWORD len);
    void ProcessScanStatus(LPBYTE p_data, DWORD len);
    void ProcessScanReport(LPBYTE p_data, DWORD len);
    void ProcessProxyDeviceNetworkData(LPBYTE p_data, DWORD len);
    void ProcessProvisionEnd(LPBYTE p_data, DWORD len);
    void ProcessProvisionLinkStatus(LPBYTE p_data, DWORD len);
    void ProcessProvisionDeviceCapabilities(LPBYTE p_data, DWORD len);
    void ProcessProvisionOobDataRequest(LPBYTE p_data, DWORD len);
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
};
