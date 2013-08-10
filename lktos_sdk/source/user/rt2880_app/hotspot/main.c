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
	driver_ranl.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "hotspot.h"

extern struct hotspot_event_ops hs_event_ops;


int usage()
{

	DBGPRINT(RT_DEBUG_OFF, "hotspot [-f <hotspot configuration file>] [-m <hotspot mode>] [-i <hotspot ipc type>] [-d <debug level>]\n");
	DBGPRINT(RT_DEBUG_OFF, "-f <hotspot configuration file>\n");
	DBGPRINT(RT_DEBUG_OFF, "-m <hotspot mode> (OPMODE_STA, OPMODE_AP)\n");
	DBGPRINT(RT_DEBUG_OFF, "-i <hotspot ipc type> (RA_WEXT, RA_NETLINK)\n");
	DBGPRINT(RT_DEBUG_OFF, "-d <hotspot debug level>\n");
	DBGPRINT(RT_DEBUG_OFF, "-h help\n");
	return 0;
}

int process_options(int argc, char *argv[], char *filename,
					int *opmode, int *drv_mode, int *debug_level)
{
	int c;
	char *cvalue = NULL;
	
	opterr = 0;

	while ((c = getopt(argc, argv, "m:f:i:d:")) != -1) {
		switch (c) {
		case 'd':
			cvalue = optarg;
			if (os_strcmp(cvalue, "0") == 0)
				*debug_level = RT_DEBUG_OFF;
			else if (os_strcmp(cvalue, "1") == 0)
				*debug_level = RT_DEBUG_ERROR;
			else if (os_strcmp(cvalue, "2") == 0)
				*debug_level = RT_DEBUG_WARN;
			else if (os_strcmp(cvalue, "3") == 0)
				*debug_level = RT_DEBUG_TRACE;
			else if (os_strcmp(cvalue, "4") == 0)
				*debug_level = RT_DEBUG_INFO;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-d option does not have this debug_level %s\n", cvalue);
				return - 1;
			}
			break;
		case 'f':
			cvalue = optarg;
			os_strcpy(filename, cvalue);
			break;
		case 'm':
			cvalue = optarg;
			if (os_strcmp(cvalue, "OPMODE_STA") == 0)
				*opmode = OPMODE_STA;
			else if (os_strcmp(cvalue, "OPMODE_AP") == 0)
				*opmode = OPMODE_AP;
			else {
				DBGPRINT(RT_DEBUG_ERROR, "-m option does not have this mode %s\n", cvalue);
				return -1;
			}
			break;
		case 'i':
			cvalue = optarg;
			if (os_strcmp(cvalue, "RA_WEXT") == 0)
				*drv_mode = RA_WEXT;
			else if (os_strcmp(cvalue, "RA_NETLINK") == 0)
				*drv_mode = RA_NETLINK;
			else {
				DBGPRINT(RT_DEBUG_OFF, "-i option does not have this type %s\n", cvalue);
				return -1;
			}
			break;
		case 'h':
			cvalue = optarg;
			usage();
			break;
		case '?':
			if (optopt == 'f') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'm') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'd') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (optopt == 'i') {
				DBGPRINT(RT_DEBUG_OFF, "Option -%c requires an argument\n", optopt);
			} else if (isprint(optopt)) {
				DBGPRINT(RT_DEBUG_OFF, "Unknow options -%c\n", optopt);
			} else {

			}
			return -1;
			break;
		}
	}
	return 0;

}

int main(int argc, char *argv[])
{

	int ret;
	int opmode;
	int drv_mode;
	int debug_level;
	char filename[256] = {0}; 
	struct hotspot hs;
	pid_t child_pid;

	/* default setting */
	opmode = OPMODE_AP;
	drv_mode = RA_WEXT;

	/* options processing */
	ret = process_options(argc, argv, filename, &opmode, &drv_mode, &debug_level);

	if (ret) {
		usage();
		return -1;
	}

	RTDebugLevel = debug_level;
	
	child_pid = fork();

	if (child_pid == 0) {	
		DBGPRINT(RT_DEBUG_OFF, "Initialize hotspot\n");
		ret = hotspot_init(&hs, &hs_event_ops, drv_mode, opmode);
	
		if (ret)
			goto error;

		if (opmode == OPMODE_STA) {
			if (os_strlen(filename) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "Use default configuration file /etc_ro/hotspot_sta.conf");
				os_strcpy(filename, "/etc_ro/hotspot_sta.conf");
			}
			ret = hotspot_init_all_config(&hs, filename);

		} else {
			if (os_strlen(filename) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "Use default configuration file /etc_ro/hotspot_ap.conf");
				os_strcpy(filename, "/etc_ro/hotspot_ap.conf");
			}
			ret = hotspot_init_all_config(&hs, filename);
		}

		if (ret) {
			DBGPRINT(RT_DEBUG_OFF, "Initial hotspot configuration file(%s) fail\n", filename);
			goto error0;
		}

		if (opmode == OPMODE_STA)
			ret = hotspot_set_sta_ifaces_all_ies(&hs);
		else
			ret = hotspot_set_ap_ifaces_all_ies(&hs);

		if (ret)
			goto error1;

		/* Enable hotspot feature for all interfaces */
		ret = hotspot_onoff_all(&hs, 1);

		if (ret)
			goto error2;

		hotspot_run(&hs);

		/* Disable hotspot feature for all interfaces */
		hotspot_onoff_all(&hs, 0);
	} else
		return 0;

error2:
	hotspot_reset_all_ap_resource(&hs);
error1:
	hotspot_deinit_all_config(&hs);
error0:
	hotspot_deinit(&hs);
error:
	
	return ret;

}
