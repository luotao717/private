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
	driver_ranl.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __DRIVER_RANL_H__
#define __DRIVER_RANL_H__

//#ifdef CONFIG_LIBNL20
#define nl_handle nl_sock
#define nl_handle_alloc_cb nl_socket_alloc_cb
#define nl_handle_destroy nl_socket_free
//#endif /* CONFIG_LIBNL20 */

#include "rt_nl_copy.h"

struct driver_ranl_data {
	struct nl_handle *nl_handle;
	struct nl_handle *nl_handle_event;
	struct nl_cache *nl_cache;
	struct nl_cache *nl_cache_event;
	struct nl_cb *nl_cb;
	struct genl_family *ranl;
	int opmode;
	u8 drv_mode;
	void *priv;
};

#endif /* __DRIVER_RANL_H__ */
