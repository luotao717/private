#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/stat.h>
#include 	<sys/types.h>
#include 	<sys/socket.h>
#include  <sys/ioctl.h>
#include 	<netinet/in.h>
#include 	<fcntl.h>
#include 	<arpa/inet.h>
#include 	<net/if_arp.h>
#include 	<net/if.h>
#include	<net/route.h>
#include 	<dirent.h>
#include 	<unistd.h>
#include 	<string.h>
#include 	<errno.h>
#include 	<time.h>
#include 	<assert.h>
#include 	<netinet/tcp.h> 
#include	<sys/ipc.h>
#include	<sys/sem.h>
#include 	<syslog.h>
#include	"nvram.h"
#include 	"cmd.h"

#define TXBYTE		0
#define TXPACKET	1
#define RXBYTE		2
#define RXPACKET	3
#define PROC_IF_STATISTIC	"/proc/net/dev"
#define commit_time 60


static inline char *stripspace(char *str)
{
	while( *str == ' ')
		str++;
	return str;
}

char* getifField(char *a_line, char *delim, int count)
{
	int i=0;
	char *tok;
	tok = strtok(a_line, delim);
	while(tok){
		if(i == count)
			break;
        i++;
		tok = strtok(NULL, delim);
    }
    if(tok && isdigit(*tok))
		return tok;

	return NULL;
}

static long long getWanStatistic(char *interface, int type)
{
	int found_flag = 0;
	int skip_line = 2;
	char buf[1024], *field, *semiColon = NULL;
	FILE *fp = fopen(PROC_IF_STATISTIC, "r");
	if(!fp){
		printf("no proc?\n");
		return -1;
	}

	while(fgets(buf, 1024, fp)){
		char *ifname;
		if(skip_line != 0){
			skip_line--;
			continue;
		}
		if(! (semiColon = strchr(buf, ':'))  )
			continue;
		*semiColon = '\0';
		ifname = buf;
		ifname = stripspace(ifname);

		if(!strcmp(ifname, interface)){
			found_flag = 1;
			break;
		}
	}
	fclose(fp);

	if(found_flag==0)
		return 0;

	semiColon++;

	switch(type){
	case TXBYTE:
		if(  (field = getifField(semiColon, " ", 8))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case TXPACKET:
		if(  (field = getifField(semiColon, " ", 9))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXBYTE:
		if(  (field = getifField(semiColon, " ", 0))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	case RXPACKET:
		if(  (field = getifField(semiColon, " ", 1))  ){
			return strtoll(field, NULL, 10);
		}
		break;
	}
	return 0;
}

static int File_Set_Wan_Flow(char *name,char *value)
{
	FILE *fp=NULL;
	char path[64];

	memset(path,'\0',64);
	sprintf(path,"%s%s","/var/",name);

	fp = fopen(path,"w+");
	if(fp == NULL)
	{
		return -1;
	}		
	else
	{
		if(value)
			fprintf ( fp, "%s", value);
		else
			fprintf( fp, "");
	}	
	
	fclose(fp);
	fp = NULL;
	return 0;
}


int main(int argc, char *argv[])
{
	char *wan_mode=NULL, *if_name=NULL, *his_flow_up=NULL, *his_flow_down=NULL;
	long long data_tx=0, data_rx=0,old_tx=0,old_rx=0,flow_up=0,flow_down=0;
	static int flow_time=0;
	char cmd[32];
	
	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}
	
	while(1)
	{
		wan_mode=nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
		if(!wan_mode)
			return;
	
		if( !strcmp(wan_mode, "STATIC"))
			if_name = "eth2";
		else if( !strcmp(wan_mode, "DHCP"))
			if_name = "eth2";
		else if( !strcmp(wan_mode, "PPPOE"))
			if_name = "ppp0";
		else if( !strcmp(wan_mode, "3G"))
			if_name = "ppp0";
		else if( !strcmp(wan_mode, "WifiAccess"))
			if_name = "ra0";
	
		data_tx = getWanStatistic(if_name, TXBYTE);
		data_rx = getWanStatistic(if_name, RXBYTE);
		
		his_flow_up = nvram_bufget(RT2860_NVRAM, "hisflowrateup");
		if(!his_flow_up||!strlen(his_flow_up))
		{
			File_Set_Wan_Flow("wan_Flow_up","0");
			nvram_bufset(RT2860_NVRAM, "hisflowrateup", "0");
			nvram_commit(RT2860_NVRAM);
		}
		else
		{
			if(flow_up==0)
				flow_up=atoi(his_flow_up)+(data_tx/1024-old_tx);
			else
				flow_up=flow_up+(data_tx/1024-old_tx);
			old_tx=data_tx/1024;
			memset(cmd,'\0',sizeof(cmd));
			sprintf(cmd,"%d",flow_up);
			File_Set_Wan_Flow("wan_Flow_up",cmd);
			if(flow_time==commit_time)
				nvram_bufset(RT2860_NVRAM, "hisflowrateup", cmd);
		}
		
		
		his_flow_down = nvram_bufget(RT2860_NVRAM, "hisflowratedown");
		if(!his_flow_down||!strlen(his_flow_down))
		{
			File_Set_Wan_Flow("wan_Flow_down","0");
			nvram_bufset(RT2860_NVRAM, "hisflowratedown", "0");
			nvram_commit(RT2860_NVRAM);
		}
		else
		{
			if(flow_down==0)
				flow_down=atoi(his_flow_down)+(data_rx/1024-old_rx);
			else
				flow_down=flow_down+(data_rx/1024-old_rx);
			old_rx=data_rx/1024;
			memset(cmd,'\0',sizeof(cmd));
			sprintf(cmd,"%d",flow_down);
			File_Set_Wan_Flow("wan_Flow_down",cmd);
			if(flow_time==commit_time)
				nvram_bufset(RT2860_NVRAM, "hisflowratedown", cmd);
		}
		
		
		if(flow_time==commit_time)
		{
			nvram_commit(RT2860_NVRAM);
			flow_time=0;
		}
		
		flow_time++;
		sleep(10);
	}
}
