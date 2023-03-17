/*
* Copyright 2022-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
* WICED Timer implementation for QT applications
*/

#include "wiced_timer.h"

#include "qtwicedtimer.h"

QtWicedTimer::QtWicedTimer()
{
    // create a timer
    qtimer = new QTimer(this);

    // setup signal and slot
    connect(qtimer, SIGNAL(timeout()), this, SLOT(TimerSlot()));

    // init parameters
    timer_arg = NULL;
}

QtWicedTimer::~QtWicedTimer()
{
    // delete timer
    delete qtimer;
}

extern "C" void timer_execute(void *timer_arg);

void QtWicedTimer::TimerSlot()
{
    timer_execute(timer_arg);
}


typedef struct _tle
{
    wiced_timer_callback_t  p_cback;        /* timer expiration callback function */
    TIMER_PARAM_TYPE        arg;            /* parameter for expiration function */
    bool                    single_shot;    /* single shot or periodic timer */
    int                     mul;            /* time out value multiplier (to calculate milliseconds) */
    QtWicedTimer            *qt_timer;      /* QT timer class */
} TIMER_LIST_ENT;

void timer_execute(void *timer_arg)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)timer_arg;

    if (p && p->p_cback)
        p->p_cback(p->arg);
}

extern "C" wiced_result_t wiced_init_timer(wiced_timer_t *p_timer, wiced_timer_callback_t TimerCb, TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    memset(p_timer, 0, sizeof(TIMER_LIST_ENT));
    p->qt_timer = new QtWicedTimer();
    if (!p->qt_timer)
        return WICED_BT_ERROR;
    p->p_cback = TimerCb;
    p->arg = cBackparam;
    switch (type)
    {
    case WICED_SECONDS_TIMER:
        p->single_shot = true;
        p->mul = 1000;
        break;
    case WICED_MILLI_SECONDS_TIMER:
        p->single_shot = true;
        p->mul = 1;
        break;
    case WICED_SECONDS_PERIODIC_TIMER:
        p->single_shot = false;
        p->mul = 1000;
        break;
    case WICED_MILLI_SECONDS_PERIODIC_TIMER:
        p->single_shot = false;
        p->mul = 1;
        break;
    }
    p->qt_timer->timer_arg = p;

    return WICED_BT_SUCCESS;
}

extern "C" wiced_result_t wiced_deinit_timer(wiced_timer_t *p_timer)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    if (p->qt_timer)
    {
        delete p->qt_timer;
        p->qt_timer = NULL;
    }

    return WICED_BT_SUCCESS;
}

extern "C" wiced_result_t wiced_start_timer(wiced_timer_t *p_timer, uint32_t timeout)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    if (!p->qt_timer)
        return WICED_BT_ERROR;

    p->qt_timer->qtimer->setSingleShot(p->single_shot);
    p->qt_timer->qtimer->start(timeout * p->mul);

    return WICED_BT_SUCCESS;
}

extern "C" wiced_result_t wiced_stop_timer(wiced_timer_t *p_timer)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    if (!p->qt_timer)
        return WICED_BT_ERROR;

    p->qt_timer->qtimer->stop();

    return WICED_BT_SUCCESS;
}

extern "C" wiced_bool_t wiced_is_timer_in_use(wiced_timer_t *p_timer)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    if (!p->qt_timer)
        return WICED_FALSE;

    if (p->qt_timer->qtimer->isActive())
        return WICED_TRUE;
    else
        return WICED_FALSE;
}
