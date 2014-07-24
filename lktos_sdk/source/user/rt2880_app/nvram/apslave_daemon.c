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
		//error(E_L, E_LOG, T("getIfIp: ioctl SIOCGIFADDR error for %s"), ifname);
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
	getIfIp("eth2.2",info->ip);
	return SUCCESS;
}

int main(void) 
{
  int sockfd,len;
  struct sockaddr_in addr;
  int addr_len=sizeof(struct sockaddr_in);
  int numbytes;
  char buffer[256],message[256],erro[5];

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
    //printf("\n revieve form %s---msg=%s",inet_ntoa(addr.sin_addr),buffer);

    if(!memcmp(buffer, "getbasicinfotomeluodevice", 26)){
    	    bzero(message,sizeof(message));
					//sprintf(message, "ip=%s#port=%d#login=%s#psw=%s#",info.ip,atol(info.port),info.user,info.psw);	
					sprintf(message, "discover=%s %s %s",info.devicename,info.mac,info.ip);			
					//printf("1 sent command (%s) to IP:%s\n", message, inet_ntoa(addr.sin_addr));
					if ((numbytes = sendto(sockfd,message,sizeof(message),0,&addr,addr_len)) == -1) {
						//fprintf(stderr, "send failed\n");
						close(sockfd);
						exit(1);
					}
				//	printf("3 sent command (%s) to IP:%s---msg=%s---byte=%d\n", message, inet_ntoa(addr.sin_addr),message,numbytes);
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
