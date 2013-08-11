#include	<stdlib.h>
#include <stdio.h>
#include	<sys/ioctl.h>
#include	<net/if.h>
#include	<net/route.h>
#include  <string.h>
#include  <dirent.h>
#include 	<sys/ioctl.h>
#include 	<sys/types.h>
#include 	<sys/socket.h>
#include 	<net/if.h>
#include 	<net/route.h>
#include 	<netinet/in.h>
#include 	<arpa/inet.h>
#include 	<net/if_arp.h>
#include 	<signal.h>
#include <errno.h>
#include <fcntl.h>
#include 	<syslog.h>

#define cprintf(fmt, args...) do { \
        FILE *fp = fopen("/dev/ttyS1", "w"); \
        if (fp) { \
                fprintf(fp, fmt, ## args); \
                fclose(fp); \
        } \
} while (0)

int JS_Send(int s, char * buf, int len, int flag)
{
   int            count,idx,ret,err=0,loop=0;
   fd_set         inset;
   struct timeval timeout;
 
   timeout.tv_sec  = 10;
   timeout.tv_usec = 0;
 
   FD_ZERO(&inset);
 FD_SET(s, &inset);
  
   count=len;idx=0;ret=0;
  
   while(count)
   {
     ret = send(s,buf+idx,count,MSG_NOSIGNAL);
     if (ret<=0)
  { 
   if ((errno==EAGAIN)||(errno==EWOULDBLOCK))
   {
    err=select(s+1,NULL,&inset,NULL,&timeout);
          if(err<=0)
          {
            printf("in JS_SEND select is failed \n");
            return -1;
          }
          else
          {
            continue;
          }
   }
      else
      {
          printf("send data error, the errno is:%d\n", errno) ;
          return -1;
      }
  }
  count-=ret;
  idx+=ret;
 }
 return len;
}

static int File_Set_Modem_Info(char *name,char *value)
{
	FILE *fp=NULL;
	char path[64];

	memset(path,'\0',64);
	sprintf(path,"%s%s","/var/usbmodem/",name);

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

static int File_Get_Modem_Info(char *name,char *value, int len)
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

void Get_CMCC_Param()
{
	int 				sockfd, i, ret;	
	struct sockaddr_in 	addr;
	struct hostent 		*pURL;
	char 				header[2048] = "", header_get[1024]="";
	unsigned char		*tmpptr=NULL;
	static char 		text[8192*2];
	fd_set 				fdset;	
	struct timeval  	timeout;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("1.1.1.1");
	addr.sin_port = htons(80);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1)	
	{
		cprintf("\n1.socket failed\n");
		return;
	}	

	if(connect(sockfd, &addr, sizeof(addr))==-1)
	{
		cprintf("\n2.connect failed,errno=%d\n",errno);
		close(sockfd);
		return;
	}

	strcat(header, "GET / HTTP/1.1\r\n");
	strcat(header, "HOST: 1.1.1.1\r\n");
	strcat(header, "\r\nConnection: Keep-Alive\r\n\r\n");

	#if 0
	strcat(header, "POST /bpss/jsp/do_login.jsp HTTP/1.1\r\n");
	strcat(header, "Host: 221.179.9.18:8080\r\n");
	//strcat(header, "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.3) Gecko/20061201 Firefox/2.0.0.3 (Ubuntu-feisty)\r\n"); 
	//strcat(header, "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"); 
	//strcat(header, "Accept-Language: zh-cn,zh;q=0.5\r\n");
	//strcat(header, "Accept-Encoding: gzip,deflate\r\n");
	//strcat(header, "Accept-Charset: gb2312,utf-8;q=0.7,*;q=0.7\r\n");
	//strcat(header, "Keep-Alive: 300\r\n");
	strcat(header, "Connection: keep-alive\r\n"); 
	strcat(header, "Referer: http://221.179.9.18:8080/bpss/jsp/do_login.jsp\r\n");
	//strcat(header, "Cookie: usrtime=1178023651; lasturl=http%3A%2F%2Fwww.su.zju.edu.cn%2Fhack%2Fvisit.php%3Ftype%3D1; loginurl=http%3A%2F%2Fwww.su.zju.edu.cn%2Fhack%2Fvote.php%3Ftype%3D2; usrsid=XgVB1KO7mVEanJwLbwJ0lqXBSHErTAcN; usripfrom=Unknow; usrtime=1178023650; lasturl=http%3A%2F%2Fwww.su.zju.edu.cn%2F\r\n");
	strcat(header, "Content-Type: application/x-www-form-urlencoded\r\n");
	strcat(header, "Content-Length: 163\r\n\r\n");
	//strcat(header, "wlanacname=0101.0755.200.00&wlanuserip=172.16.15.138&actiontype=LOGIN&wlanacssid=&logonsessid=&pwdtype=1&bpssUSERNAME=13714774312&bpssBUSPWD=222468&bpssLoginType=1\r\n");
	strcat(header, "wlanacname=0101.0755.200.00&wlanuserip=172.16.15.138&actiontype=LOGIN&wlanacssid=&logonsessid=&pwdtype=1&bpssUSERNAME=13714774312&bpssBUSPWD=222468&bpssLoginType=1\r\n");
	#endif 
	
	if(send(sockfd, header, strlen(header), 0)==-1)
	{
		cprintf("\n5.send failed\n");
		close(sockfd);
		return;
	}

	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);
	timeout.tv_sec = 25;
	timeout.tv_usec =0;

	ret=select(sockfd+1 , &fdset , NULL, NULL, &timeout);
	if(ret <= 0)
	{
		cprintf("call to select failed,ret=%d,errno=%d\n",ret,errno);
		close(sockfd);
		return ;
	}

	if(recv(sockfd, text, sizeof(text), 0)==-1)	
	{
		cprintf("\n6.recv failed\n");
		close(sockfd);
		return;
	}
	cprintf("\n%s\n",text);
	close(sockfd);

	//Location: http://221.179.9.18:8080/bpss/login.jsp?wlanacname=0101.0755.200.00&wlanuserip=172.16.15.138&ssid=CMCC&wlanacip=221.179.22.74
}

void Send_CMCC_Cmd_Setp(int index, unsigned char *cookeie, unsigned char *url, unsigned char *wlanacname, unsigned char *wlanuserip, unsigned char *cmccLoginUrlRe)
{
	int 								sockfd, i, ret, num=0, send_ret=0;	
	struct sockaddr_in 	addr;
	struct hostent 			*pURL;
	char 								header[2048] = "", logout[1024]="", cmd[1024]="", logout_str[512],url_ip[16];
	char 								logoutUrl[64],content_length[64],logout_str_re[1024],loginUser[16],loginwlandig[128],login_res[4],offline_res[4],loginwlandig_t[256];
	unsigned int				url_port;
	unsigned char				*tmpptr=NULL, *pcookie=NULL, *logoutPtr=NULL;
	static char 				text[128*1024];
	fd_set 							fdset;	
	struct timeval  		timeout;
	struct linger     	m_sLinger;
	int timeoutcount;

	if(strchr(url, ':'))
	{
		for(i=0; url[i] !=':'; i++)
			url_ip[i] = url[i];
		
		url_ip[i++] = '\0';

		url_port = atoi((url+i));
	}
	else
	{
		strcpy(url_ip, url);
		url_port = 80;
	}	

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(url_ip);
	addr.sin_port = htons(url_port);
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1)	
	{
		cprintf("\n1.%d.Send_CMCC_Post_Cmd socket failed\n",index);
		return;
	}
	
	m_sLinger.l_onoff = 1;
  	m_sLinger.l_linger = 0;
  	setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(m_sLinger));	

	if(connect(sockfd, &addr, sizeof(addr))==-1)
	{
		cprintf("\n2.%d.Send_CMCC_Post_Cmd connect failed,errno=%d\n",index,errno);
		close(sockfd);
		return;
	}

	memset(header, '\0', sizeof(header));
	if(2==index)
	{
		sprintf(header, "GET %s HTTP/1.1\r\n", cmccLoginUrlRe);
		strcat(header, 	"Cookie: JSESSIONID=");
		strcat(header, 	cookeie);
		strcat(header, 	"\r\n");
		strcat(header, 	"Host: ");
		strcat(header,  url);
		strcat(header,  "\r\n");
		strcat(header, 	"Connection: Close\r\n\r\n");
		
		//cprintf("\nindex = %d, header = %s\n",index,header);
	}
	else if(3==index) //logout
	{
  	if(!strcmp(url,"221.179.9.18:8080"))			//shenzhen
  	{
  		memset(logout_str, '\0', sizeof(logout_str));
  		File_Get_Modem_Info("logoutparameter", logout_str, sizeof(logout_str));
  		
  		//cprintf("\n%s\n",logout_str);
  		
  		if(strlen(logout_str)>0)
  		{
  			memset(logoutUrl, '\0', sizeof(logoutUrl));
				for(i=0; logout_str[i] != '?'; i++)
					logoutUrl[i] = logout_str[i];
				
				
				logoutPtr=strstr(logout_str, "LOGINIP");
				memset(logout_str_re, '\0', sizeof(logout_str_re));
				sprintf(logout_str_re, "logouttype=TYPESUBMIT&%s", logoutPtr);
				
				memset(loginUser, '\0', sizeof(loginUser));
				if(logoutPtr=strstr(logout_str,"LOGINNAME"))
				{
					logoutPtr += 10;
					for(i=0; *logoutPtr != '&'; i++,logoutPtr++)
						loginUser[i] = *logoutPtr;
				}
				
				memset(loginwlandig, '\0', sizeof(loginwlandig));
				if(logoutPtr=strstr(logout_str,"WLANDIG"))
				{
					logoutPtr += 8;
					for(i=0; *logoutPtr != '&'; i++,logoutPtr++)
						loginwlandig[i] = *logoutPtr;
				}

				if(strlen(loginwlandig)>4)
				{
					memset(loginwlandig_t, '\0', sizeof(loginwlandig_t));
					 for(i=0;*(loginwlandig+i);i++)
					{
					  sprintf(loginwlandig_t+i*3,"%s%02x","%",*(loginwlandig+i));
					}

				}
				else
				{
					sprintf(loginwlandig_t, "%s", loginwlandig);
				}
  		}
  		else
  		{
  					strcpy(logoutUrl,"/bpss/jsp/do_logout.jsp");
  					strcpy(logout_str_re,"logouttype=TYPESUBMIT&LOGINIP=183.236.12.3&LOGINNAME=13798218854&DEVICEID=0101.0755.200.00&WLANDIG=9gzQ4eLybdkYc2vUoq9TjQ==&WLANSID=E81ADDC6D493722A3DC066F8D0CB2C6F");
  					strcpy(loginUser,"13800138000");
  					strcpy(loginwlandig,"9gzQ4eLybdkYc2vUoq9TjQ%3d%3d");
  		}
  		
		#if 1
			sprintf(header, "GET %s?actiontype=LOGOUT&logonsessid=%s&wlanuserip=%s&wlanacname=%s&USER=%s&WLANDIG=%s&logouttype=TYPESUBMIT HTTP/1.1\r\n",logoutUrl,cookeie,wlanuserip,wlanacname,loginUser,loginwlandig_t);
			strcat(header, 	"Host: ");
			strcat(header,  url);
			strcat(header, "\r\nConnection: Keep-Alive\r\n");
			strcat(header, "Cache-Control: no-cache\r\n");
			strcat(header, 	"Cookie: JSESSIONID=");
			strcat(header, 	cookeie);
			strcat(header, 	"\r\n\r\n");

			//cprintf("\n%s\n",header);
		#else
			memset(content_length, '\0', sizeof(content_length));
			sprintf(header, "POST %s? HTTP/1.1\r\nReferer: http://%s/%s\r\n",logoutUrl,url,cmccLoginUrlRe);
			strcat(header, "Content-Type: application/x-www-form-urlencoded\r\n");
			strcat(header, 	"Host: ");
			strcat(header,  url);
			sprintf(content_length, "\r\nContent-Length: %d\r\n", strlen(logout_str_re));
			strcat(header, content_length);
			strcat(header, "Connection: Keep-Alive\r\n");
			strcat(header, "Cache-Control: no-cache\r\n");
			strcat(header, 	"Cookie: JSESSIONID=");
			strcat(header, 	cookeie);
			strcat(header, 	"\r\n\r\n");
			strcat(header, logout_str_re);
			
			//cprintf("\n%s\n",header);
		#endif
  	}
  	else
  	{
  		sprintf(header, "GET /do_logout.php?wlanacname=%s&wlanuserip=%s&actiontype=LOGOUT&logonsessid=%s HTTP/1.1\r\n",wlanacname,wlanuserip,cookeie);	
  		strcat(header, 	"Host: ");
  		strcat(header,  url);
  		strcat(header,  "\r\n");
  		strcat(header, 	"Connection: Keep-Alive\r\n\r\n");
  	}
	}

	if(fcntl(sockfd, F_SETFL, O_NONBLOCK)<0)
  {
	    cprintf("Send_CMCC_Post_Cmd SetSockBlockMode failed!\n");
	    close(sockfd);
			return;
  }

	send_ret = JS_Send(sockfd, header, strlen(header), 0);
	if(send_ret == -1)
	{
		cprintf("\n5.send failed\n");
		close(sockfd);
		return;
	}
	
	
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	timeout.tv_sec = 15;
	timeout.tv_usec =0;
	num = 0;
	timeoutcount=0;

	while(1)
	{
		ret=select(sockfd+1 , &fdset , NULL, NULL, &timeout);

		if(ret < 0) //errno
		{
			cprintf("Send_CMCC_Post_Cmd call to select failed,ret=%d,errno=%d,%d\n",ret,errno,index);
			break;
		}else if(ret==0){//timeout
			timeoutcount++;
		}

		if(timeoutcount==2){
			timeoutcount=0;
			break;
		}
				
		if((ret=recv(sockfd, text+num, sizeof(text), 0))<=0)	
		{
			if ((errno==EAGAIN)||(errno==EWOULDBLOCK))
			{
				cprintf("\nrecv EAGAIN EWOULDBLOCK,timeoutcount=%d\n",timeoutcount);
				continue;
			}
			else
			{
				break;
			}				
		}
		else
		{
			printf("\nnum = %d, ret = %d\n",num,ret);
			num += ret;
		}
	}	
	close(sockfd);
	
	if(2==index)
	{	
		//cprintf("\n%s\n",text);
		//login_res[4],offline_res[4]
		//syslog(LOG_DEBUG,"text: %s!\n",text);
		if((tmpptr=strstr(text, "login_res|")))
		{
			tmpptr += 10;
			memset(login_res, '\0', sizeof(login_res));
			for(i=0; *tmpptr != '|'; i++, tmpptr++)
					login_res[i] = *tmpptr;
					
			if(!strcmp(login_res,"0"))
			{
				if((tmpptr=strstr(text, "window.open('/bpss/jsp/do_logout.jsp?")))
				{
					tmpptr += 13;
					memset(logout, '\0', sizeof(logout));
					memset(cmd, '\0', sizeof(cmd));
					for(i=0; *tmpptr != ','; i++, tmpptr++)
						logout[i] = *tmpptr;
					logout[i-1] = '\0';	
						
					File_Set_Modem_Info("logoutparameter", logout);				
				}
			}
			File_Set_Modem_Info("login_res", login_res);
			unlink("/var/usbmodem/offline_res");
		}
	}
	else if(3==index)		//logout
	{	
		//cprintf("\n%s\n",text);
		
		if(!strcmp(url,"221.179.9.18:8080"))			//shenzhen
		{
				if((tmpptr=strstr(text, "offline_res|")))
				{
					tmpptr += 12;
					memset(offline_res, '\0', sizeof(offline_res));
					for(i=0; *tmpptr != '|'; i++, tmpptr++)
							offline_res[i] = *tmpptr;
				}
			
				if(!strcmp(offline_res,"0"))
				{
					cprintf("logout successful");
					unlink("/var/usbmodem/logoutparameter");
					unlink("/var/usbmodem/cmccLoginUrlRe");
					unlink("/var/usbmodem/cookeie");
				}
				else
					cprintf("logout failed shenzhen");
					
				File_Set_Modem_Info("offline_res", offline_res);
				unlink("/var/usbmodem/login_res");
		}
		else
		{
			if(strstr(text,"window.close()"))
			{
				cprintf("logout successful");
				File_Set_Modem_Info("offline_res", "0");
				unlink("/var/usbmodem/cookeie");
			}
			else
			{
				cprintf("logout failed jituan");
				File_Set_Modem_Info("offline_res", "99");			//logout exception
			}
			unlink("/var/usbmodem/login_res");
		}
		
	}
}

int main(int argc, char **argv)
{
		int i=0;
		char cookeie[64],cmccLoginUrlRe[512];
		if(argc<2)
			cprintf("please input right param\n");

		//argv[1]=url
		//argv[2]=wlanacname
		//argv[3]=wlanuserip
		//argv[4]= -
		//argv[5]=cmccindex
		File_Get_Modem_Info("cookeie",cookeie,sizeof(cookeie));
		if(atoi(argv[4]) == 0)	//logout
		{
			cprintf("\n****cookeie******len=%d******value=%s**************\n",strlen(cookeie),cookeie);
			if(strlen(cookeie) < 8)
			{
				cprintf("\nlogout filed because cookeie is Null!\n");
				return 0;
			}

			if(!strcmp(argv[1],"221.179.9.18:8080"))			//shenzhen
			{
				File_Get_Modem_Info("cmccLoginUrlRe",cmccLoginUrlRe,sizeof(cmccLoginUrlRe));
				Send_CMCC_Cmd_Setp(3, cookeie, argv[1], argv[2], argv[3], cmccLoginUrlRe);	
			}
			else
				Send_CMCC_Cmd_Setp(3, cookeie, argv[1], argv[2], argv[3], "-");	
		}
		else					//login
		{
			File_Get_Modem_Info("cmccLoginUrlRe",cmccLoginUrlRe,sizeof(cmccLoginUrlRe));
			Send_CMCC_Cmd_Setp(2, cookeie, argv[1], argv[2], argv[3], cmccLoginUrlRe);	
		}
		
		return 0;
}