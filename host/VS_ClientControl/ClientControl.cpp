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

// ClientControl.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "stdint.h"
//#include "wiced.h"
#include "ClientControl.h"
#include "ClientControlDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// This flag controls the availability of Mesh Performance Testing Feature
#define MESH_PERFORMANCE_TESTING_ENABLED     TRUE

extern "C" CRITICAL_SECTION cs;
CRITICAL_SECTION cs;

int as32BaudRate[4] =
{
    115200,
    921600,
    3000000,
    4000000
};

int aComPorts[128] = { 0 };
int ComPortSelected = -1;
int BaudRateSelected = -1;

#if 0
uint64_t TickCountInitValue;
#endif

extern "C" void ods(char* fmt_str, ...);

int host_mode_instance = 0;

/////////////////////////////////////////////////////////////////////////////
// CMeshCommandLineInfo


class CMeshCommandLineInfo : public CCommandLineInfo
{
public:
    CMeshCommandLineInfo();

    // Overrides
    void ParseParam(LPCTSTR lpszParam, BOOL bSwitch, BOOL bLast);

    // Implementation
    BOOL        m_bIsUDPClient;
    BOOL        m_bIsUDPServer;
    BOOL        m_bPerfTestMode;
    BOOL        m_bLogToFile;
    BOOL        m_bInstance;
    CString     sIPAddr; //IP Addr

};

CMeshCommandLineInfo::CMeshCommandLineInfo() : CCommandLineInfo()
{
    m_bIsUDPClient = FALSE;
    m_bIsUDPServer = FALSE;
    m_bPerfTestMode = MESH_PERFORMANCE_TESTING_ENABLED;
    m_bLogToFile = FALSE;
    m_bInstance = 0;
    sIPAddr.Empty();
}

/**************************************************
 command line
 /c IP Address      Start UDP Client and send messages to the provided IP Address of the server
 /s                 Start UDP Server
**************************************************/
void CMeshCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bSwitch, BOOL bLast)
{
    CString csParam(lpszParam);
    csParam.MakeUpper();

    if (bSwitch)
    {
        switch (*csParam.Left(1).GetBuffer())
        {
        case 'S': // UDP Server
            m_bIsUDPServer = TRUE;
            break;

        case 'C': //UDP Client
            m_bIsUDPClient = TRUE;
            break;

        case 'P': //Mesh Performance Testing
            m_bPerfTestMode = TRUE;
            break;

        case 'T': //Enable logging to file
            m_bLogToFile = TRUE;
            break;

        case 'I':
            m_bInstance = TRUE;
            break;
        }

    }
    else
    {
        if (m_bIsUDPClient)
        {
            sIPAddr = csParam.Right(csParam.GetLength());
            ods("IP Address:%S", &lpszParam[0]);
            ods("IP Address2:%S", sIPAddr.GetBuffer());
        }
        else if (m_bInstance)
        {
            host_mode_instance = _wtoi((const WCHAR*) &lpszParam[0]);
            if (host_mode_instance > 2 || host_mode_instance < 0)
                host_mode_instance = 0;
        }
    }
}

extern BOOL SetupUDPServer();
extern BOOL SetupUDPClientSocket(char* pipstr);
extern char localIPStr[];

extern char log_filename[];

void EnablePrintfAtMFC()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
    }
}

// CClientControlApp

BEGIN_MESSAGE_MAP( CClientControlApp, CWinApp )
    ON_COMMAND( ID_HELP, &CWinApp::OnHelp )
END_MESSAGE_MAP( )


// CClientControlApp construction

CClientControlApp::CClientControlApp( )
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance

    bMeshPerfMode = FALSE;
}

// The one and only CClientControlApp object

CClientControlApp theApp;

// CClientControlApp initialization

BOOL CClientControlApp::InitInstance( )
{
    InitializeCriticalSection(&cs);
    // InitCommonControlsEx( ) is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof( InitCtrls );
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx( &InitCtrls );

    CWinApp::InitInstance( );

    // Create the shell manager, in case the dialog contains
    // any shell tree view or shell list view controls.
    CShellManager *pShellManager = new CShellManager;

    // Activate "Windows Native" visual manager for enabling themes in MFC controls
    CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) );

    EnablePrintfAtMFC();

    CMeshCommandLineInfo cmdInfo;
    ParseCommandLine( cmdInfo );

    if(cmdInfo.m_bIsUDPServer)
    {
        ods("We need to start the UDP Server");
        SetupUDPServer();
        Sleep(1000);
        ods("Starting local UDP Client\n");
        ods("Connecting to IP Address of UDP Server: %s", localIPStr);
        SetupUDPClientSocket(localIPStr);
    }

    if (cmdInfo.m_bIsUDPClient)
    {
        ods("Starting UDP Client\n");
        ods("IP Address3: %S", cmdInfo.sIPAddr.GetBuffer());
        char    ipaddr[30] = { 0 };
        sprintf_s(ipaddr, sizeof(ipaddr), "%S", cmdInfo.sIPAddr.GetBuffer());
        SetupUDPClientSocket(ipaddr);
    }

    if (cmdInfo.m_bPerfTestMode)
    {
        ods("Starting application in Mesh performance testing mode\n");
        bMeshPerfMode = TRUE;
    }

    if (cmdInfo.m_bLogToFile)
    {
        strcpy(log_filename, "trace.txt");  // if you add full path make sure that directory exists, otherwise it will crash
    }


#if  defined(MESH_CLIENTCONTROL_LOG_TO_FILE) && (MESH_CLIENTCONTROL_LOG_TO_FILE == 1)
    strcpy(log_filename,"trace.txt");  // if you add full path make sure that directory exists, otherwise it will crash
#endif

    // Change the registry key under which our settings are stored
    SetRegistryKey( _T( "Cypress" ) );

    srand((DWORD)time(NULL));

    memset(aComPorts, 0, sizeof(aComPorts));
    int numPorts = 0;
    for (int i = 1; i < 128; i++)
    {
        WCHAR buf[20];
        wsprintf(buf, L"\\\\.\\COM%d", i);
        HANDLE handle = CreateFile(buf, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            aComPorts[numPorts++] = i;
        }
    }

    aComPorts[numPorts++] = 0; // For Host-Mode

#if 0
    TickCountInitValue = GetTickCount64() / 1000;
#endif

    CClientDialog dlg(_T("Mesh Client Control"));
    m_pMainWnd = &dlg;
    dlg.DoModal();

    // Delete the shell manager created above.
    if ( pShellManager != NULL )
    {
        delete pShellManager;
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

IMPLEMENT_DYNAMIC(CClientDialog, CPropertySheet)

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
#include "mesh_automation.h"
CSocketWindow *pSocketWin = NULL;
#endif

CClientDialog::CClientDialog(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    m_psh.dwFlags &= ~PSH_HASHELP;
#ifndef NO_LIGHT_CONTROL
    AddPage(&pageLight);
#endif
    AddPage(&pageMain);
    AddPage(&pageConfig);
    // Add Tab for Mesh Performance Testing only if enabled via commandline
    if (theApp.bMeshPerfMode)
        AddPage(&pageMeshPerf);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    if (pSocketWin == NULL)
        pSocketWin = new CSocketWindow();
#endif
}

CClientDialog::CClientDialog(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    m_psh.dwFlags &= ~PSH_HASHELP;
#ifndef NO_LIGHT_CONTROL
    AddPage(&pageLight);
#endif
    AddPage(&pageMain);
    AddPage(&pageConfig);

    // Add Tab for Mesh Performance Testing only if enabled via commandline
    if (theApp.bMeshPerfMode)
        AddPage(&pageMeshPerf);

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    if (pSocketWin == NULL)
        pSocketWin = new CSocketWindow();
#endif
}

CClientDialog::~CClientDialog()
{
    DeleteCriticalSection(&cs);
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    if (pSocketWin != NULL)
    {
        pSocketWin->DestroyWindow();
        delete pSocketWin;
        pSocketWin = NULL;
    }
#endif
}

BOOL CClientDialog::OnInitDialog()
{
    BOOL ret =  CPropertySheet::OnInitDialog();
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    if (pSocketWin != NULL)
        pSocketWin->Create();
#endif
    return ret;
}

BEGIN_MESSAGE_MAP(CClientDialog, CPropertySheet)
    //{{AFX_MSG_MAP(COptions)
    //ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
    //ON_COMMAND(IDOK, OnOK)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#include "wiced_timer.h"

wiced_result_t wiced_init_timer(wiced_timer_t* p_timer, wiced_timer_callback_t TimerCb, TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_deinit_timer(wiced_timer_t* p)
{
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_start_timer(wiced_timer_t* wt, uint32_t timeout)
{
    return WICED_ERROR;
}

wiced_result_t wiced_stop_timer(wiced_timer_t* wt)
{
    return WICED_ERROR;
}

extern "C" void wiced_bt_mesh_remote_provisioning_connection_state_changed(uint16_t conn_id, uint8_t reason)
{
}

extern "C" void wiced_bt_mesh_gatt_client_connection_state_changed(uint16_t conn_id, uint16_t mtu)
{
}

extern "C" uint8_t wiced_hci_send(uint16_t opcode, uint8_t *p_buffer, uint16_t length)
{
    return ((uint16_t)m_ComHelper->SendWicedCommand(opcode, p_buffer, length) != 0);
}

/////////////////////////////////////////////////////////////////////////////

extern "C" HANDLE hGatewayEvent;
HANDLE hGatewayEvent;

extern "C" void mible_wiced_init(void)
{
    hGatewayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

extern "C" void mible_wiced_wait_event(uint32_t timeout)
{
    WaitForSingleObject(hGatewayEvent, timeout);
}

extern "C" void mible_wiced_set_event(void)
{
    SetEvent(hGatewayEvent);
}
