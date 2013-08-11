/*
 * hotplug_net.c - hotplug net dispatch routine
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */
#include "hotplug.h"


#define IFC_BRIDGE_NAME		 "br0"


int
hotplug_net(void)
{
	char *interface, *action;
    	char cmd[80];
//    	char macaddr[ETHER_HDR_LEN];

	if (!(interface = getenv("INTERFACE")) ||
	    !(action = getenv("ACTION")))
		return EINVAL;


	/* Bridge WDS interfaces */
	if (!strncmp(interface, "wds", 3)) {
		if (!strcmp(action, "register")||!strcmp(action, "add")) {
						
			/* add interface to bridge */
       			sprintf(cmd, "ifconfig %s up", interface);
	   		bcmSystem(cmd);
       			sprintf(cmd, "brctl addif %s %s", IFC_BRIDGE_NAME, interface);
       			bcmSystem(cmd);

			/* enable spanning tree protocol */
       			sprintf(cmd, "brctl stp %s on", IFC_BRIDGE_NAME);
       			bcmSystem(cmd);	

   			/* send ARP packet with bridge IP and hardware address to device
   			 * this piece of code is -required- to make br0's mac work properly
   			 */
   			sprintf(cmd, "sendarp -s %s -d %s", IFC_BRIDGE_NAME, IFC_BRIDGE_NAME);
   			bcmSystem(cmd);
               
       			/* Notify NAS of adding the interface */
			notify_nas("lan", interface, "up");
		
		       	return 0;
		}
	}

	return 0;
}
