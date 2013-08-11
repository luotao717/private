#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/stat.h>
#include 	<sys/types.h>
#include 	<sys/socket.h>
#include    <sys/ioctl.h>
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

#define PROTO_UNKNOWN	0
#define PROTO_TCP		1
#define PROTO_UDP		2
#define PROTO_TCP_UDP	3
#define PROTO_ICMP		4
#define PROTO_NONE		5

#define ACTION_DROP		0
#define ACTION_ACCEPT	1

#define TXBYTE		0
#define TXPACKET	1
#define RXBYTE		2
#define RXPACKET	3
#define PROC_IF_STATISTIC	"/proc/net/dev"

union IpAdd_un
{	
	unsigned int  IpAdd;
	unsigned char c[4];
};

#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/ttyS1", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)

static int lock_pcm_sem(int fd_sem)
{
	struct sembuf 	askfor_res;
	
	askfor_res.sem_num=  0;
	askfor_res.sem_op =  -1;//0;//1;//-1;
	askfor_res.sem_flg=  SEM_UNDO;		
		
	if(semop(fd_sem,&askfor_res,1)==-1)//ask for resource
	{
		printf("down semop error");
		return -1;
	}	
	return 1;	
}

static int unlock_pcm_sem(int fd_sem)
{
	struct sembuf 	free_res;
	
	free_res.sem_num=	0;
	free_res.sem_op =	1;
	free_res.sem_flg=	SEM_UNDO;
	
	if(semop(fd_sem,&free_res,1)==-1)//free the resource.
	{	
		if(errno==EIDRM)
			printf("the semaphore set was removed\n");
			
		return -1;	
	}		
	return 1;
}

//zhubo add 2008.05.14
int write_str(int fd, char *str, int len)
{
	int i, readlen;
    char buf[256];
	unsigned char wt_str[64];
	

	for( i = 0; i < len; i ++)
	{
		write ( fd, str+i, 1);
		usleep(10000);
	}
	
	//write(fd,str,strlen(str));
	write( fd, "\r", 1);
	write(fd, "\n", 1); 

	usleep(500000);

	memset(buf, 0, sizeof(buf));
	if( (readlen = read(fd, buf, 256)) > 0)
	{
		printf("cmd: %s,return from cdma: %s\n", str,buf);
	}

	usleep(100000);

	return readlen;
}

void Verify_Login(PLoginParam plogin, PResult2Login presultlogin)
{
	char *admu = nvram_bufget(RT2860_NVRAM, "Login");
	char *admp = nvram_bufget(RT2860_NVRAM, "Password");

	if (admu && strcmp(admu, "") && admp && strcmp(admp, ""))
	{
		if(!strcmp(admu,plogin->user)&&!strcmp(admp,plogin->passwd))
			presultlogin->result = 1;
		else
			presultlogin->result = 0;
	}
	else
		presultlogin->result = -1;
}

void Set_Wan_Param(PWanParam pwanparam)
{
	int ctype;

	ctype = pwanparam->wantype;

	switch(ctype)
	{
		case 0: //static 
			nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "STATIC");
			nvram_bufset(RT2860_NVRAM, "wan_ipaddr", pwanparam->static_ip);
			nvram_bufset(RT2860_NVRAM, "wan_netmask", pwanparam->static_mask);
			nvram_bufset(RT2860_NVRAM, "wan_gateway", pwanparam->static_gw);
			nvram_bufset(RT2860_NVRAM, "wan_primary_dns", pwanparam->static_pdns);
			nvram_bufset(RT2860_NVRAM, "wan_secondary_dns", pwanparam->static_sdns);
			break;

		case 1: //dhcp
			nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "DHCP");
			break;

		case 2://pppoe 
			nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "PPPOE");
			nvram_bufset(RT2860_NVRAM, "wan_pppoe_user", pwanparam->pppoe_usrname);
			nvram_bufset(RT2860_NVRAM, "wan_pppoe_pass", pwanparam->pppoe_passwd);
			break;

		case 3: // 3G
			cprintf("1\n");
			if(pwanparam->g3_dialparam==0)
				nvram_bufset(RT2860_NVRAM, "dial3gchoicetype", "AUTO");
			else
				nvram_bufset(RT2860_NVRAM, "dial3gchoicetype", "MANUAL");
			cprintf("2\n");
			nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "3G");
			cprintf("3\n");
			nvram_bufset(RT2860_NVRAM, "dial3gsimpin", pwanparam->g3_simpin);
			cprintf("4\n");
			nvram_bufset(RT2860_NVRAM, "dial3gnum", pwanparam->g3_dialnum);
			cprintf("6\n");
			nvram_bufset(RT2860_NVRAM, "dial3gusername", pwanparam->g3_usrname);
			cprintf("7\n");
			nvram_bufset(RT2860_NVRAM, "dial3gpassword", pwanparam->g3_passwd);
			cprintf("8\n");
			nvram_bufset(RT2860_NVRAM, "dial3gapn", pwanparam->g3_apn);
			cprintf("9\n");
			break;

		case 4: // CMCC
			nvram_bufset(RT2860_NVRAM, "wanConnectionMode", "WifiAccess");
			nvram_bufset(RT2860_NVRAM, "CMCC_PHONE", pwanparam->cmcc_phonenum);
			nvram_bufset(RT2860_NVRAM, "CMCC_PASSWORD", pwanparam->cmcc_passwd);
			break;
			
		default:
			break;
	}
	nvram_commit(RT2860_NVRAM);
}


void Get_Wan_Param(PWanParam pwanparam)
{
	char *wan_mode=NULL;

	wan_mode=nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if(!wan_mode)
		return;

	if( !strcmp(wan_mode, "STATIC"))
	{
		pwanparam->wantype = 0;
		strcpy(pwanparam->static_ip,nvram_bufget(RT2860_NVRAM,"wan_ipaddr"));
		strcpy(pwanparam->static_mask,nvram_bufget(RT2860_NVRAM,"wan_netmask"));
		strcpy(pwanparam->static_gw, nvram_bufget(RT2860_NVRAM,"wan_gateway"));
		strcpy(pwanparam->static_pdns,nvram_bufget(RT2860_NVRAM,"wan_primary_dns"));
		strcpy(pwanparam->static_sdns,nvram_bufget(RT2860_NVRAM,"wan_secondary_dns"));
	}
	else if( !strcmp(wan_mode, "DHCP"))
	{
		pwanparam->wantype = 1;		
	}
	else if( !strcmp(wan_mode, "PPPOE"))
	{
		pwanparam->wantype = 2;
		strcpy(pwanparam->pppoe_usrname, nvram_bufget(RT2860_NVRAM,"wan_pppoe_user"));
		strcpy(pwanparam->pppoe_passwd, nvram_bufget(RT2860_NVRAM,"wan_pppoe_pass"));
	}
	else if( !strcmp(wan_mode, "3G"))
	{
		pwanparam->wantype = 3;
		strcpy(pwanparam->g3_simpin,nvram_bufget(RT2860_NVRAM,"dial3gsimpin"));
		if(!strcmp(nvram_bufget(RT2860_NVRAM,"dial3gchoicetype"),"AUTO"))
			pwanparam->g3_dialparam=0;
		else
			pwanparam->g3_dialparam=1;
		strcpy(pwanparam->g3_dialnum, nvram_bufget(RT2860_NVRAM,"dial3gnum"));
		strcpy(pwanparam->g3_usrname,nvram_bufget(RT2860_NVRAM,"dial3gusername"));
		strcpy(pwanparam->g3_passwd,nvram_bufget(RT2860_NVRAM,"dial3gpassword"));
		strcpy(pwanparam->g3_apn,nvram_bufget(RT2860_NVRAM,"dial3gapn"));
	}
	else if( !strcmp(wan_mode, "WifiAccess"))
	{
		pwanparam->wantype = 4;

		strcpy(pwanparam->cmcc_phonenum, nvram_bufget(RT2860_NVRAM,"CMCC_PHONE"));
		strcpy(pwanparam->cmcc_passwd, nvram_bufget(RT2860_NVRAM,"CMCC_PASSWORD"));
	}
}


void Set_Lan_Param(PLanParam planparm)
{
	int endhcp=0;

	endhcp = planparm->dhcp_en;
	if(endhcp)
	{
		nvram_bufset(RT2860_NVRAM, "dhcpEnabled", "1");
		nvram_bufset(RT2860_NVRAM, "dhcpStart", planparm->dhcp_start);
		nvram_bufset(RT2860_NVRAM, "dhcpEnd", planparm->dhcp_end);
	}	
	else
		nvram_bufset(RT2860_NVRAM, "dhcpEnabled", "0");

	nvram_bufset(RT2860_NVRAM, "lan_ipaddr", planparm->lanip);
	nvram_bufset(RT2860_NVRAM, "lan_netmask", planparm->lanmask);

	nvram_commit(RT2860_NVRAM);
}

void Get_Lan_Param(PLanParam planparm)
{
	char *dhcpenable=NULL;

	dhcpenable=nvram_bufget(RT2860_NVRAM,"dhcpEnabled");
	if(!strcmp(dhcpenable,"1"))
		planparm->dhcp_en = 1;
	else
		planparm->dhcp_en = 0;

	strcpy(planparm->lanip, nvram_bufget(RT2860_NVRAM,"lan_ipaddr"));
	strcpy(planparm->lanmask, nvram_bufget(RT2860_NVRAM,"lan_netmask"));
	strcpy(planparm->dhcp_start, nvram_bufget(RT2860_NVRAM,"dhcpStart"));
	strcpy(planparm->dhcp_end, nvram_bufget(RT2860_NVRAM,"dhcpEnd"));
		
}

void Set_Wireless_Param(PWirelessParam pwirelessparm)
{
	int 	channel=0;
	char	chn_s[4];
	

	nvram_bufset(RT2860_NVRAM, "SSID1", pwirelessparm->ssid);

	channel = pwirelessparm->channel;
	if(channel == 0 ) // auto
		nvram_bufset(RT2860_NVRAM, "AutoChannelSelect", "1");
	else
	{
		nvram_bufset(RT2860_NVRAM, "AutoChannelSelect", "0");
		memset(chn_s, '\0', sizeof(chn_s));
		sprintf(chn_s, "%d", channel);
		nvram_bufset(RT2860_NVRAM, "Channel", chn_s);
	}		

	switch (pwirelessparm->securitymode)
	{
		case 0:	//disable
		{
			nvram_bufset(RT2860_NVRAM, "AuthMode", "OPEN");
			nvram_bufset(RT2860_NVRAM, "EncrypType", "NONE");
			break;
		}			

		case 1: //wpapsk
		{
			nvram_bufset(RT2860_NVRAM, "AuthMode", "WPAPSK");

			if(pwirelessparm->wpaalg==0) //TKIP
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIP");
			else if(pwirelessparm->wpaalg==1) //AES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");
			else if(pwirelessparm->wpaalg==2) //TKIPAES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIPAES");
			else
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");

			nvram_bufset(RT2860_NVRAM, "WPAPSK1", pwirelessparm->passwd);
			break;
		}			

		case 2:	//wpa2psk
		{
			nvram_bufset(RT2860_NVRAM, "AuthMode", "WPA2PSK");

			if(pwirelessparm->wpaalg==0) //TKIP
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIP");
			else if(pwirelessparm->wpaalg==1) //AES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");
			else if(pwirelessparm->wpaalg==2) //TKIPAES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIPAES");
			else
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");
			
			nvram_bufset(RT2860_NVRAM, "WPAPSK1", pwirelessparm->passwd);
			break;
		}
		
		case 3:	//wpapskwpa2psk
		{
			nvram_bufset(RT2860_NVRAM, "AuthMode", "WPAPSKWPA2PSK");
			if(pwirelessparm->wpaalg==0) //TKIP
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIP");
			else if(pwirelessparm->wpaalg==1) //AES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");
			else if(pwirelessparm->wpaalg==2) //TKIPAES
				nvram_bufset(RT2860_NVRAM, "EncrypType", "TKIPAES");
			else
				nvram_bufset(RT2860_NVRAM, "EncrypType", "AES");

			nvram_bufset(RT2860_NVRAM, "WPAPSK1", pwirelessparm->passwd);
			break;
		}
	}
	nvram_commit(RT2860_NVRAM);
}

void Get_Wireless_Param(PWirelessParam pwirelessparm)
{
	char *autoselect = NULL, *channel = NULL, *authmode = NULL, *encrytype = NULL, *passwd = NULL;

	strcpy(pwirelessparm->ssid, nvram_bufget(RT2860_NVRAM, "SSID1"));

	autoselect = nvram_bufget(RT2860_NVRAM, "AutoChannelSelect");
	if (autoselect&&(!strncmp(autoselect, "1", 2)) )
		pwirelessparm->channel = 0;
	else
		pwirelessparm->channel = atoi(nvram_bufget(RT2860_NVRAM, "Channel"));

	authmode = nvram_bufget(RT2860_NVRAM, "AuthMode");
	if(!strcmp(authmode, "OPEN"))
		pwirelessparm->securitymode = 0;
	else if(!strcmp(authmode, "WPAPSK"))
	{
		pwirelessparm->securitymode = 1;
		encrytype = nvram_bufget(RT2860_NVRAM, "EncrypType");
		passwd    = nvram_bufget(RT2860_NVRAM, "WPAPSK1");
		if(!strcmp(encrytype, "TKIP"))
			pwirelessparm->wpaalg = 0;
		else if(!strcmp(encrytype, "AES"))
			pwirelessparm->wpaalg = 1;
		else
			pwirelessparm->wpaalg = 2;
		strcpy(pwirelessparm->passwd, passwd);
	}
	else if(!strcmp(authmode, "WPA2PSK"))
	{
		pwirelessparm->securitymode = 2;
		encrytype = nvram_bufget(RT2860_NVRAM, "EncrypType");
		passwd    = nvram_bufget(RT2860_NVRAM, "WPAPSK1");
		if(!strcmp(encrytype, "TKIP"))
			pwirelessparm->wpaalg = 0;
		else if(!strcmp(encrytype, "AES"))
			pwirelessparm->wpaalg = 1;
		else
			pwirelessparm->wpaalg = 2;
		strcpy(pwirelessparm->passwd, passwd);
	}
	else
	{
		pwirelessparm->securitymode = 3;
		encrytype = nvram_bufget(RT2860_NVRAM, "EncrypType");
		passwd    = nvram_bufget(RT2860_NVRAM, "WPAPSK1");
		if(!strcmp(encrytype, "TKIP"))
			pwirelessparm->wpaalg = 0;
		else if(!strcmp(encrytype, "AES"))
			pwirelessparm->wpaalg = 1;
		else
			pwirelessparm->wpaalg = 2;
		strcpy(pwirelessparm->passwd, passwd);
	}
}

void Set_Account_Param(PAccountParam paccountparm)
{
	nvram_bufset(RT2860_NVRAM, "Login", paccountparm->account_name);
	nvram_bufset(RT2860_NVRAM, "Password", paccountparm->account_passwd);

	nvram_commit(RT2860_NVRAM);
}

void Get_Account_Param(PAccountParam paccountparm)
{
	strcpy(paccountparm->account_name, nvram_bufget(RT2860_NVRAM, "Login"));
	strcpy(paccountparm->account_passwd, nvram_bufget(RT2860_NVRAM, "Password"));
}

void Set_Firewall_Filter(PSetFilter psetfilter)
{
	char rule[8192];
	char *mac_address;
	char *sip_1, *sip_2, *sprf, *sprt, *protocol, *action_str, *comment;
	char *dip_1, *dip_2, *dprf, *dprt;
	char *IPPortFilterRules;
	
	int sprf_int, sprt_int, dprf_int, dprt_int, proto, action;

	if(psetfilter->enable)
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "1");
	else
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "0");

	mac_address = psetfilter->mac_addr;

	sip_1 = psetfilter->source_ip;
	//if(!strlen(sip_1))
	//	sip_1 = "any/0";
	if(!strcmp(sip_1,"0.0.0.0")||!strlen(sip_1))
		sip_1 = "any/0";
	sip_2 = "0";
	
	sprf_int = psetfilter->sport_start;
	sprt_int = psetfilter->sport_end;

	dip_1 = psetfilter->dest_ip;
	//if(!strlen(dip_1))
	//	dip_1 = "any/0";
	if(!strcmp(dip_1,"0.0.0.0")||!strlen(dip_1))
		dip_1 = "any/0";
	dip_2 = "0";
	
	dprf_int = psetfilter->dport_start;
	dprt_int = psetfilter->dport_end;

	switch(psetfilter->protocol)
	{
		case 0 : //NONE
			proto = PROTO_UNKNOWN;
			sprf_int = 0;
			sprt_int = 0;
			dprf_int = 0;
			dprt_int = 0;
			break;
		case 1: //	TCP
			proto = PROTO_TCP;
			break;
		case 2: //	UDP
			proto = PROTO_UDP;
			break;
		case 3: //	TCP&UDP
			proto = PROTO_TCP_UDP;
			break;	
		case 4: //	ICMP
			proto = PROTO_ICMP;
			sprf_int = 0;
			sprt_int = 0;
			dprf_int = 0;
			dprt_int = 0;
			break;
		case 5: //	NONE
			proto = PROTO_NONE;
			sprf_int = 0;
			sprt_int = 0;
			dprf_int = 0;
			dprt_int = 0;	
		default:
			proto = PROTO_NONE;
			sprf_int = 0;
			sprt_int = 0;
			dprf_int = 0;
			dprt_int = 0;
			break;			
	}

	action = ACTION_DROP;
	comment = "";

	if(   ( IPPortFilterRules = nvram_bufget(RT2860_NVRAM, "IPPortFilterRules")) && strlen(IPPortFilterRules)>8){
		snprintf(rule, sizeof(rule), "%s;%s,%s,%d,%d,%s,%s,%d,%d,%d,%d,%s,%s", IPPortFilterRules, sip_1, sip_2, sprf_int, sprt_int, dip_1, dip_2, dprf_int, dprt_int, proto, action, comment, mac_address);
	}else{
		snprintf(rule, sizeof(rule), "%s,%s,%d,%d,%s,%s,%d,%d,%d,%d,%s,%s", sip_1, sip_2, sprf_int, sprt_int, dip_1, dip_2, dprf_int, dprt_int, proto, action, comment, mac_address);
	}

	nvram_set(RT2860_NVRAM, "IPPortFilterRules", rule);
}

void Get_Filter_List(PGetFilterList pfilterlist)
{
	char *rules = nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	char *enable = NULL;

	enable = nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");
	if(rules&&strlen(rules))
		strcpy(pfilterlist->rules, rules);
	if(enable&&strlen(enable))
		pfilterlist->enable = atoi(enable);
}

static int getlistNums(char *value, char delimit)
{
	char *pos = value;
    int count=1;
    if(!pos)
    	return 0;
	while( (pos = strchr(pos, delimit))){
		pos = pos+1;
		count++;
	}
	return count;
}

int deleteFilter(int index[],char *strFilter,int comCount,char* naramStr)
{
	char *pch,tempPch[sizeof(char)*strlen(strFilter)];
	int i=1,j=0,delFlag=0,isLast=0;
	
	for(j=0;j<comCount;j++)
	{
		if(index[j]==comCount)
		{
			isLast=1;
			break;
		}
	}
	
	memset(tempPch,'\0',(sizeof(char)*strlen(strFilter)));
	pch=strtok(strFilter,";");
	while (pch != NULL)
  	{
		for(j=0;j<comCount;j++)
		{
			if(index[j]==i)
			{
				delFlag=1;
				break;
			}
		}
		if(!delFlag)
		{
			strcat(tempPch, pch);
			if(i!=comCount)
			{
				if(isLast)
				{
					if(i!=comCount-1)
						strcat(tempPch, ";");
				}
				else
				{
						strcat(tempPch, ";");
				}
				
			}
		}
    	pch = strtok (NULL, ";");
		delFlag=0;
		i++;
  	}
	nvram_set(RT2860_NVRAM, naramStr, tempPch);
	nvram_commit(RT2860_NVRAM);
	return 0;
}

/*
 *  argument:  [IN]     index -- the index array of deleted items(begin from 0)
 *             [IN]     count -- deleted itmes count.
 *             [IN/OUT] value -- original string/return string
 *             [IN]     delimit -- delimitor
 */
static int deleteNthValue(int index[], int count, char *value, char delimit)
{
	char *begin, *end;
	int i=0,j=0;
	int need_check_flag=0;
	char *buf = strdup(value);

	begin = buf;

	end = strchr(begin, delimit);
	while(end){
		if(i == index[j]){
			memset(begin, 0, end - begin );
			if(index[j] == 0)
				need_check_flag = 1;
			j++;
			if(j >=count)
				break;
		}
		begin = end;

		end = strchr(begin+1, delimit);
		i++;
	}

	if(!end && index[j] == i)
		memset(begin, 0, strlen(begin));

	if(need_check_flag){
		for(i=0; i<strlen(value); i++){
			if(buf[i] == '\0')
				continue;
			if(buf[i] == ';')
				buf[i] = '\0';
			break;
		}
	}

	for(i=0, j=0; i<strlen(value); i++){
		if(buf[i] != '\0'){
			value[j++] = buf[i];
		}
	}
	value[j] = '\0';

	free(buf);
	return 0;
}


void Del_Firewall_Filter(PDelFilter pdelfilter)
{
	int i, j, rule_count;
	//int *deleArray;

	char *rules = nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
    if(!rules || !strlen(rules) )
    	return;

	rule_count = getlistNums(rules,';');
	if(!rule_count)
		return;
	if(rule_count>=MAX_DHCP_CLIENTS)
		rule_count = MAX_DHCP_CLIENTS;

	//deleArray = (int *)malloc(rule_count * sizeof(int));
	int deleArray[rule_count];
	for(i=0;i<rule_count;i++)
		deleArray[i]=0;
	
	for(i=1, j=0; i<= rule_count; i++)
	{
		if(pdelfilter->del[i-1])
			deleArray[j++] = i;
	}

	if(!j)
		return;

	deleteFilter(deleArray,rules,rule_count,"IPPortFilterRules");
	//deleteNthValue(deleArray, rule_count, rules, ';');
	//free(deleArray);

	//nvram_set(RT2860_NVRAM, "IPPortFilterRules", rules);
}


void Set_Nat_Filter(PSetNat	psetnat)
{
	char rule[8192];
	char *ip_address, *pfe, *prf, *prt, *protocol, *comment;
	char *PortForwardRules;

	int prf_int, prt_int, proto;

	if(psetnat->enable)
		nvram_set(RT2860_NVRAM, "PortForwardEnable", "1");
	else
		nvram_set(RT2860_NVRAM, "PortForwardEnable", "0");

	ip_address = psetnat->ipaddr;
	prf_int = psetnat->port_start;
	prt_int = psetnat->port_end;

	switch(psetnat->protocol)
	{
		case 1: //tcp
			proto = PROTO_TCP;
			break;

		case 2: //udp
			proto = PROTO_UDP;
			break;

		case 3: //all
			proto = PROTO_TCP_UDP;
			break;	
	}

	comment = "";

	if(( PortForwardRules = nvram_bufget(RT2860_NVRAM, "PortForwardRules")) && strlen( PortForwardRules)>8 )
		snprintf(rule, sizeof(rule), "%s;%s,%d,%d,%d,%s",  PortForwardRules, ip_address, prf_int, prt_int, proto, comment);
	else
		snprintf(rule, sizeof(rule), "%s,%d,%d,%d,%s", ip_address, prf_int, prt_int, proto, comment);

	nvram_set(RT2860_NVRAM, "PortForwardRules", rule);
	nvram_commit(RT2860_NVRAM);
}


void Get_Nat_List(PGetNatList pnatlist)
{
	char *rules = nvram_bufget(RT2860_NVRAM, "PortForwardRules");
	char *enable = NULL;

	enable = nvram_bufget(RT2860_NVRAM, "PortForwardEnable");

	if(rules&&strlen(rules))
		strcpy(pnatlist->rules, rules);
	if(rules&&strlen(rules))
		pnatlist->enable = atoi(enable);
}

void Del_Nat_Filter(PDelNat pdelnat)
{
	int i, j, rule_count;
	//int *deleArray;

	char *rules = nvram_bufget(RT2860_NVRAM, "PortForwardRules");
    if(!rules || !strlen(rules) )
    	return;

	rule_count = getlistNums(rules,';');
	if(!rule_count)
		return;
	if(rule_count>=MAX_DHCP_CLIENTS)
		rule_count = MAX_DHCP_CLIENTS;

	//deleArray = (int *)malloc(rule_count * sizeof(int));
	int deleArray[rule_count];
	for(i=0;i<rule_count;i++)
		deleArray[i]=0;
	
	for(i=1, j=0; i<= rule_count; i++)
	{
		if(pdelnat->del[i-1])
			deleArray[j++] = i;
	}

	if(!j)
		return;

	deleteFilter(deleArray,rules,rule_count,"PortForwardRules");
	//deleteNthValue(deleArray, rule_count, rules, ';');
	//free(deleArray);

	//nvram_set(RT2860_NVRAM, "PortForwardRules", rules);
}


void Set_Dmz(PSetDMZ psetdmz)
{
	if(psetdmz->enable)
		nvram_set(RT2860_NVRAM, "DMZEnable", "1");
	else
		nvram_set(RT2860_NVRAM, "DMZEnable", "0");

	if(strlen(psetdmz->dmz_addr))
		nvram_set(RT2860_NVRAM, "DMZIPAddress", psetdmz->dmz_addr);

	nvram_commit(RT2860_NVRAM);
}

void Get_Dmz(PGetDMZ pgetdmz)
{
	char *ip_address=NULL;
	char *enable=NULL;

	ip_address = nvram_bufget(RT2860_NVRAM, "DMZIPAddress");
	enable = nvram_bufget(RT2860_NVRAM, "DMZEnable");
	if(ip_address)
		strcpy(pgetdmz->dmz_addr, ip_address);
	if(enable&&strlen(enable))
		pgetdmz->enable = atoi(enable);
}


int get_IfIp(char *ifname, char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	close(skfd);
	return 0;
}

int get_Ifmask(char *ifname, char *if_net)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_net, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);
	return 0;
}


int get_IfMac(char *ifname, char *if_hw)
{
	struct ifreq ifr;
	char *ptr;
	int skfd;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

	close(skfd);
	return 0;
}

static void get_LanIp(char *if_addr)
{
	if (-1 == get_IfIp("br0", if_addr)) 
		if_addr = "";

	return ;
}

static void get_Lanmask(char *if_mask)
{
	if (-1 == get_Ifmask("br0", if_mask))
		if_mask = "";

	return;
}

static void get_LanMac(char *if_mac)
{
	if (-1 == get_IfMac("br0", if_mac))
		if_mac = "";

	return;
}

static void get_WanIp(char *if_name, char *if_addr)
{

	if (-1 == get_IfIp(if_name, if_addr))
		if_addr = "";

	return;
}

static void get_WanMask(char *if_name, char *if_mask)
{
	if (-1 == get_Ifmask(if_name, if_mask))
		if_mask = "";

	return;
}

static int get_NetDstIP(char *ifname, char *if_net)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFDSTADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_net, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);
	return 0;
}

static void get_WanGateway(char *if_name, char *if_gw)
{
	char   buff[256];
	int    nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	int    flgs, ref, use, metric;
	unsigned long int d,g,m;
	int    find_default_flag = 0;
	char sgw[16];
	FILE *fp;
	char if_net[16];

	if (!strcmp(if_name, "ppp0"))
	{ //fetch ip from ppp0
		if (-1 == get_NetDstIP("ppp0", if_gw))
			if_gw = "";
		return;
	}

	fp = fopen("/proc/net/route", "r");

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if (sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
						&d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				fclose(fp);
				return ;
			}

			if (flgs&RTF_UP) {
				dest.s_addr = d;
				gw.s_addr   = g;
				strcpy(if_gw, (gw.s_addr==0 ? "" : inet_ntoa(gw)));

				if (dest.s_addr == 0) {
					find_default_flag = 1;
					break;
				}
			}
		}
		nl++;
	}
	fclose(fp);

	if (find_default_flag == 1)
		return ;
	else
		if_gw = "";

	return;
}

static void get_Dns(int type, char *if_dns)
{
	FILE *fp;
	char buf[80] = {0}, ns_str[11], dns[16] = {0};
	int idx = 0, req = 0;

	if (1 == type)
		req = 1;
	else if (2 == type)
		req = 2;
	else
	{
		if_dns = "";
		return;
	}
				
	fp = fopen("/etc/resolv.conf", "r");
	if (NULL == fp)
	{
		if_dns = "";
		return;
	}
	
	while (fgets(buf, sizeof(buf), fp) != NULL) 
	{
		if (sscanf(buf, "%s%s", ns_str, if_dns) != 2)
			continue;
		idx++;
		if (idx == req) 
		{
			break;
		}
	}
	fclose(fp);
	return ;
}

int File_Modem_Info(char *name,char *value, int len)
{
	FILE *fp=NULL;
	char path[64];
	int	num=0;

	memset(path,'\0',64);
	sprintf(path,"%s%s","/var/usbmodem/",name);

	fp = fopen(path,"r");
	if(fp == NULL)
	{
		return -1;
	}
	else
	{		
		num=fgets(value,len,fp);
		fclose(fp);
		fp = NULL;
		return num;
	}
	
}


static void get_isp(char *ispname)
{
	if(File_Modem_Info("dial3gprovider", ispname, 32)<=0)
		ispname = "";

	return;
}

void Get_Dev_Info(PGetDevInfo pgetinfo)
{
	char *wan_mode=NULL;

	wan_mode=nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if(!wan_mode)
		return;

	strcpy(pgetinfo->soft_ver, FIRMWARE_VERSION);
	
	if( !strcmp(wan_mode, "STATIC"))
	{
		pgetinfo->wantype = 0;
		get_WanIp("eth2",pgetinfo->wan_ip);
		get_WanMask("eth2", pgetinfo->wan_mask);
		get_WanGateway("eth2", pgetinfo->wan_gw);
	}
	else if( !strcmp(wan_mode, "DHCP"))
	{
		pgetinfo->wantype = 1;
		get_WanIp("eth2",pgetinfo->wan_ip);
		get_WanMask("eth2", pgetinfo->wan_mask);
		get_WanGateway("eth2", pgetinfo->wan_gw);
	}
	else if( !strcmp(wan_mode, "PPPOE"))
	{
		pgetinfo->wantype = 2;
		get_WanIp("ppp0",pgetinfo->wan_ip);
		get_WanMask("ppp0", pgetinfo->wan_mask);
		get_WanGateway("ppp0", pgetinfo->wan_gw);
	}
	else if( !strcmp(wan_mode, "3G"))
	{
		pgetinfo->wantype = 3;
		get_WanIp("ppp0",pgetinfo->wan_ip);
		get_WanMask("ppp0", pgetinfo->wan_mask);
		get_WanGateway("ppp0", pgetinfo->wan_gw);

		get_isp(pgetinfo->ispname);
	}
	else if( !strcmp(wan_mode, "WifiAccess"))
	{
		pgetinfo->wantype = 4;
		get_WanIp("rai0",pgetinfo->wan_ip);
		get_WanMask("rai0", pgetinfo->wan_mask);
		get_WanGateway("rai0", pgetinfo->wan_gw);
	}

	get_LanIp(pgetinfo->lan_ip);
	get_Lanmask(pgetinfo->lan_mask);
	get_LanMac(pgetinfo->lan_mac);
	
	//get dhcp clients
	{
		FILE *fp;
		int i;
		struct in_addr addr;
		unsigned long expires, yiaddr, lease_time;
		char sigusr1[] = "-XX";
		int ret = 0;
		int count = 0;
		char *ipaddr, expires_time[50]="";
		unsigned char mac[20];
		int size;
		int zero;
		char host[80];
		int sum = 0;
		int state = 1;
		union IpAdd_un *pipu;
		unsigned d, h, m;

		system("killall -q -USR1 udhcpd");

		fp = fopen("/var/udhcpd.leases", "r");
		if (NULL == fp)
			return;

		while ( !feof(fp) ) 
		{
			 size = fread(mac, 16, 1, fp); /* optimization for ISL, write only MAC part */
			 if ( size != 1 ) break;
			 
			 zero = 0;
			 for (i = 0; i < 6; i++)
			 	if (mac[i] != 0x00)
					zero = 1;
			 size = fread(&yiaddr, 4, 1, fp);

			 if ( size != 1 ) break;

		   	 size = fread(&lease_time, 4, 1, fp);
		
			 if ( size != 1 ) break;
		
			 memset( host, 0, sizeof(host) );
			  size = fread(host, 64, 1, fp);
		
			 if ( size != 1 ) break;

			lease_time = htonl(lease_time);
			
			if (zero == 0)
				continue;
			
			pipu =(union IpAdd_un *)&yiaddr;
			for(i=0;i<=64;i++)
			{
				if(host[i]==0)
					break;
				if(host[i] >= 0xa0)
				{
					i++;
					if(i==15)
					{
						host[i-1]='\0';
						host[i]='\0';
						break;
					}
				}
				if( i >=15)
					host[15]='\0';
			}

			strcpy(pgetinfo->dhcp_devname[sum], host);
			
			sprintf(pgetinfo->dhcp_devname[sum], "%02x:%02x:%02x:%02x:%02x:%02x\0",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			
			sprintf(pgetinfo->dhcp_devip[sum], "%d.%d.%d.%d\0", pipu->c[0], pipu->c[1], pipu->c[2], pipu->c[3]);
			
			d = lease_time / (24*60*60); lease_time %= (24*60*60);
			h = lease_time / (60*60); lease_time %= (60*60);
			m = lease_time / 60; lease_time %= 60;

			if(d)
				sprintf(pgetinfo->dhcp_devtime[sum], "%u days:%02u:%02u:%02u\0",d,h,m,(unsigned)lease_time);
			else
				sprintf(pgetinfo->dhcp_devtime[sum], "%02u:%02u:%02u\0",h,m,(unsigned)lease_time);
			
			sum++;
			pgetinfo->dhcp_num = sum;
		}
		
		fclose(fp);
	}
	return;
}

void Get_3G_Signal(PGetSignal pgetsignal)
{
	char *wan_mode=NULL;
	char signal[4], value[16];
	int	  	number=0;
	float  	percent;

	wan_mode=nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if(!wan_mode)
		return;	

	memset(signal, '\0', 4);
	memset(value, '\0', 16);

	if( !strcmp(wan_mode, "3G"))
	{
		File_Modem_Info("dial3gsignal", signal, 4);

		if(signal[0]=='\0'||strcmp(signal,"0")==0||strcmp(signal,"-1")==0||strcmp(signal,"99")==0||strcmp(signal,"199")==0)
			pgetsignal->signal_level = 0;
		else
		{
			number = atoi(signal);
			if(number>=99)
			{
				if(number<=115)
					pgetsignal->signal_level = 1;
				else if(number>115&&number<=130)
					pgetsignal->signal_level = 2;
				else if(number>130&&number<=145)
					pgetsignal->signal_level = 3;
				else
					pgetsignal->signal_level = 4;
			}			
			else
			{
				if(number>=1&&number<=8)
					pgetsignal->signal_level = 1;
				else if(number>8&&number<=16)
					pgetsignal->signal_level = 2;
				else if(number>16&&number<=24)
					pgetsignal->signal_level = 3;
				else
					pgetsignal->signal_level = 4;
			}
		}

		File_Modem_Info("dial3gnetmode", pgetsignal->wan_type, sizeof(pgetsignal->wan_type));		
	}
}


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

int File_Get_Wan_Flow(char *name,char *value, int len)
{
	FILE *fp=NULL;
	char path[64];
	int	num=0;

	memset(path,'\0',64);
	sprintf(path,"%s%s","/var/",name);

	fp = fopen(path,"r");
	if(fp == NULL)
	{
		return -1;
	}
	else
	{		
		num=fgets(value,len,fp);
		fclose(fp);
		fp = NULL;
		return num;
	}
	
}


void Get_Wan_Flow(PGetFlowRate pgetflow)
{
	char *wan_mode=NULL, *if_name=NULL;
	long long data_tx=0, data_rx=0;
	static int i=0;
	char his_flow_up[32], his_flow_down[32];

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
		if_name = "rai0";

	data_tx = getWanStatistic(if_name, TXBYTE);
	data_rx = getWanStatistic(if_name, RXBYTE);

	pgetflow->cur_flow_up = data_tx/1024;
	pgetflow->cur_flow_down=data_rx/1024;
	

	memset(his_flow_up,'\0',sizeof(his_flow_up));
	File_Get_Wan_Flow("wan_Flow_up",his_flow_up,sizeof(his_flow_up));

	memset(his_flow_down,'\0',sizeof(his_flow_down));
	File_Get_Wan_Flow("wan_Flow_down",his_flow_down,sizeof(his_flow_down));
	
	if(!strlen(his_flow_up))
		pgetflow->his_flow_up = 0;
	else
		pgetflow->his_flow_up = atoi(his_flow_up);

	if(!strlen(his_flow_down))
		pgetflow->his_flow_down = 0;
	else
		pgetflow->his_flow_down = atoi(his_flow_down);
	
}

void Reset_Flow_Rate()
{
	nvram_bufset(RT2860_NVRAM, "hisflowrateup", "0");
	nvram_bufset(RT2860_NVRAM, "hisflowratedown", "0");
	nvram_commit(RT2860_NVRAM);
}

void Set_Default()
{
	system("ralink_init clear 2860");
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
	sleep(3);
	system("reboot");
}

void Set_Reboot()
{
	system("reboot");
}

int main(int argc, char *argv[])
{
  	int			          n,i,on,idx,fd_com=-1,fd_sem;
	int			          ret;
	int			          sin_len;
  	int			          pin_len;
  	int			          skfd;
  	fd_set			      fdset,acpfdset;
	struct timeval		  timeout;
	struct sockaddr_in 	  sin;
	struct sockaddr_in 	  pin;
	MySockInfo		      skinfo;
  	char              	  path[100],buf[256], *pcmd=NULL;
	FILE                  * f;
	struct linger         m_sLinger;
	CmdResult			  response;

	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

	memset(buf, '\0', sizeof(buf));

	response.flag[0] = 'W';
  	response.flag[1] = 'I';
  	response.flag[2] = 'B';
  	response.flag[3] = 'A';

  	timeout.tv_sec  = 10;
  	timeout.tv_usec = 0;

	FD_ZERO(&fdset);
  	bzero(&sin,sizeof(sin));
  	sin.sin_family = AF_INET;
  	sin.sin_addr.s_addr = htonl(INADDR_ANY);
  	sin.sin_port   = htons(CMD_PORT);
  	sin_len = sizeof(sin);

	cprintf("\n3333\n");

  	skfd = socket(AF_INET,SOCK_STREAM,0);
  	if(skfd<0)
  	{
    	printf("in netcmd.c socket failed!!\n");
    	return;
  	}

	cprintf("\nsocket\n");

  	on = 1;
  	if ( setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on) ) < 0 )
  	{
    	printf("in netcmd.c setsockopt failed!!\n");
    	close(skfd);
    	return;
  	}

  	cprintf("\setsockopt\n");

  	ret = bind(skfd, (struct sockaddr* )&sin, sizeof(sin));
	if(ret < 0)
	{		
		close(skfd);
		printf("in netcmd.c bind failed\n");
		return;
	}

	cprintf("\nbind\n");

  	listen(skfd, 5);
	FD_ZERO(&acpfdset);
	FD_SET(skfd, &acpfdset);

	cprintf("\nlisten\n");

  	while(1)
  	{    
    	ret = select(skfd + 1, &acpfdset, NULL, NULL, NULL);
		if(ret < 0)
		{
			printf("in netcmd.c select failed\n");
			continue;
		}
    	printf("in netcmd.c select success\n");
    
    	skinfo.fd = accept(skfd, (struct sockaddr* )&skinfo.sin, &skinfo.sinlen);
		if(skinfo.fd < 0) 
    	{
      		printf("in netcmd.c accept failed\n");
      		continue;
    	}

    	printf("in netcmd.c accept success\n");

		FD_ZERO(&fdset);
		FD_SET(skinfo.fd, &fdset);
		while(1)
		{
			ret = select(skinfo.fd+1, &fdset, NULL, NULL, NULL);
			if(ret <= 0)
			{
			  	printf("in netcmd.c select 2 failed the errno is %d!!\n",errno);
				close(skinfo.fd);
				break;
			}

			idx = 0;
			//先收包头
    		n = recv(skinfo.fd, buf+idx, 5, 0);//"WIBA+CMD"
			if(n <= 0)
			{
				printf("in netcmd.c recv failed,the errno is %d\n",errno);
				close(skinfo.fd);
				break;
			}
    		idx+=5;

			pcmd = buf;
			if(pcmd[4]!=0)
      			cprintf("recv pcmd is %x\n",pcmd[4]);

			switch(pcmd[4])
			{
				case NETCOM_LOGIN:
				{
					PLoginParam		pnetpro;
					Result2Login	result2login;

					cprintf("\n2222\n");
					
					n = recv(skinfo.fd, buf+idx, sizeof(LoginParam)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}

					cprintf("\n3333\n");
					
        			pnetpro = (PLoginParam)pcmd;

					memset(&result2login, '\0', sizeof(Result2Login));

					cprintf("\n1111\n");
					result2login.flag[0] = 'W';
					result2login.flag[1] = 'I';
					result2login.flag[2] = 'B';
					result2login.flag[3] = 'A';
					result2login.cmd = NETCOM_LOGIN;
					Verify_Login(pnetpro, &result2login);

					cprintf("\nlogin result = %c\n",result2login.result);

					send(skinfo.fd, &result2login, sizeof(Result2Login), MSG_NOSIGNAL);
					break;
				}
				
				case NETCOM_SET_WAN:
				{
					PWanParam	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(WanParam)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PWanParam)pcmd;

					Set_Wan_Param(pnetpro);

					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';

					response.cmd = NETCOM_SET_WAN;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_WAN:
				{
					WanParam	netpro;

					memset(&netpro, '\0', sizeof(WanParam));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_WAN;
					Get_Wan_Param(&netpro);

					send(skinfo.fd, &netpro, sizeof(WanParam), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_LAN:
				{
					PLanParam	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(LanParam)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PLanParam)pcmd;

					Set_Lan_Param(pnetpro);

										response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';

					response.cmd = NETCOM_SET_LAN;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_LAN:
				{
					LanParam	netpro;

					memset(&netpro, '\0', sizeof(LanParam));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_LAN;
					Get_Lan_Param(&netpro);

					send(skinfo.fd, &netpro, sizeof(LanParam), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_WIRELESS:
				{
					PWirelessParam	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(WirelessParam)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PWirelessParam)pcmd;

					Set_Wireless_Param(pnetpro);

										response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';

					response.cmd = NETCOM_SET_WIRELESS;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_WIRELESS:
				{
					WirelessParam	netpro;

					memset(&netpro, '\0', sizeof(WirelessParam));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_WIRELESS;
					Get_Wireless_Param(&netpro);

					send(skinfo.fd, &netpro, sizeof(WirelessParam), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_ACCOUNT:
				{
					PAccountParam	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(AccountParam)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PAccountParam)pcmd;

					Set_Account_Param(pnetpro);

					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					
					response.cmd = NETCOM_SET_ACCOUNT;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_ACCOUNT:
				{
					AccountParam	netpro;

					memset(&netpro, '\0', sizeof(AccountParam));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_ACCOUNT;
					Get_Account_Param(&netpro);

					send(skinfo.fd, &netpro, sizeof(AccountParam), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_FILTER:
				{
					PSetFilter	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(SetFilter)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PSetFilter)pcmd;

					Set_Firewall_Filter(pnetpro);

										response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';

					response.cmd = NETCOM_SET_FILTER;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_FILTER_LIST:
				{
					GetFilterList	netpro;

					memset(&netpro, '\0', sizeof(GetFilterList));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_FILTER_LIST;
					Get_Filter_List(&netpro);

					printf("\n%s\n",netpro.rules);

					send(skinfo.fd, &netpro, sizeof(GetFilterList), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_DEL_FILTER:
				{
					PDelFilter	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(DelFilter)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PDelFilter)pcmd;

					Del_Firewall_Filter(pnetpro);

					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					
					response.cmd = NETCOM_DEL_FILTER;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_NAT:
				{
					PSetNat	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(SetNat)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PSetNat)pcmd;

					Set_Nat_Filter(pnetpro);

					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					response.cmd = NETCOM_SET_NAT;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_NAT_LIST:
				{
					GetNatList	netpro;

					memset(&netpro, '\0', sizeof(GetNatList));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_NAT_LIST;
					Get_Nat_List(&netpro);

					send(skinfo.fd, &netpro, sizeof(GetNatList), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_DEL_NAT:
				{
					PDelNat	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(DelNat)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PDelNat)pcmd;

					Del_Nat_Filter(pnetpro);
					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					response.cmd = NETCOM_DEL_NAT;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_DMZ:
				{
					PSetDMZ	pnetpro;
					n = recv(skinfo.fd, buf+idx, sizeof(SetDMZ)-5, 0);
        			if(n <= 0)
    				{
    					close(skinfo.fd);
						break;
    				}
        			pnetpro = (PSetDMZ)pcmd;

					Set_Dmz(pnetpro);
					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					response.cmd = NETCOM_SET_DMZ;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_DMZ:
				{
					GetDMZ	netpro;

					memset(&netpro, '\0', sizeof(GetDMZ));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_DMZ;
					Get_Dmz(&netpro);

					send(skinfo.fd, &netpro, sizeof(GetDMZ), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_DEVIEC_INFO:
				{
					GetDevInfo netpro;

					memset(&netpro, '\0', sizeof(GetDevInfo));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_DEVIEC_INFO;
					Get_Dev_Info(&netpro);

					send(skinfo.fd, &netpro, sizeof(GetDevInfo), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_SIGNAL:
				{
					GetSignal netpro;

					memset(&netpro, '\0', sizeof(GetSignal));

					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_SIGNAL;
					Get_3G_Signal(&netpro);

					send(skinfo.fd, &netpro, sizeof(GetSignal), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_GET_FLOW:
				{
					GetFlowRate netpro;

					memset(&netpro, '\0', sizeof(GetFlowRate));
					netpro.flag[0] = 'W';
					netpro.flag[1] = 'I';
					netpro.flag[2] = 'B';
					netpro.flag[3] = 'A';
					netpro.cmd = NETCOM_GET_FLOW;
					Get_Wan_Flow(&netpro);
					
					send(skinfo.fd, &netpro, sizeof(GetFlowRate), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_RESET_TOTAL_FLOW:
				{	
					Reset_Flow_Rate();
					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					response.cmd = NETCOM_RESET_TOTAL_FLOW;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);
					break;
				}

				case NETCOM_SET_DEFAULT:
				{
					response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';					
					response.cmd = NETCOM_SET_DEFAULT;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);

					Set_Default();
					
					break;
				}

				case NETCOM_SET_REBOOT:
				{
										response.flag[0] = 'W';
				  	response.flag[1] = 'I';
				  	response.flag[2] = 'B';
				  	response.flag[3] = 'A';
					response.cmd = NETCOM_SET_REBOOT;
					response.result = 0;
					send(skinfo.fd, &response, sizeof(CmdResult), MSG_NOSIGNAL);

					Set_Reboot();
					break;
				}
				
				default:
					break;
			}
		}		
  	}
}
