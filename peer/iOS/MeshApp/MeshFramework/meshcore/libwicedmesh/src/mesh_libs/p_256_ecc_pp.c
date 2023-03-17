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
 * Simple pairing algorithms implementation
 */

#include "bt_target.h"
#include "wiced_bt_app_common.h"

#if SMP_LE_SC_INCLUDED == WICED_TRUE

#include "p_256_multprecision.h"
#include "p_256_ecc_pp.h"
//#include <stdlib.h>
#include <stdio.h>
//???#include "p_256_timer.h"
#include <stdlib.h>
#include <string.h>
//???#include "p_256_bt_rtos.h"

int nd;
EC curve;
EC curve_p256;

void InitPoint(Point *q)
{
    BT_MEMSET(q, 0, sizeof(Point));
}


void CopyPoint(Point *q, Point *p)
{
    BT_MEMCPY(q, p, sizeof(Point));
}


// q=2q
void ECC_Double(Point *q, Point *p, uint32_t keyLength)
{
    DWORD t1[KEY_LENGTH_DWORDS_P256], t2[KEY_LENGTH_DWORDS_P256], t3[KEY_LENGTH_DWORDS_P256];
    DWORD *x1, *x3, *y1, *y3, *z1, *z3;

    if(MP_isZero(p->z, keyLength))
    {
        MP_Init(q->z, keyLength);
        return;                                     // return infinity
    }
    x1=p->x; y1=p->y; z1=p->z;
    x3=q->x; y3=q->y; z3=q->z;

    MP_MersennsSquaMod(t1, z1, keyLength);          // t1=z1^2
    MP_SubMod(t2, x1, t1, keyLength);               // t2=x1-t1
    MP_AddMod(t1, x1, t1, keyLength);               // t1=x1+t1
    MP_MersennsMultMod(t2, t1, t2, keyLength);      // t2=t2*t1
    MP_LShiftMod(t3, t2, keyLength);
    MP_AddMod(t2, t3, t2, keyLength);               // t2=3t2

    MP_MersennsMultMod(z3, y1, z1, keyLength);      // z3=y1*z1
    MP_LShiftMod(z3, z3, keyLength);

    MP_MersennsSquaMod(y3, y1, keyLength);          // y3=y1^2
    MP_LShiftMod(y3, y3, keyLength);
    MP_MersennsMultMod(t3, y3, x1, keyLength);      // t3=y3*x1=x1*y1^2
    MP_LShiftMod(t3, t3, keyLength);
    MP_MersennsSquaMod(y3, y3, keyLength);          // y3=y3^2=y1^4
    MP_LShiftMod(y3, y3, keyLength);

    MP_MersennsSquaMod(x3, t2, keyLength);          // x3=t2^2
    MP_LShiftMod(t1, t3, keyLength);                // t1=2t3
    MP_SubMod(x3, x3, t1, keyLength);               // x3=x3-t1
    MP_SubMod(t1, t3, x3, keyLength);               // t1=t3-x3
    MP_MersennsMultMod(t1, t1, t2, keyLength);      // t1=t1*t2
    MP_SubMod(y3, t1, y3, keyLength);               // y3=t1-y3
}

// q=q+p,     zp must be 1
void ECC_Add(Point *r, Point *p, Point *q, uint32_t keyLength)
{
    DWORD t1[KEY_LENGTH_DWORDS_P256], t2[KEY_LENGTH_DWORDS_P256];
    DWORD *x1, *x2, *x3, *y1, *y2, *y3, *z1, *z2, *z3;

    x1=p->x; y1=p->y; z1=p->z;
    x2=q->x; y2=q->y; z2=q->z;
    x3=r->x; y3=r->y; z3=r->z;

    // if Q=infinity, return p
    if(MP_isZero(z2, keyLength))
    {
        CopyPoint(r, p);
        return;
    }
    // if P=infinity, return q
    if(MP_isZero(z1, keyLength))
    {
        CopyPoint(r, q);
        return;
    }

    MP_MersennsSquaMod(t1, z1, keyLength);      // t1=z1^2
    MP_MersennsMultMod(t2, z1, t1, keyLength);  // t2=t1*z1
    MP_MersennsMultMod(t1, x2, t1, keyLength);  // t1=t1*x2
    MP_MersennsMultMod(t2, y2, t2, keyLength);  // t2=t2*y2

    MP_SubMod(t1, t1, x1, keyLength);           // t1=t1-x1
    MP_SubMod(t2, t2, y1, keyLength);           // t2=t2-y1

    if(MP_isZero(t1, keyLength))
    {
        if(MP_isZero(t2, keyLength))
        {
            ECC_Double(r, q, keyLength) ;
            return;
        }
        else
        {
            MP_Init(z3, keyLength);
            return;                             // return infinity
        }
    }

    MP_MersennsMultMod(z3, z1, t1, keyLength);  // z3=z1*t1
    MP_MersennsSquaMod(y3, t1, keyLength);      // t3=t1^2
    MP_MersennsMultMod(z1, y3, t1, keyLength);  // t4=t3*t1
    MP_MersennsMultMod(y3, y3, x1, keyLength);  // t3=t3*x1
    MP_LShiftMod(t1, y3, keyLength);            // t1=2*t3
    MP_MersennsSquaMod(x3, t2, keyLength);      // x3=t2^2
    MP_SubMod(x3, x3, t1, keyLength);           // x3=x3-t1
    MP_SubMod(x3, x3, z1, keyLength);           // x3=x3-t4
    MP_SubMod(y3, y3, x3, keyLength);           // t3=t3-x3
    MP_MersennsMultMod(y3, y3, t2, keyLength);  // t3=t3*t2
    MP_MersennsMultMod(z1, z1, y1, keyLength);  // t4=t4*t1
    MP_SubMod(y3, y3, z1, keyLength);
}


int isBitOne(DWORD *n, int i)
{
//  return ( *(n+(i/DWORD_BITS)) & (DWORD)(1<<(i%DWORD_BITS)) );
    return (int)(n[i>>DWORD_BITS_SHIFT] & (DWORD)(1<<( i & 0x1F)));
}


// Computing the NAF of a positive integer
void ECC_NAF(uint8_t *naf, uint32_t *NumNAF, DWORD *k, uint32_t keyLength)
{
    uint32_t    sign;
    int i=0;
    int j;
    uint32_t  var;    // I added it LVE

//  while(MP_MostSignBits(k, keyLength)>=1)             // k>=1
    while((var = MP_MostSignBits(k, keyLength))>=1)     // k>=1 I replaced the previous line by this one LVE
    {
        if(k[0] & 0x01)                                 // k is odd
        {
            sign=(k[0]&0x03);                            // 1 or 3

            // k = k-naf[i]
            if(sign==1)
                k[0]=k[0]&0xFFFFFFFE;
            else
            {
                k[0]=k[0]+1;
                if(k[0]==0)                             //overflow
                {
                    j=1;
                    do
                    {
                        k[j]++;
                    }while(k[j++]==0);                  //overflow
                }
            }
        }
        else
            sign=0;

        MP_RShift(k, k, keyLength);

        naf[i / 4] |= (sign) << ((i % 4) * 2);

        i++;
    }

    *NumNAF=i;
}

// Binary NAF for point multiplication
void ECC_PM_B_NAF(Point *q, Point *p, DWORD *n, uint32_t keyLength)
{
    int i;
    uint32_t sign;
    uint8_t naf[256 / 4 +1];
    uint32_t NumNaf;
    Point minus_p;
    Point r;
    DWORD *modp;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        modp = curve_p256.p;
    }
    else
    {
        modp = curve.p;
    }

    InitPoint(&r);
    MP_Init(p->z, keyLength);
    p->z[0]=1;

    // initialization
    InitPoint(q);
    // -p
    MP_Copy(minus_p.x, p->x, keyLength);
    MP_Sub(minus_p.y, modp, p->y, keyLength);

    MP_Init(minus_p.z, keyLength);
    minus_p.z[0]=1;

    // NAF
    BT_MEMSET(naf, 0, sizeof(naf));
    ECC_NAF(naf, &NumNaf, n, keyLength);

    for(i=NumNaf-1; i>=0; i--)
    {
        CopyPoint(&r, q);
        ECC_Double(q, &r, keyLength);

        sign = (naf[i / 4] >> ((i % 4) * 2)) & 0x03;

        if(sign==1)
        {
            CopyPoint(&r, q);
            ECC_Add(q, &r, p, keyLength);
        }
        else if(sign == 3)
        {
            CopyPoint(&r, q);
            ECC_Add(q, &r, &minus_p, keyLength);
        }

    }


    MP_InvMod(minus_p.x, q->z, keyLength);

    MP_MersennsSquaMod(q->z, minus_p.x, keyLength);
    MP_MersennsMultMod(q->x, q->x, q->z, keyLength);

    MP_MersennsMultMod(q->z, q->z, minus_p.x, keyLength);

    MP_MersennsMultMod(q->y, q->y, q->z, keyLength);
}


#define OCTETS_PER_DIGIT    sizeof(unsigned int)
#define BITS_PER_DIGIT 32

int mpConvFromOctets(DWORD *a, int ndigits, const unsigned char *c, int nbytes)
/* Converts nbytes octets into big digit a of max size ndigits
   Returns actual number of digits set
*/
{
        int i;
        int j, k;
        unsigned int t;

//        for(i=0; i<6; i++)
//            a[i]=0;

        /* Read in octets, least significant first */
        /* i counts into big_d, j along c, and k is # bits to shift */
        for (i = 0, j = nbytes - 1; i < ndigits && j >= 0; i++)
        {
                t = 0;
                for (k = 0; j >= 0 && k < BITS_PER_DIGIT; j--, k += 8)
                        t |= ((unsigned int)c[j]) << k;
                a[i] = t;
        }

        return i;
}

int mpConvFromHex(DWORD *a, int ndigits, const char *s)
/* Convert a string in hexadecimal format to a big digit.
   Return actual number of digits set.
*/
{
        int newlen;
        int n;
        unsigned long t;
        unsigned char newdigits[24];
        int i, j;

//        mpSetZero(a, ndigits);
        for(i=0; i<ndigits; i++)
            a[i]=0;

        /* Create some temp storage for int values */
        n = (int)strlen(s);
        newlen = n / 2;       /* log(16)/log(256)=0.5 */
        memset(newdigits, 0, 24);

        /* Work through zero-terminated string */
        for (i = 0; s[i]; i++)
        {
                t = s[i];
                if ((t >= '0') && (t <= '9')) t = (t - '0');
                else if ((t >= 'a') && (t <= 'f')) t = (t - 'a' + 10);
                else if ((t >= 'A') && (t <= 'F')) t = (t - 'A' + 10);
                else continue;
                for (j = newlen; j > 0; j--)
                {
                        t += (unsigned long)newdigits[j-1] << 4;
                        newdigits[j-1] = (unsigned char)(t & 0xFF);
                        t >>= 8;
                }
        }

        /* Convert bytes to big digits */
        n = mpConvFromOctets(a, ndigits, newdigits, newlen);

        if(n<6)
        {
            for(i=n; i<6; i++)
                a[i]=0;
            n=6;
        }

        return n;
}

BOOL32 ecdsa_verify_(unsigned char *digest, unsigned char *signature, Point *key)
{
    // Dudley: Just a replacehold to avoid compiling issue, should not be called in this MeshApp client.
    return 0;
}

#endif
