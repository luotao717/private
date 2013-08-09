/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//           main of the ddns part
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "ddns.h"

int SYS_wan_ip;

int main(int argc,char *argv[]) 
{
	struct userInfo_cxy info;
	int ret;
	if(argc!=7 && argc!=11)	{
		usage();
		return -1;
	}

	if(strcmp(argv[1],"-s") && strcmp(argv[1],"-m")) {
		usage();
		return -1;
	}

	info.service = (struct service_cxy *)find_service((char *)argv[2]);
	strcpy(info.host, argv[3]);
	strcpy(info.usrname, argv[4]);
	strcpy(info.usrpwd, argv[5]);
	info.ip=inet_addr(argv[6]);
	SYS_wan_ip=inet_addr(argv[6]);
	//printf("info.ip=%d\n", info.ip);
	//printf("SYS_wan_ip=%d\n", SYS_wan_ip);

	if(!strcmp(argv[1],"-s")) {
		strcpy(info.mx, "0");
		strcpy(info.backmx,"0");
		info.updated_time=0;
		info.trytime=0;
	}
	else {
		strcpy(info.mx, argv[7]);
		strcpy(info.backmx,argv[8]);
		info.updated_time = atoi(argv[9]);
		info.trytime = atoi(argv[10]);
	}

	if(info.ip)
		info.status = UPDATERES_OK;
	else
		info.status = UPDATERES_ERROR;

	//printf("userIp=%s=====%d\r\n",info.ip,argc);
	//printf("sys_wan_ip=%s\n\r",SYS_wan_ip);

	ret = info.status = info.service->update_entry(&info);
	if (!ret)
		system("echo 1 >/var/ddns_ok");
	else
		system("rm -f /var/ddns_ok");
	return ret;
}



