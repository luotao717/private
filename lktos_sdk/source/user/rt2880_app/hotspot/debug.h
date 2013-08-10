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
	debug.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define NIC_DBG_STRING      ("")

#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define RT_DEBUG_INFO       4

extern int RTDebugLevel;

#define DBGPRINT(Level, fmt, args...)   \
{                                       \
    if (Level <= RTDebugLevel)          \
    {                                   \
		printf("[%s]", __FUNCTION__);	\
        printf( fmt, ## args);          \
    }                                   \
}

#define DBGPRINT_RAW(Level, fmt, args...)   \
{       	                                \
    if (Level <= RTDebugLevel)          	\
    {                                   	\
        printf( fmt, ## args);          	\
    }                                   	\
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

#endif /* __DEBUG_H__ */
