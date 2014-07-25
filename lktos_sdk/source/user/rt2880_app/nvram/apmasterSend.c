#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<netdb.h>
#include	<net/if.h>
#include <sys/ioctl.h>



#define PORT 23567
#define MAXDATASIZE 1024

#define CMDLEN  4
#define CMD01  "KY01"      // discover
#define CMD02  "KY02"      // discover  ok  and ret 
#define CMD03  "KY03"      // set config
#define CMD04  "KY04"      // set config ok and ret
#define CMD05  "KY05"      // reboot
#define CMD06  "KY06"      // reboot ok and ret

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

int main(int argc,char *argv[])
{
    int socket_fd;
    struct sockaddr_in my_addr,user_addr;
    char buf[MAXDATASIZE]={0};
    int so_broadcast=1;
    socklen_t size;
    char my_ip[12];
    char cmdbuf[MAXDATASIZE]={0};
	FILE *fp;
	if (argc<=1)
		return ;

	int cmdNo = atoi(argv[1]);
	char local_ip[16]={0};
	char ipadd_wan[16]={0};
	char broadcastaddr[20]={0};
	char *temptr=NULL;
    getIfIp("eth2.2",ipadd_wan);
	temptr = strchr(ipadd_wan,'.');
	temptr = strchr(temptr+1,'.');
	temptr = strchr(temptr+1,'.');
	*(temptr+1)='\0';
	if (temptr!=NULL)
		sprintf(broadcastaddr,"%s%s",ipadd_wan,"255");
//		temptr = strcat(ipadd_wan,"255");
printf("====BROADCASTIP=%s====\n",broadcastaddr);	
		
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(PORT);
    my_addr.sin_addr.s_addr=inet_addr(broadcastaddr);
    bzero(&(my_addr.sin_zero),8);
    
    user_addr.sin_family=AF_INET;
    user_addr.sin_port=htons(PORT);
    user_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&(user_addr.sin_zero),8);
    if((socket_fd=(socket(AF_INET,SOCK_DGRAM,0)))==-1) {
        perror("socket");
        exit(1);
    }
   setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,&so_broadcast,sizeof(so_broadcast));
    if((bind(socket_fd,(struct sockaddr *)&user_addr,sizeof(struct sockaddr)))==-1) {
        perror("bind");
        exit(1);
    }
	//  send to by cmd
	switch (cmdNo) {
		case 1 :
		    strcpy(buf,CMD01);
            sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
			size=sizeof(user_addr);
		    //recvfrom(socket_fd,buf,MAXDATASIZE,0,(struct sockaddr *)&user_addr,&size);
//    		    strcpy(my_ip,inet_ntoa(user_addr.sin_addr));
		    //printf("my_ip:%sn",inet_ntoa(user_addr.sin_addr));
		    system("rm -f /var/apscanlist");
		    while(1) {
		        bzero(buf, sizeof(buf));          
		        recvfrom(socket_fd,buf,MAXDATASIZE,0,(struct sockaddr *)&user_addr,&size);
		        printf("%s\n",buf);
				if (!strncmp(buf,CMD02,CMDLEN))
				{
			        sprintf(cmdbuf,"echo %s  >> /var/apscanlist",buf);
		    	    printf("%s\n",cmdbuf);		
		        	system(cmdbuf);
				}		    
		    }			
			break ;
		case 2  :
			fp = fopen("/var/apsendcmd", "r");
			if (NULL == fp)
				return  ;
			
			if (fgets(cmdbuf, sizeof(cmdbuf), fp) != NULL) {
				sprintf(buf,"%s#%s",CMD03,cmdbuf);
                sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
     			 size=sizeof(user_addr);
   				while(1) {
			        bzero(buf, sizeof(buf));          
			        size=sizeof(user_addr);
			        recvfrom(socket_fd,buf,MAXDATASIZE,0,(struct sockaddr *)&user_addr,&size);
			      //  printf("%s\n",buf);
					//if (strncmp(buf,CMD04,CMDLEN))
//					{
	//				}
				}
			}
			fclose(fp);
					break ;
		case  3  :
			fp = fopen("/var/apsendcmd", "r");
			if (NULL == fp)
				return ;
			
			if (fgets(cmdbuf, sizeof(cmdbuf), fp) != NULL) {
				sprintf(buf,"%s#%s",CMD05,cmdbuf);
                sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
     			 size=sizeof(user_addr);
   				while(1) {
			        bzero(buf, sizeof(buf));          
			        size=sizeof(user_addr);
			        recvfrom(socket_fd,buf,MAXDATASIZE,0,(struct sockaddr *)&user_addr,&size);
			      //  printf("%s\n",buf);
					if (strncmp(buf,CMD06,CMDLEN))
					{
					}
				}
			}
			fclose(fp);						
			break;
		default :
			break;
		}	
     /*
		        bzero(buf, sizeof(buf));
		        if(strcmp(buf,"I'm off line,bye!")==0)
		            strcpy(buf,"ok,I know,bye!");
		        else
		         {
		            strcpy(buf,"send,Hello,I get you!");
		                sleep(1);
		        }    
		        if((sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr *)&user_addr,sizeof(user_addr)))==-1)
		            perror("sendto");
		        */

    return 0;
}