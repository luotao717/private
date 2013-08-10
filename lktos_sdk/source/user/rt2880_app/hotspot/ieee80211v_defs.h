
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
	ieee80211v_defs.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __IEEE80211V_DEFS_H__
#define __IEEE80211V_DEFS_H__


enum wnm_ie_eid {
	IE_TIME_ADVERTISEMENT = 69,
	IE_TIME_ZONE = 98,
};

enum transition_query_reason {
    UNSPECIFIED,
    EXCESSIVE_FRAME_LOSS_RATE,
    EXCESSIVE_DELAY,
    INSUFFICIENT_QOS_CAPACITY,
    FIRST_ASSOCIATION_TO_ESS,
    LOAD_BALANCING,
    BETTER_AP_FOUND,
    DEAUTHENTICATED_OR_DISASSOCIATED_FROM_PREVIOUS_AP,
    AP_FAILED_IEEE8021X_EAP_AUTH,
    AP_FAILED_FOUR_WAY_HANDSHAKE,
    RECEIVED_TOO_MANY_REPLAY_COUNTER_FAILURE,
    RECEIVED_TOO_MANY_DATA_MIC_FAILURE,
    EXCEEDED_MAXIMUM_NUMBER_OF_RETRANSMISSIONS,
    RECEIVED_TOO_MANY_BROADCAST_DISASSOCIATIONS,
    RECEIVED_TOO_MANY_BROADCAST_DEAUTHENTICATION,
    PREVIOUS_TRANSITION_FAILED,
    LOW_RSSI,
    ROAM_FROM_A_NON_80211_SYSTEM,
    RECEIVED_BSS_TRANSITION_REQ_FRAME,
    PREFERRED_BSS_TRANSITION_CANDIDATE_LIST_INCLUDED,
    LEAVING_ESS,
};

enum neighbor_report_subelement_id {
    RESERVED,
    TSF_INFORMATION,
    CONDENSED_COUNTRY_STRING,
    BSS_TRANSITION_CANDIDATE_PREFERENCE,
    BSS_TERMINATION_DURATION,
    BEARING,
    MEASUREMENT_PILOT_TRANSMISSION_INFO = 66,
    RPM_ENABLE_CAPABILITIES = 70,
    MULTIPLE_BSSID,
    VENDOR_SPECIFIC_FIELD = 221,
};

struct time_advertisement_element {
	u8 eid;
	u8 length;
	u8 timing_capabilities;
	/*
 	 * Following are Time value, TIme Error, and Time Update Counter
 	 */ 
	u8 variable[0];
} __attribute__ ((packed));

struct time_zone_element {
	u8 eid;
	u8 length;
	/*
 	 * Following are Time Zone
 	 */
	u8 variable[0]; 
} __attribute__ ((packed));

struct btm_frame {
	union {
		struct {
			u8 btm_query_reason;
			/*
 			 * Following are BSS Transition Candidates List Entries
 			 */
			u8 variable[0]; 
		} __attribute__ ((packed)) btm_query;

		struct {
			u8 request_mode;
			u16 disassociation_timer;
			u8 validity_interval;
			/* 
 			 * Following are BSS Termination Duration, Session Information URL,
 			 * and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__((packed)) btm_req;

		struct {
			u8 status_code;
			u8 bss_termination_delay;
			/*
 			 * Following are Target BSSID, and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__ ((packed)) btm_rsp;
	}u;
} __attribute__ ((packed));

struct neighbor_report_subelement {
	u8 subelement_id;
	u8 length;
	union {
		struct {
			u8 preference;
		} __attribute__ ((packed)) bss_transition_candi_preference;

		struct {
			u64 bss_termination_tsf;
			u16 duration;
		} __attribute__ ((packed)) bss_termination_duration;

		struct {
			u16 bearing;
			u32 distance;
			u16 relative_height;
		} __attribute__ ((packed)) bearing;
	}u;
} __attribute__ ((packed));

struct neighbor_report_element {
	u8 eid;
	u8 length;
	u8 bssid[6];
	u32 bss_info;
	u8 regulatory_class;
	u8 channel_number;
	u8 phy_type;
	/*
 	 * Following are Optional sub elements
 	 */
	u8 variable[0];
} __attribute__ ((packed));

struct bss_transition_candi_preference_unit {
	struct dl_list list;
	u8 preference;
};

#endif /* __IEEE80211V_DEFS_H__ */
