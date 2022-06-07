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
#include "ThreadHelper.h"
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
typedef unsigned char UINT8;
typedef unsigned char BYTE;
typedef unsigned short UINT16;
typedef unsigned char *LPBYTE;
typedef int SOCKET;
#endif
//**************************************************************************************************
//*** Definitions for Socket Comm
//**************************************************************************************************
typedef struct
{
    UINT8   pkt_type;
    UINT8   data[1200];
} tSCRIPT_PKT;

//
// Serial Bus class, use this class to read/write from/to the serial bus device
//
class SocketHelper : public ThreadHelper
{
public:
    static int SOCK_PORT_NUM;
    SocketHelper();
    virtual ~SocketHelper();
    static SocketHelper* GetInstance();

    // oopen serialbus driver to access device
    void SetOpenPort(int port);
    void sendToHost(UINT16 type, LPBYTE pData, DWORD len);
protected:
    virtual BOOL Begin();
    virtual BOOL CleanUp();
    virtual BOOL Process();
private:
    static SocketHelper _socketHelper;
    // read data from device
    DWORD Read(LPBYTE b, DWORD dwLen);
    DWORD ReadNewHciPacket(BYTE* pu8Buffer, int bufLen, int * pOffset);
    DWORD Write(LPBYTE b, DWORD dwLen);
    int CallWicedHciApi(UINT8* data, UINT8 len);
    int read_script_pct(char *pPkt);
    DWORD hci_control_proc_rx_cmd(BYTE* p_buffer, DWORD length);
public:
    // write data to device
    DWORD SendWicedCommand(UINT16 command, LPBYTE b, DWORD dwLen);

    BOOL IsOpened();

private:
    int m_nPortNumber;
    SOCKET  m_ListenSocket;
    SOCKET  m_ClientSocket;
};
