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
#include "stdafx.h"
#include "ControlComm.h"
#include "hci_control_api.h"

extern void Log(WCHAR* _Format, ...);
extern "C" void ods(char* fmt_str, ...);

#define APP_UDP_PORT 9877
#define UDP_BUFFER_SIZE 1500

class ClsStopWatch
{
public:
    ClsStopWatch()
    {
    }

    void Start()
    {
        GetSystemTimeAsFileTime(&sft);

    }

    __int64 Stop()
    {
        GetSystemTimeAsFileTime(&eft);

        sli.LowPart = sft.dwLowDateTime;
        sli.HighPart = sft.dwHighDateTime;
        eli.LowPart = eft.dwLowDateTime;
        eli.HighPart = eft.dwHighDateTime;

        long long int dif = eli.QuadPart - sli.QuadPart;
        __int64 dftDuration = dif / 10000L;
        return dftDuration;
    }

private:
    FILETIME sft;
    FILETIME eft;
    SYSTEMTIME sst;
    SYSTEMTIME est;
    LARGE_INTEGER sli;
    LARGE_INTEGER eli;
};




DWORD WINAPI UDPServerReceiveThread(LPVOID lpdwThreadParam);

BOOL SetupUDPServer();

DWORD WINAPI UDPServerReceiveThread(LPVOID lpdwThreadParam);

void StopUDPServer();

BOOL SetupUDPClientSocket(char* pipstr);
/*
Function:   SendMessageToUDPServer
Parameters: p_msg - message buffer that needs to be send to the UDP Server
            len - length of the buffer
Usage:
            char fieldval[80];
            GetDlgItemTextA(m_hWnd, IDC_TC_NET_LEVEL_TRX_PDU, fieldval, sizeof(fieldval));
            SendMessageToUDPServer((char*)fieldval, strlen(fieldval));
*/
BOOL SendMessageToUDPServer(char* p_msg, UINT len);
