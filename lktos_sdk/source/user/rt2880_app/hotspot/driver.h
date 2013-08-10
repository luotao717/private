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
	driver.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "hotspot.h"

struct hotspot;

struct hotspot_drv_ops {
	int (*drv_test)(void *drv_data, const char *ifname);
	void * (*drv_inf_init)(struct hotspot *hs, const int opmode, const int drv_mode);
	int (*drv_inf_exit)(struct hotspot *hs);
	int (*drv_set_ie)(void *drv_data, const char *ifname, char *ie, 
					  size_t ie_len);
	int (*drv_send_anqp_req)(void *drv_data, const char *ifname, 
					         const char *peer_sta_addr,const char *anqp_req, 
						     size_t anqp_req_len);
	int (*drv_send_anqp_rsp)(void *drv_data, const char *ifname, 
						     const char *peer_sta_addr,const char *anqp_rsp, 
						     size_t anqp_rsp_len);
	int (*drv_hotspot_onoff)(void *drv_data, const char *ifname,
                             int enable, int event_trigger, int event_type);
	int (*drv_send_btm_req)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_req,
							size_t btm_req_len);
	int (*drv_send_btm_query)(void *drv_data, const char *ifname,
							  const char *peer_sta_addr, const char *btm_query,
							  size_t btm_query_len);
	int (*drv_send_btm_rsp)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_rsp,
							size_t btm_rsp_len);
	int (*drv_param_setting)(void *drv_data, const char *ifname, u32 param, u32 value);

	int (*drv_wifi_version)(void *drv_dta, const char *ifname,
							char *version, size_t *version_len);

	int (*drv_ipv4_proxy_arp_list)(void *drv_data, const char *ifname,
							  	   char *proxy_arp_list, size_t *proxy_arp_list_len);
	
	int (*drv_ipv6_proxy_arp_list)(void *drv_data, const char *ifname,
							  	   char *proxy_arp_list, size_t *proxy_arp_list_len);
	int (*drv_validate_security_type)(void *drv_data, const char *ifname);
	int (*drv_reset_resource)(void *drv_data, const char *ifname);
	int (*drv_get_bssid)(void *drv_data, const char *ifname, char *bssid, size_t *bssid_len);
};

#endif /* __DRIVER_H__ */
