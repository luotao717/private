#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include	<net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 23567
#define FAILURE		0
#define SUCCESS		1  

struct message_info_s {
	char devicename[64];
	char ip[32];
	char user[32];
	char psw[32];
	char mac[18];
}info;

#define CMDLEN  4
#define CMD01  "KY01"      // discover
#define CMD02  "KY02"      // discover  ok  and ret 
#define CMD03  "KY03"      // set config
#define CMD04  "KY04"      // set config ok and ret
#define CMD05  "KY05"      // reboot
#define CMD06  "KY06"      // reboot ok and ret



static int getIfIp(char *ifname, char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		
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

static int get_mib(char *val, char *mib)
{        
	FILE *fp;         
	char buf[32];        
	sprintf(buf, "nvram_get 2860 %s", mib);            
	fp = popen(buf, "r");        
	if (fp==NULL)                
		return -1;        
	if (NULL == fgets(buf, sizeof(buf),fp)) 
	{                
		pclose(fp);                
		return -1;       
	}        
	//strcpy(val, strstr(buf, "\"")+1);        
	strcpy(val, buf);        
	val[strlen(val)-1] = '\0';        
	pclose(fp);        
	return 0;
}

int get_mib_para(struct message_info_s *info)
{
	get_mib(info->devicename,		"device_name");
	get_mib(info->mac,		"WAN_MAC_ADDR");
	//get_mib(info->ip,		  "IP_ADDR"); 
	//get_mib(info->port,	  "WEB_PORT_NUM");
//	getIfIp("eth2.2",info->ip);
	return SUCCESS;
}

//   rcvIpList is "|1|2|..............|254"     
//   ret 0  not in 
int ImIn(char *rcvIpList, char* myip)
{
	char *temptr=NULL;
	char tofindstr[8] = {0};
	temptr=strchr(myip,'.');
	if(temptr != NULL)
	{
		temptr=strchr(temptr+1,'.');
	}
	if(temptr != NULL)
	{
		temptr=strchr(temptr+1,'.');
	}
	if(temptr != NULL)
	{
		 sprintf(tofindstr,"|%s|",temptr+1);
	}
	if (NULL==strstr(rcvIpList,tofindstr)) 
		return 0 ;
	else 
		return 1 ;
	
}

// confstr is  #ssid=value#pswd=value#login=value#pwd=value#
void do_set_conf(char *confstr)
{
	char cmdline[128] = {0};
    char *result = NULL;
	result =strtok(confstr,"#");
	char tmpstr[64]={0};
	while(result!=NULL){
		char *pstr=NULL;
		memset(cmdline,0,sizeof(cmdline));
		if ((pstr = strstr(result,"ssid="))!=NULL){
			sprintf(cmdline,"nvram_set 2860 SSID1 \"%s\"", pstr+5);
			system(cmdline);
//			continue;
		} 
		else if ((pstr = strstr(result,"pswd="))!=NULL){
			if (!strcmp(pstr+5,"NONE")){ //   no 
			sprintf(cmdline,"nvram_set 2860 AuthMode OPEN");
			system(cmdline);
			sprintf(cmdline,"nvram_set 2860 EncrypType NONE");
			system(cmdline);			
			}	
			else{
			sprintf(cmdline,"nvram_set 2860 WPAPSK1 \"%s\"", pstr+5);
			system(cmdline);
			sprintf(cmdline,"nvram_set 2860 AuthMode WPA2PSK");
			system(cmdline);
			sprintf(cmdline,"nvram_set 2860 EncrypType AES");
			system(cmdline);						
//			continue;
			}		
		}
		else if ((pstr = strstr(result,"login="))!=NULL){
			sprintf(cmdline,"nvram_set 2860 Login \"%s\"", pstr+6);
			system(cmdline);
//			continue;			
		}
		else if ((pstr = strstr(result,"pwd="))!=NULL){
			sprintf(cmdline,"nvram_set 2860 Password \"%s\"", pstr+11);
			system(cmdline);
//			continue;					
		}
     	 result = strtok(NULL, "#");
				
	}
		

}

int main(void) 
{
	int sockfd,len;
	struct sockaddr_in addr;
	int addr_len=sizeof(struct sockaddr_in);
	int numbytes;
	char buffer[256],message[256],erro[5];

	char	 ipadd_wan[20] = {0};

	get_mib_para(&info);
	if(info.devicename[0]=='\0')
	{
		strcpy(info.devicename,"unknow");
	}
	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		printf("\n error in socket");
		exit(1);
	}
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port =htons(PORT);
	addr.sin_addr.s_addr =htonl(INADDR_ANY);
	if(bind(sockfd,&addr,sizeof(addr))<0)
	{
		printf("\nerror in bind");
		exit(1);
	}
	while(1)
	{
		bzero(buffer,sizeof(buffer));
		if ((numbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, &addr, &addr_len)) == -1) {
			         //fprintf(stderr,"Receive failed!!!\n");
			         close(sockfd);
			         exit(1);
		}
printf("============ revieve -msg=%s\n",buffer);
		getIfIp("eth2.2",ipadd_wan);
		//  discover
		if(!memcmp(buffer, CMD01, CMDLEN)){
				bzero(message,sizeof(message));	

				sprintf(message, "%s %s %s %s",CMD02,info.devicename,info.mac,ipadd_wan);			
				if ((numbytes = sendto(sockfd,message,sizeof(message),0,&addr,addr_len)) == -1) {
						//fprintf(stderr, "send failed\n");
						close(sockfd);
						exit(1);
				}
				//	printf("3 sent command (%s) to IP:%s---msg=%s---byte=%d\n", message, inet_ntoa(addr.sin_addr),message,numbytes);
		}
		else if(!memcmp(buffer, CMD03, CMDLEN)){ 
		  //set config  cmd = cmd03#ssid=value#pwd=value#login=value#login_psw=value#lan=|1|2|3...........|254|
				char  *findstr = NULL;
		  		char  setstr[512] = {0} ;
		  		findstr = strstr(buffer,"lan=");
				if (findstr!=NULL){
printf("======ImIn = %s =======\n",findstr+4);
						if (ImIn(findstr+4,ipadd_wan)){
							*(findstr+1)='\0';
							strcpy(setstr,buffer+CMDLEN);
printf("======setstr = %s ======\n",setstr);						
							do_set_conf(setstr);
							bzero(message,sizeof(message));
							sprintf(message,"%s",CMD04);
							if ((numbytes = sendto(sockfd,message,sizeof(message),0,&addr,addr_len)) == -1) {
										//fprintf(stderr, "send failed\n");
										close(sockfd);
										exit(1);
							}	
							system("reboot");							
						}
						else
							continue;
				}
				else
				{
					bzero(erro,sizeof(erro));
					strcpy(erro, "erro");		
					sendto(sockfd,erro,sizeof(erro),0,&addr,addr_len);
				}
		}
		else if (!memcmp(buffer, CMD05, CMDLEN)){
			// reboot   cmd=cmd05#lan=|1|2|3|........|254|
				if (ImIn(buffer,ipadd_wan)){
						bzero(message,sizeof(message));
						sprintf(message,"%s",CMD06);
						if ((numbytes = sendto(sockfd,message,sizeof(message),0,&addr,addr_len)) == -1) {
									//fprintf(stderr, "send failed\n");
									close(sockfd);
									exit(1);
						}															
						system("reboot");
				}
				else
					continue;
				
		}
		else
		{
				bzero(erro,sizeof(erro));
				strcpy(erro, "erro");		
				sendto(sockfd,erro,sizeof(erro),0,&addr,addr_len);
		}
					
	}
	return 0;
}


