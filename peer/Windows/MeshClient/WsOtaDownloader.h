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

/** @file
* WsOtaDownloader.h. : Header file
*
*/

#include "BtInterface.h"

#define POLYNOMIAL              0x04C11DB7
#define WIDTH                   (8 * sizeof(unsigned long))
#define TOPBIT                  (1 << (WIDTH - 1))
#define INITIAL_REMAINDER       0xFFFFFFFF
#define FINAL_XOR_VALUE         0xFFFFFFFF
#define REFLECT_DATA(X)         ((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)    ((unsigned long) reflect((X), WIDTH))
#define CHECK_VALUE             0xCBF43926

unsigned long crcSlow(unsigned long  crc32, unsigned char const message[], int nBytes);
unsigned long crcComplete(unsigned long crc32);

class WSDownloader
{
public:
    WSDownloader(CBtInterface *pBtInterface, LPBYTE pPatch, DWORD dwPatchSize, HWND hWnd);
    virtual ~WSDownloader();

    void Start();
    void ProcessEvent(BYTE Event);

    void TransferData();
    HANDLE m_hThread;

    enum
    {
        WS_UPGRADE_CONNECTED,
        WS_UPGRADE_RESPONSE_OK,
        WS_UPGRADE_CONTINUE,
        WS_UPGRADE_START_VERIFICATION,
        WS_UPGRADE_RESPONSE_FAILED,
        WS_UPGRADE_ABORT,
        WS_UPGRADE_DISCONNECTED
    } m_event;

    enum
    {
        WS_UPGRADE_STATE_IDLE,
        WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD,
        WS_UPGRADE_STATE_DATA_TRANSFER,
        WS_UPGRADE_STATE_VERIFICATION,
        WS_UPGRADE_STATE_VERIFIED,
        WS_UPGRADE_STATE_ABORTED,
    } m_state;

private:
    BOOL m_bConnected;
    HANDLE m_hEvent;
    HWND m_hWnd;
    DWORD m_offset;
    CBtInterface *m_btInterface;
    BYTE *m_Patch;
    DWORD m_PatchSize;
    DWORD m_crc32;
};
