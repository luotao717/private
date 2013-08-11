#include  	<stdio.h>
#include  	<stdlib.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include 	<sys/stat.h>
#include	<string.h>
#include 	<fcntl.h>
#include  	<sys/socket.h>
#include  	<sys/ioctl.h>
#include  	<dirent.h>
#include  	<linux/if.h>
#include  	<arpa/inet.h>
#include  	<netinet/in.h>
#include	<errno.h>
#include 	<signal.h>
#include 	<sys/wait.h>
#include 	<syslog.h>
#include	<sys/ipc.h>
#include	<sys/sem.h>

#include	"nvram.h"
#include	"3gdial.h"
//#include	"cmd.h"

#define SIG_SERACH_MSG		(SIGUSR1+57)
#define SIG_DISCON_PPP		(SIGUSR1+59)
#define OPERA_NAME			"/var/usbmodem/operaname"
#define PERMIT_NUM			5

#define SEM_PATH 			"/var/my_sem"
static int					pcm_semid;
static unsigned char 		rddevbuf[2048*10*2];
static unsigned int			serach_net_flag = 0;
static unsigned int			g_orgin_flag = 0, g_now_flag = 0;  // 0 正确 1 错误
static int					fd_com=0;

static int init_sem()
{
	int key;
	key=ftok(SEM_PATH,'a');

	pcm_semid = semget(key, 1, IPC_CREAT|IPC_EXCL|0666);
	if(pcm_semid == -1)
	{
	  	printf("init semget failed,errno=%d\n",errno);
	  	syslog(LOG_DEBUG,"init semget failed,errno=%d\n",errno);
	  	return -1;
    }
    
    if(semctl(pcm_semid,0,SETVAL,0)==-1)
	{
		printf("semctl setval error");
		return -1;
	}		

    return pcm_semid;
}

static int lock_sem()
{
	struct sembuf 	askfor_res;
	
	askfor_res.sem_num=  0;
	askfor_res.sem_op =  -1;//0;//1;//-1;
	askfor_res.sem_flg=  SEM_UNDO;		
		
	if(semop(pcm_semid,&askfor_res,1)==-1)//ask for resource
	{
		printf("down semop error");
		return -1;
	}	
	return 1;	
}

static int unlock_sem()
{
	struct sembuf 	free_res;
	
	free_res.sem_num=	0;
	free_res.sem_op =	1;
	free_res.sem_flg=	SEM_UNDO;
	
	if(semop(pcm_semid,&free_res,1)==-1)//free the resource.
	{	
		if(errno==EIDRM)
			printf("the semaphore set was removed\n");
			
		return -1;	
	}		
	return 1;
}

static int remove_sem()
{
	if(pcm_semid!=-1)
		if(semctl(pcm_semid, 0, IPC_RMID, 0)==-1)
			return -1;
			
	return 1;		
}

static void Disconnect_PPP(int signum)
{
	int pid_pppd=0;
	
	if(signum != SIG_DISCON_PPP)
		return;

	pid_pppd = DBgetPid("pppd");
	if(pid_pppd>0)
		system("killall -15 pppd");	

	//system("comgt -d /var/ttyACM0 -s /etc_ro/ppp/3g/Generic_disconn.scr");
}

static void Serach_Curr_Net(int signum)
{
	int pid_pppd=0;
	
	if(signum != SIG_SERACH_MSG)
		return;

	system("killall -9 3gdial");

	while(1)
	{
		pid_pppd = DBgetPid("pppd");
		if(pid_pppd>0)
		{
			//gpio_set_led(13, 0, 4000, 0, 1, 4000);
			system("killall -15 pppd");
			sleep(2);
			continue;
		}			
		system("comgt -d /var/ttyACM0 -s /etc_ro/ppp/3g/Generic_disconn.scr");
		sleep(2);
		break;
	}	
	serach_net_flag = 1;
}

#if 0
int write_str(int fd, char *str, int len)
{
	int i, readlen;
    char buf[128];
	unsigned char wt_str[64];

	for( i = 0; i < len; i ++)
	{
		write ( fd, str+i, 1);
		usleep(10000);
	}
	
	//write(fd,str,strlen(str));
	write( fd, "\r", 1);
	write(fd, "\n", 1);

	if(!strcmp(str,"AT+COPS=?"))
	{
		sleep(60);
		memset(rddevbuf, 0, sizeof(rddevbuf));
		if( (readlen = read(fd, rddevbuf, 1024)) > 0)
		{
			if(strstr(rddevbuf,"OK")==NULL)
			{
				while(1)
				{
					write ( fd, 'A', 1);
					usleep(10000);
					write ( fd, 'T', 1);
					usleep(10000);
					write( fd, "\r", 1);
					write(fd, "\n", 1);
					memset(rddevbuf, 0, sizeof(rddevbuf));
					if( (readlen = read(fd, rddevbuf, 1024)) > 0)
					{
						cprintf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
						syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
						goto done;
					}
					else
						sleep(2);
				}
			}
			else
			{
				cprintf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
				syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
				goto done;
			}
		}
		else
		{
			while(1)
			{
				write ( fd, 'A', 1);
				usleep(10000);
				write ( fd, 'T', 1);
				usleep(10000);
				write( fd, "\r", 1);
				write(fd, "\n", 1);
				memset(rddevbuf, 0, sizeof(rddevbuf));
				if( (readlen = read(fd, rddevbuf, 1024)) > 0)
				{
					cprintf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
					syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
					goto done;
				}
				else
					sleep(2);
			}
		}
	}
	else
		usleep(500000);

	memset(rddevbuf, 0, sizeof(rddevbuf));
	if( (readlen = read(fd, rddevbuf, 1024)) > 0)
	{
		cprintf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
	}

	usleep(100000);

done:
	syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
	return readlen;
}
#else
int write_str(int fd, char *str, int len)
{
	int 			i, readlen=0, cops=0, ret=0, err=0;
    char 			buf[128];
	unsigned char 	wt_str[64];
	fd_set         inset;
  	struct timeval timeout;

  	timeout.tv_sec  = 70;
  	timeout.tv_usec = 0;

  	FD_ZERO(&inset);
	FD_SET(fd, &inset);

	memset(rddevbuf, '\0', sizeof(rddevbuf));

	for( i = 0; i < len; i ++)
	{
		write ( fd, str+i, 1);
		usleep(10000);
	}
	
	//write(fd,str,strlen(str));
	write( fd, "\r", 1);
	write(fd, "\n", 1);

	while(1)
	{
		err=select(fd+1,&inset,NULL,NULL,&timeout);
		if(err<=0)
		{
			printf("select failed\n");
			break;
		}

		if( (readlen = read(fd, rddevbuf+cops, 1024)) > 0)
		{
			cops += readlen;
			if(strstr(rddevbuf,"OK"))
			{
				printf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
				break;
			}
			else
				continue;
		}
	}	

	//syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
	return readlen;
}
#endif


int read_unread_sms(int fd, char *str, int len)
{
	int 			i, readlen=0, cops=0, ret=0, err=0;
    char 			buf[2048*10*2];
	fd_set         inset;
  	struct timeval timeout;

  	timeout.tv_sec  = 70;
  	timeout.tv_usec = 0;

  	FD_ZERO(&inset);
	FD_SET(fd, &inset);

	memset(buf, '\0', sizeof(buf));

	for( i = 0; i < len; i ++)
	{
		write ( fd, str+i, 1);
		usleep(10000);
	}
	
	write(fd, "\r", 1);
	write(fd, "\n", 1);

	while(1)
	{
		err=select(fd+1,&inset,NULL,NULL,&timeout);
		if(err<=0)
		{
			printf("select failed\n");
			break;
		}

		if( (readlen = read(fd, buf+cops, 1024)) > 0)
		{
			cops += readlen;
			if(strstr(buf,"OK"))
			{
				printf("cmd: %s,return from cdma: %s\n", str,buf);
				break;
			}
			else
				continue;
		}
	}	

	if(strstr(buf,"DMGL:")) // have new sms 
		return 1;
	else 
		return 0;
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

/*
+COPS: (1,"CHINA MOBILE","","46000",0),(1,"CHINA MOBILE","","46000",2),(1,"","","46002",0),(3,"","","46004",2),(1,"","","46006",0),(3,"CHN-CUGSM","","46001",0),(2,"","","46007",2),(1,"","","46003",0),(1,"","","46020",0)

OK
*/
void Analyse_Str(unsigned char *str)
{
	int 			i=0, j=0;
	FILE 			*fp;
	unsigned char 	*ptr=NULL,*tmp=NULL,oper_str[10][64];
	unsigned char	net_sta[10][2];		// 0 : 未知 1 :可用的  2: 当前注册的 3: 禁止的
	unsigned char	oper_lname[10][16];	//运营商长名称
	unsigned char	oper_sname[10][16]; //运营商短名称
	unsigned char	oper_code[10][8];	//运营商代码
	unsigned char	net_act[10][2]; 	// 0 GSM/GPRS 制式 1 TD制式
	unsigned char	result[512],cmd[64];

	if(ptr=strstr(str, "+COPS:"))
	{
		ptr += 6;
		while(ptr!='\r'&&ptr!='\n')
		{
			ptr += 2;
			if(tmp = strchr(ptr,')'))
			{
				if(i>=10)
					break;
				
				memset(oper_str[i], '\0', 64);
				strncpy(oper_str[i++], ptr, tmp-ptr);
				ptr = tmp;
				ptr += 1;
			}
			else
				break;
		}

		fp = fopen(OPERA_NAME, "w");
		if(fp == NULL)
			return ;
		
		for(j=0; j<i; j++)
		{
			///1,"CHINA MOBILE","","46000",0
			memset(net_sta[j], '\0', 2);
			memset(oper_lname[j], '\0', 16);
			memset(oper_sname[j], '\0', 16);
			memset(oper_code[j], '\0', 8);
			memset(net_act[j], '\0', 2);
			memset(cmd, '\0', 64);

			///1)
			ptr = oper_str[j];
			strncpy(net_sta[j], ptr, 1);

			///2)
			ptr += 3;
			if(*ptr != '"')
			{
				if(tmp=strchr(ptr,'"'))
				{
					if((tmp-ptr)>1)
						strncpy(oper_lname[j], ptr, tmp-ptr);

					ptr = tmp;
				}
				else
					continue;
			}				

			///3)
			ptr += 3;
			if(*ptr != '"')
			{
				if(tmp=strchr(ptr,'"'))
				{
					if((tmp-ptr)>1)
						strncpy(oper_sname[j], ptr, tmp-ptr);

					ptr = tmp;
				}
				else
					continue;
			}

			///4)
			ptr += 3;
			if(*ptr != '"')
			{
				if(tmp=strchr(ptr,'"'))
				{
					if((tmp-ptr)>1)
						strncpy(oper_code[j], ptr, tmp-ptr);

					ptr = tmp;
				}
				else
					continue;
			}
			
			///5)
			ptr += 2;
			net_act[j][0] = *ptr;

			fprintf(fp,"%s,%s,%s,%s\n",net_sta[j],oper_lname[j],oper_code[j],net_act[j]);
		}
		fclose(fp);
		
	}
	else
		return;
}

void CheckModuleCsqNet(int type)
{
	int 		fd_mod=0, i=0, csq=0;
	char 		*tmptr=NULL,*ptr=NULL,cmd[32];
	static int	no_server_count=0, no_signal_count=0, sysmode, srv_status=0, sms_coming=0;

	if(fd_com > 0)
	{
		if(no_server_count==PERMIT_NUM)
		{
			g_now_flag = 1;
			//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000); // low
			//gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
			//gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000);	//网络无服务 快闪
			g_orgin_flag = g_now_flag;

			system("killall -15 pppd");
		}

		if(g_orgin_flag)//原来网络错误
		{
			if(!g_now_flag) //现在网络恢复
				system("killall -15 pppd");

			g_orgin_flag = g_now_flag;
		}
		
		if(serach_net_flag)
		{
			serach_net_flag = 0;
			write_str(fd_com, "AT+COPS=?", strlen("AT+COPS=?"));
			Analyse_Str(rddevbuf);
			write_str(fd_com, "AT", strlen("AT"));
		}

		write_str(fd_com, "ATE0", strlen("ATE0"));

#if 1
#ifndef NOCURNET
		if(type)
		{
			write_str(fd_com, "AT^SYSINFO", strlen("AT^SYSINFO"));
			if( (ptr = strstr(rddevbuf, "^SYSINFO:")) == NULL) //^SYSINFO: 2,3,0,15,1,0,5  ^SYSINFO:2,255,0,8,1
			{
				File_Set_Modem_Info("dial3gnetmode", "");			
				return;
			}
			else
			{
				if(ptr = strchr(ptr,','))
				{
					tmptr = ptr;
					srv_status = atoi(tmptr-1);
					if(ptr = strchr(ptr+1,','))
					{
						if(ptr = strchr(ptr+1,','))
							sysmode = strtoul( ptr+1, NULL, 10);
					}
					else
					{
						srv_status = -1;
						sysmode = 1;
					}
				}
				else
				{
					srv_status = -1;
					sysmode = 1;
				}
			}		

			if(srv_status==0)
				no_server_count++;
			else
			{
				no_server_count = 0;
				g_now_flag = 1;
			}			

			switch(sysmode)
			{
				case 0:
					File_Set_Modem_Info("dial3gnetmode", "NO Server");
					break;
				case 1:
					File_Set_Modem_Info("dial3gnetmode", "AMPS");
					break;
				case 2:
					File_Set_Modem_Info("dial3gnetmode", "CDMA");
					display_net_led(2);
					break;	
				case 3:
					File_Set_Modem_Info("dial3gnetmode", "GSM/GPRS");
					display_net_led(2);
					break;
				case 4:
					File_Set_Modem_Info("dial3gnetmode", "EVDO");
					display_net_led(3);
					break;
				case 5:
					File_Set_Modem_Info("dial3gnetmode", "WCDMA");
					display_net_led(3);
					break;
				case 6:
					File_Set_Modem_Info("dial3gnetmode", "GPS");
					display_net_led(2);
					break;
				case 7:
					File_Set_Modem_Info("dial3gnetmode", "GSM/WCDMA");
					break;
				case 8:
					File_Set_Modem_Info("dial3gnetmode", "CDMA/EVDO HYBRID");
					break;
				case 15:
					File_Set_Modem_Info("dial3gnetmode", "TD-SCDMA");
					display_net_led(3);
					break;
				default:
					File_Set_Modem_Info("dial3gnetmode", "");
					break;
			}
		}	
		else
		{
			File_Set_Modem_Info("dial3gnetmode", "WCDMA");
		}
#endif
#endif		


		// for LC5740 or LC5730+
		#ifdef TESTMOBILE
			if(read_unread_sms(fd_com, "AT^DMGL=0", strlen("AT^DMGL=0")))
				sms_coming = 1;
			else
				sms_coming = 0;
		#endif

#if 1
		//Check CSQ
		for(i=0; i < 5; i++)
		{
			write_str(fd_com, "AT+CSQ", strlen("AT+CSQ"));

			if( ((ptr = strstr(rddevbuf, "+CSQ:")) == NULL))
			{
				return;
			}	
			ptr += 5;
			while( *ptr == ' ')
				ptr++;

			csq = strtoul( ptr, NULL, 10);
			if((99 == csq)||(0 == csq)||(199 == csq)||(100 == csq))
			{
				
		       if(i < 5)
			   {
		       		usleep(20000);
					continue;
		       }
				   
			   csq = 0;
			   display_signal(sms_coming, 0);			 
			   File_Set_Modem_Info("dial3gsignal", "0");
			   return;
			}
			else
				break;
		}

		display_signal(sms_coming, csq);
		sprintf(cmd,"%d\0",csq);
		File_Set_Modem_Info("dial3gsignal", cmd);
#endif
		return;		
	}	
}

int main(int argc, char **argv)
{
	int 		fd_mod, ret, go_pid=0, modem_style=0;
	int 		option_flag=48;//'0':not option modem
	int 		stsOld = IFC_STATUS_DOWN, stsNew = IFC_STATUS_DOWN;
	char		*choicetype =NULL, *ptmp=NULL, cmd[64];
	static int 	comgt_flag=0;

	if ( daemon(1,1) < 0 )
	{
    	perror("daemon()");
    	exit(-1);
	}

	modem_style = atoi(argv[1]); //0 : no check net  	1 : check net 

	g_orgin_flag = g_now_flag;

	signal(SIG_SERACH_MSG, Serach_Curr_Net);
	signal(SIG_DISCON_PPP, Disconnect_PPP);

	if(access("/var/ttyAT",F_OK)==-1)//ttyAT is not exist		
		sleep(3);
	else
	{
		system("comgt -d /var/ttyAT -s /etc_ro/ppp/3g/Generic_conn.scr");
		sleep(1);
	}

	if(fd_com<=0)
	{
		fd_com = open("/var/ttyAT", O_RDWR|O_NONBLOCK);
		fcntl(fd_com, F_SETFL, fcntl(fd_com, F_GETFL) & O_NONBLOCK);
	}

	/*SMS INIT*/
	write_str(fd_com, "AT+CMGF=0", strlen("AT+CMGF=0")); // PDU FORMAT
	write_str(fd_com, "AT+CSCS=\"UCS2\"", strlen("AT+CSCS=\"UCS2\"")); // char set is UCS2
	
	while(1)
	{	
		if(modem_style)
		{
			CheckModuleCsqNet(modem_style);
		}
		else
		{
			display_signal(0, 30);
		}
		sleep(3);
	}

	if(fd_com)
		close(fd_com);
	
	return 0;
}
