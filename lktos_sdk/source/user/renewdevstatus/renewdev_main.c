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

int get_mib(char *val, char *mib)
{
	FILE *fp;
 	char buf[32];

	sprintf(buf, "nvram_get 2860 %s", mib);
    	fp = popen(buf, "r");
	if (fp==NULL)
		return -1;

	if (NULL == fgets(buf, sizeof(buf),fp)) {
		pclose(fp);
		return -1;
	}

	//strcpy(val, strstr(buf, "\"")+1);
	strcpy(val, buf);
	val[strlen(val)-1] = '\0';
	pclose(fp);
	return 0;
}


int SYS_wan_ip;
static int powerStatusrenew=0;

int main(int argc,char *argv[]) 
{
	struct userInfo_cxy info;
	char devsn[64]={0};
	char email[64]={0};
	int ret;
	if(argc!=7 && argc!=11 && argc!=8 && argc!=4)	{
		usage();
		return -1;
	}

	if(strcmp(argv[1],"-s") && strcmp(argv[1],"-m")&& strcmp(argv[1],"-o")) {
		usage();
		return -1;
	}
	
	info.service = (struct service_cxy *)find_service((char *)argv[2]);
	get_mib(email,"BandUserEmail");
	get_mib(devsn,"devsn");
	strcpy(info.host, email);
	strcpy(info.usrname, "admin");
	strcpy(info.usrpwd, "admin");
	if(!strcmp(argv[1],"-s")) 
	{
		strcpy(info.mx, "0");
		strcpy(info.backmx,"0");
		info.updated_time=0;
		info.trytime=0;
		info.ip=inet_addr(argv[6]);
		SYS_wan_ip=inet_addr(argv[6]);
	}
	else if(!strcmp(argv[1],"-o")) 
	{
		strcpy(info.backmx, devsn);
		strcpy(info.mx, argv[3]);
		powerStatusrenew=atoi(argv[3]);
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

	
	while(1)
	{
		sleep(1);
		ret = info.status = info.service->update_entry(&info);
		if (0==ret)
		{
			if(powerStatusrenew)
			{
				system("getinternet -o orange 1 &");
				system("echo 1 > /var/serverStatus");
			}
			else
			{
				system("getinternet -o orange 0 &");
				system("echo 0 > /var/serverStatus");
			}
			break;
		}
		else
		{
			continue;
		}
	}
	return ret;
}


