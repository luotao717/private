
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
	driver_wext.h

	Abstract:
	wireless extension interface for ralink driver

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __DRIVER_WEXT_H__
#define __DRIVER_WEXT_H__

#include "types.h"

/* Ralink defined OIDs */
#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define OID_GET_SET_TOGGLE					0x8000
#if 0
#define OID_802_11_WIFI_VER					0x0887
#define OID_802_11_HS_TEST      			0x0888
#define OID_802_11_HS_IE        			0x0889
#define OID_802_11_HS_ANQP_REQ  			0x088a
#define OID_802_11_HS_ANQP_RSP  			0x088b
#define OID_802_11_HS_ONOFF     			0x088c
#define OID_802_11_HS_IPCTYPE				0x088d
#define OID_802_11_HS_PARAM_SETTING			0x088e
#define RT_OID_WE_VERSION_COMPILED			0x0622
#define OID_802_11_WNM_BTM_REQ				0x0900
#define OID_802_11_WNM_BTM_QUERY			0x0901
#define OID_802_11_WNM_BTM_RSP				0x0902
#define OID_802_11_WNM_PROXY_ARP			0x0903
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST	0x0904
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST	0x0905
#define OID_802_11_SECURITY_TYPE			0x0906
#define OID_802_11_HS_RESET_RESOURCE		0x0907
#define OID_802_11_HS_AP_RELOAD				0x0908
#define OID_802_11_HS_BSSID					0x0909
#else
#define RT_OID_WE_VERSION_COMPILED                      0x0622
#define OID_802_11_WIFI_VER                     0x0920
#define OID_802_11_HS_TEST                      0x0921
#define OID_802_11_HS_IE                        0x0922
#define OID_802_11_HS_ANQP_REQ                  0x0923
#define OID_802_11_HS_ANQP_RSP                  0x0924
#define OID_802_11_HS_ONOFF                     0x0925
#define OID_802_11_HS_PARAM_SETTING             0x0927
#define OID_802_11_WNM_BTM_REQ                  0x0928
#define OID_802_11_WNM_BTM_QUERY                0x0929
#define OID_802_11_WNM_BTM_RSP                  0x093a
#define OID_802_11_WNM_PROXY_ARP                0x093b
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST      0x093c
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST      0x093d
#define OID_802_11_SECURITY_TYPE                0x093e
#define OID_802_11_HS_RESET_RESOURCE            0x093f
#define OID_802_11_HS_AP_RELOAD                 0x0940
#define OID_802_11_HS_BSSID                     0x0941
#endif

struct driver_wext_data {
	int opmode;
	char drv_mode;
	void *priv;
	struct netlink_data *netlink;
	int ioctl_sock;
	int we_version_compiled;
};

struct anqp_req_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 anqp_req_len;
	char anqp_req[0];
};

struct anqp_rsp_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u16 status;
	u32 anqp_rsp_len;
	char anqp_rsp[0];
};

struct hs_onoff {
	u32 ifindex;
	u8 hs_onoff;
	u8 event_trigger;
	u8 event_type;
};

struct hs_param_setting {
	u32 param;
	u32 value;
};

struct btm_req_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 btm_req_len;	
	char btm_req[0];
};

struct btm_rsp_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 btm_rsp_len;
	char btm_rsp[0];
};

struct btm_query_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 btm_query_len;
	char btm_query[0];
};

struct proxy_arp_entry {
	u32 ifindex;
	u8 ip_type;
	u8 from_ds;
	char source_mac_addr[6];
	char target_mac_addr[6];
	char ip_addr[0];
};

struct security_type {
	u32 ifindex;
	u8 auth_mode;
	u8 encryp_type;
};

struct proxy_arp_ipv4_unit {
	u8   target_mac_addr[6];
	u8   target_ip_addr[4];
};

struct proxy_arp_ipv6_unit {
	u8 target_mac_addr[6];
	u8 target_ip_type;
	u8 target_ip_addr[16];
};

#endif /* __DRIVER_WEXT_H__ */
