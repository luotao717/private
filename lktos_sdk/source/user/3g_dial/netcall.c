#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<sys/stat.h>
#include 	<sys/types.h>
#include 	<sys/socket.h>
#include 	<netinet/in.h>
#include 	<fcntl.h>
#include 	<arpa/inet.h>
#include 	<dirent.h>
#include 	<unistd.h>
#include 	<string.h>
#include 	<errno.h>
#include 	<time.h>
#include 	<assert.h>
#include 	<netinet/tcp.h> 
#include 	<syslog.h>
#include	<sys/un.h>
#include 	<signal.h>
#include 	"3gdial.h"
#include 	"cmd.h"

#define SIG_CALL_MSG		(SIGUSR1+58)

int callin=0;

typedef struct tagSendComInfo
{
	int 			type; 		//0 : hung up  1 : call coming
	unsigned char 	info[128];
}SendComInfo,*PSendComInfo;


int read_com_str(int fd, char *str, int len, char *pbuf)
{
	int 			i, readlen=0, cops=0, ret=0, err=0;
	unsigned char 	wt_str[64],buf[2048];
	fd_set         inset;
  	struct timeval timeout;

  	FD_ZERO(&inset);
	FD_SET(fd, &inset);

	memset(buf, '\0', sizeof(buf));

	/*
	for( i = 0; i < len; i ++)
	{
		write ( fd, str+i, 1);
		usleep(10000);
	}
	
	//write(fd,str,strlen(str));
	write( fd, "\r", 1);
	write(fd, "\n", 1);
	*/

	while(1)
	{
		err=select(fd+1,&inset,NULL,NULL,NULL);
		if(err<=0)
		{
			printf("\nfailed\n");
			return -1;
		}

		if( (readlen = read(fd, buf, sizeof(buf))) > 0)
			printf("\nin netcall.c return from cdma: %s\n", buf);
		else
			printf("\nin netcall.c read com wrong!\n");
	}	

	return readlen;
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


void set_com_attr(int fd)
{
	//write_str(fd, "AT^DSCI=1",strlen("AT^DSCI=1"));
	
	//write_str(fd, "AT^DCPI=1",strlen("AT^DCPI=1"));

	write_str(fd, "AT+CLIP=1",strlen("AT+CLIP=1"));

	write_str(fd, "AT^DSCI=1",strlen("AT^DSCI=1"));
}

/*
		-1 : no call in
return	 0 : have call in
		 1 : hung up
*/
int get_callin_number(char *buf, char *num)
{
	int	 i=0;
	char *pstr=NULL;

	if(pstr=strstr(buf, "+CLIP: "))
	{
		pstr += 7;
		if(*pstr=='"')
		{
			pstr++;
			for(i=0; *pstr!='"'; i++,pstr++)
				num[i] = *pstr;

			return 0;
		}
		else
			return -1;
	}
	else
		return -1;
}

int send_com_info(PSendComInfo info)
{
	static int count=0;
	int clientfd;
	struct sockaddr_un client_add;
	const char path[]="/var/.tmpsocket";
	int ret;
	char buf[32];

	memset(&client_add,0,sizeof(client_add));
	client_add.sun_family=AF_LOCAL;
	strncpy(client_add.sun_path,path,strlen(path));

	clientfd=socket(AF_LOCAL,SOCK_STREAM,0);
	if(clientfd==-1)
	{
		printf("cgiGetWanNetInfo socket failed!\n");
		return;
	}

	if(connect(clientfd,(struct sockaddr *)&client_add,sizeof(client_add))==-1)
	{
		close(clientfd);
		return;
	}

	ret=send(clientfd,(PSendComInfo)info,sizeof(SendComInfo),0);
	if(ret==-1)
	{
		close(clientfd);
		return;
	}

	close(clientfd);
}

void Call_In_Status()
{
	if (0 == access("/var/.call_end", F_OK))//说明该文件存在
	{
		callin = 0;
		unlink("/var/.call_end");
	}	

	return;
}

int main(int argc, char *argv[])
{
  	int			          n,i,on,idx,fd_com=-1,fd_sem;
	int			          ret,err;
	int			          sin_len;
  	int			          pin_len;
  	int			          skfd;
  	fd_set			      fdset,acpfdset,comset;
	struct timeval		  timeout;
	struct sockaddr_in 	  sin;
	struct sockaddr_in 	  pin;
	MySockInfo		      skinfo;
  	char              	  path[100],buf[2048], *pcmd=NULL, callnum[16];
	FILE                  * f;
	struct linger         m_sLinger;
	SendComInfo 		  cominfo;

	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

	if(fd_com<=0)
	{
		#if defined (FOR_LC5740)
			system("comgt -d /dev/ttyUSB5 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
		
			fd_com = open("/dev/ttyUSB5", O_RDWR|O_NONBLOCK);
		#elif defined (FOR_AD3812) 
			system("comgt -d /dev/ttyUSB0 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
		
			fd_com = open("/dev/ttyUSB0", O_RDWR|O_NONBLOCK);
		#elif defined (FOR_T3G5300) 
			//for T3G5300
			system("comgt -d /dev/ttyACM1 -s /etc_ro/ppp/3g/Generic_conn.scr");
			sleep(1);
			
			fd_com = open("/dev/ttyACM1", O_RDWR|O_NONBLOCK);	
		#endif
		//fcntl(fd_com, F_SETFL, fcntl(fd_com, F_GETFL) & O_NONBLOCK);
	}

	//signal(SIG_CALL_MSG, Call_In_Status);

	set_com_attr(fd_com);

	FD_ZERO(&comset);
	FD_SET(fd_com, &comset);
	while(1)
	{
		err=select(fd_com+1,&comset,NULL,NULL,NULL);
		if(err<=0)
		{
			printf("\nfailed, err=%d\n",err);
			return -1;
		}

		memset(buf, '\0', sizeof(buf));
		memset(callnum, '\0', sizeof(callnum));
		
		if( read(fd_com, buf, sizeof(buf)) > 0)
		{
			//printf("%s\n",buf);
			
			if(get_callin_number(buf, callnum)==0)
			{
				Call_In_Status();
				if(callin==0)
				{					
					callin = 1;
					cominfo.type = 1; //call coming 
					strcpy(cominfo.info,callnum);
					send_com_info(&cominfo);
				}				
			}

			if(strstr(buf, "NO CARRIER")||strstr(buf, "^DSCI: 1,1,6,")||strstr(buf, "+ZCEND:"))
			{
				if(callin==1)
					callin = 0;

				cominfo.type = 0; //hung up
				send_com_info(&cominfo);
			}			
		}		
	}		
}

