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
* simple pairing algorithms implementation
*/

#include "bt_target.h"
#include "wiced_bt_app_common.h"

#if SMP_LE_SC_INCLUDED == WICED_TRUE

#include <string.h>
#include "p_256_multprecision.h"
#include "p_256_ecc_pp.h"

void MP_Init(DWORD *c, uint32_t keyLength)
{
    uint32_t i;

    for(i=0; i<keyLength; i++)
        c[i]=0;
}

void MP_Copy(DWORD *c, DWORD *a, uint32_t keyLength)
{
    uint32_t i;

    for(i=0; i<keyLength; i++)
        c[i]=a[i];
}

int MP_CMP(DWORD *a, DWORD *b, uint32_t keyLength)
{
    int i;

    for(i=keyLength-1; i>=0; i--)
    {
        if(a[i]>b[i])
            return 1;
        if(a[i]<b[i])
            return -1;
    }
    return 0;
}

int MP_isZero(DWORD *a, uint32_t keyLength)
{
    uint32_t i;

    for(i=0; i<keyLength; i++)
        if(a[i])
            return 0;

    return 1;
}

uint32_t MP_DWORDBits (DWORD a)
{
    int i;

    for (i = 0; i < DWORD_BITS; i++, a >>= 1)
        if (a == 0)
            break;

    return i;
}

uint32_t MP_MostSignDWORDs(DWORD *a, uint32_t keyLength)
{
    int i;

    for(i=keyLength-1; i>=0; i--)
        if(a[i])
            break;
    return (i+1);
}

uint32_t MP_MostSignBits(DWORD *a, uint32_t keyLength)
{
    int aMostSignDWORDs;

    aMostSignDWORDs=MP_MostSignDWORDs(a, keyLength);
    if(aMostSignDWORDs==0)
        return 0;

    return ( ((aMostSignDWORDs-1)<<DWORD_BITS_SHIFT)+MP_DWORDBits(a[aMostSignDWORDs-1]) );
}

DWORD MP_Add(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    DWORD carrier;
    uint32_t i;
    DWORD temp;

    carrier=0;
    for(i=0; i<keyLength; i++)
    {
        temp=a[i]+carrier;
        carrier = (temp<carrier);
        temp += b[i];
        carrier |= (temp<b[i]);
        c[i]=temp;
    }

    return carrier;
}

//c=a-b
DWORD MP_Sub(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    DWORD borrow;
    uint32_t i;
    DWORD temp;

    borrow=0;
    for(i=0; i<keyLength; i++)
    {
        temp=a[i]-borrow;
        borrow=(temp>a[i]);
        c[i] = temp-b[i];
        borrow |= (c[i]>temp);
    }

    return borrow;
}

// c = a << 1
void MP_LShiftMod(DWORD * c, DWORD * a, uint32_t keyLength)
{
    DWORD carrier;
    DWORD *modp;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        modp = curve_p256.p;
    }
    else
    {
        modp = curve.p;
    }

    carrier=MP_LShift(c, a, keyLength);
    if(carrier)
        MP_Sub(c, c, modp, keyLength);
    else if(MP_CMP(c, modp, keyLength)>=0)
        MP_Sub(c, c, modp, keyLength);
}

// c=a>>1
void MP_RShift(DWORD * c, DWORD * a, uint32_t keyLength)
{
    DWORD carrier;
    int i, j;
    DWORD temp;
    DWORD b = 1;

    j=DWORD_BITS-b;
    carrier=0;

    for(i=keyLength-1; i>=0; i--)
    {
        temp=a[i];                  // in case of c==a
        c[i]=(temp>>b) | carrier;
        carrier = temp << j;
    }
}

// Curve specific optimization when p is a pseudo-Mersenns prime, p=2^(KEY_LENGTH_BITS)-omega
void MP_MersennsMultMod(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    DWORD cc[2*KEY_LENGTH_DWORDS_P256];

    MP_Mult(cc, a, b, keyLength);

    if(keyLength == 6)
        MP_FastMod(c, cc);
    else if(keyLength == 8)
        MP_FastMod_P256(c, cc);
}



// Curve specific optimization when p is a pseudo-Mersenns prime
void MP_MersennsSquaMod(DWORD *c, DWORD *a, uint32_t keyLength)
{
    MP_MersennsMultMod(c, a, a, keyLength);
}

// c=(a+b) mod p, b<p, a<p
void MP_AddMod(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    DWORD carrier;
    DWORD *modp;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        modp = curve_p256.p;
    }
    else
    {
        modp = curve.p;
    }

    carrier=MP_Add(c, a, b, keyLength);
    if(carrier)
        MP_Sub(c, c, modp, keyLength);
    else if(MP_CMP(c, modp, keyLength)>=0)
        MP_Sub(c, c, modp, keyLength);
}

// c=(a-b) mod p, a<p, b<p
void MP_SubMod(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    DWORD borrow;
    DWORD *modp;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        modp = curve_p256.p;
    }
    else
    {
        modp = curve.p;
    }

    borrow=MP_Sub(c, a, b, keyLength);
    if(borrow)
        MP_Add(c, c, modp, keyLength);
}

// c=a<<b, b<DWORD_BITS, c has a buffer size of NumDWORDs+1
DWORD MP_LShift(DWORD * c, DWORD * a, uint32_t keyLength)
{
    DWORD carrier;
    uint32_t i;
    int j;
    DWORD temp;
    uint32_t b = 1;

    j=DWORD_BITS-b;
    carrier=0;

    for(i=0; i<keyLength; i++)
    {
        temp=a[i];                              // in case c==a
        c[i]=(temp<<b) | carrier;
        carrier = temp >> j;
    }

    return carrier;
}

// c=a*b; c must have a buffer of 2*Key_LENGTH_DWORDS, c != a != b
void MP_Mult(DWORD *c, DWORD *a, DWORD *b, uint32_t keyLength)
{
    uint32_t i, j;
    DWORD W, U, V;

    U = V = W=0;
    MP_Init(c, keyLength);

    //assume little endian right now
    for(i=0; i<keyLength; i++)
    {
        U = 0;
        for(j=0; j<keyLength; j++)
        {
            {
                UINT64 result;
                result = ((UINT64)a[i]) * ((UINT64)b[j]);
                W = result >> 32;
            }

            V=a[i]*b[j];

            V=V+U;
            U=(V<U);
            U+=W;

            V = V+c[i+j];
            U += (V<c[i+j]);

            c[i+j] = V;
        }
        c[i+keyLength]=U;
    }
}


void MP_FastMod(DWORD *c, DWORD *a)
{
    DWORD U, V;
    DWORD *modp = curve.p;

    c[0]=a[0]+a[6];
    U=c[0]<a[0];
    c[0]+=a[10];
    U+=c[0]<a[10];

    c[1]=a[1]+U;
    U=c[1]<a[1];
    c[1]+=a[7];
    U+=c[1]<a[7];
    c[1]+=a[11];
    U+=c[1]<a[11];

    c[2]=a[2]+U;
    U=c[2]<a[2];
    c[2]+=a[6];
    U+=c[2]<a[6];
    c[2]+=a[8];
    U+=c[2]<a[8];
    c[2]+=a[10];
    U+=c[2]<a[10];

    c[3]=a[3]+U;
    U=c[3]<a[3];
    c[3]+=a[7];
    U+=c[3]<a[7];
    c[3]+=a[9];
    U+=c[3]<a[9];
    c[3]+=a[11];
    U+=c[3]<a[11];

    c[4]=a[4]+U;
    U=c[4]<a[4];
    c[4]+=a[8];
    U+=c[4]<a[8];
    c[4]+=a[10];
    U+=c[4]<a[10];

    c[5]=a[5]+U;
    U=c[5]<a[5];
    c[5]+=a[9];
    U+=c[5]<a[9];
    c[5]+=a[11];
    U+=c[5]<a[11];

    c[0]+=U;
    V=c[0]<U;
    c[1]+=V;
    V=c[1]<V;
    c[2]+=V;
    V=c[2]<V;
    c[2]+=U;
    V=c[2]<U;
    c[3]+=V;
    V=c[3]<V;
    c[4]+=V;
    V=c[4]<V;
    c[5]+=V;
    V=c[5]<V;

    if(V)
        MP_Sub(c, c, modp, KEY_LENGTH_DWORDS_P192);
    else if(MP_CMP(c, modp, KEY_LENGTH_DWORDS_P192)>=0)
        MP_Sub(c, c, modp, KEY_LENGTH_DWORDS_P192);
}

void MP_FastMod_P256(DWORD *c, DWORD *a)
{
    DWORD A, B, C, D, E, F, G;
    uint8_t UA, UB, UC, UD, UE, UF, UG;
    DWORD U;
    DWORD *modp = curve_p256.p;

    // C = a[13] + a[14] + a[15];
    C = a[13];
    C += a[14];
    UC = (C < a[14]);
    C += a[15];
    UC += (C < a[15]);

    // E = a[8] + a[9];
    E = a[8];
    E += a[9];
    UE = (E < a[9]);

    // F = a[9] + a[10];
    F = a[9];
    F += a[10];
    UF = (F < a[10]);

    // G = a[10] + a[11]
    G = a[10];
    G += a[11];
    UG = (G < a[11]);

    // B = a[12] + a[13] + a[14] + a[15] == C + a[12]
    B = C;
    UB = UC;
    B += a[12];
    UB += (B < a[12]);

    // A = a[11] + a[12] + a[13] + a[14] == B + a[11] - a[15]
    A = B;
    UA = UB;
    A += a[11];
    UA += (A < a[11]);
    UA -= (A < a[15]);
    A -= a[15];

    // D = a[10] + a[11] + a[12] + a[13] == A + a[10] - a[14]
    D = A;
    UD = UA;
    D += a[10];
    UD += (D < a[10]);
    UD -= (D < a[14]);
    D -= a[14];

    c[0] = a[0];
    c[0] += E;
    U = (c[0] < E);
    U += UE;
    U -= (c[0] < A);
    U -= UA;
    c[0] -= A;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[1] < UU);
        c[1] = a[1] - UU;
    }
    else
    {
        c[1] = a[1] + U;
        U = (c[1] < a[1]);
    }

    c[1] += F;
    U += (c[1] < F);
    U += UF;
    U -= (c[1] < B);
    U -= UB;
    c[1] -= B;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[2] < UU);
        c[2] = a[2] - UU;
    }
    else
    {
        c[2] = a[2] + U;
        U = (c[2] < a[2]);
    }

    c[2] += G;
    U += (c[2] < G);
    U += UG;
    U -= (c[2] < C);
    U -= UC;
    c[2] -= C;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[3] < UU);
        c[3] = a[3] - UU;
    }
    else
    {
        c[3] = a[3] + U;
        U = (c[3] < a[3]);
    }

    c[3] += A;
    U += (c[3] < A);
    U += UA;
    c[3] += a[11];
    U += (c[3] < a[11]);
    c[3] += a[12];
    U += (c[3] < a[12]);
    U -= (c[3] < a[14]);
    c[3] -= a[14];
    U -= (c[3] < a[15]);
    c[3] -= a[15];
    U -= (c[3] < E);
    U -= UE;
    c[3] -= E;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[4] < UU);
        c[4] = a[4] - UU;
    }
    else
    {
        c[4] = a[4] + U;
        U = (c[4] < a[4]);
    }

    c[4] += B;
    U += (c[4] < B);
    U += UB;
    U -= (c[4] < a[15]);
    c[4] -= a[15];
    c[4] += a[12];
    U += (c[4] < a[12]);
    c[4] += a[13];
    U += (c[4] < a[13]);
    U -= (c[4] < F);
    U -= UF;
    c[4] -= F;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[5] < UU);
        c[5] = a[5] - UU;
    }
    else
    {
        c[5] = a[5] + U;
        U = (c[5] < a[5]);
    }

    c[5] += C;
    U += (c[5] < C);
    U += UC;
    c[5] += a[13];
    U += (c[5] < a[13]);
    c[5] += a[14];
    U += (c[5] < a[14]);
    U -= (c[5] < G);
    U -= UG;
    c[5] -= G;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[6] < UU);
        c[6] = a[6] - UU;
    }
    else
    {
        c[6] = a[6] + U;
        U = (c[6] < a[6]);
    }

    c[6] += C;
    U += (c[6] < C);
    U += UC;
    c[6] += a[14];
    U += (c[6] < a[14]);
    c[6] += a[14];
    U += (c[6] < a[14]);
    c[6] += a[15];
    U += (c[6] < a[15]);
    U -= (c[6] < E);
    U -= UE;
    c[6] -= E;

    if(U & 0x80000000)
    {
        DWORD UU;
        UU = 0 - U;
        U = (a[7] < UU);
        c[7] = a[7] - UU;
    }
    else
    {
        c[7] = a[7] + U;
        U = (c[7] < a[7]);
    }

    c[7] += a[15];
    U += (c[7] < a[15]);
    c[7] += a[15];
    U += (c[7] < a[15]);
    c[7] += a[15];
    U += (c[7] < a[15]);
    c[7] += a[8];
    U += (c[7] < a[8]);
    U -= (c[7] < D);
    U -= UD;
    c[7] -= D;

    if(U & 0x80000000)
    {
        while(U)
        {
            MP_Add(c, c, modp, KEY_LENGTH_DWORDS_P256);
            U++;
        }
    }
    else if(U)
    {
        while(U)
        {
            MP_Sub(c, c, modp, KEY_LENGTH_DWORDS_P256);
            U--;
        }
    }

    if(MP_CMP(c, modp, KEY_LENGTH_DWORDS_P256)>=0)
        MP_Sub(c, c, modp, KEY_LENGTH_DWORDS_P256);

}

void MP_InvMod(DWORD *aminus, DWORD *u, uint32_t keyLength)
{
    DWORD v[KEY_LENGTH_DWORDS_P256];
    DWORD A[KEY_LENGTH_DWORDS_P256+1], C[KEY_LENGTH_DWORDS_P256+1];
    DWORD *modp;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        modp = curve_p256.p;
    }
    else
    {
        modp = curve.p;
    }

    MP_Copy(v, modp, keyLength);
    MP_Init(A, keyLength);
    MP_Init(C, keyLength);
    A[0]=1;

    while(!MP_isZero(u, keyLength))
    {
        while( !(u[0] & 0x01) )                 // u is even
        {
            MP_RShift(u, u, keyLength);
            if( !(A[0] & 0x01) )                // A is even
                MP_RShift(A, A, keyLength);
            else
            {
                A[keyLength]=MP_Add(A, A, modp, keyLength); // A =A+p
                MP_RShift(A, A, keyLength);
                A[keyLength-1] |= (A[keyLength]<<31);
            }
        }

        while( !(v[0] & 0x01) )                 // v is even
        {
            MP_RShift(v, v, keyLength);
            if( !(C[0] & 0x01) )                // C is even
                MP_RShift(C, C, keyLength);
            else
            {
                C[keyLength]=MP_Add(C, C, modp, keyLength); // C =C+p
                MP_RShift(C, C, keyLength);
                C[keyLength-1] |= (C[keyLength]<<31);
            }
        }

        if(MP_CMP(u, v, keyLength)>=0)
        {
            MP_Sub(u, u, v, keyLength);
            MP_SubMod(A, A, C, keyLength);
        }
        else
        {
            MP_Sub(v, v, u, keyLength);
            MP_SubMod(C, C, A, keyLength);
        }
    }

    if(MP_CMP(C, modp, keyLength)>=0)
        MP_Sub(aminus, C, modp, keyLength);
    else
        MP_Copy(aminus, C, keyLength);
}

#endif
