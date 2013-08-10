/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	util.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include "types.h"

u8 BtoH(char ch)
{
    if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
    if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
    if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
    return(255);
}

void AtoH(char *src, char *dest, int destlen)
{
    char *srcptr;
    char *destTemp;

    srcptr = src;   
    destTemp = (char *) dest; 

    while(destlen--)
    {
        *destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
        *destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
        destTemp++;
    }
}

void * __hide_aliasing_typecast(void *foo)         
{                                                                       
    return foo;        
}
