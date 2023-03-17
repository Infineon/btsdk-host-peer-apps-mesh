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
#include "stdafx.h"
#include "ControlComm.h"
#include "hci_control_api.h"

#include "MeshPerformance.h"

extern void Log(WCHAR* _Format, ...);

#define APP_UDP_PORT 9877
#define UDP_BUFFER_SIZE 1500

SOCKADDR_IN serveraddr;
SOCKET app_server_sock = INVALID_SOCKET;
SOCKET app_client_socket = INVALID_SOCKET;

BOOL bstoprecvthread = FALSE;

CRITICAL_SECTION cs;
HANDLE m_hThreadRead;

DWORD WINAPI UDPServerReceiveThread(LPVOID lpdwThreadParam);

char localIPStr[30] = { 0 };

BOOL SetupUDPServer()
{
    InitializeCriticalSection(&cs);

    setvbuf(stdout, NULL, _IONBF, 0);

    // create thread to read the uart data.
    DWORD dwThreadId;
    m_hThreadRead = CreateThread(NULL, 0, UDPServerReceiveThread, 0, 0, &dwThreadId);
    if (!m_hThreadRead)
    {
        Log(L"Could not create UDP server thread\n");

    }

    return 0;
}

extern "C" void ods(char* fmt_str, ...);

DWORD WINAPI UDPServerReceiveThread(LPVOID lpdwThreadParam)
{
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (err != 0)
        return -3;
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Hello world!\n");

    ods("Set UDP App UDPServerReceiveThread.");


    SOCKADDR_IN saExt;
    /* Open the read and write sockets */
    app_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (app_server_sock == INVALID_SOCKET)
    {
        printf("Create UDP receive socket failed\n");
        return (0);
    }

    // Set socket receive buffer size
    int buf_size = UDP_BUFFER_SIZE;
    err = setsockopt(app_server_sock, SOL_SOCKET, SO_RCVBUF, (CHAR*)&buf_size, sizeof(int));
    if (err == SOCKET_ERROR)
    {
        printf("Set UDP App socket rcv buff size failed.");
        return (0);
    }

    memset(&saExt, 0, sizeof(SOCKADDR_IN));

    hostent* localHost;
    char* localIP;
    // Get the local host information
    localHost = gethostbyname("");
    localIP = inet_ntoa(*(struct in_addr*) * localHost->h_addr_list);

    printf("Local IP used for server: %s\n", localIP);

    strcpy(localIPStr, localIP);

    printf("Local IP used for server: %s\n", localIPStr);


    // Set up the sockaddr structure
    saExt.sin_family = AF_INET;
    saExt.sin_addr.s_addr = inet_addr(localIP);
    saExt.sin_port = htons(APP_UDP_PORT);

    err = bind(app_server_sock, (SOCKADDR*)&saExt, sizeof(SOCKADDR_IN));
    if (err == SOCKET_ERROR)
    {
        printf("UDP App socket bind failed. WSAGetLastError() gets %d.", WSAGetLastError());
        return (0);
    }
    printf("Listening on UDP port: %d\n", APP_UDP_PORT);

    while (!bstoprecvthread)
    {
        int bytes_rcvd;
        BYTE buff[1500];

        memset(buff, 0, sizeof(buff));
        bytes_rcvd = recv(app_server_sock, (char*)buff, UDP_BUFFER_SIZE, 0);

        if (bytes_rcvd > 0)
        {
            printf("Server received: Str is %s len: %d\n", buff, bytes_rcvd);
        }
    }
    return 0;
}

void StopUDPServer()
{
    EnterCriticalSection(&cs);
    bstoprecvthread = TRUE;
    LeaveCriticalSection(&cs);
}

BOOL SetupUDPClientSocket(char* pipstr)
{
    int ip[4];

    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (err != 0)
        return -3;

    /* socket: create the socket */
    app_client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (app_client_socket < 0)
    {
        printf("ERROR opening socket");
        return 0;
    }

    /* build the server's Internet address */
    int i = sscanf_s(pipstr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    if (i != 4)
    {
        printf("Error parsing the IP address\n");
        return 0;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = (ip[3] << 24) + (ip[2] << 16) + (ip[1] << 8) + ip[0];
    serveraddr.sin_port = htons(APP_UDP_PORT);

    return 0;
}

/*
Function: SendMessageToUDPServer
Parameters: p_msg - message buffer that needs to be send to the UDP Server
len - length of the buffer
Usage:
char fieldval[80];
GetDlgItemTextA(m_hWnd, IDC_TC_NET_LEVEL_TRX_PDU, fieldval, sizeof(fieldval));
SendMessageToUDPServer((char*)fieldval, strlen(fieldval));
*/
BOOL SendMessageToUDPServer(char* p_msg, UINT len)
{
    return 0;

    int serverlen, n;
    /* send the message to the server */
    printf("Str is %s len: %d", p_msg, len);
    serverlen = sizeof(serveraddr);

    n = sendto(app_client_socket, p_msg, len, 0, (const sockaddr*)&serveraddr, sizeof(SOCKADDR_IN));
    if (n == SOCKET_ERROR) {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        return 0;
    }

    return 0;
}
