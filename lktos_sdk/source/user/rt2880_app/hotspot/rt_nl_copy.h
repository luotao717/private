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
	rt_nl.h

	Abstract:
	ralink technology netlink-based wireless interface public header

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __RT_NL_H__
#define __RT_NL_H__

/*
 * RANL_CMD_TEST: Test netlink interface
 *
 * RANL_CMD_IE : Set/Receive information element
 *
 * RANL_CMD_ANQP_REQ: Send/Receive ANQP request to/from driver
 *
 * RANL_CMD_ANQP_RSP: Send/Receive ANQP response to/from driver
 *
 * RANL_CMD_HOTSPOT_ONOFF: Enable/Disable Hotspot2.0 feature
 *
 * RANL_CMD_JOIN: Join to workspace
 *
 * RANL_CMD_LEAVE: Leave the workspace
 *
 */
enum ranl_commands {
	RANL_CMD_UNSPEC,
	RANL_CMD_TEST,
	RANL_CMD_IE,
	RANL_CMD_ANQP_REQ,
	RANL_CMD_ANQP_RSP,
	RANL_CMD_HOTSPOT_ONOFF,
	RANL_CMD_JOIN,
	RANL_CMD_LEAVE,
	__RANL_CMD_AFTER_LAST,
	RANL_CMD_MAX = __RANL_CMD_AFTER_LAST - 1
};


enum ranl_attrs {
	RANL_ATTR_UNSPEC,
	RANL_ATTR_TEST,
	RANL_ATTR_IFINDEX,
	RANL_ATTR_OPMODE,
	RANL_ATTR_PID,
	RANL_ATTR_IE,
	RANL_ATTR_ANQP_DATA,
	RANL_ATTR_HOTSPOT_ONOFF,
	RANL_ATTR_PEER_MAC_ADDR,
	RANL_ATTR_STATUS,
	RANL_ATTR_WORKSPACE_ID,
	RANL_ATTR_HOTSPOT_IPCTYPE,
	__RANL_ATTR_AFTER_LAST,
	RANL_ATTR_MAX = __RANL_ATTR_AFTER_LAST - 1
};


#define RANL_FLAG_NEED_NETDEV 0x01 
#define HS_DAEMON_ID	0x00

int ranl_init(void);
void ranl_exit(void);
void ranl_hotspot_test_event(void);
void ranl_send_anqp_req_event(void *net_dev, const char *peer_mac_addr, 
				              const char *anqp_req, u16 anqp_req_len);
void ranl_send_anqp_rsp_event(void *net_dev, const char *peer_mac_addr,
							  u16 status,
					          const char *anqp_rsp, u16 anqp_rsp_len);
void ranl_hotspot_onff_event(void *net_dev, int onoff);
#endif /* __RT_NL_H__ */
