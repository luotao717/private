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
#include <syslog.h>
#include <pthread.h>

#include "cmd.h"
#include "pcm_ctrl.h"

#define MALLOC_MAX_PACKET_SIZE  (32*1024)

typedef struct  tagDestObj
{
  int               front;
  int               rear;
  int               count;
  int				uncount;	//无用的数据
  int               maxsize;
  unsigned char     canin;     //判断数据是否能进入
  unsigned char     uses;
  char              pcmbuf[MALLOC_MAX_PACKET_SIZE];  
}DestObj,*PDestObj;

PDestObj  			ppcmobj=NULL;
pthread_mutex_t   	mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t    	cond=PTHREAD_COND_INITIALIZER;
static int  		pcm_fd=0;

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

  //SetSockBlockMode(skfd, 1);
}


int JS_Recv(int s, char * buf, int len, int flag)
{
  int count,idx,ret;
	count=len;idx=0;ret=0;
	while (count)
	{
		ret=recv(s,buf+idx,count,flag);
		if (0>=ret)
		{
			return -1;
		}
		count-=ret;
		idx+=ret;
	}
	return len;
}

int init_param(PDestObj pobj)
{
	pobj->front= 0;
	pobj->rear = 0;
	pobj->count= 0;
	pobj->canin = 1;
	pobj->maxsize = MALLOC_MAX_PACKET_SIZE;
	memset(pobj->pcmbuf, '\0', MALLOC_MAX_PACKET_SIZE);

	return 0;
}


int destlist_tcp_getdata(PDestObj obj,char* pbuf, int length)
{
  int size=0;
  int curpos;
 
  pthread_mutex_lock(&mutex);

  printf("out\n");
  
  if (obj->count<=0)
  {
	    pthread_mutex_unlock(&mutex);
	    return 0;  
  }
  if (obj->count>=length)
  {
		size=length;
		curpos=obj->maxsize-obj->front;
		if (curpos>=size)
	    {     
			memcpy(pbuf,obj->pcmbuf+obj->front,size);
			obj->front+=size;
			obj->front%=obj->maxsize;
			obj->count-=size;
		}
		else
		{		  
			memcpy(pbuf,obj->pcmbuf+obj->front,curpos);
			memcpy(pbuf+curpos,obj->pcmbuf,size-curpos);      
			obj->front+=size;
	    	obj->front%=obj->maxsize;
			obj->count-=size;			
		}
	}
	else
	{
		printf("No enough data.\n") ;
	}
    
	pthread_mutex_unlock(&mutex);
	return size;
}


void destlist_tcp_inputdata(PDestObj obj,char* data, int len, int res)
{
  	int size,headsize,bufsize;

	pthread_mutex_lock(&mutex);
 
  	size=len;
	
	if (obj->canin == 0x01)
	{
		printf("in\n");
		if ((obj->maxsize-obj->count)>=size)
		{	
		  	bufsize=obj->maxsize-obj->rear;
      
      		obj->canin=0x01;

			if (bufsize>=len)
      		{
				memcpy(obj->pcmbuf+obj->rear,data,len);
			  	obj->rear+=len;
			  	obj->rear%=obj->maxsize;
				obj->count+=len;
			}
			else 
			{		   
			  	memcpy(obj->pcmbuf+obj->rear,data,bufsize);
			  	memcpy(obj->pcmbuf,data+bufsize,len-bufsize);
			  	obj->rear+=len;
			  	obj->rear%=obj->maxsize;
			  	obj->count+=len;          
			}   
      		pthread_cond_signal(&cond); 
		}
		else 
		{		  
			obj->canin=0x00;
      		//printf("Free video buffer size less than data size.\n") ;
		}
	}

	pthread_mutex_unlock(&mutex);
	return;
}


int put_pcm_task()
{
	char pbuf[1024];
	int  len,ret=-1;

	while(1)
	{
		ret=pthread_mutex_lock(&mutex);
		if(ret != 0)
		{
			printf("mutex lock ret=%d,error=%d\n",ret,errno);
			sleep(10);
		}
		
		ret=pthread_cond_wait(&cond,&mutex);
		if(ret != 0)
		{
			printf("cond wait ret=%d,error=%d\n",ret,errno);
			sleep(10);
		}

		
		ret=pthread_mutex_unlock(&mutex);
		if(ret != 0)
		{
			printf("mutex unlock ret=%d,error=%d\n",ret,errno);
			sleep(10);
		}

		memset(pbuf, '\0', sizeof(pbuf));

		while(1)
		{
			len = 0;
      		len = destlist_tcp_getdata(ppcmobj,pbuf,160) ;
      		if(len<160)
      		{
        		printf("Video buffer is empty!\n") ;
        		//break;
      		}

			ioctl(pcm_fd, PCM_PUTDATA, pbuf);	
		}
	}		
}

int get_pcm_task()
{
	int			          n,i,on,idx;
	int			          ret, nLen=0;
	int			          sin_len;
  	int			          pin_len;
  	int			          skfd;
  	fd_set			      acpfdset,rdset,wdset;
	struct timeval		  timeout;
	struct sockaddr_in 	  sin;
	struct sockaddr_in 	  pin;
	MySockInfo		      skinfo;
  	char              	  path[100],pcmdata[1024*2], *pcmd=NULL;
	FILE                  *fpcm;
	struct linger         m_sLinger;
	pcm_record_type 	  pcm_record;
	pcm_playback_type 	  pcm_playback;
	pthread_t 			  putthread;

	bzero(&sin,sizeof(sin));
  	sin.sin_family = AF_INET;
  	sin.sin_addr.s_addr = htonl(INADDR_ANY);
  	sin.sin_port   = htons(RCV_PORT);
  	sin_len = sizeof(sin);
	
	pcm_fd = open("/dev/pcm0", O_RDWR); 	
	if(pcm_fd < 0) 
	{
		printf("open pcm driver failed (%d)...exit\n",pcm_fd);
		return -1;
	}

  	skfd = socket(AF_INET,SOCK_STREAM,0);
  	if(skfd<0)
  	{
    	printf("in netrcv.c socket failed!!\n");
    	return -1;
  	}

  	on = 1;
  	if ( setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on) ) < 0 )
  	{
    	printf("in netrcv.c setsockopt failed!!\n");
    	close(skfd);
    	return -1;
  	}

  	ret = bind(skfd, (struct sockaddr* )&sin, sizeof(sin));
	if(ret < 0)
	{		
		close(skfd);
		printf("in netrcv.c bind failed\n");
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
			printf("in netrcv.c select failed\n");
			continue;
		}
    	printf("in netrcv.c select success\n");
    
    	skinfo.fd = accept(skfd, (struct sockaddr* )&skinfo.sin, &skinfo.sinlen);
		if(skinfo.fd < 0) 
    	{
      		printf("in netrcv.c accept failed\n");
      		continue;
    	}

    	printf("in netrcv.c accept success\n");

		/*
		fpcm = fopen("/var/recv.pcm","rb");
		if(fpcm==NULL)	    	
		{
			printf("open pcm file failed..exit\n");
			return -1;
		}
		*/

		FD_ZERO(&rdset);
		FD_SET(skinfo.fd, &rdset);
		FD_ZERO(&wdset);
		FD_SET(skinfo.fd, &wdset);

		SetSockOpt(skinfo.fd);

		memset(pcmdata, '\0', sizeof(pcmdata));

		sleep(3);

		while(1)
		{
			/*
			ret = select(skinfo.fd+1, &rdset, NULL, NULL, NULL);
			if(ret <=0 )
			{
				close(skinfo.fd);
				break;
			}
			*/
			
			ret = recv(skinfo.fd, pcmdata, sizeof(pcmdata), 0);
			if(ret<=0)
			{
				//fclose(fpcm);
				close(skinfo.fd);
				break;
			}

			//printf("recv ret = %d\n",ret);

			if(ret%160)
				continue;

			for(i=0; i<(ret/160); i++)
				ioctl(pcm_fd, PCM_PUTDATA, pcmdata+160*i);

			//destlist_tcp_inputdata(ppcmobj,pcmdata,ret,0);

			//printf("recv ret = %d\n",ret);
			
			//fwrite(pcmdata, 1, ret, fpcm);

			//fread(pcmdata, 1, PCM_FIFO_SIZE, fpcm);		
			
		}		
  	}
}

int main(int argc, char *argv[])
{

	pthread_t 			  putthread,getthread;
	  
	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

	//pthread_mutex_init(&mutex,NULL);
	//pthread_cond_init(&cond,NULL);
	/*
	ppcmobj = (PDestObj)malloc(sizeof(DestObj));
	if(NULL==ppcmobj)
	{
	    printf("in netrcv.c AddObjTODestList malloc failed!\n");
	    return -1;
	}
	
	if ( pthread_create( &getthread, NULL, (void*)get_pcm_task, NULL) ) 
	{
		printf("error creating get_pcm_task thread.");
		return -1;
	}
	*/

	/*	
  	if ( pthread_create( &putthread, NULL, (void*)put_pcm_task, NULL) ) 
	{
		printf("error creating put_pcm_task thread.");
		return -1;
	}
	*/

	get_pcm_task();
}
