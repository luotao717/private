
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
//#include <libtd.h>
#include "blowfish.h"
#include "md5.h"
#include "hmac-md5.h"
#include "base64.h"

#define DEBUG_UPDATELOG
#define userlog printf 
extern int SYS_wan_ip;

#define ORAY_HOST "ph002.oray.net"
#define ORAY_PORT 5050
#define ORAY_PORT2 6010

#define LOGIN_FAIL(err) fclose(sock_file); \
			close(host_sock); \
			return err;

char *md5_crypt(const char *pw,int pw_len,const char *salt,int salt_len);
static unsigned char server_key[100];
static unsigned long session_id;
static unsigned long session_index;
#define MAX_FAILDCOUNT	4
int faild_count=MAX_FAILDCOUNT;
//CBlowfish myblowfish;
static int oray_port;
struct in_addr oray_server_addr;

int oray_keepalive(const char *domain,int flag);

int oray_ddns_login(const char *userid,const char*pwd,const char *ip,const char *domain)
{
	char buf[256],buf2[256];
	unsigned char *temp;
	int ret,len,i,retry_cnt;
	int host_sock;
	struct hostent *site;
	struct sockaddr_in me;
	FILE *sock_file;
	fd_set fds;
	struct timeval to;

	CBlowfish_Constructor();
	oray_port=ORAY_PORT;
	site=gethostbyname(ORAY_HOST);
	if(site==NULL)
		return -1;
	host_sock=socket(AF_INET,SOCK_STREAM,0);
	if(host_sock<0)
		return -2;
retry:
        memset(&me,0,sizeof(struct sockaddr_in));
        memcpy(&me.sin_addr,site->h_addr_list[0],site->h_length);
        oray_server_addr.s_addr = me.sin_addr.s_addr;
        me.sin_family=AF_INET;
	me.sin_port=htons(oray_port);
	if(connect(host_sock,(struct sockaddr*)&me,sizeof(struct sockaddr))!=0){
                close(host_sock);
		if(oray_port==ORAY_PORT2)
                	return -1;
		else{
			oray_port=ORAY_PORT2;
			goto retry;
		}
        }	
	retry_cnt = 0;
	to.tv_sec=5;
	to.tv_usec=0;
retry1:
	FD_ZERO(&fds);
	FD_SET(host_sock,&fds);
	i=select(host_sock+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
	if ((i!=1)||(!FD_ISSET(host_sock,&fds))) 
	{
		if (i<0) 
		{
			close(host_sock);
			return -3;
		}
		else
		{
			if(retry_cnt>3)
			{
				close(host_sock);
				return -3;	
			}
			else
			{	
				retry_cnt++;
				goto retry1;
			}	
		}	
	}

	ret=recv(host_sock,buf,sizeof(buf)-1,0);
	buf[ret]='\0';
	if(atoi((char*)buf)!=220){
		close(host_sock);
		if(oray_port==ORAY_PORT2)
                	return -3;//Server msg error
		else{
			oray_port=ORAY_PORT2;
			goto retry;
		}
	}
	strcpy(buf,"AUTH CRAM-MD5\r\n");
	ret=send(host_sock,buf,strlen(buf),0);
	retry_cnt = 0;
	to.tv_sec=5;
	to.tv_usec=0;
retry2:
	FD_ZERO(&fds);
	FD_SET(host_sock,&fds);
	i=select(host_sock+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
	if ((i!=1)||(!FD_ISSET(host_sock,&fds)))
	{
		if (i<0) 
		{
			close(host_sock);
			return -3;
		}
		else
		{
			if(retry_cnt>3)
			{
				close(host_sock);
				return -3;	
			}
			else
			{	
				retry_cnt++;
				goto retry2;
			}	
		}	
	}

	ret=recv(host_sock,buf,sizeof(buf)-1,0);
	if(atoi(buf)!=334){
		close(host_sock);
		return -3;
	}
	strcpy((char*)server_key,buf+4);
	ret=base64_decode((char*)server_key,buf,sizeof(buf));
	memcpy(server_key,buf,ret);
	hmac_md5((unsigned char*)pwd,strlen(pwd),(unsigned char*)buf,16,(unsigned char*)buf2);
	memset(buf,0,sizeof buf);
	strcpy(buf,userid);
	buf[strlen(userid)]=' ';
	len=strlen(buf);
	memcpy(buf+len,buf2,16);
	ret=base64_encode(buf,16+len,buf2,256);
	for(i=0;i<ret;i++)
		sprintf(buf+i,"%c",buf2[i]);
	strcat(buf,"\r\n");
	ret=send(host_sock,buf,strlen(buf),0);
	sock_file=fdopen(host_sock,"r+");
	fgets(buf,sizeof buf,sock_file);
	if(atoi(buf)!=250){
		LOGIN_FAIL(-4);//Login fail
	}
	memset(buf2,0,sizeof(buf2));
	while(fgets(buf,sizeof buf,sock_file) && buf[0]!='.'){
		//if(domain && strcmp(buf,domain)==0)
		if(domain && memcmp(buf,domain,strlen(domain))==0)
		{
			if((strlen(domain)+2) == strlen(buf))
				sprintf(buf2,"REGI A %s",buf);
		}	
		else if(!domain)
			sprintf(buf2+strlen(buf2),"REGI A %s",buf);
	}
	if(strlen(buf2)<5){
		LOGIN_FAIL(-5);//no set domain
	}
	strcat(buf2,"CNFM\r\n");
	ret=send(host_sock,buf2,strlen(buf2),0);
	fgets(buf,sizeof buf,sock_file);
	if(atoi(buf)!=250){
		LOGIN_FAIL(-5);
	}
	CBlowfish_SetKey(server_key,16);
	fgets(buf,sizeof buf,sock_file);
	session_id=atoi(buf+4);
	session_index=atoi(strchr(buf+4,' '));
	strcpy(buf,"QUIT\r\n");
	ret=send(host_sock,buf,strlen(buf),0);
	fgets(buf,sizeof buf,sock_file);
	fclose(sock_file);
	close(host_sock);
	faild_count=0;
	return 0;
}
//flag:0-->no cnt, flag:1-->cnt
int oray_keepalive(const char *domain,int flag)
{
	struct sockaddr_in me,from;
	long packet[4];
	long packet_d[5];
	long packet_r[5];
	int host_sock;
	int ret,len;
	struct timeval timeout;

	host_sock=socket(AF_INET,SOCK_DGRAM,0);
	if(host_sock<0)
		return -2;
	packet[0]=10;
	packet[1]=session_index++;
	packet[2]=0-packet[0]-packet[1];
	packet[3]=0;
	ret=CBlowfish_EnCode((char*)(packet),(char*)(packet_d+1),4*sizeof(long));
	if(ret!=0)
	{
		close(host_sock);
		return -1;
	}	
	packet_d[0]=session_id;
	memset(&me,0,sizeof(struct sockaddr_in));
        me.sin_addr.s_addr = oray_server_addr.s_addr;
        me.sin_family=AF_INET;
	me.sin_port=htons(oray_port);
	sendto(host_sock,packet_d,sizeof(packet_d),0,(struct sockaddr*)&me,sizeof(struct sockaddr));
	timeout.tv_sec=5;
	timeout.tv_usec=0;
	setsockopt(host_sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	//ret=recvfrom(host_sock,packet_r,sizeof(packet_r),0,NULL,NULL);
	ret=recvfrom(host_sock,packet_r,sizeof(packet_r),0,(struct sockaddr*)&from,(socklen_t*)&len);
	//yfchou mark 4/14/2004
	//if(ret==20){
	if(ret>=0){
		ret=CBlowfish_DeCode((char*)(&packet_r[1]),(char*)(&packet_d[1]),16);
		if((ret==0) && (packet_r[0]==session_id) && (packet_d[1]==50) && (abs(packet_d[2]-session_index+1) < 6)){
//		if((ret==0) && (packet_r[0]==session_id) && (packet_d[1]==50) ){
			if(flag)
				faild_count=0;
			session_index=session_index<packet_d[2]+1?packet_d[2]+1:session_index;
#ifdef DEBUG_UPDATELOG
			//userlog("DDNS:update ok\n");
#endif
		}else{
#ifdef DEBUG_UPDATELOG
			char buf[100];
			sprintf(buf,"packet_r[0]:%x;session_id:%x;packet_d[2]:%d;session_index:%d;packet_d[1]:%d;ret:%d",
			packet_r[0],session_id,packet_d[2],session_index,packet_d[1],ret);
			userlog(buf,buf);
#endif
			if(flag)
				faild_count=MAX_FAILDCOUNT;
		}
		ret=0;
	}else{
		if(flag)
			faild_count++;
		ret=-3;
#ifdef DEBUG_UPDATELOG
		//userlog("DDNS:update timeout\n");
#endif
	}
	close(host_sock);
	return ret;
}
int ddns_oray(const char *userid,const char*pwd,const char *ip,const char *domain)
{
	int ret=0;

	if(faild_count>=MAX_FAILDCOUNT)
		{
		ret=oray_ddns_login(userid,pwd,NULL,domain);
		if(ret==0)
			{
			userlog("DDNS:ORAY Register Ok\n");
			}
		else if(ret==-1)
			userlog("DDNS:ORAY Connnect server error\n");
		else if(ret==-4)
			userlog("DDNS:ORAY Login Fail\n");
		else if(ret==-5)
			userlog("DDNS:ORAY CNFM Fail\n");
		else if(ret==-3)
			userlog("DDNS:ORAY Server Error\n");
		else
			userlog("DDNS:ORAY Register Fail\n");
		}
	else
		{
		ret=oray_keepalive(domain,1);
		if(ret!=0)
			{
#if 0 //added by maocw 2006-4-4			
			cyg_thread_delay(200);
#else
			sleep(10);
#endif
			oray_keepalive(domain,0);
			}
		return 1; //yfchou add another state
		}
	return ret;
}
void oray_count_reset(void)
{
	faild_count=MAX_FAILDCOUNT;
}	
