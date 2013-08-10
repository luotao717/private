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
	util.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __UTIL_H__
#define __UTIL_H__

u8 BtoH(char ch);
void AtoH(char *src, char *dest, int destlen);

/* Endian byte swapping codes */
#define SWAP16(x) \
    ((u16) (\
           (((u16) (x) & (u16) 0x00ffU) << 8) | \
           (((u16) (x) & (u16) 0xff00U) >> 8))) 
 
#define SWAP32(x) \
    ((u32) (\
           (((u32) (x) & (u32) 0x000000ffUL) << 24) | \
           (((u32) (x) & (u32) 0x0000ff00UL) << 8) | \
           (((u32) (x) & (u32) 0x00ff0000UL) >> 8) | \
           (((u32) (x) & (u32) 0xff000000UL) >> 24))) 

#define SWAP64(x) \
    ((u64)( \
    (u64)(((UINT64)(x) & (u64) 0x00000000000000ffULL) << 56) | \
    (u64)(((UINT64)(x) & (u64) 0x000000000000ff00ULL) << 40) | \
    (u64)(((UINT64)(x) & (u64) 0x0000000000ff0000ULL) << 24) | \
    (u64)(((UINT64)(x) & (u64) 0x00000000ff000000ULL) <<  8) | \
    (u64)(((UINT64)(x) & (u64) 0x000000ff00000000ULL) >>  8) | \
    (u64)(((UINT64)(x) & (u64) 0x0000ff0000000000ULL) >> 24) | \
    (u64)(((UINT64)(x) & (u64) 0x00ff000000000000ULL) >> 40) | \
    (u64)(((UINT64)(x) & (u64) 0xff00000000000000ULL) >> 56) ))
 
#ifdef RT_BIG_ENDIAN
#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((u64)(x))
#define be2cpu64(x) ((u64)(x))
#define cpu2be32(x) ((u32)(x))
#define be2cpu32(x) ((u32)(x))
#define cpu2be16(x) ((u16)(x))
#define be2cpu16(x) ((u16)(x))
#else /* Little_Endian */
#define cpu2le64(x) ((u64)(x))
#define le2cpu64(x) ((u64)(x))
#define cpu2le32(x) ((u32)(x))
#define le2cpu32(x) ((u32)(x))
#define cpu2le16(x) ((u16)(x))
#define le2cpu16(x) ((u16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))
#endif /* RT_BIG_ENDIAN */

void * __hide_aliasing_typecast(void *foo);
#define aliasing_hide_typecast(a,t) (t *) __hide_aliasing_typecast((a))

#endif /* __UTIL_H__ */
