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
	hotspot_cli.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot_ctrl.h"
#include "hotspot_cli.h"

static struct hotspot_ctrl *ctrl_conn;

int RTDebugLevel = RT_DEBUG_TRACE;

static int hotspot_cli_open_connection(const char *ctrl_path)
{
	ctrl_conn = hotsot_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		DBGPRINT(RT_DEBUG_ERROR, "hotspot_ctrl_open fail\n");
		return -1;
	}

	return 0;
}

static void hotspot_cli_close_connection(void)
{
	hotspot_ctrl_close(ctrl_conn);
	ctrl_conn = NULL;
}

static int _hotspot_ctrl_command(struct hotspot_ctrl *ctrl, const char *cmd, 
													char *rsp, size_t *rsp_len)
{
	int ret;

	if (ctrl_conn == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "Connect to hotspot daemon first\n");
		return -1;
	}

	ret = hotspot_ctrl_command(ctrl, cmd, os_strlen(cmd), rsp, rsp_len);

	if (ret == -2) {
		DBGPRINT(RT_DEBUG_ERROR, "Timeout\n");
		return -2;
	} else if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Command fail\n");
		return -1;
	}

	return 0;
}

static int hotspot_cli_cmd_help(struct hotspot_ctrl *ctrl, int argc, char *argv[]);

static int hotspot_cli_cmd_version(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=hs_version\n");

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);

	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static int hotspot_cli_cmd_on(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=on\n");

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_off(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret = 0;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=off\n");

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);

	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static struct hotspot_cli_get_param hs_cli_get_params[] = {
	{"all", "All Parameters"},
};

static struct hotspot_cli_set_param hs_cli_set_params[] = {
	{"interface", "Interface name", STRING_TYPE},
	{"interworking", "Interworing (Enabled(1) / Disabled(0))", INTEGER_TYPE},
	{"access_network_type", "Hotspot Access Network Type", STRING_TYPE},
	{"internet", "Hotspot Internet Support (Enabled(1)/ Disabled(0))", INTEGER_TYPE},
	{"venue_group", "Venue Group Information", STRING_TYPE},
	{"venue_type", "Venue Type Information", STRING_TYPE},
	{"anqp_query", "ANQP Query (Enabled(1) / Disabled(0))", INTEGER_TYPE}, 
	{"mih_support", "MIH Information Service Advertisement Protocol Enabled/Disabled", INTEGER_TYPE},
	{"venue_name", "Venue Name", STRING_TYPE},
	{"venue_name_id", "Venue Name ID", INTEGER_TYPE},
	{"hessid", "HESSID", STRING_TYPE},
	{"roaming_consortium_oi", "Roaming Consortium OI (Hex)", STRING_TYPE},
	{"advertisement_proto_id", "Advertisement Protocol ID", INTEGER_TYPE},
	{"domain_name", "Domain Name", STRING_TYPE},
	{"network_auth_type", "Network Auth Type", STRING_TYPE},
	{"net_auth_type_id", "Network Auth Type ID", INTEGER_TYPE},
	{"ipv4_type", "IPV4 Type", INTEGER_TYPE},
	{"ipv6_type", "IPV6 Type", INTEGER_TYPE},
	{"ip_type_id", "IP Address Type Availability ID", INTEGER_TYPE},
	{"nai_realm_data", "NAI Realm Data", STRING_TYPE},
	{"nai_realm_id", "NAI Realm List ID", INTEGER_TYPE},
	{"op_friendly_name", "Operator Friendly Name", STRING_TYPE},
	{"op_friendly_name_id", "Operator Friendly Name ID", INTEGER_TYPE},
	{"proto_port", "Protocol Port", STRING_TYPE},
	{"con_cap_id", "Connection Capability Element ID", INTEGER_TYPE},
	{"wan_metrics", "WAN Metrics", STRING_TYPE},
	{"wan_metrics_id", "WAN Metrics ID", INTEGER_TYPE},
	{"plmn", "3GPP cellular network information", STRING_TYPE},
	{"operating_class", "Operating Class information", STRING_TYPE},
	{"operating_class_id", "Operating Class ID", STRING_TYPE},
	{"preferred_candi_list_included", "Preferred Candidate List", INTEGER_TYPE},
	{"abridged", "Abridged", INTEGER_TYPE},
	{"disassociation_imminent", "Disassiciation Imminent", INTEGER_TYPE},
	{"bss_termination_included", "BSS Termination Included", INTEGER_TYPE},
	{"ess_disassociation_imminent", "ESS Disassociation Imminent", INTEGER_TYPE},
	{"disassociation_timer", "Disassociation Timer", INTEGER_TYPE},
	{"validity_interval", "Validity Interval", INTEGER_TYPE},
	{"bss_termination_duration", "BSS Termination Duration", INTEGER_TYPE},
	{"session_information_url", "Session Information URL", STRING_TYPE},
	{"bss_transisition_candi_list_preferences", "BSS Transition Candidates List Preferences", STRING_TYPE},
	{"timezone", "Time Zone", STRING_TYPE},
	{"dgaf_disabled", "Downstream Group addressed Forwarding Disabled Bit (0/1)", INTEGER_TYPE},
	{"proxy_arp", "Proxy ARP Support (Enabled(1) / Disabled(0)", INTEGER_TYPE},
	{"l2_filter", "L2 Traffic Inspection and Filtering", INTEGER_TYPE},
	{"icmpv4_deny", "Deny icmpv4 packet", INTEGER_TYPE},
	{"p2p_cross_connect_permitted", "P2P Cross Connect Permitted", INTEGER_TYPE},
	{"mmpdu_size", "MMPDU Size", INTEGER_TYPE},
	{"external_anqp_server_test", "External ANQP Server Test", INTEGER_TYPE},
	{"gas_cb_delay", "GAS Comeback Delay in TUs", INTEGER_TYPE},
	{"hs2_openmode_test", "Test HS2 Under Open Security Mode", INTEGER_TYPE},
}; 

static int hotspot_cli_cmd_get(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	struct hotspot_cli_get_param *param = hs_cli_get_params;
	struct hotspot_cli_get_param *match = NULL;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	while (param->param) {
		if (os_strcmp(param->param, argv[2]) == 0) {
			match = param;
			break;
		}
		
		param++;
	}

	if (match) {
		sprintf(&cmd[i], "interface=%s\n", argv[0]);
		i += 11 + os_strlen(argv[0]);

		sprintf(&cmd[i], "cmd=get %s", argv[2]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown parameter\n");
		return -1;
	}

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_set(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	struct hotspot_cli_set_param *param = hs_cli_set_params;
	struct hotspot_cli_set_param *match = NULL;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	while (param->param) {
		if (os_strcmp(param->param, argv[2]) == 0) {
			match = param;
			break;
		}
		
		param++;
	}

	if (match) {
		sprintf(&cmd[i], "interface=%s\n", argv[0]);
		i += 11 + os_strlen(argv[0]);

		sprintf(&cmd[i], "cmd=set %s %s", argv[2], argv[3]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown parameter\n");
		return -1;
	}

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);
	
	return ret;
}

static int hotspot_cli_cmd_btmreq(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=btmreq %s", argv[2]);

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_drv_version(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);	
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=drv_version\n");

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_OFF, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static int hotspot_cli_cmd_ipv4_proxy_arp_list(struct hotspot_ctrl *ctrl, 
												int argc, char *argv[])
{
	char cmd[256];
	int i = 0, j = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	struct proxy_arp_ipv4_unit *proxy_arp_unit;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=ipv4_proxy_arp_list\n");

	ret =  _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';

	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\n", cmd);
	
	DBGPRINT_RAW(RT_DEBUG_OFF, "MAC\t IPv4 Address\n");
	
	proxy_arp_unit = (struct proxy_arp_ipv4_unit *)rsp;

	for (i = 0; i < (rsp_len / sizeof(*proxy_arp_unit)); i++) {
		for (j = 0; j < 6; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_mac_addr[j]);

		 DBGPRINT_RAW(RT_DEBUG_OFF, "\t");
	
		for (j = 0; j < 4; j++) {
			if (j == 3) {
				DBGPRINT_RAW(RT_DEBUG_OFF, "%d", proxy_arp_unit->target_ip_addr[j]);
			} else {
				DBGPRINT_RAW(RT_DEBUG_OFF, "%d.", proxy_arp_unit->target_ip_addr[j]);
			}
		}
		
	 	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
		proxy_arp_unit++;
	}
	
	return ret;
}

static int hotspot_cli_cmd_ipv6_proxy_arp_list(struct hotspot_ctrl *ctrl, 
												int argc, char *argv[])
{
	char cmd[256];
	int i = 0, j = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;
	struct proxy_arp_ipv6_unit *proxy_arp_unit;
	
	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;

	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);
	
	sprintf(&cmd[i], "cmd=ipv6_proxy_arp_list\n");

	ret =  _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';

	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\n", cmd);

	DBGPRINT_RAW(RT_DEBUG_OFF, "IPv6 Type\t MAC\t IPv6 Address\n");
	
	proxy_arp_unit = (struct proxy_arp_ipv6_unit *)rsp;

	for (i = 0; i < (rsp_len / sizeof(*proxy_arp_unit)); i++) {

		if (proxy_arp_unit->target_ip_type == 0) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "Link Local\t");
		} else {
			DBGPRINT_RAW(RT_DEBUG_OFF, "Global\t");
		}

		for (j = 0; j < 6; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_mac_addr[j]);

		 DBGPRINT_RAW(RT_DEBUG_OFF, "\t");
	
		for (j = 0; j < 16; j++)
			DBGPRINT_RAW(RT_DEBUG_OFF, "%02x ", proxy_arp_unit->target_ip_addr[j]);
		
	 	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
		proxy_arp_unit++;
	}
	
	return ret;
}

static int hotspot_cli_cmd_reload(struct hotspot_ctrl *ctrl,
								  int argc, char *argv[])
{
	char cmd[256];
	int i = 0, ret;
	char rsp[2048];
	size_t rsp_len = 0;

	os_memset(cmd, 0, 256);
	os_memset(rsp, 0, 2048);
	rsp_len = sizeof(rsp) - 1;
	
	sprintf(&cmd[i], "interface=%s\n", argv[0]);
	i += 11 + os_strlen(argv[0]);

	sprintf(&cmd[i], "cmd=reload\n");

	ret = _hotspot_ctrl_command(ctrl, cmd, rsp, &rsp_len);
	
	rsp[rsp_len] = '\0';
	DBGPRINT(RT_DEBUG_TRACE, "\ncmd = {\n%s}\nrsp = {\n%s\n}\n", cmd, rsp);

	return ret;
}

static struct hotspot_cli_cmd hs_cli_cmds[] = {
	{"help", hotspot_cli_cmd_help, "command usage"},
	{"hs_version", hotspot_cli_cmd_version, "show hotspot daemon version"},
	{"on", hotspot_cli_cmd_on, "enable hotspot"},
	{"off", hotspot_cli_cmd_off, "disable hotspot"},
	{"get [param]", hotspot_cli_cmd_get, "get hotspot parameter"},
	{"set [param] [value]", hotspot_cli_cmd_set, "set hotspot parameter"},
	{"btmreq [peer_mac_addr]", hotspot_cli_cmd_btmreq,
		"send bss transition request frame to peer address"},
	{"drv_version", hotspot_cli_cmd_drv_version, "show wifi driver version"},
	{"ipv4_proxy_arp_list", hotspot_cli_cmd_ipv4_proxy_arp_list, "show ipv4 proxy arp table"},
	{"ipv6_proxy_arp_list", hotspot_cli_cmd_ipv6_proxy_arp_list, "show ipv6 proxy arp table"},
	{"reload", hotspot_cli_cmd_reload, "reload all configuration"},
};

static int hotspot_cli_cmd_help(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	struct hotspot_cli_cmd *cmd;
	int cmd_num = sizeof(hs_cli_cmds) / sizeof(struct hotspot_cli_cmd);
	int i;

	cmd = hs_cli_cmds;
	DBGPRINT_RAW(RT_DEBUG_OFF, "hsctrl [interface] [cmd] [args]\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "Command Usage:\n");
	
	for (i = 0; i < cmd_num; i++, cmd++) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "  %-30s %-80s\n", cmd->cmd, cmd->usage);
	}

	return 0;
}

static int hotspot_cli_request(struct hotspot_ctrl *ctrl, int argc, char *argv[])
{
	struct hotspot_cli_cmd *cmd, *match = NULL;
	int ret = 0;

	cmd = hs_cli_cmds;

	while (cmd->cmd) {
		if (os_strncmp(cmd->cmd, argv[1], os_strlen(argv[1])) == 0) {
			match = cmd;
			break;
		}
		cmd++;
	}

	if (match) {
		ret = match->cmd_handler(ctrl, argc, &argv[0]);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknown command\n");
		ret = -1;
	}

	return ret;
}

int optind = 1;
int main(int argc, char *argv[])
{
	int ret = 0;

	hotspot_cli_open_connection("/tmp/hotspot");

	ret = hotspot_cli_request(ctrl_conn, argc - optind, &argv[optind]);

	hotspot_cli_close_connection();

	return ret;
}
