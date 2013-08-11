/*
 * hotplug_usb_lp.c - hotplug usb printer dispatch routine
 *
 * Copyright 2007, T&W
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of T&W;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of T&W.                            
 *
 * Create by leida@twsz.com
 *
 * $Id$
 */
#include "hotplug.h"

#define	PRINTER_NAME	"usblp"
#define	FIRMWARE_PATH	"/etc/hotplug/firmware/"
#define	PRINTER_DEV	"/dev/printer0"

typedef struct {
	int	vendorid;
	int	productid;
	char *firmware;
}TproductTable;
static TproductTable g_printerTable[] = {
	{0x03F0, 0x2B17, "hp1020.dl"}, /* hp LaserJet 1020 */
};

#define	RETRY_TIMES	(3)
#define	SLEEP_SEC	(5)
/*
* function: load the correct firmware to "todev" which maching vendorid & productid in g_printerTable[] giving.
*
* vendorid: printer "vendorid"
* productid: printer "productid"
* *todev: the device file whole path. such as: "/dev/printer0"
*/
static int hotplug_usb_lp_firmware_load(unsigned int vendorid, unsigned int productid, const char *todev)
{
	int i;
	int fd;
	int times;
	char cmd[128];
	char *pToDev = todev;
	int flgSucceed = 0;

	for (i=0; i < sizeof(g_printerTable)/sizeof(g_printerTable[0]); i++) {
		if ( (vendorid == g_printerTable[i].vendorid)
			&& (productid == g_printerTable[i].productid) )
		{
			for (times = 0; (times < RETRY_TIMES) && (!flgSucceed); times++) {				
				fd = open(pToDev, O_RDWR);
				switch( fd ){
					case -1:
						/* always need to sleep a while for system preparing */
						sleep(SLEEP_SEC);						
						break;
					case 1:
						close(fd);
						break;
					default:
						flgSucceed = 1;
						close(fd);
						break;
				} /* end switch */
			}
			
			if (flgSucceed){			
				sprintf(cmd, "cat %s%s > %s", FIRMWARE_PATH, g_printerTable[i].firmware, pToDev);
				bcmSystem(cmd);
			}
			break;
		} /* end if */
	}/* end for */
	return flgSucceed;
}

int hotplug_usb_lp(void)
{
	char *action, *pVendorid, *pProductid;
	unsigned int vendorid, productid;
	
	/* env from usblp.c hotplug*/
	if (!(action = getenv("ACTION"))
	    || !(pVendorid = getenv("VENDORID"))
	    || !(pProductid = getenv("PRODUCTID")) )
	    return EINVAL;
	
	if ( 0 == strcmp(action, "add") ) {
		vendorid = strtoul(pVendorid, NULL, 16);
		productid = strtoul(pProductid, NULL, 16);

		/* firmware load */
		hotplug_usb_lp_firmware_load(vendorid, productid, PRINTER_DEV);
	}/* end if */

	return 0;
}
