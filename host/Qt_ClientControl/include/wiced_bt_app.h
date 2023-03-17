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

 /** @file
 *
 *  Defines functions for using the AIROC MPAF Framework
 */
#ifndef WICED_BT_APP_H
#define WICED_BT_APP_H

#include "wiced.h"
//typedef UINT32 (*WICED_BT_GET_TIME_TO_SLEEP_FP)(void);
//typedef MPAF_STATUS (*WICED_BT_STACK_EVENT_HANDLER_CB)( const BTHCI_EVENT_HDR_t* event );

/*****************************************************************************
 *                                              Function Definitions
 ****************************************************************************/
//BOOL32 wiced_bt_app_memInit(void);
//void wiced_bt_app_install( MPAF_APP_ID app_id );
//MPAF_APP_ID wiced_get_app_id ( void );
//MPAF_SAP_HANDLE wiced_bt_app_get_mpaf_pf_handle( void );
//extern void wiced_bt_InstallIdleThreadCSAPoll(MPAF_CORE_VOID_CB callback);
//extern void wiced_bt_InstallGetTimeToSleep(WICED_BT_GET_TIME_TO_SLEEP_FP callback);


/*
* Enum type received on callabck
*/
enum
{
    /// Ready to send out an adv in the next few mS. App can change ADV data if required.
    /// Typically invoked about 2.5mS before te ADV. If there are other higher priority
    /// tasks or other events in the app thread event queue, this will be delayed.
    /// Notification is best effort.
    WICED_BT_ADV_NOTIFICATION_READY,

    /// Just completed transmitting an ADV packet.
    WICED_BT_ADV_NOTIFICATION_DONE
};

// TODO: Move this to appropriate file in wiced_bt.
// This function is to add just the "wiced_bt_" prefix to the function.
/*******************************************************************************
* Function: wiced_bt_notifyAdvPacketTransmissions
*
* Abstract: Allows the application to register a callback that will be invoked
*           just before an ADV is packet is about to be sent out and again,
*           immediately after.
*
* Input : clientCallback - Pointer to a function that will be invoked in application
*                          thread context with WICED_BT_ADV_NOTIFICATION_READY for
                           before ADV and WICED_BT_ADV_NOTIFICATION_DONE after ADV
                           packet is complete.
*         advanceNoticeInMicroSeconds - Number of microseconds before the ADV the
*                          notification is to be sent. Will be rounded down to
*                          the nearest 1.25mS. Has to be an even multiple of 625uS.
* Output: TRUE if feature available, FALSE otherwise.
*
*******************************************************************************/
wiced_bool_t wiced_bt_notifyAdvPacketTransmissions(void (*clientCallback)(uint8_t),
                                             uint32_t advanceNoticeInMicroSeconds);

#endif /* WICED_BT_APP_H*/
