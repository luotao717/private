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
	ieee80211_defs.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __IEEE80211_DEFS_H__
#define __IEEE80211_DEFS_H__

#include "types.h"
#include "list.h"

enum dot11u_status_code {
	ADVERTISEMENT_PROTOCOL_NOT_SUPPORTED = 59,
	UNSPECIFIED_FAILURE = 60,
	RESPONSE_NOT_RECEIVED_FROM_SERVER = 61,
	TIMEOUT = 62,
	QUERY_RESPONSE_TOO_LARGE = 63,
	REQUEST_REFUSED_HOME_NETWORK_NOT_SUPPORT = 64,
	SERVER_UNREACHABLE = 65,
	REQUEST_REFUSED_PERMISSIONS_RECEIVED_FROM_SSPN = 67,
	REQUEST_REFUSED_AP_NOT_SUPPORT_UNAUTH_ACCESS = 68,
	TRANSMISSION_FAILURE = 79,	
};


/* ANQP Info ID definitions */
enum {
	ANQP_QUERY_LIST = 256,
	ANQP_CAPABILITY,
	VENUE_NAME_INFO,
	EMERGENCY_CALL_NUMBER_INFO,
	NETWORK_AUTH_TYPE_INFO,
	ROAMING_CONSORTIUM_LIST,
	IP_ADDRESS_TYPE_AVAILABILITY_INFO,
	NAI_REALM_LIST,
	ThirdGPP_CELLULAR_NETWORK_INFO,
	AP_GEOSPATIAL_LOCATION,
	AP_CIVIC_LOCATION,
	AP_LOCATION_PUBLIC_IDENTIFIER_URI,
	DOMAIN_NAME_LIST,
	EMERGENCY_ALERT_IDENTIFIER_URI,
	EMERGENCY_NAI = 271,
	ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST = 56797,
};

/* HS2.0 ANQP Element subtype definition */
enum {
	HS_QUERY_LIST = 1,
	HS_CAPABILITY,
	OPERATOR_FRIENDLY_NAME,
	WAN_METRICS,
	CONNECTION_CAPABILITY,
	NAI_HOME_REALM_QUERY,
	OPERATING_CLASS,
};

/* EAP method types */
enum {
	EAP_TLS = 13,
	EAP_SIM = 18,
	EAP_TTLS = 21,
	EAP_AKA = 23,
};

/* Authentication parameter types */
enum {
	EXPANDED_EAP_METHOD = 1,
	NON_EAP_INNER_AUTH_TYPE,
	INNER_AUTH_EAP_METHOD_TYPE,
	EXPANDED_INNER_EAP_METHOD,
	CREDENTIAL_TYPE,
	TUNNELED_EAP_METHOD_CREDENTIAL_TYPE,
	VENDOR_SPECIFIC = 221,
};

struct anqp_frame {
	u16 info_id;
	u16 length;
	char variable[0];
} __attribute__ ((packed));


struct hs_anqp_frame {
	u16 info_id;
	u16 length;
	char oi[3];
	u8 type;
	u8 subtype;
	u8 reserved;
	char variable[0];
} __attribute__ ((packed));

#define IE_NEIGHBOR_REPORT		52
#define IE_INTERWORKING			107
#define IE_ADVERTISEMENT_PROTO	108
#define IE_QOS_MAP_SET			110
#define IE_ROAMING_CONSORTIUM	111
#define IE_HS2_INDICATION		221

#define WFA_TIA_HS 0x11


struct interworking_element {
	u8 eid;
	u8 length;
	u8 access_network_options;
	/*
 	 * Following are Venue info and HESSID
 	 */
	char variable[0];
} __attribute__ ((packed));

struct roaming_consortium_info_element {
	u8 eid;
	u8 length;
	u8 num_anqp_oi;
	u8 oi1_oi2_length;
	/*
 	 * Following are OI#1, OI#2(optional), and OI#3(optional)
 	 */
	char variable[0];
} __attribute__ ((packed));

struct hotspot2dot0_indication_element {
	u8 eid;
	u8 length;
	char oi[3]; /* 0x50 6F 9A */
	u8 type;
	u8 hotspot_conf;
} __attribute__ ((packed));

struct p2p_attribute {
	u8 attribute_id;
	u16 length;
	char variable[0];
} __attribute__ ((packed));

struct p2p_info_element {
	u8 eid;
	u8 length;
	char oui[3]; /* 0x50 6F 9A */
	u8 oui_type; /* 0x09 */
	/*
 	 * Following are P2P attributes
 	 */
	char variable[0];
} __attribute__ ((packed));

struct advertisement_proto_element {
	u8 eid;
	u8 length;
	/*
 	 * Following are advertisement protocol tuple #1, #2, ..#n
 	 */	 
	char variable[0];
} __attribute__ ((packed)); 


struct plmn_IEI {
	u8 plmn_list_iei;
	u8 plmn_list_len;
	u8 plmn_list_num;
	char variable[0];
} __attribute__ ((packed));

struct domain_name_field {
	struct dl_list list;
	u8 length;
	char domain_name[0];
};

struct venue_name_duple {
	struct dl_list list;
	u8 length;
	char language[3];
	char venue_name[0];
};

struct operator_name_duple {
	struct dl_list list;
	u8 length;
	char language[3];
	char operator_name[0];
};

struct oi_duple {
	struct dl_list list;
	u8 length;
	char oi[0];
};

struct net_auth_type_unit {
	struct dl_list list;
	u8 net_auth_type_indicator;
	u16 re_direct_URL_len;
	char re_direct_URL[0];
};

struct proto_port_tuple {
	struct dl_list list;
	u8 ip_protocol;
	u16 port;
	u8 status;
};

struct anqp_capability {
	struct dl_list list;
	u16 info_id;
};

struct anqp_hs_capability {
	struct dl_list list;
	u8 subtype;
};

struct auth_param {
	struct dl_list list;
	u8 id;
	u8 len;
	char auth_param_value[0];
};

struct eap_method {
	struct dl_list list;
	u8 len;
	u8 eap_method;
	u8 auth_param_count;
	struct dl_list auth_param_list;
};

struct nai_realm_data {
	struct dl_list list;
	u16 nai_realm_data_field_len;
	u8 nai_realm_encoding;
	u8 nai_realm_len;
	u8 eap_method_count;
	struct dl_list eap_method_list;
	char nai_realm[0];
};

struct nai_home_realm_data_query {
	struct dl_list list;
	u8 nai_home_realm_encoding;
	u8 nai_home_realm_len;
	char nai_home_realm[0];
};

struct wan_metrics {
	u8 link_status;
	u8 at_capacity;
	u32 dl_speed;
	u32 ul_speed;
	u8 dl_load;
	u8 ul_load;
	u16 lmd;
};

struct plmn {
	struct dl_list list;
	char mcc[3];
	char mnc[3];
};

struct operating_class_unit {
	struct dl_list list;
	u8 op_class;
};

#define GAS_ANQP_QUERY 	  0
#define GAS_ANQP_HS_QUERY 1

#define GUD_VER1 0
#define IEI_PLMN 0
#endif /* IEEE80211_DEFS_H__ */
