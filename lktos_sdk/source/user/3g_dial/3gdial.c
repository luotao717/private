#include 	<stdio.h>
#include  <stdlib.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include 	<sys/stat.h>
#include	<string.h>
#include  <sys/socket.h>
#include	<ctype.h>

#include  <arpa/inet.h>
#include  <netinet/in.h>
#include	<errno.h>
#include 	<signal.h>
#include 	<sys/wait.h>
#include 	<syslog.h>

#include	"nvram.h"
#include	"3gdial.h"



#define SIG_CON_MSG			(SIGUSR1+55)
#define SIG_DISCON_MSG		(SIGUSR1+56)

static int 			con_flag=1,discon_flag = 0;
static OptionNetS	g_optionnet;
static CDMAInfo		cdma_csq_info;
static char dial3g_apn[32];
static char dial3g_username[32];
static char dial3g_passwd[32];
static char dial3g_num[32];
static char trueusbdevpath[32]={0};
char		modemtype[32];
char		g_operation[64];
char		g_3gnetmode[32];

static unsigned char 	rddevbuf[256];
POptInfo                pCur_dial_info=NULL;
unsigned char			*devname_hs1  = "/dev/ttyHS1";	  //读取ttyHS1设备文件
int 					g_option_modem=0;
int 					g_sim_modem=0;
int 					g_DT_modem=0;
int						g_TD_LC631=0; //zhubo add 2009-03-10,为TD模块，注册命令较为特殊
int     				g_TD_DT901=0;
int     				g_MD300_MODEM=0;//zhubo add  2009.04.08 为MD300，该模块需要注册
int						g_TD_MU350=0;//zhubo add 2009.05.15
int 					g_DO_AC580=0;//zhubo add 2009.08.31 该模块很特殊，首先要在USB2上发送一条AT命令，然后再在USB0上拨号，这样才有数据通信
int						g_TD_LC5740=0;
int						g_TD_LC5730=0;
int						g_MC8780=0;
int						g_AD3812=0;
int						g_MC2716=0;
int						g_EM770 = 0;
int						g_EV888U=0;
int						gt_y3200=0;
int						g_qunkewcdma=0;
int						g_GTM380=0;


#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})

int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
int
_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int fd;
	int flags;
	int sig;
	char buf[254]="";
	int i;
	int fdtablesize;

	
	//get fdtablesize
	fdtablesize = getdtablesize();
	pid = fork();

	
	switch (pid) 
	{
	case -1:	/* error */
		perror("fork");
		return errno;
	case 0:		/* child */
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);
		
		
		for(i=3; i<fdtablesize; i++)
			close(i);

		/* Clean up */
		ioctl(0, TIOCNOTTY, 0);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		setsid();

		/* check the board if exist UART? */
		if ((fd = open("/dev/console", O_RDWR)) < 0) 
		{
		 	(void) open("/dev/null", O_RDONLY);
	 	         (void) open("/dev/null", O_WRONLY);
	 	         (void) open("/dev/null", O_WRONLY);
		}
		else
		{
			close(fd);
			(void) open("/dev/console", O_RDONLY);
            		(void) open("/dev/console", O_WRONLY);
            		(void) open("/dev/console", O_WRONLY);
		}

		/* Redirect stdout to <path> */
		if (path) 
		{
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2))
			{
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} 
			else if (!strncmp(path, ">", 1))
			{
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else 
			{
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
		}

		/* execute command */
		for(i=0 ; argv[i] ; i++)
			snprintf(buf+strlen(buf), sizeof(buf), "%s ", argv[i]);
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	/* parent */
		if (ppid) 
		{
			*ppid = pid;
			return 0;
		} 
		else 
		{
			waitpid(pid, &status, 0);
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

int write_str_ex(int fd, char *str, int len)
{
	int 			i, readlen=0, cops=0, ret=0, err=0;
	char 			buf[128];
	unsigned char 	wt_str[64];
	fd_set         inset;
  	struct timeval timeout;

  	timeout.tv_sec  = 20;
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
			printf("failed\n");
			break;
		}

		if( (readlen = read(fd, rddevbuf+cops, 1024)) > 0)
		{
			cops += readlen;
			if(strstr(rddevbuf,"OK"))
			{
				cprintf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
				break;
			}
			else
				continue;
		}
	}	

	syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
	return readlen;
}

#if 1
//zhubo add 2008.05.14
int write_str(int fd, char *str, int len)
{
	int i, readlen;
    	char buf[128];
	unsigned char wt_str[64];
	
	if(g_option_modem == 1)
	{
		if(fd>0)
		{
			sprintf(wt_str,"%s\r",str);
        		write(fd,wt_str,strlen(wt_str));			
		}
		else
		{
			fd = open(devname_hs1, O_RDWR|O_NONBLOCK);//ttyHS0
			if(fd<=0)
			{
				printf("\nopen %s,%d,%d\n",devname_hs1,fd,errno);
				return -1;
			}			
			sprintf(wt_str,"%s\r",str);
        		write(fd,wt_str,strlen(wt_str));			
		}
	}
	else
	{
		
		for( i = 0; i < len; i ++)
		{
			write ( fd, str+i, 1);
			usleep(10000);
		}
		
		//write(fd,str,strlen(str));
		write( fd, "\r", 1);
		write(fd, "\n", 1); //add by leo 2008.01.09
	}

	usleep(500000);

	memset(rddevbuf, 0, sizeof(rddevbuf));
	if( (readlen = read(fd, rddevbuf, 256)) > 0)
	{
		printf("cmd: %s,return from cdma: %s\n", str,rddevbuf);
	}

	usleep(100000);
//#ifdef SUPPORT_LOG_SECTION
	syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
//#endif

done:	
	return readlen;
}
#else

void dormir(unsigned long microsecs) {
  struct timeval timeout;
  timeout.tv_sec=microsecs/1000000L;
  timeout.tv_usec=microsecs-(timeout.tv_sec*1000000L);
  select(1,0,0,0,&timeout);
}


/* Gets a single byte from comm. device.  Return -1 if none avail. */
int read_str(int fd) {
  fd_set rfds;
  int res,lastcharnl=0,j=0;
  char ch;
  struct timeval timeout;
  timeout.tv_sec=0L;
  timeout.tv_usec=10000L;
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  unsigned long seconds;

  memset(rddevbuf,0,sizeof(rddevbuf));
  seconds=time(0)+3;
  cprintf("\ntime %d %d\n",time(0),seconds);
  while(time(0) < seconds)
  {
  	  res=select(fd+1,&rfds,NULL,NULL,&timeout);
  	  if( res >0 ) 
	  {
	    res=read(fd,&ch,1);    
	    if(res==1) 
		{
			/*
	        if(ch=='\n')
				lastcharnl=1;
	        else
			{
	          if(ch!='\r') 
			  	lastcharnl=0;
	        }
	        */
	        rddevbuf[j++] = ch;
	    }
		else
			cprintf("read error j=%d\n",j);
	    /*
	    res=read(fd, rddevbuf, 256);
		if(res>0)
			printf("return from cdma: %s\n", rddevbuf);
		*/	
	  }
	  /*
	  else if(res==0)
	  	printf("select timeout \n");
	  else if(res < 0)
	  	printf("slect error\n");
	  */	
	  
  }

  cprintf("noraml j=%d\n",j);
  return(0);
}


int write_str(int fd, char *str, int len)
{
	int res;
  	unsigned int a;
  	char ch;

	for(a=0;a<strlen(str);a++) 
	{
    	ch=str[a];
    	res=write(fd,&ch,1);
    	dormir(500);
    	if(res!=1) 
		{
      		printf("Could not write to COM device",1);
			return -1;
    	}
  	}
	res=write( fd, "\r", 1);
	write(fd, "\n", 1);
	/*
	dormir(500);
	if(res!=1) 
	{
  		printf("Could not write to COM device n",1);
		return -1;
	}
	res=write(fd, "\n", 1);
	dormir(500);
	if(res!=1) 
	{
  		printf("Could not write to COM device n",1);
		return -1;
	}
	*/

 	read_str(fd);
	cprintf("return from cdma: %s\n", rddevbuf);
	return 0;
}
#endif


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

int CheckModuleStatus(int fd_mod,CDMAInfo *pcdma_csq_info, char *netcode)
{
	int 			i=0,j=0,k=-1,n=0,t=0;
	int 			evdoflag = 0, go_pid=0;
	char 			*ptr=NULL,*choicetype=NULL,*pcimi=NULL,*ptmp=NULL, *dial3gsimpin=NULL,cimivalue[32];
	unsigned char	*tmpstr=NULL,*pstr=NULL,version[256],buf[64],cmd[32],str[128];
	unsigned int	at_check_time=3, noreg=0;
		
	pCur_dial_info = NULL;
	
	if(fd_mod<=0)
		return -1;

/******************  AT&F  START ***************************************************/
	write_str(fd_mod, "AT&F", strlen("AT&F")); //让modem恢复默认值
	sleep(1);
/******************  AT&F  END   ***************************************************/

write_str(fd_mod, "AT+CGMR", strlen("AT+CGMR"));

/******************  ATE0  START ***************************************************/
	write_str(fd_mod, "ate0", strlen("ate0")); //关闭回显
/******************  ATE0  END   ***************************************************/


		/*test for antenna
		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
		sleep(5);
		while(1)
		{
			write_str(fd_mod, "ATD112;", strlen("ATD112;"));
			sleep(10);
		}
		*/


/******************  ATI  START ***************************************************/
	File_Set_Modem_Info("usbmodemflag","yes");//检查到modem	

	memset(version, '\0', sizeof(version));

#ifndef NOTDDIAL
	write_str(fd_mod, "AT+CGSN", strlen("AT+CGSN"));//查询USB MODEM的软件版本
	if((tmpstr=strstr(rddevbuf,"OK"))||(tmpstr=strstr(rddevbuf,"ok")))
	{
		if(pstr=strstr(rddevbuf,"+CGSN:"))
		{
			pstr += 7;
			for(j=0; isdigit(*pstr); j++, pstr++)
				version[j] = *pstr;
		}
		else
		{
			for(j=2,t=0; isdigit(rddevbuf[j]); j++, t++)
				version[t] = rddevbuf[j];
		}
		File_Set_Modem_Info("modemversion",version);
	}
	else
	{
		write_str_ex(fd_mod, "ati", strlen("ati"));//查询USB MODEM的软件版本	
		if((tmpstr=strstr(rddevbuf,"OK"))||(tmpstr=strstr(rddevbuf,"ok")))
		{
			pstr = rddevbuf;
			if((rddevbuf[0]=='\r')||(rddevbuf[0]=='\n'))
				pstr++;
			if((rddevbuf[1]=='\r')||(rddevbuf[1]=='\n'))
				pstr++;

			for(j=0; pstr!=tmpstr; j++)
			{
				if((*pstr=='\n')||(*pstr=='\r'))
					version[j] = ' ';
				else
					version[j] = *pstr;
				pstr++;
			}
			File_Set_Modem_Info("modemversion",version);
		}
	}
#else
		write_str_ex(fd_mod, "ati", strlen("ati"));//查询USB MODEM的软件版本	
		if((tmpstr=strstr(rddevbuf,"OK"))||(tmpstr=strstr(rddevbuf,"ok")))
		{
			pstr = rddevbuf;
			if((rddevbuf[0]=='\r')||(rddevbuf[0]=='\n'))
				pstr++;
			if((rddevbuf[1]=='\r')||(rddevbuf[1]=='\n'))
				pstr++;

			for(j=0; pstr!=tmpstr; j++)
			{
				if((*pstr=='\n')||(*pstr=='\r'))
					version[j] = ' ';
				else
					version[j] = *pstr;
				pstr++;
			}
			File_Set_Modem_Info("modemversion",version);
		}
#endif
	pcdma_csq_info->ModuleType = 0;
/******************  ATI  END   *******************************************************/


	//WCDMA ONLY
	//write_str(fd_mod, "AT+CNMP=14", strlen("AT+CNMP=14"));
	//GSM ONLY
	//write_str(fd_mod, "AT+CNMP=13", strlen("AT+CNMP=13"));
	//sleep(1);
	
	/*
	//EVDO ONLY
	//write_str(fd_mod, "AT^PREFMODE=4", strlen("AT^PREFMODE=4"));
	//CDMA ONLY
	write_str(fd_mod, "AT^PREFMODE=2", strlen("AT^PREFMODE=2"));
	sleep(1);
	*/

if(!g_EV888U)
{
#ifndef FOREIGN_MODULE
/******************  AT+CPIN  START ***************************************************/

	for(i=0; i<15; i++)
	{
		write_str(fd_mod, "AT+CPIN?", strlen("AT+CPIN?"));
		if( ptr = strstr(rddevbuf, "OK") == 0)
		{
			strcpy(pcdma_csq_info->Status,"SIM failure");
			pcdma_csq_info->csq = -1;

		#ifdef SUPPORT_LOG_SECTION
		   syslog(LOG_DEBUG,"SIM failure!\n");
		#endif

			File_Set_Modem_Info("dial3gsimcard", "failure");
			if(i < 8 )
			{
				sleep(3);
				continue;
			}
			else
			{
				return -1;
			}
				
		}		
		
		if(ptr = strstr(rddevbuf, "SIM PIN"))//需要解pin密码
		{

			File_Set_Modem_Info("simpinlock", "OPEN"); // sim pin lock is open
			
			dial3gsimpin = nvram_bufget(RT2860_NVRAM,"dial3gsimpin");

			if(dial3gsimpin&&strlen(dial3gsimpin))
				sprintf(cmd,"AT+CPIN=\"%s\"\0",dial3gsimpin);
			else
			{
				#if 0 //modify  by hs
				#ifdef NOTDDIAL
					gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
				#else
				#ifdef SET_PASSWORD
					gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
				#else
					gpio_set_led(GPIO_3G_LED2, 1, 1, 1, 0, 4000);	//SIM PIN  快闪
				#endif
				#endif
				#endif
				
				File_Set_Modem_Info("dial3gsimcard", "SIMPINNull");
				while(1)
				{
					sleep(5);
					write_str(fd_mod, "AT+CPIN?", strlen("AT+CPIN?"));
					if( ptr = strstr(rddevbuf, "READY"))
					{
						#ifdef SUPPORT_LOG_SECTION
				   			syslog(LOG_DEBUG,"SIM READY!\n");
						#endif

						//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000); //  PIN恢复慢闪

						File_Set_Modem_Info("dial3gsimcard", "successful");
						break;
					}
				}
				break;
			}
			
			write_str(fd_mod, cmd, strlen(cmd));
			if( ptr = strstr(rddevbuf, "OK") == 0)
			{
				strcpy(pcdma_csq_info->Status,"SIM PIN");
				pcdma_csq_info->csq = -1;
#if 0		//modify by hs
				#ifdef NOTDDIAL
					gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
				#else
				#ifdef SET_PASSWORD
					gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
				#else
					gpio_set_led(GPIO_3G_LED2, 1, 1, 1, 0, 4000);	//SIM PIN  快闪
				#endif
				#endif
#endif
				File_Set_Modem_Info("dial3gsimcard", "SIMPINFail");

				#ifdef SUPPORT_LOG_SECTION
				   syslog(LOG_DEBUG,"SIM PIN!\n");
				#endif

				while(1)
				{
					sleep(5);
					write_str(fd_mod, "AT+CPIN?", strlen("AT+CPIN?"));
					if( ptr = strstr(rddevbuf, "READY"))
					{
						#ifdef SUPPORT_LOG_SECTION
				   			syslog(LOG_DEBUG,"SIM READY!\n");
						#endif	

						//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000); //  PIN恢复慢闪

						File_Set_Modem_Info("dial3gsimcard", "successful");
						break;
					}
				}
				break;
			}
		}
		else if(ptr = strstr(rddevbuf, "SIM PUK"))//需要解puk密码
		{
			File_Set_Modem_Info("simpinlock", "OPEN"); // sim pin lock is open
#if 0		//modify by hs
			#ifdef NOTDDIAL
				gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
			#else
			#ifdef SET_PASSWORD
				gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
			#else
				gpio_set_led(GPIO_3G_LED2, 1, 1, 1, 0, 4000);	//SIM PIN  快闪
			#endif
			#endif
#endif		
			File_Set_Modem_Info("dial3gsimcard", "SIMPUK");

			#ifdef SUPPORT_LOG_SECTION
				   syslog(LOG_DEBUG,"SIM PUK!\n");
				#endif

			while(1)
			{
				sleep(5);
				write_str(fd_mod, "AT+CPIN?", strlen("AT+CPIN?"));
				if( ptr = strstr(rddevbuf, "READY"))
				{
					#ifdef SUPPORT_LOG_SECTION
			   			syslog(LOG_DEBUG,"SIM READY!\n");
					#endif	

					//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000); //  PIN恢复慢闪

					File_Set_Modem_Info("dial3gsimcard", "successful");
					break;
				}
			}
			break;
		}
		else if(ptr = strstr(rddevbuf, "READY"))
		{
			File_Set_Modem_Info("dial3gsimcard", "successful");
			break;
		}			
	}	

/******************  AT+CPIN  END ***************************************************/
#endif
}

/******************  AT+CIMI  START ***************************************************/
	memset(cimivalue, '\0', sizeof(cimivalue));
	//write_str_ex(fd_mod, "at+cimi", strlen("at+cimi"));
	write_str(fd_mod, "at+cimi", strlen("at+cimi")); //by luotao
	if(strstr(rddevbuf,"OK"))
	{
			/*
			ptmp = rddevbuf;
			if((rddevbuf[0]=='\r')||(rddevbuf[0]=='\n'))
				ptmp++;
			if((rddevbuf[1]=='\r')||(rddevbuf[1]=='\n'))
				ptmp++;
			*/	

			if(ptmp = strstr(rddevbuf, "+CIMI:"))
			{
				ptmp += 7;
				for(i=0; isdigit(*ptmp); i++)
				{
					cimivalue[i] = *ptmp;
					ptmp++;
				}				
			}
			else
			{
				for(j=2,t=0; isdigit(rddevbuf[j]); j++, t++)
					cimivalue[t] = rddevbuf[j];
			}
			File_Set_Modem_Info("cimivalue",cimivalue);
			ptmp = NULL;
			
		    for (j=0; OptName[j].ID != NULL;j++ )
	    	{
				ptmp = strstr(rddevbuf,OptName[j].ID);
				if(ptmp)
				{
					if(!pcimi)
					{
						pcimi = ptmp;
						k = j;
					}
					else
					{	    					
						if(pcimi>ptmp)
						{
							pcimi = ptmp;
							k = j;
						}	
					}
				}	    			
	    	 }           

			if(k != -1) //列表中有相同的对应关系
			{
				if(OptName[k].optname!=NULL)
				{
						char CMD[100];
						
				  		pCur_dial_info = (POptInfo)&OptName[k];
						strcpy(pcdma_csq_info->operatorName, OptName[k].optname);
						
						//nvram_set("simcimi",pCur_dial_info->ID);
						File_Set_Modem_Info("simcimi", pCur_dial_info->ID);
						
						//运营商名称
						//nvram_set("dial3gprovider",pCur_dial_info->optname);
						File_Set_Modem_Info("dial3gprovider", pCur_dial_info->optname);
						strcpy(g_operation,pCur_dial_info->optname);
																			
						//设置拨号号码-自动
						sprintf(CMD, "nvram_set dial3gnum \\%s", pCur_dial_info->dialnum);system(CMD);
						File_Set_Modem_Info("dial3gnum_auto", pCur_dial_info->dialnum);

						//设置用户名-自动
						sprintf(CMD, "nvram_set dial3gusername %s", pCur_dial_info->username);system(CMD);
						File_Set_Modem_Info("dial3gusername_auto", pCur_dial_info->username);

						//设置密码-自动
						sprintf(CMD, "nvram_set dial3gpassword %s", pCur_dial_info->passwd);system(CMD);
						File_Set_Modem_Info("dial3gpassword_auto", pCur_dial_info->passwd);

						//设置APN-自动
						sprintf(CMD, "nvram_set dial3gapn %s", pCur_dial_info->papn);system(CMD);
						File_Set_Modem_Info("dial3gapn_auto", pCur_dial_info->papn);
				}	     		
				else
					memset(pcdma_csq_info->operatorName, '\0', 16);	
			}
			else
				memset(pcdma_csq_info->operatorName, '\0', 16);	

			
			  				
	} 
		if(g_EV888U)		///设置运营商名称
		{
			File_Set_Modem_Info("dial3gprovider", "CHINA TELECOM");
	    	}
	/*
	else
	{
		memset(pcdma_csq_info->operatorName, '\0', 16);
		File_Set_Modem_Info("dial3gsimcard", "failure");
		return -1;
	}
	*/
/******************  AT+CIMI  END ***************************************************/

	/*
	write_str(fd_mod, "AT+SPEAKER=2", strlen("AT+SPEAKER=2"));
	write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
	sleep(10);
	while(1)
	{
		write_str(fd_mod, "at+csq", strlen("at+csq"));
		write_str(fd_mod, "ATD13510244170;", strlen("ATD13510244170;"));
		sleep(40);
	}
	*/

/*********************** TD MODEM AT COMMAND START***********************************/
  	if(g_sim_modem)
	{
        		write_str(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));//设置显示字符
        			sleep(10);
        		write_str(fd_mod, "AT+CGEQREQ=1,3,64,2048,64,2048", strlen("AT+CGEQREQ=1,3,64,2048,64,2048"));//设置显示字符
        			sleep(5);

		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);
	}
	else if(gt_y3200)
	{
		write_str(fd_mod, "AT+CREG=1", strlen("AT+CREG=1"));
        		sleep(2);

		//TD ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=15,2,1,2", strlen("AT^SYSCONFIG=15,2,1,2"));
		//GSM ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=13,1,1,2", strlen("AT^SYSCONFIG=13,1,1,2"));
		//TD 优先
		write_str(fd_mod, "AT^SYSCONFIG=2,2,1,2", strlen("AT^SYSCONFIG=2,2,1,2"));
		sleep(1);

		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
        		sleep(2);
		
		if(netcode)
		{
			write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
			sleep(2);

			memset(buf, '\0', 64);
			sprintf(buf,"AT+COPS=1,2,\"%s\",2",netcode);
			write_str(fd_mod, buf, strlen(buf));
        			sleep(4);
		}

		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);

		//while(at_check_time--)
		while(1)
		{
			write_str(fd_mod, "AT+CREG?", strlen("AT+CREG?"));
			if(strstr(rddevbuf,"+CREG: 1,1")||strstr(rddevbuf,"+CREG: 1,5"))/// 1 注册了本地网络  5 注册了漫游网络
				break;
			else
			{	
				write_str(fd_mod, "AT+CGREG?", strlen("AT+CGREG?"));
				if(strstr(rddevbuf,"+CGREG: 1,1")||strstr(rddevbuf,"+CGREG: 1,5"))/// 1 注册了本地网络  5 注册了漫游网络
					break;
				else
				{
					if(noreg++ == 7)
						printf("set led action is disable\n");
					//gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);	// 网络无法注册 快闪
				}
				sleep(2);	
			}				
		}
		noreg = 0;
#if 0
		#ifdef NOTDDIAL
			gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
		#else
		#ifdef SET_PASSWORD
			gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
		#else
			gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000); //  网络恢复 慢闪
		#endif
		#endif
#endif
	}
  	else if(g_DT_modem)
	{	
		write_str(fd_mod, "AT+CREG=1", strlen("AT+CREG=1"));
        sleep(2);

		//TD ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=15,2,1,2", strlen("AT^SYSCONFIG=15,2,1,2"));
		//GSM ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=13,1,1,2", strlen("AT^SYSCONFIG=13,1,1,2"));
		//sleep(1);

		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
        sleep(2);
		
		if(netcode)
		{
			write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
			sleep(2);

			memset(buf, '\0', 64);
			sprintf(buf,"AT+COPS=1,2,\"%s\",2",netcode);
			write_str(fd_mod, buf, strlen(buf));
        	sleep(4);
		}

		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);

		//while(at_check_time--)
		while(1)
		{
			write_str(fd_mod, "AT+CREG?", strlen("AT+CREG?"));
			if(strstr(rddevbuf,"+CREG: 1,1")||strstr(rddevbuf,"+CREG: 1,5"))/// 1 注册了本地网络  5 注册了漫游网络
				break;
			else
			{	
				write_str(fd_mod, "AT+CGREG?", strlen("AT+CGREG?"));
				if(strstr(rddevbuf,"+CGREG: 1,1")||strstr(rddevbuf,"+CGREG: 1,5"))/// 1 注册了本地网络  5 注册了漫游网络
					break;
				else
				{
					if(noreg++ == 7)
						printf("set led action is disable\n");
					//gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);	// 网络无法注册 快闪
				}
				sleep(2);	
			}				
		}
		noreg = 0;
#if 0
		#ifdef NOTDDIAL
			gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
		#else
		#ifdef SET_PASSWORD
			gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
		#else
			gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000); //  网络恢复 慢闪
		#endif
		#endif
#endif
	}
	else if(g_TD_MU350)
	{		
	    //write_str(fd_mod, "AT^MODE=0", strlen("AT^MODE=0"));
		//usleep(500000);
		write_str(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));//设置显示字符
	    sleep(1);
		write_str(fd_mod, "AT+CREG=1", strlen("AT+CREG=1"));
        sleep(2);
		write_str(fd_mod, "AT^CARDMODE", strlen("AT^CARDMODE"));
		usleep(500000);
		write_str(fd_mod, "AT^SYSINFO", strlen("AT^SYSINFO"));
		usleep(500000);
		write_str(fd_mod, "AT+CREG?", strlen("AT+CREG?"));

		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);
	}
	else if(g_TD_DT901)
	{		
		write_str(fd_mod, "AT+CREG=1", strlen("AT+CREG=1"));
        sleep(2);
	printf("\r\n G_DT901");
		//TD ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=15,2,1,2", strlen("AT^SYSCONFIG=15,2,1,2"));
		//GSM ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=13,1,1,2", strlen("AT^SYSCONFIG=13,1,1,2"));
		//TD 优先
		//write_str(fd_mod, "AT^SYSCONFIG=2,2,1,2", strlen("AT^SYSCONFIG=2,2,1,2"));
		//sleep(1);

		write_str(fd_mod, "AT+CFUN=5", strlen("AT+CFUN=5"));
		sleep(3);

		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
		//sleep(10);

		/*
		write_str(fd_mod, "AT+COPS=?", strlen("AT+COPS=?"));

		write_str(fd_mod, "AT", strlen("AT"));

		//TEST
		write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
		sleep(1);		
		write_str(fd_mod, "AT+COPS=1,2,\"46007\",2", strlen("AT+COPS=1,2,\"46007\",2"));
        sleep(4);
        */

		if(netcode)
		{
			write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
			sleep(2);

			memset(buf, '\0', 64);
			sprintf(buf,"AT+COPS=1,2,\"%s\",2",netcode);
			//write_str(fd_mod, buf, strlen(buf));
        	//sleep(4);
        	write_str_ex(fd_mod, buf, strlen(buf));
		}

		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);

		while(1)
		{
			printf("\r\n G_DT901--TTTT");
			write_str(fd_mod, "AT+CREG?", strlen("AT+CREG?"));
			if(strstr(rddevbuf,"+CREG: 1,1")||strstr(rddevbuf,"+CREG: 1,5"))
				break;
			else
				sleep(2);
			break;
		}
	}
	else if(g_TD_LC5740)
	{	
		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
		sleep(2);
		//TD ONLY
		//write_str(fd_mod, "AT^SYSCONFIG=15,2,1,2", strlen("AT^SYSCONFIG=15,2,1,2"));
		sleep(2);
		if(netcode)
		{
			write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
			sleep(2);

			memset(buf, '\0', 64);
			sprintf(buf,"AT+COPS=1,2,\"%s\",2",netcode);
			write_str(fd_mod, buf, strlen(buf));
        	sleep(4);
		}

		go_pid = DBgetPid("checkat");
		if(!go_pid)
			eval("checkat", "1");

	}
	else if(g_TD_LC5730)
	{	
		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
		sleep(2);

		if(netcode)
		{
			write_str(fd_mod, "AT+COPS=3,2", strlen("AT+COPS=3,2"));
			sleep(2);

			memset(buf, '\0', 64);
			sprintf(buf,"AT+COPS=1,2,\"%s\",2",netcode);
			write_str(fd_mod, buf, strlen(buf));
        	sleep(4);
		}

		go_pid = DBgetPid("checkat");
		if(!go_pid)
			eval("checkat", "1");

	}
	else if(g_AD3812)
	{
		write_str(fd_mod, "AT+SPEAKER=2", strlen("AT+SPEAKER=2"));

		go_pid = DBgetPid("checkat");
		if(!go_pid)
			eval("checkat", "0");
	}
	else if(g_EM770)
	{
		go_pid = DBgetPid("checkat");
		if(!go_pid)
			eval("checkat", "1");
	}
	else if(g_MC2716)
	{
		if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);
	}
	else if(g_GTM380)
	{
		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
       		 sleep(2);
		write_str(fd_mod, "AT_OCTI=1", strlen("AT_OCTI=1"));
       		 sleep(1);
		write_str(fd_mod, "AT_OWIND=1", strlen("AT_OWIND=1"));
       		 sleep(1);
		write_str(fd_mod, "AT_OIDLETEXT=1", strlen("AT_OIDLETEXT=1"));
       		 sleep(1);
		write_str(fd_mod, "AT+CNMI=2,1,0,1", strlen("AT+CNMI=2,1,0,1"));
        		sleep(1);
		write_str(fd_mod, "AT+COPS=3,0", strlen("AT+COPS=3,0"));
        		sleep(1);
		
	if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
		display_signal(0,31);
	}
	else if(g_EV888U)
	{
		write_str(fd_mod, "AT+CREG=1", strlen("AT+CREG=1"));
        sleep(2);

		write_str_ex(fd_mod, "AT+CFUN=1", strlen("AT+CFUN=1"));
        sleep(2);

	if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
		display_signal(0,31);
	}
	else
	{
	  	if(strcmp(REALM_STR,"TOTO LINK")&&strcmp(REALM_STR,"NG4190"))
			display_signal(0,31);
	}
	
/*********************** TD MODEM AT COMMAND END*************************************/

#if 1
/******************  AT+CSQ  START ***************************************************/
#ifdef TEST_SIGNAL
		while(1)
		{
			write_str(fd_mod, "at+csq", strlen("at+csq"));
			   
			if( (ptr = strstr(rddevbuf, "+CSQ:")) == NULL)
			{
				pcdma_csq_info->csq = -1;
				File_Set_Modem_Info("dial3gsignal", "-1");
				sleep(3);
				continue;
			}
			ptr += 5;
			while( *ptr == ' ')
				ptr++;
			pcdma_csq_info->csq = strtoul( ptr, NULL, 10);
			if((99 == pcdma_csq_info->csq)||(0 == pcdma_csq_info->csq)||(199 == pcdma_csq_info->csq))
			   	File_Set_Modem_Info("dial3gsignal", "0");
			else
			{
				sprintf(cmd,"%d\0",pcdma_csq_info->csq);
				File_Set_Modem_Info("dial3gsignal", cmd);
			}				

			sleep(3);
		}
#else
		#ifndef FOREIGN_MODULE
			//Check CSQ
			for(i=0; i < 500; i++)
			{ 
				write_str(fd_mod, "at+csq", strlen("at+csq"));

				if(g_EV888U)
				{
					if( (ptr = strstr(rddevbuf, ",")) == NULL)
					{
						pcdma_csq_info->csq = -1;
						//nvram_set("dial3gsignal","-1");
						File_Set_Modem_Info("dial3gsignal", "-1");
						return -1;
					}	
					pcdma_csq_info->csq = strtoul( strtok(rddevbuf, ","), NULL, 10);
					if((99 == pcdma_csq_info->csq)||(0 == pcdma_csq_info->csq)||(199 == pcdma_csq_info->csq))
					{
				       if(i < 5)
					   {
				       		usleep(20000);
							continue;
				       }
						   
					   strcpy(pcdma_csq_info->Status,"NO Signal");
					   pcdma_csq_info->csq = 0;

				   		//#ifdef POWER3G9
							//gpio_set_led(GPIO_3G_LED2, 4000, 0, 1, 0, 4000);//常亮
						//#endif

					   			 
					   //nvram_set("dial3gsignal","0");
					   File_Set_Modem_Info("dial3gsignal", "0");
					   return -1;
					}
					else
						break;
			}	
			else if(g_qunkewcdma)
			{
				if( (ptr = strstr(rddevbuf, "+CSQ:")) == NULL)
				{
					pcdma_csq_info->csq = -1;
					File_Set_Modem_Info("dial3gsignal", "-1");
					return 0;
				}	
				ptr += 6;
				while( *ptr == ',')
					ptr++;
				pcdma_csq_info->csq = strtoul( ptr, NULL, 10);
				if((99 == pcdma_csq_info->csq)||(0 == pcdma_csq_info->csq)||(199 == pcdma_csq_info->csq))
				{
			       if(i < 500)
				   {
			       		usleep(20000);
						continue;
			       }
					   
				   strcpy(pcdma_csq_info->Status,"NO Signal");
				   pcdma_csq_info->csq = 0;
				   			 
				   File_Set_Modem_Info("dial3gsignal", "0");
				   return 0;
				}
				else
					break;
			}
			else
			{
				if( (ptr = strstr(rddevbuf, "+CSQ:")) == NULL)
				{
					pcdma_csq_info->csq = -1;
					File_Set_Modem_Info("dial3gsignal", "-1");
					return 0;
				}	
				ptr += 5;
				while( *ptr == ' ')
					ptr++;
				pcdma_csq_info->csq = strtoul( ptr, NULL, 10);
				if((99 == pcdma_csq_info->csq)||(0 == pcdma_csq_info->csq)||(199 == pcdma_csq_info->csq))
				{
					printf("\r\n for test force dial");
					break;
			       if(i < 500)
				   {
			       		usleep(20000);
						continue;
			       }
					   
				   strcpy(pcdma_csq_info->Status,"NO Signal");
				   pcdma_csq_info->csq = 0;
				   			 
				   File_Set_Modem_Info("dial3gsignal", "0");
				   return 0;
				}
				else
					break;
			}
		}
			
			sprintf(cmd,"%d\0",pcdma_csq_info->csq);
			File_Set_Modem_Info("dial3gsignal", cmd);
		#endif
#endif
/******************  AT+CSQ  END ***************************************************/
#endif
		
	return 0;
}

void WriteWcdmaApn(int fd, char *devname, char *apnvalue)
{
	unsigned char buf[100];
	//usleep(100000);

	/*
	dial3g_apn=nvram_bufget(RT2860_NVRAM,"dial3gapn");
				
	fd = open(devname, O_NONBLOCK | O_RDWR, 0);
	if( fd <= 0)
	{
		printf("open %s error\n", devname);
		return;
	}

	if(g_option_modem == 1)
	{
		close(fd);
		fd = 0;
	}	
	
	//sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", pWebVar->wanAPN);
	if(cdma_csq_info.ModuleType==2)//evdo模块不需要写APN
		return;

	if(strlen(pWebVar->wanAPN))
		sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", pWebVar->wanAPN);
	else
		sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", "cmnet");
	
	write_str(fd, buf, strlen(buf));
	if(!g_option_modem)
	{
		close(fd);
		fd = 0;
	}
*/
	if(apnvalue[0]=='\0')
		return;

	sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", apnvalue);	
	write_str(fd, buf, strlen(buf));
}



/*
 * create_resolv - create the replacement resolv.conf file
 */
static void
create_resolv(unsigned char *peerdns1, unsigned char *peerdns2)
{
    FILE *f;

    f = fopen("/etc/resolv.conf", "w");
    if (f == NULL) {
	error("Failed to create %s: %m", "/etc/resolv.conf");
	return;
    }

    if (peerdns1)
	fprintf(f, "nameserver %s\n", peerdns1);

    if (peerdns2)
	fprintf(f, "nameserver %s\n", peerdns2);

    if (ferror(f))
	error("Write failed to %s: %m", "/etc/resolv.conf");

    fclose(f);
}


/*
特别针对option模块拨号
*/
void Dial_Option_Modem(void)
{
	#if 0
	int 		  fd=0, fd_option=0,i=0;
	int     	  wflag=0;//是否写flash的标志
	unsigned char buf[64],ip[16],dns1[16],dns2[16],*pb=NULL,cmd_dns[128];

	dial3g_username=nvram_bufget(RT2860_NVRAM,"dial3gusername");
	dial3g_passwd=nvram_bufget(RT2860_NVRAM,"dial3gpasswd");
		
	memset(buf,'\0',64);
	memset(g_optionnet.ip,'\0',16);
	memset(g_optionnet.dns1,'\0',16);
	memset(g_optionnet.dns2,'\0',16);
	memset(cmd_dns,'\0',128);	

	sprintf(buf,"AT$QCPDPP=1,1,\"%s\",\"%s\"",dial3g_passwd,dial3g_username);		
	printf("option buf=%s\n",buf);

	write_str(fd, buf, strlen(buf));
	sleep(1);
	printf("AT_OWANCALL=1,1,0\n");
	write_str(fd, "AT_OWANCALL=1,1,0",strlen("AT_OWANCALL=1,1,0"));
	sleep(5);
	printf("AT_OWANDATA=1\n");
	write_str(fd, "AT_OWANDATA=1",strlen("AT_OWANDATA=1"));	
	sleep(6);
	if(pb=strchr(rddevbuf,','))
	{
		pb += 2;
		i = 0;
		while(*pb != ',')
		{
			g_optionnet.ip[i] = *pb;
			i++;
			pb++;
		}
		g_optionnet.ip[i] = '\0';

		pb += 11;
		i = 0;
		while(*pb != ',')
		{
			g_optionnet.dns1[i] = *pb;
			pb++;
			i++;
		}
		g_optionnet.dns1[i] = '\0';

		pb += 2;
		i = 0;
		while(*pb != ',')
		{
			g_optionnet.dns2[i] = *pb;
			pb++;
			i++;
		}
		g_optionnet.dns2[i] = '\0';
		printf("Dial option is %s,%s,%s\n",g_optionnet.ip,g_optionnet.dns1,g_optionnet.dns2);	

		system("rm -f /var/fyi/sys/dns");
		system("rm -rf /var/fyi/sys");
		system("rm -rf /var/fyi");
		system("mkdir -p /var/fyi/sys");
   		system("echo > /var/fyi/sys/dns");

		sprintf(cmd_dns, "%s%s%s","echo  \"nameserver ", g_optionnet.dns1, "\" >> /var/fyi/sys/dns");
		system(cmd_dns);
		sleep(1);
		sprintf(cmd_dns, "%s%s%s","echo  \"nameserver ", g_optionnet.dns2, "\" >> /var/fyi/sys/dns");
		system(cmd_dns);
		sleep(1);

		sprintf(cmd_dns,"ifconfig ppp0 %s up",g_optionnet.ip);
		system(cmd_dns);
		usleep(10000);
		sprintf(cmd_dns,"%s","route add default ppp0");
		system(cmd_dns);
		
		create_resolv(g_optionnet.dns1, g_optionnet.dns2);

		system("killall -9 dnsmasq");
		sleep(1);
		system("dnsmasq -h -i br0 -r /etc/resolv.conf &");
	}	
	#endif
}




void DoCommand ( char *cmd )
{
	int pid;
	char *ptr;


	if ( cmd == NULL )
	{
		//printf ( "command: %s\n", cmd );
		return;
	}
	
	if ( !strlen(cmd) )
	{
		//printf ( "command len is 0\n" );
		return;
	}
	
#if 1
	pid = vfork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		//printf ( "%s\n", cmd );
		execl ( "/bin/sh", "sh", "-c", cmd, NULL );
		exit(127);
        }
#else
	_eval(cmd, NULL, NULL, NULL);
#endif
}


void CDMADial( void )
{
	int ret, mtu = 1452;
	static int i=0;
	char cmd[160];
	pid_t pid;

	/*
	pid = DBgetPid("led");
	if ( !pid )
	{
		sprintf(cmd, "led blink &");
		DoCommand ( cmd );
	}
	*/
	
	pid = DBgetPid("pppd");
    if ( !pid )
	{

	#ifdef NOTDDIAL
		//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
	#else
	#ifdef SET_PASSWORD
		//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
	#else
		memset(g_3gnetmode, '\0', sizeof(g_3gnetmode));
		File_Get_Modem_Info("dial3gnetmode",g_3gnetmode, 32);

		if(g_TD_LC5740||g_AD3812)
		{
			if(!strcmp(g_3gnetmode,"WCDMA")||!strcmp(g_3gnetmode,"EVDO")||!strcmp(g_3gnetmode,"TD-SCDMA"))
			{
				//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
				//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
				
				//gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000);
			}		
			else
			{
				//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
				//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
				
				//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
			}
		}
		else
		{
			//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
			//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
				
			//gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000);
		}
	#endif
	#endif
			
		eval("pppd","file", "/var/config/ppp/peers/3g");
		//eval("pppd", "file", "/etc_ro/options.ttySx");
	}	
}



void delete_modem_info()
{
	int 			fd=0, n=0, i=0;
	char			path[64];
	struct dirent** namelist = NULL;

	n = scandir("/var/usbmodem",&namelist,0,alphasort);
	for(i=0; i<n; i++)
	{
		if((strcmp(".",namelist[i]->d_name)==0)||(strcmp("..",namelist[i]->d_name)==0))
		{
	    	free(namelist[i]);
	    	continue;
		}
		else
	    {
			sprintf(path,"%s%s\0", "/var/usbmodem/", namelist[i]->d_name);
			if(strncmp(namelist[i]->d_name,"mode_insert",strlen("mode_insert")))
				unlink(path);
	    }
	    free(namelist[i]);
	}
	free(namelist);
}


void ConSigHandler(int signum)
{
	if(signum != SIG_CON_MSG)
		return;

	con_flag   = 1;
	discon_flag = 0;
}

void DisConSigHandler(int signum)
{
	int pid_pppd;
	
	if(signum != SIG_DISCON_MSG)
		return;

	discon_flag = 1;
	con_flag    = 0;
	//eval("killall","-9","pppd");
	//system("killall -9 pppd");
	//sleep(25);
	/*
	while(1)
	{
		pid_pppd = DBgetPid("pppd");
		if(pid_pppd>0)
			sleep(1);
		else
			break;
	}
	
	eval("comgt","-d","/var/ttyACM0","-s","/etc_ro/ppp/3g/Generic_disconn.scr");
	sleep(2);
	*/
}

int main(int argc, char **argv)
{
	FILE 		* fp;
	int 		ret, go_pid=0, dial_style=0, pid_pppd=0;
	int 		option_flag=48;//'0':not option modem
	int 		stsOld = IFC_STATUS_DOWN, stsNew = IFC_STATUS_DOWN;
	char		*choicetype =NULL, *ptmp=NULL;
	static int 	at_flag=0, com_discon=0;

	if(argc < 2)
		return 0;

	dial_style = atoi(argv[1]); //0 : auto 	1 : manual

	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

	signal(SIG_CON_MSG, ConSigHandler);
	signal(SIG_DISCON_MSG, DisConSigHandler);

	memset(dial3g_apn,'\0',32);
	memset(dial3g_username,'\0',32);
	memset(dial3g_passwd,'\0',32);
	memset(dial3g_num,'\0',32);
	memset(modemtype, '\0',32);	
	system("echo 5 > /var/6666");
	printf("\r\ntesd");
	while(1)
	{
		//printf("\r\ntesd---%d",discon_flag);
		if(discon_flag)//断开
		{
			//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
			//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
			system("echo 5 > /var/7777");
			pid_pppd = DBgetPid("pppd");
			if(pid_pppd>0)
				eval("killall","-15","pppd");
			else
			{
				if( !com_discon)
				{
					com_discon = 1;
					eval("comgt","-d","/var/ttyACM0","-s","/etc_ro/ppp/3g/Generic_disconn.scr");
					sleep(2);
				}				
			}			
			goto done;
		}
		
		if(access("/var/ttyACM0",F_OK)==-1)
		{//ttyUSB0 is not exist
			at_flag = 0;				
			if (-1 == access("/var/hotplugadd", F_OK))//说明该文件不存在
			{
				delete_modem_info();
			}
			dial3g_apn[0] = '\0';
			dial3g_username[0] = '\0';
			dial3g_passwd[0] = '\0';
			dial3g_num[0] = '\0';
			modemtype[0] = '\0';
			g_TD_DT901 = 0;
			g_TD_LC631 = 0;
			g_DT_modem = 0;
			g_DO_AC580 = 0;
			g_TD_MU350 = 0;
			g_sim_modem= 0;
			g_MC8780=0;
			g_TD_LC5740= 0;
			g_TD_LC5730=0;
			g_AD3812 = 0;
			g_EV888U=0;
			gt_y3200=0;
			g_qunkewcdma=0;
			g_GTM380=0;

			stsNew = IFC_STATUS_DOWN;
#if 0
			#ifdef NOTDDIAL
				gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
			#else
			#ifdef SET_PASSWORD
				gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
			#else
				//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
				gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 1, 4000);
			#endif
			#endif
#endif		
			sleep(3);
			continue;
		}
		else
		{
			fp=fopen("/var/ttyACM0","r");

			if(fp==NULL)
			{
				at_flag = 0;
				if (-1 == access("/var/hotplugadd", F_OK))//说明该文件不存在
				{
					delete_modem_info();
				}
				
				dial3g_apn[0] = '\0';
				dial3g_username[0] = '\0';
				dial3g_passwd[0] = '\0';
				dial3g_num[0] = '\0';
				modemtype[0] = '\0';
				g_TD_DT901 = 0;
				g_TD_LC631 = 0;
				g_DT_modem = 0;
				g_DO_AC580 = 0;
				g_TD_MU350 = 0;
				g_sim_modem= 0;	
				g_TD_LC5740= 0;
				g_TD_LC5730=0;
				g_MC8780=0;
				g_AD3812 = 0;
				g_EV888U=0;
				gt_y3200=0;
				g_qunkewcdma=0;
				g_GTM380=0;

				stsNew = IFC_STATUS_DOWN;
#if 0
				#ifdef NOTDDIAL
					gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
				#else
				#ifdef SET_PASSWORD
					gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
				#else
					//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 1, 4000);
					gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 1, 4000);
				#endif
				#endif
#endif			
				if(DBgetPid("pppd"))
				{
					system("killall -15 pppd");
				}
				if(DBgetPid("checkat"))
				{
					system("killall -9 checkat");
				}
				eval("rm","-f","/var/ttyACM0");
				sleep(1);
				eval("rm","-f","/var/ttyAT");
				continue;
			}
			else
				fclose(fp);
		}

		ret = DBgetPPPStatus( "ppp0", "pppd" );		
		if ( ret == _DISCONNECT ) //如果是断开连接的,则拨号
		{
			printf("\r\ntest disconnect !");
			eval("comgt","-d","/var/ttyACM0","-s","/etc_ro/ppp/3g/Generic_disconn.scr");
			//sleep(2);
			sleep(1);
			
			if(at_flag==0)
			{
				at_flag = 1;

				eval("comgt","-d","/var/ttyACM0","-s","/etc_ro/ppp/3g/Generic_conn.scr");
				sleep(1);

				int fdd=0;
				fdd=open("/var/ttyACM0", O_RDWR|O_NONBLOCK);
				if(fdd > 0)
				{
#if 0
					#ifdef NOTDDIAL
						gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
					#else
				       #ifdef SET_PASSWORD
					 	gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
				     	#else
						//gpio_set_led(GPIO_PORT_3G, 10, 10, 10, 0, 4000);
						gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000); // low 
						gpio_set_led(GPIO_3G_LED2, 10, 10, 10, 0, 4000); //  网络恢复 慢闪
					#endif
					#endif
#endif					
					File_Get_Modem_Info("modemtype", modemtype, 32);
					if(!strncmp(modemtype,"dtmmodem", strlen("dtmmodem")))
						g_DT_modem=1;
					else if(!strncmp(modemtype,"simmodem", strlen("simmodem")))
						g_sim_modem=1;
					else if(!strncmp(modemtype,"901mmodem", strlen("901mmodem")))
						g_TD_DT901 = 1;
					else if(!strncmp(modemtype,"mu350mmodem", strlen("mu350mmodem")))
						g_TD_MU350 = 1;
					else if(!strncmp(modemtype,"LC5740", strlen("LC5740")))
						g_TD_LC5740 = 1;
					else if(!strncmp(modemtype,"LC5730", strlen("LC5730")))
						g_TD_LC5730 = 1;
					else if(!strncmp(modemtype,"mc8780", strlen("mc8780")))
						g_MC8780= 1;
					else if(!strncmp(modemtype,"ad3812modem", strlen("ad3812modem")))
						g_AD3812 = 1;
					else if(!strncmp(modemtype,"EM770", strlen("EM770")))
						g_EM770= 1;
					else if(!strncmp(modemtype,"MC2716", strlen("MC2716")))
						g_MC2716= 1;
					else if(!strncmp(modemtype,"EV888U", strlen("EV888U")))
						g_EV888U= 1;
					else if(!strncmp(modemtype,"gty3200", strlen("gty3200")))
						gt_y3200= 1;
					else if(!strncmp(modemtype,"QUNKEWCDMA", strlen("QUNKEWCDMA")))
						g_qunkewcdma= 1;
					else if(!strncmp(modemtype,"GTM380", strlen("GTM380")))
						g_GTM380= 1;

					if(argv[2]==NULL)
						ret = CheckModuleStatus(fdd,&cdma_csq_info,NULL);
					else
						ret = CheckModuleStatus(fdd,&cdma_csq_info,argv[2]);

					if(ret < 0)
					{						
						close(fdd);
						fdd = 0;
				#if 0
						//gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);	//SIM 故障 快闪
						#ifdef NOTDDIAL
							gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
						#else
						#ifdef SET_PASSWORD
							gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
						#else
							gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000); // low 
							gpio_set_led(GPIO_3G_LED2, 1, 1, 1, 0, 4000);	//SIM 故障 快闪
						#endif
						#endif
				#endif		
						
						exit(0);
						goto done;
					}

					nvram_init(RT2860_NVRAM);
					system("echo 3434 > /var/trtrtr");
					choicetype=nvram_bufget(RT2860_NVRAM, "dial3gchoicetype");
					if(choicetype && !strncmp(choicetype,"MANUAL",strlen("MANUAL"))) //MANUAL
					{
						if(ptmp=nvram_bufget(RT2860_NVRAM, "dial3gapn"))
							strcpy(dial3g_apn,ptmp);
						else
							dial3g_apn[0] = '\0';

						if(ptmp=nvram_bufget(RT2860_NVRAM, "dial3gusername"))
							strcpy(dial3g_username,ptmp);
						else
							dial3g_username[0] = '\0';

						if(ptmp=nvram_bufget(RT2860_NVRAM, "dial3gpassword"))
							strcpy(dial3g_passwd,ptmp);
						else
							dial3g_passwd[0] = '\0';

						if(ptmp=nvram_bufget(RT2860_NVRAM, "dial3gnum"))
							strcpy(dial3g_num,ptmp);
						else
							dial3g_num[0] = '\0';
					}
					else //AUTO
					{
						if(g_EV888U)
						{
							strcpy(dial3g_apn,"");
							strcpy(dial3g_num,"#777");
							strcpy(dial3g_username,"card");
							strcpy(dial3g_passwd,"card");
						}
						else
						{
							if(File_Get_Modem_Info("dial3gapn_auto",dial3g_apn,32)<0)
								dial3g_apn[0] = '\0';

							if(File_Get_Modem_Info("dial3gusername_auto",dial3g_username,32)<0)
								dial3g_username[0] = '\0';

							if(File_Get_Modem_Info("dial3gpassword_auto",dial3g_passwd, 32)<0)
								dial3g_passwd[0] = '\0';

							if(File_Get_Modem_Info("dial3gnum_auto",dial3g_num, 32)<0)
								dial3g_num[0] = '\0';
							if(File_Get_Modem_Info("usbdevname",trueusbdevpath, 32)<0)
								trueusbdevpath[0] = '\0'; //by luot
						}
					}

					printf("\r\nwite %s %s %s",dial3g_username,dial3g_passwd,dial3g_num);

					WriteWcdmaApn(fdd,"/var/ttyACM0", dial3g_apn);
					close(fdd);
					fdd = 0;
				}						
			}

			fp = fopen(PPP_3G_CONN_SCR, "w");
			if(fp == NULL)
				return;

			if(g_TD_DT901||g_TD_MU350||g_TD_LC5730)
			{
				fprintf ( fp, "opengt\n");
				fprintf ( fp, "set com 115200n81\n");						
				fprintf ( fp, "set senddelay 0.05\n");						
				fprintf ( fp, "waitquiet 1 0.2\n");						
				//fprintf ( fp, "send \"ATZ^m\"\n");						
				//fprintf ( fp, "waitfor 3 \"OK\",\"ERR\",\"ERROR\"\n");						
				//fprintf ( fp, "if %% = -1 goto timeerror\n");						
				//fprintf ( fp, "if %% = 0 goto next2\n");						
				//fprintf ( fp, "if %% = 1 goto error\n");						
				//fprintf ( fp, "if %% = 2 goto error\n");
				//fprintf ( fp, ":next2\n");						
				//fprintf ( fp, "send \"ATQ0V1E0S0=0^m\"\n");						
				//fprintf ( fp, "waitfor 3 \"OK\",\"ERR\",\"ERROR\"\n");						
				//fprintf ( fp, "if %% = -1 goto timeerror\n");						
				//fprintf ( fp, "if %% = 0 goto next3\n");						
				//fprintf ( fp, "if %% = 1 goto next3\n");						
				//fprintf ( fp, "if %% = 2 goto next3\n");
				//fprintf ( fp, ":next3\n");					
				fprintf ( fp, "send \"ATD%s^m\"\n",dial3g_num);
				fprintf ( fp, "waitfor 10 \"CONNECT\",\"ERR\",\"ERROR\"\n");					
				fprintf ( fp, "if %% = -1 goto timeerror\n");						
				fprintf ( fp, "if %% = 0 goto next4\n");						
				fprintf ( fp, "if %% = 1 goto error\n");						
				fprintf ( fp, "if %% = 2 goto error\n");
				fprintf ( fp, ":next4\n");
				fprintf ( fp, "print \"CONNECTED\\n\"\n");
				fprintf ( fp, "exit 0\n");
				fprintf ( fp, ":error\n");
				fprintf ( fp, "print \"CONNECT ERROR\\n\"\n");
				fprintf ( fp, "exit 1\n");
				fprintf ( fp, ":timeerror\n");
				fprintf ( fp, "print \"CONNECT TIMEOUT\\n\"\n");
				fprintf ( fp, "exit 1\n");
				fclose(fp);

				eval("comgt","-d","/var/ttyACM0","-s",PPP_3G_CONN_SCR);
				//sleep(5);
				sleep(1);
			}	
			else if(g_TD_LC5740)
			{
				fprintf ( fp, "ABORT\t'BUSY'\n");
				fprintf ( fp, "ABORT\t'NO CARRIER'\n");
				fprintf ( fp, "ABORT\t'ERROR'\n" );
				fprintf ( fp, "ABORT\t'+CME ERROR:100'\n" );
				fprintf( fp,  "\"\"\tAT\n");
				fprintf ( fp, "OK\tATDT%s\n", dial3g_num);
				fprintf ( fp, "CONNECT\t''\n");
				fclose(fp);
			}
			else
			{
				fprintf ( fp, "TIMEOUT 9\n");
				fprintf ( fp, "ABORT	\t'\\nBUSY\\r'\n");
				fprintf ( fp, "ABORT\t'\\nNO ANSWER\\r'\n");
				fprintf ( fp, "ABORT	\t'\\nRINGING\\r\\n\\r\\nRINGING\\r'\n" );
				fprintf( fp, "''\rATZ\n");
				fprintf( fp, "OK 'ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0'\n");
				fprintf ( fp, "TIMEOUT 5\n");
				fprintf ( fp, "OK\tATD%s\n", dial3g_num);
				fprintf ( fp, "CONNECT\t''\n");
				fclose(fp);
			}

			if(!g_TD_DT901 && !g_TD_MU350 &&!g_TD_LC5730)
			{
				fp = fopen(PPP_3G_FILE, "w");
				if(fp == NULL)
					return ;
				if(g_MC8780)
				{
					fprintf ( fp, "/dev/ttyUSB0\n");
				}
				else
					fprintf ( fp, "%s\n",trueusbdevpath);
			#if 0
				//fprintf ( fp, "crtscts\n");
				fprintf ( fp, "nocrtscts\n");
				fprintf ( fp, "115200\n");
				fprintf ( fp, "-detach\n" );
				fprintf ( fp, "-ccp\n" );
				fprintf ( fp, "-vj\n" );
				fprintf ( fp, "#lock\n" );
				fprintf ( fp, "debug\n" );
				fprintf ( fp, "modem\n" );
				fprintf ( fp, "asyncmap 0\n");
				fprintf ( fp, "defaultroute\n");
				fprintf ( fp, "usepeerdns\n");
				fprintf ( fp, "noauth\n");
				fprintf ( fp, "nomagic\n");
				fprintf ( fp, "proxyarp\n");
				fprintf ( fp, "noendpoint\n");
				fprintf ( fp, "ipcp-accept-remote\n");
				fprintf ( fp, "ipcp-accept-local\n");
				fprintf ( fp, "lcp-echo-interval 30\n");
			#endif
				fprintf ( fp,"115200\n");
				fprintf ( fp,"nocrtscts\n");
				fprintf ( fp,"nocdtrcts\n");
				fprintf ( fp,"local\n");
				fprintf ( fp,"usepeerdns\n");
				fprintf ( fp,"defaultroute\n");
				fprintf ( fp,"noipdefault\n");
				fprintf ( fp,"ipcp-accept-local\n");
				fprintf ( fp,"ipcp-accept-remote\n");
	
				fprintf ( fp,"lock\n");
				fprintf ( fp,"debug\n");
				fprintf ( fp,"kdebug 7\n");
				fprintf ( fp,"logfile /tmp/ppplog11\n");
			
			//zhubo add 2008.12.25
				//fprintf(fp,"-chap\n");
				//fprintf(fp,"ipcp-max-failure 30\n");

				if(dial3g_username&&strlen(dial3g_username))
					fprintf ( fp, "user %s\n", dial3g_username);
				else
					fprintf ( fp, "user card\n");

				if(dial3g_passwd&&strlen(dial3g_passwd))
					fprintf ( fp, "password %s\n", dial3g_passwd);
				else
					fprintf ( fp, "password card\n");

				fprintf ( fp, "connect 'chat -Ss -f %s'\n",PPP_3G_CONN_SCR);
				fclose(fp);
			}
			else
			{
				fp = fopen(PPP_3G_FILE, "w");
				if(fp == NULL)
					return ;

				//fprintf ( fp, "/var/ttyACM0\n");
				//fprintf ( fp, "/dev/ttyUSB0\n");
				fprintf ( fp, "%s\n",trueusbdevpath);
			#if 1
				fprintf ( fp, "crtscts\n");
				//fprintf ( fp, "nocrtscts\n");
				fprintf ( fp, "115200\n");
				fprintf ( fp, "-detach\n" );
				fprintf ( fp, "-ccp\n" );
				fprintf ( fp, "-vj\n" );
				fprintf ( fp, "#lock\n" );
				fprintf ( fp, "debug\n" );
				fprintf ( fp, "modem\n" );
				fprintf ( fp, "asyncmap 0\n");
				fprintf ( fp, "defaultroute\n");
				fprintf ( fp, "usepeerdns\n");
				fprintf ( fp,"noipdefault\n"); //by luot
				fprintf ( fp, "noauth\n");
				fprintf ( fp, "nomagic\n");
				//fprintf ( fp, "proxyarp\n");
				fprintf ( fp, "noendpoint\n");
			#endif
				fprintf ( fp, "lcp-echo-interval 10\n");

				if(strlen(dial3g_username))
					fprintf ( fp, "user %s\n", dial3g_username);
				else
					fprintf ( fp, "user card\n");

				if(strlen(dial3g_passwd))
					fprintf ( fp, "password %s\n", dial3g_passwd);
				else
					fprintf ( fp, "password card\n");

				//fprintf ( fp, "connect \"/bin/comgt -d /var/ttyACM0 -s %s\"\n",PPP_3G_CONN_SCR);
				//fprintf ( fp, "disconnect \"/bin/comgt -d /dev/ttyUSB0 -s %s\"\n",PPP_3G_DISCON_SCR);
				fclose(fp);
			}			

			fp = fopen (PPP_PAP_FILE, "wt" );
			if ( !fp ) 
			{
				return;
			}

			if(dial3g_username&&strlen(dial3g_username))
				fprintf( fp, "\"%s\"\t*\t\"%s\"", dial3g_username, dial3g_passwd);
			else
				fprintf( fp, "\"card\"\t*\t\"card\"");

	       	fclose(fp);

			fp = fopen (PPP_CHAP_FILE, "wt" );
			if ( !fp )  
			{
				return;
			}

			if(dial3g_username&&strlen(dial3g_username))
					fprintf( fp, "\"%s\"\t*\t\"%s\"", dial3g_username, dial3g_passwd);
			else
					fprintf( fp, "\"card\"\t*\t\"card\"");

	       	fclose(fp);

			stsNew = IFC_STATUS_DOWN;

			#ifdef NOTDDIAL
				if(!strcmp(g_operation,"CHINA MOBILE"))
					goto done;

				//gpio_set_led(GPIO_PORT_3G, 1, 1, 1, 0, 4000);
			#endif			

			if(dial_style==0) //自动
			{
				if(discon_flag)//断开
				{
					//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
					//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
					goto done;
				}
					
				com_discon = 0;
				CDMADial();
			}
			else//手动
			{
				//gpio_set_led(GPIO_PORT_3G, 0, 4000, 0, 0, 4000);
				//gpio_set_led(GPIO_3G_LED2, 0, 4000, 0, 0, 4000);
				if(con_flag)
				{
					com_discon = 0;
					CDMADial();
				}					
			}
		}
		else if(ret == _CONNECT)
		{
			stsNew = IFC_STATUS_UP;
		}
done:	
		usleep(50000);
	}	
	return 0;
}
