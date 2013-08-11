#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
//#include "jrtp/rtpsession.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ortp/ortp.h>  
#include <signal.h>
#include <sys/time.h>  
#include <sys/un.h>
#include <signal.h>
#include "nvram.h"
#include "pcm_ctrl.h"
#include "3gdial.h"

#define MALLOC_MAX_PACKET_SIZE  	(160*10*2)//(10*1024)//(32*1024)
#define SIG_CALL_MSG				(SIGUSR1+58)
#define SIG_EXIT_OSIP_THREAD		(SIGUSR1+60)

pthread_mutex_t 	mutex_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t 	mutex_recv = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t 	mutex_put  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t    	cond_put   = PTHREAD_COND_INITIALIZER;

struct osip_thread *audio_send_thread, *audio_recv_thread, *local_socket_thread, *put_audio_thread;

eXosip_event_t 		*je = NULL;
RtpSession 			*pRtpSession = NULL;
int 				call_id, dialog_id;
char 				lan_addr[16],rtp_addr[32], rtp_port[16], g_callin_num[32];
static int 			fd_com=0,pcm_fd=0;
int					g_stop_media = 0, g_hungup = 0;
char 				source_call[128];//*source_call = "sip:13510244170@192.168.0.1";   
char 				dest_call[128];//*dest_call = "sip:1000@192.168.0.2:5060";

typedef struct  tagDestObj
{
  int               front;
  int               rear;
  int               count;
  int               maxsize;
  unsigned int		uncount;
  unsigned char     canin;     //判断数据是否能进入
  unsigned char     uses;
  char              pcmbuf[MALLOC_MAX_PACKET_SIZE];  
}DestObj,*PDestObj;

typedef struct tagRtp_Info
{
	char addr[32];
	char port[32];
	char s_name[32]; //session name
}Rtp_Info_S;

typedef struct tagRecvComInfo
{
	int 			type; 		//0 : hung up  1 : call coming
	unsigned char 	info[128];
}RecvComInfo,*PRecvComInfo;

Rtp_Info_S 			Rtp_Info, Regist_Info;
DestObj  			pcmobj;

// 时间戳增量
#define TIME_STAMP_INC 		160
#define TIME_STAMP_INC_REV 	160
//#define TIME_STAMP_INC 	100
#define BYTES_PER_COUNT	256
 
// 时间戳   
uint32_t g_user_ts, g_user_ts_rcv;


int ULawDecode(unsigned char ulaw)	
{	
	short 		linear;
	unsigned 	shift;
		
	ulaw ^= 0xff;  // u-law has all bits inverted for transmission

	linear = ulaw&0x0f;

	linear <<= 3;

	linear |= 0x84;  // Set MSB (0x80) and a 'half' bit (0x04) to place PCM value in middle of range

	shift = ulaw>>4;

	shift &= 7;

	linear <<= shift;

	linear -= 0x84; // Subract uLaw bias

	if(ulaw&0x80)
		return -linear;
	else
		return linear;
}

void lock(pthread_mutex_t *mutex)
{
  pthread_mutex_lock(mutex);  
  return;
}

void unlock(pthread_mutex_t *mutex)
{
  pthread_mutex_unlock(mutex);
  return;
}

void wait_cond(pthread_mutex_t *mutex, pthread_cond_t *cond)
{
	pthread_cond_wait(mutex,cond);
	return;
}

void signal_cond(pthread_cond_t *cond)
{
	pthread_cond_signal(cond); 
}

void kill_signal()
{
	/*
	int pid_netcall=0;
	
	pid_netcall = DBgetPid("netcall");
	if(pid_netcall>0)
	{
		kill(pid_netcall, SIG_CALL_MSG);
	}
	*/

	FILE *fp_callin=NULL;
	fp_callin = fopen("/var/.call_end","w");
	if(fp_callin==NULL)
	{
		printf("open /var/call_end failed..exit\n");
		return;
	}
	fclose(fp_callin);
	return;
}

int getIpaddr(char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, "br0", strlen("br0"));
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	close(skfd);
	return 0;
}


void start_pcm(int fd)
{
	ioctl(fd, PCM_START, 1);
}

void stop_pcm(int fd)
{
	ioctl(fd, PCM_STOP, 1);
}

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

void dial_num(int fd_com, char *num)
{
	char	cmd[64];

	memset(cmd, '\0', sizeof(cmd));
	sprintf(cmd, "ATD%s;", num);

	if(fd_com>0)
		write_str(fd_com, cmd, strlen(cmd));
}

void hung_up(int fd_com)
{	
	if(fd_com>0)
		write_str(fd_com, "AT+CHUP", strlen("AT+CHUP"));
}

void line_on(int fd_com)
{	
	if(fd_com)
		write_str(fd_com, "ATA", strlen("ATA"));
}


int init_obj(PDestObj obj)
{
	obj->front= 0;
	obj->rear = 0;
	obj->count= 0;
	obj->uncount = 0;
	obj->canin = 1;
	obj->maxsize = MALLOC_MAX_PACKET_SIZE;
	memset(obj->pcmbuf, '\0', MALLOC_MAX_PACKET_SIZE);

	return 0;
}

int destlist_getdata(PDestObj obj,char* pbuf, int length)
{
  int size=0;
  int curpos;
 
  lock(&mutex_put);

  if (obj->count<=0)
  {
  		//cprintf("\n3333333333, count = %d\n", obj->count);
	    unlock(&mutex_put);
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
  	//else
		//cprintf("\n4444444444444\n");
  
	    
	unlock(&mutex_put);
	return size;
}



void destlist_inputdata(PDestObj obj,char* data, int len, int res)
{
  	int size,headsize,bufsize;

	lock(&mutex_put);
 
  	size=len;
	
	//if (obj->canin == 0x01)
	//{
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
      		//signal_cond(&cond_put); 
		}
		else 
		{		  
			obj->canin=0x00;
      		//cprintf("maxsize = %d, count = %d, size = %d.\n",obj->maxsize, obj->count, size) ;
		}
	//}
	//else
		//cprintf("\n22222222\n");

	unlock(&mutex_put);
	return;
}



/**  初始化     
 *       
 *   主要用于对ortp以及其它参数进行初始化    
 *   @param:  char * ipStr 目的端IP地址描述串     
 *   @param:  iint port 目的端RTP监听端口     
 *   @return:  RtpSession * 返回指向RtpSession对象的指针,如果为NULL，则初始化失败     
 *   @note:       
 */   
RtpSession * rtpInit(char * ipStr,int port)  
{  
    // Rtp会话对象    
    RtpSession *session;
	int status, index;
    char *ssrc;  
      
    // 时间戳初始化    
    g_user_ts = 0; 
	g_user_ts_rcv = 0;

	//status = session.Create(20188);
  	//checkerror(status);
	
    // 创建新的rtp会话对象  
    session=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_recv_buf_size(session,1500);  

	rtp_session_set_scheduling_mode(session,1);  
    rtp_session_set_blocking_mode(session,1);

	//rtp_session_set_scheduling_mode(session,0);  
    //rtp_session_set_blocking_mode(session,0);

	//rtp_session_set_local_addr(session, "192.168.0.1", 20188);
	rtp_session_set_local_addr(session,lan_addr, 20188);
	
    // 设置远程RTP客户端的的IP和监听端口（即本rtp数据包的发送目的地址）   
    rtp_session_set_remote_addr(session,ipStr,port);

	//rtp_session_set_connected_mode(session,1);
    //rtp_session_set_symmetric_rtp(session,1);

	rtp_session_set_jitter_compensation(session,20);
	rtp_session_enable_adaptive_jitter_compensation(session,1);	
	
    // 设置负载类型    
    rtp_session_set_payload_type(session,0);  
      
    // 获取同步源标识    
    ssrc=getenv("SSRC");  
    if (ssrc!=NULL)   
    {  
        //printf("using SSRC=%i.\n",atoi(ssrc));  
        rtp_session_set_ssrc(session,atoi(ssrc));  
    }  
      
    return session;  
 
}

/**  发送rtp数据包     
 *       
 *   主要用于发送rtp数据包     
 *   @param:  RtpSession *session RTP会话对象的指针     
 *   @param:  const char *buffer 要发送的数据的缓冲区地址      
 *   @param: int len 要发送的数据长度     
 *   @return:  int 实际发送的数据包数目     
 *   @note:     如果要发送的数据包长度大于BYTES_PER_COUNT，本函数内部会进行分包处理     
 */ 
int rtpSend(RtpSession *session,const char *buffer, int len)  
{  
    int curOffset = 0;  
    int sendBytes = 0;  
    int clockslide=500;   
    // 发送包的个数  
    int sendCount = 0;  
 
    // 是否全部发送完毕    
    while(curOffset < len )  
    {  
        // 如果需要发送的数据长度小于等于阙值，则直接发送  
        if( len <= BYTES_PER_COUNT )  
        {  
            sendBytes = len;  
        }  
        else 
        {  
            // 如果当前偏移 + 阈值 小于等于 总长度，则发送阈值大小的数据  
            if( curOffset + BYTES_PER_COUNT <= len )  
            {  
                sendBytes = BYTES_PER_COUNT;  
            }  
            // 否则就发送剩余长度的数据  
            else
            {  
                sendBytes = len - curOffset;  
            }  
        }  
          
        rtp_session_send_with_ts(session,(char *)(buffer+curOffset),sendBytes,g_user_ts);  
          
        // 累加  
        sendCount ++;  
        curOffset += sendBytes;                   
        g_user_ts += TIME_STAMP_INC;      

    }  
    return 0;  
}

int rtpRcv(RtpSession *session,const char *buffer, int len)
{
	int have_more = 1;
	int recvd_bytes=0;
	int recvBytes  = 0;  
    int totalBytes = 0;  
	int stream_received=0, err=0;
	mblk_t *mp=NULL;

#if 0
    while(have_more)  
    {  
        recvBytes = rtp_session_recv_with_ts(session, buffer+totalBytes, len, g_user_ts_rcv,&have_more);  
        if (recvBytes <= 0)  
        {  
        	cprintf("\n111111recvBytes = %d, have_more = %d\n", recvBytes, have_more);
            break;  
        }  
        totalBytes += recvBytes;  
    }  
 
    /** 时间戳增加 */   
    g_user_ts_rcv += 160;
		
	//cprintf("\nlen=%d, have_more=%d, recv_bytes=%d, g_user_ts_rcv=%d\n",len,have_more,totalBytes,g_user_ts_rcv);

	return totalBytes;
#else
	ortp_socket_t 		sockfd=session->rtp.socket;
	struct timeval      timeout;
	struct sockaddr 	remaddr;
	socklen_t 			addrlen = sizeof (remaddr);

	fd_set	fdset;
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	err = select(sockfd+1, &fdset, NULL, NULL, &timeout);
	if(err==0) //time out 
	{
		cprintf("select time out\n");
		return 0;
	}
	else if(err<0) //err 
	{
		cprintf("select err\n");
		return -1;
	}

	if (FD_ISSET (sockfd, &fdset))
	{
		err = recvfrom(sockfd, buffer, len, 0, (struct sockaddr *)&remaddr, (socklen_t*)&addrlen);

		if(err == 0)
		{
			cprintf("recvfrom 0\n");
			return 0;
		}			
		else if(err < 0)
		{
			cprintf("recvfrom -1\n");
			return -1;
		}
		else 
			recvd_bytes = err;
	}

	//if(err != 172)
		//cprintf("recvfrom %d\n", recvd_bytes);

	return recvd_bytes;
#endif	
}
 
/**  结束ortp的发送，释放资源     
 *      
 *   @param:  RtpSession *session RTP会话对象的指针     
 *   @return:  0表示成功     
 *   @note:         
 */ 
int rtpExit(RtpSession *session)  
{  
    g_user_ts = 0; 
	g_user_ts_rcv = 0;
      
    rtp_session_destroy(session);  
    //ortp_exit();  
    ortp_global_stats_display();  
 
    return 0;  
}

void send_audio_task()
{
	char pBuffer[2048], pRcvBuf[2048], pulawBuf[1024], pulawSend[1024], plineBuf[1024],*pLiner=NULL;
	char *ptmp=NULL, *pRcvtmp=NULL;
	short output;
	FILE *fp_pcm=NULL, *fp_ulaw=NULL;
	static int o_pcm_flag = 0;
	int  ret=0, i=0, j=0,ulen=0, k=0;
	pcm_record_type pcm_record;

	/*
	fp_pcm = fopen("/var/record.pcm","w+");
	if(fp_pcm==NULL)
	{
		printf("open pcm file failed..exit\n");
		return;
	}

	
	fp_ulaw = fopen("/var/ulaw.pcm","w+");
	if(fp_ulaw==NULL)
	{
		printf("open pcm file failed..exit\n");
		return;
	}
	*/

	memset(pBuffer, '\0', sizeof(pBuffer));
	memset(pRcvBuf, '\0', sizeof(pRcvBuf));
	memset(pulawBuf, '\0', sizeof(pulawBuf));
	memset(pulawSend, '\0', sizeof(pulawSend));
	pcm_record.pcmbuf = pBuffer;

	while(1)  
    {  
		lock(&mutex_send);

		if(g_stop_media)
		{
			unlock(&mutex_send);

			while(g_stop_media)
				sleep(1);
			sleep(1);
			
			continue;
		}
		
		if(pRtpSession)
		{	
			unlock(&mutex_send);
		done:			
			ioctl(pcm_fd, PCM_GETDATA, pBuffer+PCM_FIFO_SIZE);
			ptmp = pBuffer+PCM_FIFO_SIZE;
				//ioctl(pcm_fd, PCM_GETDATA, pBuffer);
				//ptmp = pBuffer;				

				//fwrite(ptmp, 1, PCM_FIFO_SIZE, fp_pcm);

			#if 1	
				for(i=0; i<PCM_FIFO_SIZE/2; i++)
				{
					*(pulawBuf+i) = linear2ulaw(((ptmp[1]&0x00ff)<<8)|(ptmp[0]&0x00ff));
					ptmp += 2;
				}			
				strncpy(pulawSend+ulen, pulawBuf,PCM_FIFO_SIZE/2);
				ulen += PCM_FIFO_SIZE/2;			

				if(ulen < PCM_FIFO_SIZE)
					goto done;
				else
					ulen = 0;

				//fwrite(pulawSend, 1, PCM_FIFO_SIZE, fp_ulaw);

				lock(&mutex_send);
				if(pRtpSession)
				{
					if( rtpSend(pRtpSession,pulawSend,PCM_FIFO_SIZE) != 0)
			        {  
			            //printf("error rtpSend");  
			            break;  
			        }
				}
				else
				{
					unlock(&mutex_send);
					sleep(2);
					continue;
				}
				unlock(&mutex_send);
			#endif	
		}
		else
		{
			unlock(&mutex_send);
			sleep(2);
			continue;
		}
    }
}

void recv_audio_task()
{
	char pBuffer[2048], pRcvBuf[2048],plineBuf[1024],*ptmp=NULL,*pRcvtmp=NULL;
	int  ret=0, i=0, j=0,k=0;
	short output;
	FILE *fp_pcm=NULL, *fp_linear=NULL;
	struct timeval timeout;
  	timeout.tv_sec  = 0;
  	timeout.tv_usec = 20000;
	pcm_playback_type pcm_playback;

	/*
	fp_pcm = fopen("/var/recvulaw.pcm","w+");
	if(fp_pcm==NULL)
	{
		printf("open recv pcm file failed..exit\n");
		return;
	}

	fp_linear = fopen("/var/recvlinear.pcm","w+");
	if(fp_linear==NULL)
	{
		printf("open recv pcm file failed..exit\n");
		return;
	}
	*/


	while(1)
	{
		lock(&mutex_recv);

		if(g_stop_media)
		{
			unlock(&mutex_recv);

			while(g_stop_media)
				sleep(1);
			sleep(1);

			continue;
		}		

		if(pRtpSession)
		{
			#if 1
				if( (ret = rtpRcv(pRtpSession,pRcvBuf, sizeof(pRcvBuf))) >0)
				{
					pRcvtmp = pRcvBuf+12;

					//fwrite(pRcvtmp, 1, ret-12, fp_pcm);

					pcmobj.uncount += (ret-12);
					if(pcmobj.uncount <= 3*MALLOC_MAX_PACKET_SIZE)
					{
						//cprintf("\n11111\n");
						unlock(&mutex_recv);
						continue;
					}					

					/*
					for(i=0,k=0; i<(ret-12); i++)
					{
						output = ulaw2linear(pRcvtmp[i]);
						plineBuf[k] = output & 0x00ff;
						plineBuf[k+1] = (output & 0xff00) >> 8;
						k += 2;
					}
					*/

					//fwrite(plineBuf, 1, 320, fp_linear);
					//fread(plineBuf, 1, 320, fp_linear);
					unlock(&mutex_recv);				
						
					destlist_inputdata((PDestObj)&pcmobj, pRcvtmp, ret-12, 0);
				}
			#else
				if( (ret = rtpRcv(pRtpSession,pRcvBuf, 160)) > 0)
				//if( (ret = rtpRcv(pRtpSession,pRcvBuf, 80)) > 0)
				{
					pRcvtmp = pRcvBuf;

					//fwrite(pRcvtmp, 1, ret, fp_pcm);

					pcmobj.uncount += ret ;
					if(pcmobj.uncount <= 3*MALLOC_MAX_PACKET_SIZE)
					{
						cprintf("\n11111\n");
						unlock(&mutex_recv);
						continue;
					}					

					/*
					for(i=0,k=0; i<(ret-12); i++)
					{
						output = ulaw2linear(pRcvtmp[i]);
						plineBuf[k] = output & 0x00ff;
						plineBuf[k+1] = (output & 0xff00) >> 8;
						k += 2;
					}
					*/

					//fwrite(plineBuf, 1, 320, fp_linear);
					//fread(plineBuf, 1, 320, fp_linear);
					unlock(&mutex_recv);				
						
					destlist_inputdata((PDestObj)&pcmobj, pRcvtmp, ret, 0);
				}
			#endif
				else
				{
					unlock(&mutex_recv);
					continue;
				}
		}
		else
		{
			unlock(&mutex_recv);
			sleep(2);
			continue;
		}
	}	
}

void local_socket_server()
{
	osip_message_t *invite = NULL;
	struct sockaddr_un serv_add;
	const char 	path[]="/var/.tmpsocket";
	int 		servfd;	
	int 		servfda; /*用来接受一个连接*/
	fd_set 		readset; /*描述符集，检测可用连接*/
	int 		ret, i=0;
	int 		len;
	char 		buf[32];
	char 		tmp[1024];
	RecvComInfo	recvinfo;

	unlink(path);
	memset(&serv_add,0,sizeof(serv_add));
	serv_add.sun_family=AF_LOCAL;
	strncpy(serv_add.sun_path,path,strlen(path));
	servfd=socket(AF_LOCAL,SOCK_STREAM,0);
	if(servfd==-1)
	{
		//printf("\nserver socket error!\n");
		return;
	}
	
	len=sizeof(serv_add);	
	if(bind(servfd,(struct sockaddr *)&serv_add,len)!=0)
	{
		//printf("\nserver bind error!\n");
		close(servfd);
		return;	
	}
	
	if(listen(servfd,5)==-1)
	{
		//printf("\nserver listen error!\n");
		close(servfd);
		return;
	}

	while(1)
	{
		len=sizeof(serv_add);
		servfda=accept(servfd,(struct sockaddr *)&serv_add,&len);
		if(servfda==-1)
		{
			//printf("server accept failed!\n");
			close(servfd);
			return;	
		}
		
		FD_ZERO(&readset); //清空描述符集
		FD_SET(servfda,&readset); //把servfda放入测试描述符集
		ret=select(servfda+1,&readset,NULL,NULL,NULL);
		if(ret==-1)
		{
			close(servfda);
			close(servfd);
			return;	
		}
		
		if(FD_ISSET(servfda,&readset))
		{
			memset(buf,0,sizeof(buf));
			ret=recv(servfda,(PRecvComInfo)&recvinfo,sizeof(RecvComInfo),0);
			if(ret==-1)
			{
				close(servfda);
				continue;
			}

			switch(recvinfo.type)
			{
				case 0: //hungup
					eXosip_lock ();   
      				eXosip_call_terminate (call_id, dialog_id);   
      				eXosip_unlock ();

					if(g_stop_media == 0)
						g_stop_media = 1;

					lock(&mutex_send);
					lock(&mutex_recv);

					End_Rtp_Media();

					if(g_stop_media == 1)
						g_stop_media = 0;

					unlock(&mutex_recv);
					unlock(&mutex_send);

					hung_up(fd_com);

					kill_signal();					
					break;

				case 1: //call in					
					strncpy(g_callin_num, recvinfo.info, sizeof(g_callin_num));
					
					memset(source_call, '\0', sizeof(source_call));
					memset(dest_call, '\0', sizeof(dest_call));

					sprintf(source_call, "sip:%s@%s", g_callin_num, lan_addr);

					if(Rtp_Info.addr[0] != '\0')
					{
						if(Regist_Info.port[0] != '\0')
							sprintf(dest_call, "sip:1000@%s:%s",Rtp_Info.addr,Regist_Info.port);
						else
							sprintf(dest_call, "sip:1000@%s:5060",Rtp_Info.addr);
					}						
					else if(Regist_Info.addr[0] != '\0')
					{
						if(Regist_Info.port[0] != '\0')
							sprintf(dest_call, "sip:1000@%s:%s", Regist_Info.addr,Regist_Info.port);
						else
							sprintf(dest_call, "sip:1000@%s:5060", Regist_Info.addr);
					}						
					else
						sprintf(dest_call, "sip:1000@192.168.0.2:5060");

					//printf("g_callin_num = %s, source_call = %s, dest_call = %s\n",g_callin_num,source_call,dest_call);
					
					i = eXosip_call_build_initial_invite (&invite, dest_call, source_call, NULL, "This is a call for a conversation");   
				    if (i != 0)   
				    {   
				      //printf ("Intial INVITE failed!\n");   
				      break;   
				    }   

					/*
			        snprintf(tmp, 4096,   
			            "v=0\r\n"   
			            "o=- 0 0 IN IP4 %s\r\n"
			            "s=session\r\n"
			            "t=0 0\r\n"
			            "c=IN IP4 %s\r\n"
			            "m=audio 20188 RTP/AVP 0\r\n"
			            "a=rtpmap:0 PCMU/8000\r\n"
			            "a=ptime:20\r\n"
			            "a=sendrecv\r\n",lan_addr,lan_addr);
			       */

					snprintf(tmp, 4096,   
			            "v=0\r\n"   
			            "o=- 0 0 IN IP4 %s\r\n"
			            "s=session\r\n"
			            "t=0 0\r\n"
			            "c=IN IP4 %s\r\n"
			            "m=audio 20188 RTP/AVP 0\r\n"
			            "a=rtpmap:0 PCMU/8000\r\n",lan_addr,lan_addr);
				  
			      osip_message_set_body (invite, tmp, strlen(tmp));   
			      osip_message_set_content_type (invite, "application/sdp");   
			         
			      eXosip_lock ();   
			      i = eXosip_call_send_initial_invite (invite);
			      eXosip_unlock ();
				  break;

				default:
					break;
			}			
			close(servfda);
		}			
	}
	close(servfd);
	return 0;
}

void put_data_task()
{
	char 	pBuf[256], plineBuf[256];
	short 	output;
	int  	len = 0, ret = 0, i = 0, k = 0;
	FILE 	*fp_linear=NULL;
	struct timeval      timeout;
	
	memset(pBuf, '\0', sizeof(pBuf));

	/*
	fp_linear = fopen("/var/recvlinear.pcm","w+");
	if(fp_linear==NULL)
	{
		printf("open recv pcm file failed..exit\n");
		return;
	}
	*/
	
	while(1)
	{
		//lock(&mutex_put);
    	//wait_cond(&cond_put,&mutex_put);	
		//unlock(&mutex_put);

		if(g_stop_media)
		{
			while(g_stop_media)
				sleep(1);
			sleep(1);

			continue;
		}		

		
		if(pRtpSession)
		{
			//cprintf("count = %d\n",pcmobj.count);
			//if(pcmobj.count < 640)
			if(pcmobj.count <= 80)
			{
				//cprintf("\n555, count = %d\n", pcmobj.count);
				pcmobj.canin = 0x01;
				timeout.tv_sec = 0;
				timeout.tv_usec = 10000;
				select(0, NULL, NULL, NULL, &timeout);
				continue;
			}				
			
	  		len = destlist_getdata((PDestObj)&pcmobj, pBuf+ret, 80) ;
	  		if(len < 80)
	  		{
	  			ret += len;
	    		continue;
	  		}
			ret = 0;

			for(i=0,k=0; i<80; i++)
			{
				output = ulaw2linear(pBuf[i]);
				//output = ULawDecode(pBuf[i]);
				plineBuf[k] = output & 0x00ff;
				plineBuf[k+1] = (output & 0xff00) >> 8;
				k += 2;
			}
			
			ioctl(pcm_fd, PCM_PUTDATA, plineBuf);
		}
		else
		{
			sleep(2);
			continue;
		}
	}
}

int Start_Rtp_Trans()
{
	local_socket_thread = osip_thread_create(20000, local_socket_server, NULL);	
	if (local_socket_thread == NULL)
	{
		//printf("error local_socket_thread");
		exit(1);
	}

	audio_recv_thread = osip_thread_create(20000, recv_audio_task, NULL);	
	if (audio_recv_thread == NULL)
	{
		//printf("error audio_send_thread");
		exit(1);
	}

	audio_send_thread = osip_thread_create(20000, send_audio_task, NULL);	
	if (audio_send_thread == NULL)
	{
		//printf("error audio_send_thread");
		exit(1);
	}
	

	put_audio_thread = osip_thread_create(20000, put_data_task, NULL);
	if (put_audio_thread == NULL)
	{
		//printf("error put_audio_thread");
		exit(1);
	}

	osip_thread_set_priority(audio_recv_thread, 99);
	
	return 0;
}

int Init_Rtp_Medial(char *ipStr,char *port)
{
	//start_pcm(pcm_fd);
	//printf("\nInit_Rtp_Medial ipStr = %s, port = %s\n",ipStr,port);
	pRtpSession = rtpInit(ipStr,atoi(port));  
    if(pRtpSession==NULL)  
    {  
        //printf("error rtpInit");  
        return 0;  
    }

	return 1;
}

int End_Rtp_Media()
{
	//stop_pcm(pcm_fd);
	if(pRtpSession)
	    rtpExit(pRtpSession);

	pRtpSession = NULL;
	return 1;
}

int mRegisterResponse()   
{   
    osip_body_t *body;   
    osip_message_t *answer = NULL;
	osip_via_t *via;
   
    //printf("EXOSIP_MESSAGE_NEW!\n");   
    if(MSG_IS_MESSAGE(je->request))      //???????????MESSAGE   
    {   
        osip_message_get_body(je->request, 0, &body);    
        //printf("I get the msg is: %s\n", body->body);   
   
        //????,????200 OK??   
        //printf("begin to send message 200\n");   
        eXosip_lock();   
        eXosip_message_build_answer(je->tid, 200,&answer);   
        eXosip_message_send_answer(je->tid, 200,answer);   
        eXosip_unlock();   
        //printf("send end.\n");   
    }   
    else if(MSG_IS_REGISTER(je->request))    // 判断是否是注册信息
    {   
        //printf("username = %s, password = %s, host = %s\n",je->request->req_uri->username, je->request->req_uri->password,je->request->req_uri->host);

		via = (osip_via_t *) osip_list_get(&je->request->vias, 0);
		strcpy(Regist_Info.addr, via->host);
		strcpy(Regist_Info.port, via->port);

		//printf("binld answer(200)...\n");   
        eXosip_message_build_answer(je->tid, 200, &answer);   
        //printf("send answer(200)...\n");   
        eXosip_message_send_answer(je->tid, 200, answer);   
    }   
    return 0;   
}

int mInviteResponse()   
{   
    osip_message_t *ack = NULL;   
    sdp_message_t  *remote_sdp = NULL;   
    osip_message_t *answer = NULL;   
    sdp_attribute_t*at;
	sdp_media_t    *media_sdp = NULL;
	FILE *fp_rtp=NULL;	
   
    int i;   
    char    tmp[4096];   
    int pos = 0;	
   
    //printf("recieve invite.\n");   

    /*printf("Received a INVITE msg from %s:%s, UserName is %s, password is %s\n",   
        je->request->req_uri->host,   
        je->request->req_uri->port,    
        je->request->req_uri->username,    
        je->request->req_uri->password);*/   
    call_id     = je->cid;   
    dialog_id   = je->did;

#if 1
	//得到消息体,认为该消息就是SDP格式.  
    remote_sdp  = eXosip_get_remote_sdp(je->did);

	media_sdp = eXosip_get_audio_media (remote_sdp);

	
	//memset(rtp_addr,'\0',sizeof(rtp_addr));
	//memset(rtp_port,'\0',sizeof(rtp_port));
	//strcpy(rtp_addr,remote_sdp->o_addr);
	//strcpy(rtp_port,media_sdp->m_port);
	memset(&Rtp_Info, '\0', sizeof(Rtp_Info));
	strcpy(Rtp_Info.addr, remote_sdp->o_addr);
	strcpy(Rtp_Info.port, media_sdp->m_port);
	strcpy(Rtp_Info.s_name, remote_sdp->s_name);

	//printf("\nsdp_message rtp.addr = %s, rtp.port = %s, rtp.sname = %s\n",Rtp_Info.addr,Rtp_Info.port,Rtp_Info.s_name);

	/*
	fp_rtp = fopen("/var/rtp","w+");
	if(fp_rtp==NULL)
	{
		printf("open pcm file failed..exit\n");
		return -1;
	}
	fwrite ( &Rtp_Info, sizeof(Rtp_Info), 1, fp_rtp);
	fclose(fp_rtp);	
   
    //显示出在sdp消息体中的 attribute 的内容,里面计划存放我们的信息
    printf("the INFO is :\n");   
    while( !osip_list_eol(&remote_sdp->a_attributes, pos) )//modified by xulan   
    {   
        at = (sdp_attribute_t *)osip_list_get(&remote_sdp->a_attributes, pos);   
        //这里解释了为什么在SDP消息体中属性a里面存放必须是两列 
        printf("%s : %s\n", at->a_att_field, at->a_att_value);   
        pos ++;   
    }
    */    
#endif	   

    eXosip_lock();
	//printf("Send 100......\n"); 
	eXosip_call_send_answer(je->tid, 100, NULL);
	eXosip_unlock();

	eXosip_lock();
    //printf("Send 180......\n");   
    eXosip_call_send_answer(je->tid, 180, NULL);
	eXosip_unlock();

	eXosip_lock();
    //printf("Bind answer(200)......\n");   
    i = eXosip_call_build_answer(je->tid, 200, &answer);
    if (i != 0)   
    {   
        //printf("This request msg is invalid!Cann't response!\n");   
        eXosip_call_send_answer(je->tid, 400, NULL);
    }   
    else   
    {   
    	/*
            snprintf(tmp, 4096,   
            "v=0\r\n"   
            "o=- 0 0 IN IP4 %s\r\n"
            "s=%s\r\n"
            "t=0 0\r\n"
            "c=IN IP4 %s\r\n"
            "m=audio 20188 RTP/AVP 0 100\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:100 telephone-event/8000\r\n"
            "a=ptime:20\r\n"
            "a=sendrecv\r\n",lan_addr,Rtp_Info.s_name,lan_addr);
        */

		snprintf(tmp, 4096,   
            "v=0\r\n"   
            "o=- 0 0 IN IP4 %s\r\n"
            "s=session\r\n"
            "t=0 0\r\n"
            "c=IN IP4 %s\r\n"
            "m=audio 20188 RTP/AVP 0\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=ptime:20\r\n"
            "a=sendrecv\r\n",lan_addr,lan_addr);

        osip_message_set_body(answer, tmp, strlen(tmp));   
        osip_message_set_content_type(answer, "application/sdp");   
                   
        eXosip_call_send_answer(je->tid, 200, answer);
        //printf("send 200 over!\n");   
    }   
    eXosip_unlock();
}

void exit_cur_thread()
{
	int i;
	eXosip_quit();

	/*
	if (audio_send_thread != NULL) {
		i = osip_thread_join((struct osip_thread *) audio_send_thread);
		if (i != 0) {
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: can't terminate thread!\n"));
		}
		osip_free((struct osip_thread *)audio_send_thread);
	}

	if (audio_recv_thread != NULL) {
		i = osip_thread_join((struct osip_thread *) audio_recv_thread);
		if (i != 0) {
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: can't terminate thread!\n"));
		}
		osip_free((struct osip_thread *)audio_recv_thread);
	}

	if (local_socket_thread != NULL) {
		i = osip_thread_join((struct osip_thread *) local_socket_thread);
		if (i != 0) {
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: can't terminate thread!\n"));
		}
		osip_free((struct osip_thread *)local_socket_thread);
	}

	if (put_audio_thread != NULL) {
		i = osip_thread_join((struct osip_thread *) put_audio_thread);
		if (i != 0) {
			OSIP_TRACE(osip_trace
					   (__FILE__, __LINE__, OSIP_ERROR, NULL,
						"eXosip: can't terminate thread!\n"));
		}
		osip_free((struct osip_thread *)put_audio_thread);
	}
	*/
}

int
main (int argc, char *argv[])
{  
  osip_message_t *ack = NULL;
  osip_message_t *invite = NULL;
  osip_message_t *answer = NULL;
  sdp_message_t *remote_sdp = NULL;
  osip_message_t *reg=NULL;
  RtpSession * pRtpSession = NULL;
  int i,j;
  int id;
  char command[64];

  	if ( daemon(1,1) < 0 )
	{
	   perror("daemon()");
	   exit(-1);
	}


	signal(SIG_EXIT_OSIP_THREAD, exit_cur_thread);

  memset(g_callin_num, '\0', sizeof(g_callin_num));
  memset(&Regist_Info, '\0', sizeof(Regist_Info));

  sleep(2);
	
  //初始化sip
  	i = eXosip_init ();
  	if (i != 0)
    {
    	//printf ("Can't initialize eXosip!\n");
      	return -1;
    }

  	i = eXosip_listen_addr (IPPROTO_UDP, NULL, 5060, AF_INET, 0);
  	if (i != 0)
	{
		eXosip_quit ();
	  	fprintf (stderr, "eXosip_listen_addr error!\nCouldn't initialize transport layer!\n");
	}

	// ortp的一些基本初始化操作   
    ortp_init();  
    ortp_scheduler_init();	

	pcm_fd = open("/dev/pcm0", O_RDWR); 	
	if(pcm_fd < 0) 
	{
		//printf("open pcm driver failed (%d)...exit\n",pcm_fd);
		return -1;
	}

	if(fd_com<=0)
	{	
		#if defined (FOR_LC5740)
			//for LC5740
			system("comgt -d /dev/ttyUSB5 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
			fd_com = open("/dev/ttyUSB5", O_RDWR|O_NONBLOCK);
		#elif defined (FOR_AD3812) 
			//for AD3812
			system("comgt -d /dev/ttyUSB0 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
			fd_com = open("/dev/ttyUSB0", O_RDWR|O_NONBLOCK);
			write_str(fd_com, "AT+SPEAKER=2", strlen("AT+SPEAKER=2"));
		#elif defined (FOR_T3G5300) 
			//for T3G5300
			system("comgt -d /dev/ttyACM1 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
			fd_com = open("/dev/ttyACM1", O_RDWR|O_NONBLOCK);
		#endif
		
		fcntl(fd_com, F_SETFL, fcntl(fd_com, F_GETFL) & O_NONBLOCK);
	}

	memset(lan_addr, '\0', sizeof(lan_addr));
	getIpaddr(lan_addr);

	start_pcm(pcm_fd);
	
	Start_Rtp_Trans();	

  	for(;;)
	{
	  //侦听是否有消息到来
	  je = eXosip_event_wait (0,50);
	  //协议栈带有此语句,具体作用未知
	  eXosip_lock ();
	  eXosip_default_action (je);
	  eXosip_automatic_refresh ();
	  eXosip_unlock ();	  
	  
	  if (je == NULL)//没有接收到消息
		continue;

		switch (je->type)
		{
        case EXOSIP_MESSAGE_NEW:			
            mRegisterResponse();   
            break;   
        case EXOSIP_REGISTRATION_NEW:/** announce new registration.*/   
            eXosip_message_build_answer(je->tid, 200,&answer);   
            eXosip_call_send_answer(je->tid, 200, answer);   
            break;   
        case EXOSIP_REGISTRATION_SUCCESS://success to register   
            break;   
        case EXOSIP_REGISTRATION_FAILURE://failure to register   
            break;   
        case EXOSIP_CALL_INVITE:   
            mInviteResponse();   
            break;

		case EXOSIP_CALL_ANSWERED:   
          {
		  	//printf("EXOSIP_CALL_ANSWERED!\n\n");
          	sdp_message_t  *remote_sdp = NULL;
			sdp_media_t    *media_sdp = NULL;

			call_id     = je->cid;   
    		dialog_id   = je->did;

			remote_sdp  = eXosip_get_remote_sdp(je->did);
			media_sdp = eXosip_get_audio_media (remote_sdp);

			init_obj((PDestObj)&pcmobj);

			lock(&mutex_send);
			lock(&mutex_recv);
			Init_Rtp_Medial(remote_sdp->o_addr,media_sdp->m_port);
			unlock(&mutex_recv);
			unlock(&mutex_send);

			line_on(fd_com);			
          }
  
          eXosip_call_build_ack (je->did, &ack);   
          eXosip_call_send_ack (je->did, ack);   
          break;   	

		case EXOSIP_CALL_ACK:   
            //printf("ACK recieved!\n\n");		
			
			init_obj((PDestObj)&pcmobj);
						
			lock(&mutex_send);
			lock(&mutex_recv);
			Init_Rtp_Medial(Rtp_Info.addr,Rtp_Info.port);
			unlock(&mutex_recv);
			unlock(&mutex_send);
			//printf("4\n");
			dial_num(fd_com,je->request->req_uri->username);
            break;   

        case EXOSIP_CALL_CLOSED:   
            //printf("the remote hold the session!\n");   
            i = eXosip_call_build_answer(je->tid, 200, &answer);   
            if (i != 0)   
            {   
              //printf("This request msg is invalid!Cann't response!\n");   
              eXosip_call_send_answer(je->tid, 400, NULL);   
   
            }   
            else   
            {   
              eXosip_call_send_answer (je->tid, 200, answer);   
              //printf ("bye send 200 over!\n");   
            }

			if(g_stop_media == 0)
				g_stop_media = 1;

			//printf("5\n");
			lock(&mutex_send);
			lock(&mutex_recv);
			//printf("6\n");
			End_Rtp_Media();

			if(g_stop_media == 1)
				g_stop_media = 0;

			unlock(&mutex_recv);
			unlock(&mutex_send);
			
			hung_up(fd_com);
			//printf("3\n");

			kill_signal();
            break;   
   
        case EXOSIP_CALL_MESSAGE_NEW:
            //printf(" EXOSIP_CALL_MESSAGE_NEW\n\n");   
            if(MSG_IS_INFO(je->request))
            {   
                 eXosip_lock();   
                 i = eXosip_call_build_answer(je->tid, 200, &answer);   
                 if (i == 0)   
                 {   
                    eXosip_call_send_answer(je->tid, 200, answer);   
                 }   
                 eXosip_unlock();   
   
                 osip_body_t *body;   
                 osip_message_get_body (je->request, 0, &body);   
                 //printf ("the body is %s\n", body->body);   
            }   
            break;

		case EXOSIP_CALL_GLOBALFAILURE:
		case EXOSIP_CALL_REQUESTFAILURE:	
			//printf("EXOSIP_CALL_GLOBALFAILURE\n\n");
			lock(&mutex_send);
			lock(&mutex_recv);
			End_Rtp_Media();

			if(g_stop_media == 1)
				g_stop_media = 0;

			unlock(&mutex_recv);
			unlock(&mutex_send);
			
			hung_up(fd_com);

			kill_signal();
			break;

		case EXOSIP_CALL_PROCEEDING:
		case EXOSIP_CALL_RINGING:
			call_id     = je->cid;   
    		dialog_id   = je->did;
			break;
   
        default:   
            ;//printf("Could not parse the msg: %d!\n", je->type);
		}		
	}
	eXosip_event_free(je);
}
