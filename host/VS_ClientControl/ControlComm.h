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
#ifndef CONTROL_COMM_H
#define CONTROL_COMM_H

//**************************************************************************************************
//*** Definitions for BTW Serial Bus
//**************************************************************************************************

// Helper class to print debug messages to Debug Console
class DebugHelper
{
public:
    void DebugOut() { OutputDebugStringA(m_Buffer); }
    void DebugOut(LPCSTR v) { OutputDebugStringA(v); }
    void DebugOut(LPCSTR fmt, LPCSTR v);
    void DebugOut(LPCSTR fmt, DWORD v1);
    void DebugOut(LPCSTR fmt, DWORD v1, DWORD v2);
    void PrintCommProp(COMMPROP& commProp);
    void PrintCommState(DCB& serial_config);

    static char m_Buffer[1024];
};

//
// Serial Bus class, use this class to read/write from/to the serial bus device
//
class ComHelper
{
public:
    ComHelper(HWND hWnd);
    virtual ~ComHelper();

    // oopen serialbus driver to access device
    virtual BOOL OpenPort(int port, int baudRate);
    virtual void ClosePort();

    // read data from device
    virtual DWORD Read(LPBYTE b, DWORD dwLen);
    virtual DWORD ReadNewHciPacket(BYTE* pu8Buffer, int bufLen, int* pOffset);
    virtual DWORD ReadWorker();

    // write data to device
    virtual DWORD SendWicedCommand(UINT16 command, LPBYTE b, DWORD dwLen);
    virtual DWORD Write(LPBYTE b, DWORD dwLen);

    virtual BOOL IsOpened();

protected:
    HWND m_hWnd;
    // overlap IO for Read and Write
    OVERLAPPED m_OverlapRead;
    OVERLAPPED m_OverlapWrite;
    HANDLE m_handle;
    HANDLE m_hThreadRead;
    HANDLE m_hShutdown;
    BOOL m_bClosing;
    BOOL m_CleanHciState;
};


//
// Use this class to read/write from/to a Windows Socket
//
class ComHelperHostMode : public ComHelper
{
public:
    ComHelperHostMode(HWND hWnd);
    virtual ~ComHelperHostMode();

    virtual BOOL OpenPort(int port, int baudRate);
    virtual void ClosePort();
    virtual DWORD Read(LPBYTE b, DWORD dwLen);
    virtual DWORD Write(LPBYTE b, DWORD dwLen);
    virtual BOOL IsOpened();

};

extern void DumpData(char* description, void* p, UINT32 length, UINT32 max_lines);

#endif
