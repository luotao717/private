
/*  usb hotplug daemon for 3g and storage  */
/*  by luot yucosta@sohu.com               */
/*  ver 1.0: supprot 3g 20130901           */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include <linux/types.h>
#include "musb_3gmodem.h"

/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})

int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);
int mc8780_flag;

#define FAILURE			0
#define SUCCESS			1

#define DEV_NONE		0
#define	DEV_ADD			1	
#define	DEV_REMOVE	2

#define BUSYWAIT(x)	{while(x){;}}

enum function
{
	FUNC_NONE=0,
	FUNC_SCSI,
	FUNC_TTY,
	FUNC_MAX
};

#define DEF_DEBUG 1
#define UEVENT_BUFFER_SIZE 2048

#if DEF_DEBUG
#define DEBUG_PRINTF	printf
#else
#define DEBUG_PRINTF(format,args...)
#endif

unsigned char        *devname_other= "/dev/ttyACM0"; 
unsigned char        *devname_usb0 = "/dev/ttyUSB0";  
unsigned char		*devname_usb1 = "/dev/ttyUSB1"; 
unsigned char		*devname_usb2 = "/dev/ttyUSB2";  
unsigned char        *devname_usb3 = "/dev/ttyUSB3"; 
unsigned char        *devname_usb5 = "/dev/ttyUSB5";
unsigned char        *devname_usb9 = "/dev/ttyUSB9";
unsigned char        *devname_hs0  = "/dev/ttyHS0"; 
unsigned char		*devname_hs1  = "/dev/ttyHS1";
unsigned char 	*devname_hs2  = "/dev/ttyHS2";  

static unsigned char 	rddevbuf[256];

int    g_simflag = -1;
int 	g_huawei_modem = 0;     
int 	g_zte_modem = 0;        //ZTEmodem
int	g_longqi_modem = 0;    //longqi Modem
int	g_option_modem = 0;    //option Modem
int 	g_sim_modem=0;
int 	g_DT_modem=0;
int	g_TD_LC631=0; 
int	g_TD_DT901=0;
int   g_MD300_MODEM=0;
int 	g_DO_AC580=0;
int	g_TD_MU350=0;
unsigned char   g_str_vendor[8]={0},g_str_product[8]={0};
int Modem_usbtype = -1;
int Modem_AT_Port = -1; //modem at command

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
int _eval(char *const argv[], char *path, int timeout, int *ppid)
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
	
	switch (pid) {
	
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
		if ((fd = open("/dev/console", O_RDWR)) < 0) {
        	(void) open("/dev/null", O_RDONLY);
            (void) open("/dev/null", O_WRONLY);
            (void) open("/dev/null", O_WRONLY);
		}
		else{
			close(fd);
			(void) open("/dev/console", O_RDONLY);
            (void) open("/dev/console", O_WRONLY);
            (void) open("/dev/console", O_WRONLY);
		}

		/* Redirect stdout to <path> */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
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
		if (ppid) {
			*ppid = pid;
			
			return 0;
		} else {
			waitpid(pid, &status, 0);

			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

extern char **environ;
int bcmSystem (char *command) {
   int pid = 0, status = 0;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      printf("app: %s\r\n", command);
      execve("/bin/sh", argv, environ);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;
}

void convert_str(char *pDest, char *pSrt)
{
	int len = 0;

	len = strlen(pSrt);

	switch (len)
	{
		case 1:
			sprintf(pDest,"000%s",pSrt);
			break;
		case 2:
			sprintf(pDest,"00%s",pSrt);
			break;
		case 3:
			sprintf(pDest,"0%s",pSrt);
			break;
		case 4:
			sprintf(pDest,"%s",pSrt);
			break;
		default:
			break;
	}
		
}

int File_Set_Modem_Info(char *name,char *value)
{
	FILE *fp=NULL;
	char path[64];

	memset(path,'\0',64);
	sprintf(path,"%s%s",USBMODEMPATH,name);

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


int find_profile(char *profile)
{
	FILE *fp;
	char buffer[128];
	int sys_ret = 0;
	printf("\r\nfilefff=%s",profile);
	fp = popen("ls -a /etc_ro/usb_modeswitch.d", "r");
	while (NULL != fgets(buffer, sizeof(buffer),fp))
	{
		printf("\r\nhh--buf=%s",buffer);
		if (strstr(buffer, profile))
		{
			printf("\r\nyes--\r\n");
			sprintf(buffer, "cp /etc_ro/usb_modeswitch.d/%s /var/usb_modeswitch.conf", profile);
			sys_ret = system(buffer);
			pclose(fp);
			return SUCCESS;
		}
	}
	pclose(fp);
	return FAILURE;
}


int get_tty(char *ttyitf)
{
	FILE   *fp;
	char   buffer[64];
	int    sys_ret = 0;

	sys_ret = system("ls /sys/bus/usb-serial/devices > /var/tty-devices");
	fp = fopen("/var/tty-devices", "r");
	if (fp !=NULL && NULL != fgets(buffer, sizeof(buffer),fp)) 
	{
		char *ptr = strstr(buffer, " ");
		if (ptr != NULL)
			return FAILURE;

		ptr = '\0';
		strcpy(ttyitf, buffer);
		DEBUG_PRINTF("[%s:%s:%d] first one is /dev/%s\n", __FILE__, __FUNCTION__, __LINE__, ttyitf);
		sys_ret = system("rm /var/tty-devices >/dev/null 2>&1");
		fclose(fp);

		return SUCCESS;
	}

	sys_ret = system("rm /var/tty-devices >/dev/null 2>&1");
	fclose(fp);
	return FAILURE;
}

void Get_Usb_Info()
{
	int     fd_proc=-1,size=0,i=0;
	char		buf[2*1024],*str=NULL;

	memset(g_str_vendor,'\0',8);
	memset(g_str_product,'\0',8);

	fd_proc = open("/proc/bus/usb/devices",O_RDWR|O_NONBLOCK);
	if(fd_proc<=0)
	{
   		DEBUG_PRINTF("Get_Usb_Info open /proc/bus/usb/devices error!\n");

		return ;
	}
	size = read(fd_proc,buf,2*1024);
	close(fd_proc);
	if(size<=0)
		return ;
	str = buf;
	while(str)
	{
		str = strstr(str,"Cls=");
		if(str)	
		{
			str += 4;
			if((*str++=='0')&&(*str=='9'))//è®¾å¤‡ç±»åž‹ä¸º09,å³æ˜¯hub,å¿½ç•¥,ç»§ç»­æŸ¥æ‰¾
				 continue;
		}
		else
			break;
		
		str = strstr(str,"Vendor=");
		if(str)
		{
			str += 7;
			i = 0;
			while((*str != ' ')&&(i<4))
			{
				g_str_vendor[i] = *str;
				str++;
				i++;
			}
			g_str_vendor[i] = '\0';
			
			if(strncmp(g_str_vendor,"0000",4)==0)
				continue;
			str = strstr(str,"ProdID=");
			if(str)
			{
				str += 7;
				i = 0;
				while((*str != ' ')&&(i<4))
				{
					g_str_product[i] = *str;
					str++;
					i++;
				}
				g_str_product[i] = '\0';

				if(strncmp(g_str_product,"0000",4)==0)
					continue;
				else
				{
					str = NULL;
					break;
				}					
			}
		}
	}	
}

void Eject_Operate(unsigned char *pid)
{
	int 	i=0;
	unsigned char tmppid[8];

	strcpy(tmppid,pid);
	bcmSystem("eject /dev/sr0");//PID will be changed after eject
	sleep(3);
	
	for(i=0; i<16; i++)//James 2008-12-30
	{
		Get_Usb_Info();
		
		DEBUG_PRINTF("Eject_Operate vendor,product i is %s,%s,%d!\n",g_str_vendor,g_str_product,i);
				
		if((strlen(g_str_vendor) != 0)&&(strncmp(g_str_product, tmppid, 4)!= 0))
			break;
		
		sleep(1);
	}

	return;
}

int make_visual_ppp_at_port(char *truetty)
{
	switch(Modem_usbtype)
	{
		case ACM0_TYPE:
			eval("ln","-s",STR_ACM0,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_ACM0); //by luot
			break;
		case USB0_TYPE:
			eval("ln","-s",STR_USB0,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_USB0);
			break;
		case USB1_TYPE:
			eval("ln","-s",STR_USB1,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_USB1);
			break;	
		case USB2_TYPE:
			if(!g_DO_AC580)
			{
				eval("ln","-s",STR_USB2,STR_IN_FILE);
				File_Set_Modem_Info("usbdevname", STR_USB2);
			}
			else
			{
				eval("ln","-s",STR_USB0,STR_IN_FILE);
				File_Set_Modem_Info("usbdevname", STR_USB0);
			}
			break;
		case USB3_TYPE:
			if(!g_TD_LC631)
			{
				eval("ln","-s",STR_USB3,STR_IN_FILE);
				File_Set_Modem_Info("usbdevname", STR_USB3);
			}
			else	
			{
				eval("ln","-s",STR_USB0,STR_IN_FILE);//
				File_Set_Modem_Info("usbdevname", STR_USB0);
			}
			break;				
		case USB5_TYPE:
			eval("ln","-s",STR_USB5,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_USB5);
			break;		
		case USB9_TYPE:
			eval("ln","-s",STR_USB9,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_USB9);
			break;	
		case HS1_TYPE:
			//eval("ln","-s",STR_HS0,STR_IN_FILE);
			break;
		case ACM2_TYPE:
			eval("ln","-s",STR_ACM2,STR_IN_FILE);
			File_Set_Modem_Info("usbdevname", STR_ACM2);
			break;
		default:
			eval("ln","-s",truetty,STR_IN_FILE);
			break;
	}

	if(strncmp(g_str_vendor,"1782",4)!=0&&strncmp(g_str_product,"3d00",4)!=0)
	{
		switch(Modem_AT_Port)
		{
			case ACM0_TYPE:
				eval("ln","-s",STR_ACM0,"/var/ttyAT");
				break;
			case ACM1_TYPE:
				eval("ln","-s",STR_ACM1,"/var/ttyAT");
				break;
			case ACM2_TYPE:
				eval("ln","-s",STR_ACM2,"/var/ttyAT");
				break;
			case USB0_TYPE:
				eval("ln","-s",STR_USB0,"/var/ttyAT");
				break;	
			case USB1_TYPE:
				eval("ln","-s",STR_USB1,"/var/ttyAT");
				break;
			case USB2_TYPE:
				eval("ln","-s",STR_USB2,"/var/ttyAT");
				break;
			case USB3_TYPE:
				eval("ln","-s",STR_USB3,"/var/ttyAT");
				break;	
			default:
				break;
		}
	}

	if(strncmp(g_str_vendor,"1782",4)!=0&&strncmp(g_str_product,"4d00",4)!=0)
	{
		switch(Modem_AT_Port)
		{
			case ACM0_TYPE:
				eval("ln","-s",STR_ACM0,"/var/ttyAT");
				break;
			case ACM1_TYPE:
				eval("ln","-s",STR_ACM1,"/var/ttyAT");
				break;
			case ACM2_TYPE:
				eval("ln","-s",STR_ACM2,"/var/ttyAT");
				break;
			case USB0_TYPE:
				eval("ln","-s",STR_USB0,"/var/ttyAT");
				break;	
			case USB1_TYPE:
				eval("ln","-s",STR_USB1,"/var/ttyAT");
				break;
			case USB2_TYPE:
				eval("ln","-s",STR_USB2,"/var/ttyAT");
				break;
			case USB3_TYPE:
				eval("ln","-s",STR_USB3,"/var/ttyAT");
				break;	
			default:
				break;
		}
	}	
	return 1;
}

#if 1

int get_3gkard_info_tty()
{
	int 		fd=-1;
	char		cmdstr[100], usbinfo[32];

	memset(usbinfo,'\0',32);
	sleep(1);

  	if(strncmp(g_str_vendor, "19d2",4) == 0 ) //ZTE Modem 
	{					
		if((strncmp(g_str_product,"0057",4)==0) || (strncmp(g_str_product,"0039",4)==0))
		{
			//Eject_Operate(g_str_product);
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"0039",4)==0) //MF637U
		{
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"0031",4)==0) //MF637U
		{
			Modem_usbtype = USB3_TYPE;
		}
		else if(strncmp(g_str_product,"0015",4)==0) //MF628
		{
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"0037",4)==0)
		{
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"0016",4)==0)
		{
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"0003",4)==0)//add 2009.05.14 TD-MU350 usb modeswitch have it. bu we use our private function
		{
			g_TD_MU350 = 1;
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","mu350mmodem");
		}
		else if(strncmp(g_str_product,"fff1",4)==0) //zhubo add 2009.04.18
		{
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"0034",4)==0)
		{
			//Modem_usbtype = ACM0_TYPE;
			Modem_usbtype = ACM2_TYPE;
			File_Set_Modem_Info("modemtype","TDA356");
		}
		else if(strncmp(g_str_product,"0079",4)==0) //A356 TD
		{
			Modem_usbtype = USB0_TYPE;	
			File_Set_Modem_Info("modemtype","901mmodem");
		}
		else if(strncmp(g_str_product,"0094",4)==0) //AC580 EVD0
		{
			Modem_usbtype = USB2_TYPE;
			g_DO_AC580 = 1;			
		}
		else if(strncmp(g_str_product,"2002",4)==0)//K3765Z
		{
			Modem_usbtype = USB3_TYPE;
		}
		else if(strncmp(g_str_product,"2003",4)==0)//MF627
		{
			Modem_usbtype = USB3_TYPE;
		}
		else if(strncmp(g_str_product,"0063",4)==0)//K3565Z
		{
			Modem_usbtype = USB3_TYPE;
		}
		else if(strncmp(g_str_product,"ffeb",4)==0)//MC2716
		{
			Modem_usbtype = USB3_TYPE;
			Modem_AT_Port = USB0_TYPE;
			File_Set_Modem_Info("modemtype","ad3812modem");
		}
		else if(strncmp(g_str_product,"1303",4)==0)//MU301
		{
			Modem_usbtype = USB0_TYPE;
			sleep(2);
			File_Set_Modem_Info("modemtype","ztemu301");
		}
		else if(strncmp(g_str_product,"ffed",4)==0)//MC2716
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB1_TYPE;

			File_Set_Modem_Info("modemtype","MC2716");
		}
		else if(strncmp(g_str_product,"0117",4)==0)	//MF200
		{
			Modem_usbtype = USB2_TYPE;
			File_Set_Modem_Info("modemtype","901mmodem");
		}
		else//the above two guys are ttyUSB2, the others are ttyUSB0
		{
			Modem_usbtype = USB2_TYPE;
		}
	}
  	else if(strncmp(g_str_vendor,"1ed1",4)==0)
	{
		Modem_usbtype = USB0_TYPE;
	}	
	else if(strncmp(g_str_vendor,"12d1",4)==0 || strncmp(g_str_vendor,"1614",4)==0)//»ªÎªÏÄÐÂ
	{
		
		if(strncmp(g_str_product,"14ac",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			sleep(2);
		}
		else if(strncmp(g_str_product,"140c",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1404",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;

			File_Set_Modem_Info("modemtype","EM770");
		}
		else if(strncmp(g_str_product,"1506",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
		}
		else
		{
			g_huawei_modem = 1;//Regard all of them as HuaWei Modem
			Modem_usbtype = USB0_TYPE;
		}		
	}	
	else if(strncmp(g_str_vendor,"05c6",4)==0)//SIM Modem must after inject can use ttyACM0
	{
		if(strncmp(g_str_product,"6000",4)==0)//EVDO on 2009-03-07
		{
			//Modem_usbtype = USB1_TYPE;
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"9003",4)==0)//EVDO on 2009-03-07
		{

			Modem_usbtype = USB3_TYPE;
			Modem_AT_Port = USB2_TYPE;
			File_Set_Modem_Info("modemtype","QUNKEWCDMA");
		}
		else if(strncmp(g_str_product,"0015",4)==0)
		{
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"00a0",4)==0)
		{
			Modem_usbtype = USB0_TYPE;			
		}
		else
		{
			g_simflag = 1;
			Modem_usbtype = ACM0_TYPE;	
		}
	}
	else if(strncmp(g_str_vendor,"04e8",4)==0)
	{
		if(strncmp(g_str_product,"6772",4)==0)
		{
			sleep(2);
			Modem_usbtype = ACM0_TYPE;
			File_Set_Modem_Info("modemtype","gty3200");
		}			
	}
	
	else if(strncmp(g_str_vendor,"1d09",4)==0)
	{
		if(strncmp(g_str_product,"aef4",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			sleep(2);
 		}
	}
	else if(strncmp(g_str_vendor,"1519",4)==0)
	{
		if(strncmp(g_str_product,"0020",4)==0)
		{
			Modem_usbtype = ACM0_TYPE;
 		}
	}
	else if(strncmp(g_str_vendor,"15eb",4)==0)
	{
		if(strncmp(g_str_product,"7153",4)==0)
		{
				sleep(2);
				Modem_usbtype = USB2_TYPE;
			}
			else if(strncmp(g_str_product,"7152",4)==0)
			{
				sleep(2);
				Modem_usbtype = USB2_TYPE;
			}
			else if(strncmp(g_str_product,"0001",4)==0)
			{
				sleep(2);
				Modem_usbtype = USB0_TYPE;
			}else if(strncmp(g_str_product,"1231",4)==0)
			{
				sleep(1);
				Modem_usbtype = USB0_TYPE;
			}
		else
		{
			Modem_usbtype = USB0_TYPE;
		}			
	}
#if 1		
	else if(strncmp(g_str_vendor,"1ed1",4)==0)
	{
		Modem_usbtype = USB0_TYPE;
	}
	
	
	else if(strncmp(g_str_vendor,"1ab7",4)==0)//DT Modem James 2009-02-05
	{
		if(strncmp(g_str_product,"5730",4)==0)
		{

			Modem_usbtype = USB1_TYPE;
			Modem_AT_Port = USB0_TYPE;
			File_Set_Modem_Info("modemtype","LC5730");
			//File_Set_Modem_Info("modemtype","901mmodem");
		}
		else if(strncmp(g_str_product,"5740",4)==0)
		{

			Modem_usbtype = USB1_TYPE;
			Modem_AT_Port = USB3_TYPE;
			File_Set_Modem_Info("modemtype","LC5740");
			//File_Set_Modem_Info("modemtype","dtmmodem");
		}
	}
	else if(strncmp(g_str_vendor,"04cc",4)==0) //TD Modem
	{
			if(strncmp(g_str_product,"226e",4)==0)
			{

				Modem_usbtype = ACM0_TYPE;
				Modem_AT_Port = ACM2_TYPE;
				File_Set_Modem_Info("modemtype","dtmmodem");
			}
			if(strncmp(g_str_product,"2263",4)==0)
			{

				Modem_usbtype = ACM0_TYPE;
				Modem_AT_Port = ACM1_TYPE;
				File_Set_Modem_Info("modemtype","dtmmodem");
			}
			/*
			if(strncmp(g_str_product,"2259",4)==0)
			{

				Modem_usbtype = ACM0_TYPE;
				Modem_AT_Port = ACM1_TYPE;
				File_Set_Modem_Info("modemtype","dtmmodem");
			}
			*/
			else
			{

			}			
	}
	else if(strncmp(g_str_vendor,"1c9e",4)==0)//longqi modem
	{
		
		if(strncmp(g_str_product,"9e00",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"6061",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
		}
		else
		{
			g_longqi_modem = 1;//longqi modem,éœ€è¦æ“ä½œttyUSB2è®¾å¤‡
			Modem_usbtype = USB2_TYPE;
		}		
	}	
	else if(strncmp(g_str_vendor,"0af0",4)==0)//option Modem,¸ÃÄ£¿é±È½ÏÌØÊâ
	{
		
		if(strncmp(g_str_product,"7211",4)==0)
		{
			/*
			if(mc8780_flag==0)
			{
				eval("rmmod", "option");
				sleep(1);
				eval("rmmod", "usbserial");
				sleep(1);
				eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x7211", "-R", "1");
				sleep(4);
				eval("insmod", "usbserial", "vendor=0x0af0", "product=0x7211");
				sleep(4);
				mc8780_flag=1;
			}
			Modem_usbtype	= USB9_TYPE;
			*/
			Modem_usbtype	= USB2_TYPE;
			sleep(2);
			File_Set_Modem_Info("modemtype","GTM380");
		}
	}
	else if(strncmp(g_str_vendor,"1e89",4)==0)
	{
		if(strncmp(g_str_product,"1e16",4)==0)
		{
			//eval("usb_modeswitch", "-v", "0x1e89", "-p", "0x1e16", "-R", "1");
			//sleep(10);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1a20",4)==0)
		{
			Modem_usbtype = USB3_TYPE;
		}
	}
	
	else if(strncmp(g_str_vendor,"1782",4)==0)
	{
		if(strncmp(g_str_product,"4d00",4)==0)
		{
			sleep(6);
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","901mmodem");  
		}
		else if(strncmp(g_str_product,"3d00",4)==0)
		{
			sleep(2);
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","901mmodem");  //dtmmodem
		}
		else if(strncmp(g_str_product,"3d01",4)==0)
			Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"0685",4)==0)
	{
		if(strncmp(g_str_product,"6001",4)==0)
			Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"028a",4)==0)
	{
		if(strncmp(g_str_product,"1006",4)==0)
		{
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}			
	}
	else if(strncmp(g_str_vendor,"8848",4)==0)
	{
		if(strncmp(g_str_product,"3721",4)==0)
		{
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}			
	}
	else if(strncmp(g_str_vendor,"04e8",4)==0)
	{
		if(strncmp(g_str_product,"6772",4)==0)
		{
			sleep(2);
			Modem_usbtype = ACM0_TYPE;
			File_Set_Modem_Info("modemtype","dtmmodem");
		}			
	}
	else if(strncmp(g_str_vendor,"20a6",4)==0)
	{
		if(strncmp(g_str_product,"1105",4)==0)
		{
			sleep(2);
			Modem_usbtype = USB0_TYPE;
		}			
	}
	else if(strncmp(g_str_vendor,"0bdb",4)==0)
	{
		if(strncmp(g_str_product,"1910",4)==0)
		{
			sleep(2);
			Modem_usbtype = ACM0_TYPE;
		}
		else if(strncmp(g_str_product,"190d",4)==0)
		{
			Modem_usbtype = ACM0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"0930",4)==0)
	{
		if(strncmp(g_str_product,"1302",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
		}
	}
	else //default
	{
	}
#endif	
	sprintf(usbinfo,"0x%s/0x%s",g_str_vendor,g_str_product);
	File_Set_Modem_Info("modeminfo", usbinfo);
	return 1;
}

#endif

#if 1

int Mount_OurDefine_Usb_Module()
{
	int 		fd=-1;
	char		cmdstr[100], usbinfo[32];

	memset(usbinfo,'\0',32);
	sleep(1);

	if( (strncmp(g_str_vendor,"0000",4)==0) || (strncmp(g_str_product,"0000",4)==0) || (strlen(g_str_vendor)==0) || (strlen(g_str_product)==0))
	{
		return FAILURE;
	}
  	if(strncmp(g_str_vendor, "19d2",4) == 0 ) //ZTE Modem 
	{	
		
		if(strncmp(g_str_product,"2000",4)==0)//usb modeswitch have it. bu we use our private function
		{	
			Eject_Operate(g_str_product);
			sleep(2);
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"0002",4)==0)//add 2009.02.02 MF628(ONDA)
		{
			Eject_Operate(g_str_product);
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"fff5",4)==0) //zhubo add 2009.04.18
		{			
			eval("usb_modeswitch","-v","0x19d2","-p","0xfff5","-V","0x19d2","-P","0xfff1","-M","5553424312345678c00000008000069f030000000000000000000000000000");
			sleep(3);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"0026",4)==0) //AC580 EVD0
		{
			sleep(6);
			Eject_Operate(g_str_product);
			Modem_usbtype = USB2_TYPE;
			g_DO_AC580 = 1;			
		}
		else if(strncmp(g_str_product,"0090",4)==0) //MU330 TD
		{
			Eject_Operate(g_str_product);
			//eval("rmmod","usb-storage.ko");
			Modem_usbtype = ACM0_TYPE;
			File_Set_Modem_Info("modemtype","TDA356");			
		}
		else if(strncmp(g_str_product,"0120",4)==0) //A356 TD
		{
			sleep(6);
			Eject_Operate(g_str_product);
			Modem_usbtype = USB0_TYPE;	
			File_Set_Modem_Info("modemtype","TDA356");
		}
		else if(strncmp(g_str_product,"0053",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0053", "-M", "55534243123456782000000080000c85010101180101010101000000000000");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
		}
		else//the above two guys are ttyUSB2, the others are ttyUSB0
		{
			g_zte_modem=1;
		}
	}
	else if(strncmp(g_str_vendor,"0421",4)==0)//NOKIA zhubo add 2009.10.19 
	{
		Eject_Operate(g_str_product);
		Modem_usbtype = ACM0_TYPE;
	}
	else if(strncmp(g_str_vendor,"12d1",4)==0 || strncmp(g_str_vendor,"1614",4)==0)//»ªÎªÏÄÐÂ
	{
		if(strncmp(g_str_product,"1da1",4)==0)
		{
			eval("usb_modeswitch","-v","0x12d1","-p","0x1da1","-H","1");
			sleep(6);
			Modem_usbtype = ACM0_TYPE;
		}
		else if(strncmp(g_str_product,"1003",4)==0)   //new add 
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1003", "-H", "1", "-R", "2");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1521",4)==0) //new add K4505
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1521", "-V", "0x12d1", "-P", "0x1464", "-M", "55534243123456780000000000000011060000000000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1411",4)==0) //new add K4505
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1411", "-H", "1");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1412",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1412", "-H", "1");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1414",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1414", "-H", "1");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1446",4)==0)
		{
			//eval("usb_modeswitch","-v","0x12d1","-p","0x1446","-V","0x12d1","-P","0x1001","-M","55534243000000000000000000000011060000000000000000000000000000","-s","5");
			//eval("usb_modeswitch","-v","0x12d1","-p","0x1446","-V","0x12d1","-P","0x1001","-M","55534243000000000000000000000011060000000000000000000000000000");
			eval("usb_modeswitch","-v","0x12d1","-p","0x1446","-V","0x12d1","-P","0x14ac","-M","55534243123456780000000000000011060000000000000000000000000000");
			sleep(8);
			Modem_usbtype = USB0_TYPE;		
		}
		else if(strncmp(g_str_product,"1464",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1464", "-V", "0x12d1", "-P", "0x1465", "-H", "1");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"140b",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x140b", "-H", "1");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1520",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1520", "-V", "0x12d1", "-P", "0x1465", "-M", "55534243123456780000000000000011060000000000000000000000000000");
			sleep(6);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1505",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1505", "-V", "0x12d1", "-P", "0x140c", "-M", "55534243123456780000000000000011062000000100000000000000000000");
			sleep(6);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1001",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1001", "-H", "1");
			sleep(6);
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"1003",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x1003", "-H", "1");
			sleep(8);
			Modem_usbtype = USB0_TYPE;
		}
		else if (strncmp(g_str_product,"14fe",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
			
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x14fe", "-V", "0x12d1", "-P", "0x1506" ,"-M" ,"55534243123456780000000000000011062000000100000000000000000000");
			sleep(8);
		}
		else
		{
			g_huawei_modem = 1;//Regard all of them as HuaWei Modem
		}		
	}	
	else if(strncmp(g_str_vendor,"05c6",4)==0)//SIM Modem must after inject can use ttyACM0
	{
		if(strncmp(g_str_product,"f000",4)==0)//PHu9 on 2009-02-25
		{
			Eject_Operate(g_str_product);
			Modem_usbtype = USB2_TYPE;
		}
		/*
		else if(strncmp(g_str_product,"1000",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			eval("usb_modeswitch","-v","0x05c6","-p","0x1000","-V","0x05c6","-P","0x6000","-M","5553424312345678c000000080000671020000000000000000000000000000");
			sleep(8);
		}
		*/
		else if(strncmp(g_str_product,"1000",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Eject_Operate(g_str_product);
			sleep(8);
		}
		else if(strncmp(g_str_product,"2001",4)==0)
		{	
			Modem_usbtype = USB0_TYPE;
			Eject_Operate(g_str_product);
		}
		else if(strncmp(g_str_product,"2000",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Eject_Operate(g_str_product);
		}
		else if(strncmp(g_str_product,"0010",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Eject_Operate(g_str_product);
		}
		else
		{
			g_simflag = 1;
			Modem_usbtype = ACM0_TYPE;	
			Eject_Operate(g_str_product);
		}
	}
	else if(strncmp(g_str_vendor,"1e89",4)==0)
	{
		eval("usb_modeswitch","-v","0x1e89","-p","0x1e16","-R","1");
		sleep(10);
		Modem_usbtype = USB0_TYPE;
	}		
	else if(strncmp(g_str_vendor,"15eb",4)==0)
	{
		
		eval("eject","/dev/sr0");
		sleep(10);
		Modem_usbtype = USB0_TYPE;		
	}
	else if(strncmp(g_str_vendor,"1bc7",4)==0)
	{
			eval("eject","/dev/sr0");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
	}				

#if 1		
	
	else if(strncmp(g_str_vendor,"201e",4)==0)//James on 2009-05-19, for EPHONE EVDO modem.
	{
			eval("eject","/dev/sr0");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"1410",4)==0)
	{
			if(strncmp(g_str_product, "5031", 4)==0)
			{
				eval("usb_modeswitch", "-v", "0x1410", "-p", "0x5031", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
				sleep(5);
				Modem_usbtype = USB0_TYPE;
			}
			else
			{
				eval("eject","/dev/sr0");
				sleep(5);
				Modem_usbtype = USB0_TYPE;
			}			
			
	}
	else if(strncmp(g_str_vendor,"0406",4)==0)//James 2009-01-08
	{
		g_sim_modem=1;
		Eject_Operate(g_str_product);
		Modem_usbtype = USB0_TYPE;
		File_Set_Modem_Info("modemtype","simmodem");
	}
	else if(strncmp(g_str_vendor,"1ab7",4)==0)//DT Modem James 2009-02-05
	{
		if(strncmp(g_str_product,"1206",4)==0)
		{

			g_DT_modem=1;
			eval("eject","/dev/sr0");
			sleep(4);
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","dtmmodem");
		}
		else if(strncmp(g_str_product,"2000",4)==0)
		{
			
			sleep(6);
			eval("eject","/dev/sr0");
			
			sleep(2);		

			return -1;

		}
		else if(strncmp(g_str_product,"1200",4)==0)
		{
			
			g_DT_modem=1;
			eval("eject","/dev/sr0");
			sleep(4);
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","dtmmodem");
		}		
		else if(strncmp(g_str_product,"5700",4)==0)
		{
			/*
			sleep(3);
			eval("usb_modeswitch","-v","0x1ab7","-p","0x5700","-V","0x1ab7","-P","0x2000","-M","5553424312345678000000000000061b000000030000000000000000000000","-m","0x04");
			sleep(3);
			Modem_usbtype = USB0_TYPE;
			g_TD_DT901 = 1;
			File_Set_Modem_Info("modemtype","901mmodem");
			Get_Usb_Info();
			*/

			g_DT_modem=1;
			eval("eject","/dev/sr0");
			sleep(4);
			Modem_usbtype = USB1_TYPE;
			File_Set_Modem_Info("modemtype","901mmodem");
		}
	}
	else if(strncmp(g_str_vendor,"04cc",4)==0) //TD Modem
	{
		g_sim_modem=1;
		eval("eject","/dev/sr0");
		sleep(6);
		Modem_usbtype = ACM0_TYPE;
		File_Set_Modem_Info("modemtype","simmodem");
	}
	else if(strncmp(g_str_vendor,"1033",4)==0)
	{
		if(strncmp(g_str_product,"0035",4)==0)//HUAWEI E630
		{
			eval("usb_modeswitch", "-v", "0x1033", "-p", "0x0035", "-V", "0x12d1", "-P", "0x1003", "-d", "1");
			sleep(3);
			Modem_usbtype = USB0_TYPE;
		}
	}	
	else if(strncmp(g_str_vendor,"1c9e",4)==0)//longqi modem
	{
		if(strncmp(g_str_product,"6000",4)==0)
		{			
			eval("usb_modeswitch", "-v", "0x1c9e", "-p", "0x6000", "-M", "55534243123456780000000000000601000000000000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"f000",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1c9e", "-p", "0xf000", "-V", "0x1c9e", "-P", "0x9603", "-M", "55534243123456788000000080000606f50402527000000000000000000000", "-m", "0x01");
			sleep(15);
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"1001",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1c9e", "-p", "0x1001", "-V", "0x1c9e", "-P", "0x6061", "-M", "55534243123456780000000000000606f50402527000000000000000000000");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
		}
		else
		{
			g_longqi_modem = 1;//longqi modem,éœ€è¦æ“ä½œttyUSB2è®¾å¤‡
		}		
	}	
	else if(strncmp(g_str_vendor,"0fce",4)==0)//MD300,åŒä¸Š
	{
		g_simflag = 1;
		Modem_usbtype = ACM0_TYPE;

		g_MD300_MODEM = 1;

		eval("udevtrigger");
		
		sleep(3);
	}
	else if(strncmp(g_str_vendor,"0af0",4)==0)//option Modem,¸ÃÄ£¿é±È½ÏÌØÊâ
	{
		if(strncmp(g_str_product,"7501",4)==0)//Vodafone K3760 
		{
			Modem_usbtype	= HS1_TYPE;
			eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x7501", "-C", "0xff", "-M", "55534243785634120100000080000601000000000000000000000000000000");
			sleep(2);
			eval("insmod", "hso.ko");
			sleep(1);
		}
		else if(strncmp(g_str_product,"7601",4)==0)
		{
			if(mc8780_flag==0)
			{
				eval("rmmod", "option");
				sleep(1);
				eval("rmmod", "usbserial");
				sleep(1);
				eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x7601", "-R", "1");
				sleep(4);
				eval("insmod", "usbserial", "vendor=0x0af0", "product=0x7601");
				sleep(4);
				mc8780_flag=1;
			}
			Modem_usbtype	= USB9_TYPE;
			File_Set_Modem_Info("modemtype","GTM380");
		}
		else if(strncmp(g_str_product,"6911",4)==0)//Icon 7.2
		{
			Modem_usbtype	= HS1_TYPE;
			eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x6911", "-C", "0xff", "-M", "55534243123456780000000000000601000000000000000000000000000000");
			sleep(2);
			eval("insmod", "hso.ko");
			sleep(1);
		}
		else if(strncmp(g_str_product,"6971",4)==0)
		{
			g_option_modem = 1;
			sleep(4);
			
			//eval("ozerocdoff","-i","0x6971");
			//sleep(1);
			
			//eval("insmod","/lib/modules/2.6.8.1/kernel/drivers/usb/class/hso.ko");
			//sleep(1);
			eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x6971", "-C", "0xff", "-M", "55534243785634120100000080000601000000000000000000000000000000");
			sleep(4);
			
			Modem_usbtype	= HS1_TYPE;
		}
		else if(strncmp(g_str_product,"6000",4)==0)
		{
			sleep(7);
			eval("usb_modeswitch", "-v", "0x0af0", "-p", "0x6000", "-V", "0xfeed", "-P", "0x1234", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
			sleep(5);
			Modem_usbtype	= ACM0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"1e89",4)==0)
	{
		if(strncmp(g_str_product,"f000",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1e89", "-p", "0xf000", "-M", "555342435808d884800000008000060619181a207000000000000000000000");
			sleep(8);
			Modem_usbtype = USB1_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"16d5",4)==0)//andydata
	{
		eval("usb_modeswitch", "-v", "0x16d5", "-p", "0x65d2", "-R", "1");
		sleep(5);
		Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"1199",4)==0) //sierra
	{
		if(strncmp(g_str_product,"0fff",4)==0)
		{
			/*
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
				sleep(6);
			}			
			eval("eject","/dev/sr0");
			sleep(6);
			Modem_usbtype = USB2_TYPE;
			*/
			eval("usb_modeswitch", "-v", "0x1199", "-p", "0x0fff", "-V", "0x1199", "-P", "0x6856", "-S", "1");
			sleep(5);
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"6856",4)==0)
		{
			if(mc8780_flag==0)
			{	
				eval("rmmod","option");
				sleep(2);
				eval("rmmod","usbserial");
				sleep(2);								
				eval("insmod","usbserial", "vendor=0x1199", "product=0x6856");
				mc8780_flag=1;
				sleep(6);
			}		
			Modem_usbtype=USB2_TYPE;
			File_Set_Modem_Info("modemtype","mc8780");
		}
		else if(strncmp(g_str_product,"0120",4)==0)
		{
			if(mc8780_flag==0)
			{	
				eval("rmmod","option");
				sleep(2);
				eval("rmmod","usbserial");
				sleep(2);								
				eval("insmod","usbserial", "vendor=0x1199", "product=0x0120");
				mc8780_flag=1;
				sleep(6);
			}
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","EV888U");
		}
	}
	else if(strncmp(g_str_vendor,"1bbb",4)==0)//ALCATEL
	{
		if(strncmp(g_str_product, "f000", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1bbb", "-p", "0xf000", "-V", "0x1bbb", "-P", "0x0000", "-M", "55534243123456788000000080000606f50402527000000000000000000000");
			sleep(20);
			Modem_usbtype = USB2_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"0471",4)==0)
	{
		if(strncmp(g_str_product, "1237", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x0471", "-p", "0x1237", "-V", "0x0471", "-P", "0x1234", "-M", "5553424312345678000000000000061b000000030000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product, "1210", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x0471", "-p", "0x1210", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"148f",4)==0)
	{
		if(strncmp(g_str_product, "2578", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x148f", "-p", "0x2578", "-V", "0x148f", "-P", "0x9021", "-M", "55534243908ecd89000000000000061b000000020000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"16d8",4)==0)
	{
		if(strncmp(g_str_product, "700a", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x16d8", "-p", "0x700a", "-C", "0xff", "-M", "55534243123456782400000080000dfe524445564348473d4e444953000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"0ace",4)==0)
	{
		if(strncmp(g_str_product, "2011", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x0ace", "-p", "0x2011", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product, "20ff", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x0ace", "-p", "0x20ff", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"072f",4)==0)
	{
		if(strncmp(g_str_product, "100d", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x072f", "-p", "0x100d", "-V", "0x072f", "-P", "0x90cc", "-M", "01b0000000000000000000000000000000000000000000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"106c",4)==0)
	{
		if(strncmp(g_str_product, "3b06", 4)==0)
		{
			eval("usb_modeswitch", "-v", "0x106c", "-p", "0x3b06", "-V", "0x106c", "-P", "0x3717", "-M", "55534243b82e238c24000000800008ff020000000000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"d324",4)==0)
	{
		if(strncmp(g_str_product,"f000",4)==0) //hexing modem
		{
			eval("usb_modeswitch", "-v", "0xd324", "-p", "0xf000", "-V", "0xd324", "-P", "0x9002", "-M", "55534243100ad0840000000000010aff000324010000000000000000000000", "-i" "0x03");
			sleep(3);
			Modem_usbtype = USB1_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"1e0e",4)==0)
	{
		if(strncmp(g_str_product,"defe",4)==0)
		{
			eval("eject","/dev/sr0");
			sleep(6);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"21f5",4)==0)
	{
		eval("eject","/dev/sr0");
		sleep(6);
		Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"6000",4)==0)
	{
		if(strncmp(g_str_product,"1000",4)==0)
		{	
			eval("eject","/dev/sr0");
			sleep(4);
			Modem_usbtype = USB0_TYPE;
		}			
	}
	else if(strncmp(g_str_vendor,"20a6",4)==0)
	{
		if(strncmp(g_str_product,"f00e",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x20a6", "-p", "0xf00e", "-V", "0x20a6", "-P", "0x1105", "-M", "5553424308339a84000000000001061b010000000000000000000000000000");
			sleep(5);
			Modem_usbtype = USB0_TYPE;
		}	
	}
	else if(strncmp(g_str_vendor,"230d",4)==0)
	{
		if(strncmp(g_str_product,"0001",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x230d", "-p", "0x0001", "-C", "0xff", "-u", "3");
			sleep(2);
			Modem_usbtype = ACM1_TYPE;
		} 
		else if(strncmp(g_str_product,"0007",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x230d", "-p", "0x0007", "-u", "2");
			sleep(2);
			Modem_usbtype = ACM0_TYPE;
			File_Set_Modem_Info("modemtype","vid230d");
		} 
	}
	else if(strncmp(g_str_vendor,"03f0",4)==0)
	{
		if(strncmp(g_str_product,"1e1d",4)==0)
		{
			//sleep(2);
			//Modem_usbtype = USB2_TYPE;
			if(mc8780_flag==0)
			{	
				eval("rmmod","option");
				sleep(2);
				eval("rmmod","usbserial");
				sleep(2);								
				eval("insmod","usbserial", "vendor=0x03f0", "product=0x1e1d");
				mc8780_flag=1;
				sleep(6);
			}
			Modem_usbtype = USB0_TYPE;
		} 
	}
#endif	
	sleep(2);
	sprintf(usbinfo,"0x%s/0x%s",g_str_vendor,g_str_product);
	File_Set_Modem_Info("modeminfo", usbinfo);
	return 1;
}

#endif
void unDeviceInfo()
{
		
	//create_ipaddr("xxx.xxx.xxx.xxx");
	//create_dialstatus(0);

	FILE *fp = NULL;
	char part[30];	
	
	unlink(STR_IN_FILE);
	unlink("/var/ttyAT");

	system("killall -15 pppd 1>/dev/null 2>&1");
	
	if (NULL == (fp = fopen("/proc/mounts", "r")))
	{
		return;
	}
	while(EOF != fscanf(fp, "%s %*s %*s %*s %*s %*s\n", part))
	{
		if (NULL != strstr(part, "/dev/sd"))
			eval("umount", "-l", part);
	}
	fclose(fp);		
	
}

int parse_hotplug_info(char *buf, int len, char *devinfo, int *action, int *isusb)
{
	char *ptr;
	int i = 0, subsystem = 0, devtype = 0, product = 0;
	char *vid=NULL,*pid=NULL;
		
	*action   = DEV_NONE;
	devtype   = 0;
	product   = 0;
	subsystem = 0;
	*isusb		=	0;

	while (i < len)
	{
		if (*action == DEV_NONE) 
		{
			if (strstr(buf+i,"ACTION=add"))
			{
	     			*action = DEV_ADD;
		  	}
	  		else if (strstr(buf+i,"ACTION=remove"))
	  		{
	      			*action = DEV_REMOVE;
	  		}
			else
			{
			}
		}
		else if (subsystem == 0) 
		{
  			if (strcmp(buf+i,"SUBSYSTEM=usb"))
  				subsystem = 1;
		}
		//else if (devtype == 0) 
		//{
			//if (strstr(buf+i, "DEVTYPE=usb_device"))
				//devtype = 1;
		//}
		else if (*action != DEV_NONE && subsystem == 1) 
		{
			ptr = strstr(buf+i, "PRODUCT=");
			
			if (ptr) 
  			{
  				vid = ptr+8;
				ptr = strchr(vid,'/');
				if(ptr)
				{
					*ptr=0; 
					ptr++; 
					pid = ptr;
					ptr = strchr(ptr,'/');
				
					if ( ptr ) 
						*ptr=0;

					convert_str(g_str_vendor, vid);
					convert_str(g_str_product, pid);
				}
				for(i=0;i<4;i++)
				{
					devinfo[i]=g_str_vendor[i];
					devinfo[i+5]=g_str_product[i];
					devinfo[4]=':';
					devinfo[9]='\0';
				}
    				*isusb=1;
    				return SUCCESS;
  			}
		}

		i += strlen(buf+i) + 1;
	}

	return FAILURE;
}

int main(int argc, char* argv[])
{
	struct sockaddr_nl nls;
	struct pollfd pfd;
	const int buffersize = 16 * 1024 * 1024;
	char buf[UEVENT_BUFFER_SIZE*2] = {0};
  	char devinfo[20];
	char usbinfo[26]={0};
	char guessTtyif[12]={0};
	int action = DEV_NONE;
	int func   = 0;
	int len    = 0;
	int usbFlag = 0;
	FILE *fp;
	int i=0,j=0;
	mc8780_flag = 0;

	eval("mkdir","-p","/var/usbmodem/");
	memset(&nls, 0, sizeof(nls));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	pfd.events = POLLIN;
	pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (pfd.fd == -1) 
	{
		DEBUG_PRINTF("Not root\n");
		return -1;
	}
	setsockopt(pfd.fd, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
	if (bind(pfd.fd, (void*)&nls, sizeof(nls))) 
	{
		DEBUG_PRINTF("bind failed\n");
		return -1;
	}

	//if (g_pwroff == 1) 
	//{
		//run_fork_modeswitch();
		//need_a_poweroff();
	//}
	
	while (-1 != poll(&pfd, 1, -1)) 
	{
		func = FUNC_NONE;
		len  = recv(pfd.fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (len == -1) 
		{
			DEBUG_PRINTF("recv failed\n");
			continue;
		}

		/* get device info */
		memset(devinfo,'\0',sizeof(devinfo));
		if (parse_hotplug_info(buf, len, devinfo, &action, &usbFlag)) 
		{
			DEBUG_PRINTF("\t*** Device %s[%d] ***\n", devinfo, action);
		}
		else
		{
			DEBUG_PRINTF("No Usb device\n");
			continue;
		}
		if(usbFlag && action == DEV_REMOVE)
		{
			if(0 == mc8780_flag)
			{
				if (0 == access("/var/hotplugadd", F_OK))
				{
					unlink("/var/hotplugadd");
				}
		
				unDeviceInfo();
			}
		}
		else if(usbFlag && action == DEV_ADD)
		{
			printf("\nvid=%s,pid=%s\n",g_str_vendor,g_str_product);
			if(!strcmp(g_str_vendor,"0000")||(!strcmp(g_str_vendor,"04cc")&&!strcmp(g_str_product,"1520")))
			{
				continue;
			}
			if (0 == access("/var/hotplugadd", F_OK))
			{
				continue;
			}					
			else
			{
				fp = fopen("/var/hotplugadd", "a");
				if (fp)
				{
					fclose(fp);
				}
			}
			printf("\r\n in init Deviceinfo new mode 11\r\n");
			File_Set_Modem_Info("usbmodemflag","yes");
			sprintf(usbinfo,"0x%s/0x%s",g_str_vendor,g_str_product);
			File_Set_Modem_Info("modeminfo", usbinfo);
			if(get_tty(guessTtyif))
			{
				DEBUG_PRINTF("have tty device \n");
				func=FUNC_TTY;
				if(Modem_usbtype == -1)
				{
					get_3gkard_info_tty();
				}
				make_visual_ppp_at_port(guessTtyif);
			}
			else
			{
				Mount_OurDefine_Usb_Module();
				if(Modem_usbtype == -1)
				{
					if(find_profile(devinfo))
					{
						DEBUG_PRINTF("is in mode switch list so we attemp switch it\n");
						func=FUNC_SCSI;
						eval("usb_modeswitch", "-c"," /var/usb_modeswitch.conf");
					}
					else
					{
						DEBUG_PRINTF("not in mode switch list and unknow kard\n");
					}
				}
			}
			
		}
		else
		{
			g_str_vendor[0] = '\0';
			g_str_product[0] = '\0';
		}
		//func = FUNC_SCSI;
		//else if (parse_hotplug_tty_info(buf, len, devinfo, &action))
			//func = FUNC_TTY;

		if (func==FUNC_SCSI || func==FUNC_TTY) 
		{
			//run_fork(func, action);
		}
	}
	DEBUG_PRINTF("musb demon end...\n");

	return SUCCESS;
}

