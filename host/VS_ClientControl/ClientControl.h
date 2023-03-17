/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
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

// ClientControl.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "ControlComm.h"

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"        // main symbols

#if defined(MESH_AUTOMATION_ENABLED) && (MESH_AUTOMATION_ENABLED == TRUE)
#include "mesh_client_script.h"
#endif


// CClientControlApp:
// See ClientControl.cpp for the implementation of this class
//

class CClientControlApp : public CWinApp
{
public:
    CClientControlApp( );

// Overrides
public:
    virtual BOOL InitInstance( );

// Implementation

    DECLARE_MESSAGE_MAP( )

    public:
        BOOL bConnectedMesh;
        BOOL bMeshPerfMode;

};

typedef struct
{
    WCHAR* PropName;
    USHORT PropId;
    int len;
} LightLcProp;
extern LightLcProp lightLcProp[];
extern int numLightLcProps;

void WriteLog( const char * format ... );

extern CClientControlApp theApp;

extern int idxPageLight;
extern int idxPageMain;
extern int idxPageConfig;
extern int idxPageDirectedForwarding;
extern int idxPageConnectedMesh;
extern int idxPageMeshPerf;

extern ComHelper* m_ComHelper;
extern ComHelper* m_ComHelper2;
extern int ComPort;
extern int ComPort2;
extern int BaudRate;
extern int SendDataTime;
extern int AutoNodes;
extern int as32BaudRate[4];
extern int aComPorts[];
extern int ComPortSelected;
extern int BaudRateSelected;
extern BOOL bAuto;

BYTE ProcNibble(char n);
int FindBaudRateIndex(int baud);
DWORD GetHexValue(char *szbuf, LPBYTE buf, DWORD buf_size);
BYTE GetNumElements(BYTE *p_data, USHORT len);
void SendGetCompositionData(USHORT dst, BYTE page);
void SendAddAppKey(USHORT dst, USHORT net_key_idx, USHORT app_key_idx, BYTE *p_key);
void SendBind(USHORT dst, USHORT element_addr, ULONG model_id, USHORT app_key_idx);
void SendSetDeviceKey(USHORT dst);
int  GetModelElementIdx(USHORT company_id, USHORT model_id, BYTE *p_composition_data, USHORT composition_data_len);
#define COMPOSITION_DATA_TYPE_HSL       1
#define COMPOSITION_DATA_TYPE_CTL       2
#define COMPOSITION_DATA_TYPE_XYL       3
#define COMPOSITION_DATA_TYPE_DIMMABLE  4
#define COMPOSITION_DATA_TYPE_ONOFF     5
BOOL CheckCompositionData(BYTE type, BYTE *p_composition_data, USHORT len);
