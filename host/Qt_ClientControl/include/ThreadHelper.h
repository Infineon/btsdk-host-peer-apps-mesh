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
//#ifdef __linux__
#if defined(__linux__) || defined(__mac__)
#include <pthread.h>
#include <memory.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define WINAPI
#define INVALID_HANDLE_VALUE (-1)
#define FALSE   0
#define BOOL    bool
#define TRUE    true
typedef void *LPVOID;
typedef unsigned int DWORD;

#define Log(a)
class CAutoLock
{
public:
    CAutoLock(pthread_mutex_t* lock) :
        m_lock(lock)
    {
        pthread_mutex_lock(lock);
    }
    ~CAutoLock()
    {
        pthread_mutex_unlock(m_lock);
    }
private:
    pthread_mutex_t* m_lock;
};

class SyncObjectPosix
{
private:

    bool signalled;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

public:

    SyncObjectPosix()
    {
        signalled = false;
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~SyncObjectPosix()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    bool WaitForSignal(unsigned long timeout)
    {
        struct timespec now;
        memset(&now, 0, sizeof(now));
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_nsec += (timeout % 1000) * 1000000;

        CAutoLock lock(&mutex);
        while (!signalled)
        {
            int ret = pthread_cond_timedwait(&cond, &mutex, &now);
            if (timeout != 0 && ret == ETIMEDOUT)
            {
                return false;
            }
        }
        signalled = false;
        return TRUE;
    }

    void Signal()
    {
        CAutoLock lock(&mutex);
        signalled = true;
        pthread_cond_signal(&cond);
    }
};
#endif

class ThreadHelper;

typedef struct _param {
    ThreadHelper* pThreadHelper;
    LPVOID pThreadparm;
} ThreadParm;


class ThreadHelper
{
public:
    ThreadHelper();
    virtual ~ThreadHelper();

    BOOL CreateThread(LPVOID threadparm);
    BOOL Close();
protected:
    virtual BOOL Begin();
    virtual BOOL Process();
    virtual BOOL CleanUp();
private:
    static ThreadParm s_ThreadParm;
    static LPVOID ThreadProc(LPVOID lpdwThreadParam);
    DWORD Worker(LPVOID pThreadparam);
    pthread_t m_hThread;
protected:
    SyncObjectPosix m_hShutdown;
    BOOL m_bClosing;
};
