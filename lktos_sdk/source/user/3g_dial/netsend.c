#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <netinet/tcp.h> 

#include "cmd.h"
#include "pcm_ctrl.h"

#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/ttyS1", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)


//设置socket属性，0:阻塞，1:非阻塞
int SetSockBlockMode(int skfd, int flag)
{
  if(flag)
  {
    if(fcntl(skfd, F_SETFL, O_NONBLOCK)<0)
    {
      printf("in def.c SetSockBlockMode failed!\n");
      return -1;
    }
  }
  return 1;
}


void SetSockOpt(int skfd)
{
  struct linger     m_sLinger;
  int               optval,flag,on;
  
  m_sLinger.l_onoff = 1; // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
  m_sLinger.l_linger = 0; // (容许逗留的时间为0秒)
  setsockopt(skfd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(m_sLinger));

  /*	
  optval=128*1024;
  setsockopt(skfd,SOL_SOCKET,SO_SNDBUF,(char *)(&optval),sizeof(optval));
  */

  on = 1;
  setsockopt (skfd, IPPROTO_TCP, TCP_NODELAY , &on, sizeof (on));

  SetSockBlockMode(skfd, 1);
}


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



int main(int argc, char *argv[])
{
  	int			          n,i,on,idx,pcm_fd;
	int			          ret, nLen=0;
	int			          sin_len;
  	int			          pin_len;
  	int			          skfd;
  	fd_set			      acpfdset,rdset,wdset;
	struct timeval		  timeout;
	struct sockaddr_in 	  sin;
	struct sockaddr_in 	  pin;
	MySockInfo		      skinfo;
  	char              	  path[100],pcmdata[20*1024], *pcmd=NULL, req_cmd[64];
	FILE                  *fpcm,*fp_pcm;
	struct linger         m_sLinger;
	char 				  buffer[2*1024];
	pcm_record_type 	  pcm_record;
	pcm_playback_type 	  pcm_playback;

	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

  	bzero(&sin,sizeof(sin));
  	sin.sin_family = AF_INET;
  	sin.sin_addr.s_addr = htonl(INADDR_ANY);
  	sin.sin_port   = htons(SEND_PORT);
  	sin_len = sizeof(sin);

	system("rmmod ralink_pcm.ko");
	sleep(2);

	system("insmod ralink_pcm.ko");
	sleep(3);	

	pcm_fd = open("/dev/pcm0", O_RDWR); 	
	if(pcm_fd < 0) 
	{
		printf("open pcm driver failed (%d)...exit\n",pcm_fd);
		return -1;
	}

			
	//ioctl(pcm_fd, PCM_START, NULL);
	//sleep(2);

	//memset(req_cmd, '\0', sizeof(req_cmd));
	//sprintf(req_cmd, "netrcv %c", (char)pcm_fd);
	//system(req_cmd);

	skfd = socket(AF_INET,SOCK_STREAM,0);
  	if(skfd<0)
  	{
    	printf("in netdata.c socket failed!!\n");
    	return -1;
  	}

  	on = 1;
  	if ( setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on) ) < 0 )
  	{
    	printf("in netdata.c setsockopt failed!!\n");
    	close(skfd);
    	return -1;
  	}
  

  	ret = bind(skfd, (struct sockaddr* )&sin, sizeof(sin));
	if(ret < 0)
	{		
		close(skfd);
		printf("in netdata.c bind failed\n");
		return -1;
	}

  	listen(skfd, 5);
	FD_ZERO(&acpfdset);
	FD_SET(skfd, &acpfdset);

	timeout.tv_sec = 10;
    timeout.tv_usec =0;

  	while(1)
  	{   
    	ret = select(skfd + 1, &acpfdset, NULL, NULL, NULL);
		if(ret < 0)
		{
			printf("in netdata.c select failed\n");
			continue;
		}
    	printf("in netdata.c select success\n");
    
    	skinfo.fd = accept(skfd, (struct sockaddr* )&skinfo.sin, &skinfo.sinlen);
		if(skinfo.fd < 0) 
    	{
      		printf("in netdata.c accept failed\n");
      		continue;
    	}

    	printf("in netdata.c accept success\n");		
		
		FD_ZERO(&rdset);
		FD_SET(skinfo.fd, &rdset);
		FD_ZERO(&wdset);
		FD_SET(skinfo.fd, &wdset);		

		SetSockOpt(skinfo.fd);

		/*
		fp_pcm = fopen("/var/record.pcm","wb");
		if(fp_pcm==NULL)
		{
			printf("open pcm file failed..exit\n");
			return -1;
		}
		*/

		memset(buffer, 0, sizeof(buffer));

		{
			long param[2];
			param[0] = 0; ///ch
			param[1] = 1; /// for ulaw
			
			ioctl(pcm_fd, PCM_START, 1);
			ioctl(pcm_fd, PCM_SET_CODEC_TYPE, param);
		}		

		while(1)
		{
			//ioctl(pcm_fd, PCM_READ_PCM, &pcm_record);

			ioctl(pcm_fd, PCM_GETDATA, buffer+PCM_FIFO_SIZE);
			
			//fwrite(buffer+PCM_FIFO_SIZE, 1, PCM_FIFO_SIZE, fp_pcm);
			
			//ret = JS_Send(skinfo.fd, pcm_record.pcmbuf, pcm_record.size, 0);
			ret = JS_Send(skinfo.fd, buffer+PCM_FIFO_SIZE, PCM_FIFO_SIZE, 0);
			if(ret<=0)
			{
				//fclose(fp_pcm);
				close(skinfo.fd);
				ioctl(pcm_fd, PCM_STOP, 1);
				break;
			}				
		}	
  	}
}

