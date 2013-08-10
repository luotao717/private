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
	ctrl_iface_unix.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot.h"
#include "ieee80211_defs.h"
#include <sys/socket.h>
#include <unistd.h>
#include "ctrl_iface_unix.h"

static int hotspot_ctrl_iface_event_register(struct hotspot_ctrl_iface *ctrl_iface,
											 struct sockaddr_un *from,
											 socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst;

	ctrl_dst = os_zalloc(sizeof(*ctrl_dst));

	if (!ctrl_dst) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	os_memcpy(&ctrl_dst->addr, from, sizeof(struct sockaddr_un));
	ctrl_dst->addrlen = fromlen;
	dl_list_add(&ctrl_iface->hs_ctrl_dst_list, &ctrl_dst->list);
		
	return 0;
}

static int hotspot_ctrl_iface_event_unregister(struct hotspot_ctrl_iface *ctrl_iface,
											   struct sockaddr_un *from,
											   socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst, *ctrl_dst_tmp;

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
								struct hotspot_ctrl_dst, list) {
		if (fromlen == ctrl_dst->addrlen && os_memcpy(from->sun_path, ctrl_dst->addr.sun_path,
												fromlen - offsetof(struct sockaddr_un, sun_path))
												== 0) {
			dl_list_del(&ctrl_dst->list);
			os_free(ctrl_dst);
			return 0;
		}
	}

	return -1;
}

static int hotspot_ctrl_iface_cmd_version(struct hotspot *hs,
											char *reply, size_t *reply_len )
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	os_strncpy(reply, HOTSPOT_VERSION, 5);
	*reply_len = 5;
	return 0;
}

static int hotspot_ctrl_iface_cmd_on(struct hotspot *hs, const char *iface,
									 char *reply, size_t *reply_len)
{
	int ret = 0;
	
	ret = hotspot_onoff(hs, iface, 1, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	*reply_len = 0;
 
	return ret;
}

static int hotspot_ctrl_iface_cmd_off(struct hotspot *hs, const char *iface,
									  char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_onoff(hs, iface, 0, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	*reply_len = 0;

	return ret;
}

static int hotspot_ctrl_iface_cmd_get(struct hotspot *hs, const char *iface,
									  char *param, char *reply, size_t *reply_len)
{
	int ret = 0;
	struct hotspot_conf *conf = NULL;
	int is_found;

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (is_found) {
		if (os_strcmp(param, "access_network_type") == 0) {
			sprintf(reply, "access_network_type=%d", conf->access_network_type);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "internet") == 0) {
			sprintf(reply, "internet=%d", conf->internet);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "timezone") == 0) {
			sprintf(reply, "timezone=%s", conf->time_zone);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "all") == 0) {
			sprintf(reply, "access_network_type=%d\n"
						   "internet=%d\n",
						   conf->access_network_type,
						   conf->internet);
			*reply_len = os_strlen(reply);
		} else
			ret = -1;
	} else
		ret = -1; 

	return ret;
}

static void hotspot_ctrl_clear_general_multiple_param(const char *confname,
											 		  const char *param)
{
	FILE *file, *tmpfile;
	char buf[256];
	char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf";
	u8 is_clear = 0;

	os_memset(buf, 0, 256);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}

	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}
	
	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, param) != NULL) {
			if (!is_clear) {
				fprintf(tmpfile, "%s=n/a\n", param);
				is_clear = 1;
			}
		} else {
			fputs(buf, tmpfile);
		} 
	}
	
	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_general_param(const char *confname,
										   const char *param, char *value)
{
	FILE *file, *tmpfile;
	char buf[256];
	char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 

	os_memset(buf, 0, 256);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}

	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}
	
	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, param) != NULL) {
			fprintf(tmpfile, "%s=%s\n", param, value);
		} else {
			fputs(buf, tmpfile);
		} 
	}
	
	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_interface_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "interface", value);
}

static void hotspot_ctrl_set_interworking_param(struct hotspot_conf *conf,
												const char *confname, 
					 							char *value)
{
	hotspot_ctrl_set_general_param(confname, "interworking", value);
}

static void hotspot_ctrl_set_access_net_type_param(struct hotspot_conf *conf,
												   const char *confname, 
					 							   char *value)
{
	hotspot_ctrl_set_general_param(confname, "access_network_type", value);
}

static void hotspot_ctrl_set_internet_param(struct hotspot_conf *conf,
											const char *confname, 
					 						char *value)
{
	hotspot_ctrl_set_general_param(confname, "internet", value);
}

static void hotspot_ctrl_set_venue_group_param(struct hotspot_conf *conf,
											   const char *confname, 
					 						   char *value)
{
	hotspot_ctrl_set_general_param(confname, "venue_group", value);
}

static void hotspot_ctrl_set_venue_type_param(struct hotspot_conf *conf,
											  const char *confname, 
					 						  char *value)
{
	hotspot_ctrl_set_general_param(confname, "venue_type", value);
}

void hotspot_ctrl_set_hessid_param(struct hotspot_conf *conf,
								   const char *confname,
								   char *value)
{
	hotspot_ctrl_set_general_param(confname, "hessid", value);
}

static void hotspot_ctrl_set_roaming_consortium_oi_param(struct hotspot_conf *conf,
														 const char *confname,
														 char *value)
{
	hotspot_ctrl_set_general_param(confname, "roaming_consortium_oi", value);
}

static void hotspot_ctrl_set_advertisement_proto_id_param(struct hotspot_conf *conf,
														  const char *confname,
														  char *value)
{
	hotspot_ctrl_set_general_param(confname, "advertisement_proto_id", value);
}

static void hotspot_ctrl_set_anqp_query_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_param(confname, "anqp_query", value);
}

static void hotspot_ctrl_set_mih_support_param(struct hotspot_conf *conf,
											   const char *confname,
											   char *value)
{
	hotspot_ctrl_set_general_param(confname, "mih_support", value);
}

static void hotspot_ctrl_set_dgaf_disabled_param(struct hotspot_conf *conf,
												 const char *confname,
												 char *value)
{
	hotspot_ctrl_set_general_param(confname, "dgaf_disabled", value);
}

static void hotspot_ctrl_set_timezone_param(struct hotspot_conf *conf,
											const char *confname,
										    char *value)
{
	hotspot_ctrl_set_general_param(confname, "timezone", value);
}

static void hotspot_ctrl_set_gas_cb_delay_param(struct hotspot_conf *conf,
												const char *confname,
												char *value)
{
	hotspot_ctrl_set_general_param(confname, "gas_cb_delay", value);
}

static void hotspot_ctrl_set_hs2_openmode_test_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "hs2_openmode_test", value);
}

void hotspot_ctrl_set_proxy_arp_param(struct hotspot_conf *conf,
									  const char *confname,
									  char *value)
{
	hotspot_ctrl_set_general_param(confname, "proxy_arp", value);
}

static void hotspot_ctrl_set_l2_filter_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "l2_filter", value);
}

static void hotspot_ctrl_set_p2p_cross_connect_permitted_param(struct hotspot_conf *conf,
															   const char *confname,
															   char *value)
{
	hotspot_ctrl_set_general_param(confname, "p2p_cross_connect_permitted", value);
}

static void hotspot_ctrl_set_mmpdu_size_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_param(confname, "mmpdu_size", value);
}

static void hotspot_ctrl_set_external_anqp_server_test_param(struct hotspot_conf *conf,
															 const char *confname,
															 char *value)
{
	hotspot_ctrl_set_general_param(confname, "external_anqp_server_test", value);
}

static void hotspot_ctrl_set_icmpv4_deny_param(struct hotspot_conf *conf,
											   const char *confname,
										 	   char *value)
{
	hotspot_ctrl_set_general_param(confname, "icmpv4_deny" ,value);
}

static void hotspot_ctrl_set_ipv4_type_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "ipv4_type", value);
}

static void hotspot_ctrl_set_ipv6_type_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "ipv6_type", value);
}

static void hotspot_ctrl_set_ip_type_id_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_ipv4_type_param(conf, confname, "n/a");
	hotspot_ctrl_set_ipv6_type_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_ipv4_type_param(conf, confname, "3");
		hotspot_ctrl_set_ipv6_type_param(conf, confname, "0");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow IP Type ID\n");
	}
}

static void hotspot_ctrl_set_domain_name_param(struct hotspot_conf *conf,
											   const char *confname,
											 	char *value)
{
	hotspot_ctrl_set_general_param(confname, "domain_name", value);
}

static void hotspot_set_multiple_param_nums(struct hotspot_conf *conf,
											const char *param,
											u8 param_nums)
{
	if (os_strcmp(param, "venue_name") == 0)
		conf->venue_name_nums = param_nums;
	else if (os_strcmp(param, "network_auth_type") == 0)
		conf->network_auth_type_nums = param_nums;
	else if (os_strcmp(param, "op_friendly_name") == 0)
		conf->op_friendly_name_nums = param_nums;
	else if (os_strcmp(param, "plmn") == 0)
		conf->plmn_nums = param_nums;
	else if (os_strcmp(param, "proto_port") == 0)
		conf->proto_port_nums = param_nums;
	else if (os_strcmp(param, "wan_metrics") == 0)
		conf->wan_metrics_nums = param_nums;
	else if (os_strcmp(param, "nai_realm_data") == 0)
		conf->nai_realm_data_nums = param_nums;
	else
		DBGPRINT(RT_DEBUG_ERROR, "Unknow parameters(%s)\n", param);
}

static u8 hotspot_get_multiple_param_nums(struct hotspot_conf *conf,
											const char *param)
{
	u8 param_nums = 0;

	if (os_strcmp(param, "venue_name") == 0)
		param_nums = conf->venue_name_nums;
	else if (os_strcmp(param, "network_auth_type") == 0)
		param_nums = conf->network_auth_type_nums;
	else if (os_strcmp(param, "op_friendly_name") == 0)
		param_nums = conf->op_friendly_name_nums;
	else if (os_strcmp(param, "plmn") == 0)
		param_nums = conf->plmn_nums;
	else if (os_strcmp(param, "proto_port") == 0)
		param_nums = conf->proto_port_nums;
	else if (os_strcmp(param, "wan_metrics") == 0)
		param_nums = conf->wan_metrics_nums;
	else if (os_strcmp(param, "nai_realm_data") == 0)
		param_nums = conf->nai_realm_data_nums;
	else
		DBGPRINT(RT_DEBUG_ERROR, "Unknow parameters(%s)\n", param);

	return param_nums;			
}
static void hotspot_ctrl_set_general_multiple_param(struct hotspot_conf *conf,
													const char *confname,
													const char *param,
													const char *value)
{
	FILE *file, *tmpfile;
	char buf[256];
	char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;

	os_memset(buf, 0, 256);
	
	if (os_strcmp(value, "n/a") == 0) {	
		hotspot_ctrl_clear_general_multiple_param(confname, param);
		hotspot_set_multiple_param_nums(conf, param, 0);
	} else {
		file = fopen(confname, "r");

		if (!file) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
			return;
		}

		tmpfile = fopen(tmp_confname, "w");

		if (!tmpfile) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
			fclose(file);
			return;
		}
	
		total_param_nums = hotspot_get_multiple_param_nums(conf, param);
		while (fgets(buf, sizeof(buf), file) != NULL) {
			if ((strstr(buf, param) != NULL)) {
				cur_param_nums++;
				if (total_param_nums == cur_param_nums) {
					fputs(buf, tmpfile);
					fprintf(tmpfile, "%s=%s\n", param, value);
					total_param_nums++;
					hotspot_set_multiple_param_nums(conf, param, total_param_nums);
				} else if (total_param_nums < cur_param_nums) {
					fprintf(tmpfile, "%s=%s\n", param, value);
					hotspot_set_multiple_param_nums(conf, param, 1);
				} else {
					fputs(buf, tmpfile);
				}
			} else {
				fputs(buf, tmpfile);
			} 
		}
	
		fclose(tmpfile);
		fclose(file);

		unlink(confname);
		rename(tmp_confname, confname);
	}
}

static void hotspot_ctrl_set_venue_name_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname, 
											"venue_name", value);
}

static void hotspot_ctrl_set_venue_name_id_param(struct hotspot_conf *conf,
												 const char *confname,
												 char *value)
{
	hotspot_ctrl_set_venue_name_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_venue_name_param(conf, confname, "eng%'Wi-Fi Alliance Labs 2989 Copper Road Santa Clara, CA 95051, USA'");
		hotspot_ctrl_set_venue_name_param(conf, confname, "chi%美國CA聖塔克拉拉郡谷柏道2989號Wi-Fi聯盟實驗室");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Venue Name ID\n");
	}
}

static void hotspot_ctrl_set_network_auth_type_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname, 
											"network_auth_type", value);
}

static void hotspot_ctrl_set_net_auth_type_id_param(struct hotspot_conf *conf,
													const char *confname,
													char *value)
{

}

static void hotspot_ctrl_set_op_friendly_name_param(struct hotspot_conf *conf,
													const char *confname,
													char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname,
											"op_friendly_name", value);
}

static void hotspot_ctrl_set_op_friendly_name_id_param(struct hotspot_conf *conf,
													   const char *confname,
													   char *value)
{
	hotspot_ctrl_set_op_friendly_name_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_op_friendly_name_param(conf, confname, "eng,Wi-Fi Alliance");
		hotspot_ctrl_set_op_friendly_name_param(conf, confname, "chi,Wi-Fi無線聯盟");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Operator Friendly Name ID\n");
	}
}
	
static void hotspot_ctrl_clear_brace_multiple_param(struct hotspot_conf *conf,
													const char *confname,
										   			const char *param)
{
	FILE *file, *tmpfile;
	char tmpbuf[256];
	char buf[256];
	char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf";
	u8 is_brace_open = 0;
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;

	os_memset(tmpbuf, 0, 256);
	os_memset(buf, 0, 256);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}
	
	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}

	total_param_nums = hotspot_get_multiple_param_nums(conf, param);

	if (total_param_nums == 0)
		sprintf(tmpbuf, "%s=n/a\n", param);
	else
		sprintf(tmpbuf, "%s={\n", param);

	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, tmpbuf) != NULL) {
			if ((total_param_nums == 0) || (cur_param_nums == (total_param_nums - 1))) {
				fprintf(tmpfile, "%s=n/a\n", param);
			}

			if (total_param_nums != 0)
				is_brace_open = 1;
		} else if (strstr(buf, "}") != NULL && is_brace_open) {
			cur_param_nums++;
			is_brace_open = 0;
		} else {
			if (!is_brace_open)
				fputs(buf, tmpfile);
		} 
	}

	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_brace_multiple_param(struct hotspot_conf *conf,
												  const char *confname,
												  const char *param,
												  const char *value)
{
	FILE *file, *tmpfile;
	char *token;
	char buf[256], tmp_value[256];
	char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;
	u8 is_param = 0;

	os_memset(buf, 0, 256);
	os_memset(tmp_value, 0, 256);
	
	if (os_strcmp(value, "n/a") == 0) {
		hotspot_ctrl_clear_brace_multiple_param(conf, confname, param);
		hotspot_set_multiple_param_nums(conf, param, 0);
	} else {
		file = fopen(confname, "r");

		if (!file) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
			return;
		}

		tmpfile = fopen(tmp_confname, "w");

		if (!tmpfile) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
			fclose(file);
			return;
		}
	
		total_param_nums = hotspot_get_multiple_param_nums(conf, param);
		os_memset(tmp_value, 0, 256);
		os_memcpy(tmp_value, value, os_strlen(value));
		while (fgets(buf, sizeof(buf), file) != NULL) {
			if ((strstr(buf, param) != NULL)) {
				is_param = 1;
				if (total_param_nums == 0) {
					fprintf(tmpfile, "%s={\n", param);
					token = strtok(tmp_value, ",");
					do {
						fprintf(tmpfile, "	%s=", token);
						token = strtok(NULL, ",");
						fprintf(tmpfile, "%s\n", token);
						token = strtok(NULL, ",");
					} while (token != NULL);
					fprintf(tmpfile, "}\n");
					hotspot_set_multiple_param_nums(conf, param, 1);
				} else {
					fputs(buf, tmpfile);
				}
			} else {
				if ((strstr(buf, "}") != NULL) && (is_param)) {
					cur_param_nums++;
					if (cur_param_nums == total_param_nums) {
						fputs(buf, tmpfile);
						fprintf(tmpfile, "%s={\n", param);
						token = strtok(tmp_value, ",");
						do {
							fprintf(tmpfile, "	%s=", token);
							token = strtok(NULL, ",");
							fprintf(tmpfile, "%s\n", token);
							token = strtok(NULL, ",");
						} while (token != NULL);
						fprintf(tmpfile, "}\n");
						total_param_nums++;
						hotspot_set_multiple_param_nums(conf, param, total_param_nums);
					} else
						fputs(buf, tmpfile);
					is_param = 0;
				} else
					fputs(buf, tmpfile);
			}
		}
	
		fclose(tmpfile);
		fclose(file);

		unlink(confname);
		rename(tmp_confname, confname);
	}
}
													

static void hotspot_ctrl_set_plmn_param(struct hotspot_conf *conf,
										const char *confname,
										char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "plmn", value);
}

static void hotspot_ctrl_set_operating_class_param(struct hotspot_conf *conf,
												   const char *confname,
												   char *value)
{
	hotspot_ctrl_set_general_param(confname, "operating_class", value);
}

static void hotspot_ctrl_set_operating_class_id_param(struct hotspot_conf *conf,
													  const char *confname,
													  char *value)
{
	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "81");
	} else if (os_strcmp(value, "2") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "115");
	} else if (os_strcmp(value, "3") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "81,115");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Operating Class ID\n");
	}
}

static void hotspot_ctrl_set_preferred_candi_list_included_param(struct hotspot_conf *conf,
																 const char *confname,
																 char *value)
{
	hotspot_ctrl_set_general_param(confname, "preferred_candi_list_included", value);
}

static void hotspot_ctrl_set_abridged_param(struct hotspot_conf *conf,
											const char *confname,
											char *value)
{
	hotspot_ctrl_set_general_param(confname, "abridged", value);
}

static void hotspot_ctrl_set_disassociation_imminent_param(struct hotspot_conf *conf,
														   const char *confname,
														   char *value)
{
	hotspot_ctrl_set_general_param(confname, "disassociation_imminent", value);
}

static void hotspot_ctrl_set_bss_termination_included_param(struct hotspot_conf *conf,
															const char *confname,
															char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_termination_included", value);
}

static void hotspot_ctrl_set_ess_disassociation_imminent_param(struct hotspot_conf *conf,
															   const char *confname,
															   char *value)
{
	hotspot_ctrl_set_general_param(confname, "ess_disassociation_imminent", value);
}

static void hotspot_ctrl_set_disassociation_timer_param(struct hotspot_conf *conf,
														const char *confname,
														char *value)
{
	hotspot_ctrl_set_general_param(confname, "disassociation_timer", value);
}

static void hotspot_ctrl_set_validity_interval_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "validity_interval", value);
}

static void hotspot_ctrl_set_bss_termination_duration_param(struct hotspot_conf *conf,
															const char *confname,
															char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_termination_duration", value);
}

static void hotspot_ctrl_set_session_information_url_param(struct hotspot_conf *conf,
														   const char *confname,
														   char *value)
{
	hotspot_ctrl_set_general_param(confname, "session_information_url", value);
}

static void hotspot_ctrl_set_bss_transisition_candi_list_preferences_param(struct hotspot_conf *conf,
																		   const char *confname,
																		   char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_transisition_candi_list_preferences", value);
}

static void hotspot_ctrl_set_proto_port_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "proto_port", value);
}

static void hotspot_ctrl_set_con_cap_id_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_proto_port_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,1,port,0,status,0");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,20,status,1");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,22,status,0");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,80,status,1");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,443,status,1");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,1723,status,0");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,5060,status,0");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,500,status,1");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,5060,status,0");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,4500,status,1");
		hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,50,port,0,status,1");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Connection Capability ID\n");
	}
}

static void hotspot_ctrl_set_wan_metrics_param(struct hotspot_conf *conf,
											   const char *confname,
											   char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "wan_metrics", value);
}

static void hotspot_ctrl_set_wan_metrics_id_param(struct hotspot_conf *conf,
												  const char *confname,
												  char *value)
{
	hotspot_ctrl_set_wan_metrics_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,25534,ul_speed,384,dl_load,0,up_load,0,lmd,100");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow WAN Metrics ID\n");
	}
}

static void hotspot_ctrl_set_nai_realm_data_param(struct hotspot_conf *conf,
												  const char *confname,
												  char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "nai_realm_data", value);
}

static void hotspot_ctrl_set_nai_realm_id_param(struct hotspot_conf *conf,
												const char *confname,
												char *value)
{
	hotspot_ctrl_set_nai_realm_data_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,cisco.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,example.com,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "2") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "3") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,cisco.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,example.com,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "4") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-tls,auth_param,5:6");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow NAI Realm List ID\n");
	}
}

/*
 * hotspot related parameters setting
 */
static struct hotspot_ctrl_set_param hs_ctrl_set_params[] = {
	{"interface", hotspot_ctrl_set_interface_param},
	{"interworking", hotspot_ctrl_set_interworking_param},
	{"access_network_type", hotspot_ctrl_set_access_net_type_param},
	{"internet", hotspot_ctrl_set_internet_param},
	{"venue_group", hotspot_ctrl_set_venue_group_param},
	{"venue_type", hotspot_ctrl_set_venue_type_param},
	{"anqp_query", hotspot_ctrl_set_anqp_query_param},
	{"mih_support", hotspot_ctrl_set_mih_support_param},
	{"venue_name", hotspot_ctrl_set_venue_name_param},
	{"venue_name_id", hotspot_ctrl_set_venue_name_id_param},
	{"hessid", hotspot_ctrl_set_hessid_param},
	{"roaming_consortium_oi", hotspot_ctrl_set_roaming_consortium_oi_param},
	{"advertisement_proto_id", hotspot_ctrl_set_advertisement_proto_id_param},
	{"domain_name", hotspot_ctrl_set_domain_name_param},
	{"network_auth_type", hotspot_ctrl_set_network_auth_type_param},
	{"net_auth_type_id", hotspot_ctrl_set_net_auth_type_id_param},
	{"ipv4_type", hotspot_ctrl_set_ipv4_type_param},
	{"ipv6_type", hotspot_ctrl_set_ipv6_type_param},
	{"ip_type_id", hotspot_ctrl_set_ip_type_id_param},
	{"nai_realm_data", hotspot_ctrl_set_nai_realm_data_param},
	{"nai_realm_id", hotspot_ctrl_set_nai_realm_id_param},
	{"op_friendly_name", hotspot_ctrl_set_op_friendly_name_param},
	{"op_friendly_name_id", hotspot_ctrl_set_op_friendly_name_id_param},
	{"proto_port", hotspot_ctrl_set_proto_port_param},
	{"con_cap_id", hotspot_ctrl_set_con_cap_id_param},
	{"wan_metrics", hotspot_ctrl_set_wan_metrics_param},
	{"wan_metrics_id", hotspot_ctrl_set_wan_metrics_id_param},
	{"plmn", hotspot_ctrl_set_plmn_param},
	{"operating_class", hotspot_ctrl_set_operating_class_param},
	{"operating_class_id", hotspot_ctrl_set_operating_class_id_param},
	{"preferred_candi_list_included", hotspot_ctrl_set_preferred_candi_list_included_param},
	{"abridged", hotspot_ctrl_set_abridged_param},
	{"disassociation_imminent", hotspot_ctrl_set_disassociation_imminent_param},
	{"bss_termination_included", hotspot_ctrl_set_bss_termination_included_param},
	{"ess_disassociation_imminent", hotspot_ctrl_set_ess_disassociation_imminent_param},
	{"disassociation_timer", hotspot_ctrl_set_disassociation_timer_param},
	{"validity_interval", hotspot_ctrl_set_validity_interval_param},
	{"bss_termination_duration", hotspot_ctrl_set_bss_termination_duration_param},
	{"session_information_url", hotspot_ctrl_set_session_information_url_param},
	{"bss_transisition_candi_list_preferences", hotspot_ctrl_set_bss_transisition_candi_list_preferences_param},
	{"timezone", hotspot_ctrl_set_timezone_param},
	{"dgaf_disabled", hotspot_ctrl_set_dgaf_disabled_param},
	{"proxy_arp", hotspot_ctrl_set_proxy_arp_param},
	{"l2_filter", hotspot_ctrl_set_l2_filter_param},
	{"icmpv4_deny", hotspot_ctrl_set_icmpv4_deny_param},
	{"p2p_cross_connect_permitted", hotspot_ctrl_set_p2p_cross_connect_permitted_param},
	{"mmpdu_size", hotspot_ctrl_set_mmpdu_size_param},
	{"external_anqp_server_test", hotspot_ctrl_set_external_anqp_server_test_param},
	{"gas_cb_delay", hotspot_ctrl_set_gas_cb_delay_param},
	{"hs2_openmode_test", hotspot_ctrl_set_hs2_openmode_test_param},
}; 

static int hotspot_ctrl_iface_cmd_set(struct hotspot *hs, const char *iface,
									  char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0;
	struct hotspot_conf *conf = NULL;
	int is_found = 0;
	char *token;
	char confname[256];
	struct hotspot_ctrl_set_param *match = NULL;
	struct hotspot_ctrl_set_param *param = hs_ctrl_set_params;

	os_memset(confname, 0, 256);

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (is_found) {
		token = strtok(param_value_pair, " ");
		while (param->param) {
			if (os_strcmp(param->param, token) == 0) {
				match = param;
				break;
			}
			param++;
		}
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Do not find corresponding hotspot configuration for this interface%s\n", iface);
		return -1;
	}
	
	if (match) {
		sprintf(confname, "/etc_ro/hotspot_ap_%s.conf", iface);
		token = strtok(NULL, "");
		match->set_param(conf, confname, token);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "Unkown parameters\n");
		return -1;
	}

	return ret;
}
static int hotspot_ctrl_iface_cmd_btmreq(struct hotspot *hs, const char *iface,
										 char *peer_sta_addr,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char *buf;
	size_t btm_req_len = 0;
	struct hotspot_conf *conf;
	int is_found = 0;
	char peer_addr[6], *token;
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_ERROR, "can not find configuration for interface(%s)\n", iface);
		return -1;
	}

	token = strtok(peer_sta_addr, ":");
	while (token != NULL) {
		AtoH(token, &peer_addr[i], 1);
		DBGPRINT(RT_DEBUG_INFO, "peer_mac[%d] = 0x%02x\n", i, peer_addr[i]);
		i++;
		token = strtok(NULL, ":");
	}

	if (i != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "Wrong mac addr\n");
		return -1;
	}


	btm_req_len = hotspot_calc_btm_req_len(hs, conf);

	buf = os_zalloc(btm_req_len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "Not available memory\n");
		return -1;
	}

	hotspot_collect_btm_req(hs, conf, peer_addr, buf);

	hotspot_send_btm_req(hs, iface, peer_addr, buf, btm_req_len);

	os_free(buf);
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_drv_version(struct hotspot *hs, const char *iface,
										 	  char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_driver_version(hs, iface, reply, reply_len);

	return ret;	
}

static int hotspot_ctrl_iface_cmd_ipv4_proxy_arp_list(struct hotspot *hs, const char *iface,
										 		 char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_ipv4_proxy_arp_list(hs, iface, reply, reply_len);

	return ret;
}

static int hotspot_ctrl_iface_cmd_ipv6_proxy_arp_list(struct hotspot *hs, const char *iface,
										 		 char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_ipv6_proxy_arp_list(hs, iface, reply, reply_len);

	return ret;
}

static int hotspot_ctrl_iface_cmd_reload(struct hotspot *hs, const char *iface,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	
	if (hs->opmode == OPMODE_AP)
		ret = hotspot_ap_reload(hs, iface);
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_process(struct hotspot *hs, char *buf, 
											char *reply, size_t *reply_len)
{
	int ret = 0;
	char *token, *token1;
	char tmp[256];
	char iface[256], cmd[256];
	int linelen = 0;


	token = strtok(buf, "\n");

	os_memset(tmp, 0, 256);
	os_memset(iface, 0, 256);
	os_memset(cmd, 0, 256);

	while (token != NULL) {
		linelen = os_strlen(token);
		os_strncpy(tmp, token, 256);
		tmp[255] = '\0';

		token1 = strtok(tmp, "=");

		if (os_strcmp(token1, "interface") == 0) {
			token1 = strtok(NULL, "");
			os_strncpy(iface, token1, 256);
			iface[255] = '\0';
		} else if (os_strcmp(token1, "cmd") == 0) {
			token1 = strtok(NULL, "");
			os_strncpy(cmd, token1, 256);
			cmd[255] = '\0';
		}

		token = strtok(token + linelen + 1, "\n");
	}

	os_sleep(0, 5000);

	DBGPRINT(RT_DEBUG_TRACE, "interface = %s, cmd = %s\n", iface, cmd);

	if (os_strcmp(cmd, "hs_version") == 0)
		ret = hotspot_ctrl_iface_cmd_version(hs, reply, reply_len);
	else if (os_strcmp(cmd, "on") == 0)
		ret = hotspot_ctrl_iface_cmd_on(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "off") == 0)
		ret = hotspot_ctrl_iface_cmd_off(hs, iface, reply, reply_len);
	else if (os_strncmp(cmd, "get", 3) == 0)
		ret = hotspot_ctrl_iface_cmd_get(hs, iface, cmd + 4, reply, reply_len);
	else if (os_strncmp(cmd, "set", 3) == 0)
		ret = hotspot_ctrl_iface_cmd_set(hs, iface ,cmd + 4, reply, reply_len);
	else if (os_strncmp(cmd, "btmreq", 6) == 0)
		ret = hotspot_ctrl_iface_cmd_btmreq(hs, iface, cmd + 7, reply, reply_len);
	else if (os_strncmp(cmd, "drv_version", 11) == 0)
		ret = hotspot_ctrl_iface_cmd_drv_version(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "ipv4_proxy_arp_list") == 0)
		ret = hotspot_ctrl_iface_cmd_ipv4_proxy_arp_list(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "ipv6_proxy_arp_list") == 0)
		ret = hotspot_ctrl_iface_cmd_ipv6_proxy_arp_list(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "reload") == 0)
		ret = hotspot_ctrl_iface_cmd_reload(hs, iface, reply, reply_len);
	else {
		DBGPRINT(RT_DEBUG_ERROR, "no such command\n");
		ret = -1;
	}

	if (ret == 0 && reply_len > 0)
		ret = 1;

	return ret;
}

static void hotspot_ctrl_iface_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct hotspot *hs = eloop_ctx;
	struct hotspot_ctrl_iface *ctrl_iface = sock_ctx;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	int receive_len;
	char buf[256];
	size_t replylen = 2047;
	char reply[2048];
	int ret;

	os_memset(buf, 0, 256);
	os_memset(reply, 0, 2048);

	receive_len = recvfrom(sock, buf, sizeof(buf) - 1, 0, 
							(struct sockaddr *)&from, &fromlen);

	if (receive_len < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "receive from control interface fail\n");
		return;
	}

	buf[receive_len] = '\0';

	if (os_strcmp(buf, "EVENT_REGISTER") == 0) {
		if (hotspot_ctrl_iface_event_register(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);

	} else if(os_strcmp(buf , "EVENT_UNREGISTER") == 0) {
		if (hotspot_ctrl_iface_event_unregister(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
	} else {
			ret = hotspot_ctrl_iface_cmd_process(hs, buf, reply, &replylen);
		if (ret == -1)
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else if (ret == 0)
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, reply, replylen, 0, (struct sockaddr *)&from, fromlen);
	}

	return;
}

struct hotspot_ctrl_iface *hotspot_ctrl_iface_init(struct hotspot *hs)
{
	struct hotspot_ctrl_iface *ctrl_iface;
	struct sockaddr_un addr;

	ctrl_iface = os_zalloc(sizeof(*ctrl_iface));

	if (!ctrl_iface) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		goto error0;
	}

	dl_list_init(&ctrl_iface->hs_ctrl_dst_list);
	ctrl_iface->sock = -1;

	ctrl_iface->sock = socket(PF_UNIX, SOCK_DGRAM, 0);

	if (ctrl_iface->sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "create socket for ctrl interface fail\n");
		goto error1;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	
	os_strncpy(addr.sun_path, "/tmp/hotspot", sizeof(addr.sun_path));

	if (bind(ctrl_iface->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "bind addr to ctrl interface fail\n");
		goto error2;
	}

	eloop_register_read_sock(ctrl_iface->sock, hotspot_ctrl_iface_receive, hs, 
										ctrl_iface);

	return ctrl_iface;

error2:
	close(ctrl_iface->sock);
error1:
	os_free(ctrl_iface);
error0:
	return NULL;
}

void hotspot_ctrl_iface_deinit(struct hotspot *hs)
{
	struct hotspot_ctrl_iface *ctrl_iface = hs->hs_ctrl_iface;
	struct hotspot_ctrl_dst	*ctrl_dst, *ctrl_dst_tmp;

	eloop_unregister_read_sock(ctrl_iface->sock);
		
	close(ctrl_iface->sock);
	ctrl_iface->sock = -1;
	unlink("/tmp/hotspot");

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
									struct hotspot_ctrl_dst, list) {
		os_free(ctrl_dst);
	}

	os_free(ctrl_iface);
}
