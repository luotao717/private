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
	hotspot.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#include <net/if.h>
#include <stdint.h>
#include "types.h"
#include "os.h"
#include "list.h"
#include "util.h"
#include "driver.h"
#include "event.h"
#include "debug.h"
#include "eloop.h"
#include "rt_nl_copy.h"
#include "ctrl_iface_unix.h"
#include "ieee80211_defs.h"
#include "ieee80211v_defs.h"
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>

#define HOTSPOT_VERSION "1.0.0"

enum {
	PARAM_DGAF_DISABLED,
	PARAM_PROXY_ARP,
	PARAM_L2_FILTER,
	PARAM_ICMPV4_DENY,
	PARAM_MMPDU_SIZE,
	PARAM_EXTERNAL_ANQP_SERVER_TEST,
};

enum {
	IPV4,
	IPV6,
};

struct hotspot_conf {
	struct dl_list list;
	int hotspot_onoff;
	char iface[IFNAMSIZ + 1];
	char confname[256];
	int access_network_type;
	u8 interworking;
	int internet;
	int venue_group;
	int is_venue_group;
	int venue_type;
	int is_venue_type;
	char hessid[6];
	int is_hessid;
	u8 anqp_query;
	u8 mih_support;
	char *advertisement_proto;
	int advertisement_proto_num;

	u8 anqp_req_type;
	char hs_peer_mac[6];

	size_t calc_anqp_rsp_len;
	u8 have_anqp_capability_list;
	u8 query_anqp_capability_list;
	struct dl_list anqp_capability_list;

	u8 venue_name_nums;
	u8 have_venue_name;
	u8 query_venue_name;
	struct dl_list venue_name_list;
	
	u8 have_emergency_call_number;
	u8 query_emergency_call_number;
	struct dl_list emergency_call_number_list;
	
	u8 network_auth_type_nums;
	u8 have_network_auth_type;
	u8 query_network_auth_type;
	struct dl_list network_auth_type_list;
	
	u8 have_roaming_consortium_list;
	u8 query_roaming_consortium_list;
	struct dl_list oi_duple_list;
	
	u8 have_ip_address_type;
	u8 query_ip_address_type;
	u8 ipv4_address_type;
	u8 ipv6_address_type;

	u8 nai_realm_data_nums;
	u8 have_nai_realm_list;
	u8 query_nai_realm_list;
	struct dl_list nai_realm_list;

	u8 plmn_nums;
	u8 have_3gpp_network_info;
	u8 query_3gpp_network_info;
	u8 gud; /* generic container user data */
	u8 udhl; /* user data header length */
	struct dl_list plmn_list;

	u8 have_operating_class;
	u8 query_operating_class;
	struct dl_list operating_class_list;
	
	u8 have_ap_geospatial_location;
	u8 query_ap_geospatial_location;
	char *ap_geospatial_location;
	u16 ap_geospatial_location_len;
	
	u8 have_ap_civic_location;
	u8 query_ap_civic_location;
	char *ap_civic_location;
	u16 ap_civic_location_len;
	
	u8 have_ap_location_public_uri;
	u8 query_ap_location_public_uri;
	char *ap_location_public_uri;
	u16 ap_location_public_uri_len;
	
	u8 have_domain_name_list;
	u8 query_domain_name_list;
	struct dl_list domain_name_list;
	
	u8 have_emergency_alert_uri;
	u8 query_emergency_alert_uri;
	char *emergency_alert_uri;
	u16 emergency_alert_uri_len;
	
	u8 have_emergency_nai;
	u8 query_emergency_nai;
	char *emergency_nai;
	u16 emergency_nai_len;

	/* Following are additional hotspot 2.0 elements */
	size_t calc_hs_anqp_rsp_len;
	u8 have_hs_capability_list;
	u8 query_hs_capability_list;
	struct dl_list hs_capability_list;

	u8 op_friendly_name_nums;
	u8 have_operator_friendly_name;
	u8 query_operator_friendly_name;
	struct dl_list operator_friendly_duple_list;

	u8 wan_metrics_nums;
	u8 have_wan_metrics;
	u8 query_wan_metrics;
	struct wan_metrics metrics;

	u8 proto_port_nums;
	u8 have_connection_capability_list;
	u8 query_connection_capability_list;
	struct dl_list connection_capability_list;

	u8 have_nai_home_realm_query;
	u8 query_nai_home_realm;
	size_t calc_hs_nai_home_realm_anqp_rsp_len;
	struct dl_list nai_home_realm_name_query_list;

	u8 preferred_candi_list_included;
	u8 abridged;
	u8 disassociation_imminent;
	u8 bss_termination_included;
	u8 ess_disassociation_imminent;
	u16 disassociation_timer;
	u8 validity_interval;
	u8 have_bss_termination_duration;
	u64 bss_termination_tsf;
	u16 bss_termination_duration; 
	u8 have_session_info_url;
	int session_info_url_len;
	char *session_info_url;
	u8 have_bss_transition_candi_list;
	struct dl_list bss_transition_candi_list;

	u8 have_time_zone;
	int time_zone_len;
	char *time_zone;

	u8 DGAF_disabled;
	u8 proxy_arp;
	u8 l2_filter;
	u8 icmpv4_deny;
	u8 p2p_cross_connect_permitted;
	u32 mmpdu_size;
	u8 external_anqp_server_test;
	u16 gas_cb_delay;
	u8 hs2_openmode_test;
};

struct hs_peer_entry {
	struct dl_list list;
	u8 peer_mac_addr[6];
};

struct hotspot {
	/* driver interface operation */
	const struct hotspot_drv_ops *drv_ops;

	/* event operation */
	const struct hotspot_event_ops *event_ops;

	/* driver interface data */
	void *drv_data;

	/* hotspot configuration list */
	struct dl_list hs_conf_list;

	/* hotspot peer entry list */
	struct dl_list hs_peer_list;

	/* hotspot control interface */
	struct hotspot_ctrl_iface *hs_ctrl_iface;
	
	int opmode;

	u8 drv_mode;
};

int hotspot_init(struct hotspot *hs, struct hotspot_event_ops *event_ops, int drv_mode, int opmode);
void hotspot_run(struct hotspot *hs);
int hotspot_deinit(struct hotspot *hs);
int hotspot_onoff(struct hotspot *hs, const char *ifname, int enable, 
					int event_trigger, int event_type);

size_t hotspot_calc_btm_req_len(struct hotspot *hs,
							 	struct hotspot_conf *conf);

int hotspot_collect_btm_req(struct hotspot *hs,
							struct hotspot_conf *conf,
							const char *peer_addr,
							char *btm_req);

char * hotspot_config_get_line(char *s, int size, FILE *stream, int *line,
									 char **_pos);

int hotspot_init_all_config(struct hotspot *hs, const char *confname);
int hotspot_deinit_all_config(struct hotspot *hs);
int hotspot_set_sta_all_ies(struct hotspot *hs, const char *iface);
int hotspot_set_sta_ifaces_all_ies(struct hotspot *hs);
int hotspot_set_ap_all_ies(struct hotspot *hs, const char *iface);
int hotspot_set_ap_ifaces_all_ies(struct hotspot *hs);
int hotspot_clear_all_ap_ies(struct hotspot *hs);
int hotspot_onoff_all(struct hotspot *hs, int enable);

int hotspot_driver_version(struct hotspot *hs, const char *ifname,
						   char *reply, size_t *reply_len);

int hotspot_ipv4_proxy_arp_list(struct hotspot *hs, const char *ifname,
						   		char *reply, size_t *reply_len);

int hotspot_ipv6_proxy_arp_list(struct hotspot *hs, const char *ifname,
						   		char *reply, size_t *reply_len);

int hotspot_icmpv4_deny(struct hotspot *hs, const char *ifname,
						u8 deny, char *reply, size_t *reply_len);

int hotspot_send_btm_req(struct hotspot *hs, const char *iface,
						 const char *peer_mac_addr,
						 const char *btm_req,
						 size_t btm_req_len);

int hotspot_ap_reload(struct hotspot *hs,
					  const char *iface);

int hotspot_reset_all_ap_resource(struct hotspot *hs);

/* Hotspot operation mode */
#define OPMODE_STA 0
#define OPMODE_AP  1

/* Hotspot driver mode */
#define RA_WEXT 	0
#define RA_NETLINK  1

struct _ipv6_addr {
	union {
		u8 ipv6Addr8[16];
		u16 ipv6Addr16[8];
		u32 ipv6Addr32[4]; 
	}addr;
#define ipv6_addr addr.ipv6Addr8
#define ipv6_addr16 addr.ipv6Addr16
#define ipv6_addr32 addr.ipv6Addr32
};


#endif /* __HOTSPOT_H__ */
