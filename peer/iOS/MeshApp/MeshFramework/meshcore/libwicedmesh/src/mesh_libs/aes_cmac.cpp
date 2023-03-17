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
 * This C source is designed to generate the test vectors that appear in
 * this memo to verify correctness of the algorithm.  The source code is
 * not intended for use in commercial products.
 */

// #define AES_CMAC_UNIT_TEST

#include "aes_cmac.h"
#include "aes.h"

#ifdef AES_CMAC_UNIT_TEST
  /****************************************************************/
  /* AES-CMAC with AES-128 bit                                    */
  /* CMAC     Algorithm described in SP800-38B                    */
  /* Author: Junhyuk Song (junhyuk.song@samsung.com)              */
  /*         Jicheol Lee  (jicheol.lee@samsung.com)               */
  /****************************************************************/
  #include <stdio.h>
#endif

  /* For CMAC Calculation */
static  unsigned char const_Rb[16] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
  };

#ifdef AES_CMAC_UNIT_TEST
static  unsigned char const_Zero[16] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
#endif

#ifndef UINT8
typedef unsigned  char UINT8;

#endif

  /* Basic Functions */

#if 0
void AES_128(unsigned char *key, unsigned char *text, unsigned char *outText  )
{
    aes_encrypt_128( text, outText, key,key);
}
#else
static void AES_128(unsigned char *key, unsigned char *text, unsigned char *outText  )
{
    UINT8 k[16];
    UINT8 t[16];
    int i;

    for(i=0; i< 16;i++)
    {
#if 0
        // fill the key and text backward. The aes encrypt uses big endian.
        t[15-i] = text[i];
        k[15-i] = key[i];
#else
        t[i] = text[i];
        k[i] = key[i];
#endif
    }


    // software AES. This function is big endian.
    aes_encrypt_128( t, outText, k,k);

#if 0
    // swap the output also.
    for(i=0; i< 16/2;i++)
    {
        int tmp = outText[i];
        // fill the key and text backward. The aes encrypt uses big endian.
        outText[i] = outText[15-i];
        outText[15-i] = tmp;
    }
#endif
}
#endif



static  void xor_128(unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}

#ifdef AES_CMAC_UNIT_TEST
static  void print_hex(char *str, unsigned char *buf, int len)
{
    int     i;

    for ( i=0; i<len; i++ ) {
        if ( (i % 16) == 0 && i != 0 ) printf(str);
        printf("%02x", buf[i]);
        if ( (i % 4) == 3 ) printf(" ");
        if ( (i % 16) == 15 ) printf("\n");
    }
    if ( (i % 16) != 0 ) printf("\n");
}

static void print128(unsigned char *bytes)
{
    int         j;
    for (j=0; j<16;j++) {
        printf("%02x",bytes[j]);
        if ( (j%4) == 3 ) printf(" ");
    }
}

static  void print96(unsigned char *bytes)
{
    int         j;
    for (j=0; j<12;j++) {
        printf("%02x",bytes[j]);
        if ( (j%4) == 3 ) printf(" ");
    }
}
#endif

  /* AES-CMAC Generation Function */

static  void leftshift_onebit(unsigned char *input,unsigned char *output)
{
    int         i;
    unsigned char overflow = 0;

    for ( i=15; i>=0; i-- ) {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80)?1:0;
    }
    return;
}

static void generate_subkey(unsigned char *key, unsigned char *K1,
        unsigned char *K2)
{
    unsigned char L[16];
    unsigned char Z[16];
    unsigned char tmp[16];
    int i;

    for ( i=0; i<16; i++ ) Z[i] = 0;

    AES_128(key,Z,L);

    if ( (L[0] & 0x80) == 0 ) { /* If MSB(L) = 0, then K1 = L << 1 */
        leftshift_onebit(L,K1);
    } else {    /* Else K1 = ( L << 1 ) (+) Rb */


        leftshift_onebit(L,tmp);
        xor_128(tmp,const_Rb,K1);
    }

    if ( (K1[0] & 0x80) == 0 ) {
        leftshift_onebit(K1,K2);
    } else {
        leftshift_onebit(K1,tmp);
        xor_128(tmp,const_Rb,K2);
    }
    return;
}

static void padding ( unsigned char *lastb, unsigned char *pad, int length )
{
    int j;

    /* original last block */
    for ( j=0; j<16; j++ ) {
        if ( j < length ) {
            pad[j] = lastb[j];
        } else if ( j == length ) {
            pad[j] = 0x80;
        } else {
            pad[j] = 0x00;
        }
    }
}

void AES_CMAC ( unsigned char *key, unsigned char *input, int length,
                                                          unsigned char *mac )
{
    unsigned char       X[16],Y[16], M_last[16], padded[16];
    unsigned char       K1[16], K2[16];
    int         n, i, flag;
    generate_subkey(key,K1,K2);

    n = (length+15) / 16;       /* n is number of rounds */

    if ( n == 0 ) {
        n = 1;
        flag = 0;
    } else {
        if ( (length%16) == 0 ) { /* last block is a complete block */
            flag = 1;
        } else { /* last block is not complete block */
            flag = 0;
        }
    }

    if ( flag ) { /* last block is complete block */
        xor_128(&input[16*(n-1)],K1,M_last);
    } else {
        padding(&input[16*(n-1)],padded,length%16);
        xor_128(padded,K2,M_last);
    }

    for ( i=0; i<16; i++ ) X[i] = 0;
    for ( i=0; i<n-1; i++ ) {
        xor_128(X,&input[16*i],Y); /* Y := Mi (+) X  */
        AES_128(key,Y,X);      /* X := AES-128(KEY, Y); */
    }

    xor_128(X,M_last,Y);
    AES_128(key,Y,X);

    for ( i=0; i<16; i++ ) {
        mac[i] = X[i];
    }
}

#ifdef AES_CMAC_UNIT_TEST
int main()
{
    unsigned char L[16], K1[16], K2[16], T[16], TT[12];
    unsigned char M[64] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
        0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
        0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
        0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
        0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
    };
    unsigned char key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };

    printf("--------------------------------------------------\n");
    printf("K              "); print128(key); printf("\n");

    printf("\nSubkey Generation\n");
    AES_128(key,const_Zero,L);
    printf("AES_128(key,0) "); print128(L); printf("\n");
    generate_subkey(key,K1,K2);

    printf("K1             "); print128(K1); printf("\n");
    printf("K2             "); print128(K2); printf("\n");

    printf("\nExample 1: len = 0\n");
    printf("M              "); printf("<empty string>\n");

    AES_CMAC(key,M,0,T);
    printf("AES_CMAC       "); print128(T); printf("\n");

    printf("\nExample 2: len = 16\n");
    printf("M              "); print_hex("                ",M,16);
    AES_CMAC(key,M,16,T);
    printf("AES_CMAC       "); print128(T); printf("\n");
    printf("\nExample 3: len = 40\n");
    printf("M              "); print_hex("               ",M,40);
    AES_CMAC(key,M,40,T);
    printf("AES_CMAC       "); print128(T); printf("\n");

    printf("\nExample 4: len = 64\n");
    printf("M              "); print_hex("               ",M,64);
    AES_CMAC(key,M,64,T);
    printf("AES_CMAC       "); print128(T); printf("\n");

    printf("--------------------------------------------------\n");

    return 0;
}

#endif
