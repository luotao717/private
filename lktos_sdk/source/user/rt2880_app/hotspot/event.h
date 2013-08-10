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
	event.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __EVENT_H__
#define __EVENT_H__

#include "hotspot.h"

struct hotspot;

struct hotspot_event_ops {
	int (*event_anqp_req)(struct hotspot *hs, 
					  	  const char *ifname, 
					  	  const char *peer_mac_addr, 
					  	  char *anqp_req, 
					  	  size_t anqp_req_len);
	
	int (*event_anqp_rsp)(struct hotspot *hs,
						  const char *ifname,
						  const char *peer_mac_addr,
						  int status,
						  const char *anqp_rsp,
						  size_t anqp_rsp_len);

	int (*event_test)(struct hotspot *hs);

	int (*event_hs_onoff)(struct hotspot *hs,
						 const char *ifname,
						 int enable);

	int (*event_btm_query)(struct hotspot *hs,
						   const char *ifname,
						   const char *peer_mac_addr,
						   const char *btm_query,
						   size_t btm_query_len);

	int (*event_btm_rsp)(struct hotspot *hs,
						 const char *ifname,
						 const char *peer_mac_addr,
						 const char *btm_rsp,
						 size_t btm_rsp_len);

	int (*event_proxy_arp)(struct hotspot *hs,
						   const int ifindex,
						   u8 ip_type,
						   u8 from_ds,
						   const char *source_mac_addr,
						   const char *source_ip_addr,
						   const char *target_mac_addr,
						   const char *target_ip_addr);
	
	int (*event_ap_reload)(struct hotspot *hs,
						   const char *ifname);
};

enum {
	HS_ON_OFF_BASE,
	HS_AP_RELOAD,
};

enum {
	EVENT_TRIGGER_OFF,
	EVENT_TRIGGER_ON,
};

#endif /* __EVENT_H__ */
