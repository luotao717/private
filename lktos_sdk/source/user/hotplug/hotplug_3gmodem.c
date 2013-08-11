#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <dirent.h>
#include "hotplug.h"
#include "hotplug_3gmodem.h"

CDMAInfo 								cdma_csq_info;
int                     g_simflag = -1;                     /* 3.option模块，可操作ttyHS*设备
																															 2.usb模块,可操作ttyUSB*设备
																															 1:sim模块，可操作ttyACM0，可以通过ttyS1操作的
																															 0:其他模块,可操作ttyACMO,但不可操作ttyS1
																															 -1:打开错误
																														*/
int											sim_flag=0;												//是否正确接入sim卡 	
int                     fd_mod = -1;											//打开模块设备句柄									 
unsigned char           *devname_other= "/dev/ttyACM0"; //读其他模块所需文件
unsigned char           *devname_usb0 = "/dev/ttyUSB0";  //读取ttyUSB0设备文件
unsigned char			*devname_usb1 = "/dev/ttyUSB1";  //读取ttyUSB1设备文件
unsigned char           *devname_usb2 = "/dev/ttyUSB2";  //读取ttyUSB2设备文件
unsigned char           *devname_usb3 = "/dev/ttyUSB3";  //读取ttyUSB3设备文件
unsigned char           *devname_usb5 = "/dev/ttyUSB5";  //读取ttyUSB5设备文件
unsigned char           *devname_usb9 = "/dev/ttyUSB9";  //读取ttyUSB9设备文件
unsigned char           *devname_hs0  = "/dev/ttyHS0";   //读取ttyHS0设备文件
unsigned char			*devname_hs1  = "/dev/ttyHS1";	  //读取ttyHS1设备文件
unsigned char 			*devname_hs2  = "/dev/ttyHS2";   //读取ttyHS2设备文件 
unsigned char        	g_str_vendor[8],g_str_product[8];
static unsigned char 	rddevbuf[256];

int 					g_huawei_modem = 0;     
int 					g_zte_modem = 0;        //ZTEmodem
int						g_longqi_modem = 0;    //longqi Modem
int						g_option_modem = 0;    //option Modem
int 					g_sim_modem=0;
int 					g_DT_modem=0;
int						g_TD_LC631=0; //zhubo add 2009-03-10,为TD模块，注册命令较为特殊
int     				g_TD_DT901=0;
int     				g_MD300_MODEM=0;//zhubo add  2009.04.08 为MD300，该模块需要注册
int						g_TD_MU350=0;//zhubo add 2009.05.15
int 					g_DO_AC580=0;//zhubo add 2009.08.31 该模块很特殊，首先要在USB2上发送一条AT命令，然后再在USB0上拨号，这样才有数据通信
int						Modem_usbtype = 0;
int						Modem_AT_Port = 0; //modem at command

/**********************信号灯操作函数************************/
extern int	eval_flag;

//先定义如下宏
#define GPIO_IOC_MAGIC  'G'
#define  GPIO_IOC_LED           _IOWR(GPIO_IOC_MAGIC, 6, struct gpio_ioctl)
/*在需要对GPIO操作时，直接调用该函数
pin:  所要操作的GPIO
high: 0关闭该led
	  1打开该led
*/
static void led_ctrl(int pin, int high)
{
	int internet_gpio_fd = -1;
	int type;
	static int count=0;
	struct gpio_ioctl 
	{
		unsigned int mask;
		unsigned int val;
	}gpio;

	internet_gpio_fd = open("/dev/gpio", O_RDWR);
	if(internet_gpio_fd == -1) 
	{
		printf("......Failed to open /dev/gpio\n");
		return;
	}

	gpio.mask  = pin;
	gpio.val  = high;
	if(ioctl(internet_gpio_fd, GPIO_IOC_LED, &gpio) < 0) 
	{
		printf("......invalid reserve gpio \n");
		return;
	}
	close(internet_gpio_fd);
}

#define PPPDEFGW		0x0A707070
#define PPPDEFIP		0x0A404040
#define _PPPoE_ENABLE        3
#define _CONNECT             1
#define _DISCONNECT          4
#define _LOGIN_IN_PROGRESS   2
typedef struct proc_s {
	char cmd[16];					/* basename of executable file in call to exec(2) */
	int ruid;						/* real only (sorry) */
	int pid;						/* process id */
	int ppid;						/* pid of parent process */
	char state;						/* single-char code for process state (S=sleeping) */
	unsigned int vmsize;			/* size of process as far as the vm is concerned */
} proc_t;
int DBgetPid(char *progname)
{
	proc_t p;
	DIR *dir;
	FILE *file;
	struct dirent *entry;
	char path[32], sbuf[512];
	char uidName[9], stmp[200];
	int len, i, c, pid=0, ret, shcmd=0;

	dir = opendir("/proc");
	if (!dir)
	{
		return 0;
	}


	while ((entry = readdir(dir)) != NULL) {
		if (!isdigit(*entry->d_name))
			continue;
		sprintf(path, "/proc/%s/status", entry->d_name);
		if ((file2str_ex(path, sbuf, sizeof(sbuf))) != -1) {
			ret = parse_proc_status(sbuf, &p);
			if ( ret == 0 ) break;
		}

		sprintf(path, "/proc/%s/cmdline", entry->d_name);
		file = fopen(path, "r");
		if (file == NULL)
			continue;
		i = 0;

		memset( stmp, 0, sizeof(stmp) );
		shcmd = 0;
		while (((c = getc(file)) != EOF) && (i < 200)) {
			stmp[i] = c;
			if (c == '\0' && i)
			{
				if ( strcmp(stmp,"sh") != 0 )
				{
				  if ( strcmp(stmp,"-c") != 0 )
				  	break;
				  else 
				  	shcmd=2;
				}
				else
				    shcmd=1;
				i=0;
			}
			else
				i++;
		}

		fclose(file);
		
		if (i == 0)
		{
			if ( !strcmp(p.cmd, progname) )
			{
				pid = p.pid;
				break;
			}
		}
		else
		{
			if ( !strcmp(stmp, progname) )
			{
				pid = p.pid;
				break;
			}
			else if ( shcmd && strstr(stmp, progname) )
			{
				pid = p.pid;
				break;
			}
		}
	}
	closedir(dir);
	return pid;
}

int DBgetPPPStatus( char *pppname, char *subpty )
{
	struct ifreq ifr;
	struct sockaddr_in *ina;
	int ret, i, status, pid, sid;
	long ip;
	FILE *fp;

	pid = DBgetPid("pppd");
	if ( pid == 0 )
	{
		return  _DISCONNECT;
	}

	
	status = _LOGIN_IN_PROGRESS;
	sid = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( sid < 0 )
	{
		printf( "sock sid < 0\n" );
		return status;
	}

	for ( i=0; i<IFNAMSIZ; i++ )
		ifr.ifr_name[i]=0;

	strcpy(ifr.ifr_name, pppname );
	ina = (struct sockaddr_in *)&(ifr.ifr_ifru.ifru_addr);
	memset(ina, 0, sizeof(*ina));
  	ina->sin_family = AF_INET;
	
	/*  Get the ppp0 IP Address */
	ret=ioctl(sid, SIOCGIFADDR, &ifr);
	if ( ret == -1 )
	{		
//		nvram_bufset(RT2860_NVRAM,"dial3gip","");

		close(sid);
		return status;
	}
	else
	{
		ip = (long)(ina->sin_addr.s_addr);
		
//		nvram_bufset(RT2860_NVRAM,"dial3gip",inet_ntoa(ina->sin_addr));
		
		if ( ip == 0 )
		{
			printf ( "ip ==0\n" );			
			close(sid);
			return status;
		}
		if ( ip == PPPDEFGW || ip == PPPDEFIP ) // 10.64.64.64 10.112.112.112
		{
			close(sid);
			pid = DBgetPid(subpty);
			if ( pid == 0 )
  	    		return  _DISCONNECT;
			else
				return _LOGIN_IN_PROGRESS;
		}
		else
		{
			pid = DBgetPid(subpty);
			if ( pid == 0 )
			{
				printf ( "%s pid ==0\n", subpty );
				close(sid);
   				return  _DISCONNECT;
			}
		}
	}
	close(sid);

	status = _CONNECT;	
	return status;
}

int file2str_ex(char *filename, char *ret, int cap)
{
	int fd, num_read;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if ((num_read = read(fd, ret, cap - 1)) <= 0)
		return -1;
	ret[num_read] = 0;
	close(fd);
	return num_read;
}

int parse_proc_status(char *S, proc_t * P)
{
	char *tmp;

	memset(P->cmd, 0, sizeof P->cmd);
	sscanf(S, "Name:\t%15c", P->cmd);
	tmp = strchr(P->cmd, '\n');
	if (tmp)
		*tmp = '\0';
	tmp = strstr(S, "State");
	sscanf(tmp, "State:\t%c", &P->state);

	P->vmsize = 0;
	tmp = strstr(S, "Pid:");
	if (tmp)
		sscanf(tmp, "Pid:\t%d\n" "PPid:\t%d\n", &P->pid, &P->ppid);
	else
		return 0;

	/* For busybox, ignoring effective, saved, etc. */
	tmp = strstr(S, "Uid:");
	if (tmp)
		sscanf(tmp, "Uid:\t%d", &P->ruid);
	else
		return 0;

	tmp = strstr(S, "VmSize:");
	if (tmp)
		sscanf(tmp, "VmSize:\t%d", &P->vmsize);

	return 1;
}


int IsUsbStorageModule()
{
	FILE	*fp = NULL;
	char	string[1024];

	fp = fopen("/proc/modules","r");
	if(fp == NULL)
		return -1;

	fgets(string,1024,fp);
	fclose(fp);
	fp = NULL;
	
	if(strstr(string,"usb_storage"))
		return 1;
	else
		return 0;
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

int File_Get_Modem_Info(char *name,char *value)
{
	FILE *fp=NULL;
	char path[64];
	int	num=0;

	memset(path,'\0',64);
	sprintf(path,"%s%s",USBMODEMPATH,name);

	fp = fopen(path,"r");
	if(fp == NULL)
	{
		return -1;
	}
	else
	{		
		num=fgets(value,32,fp);
		fclose(fp);
		fp = NULL;
		return num;
	}
	
}

//zhubo add 2008.05.14
int write_str(int fd, char *str, int len)
{
	int i, readlen;
	unsigned char wt_str[64], count=3;
	
	if(g_option_modem == 1)
	{
		if(fd>0)
		{
			sprintf(wt_str,"%s\r",str);
        	write(fd,wt_str,strlen(wt_str));			
		}
		else
		{
			fd = open(devname_hs0, O_RDWR|O_NONBLOCK);//ttyHS0
			if(fd<=0)
			{				
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
		write( fd, "\r", 1);
		write(fd, "\n", 1); //add by leo 2008.01.09
	}	
	usleep(500000);

	while(count--)
	{
		memset(rddevbuf, 0, sizeof(rddevbuf));
		if( (readlen = read(fd, rddevbuf, 256)) > 0)
		{ 
			if(g_option_modem == 1)
			{
				close(fd);
				fd = 0;
			}
			break;
		}
		else
			sleep(1);
	}	

	//usleep(100000);

	if(g_option_modem == 1)//该模块比较特殊，要再发一次AT才能读出上次命令的返回值
	{
		if(fd>0)
		{
			write(fd,"at\r",strlen("at\r"));			
		}
		else
		{
			fd = open(devname_hs0, O_RDWR|O_NONBLOCK);//ttyHS0
			if(fd<=0)
			{
				return -1;
			}			
			write(fd,"at\r",strlen("at\r"));		
		}
		usleep(400000);

		memset(rddevbuf, 0, sizeof(rddevbuf));
		if( (readlen = read(fd, rddevbuf, 256)) > 0)
		{ //modi by leo 2008.01.31
			if(g_option_modem == 1)
			{
				close(fd);
				fd = 0;
			}
		}		
	}

	printf("\nwrite to modem, cmd:%s,return:%s!\n",str,rddevbuf);
	syslog(LOG_DEBUG,"Write To Moudule, Cmd: %s,Return: %s!\n",str,rddevbuf);
	
	return readlen;
}	

int	crtscts = 0;		/* Use hardware flow control */
int	modem = 1;			/* Use modem control lines */
void setdtr (int tty_fd, int on)
{
    int modembits = TIOCM_DTR;
    ioctl(tty_fd, (on ? TIOCMBIS : TIOCMBIC), &modembits);
}
void set_up_tty(int tty_fd, int local, int flag)
{
    int speed;
    struct termios tios;

    setdtr(tty_fd, 1);
    if (tcgetattr(tty_fd, &tios) < 0) {
	return;
    }
    tios.c_cflag     &= ~(CSIZE | CSTOPB | PARENB | CLOCAL);
    tios.c_cflag     |= CS8 | CREAD | HUPCL;
    tios.c_iflag      = IGNBRK | IGNPAR;
    tios.c_oflag      = 0;
    tios.c_lflag      = 0;
    tios.c_cc[VMIN]   = 1;
    tios.c_cc[VTIME]  = 0;
    if (local || !modem)
	tios.c_cflag ^= (CLOCAL | HUPCL);
    switch (crtscts) {
    case 1:
	tios.c_cflag |= CRTSCTS;
	break;
    case -2:
	tios.c_iflag     |= IXON | IXOFF;
	tios.c_cc[VSTOP]  = 0x13;	/* DC3 = XOFF = ^S */
	tios.c_cc[VSTART] = 0x11;	/* DC1 = XON  = ^Q */
	break;
    case -1:
	tios.c_cflag &= ~CRTSCTS;
	break;
    default:
	break;
    }

	if(flag)
		speed = B115200;
	else
		speed = B4000000;


    if (speed) {
	cfsetospeed (&tios, speed);
	cfsetispeed (&tios, speed);
    }
/*
 * We can't proceed if the serial port speed is B0,
 * since that implies that the serial port is disabled.
 */
    else {
	speed = cfgetospeed(&tios);
	if (speed == B0)
	    ;
    }
    if (tcsetattr(tty_fd, TCSAFLUSH, &tios) < 0)
			;
}



/*
检测模块是否存在: 0 存在
				 -1	不存在
*/
int Check_Module_Exist(int fd)
{
	int 						i, num = 3, exist = 0;
	
	for(i=0; i<num; i++)
	{
		if(g_option_modem==1)
			write_str(fd, "ate0", strlen("ate0"));
		else
			write_str(fd, "ate0", strlen("ate0"));
		
		if(strstr(rddevbuf,"OK")||strstr(rddevbuf,"ok"))
		{
			return 0;			
		}			
		else
		{
			exist = 0;
			usleep(500000);
			continue;
		}
	}
	
	///3次尝试发at命令，设备没回应或者回应错误，则认为该模块不存在
	return -1;
}


/*当打开ttyACM0设备均失败时，要动态加载usbserial驱动，要得到此模块的vendor和product
加载完成后，再次打开ttyACMO,若能正常打开则返回数值，若打开失败则返回-1
*/
void Get_Usb_Info()
{
	int     fd_proc=-1,size=0,i=0;
	char		buf[2*1024],*str=NULL;

	memset(g_str_vendor,'\0',8);
	memset(g_str_product,'\0',8);

	fd_proc = open("/proc/bus/usb/devices",O_RDWR|O_NONBLOCK);
	if(fd_proc<=0)
	{
   		syslog(LOG_DEBUG,"Get_Usb_Info open /proc/bus/usb/devices error!\n");

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
			if((*str++=='0')&&(*str=='9'))//设备类型为09,即是hub,忽略,继续查找
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



//对于某些特定模块，需要eject的操作，且其PID会与eject前不一样
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
		
		syslog(LOG_DEBUG,"Eject_Operate vendor,product i is %s,%s,%d!\n",g_str_vendor,g_str_product,i);
				
		if((strlen(g_str_vendor) != 0)&&(strncmp(g_str_product, tmppid, 4)!= 0))
			break;
		
		sleep(1);
	}

	return;
}


int Mount_Usb_Module()
{
	int 		fd=-1;
	char		cmdstr[100], usbinfo[32];

	memset(usbinfo,'\0',32);
	sleep(1);

	if( (strncmp(g_str_vendor,"0000",4)==0) || (strncmp(g_str_product,"0000",4)==0)
		|| (strlen(g_str_vendor)==0) || (strlen(g_str_product)==0))//说明没找到模块
	{
		return -1;
	}

	//James 2008-11-18
  	if(strncmp(g_str_vendor, "19d2",4) == 0 ) //ZTE Modem，该modem要eject后才能映射出ttyUSB0-2
	{	
		
		if(strncmp(g_str_product,"2000",4)==0)
		{	
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}
			
			sleep(6);
			Eject_Operate(g_str_product);
			sleep(2);
			Modem_usbtype = USB2_TYPE;
		}
						
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
		else if(strncmp(g_str_product,"0002",4)==0)//zhubo add 2009.02.02 MF628(ONDA)
		{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}		
			
			sleep(6);
			Eject_Operate(g_str_product);
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"0003",4)==0)//zhubo add 2009.05.14 TD-MU350
		{

			//eval("usb_modeswitch","-v","0x19d2","-p","0x0003", "-M", "5553424312345678000000000000061b000000020000000000000000000000", "-R", "1");
			//eval("usb_modeswitch","-v","0x19d2","-p","0x0003", "-M", "5553424312345678000000000000061b000000020000000000000000000000");
			//sleep(5);
			g_TD_MU350 = 1;
			Modem_usbtype = USB0_TYPE;
			File_Set_Modem_Info("modemtype","mu350mmodem");
		}
		else if(strncmp(g_str_product,"fff1",4)==0) //zhubo add 2009.04.18
		{
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"fff5",4)==0) //zhubo add 2009.04.18
		{			
			eval("usb_modeswitch","-v","0x19d2","-p","0xfff5","-V","0x19d2","-P","0xfff1","-M","5553424312345678c00000008000069f030000000000000000000000000000");
			sleep(3);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"0026",4)==0) //AC580 EVD0
		{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}						
			sleep(6);
			Eject_Operate(g_str_product);
			Modem_usbtype = USB2_TYPE;
			g_DO_AC580 = 1;			
		}
		else if(strncmp(g_str_product,"0090",4)==0) //MU330 TD
		{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
				sleep(6);
			}						
			
			Eject_Operate(g_str_product);
			//eval("rmmod","usb-storage.ko");
			Modem_usbtype = ACM0_TYPE;
			File_Set_Modem_Info("modemtype","TDA356");			
		}
		else if(strncmp(g_str_product,"0034",4)==0)
		{
			//Modem_usbtype = ACM0_TYPE;
			Modem_usbtype = ACM2_TYPE;
			File_Set_Modem_Info("modemtype","TDA356");
		}
		else if(strncmp(g_str_product,"0120",4)==0) //A356 TD
		{	
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
				sleep(6);
			}
			Eject_Operate(g_str_product);
			Modem_usbtype = USB0_TYPE;	
			File_Set_Modem_Info("modemtype","TDA356");
		}
		else if(strncmp(g_str_product,"0079",4)==0) //A356 TD
		{
			Modem_usbtype = USB0_TYPE;	
			//File_Set_Modem_Info("modemtype","TDA356");
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
		else if(strncmp(g_str_product,"0053",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x19d2", "-p", "0x0053", "-M", "55534243123456782000000080000c85010101180101010101000000000000");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
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
	else if(strncmp(g_str_vendor,"0421",4)==0)//NOKIA zhubo add 2009.10.19 
	{
		if(IsUsbStorageModule()<=0)
		{
			eval("insmod","usb-storage.ko");
		}
		sleep(6);
		Eject_Operate(g_str_product);
		Modem_usbtype = ACM0_TYPE;
	}
	else if(strncmp(g_str_vendor,"12d1",4)==0 || strncmp(g_str_vendor,"1614",4)==0)//华为modem,夏新modem
	{
		if(strncmp(g_str_product,"1da1",4)==0)
		{
			eval("usb_modeswitch","-v","0x12d1","-p","0x1da1","-H","1");
			sleep(6);
			Modem_usbtype = ACM0_TYPE;
			goto done;
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
			//goto done;			
		}
		else if(strncmp(g_str_product,"14ac",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			sleep(2);
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
		else if (strncmp(g_str_product,"14fe",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
			
			eval("usb_modeswitch", "-v", "0x12d1", "-p", "0x14fe", "-V", "0x12d1", "-P", "0x1506" ,"-M" ,"55534243123456780000000000000011062000000100000000000000000000");
			sleep(8);
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
	else if(strncmp(g_str_vendor,"05c6",4)==0)//SIM Modem，该modem要eject后才能映射出ttyACM0
	{
		if(strncmp(g_str_product,"f000",4)==0)//PHu9 on 2009-02-25
		{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}			
			sleep(6);
			Eject_Operate(g_str_product);
			Modem_usbtype = USB2_TYPE;
		}
		else if(strncmp(g_str_product,"6000",4)==0)//EVDO on 2009-03-07
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
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}			
			sleep(6);
			Eject_Operate(g_str_product);
			sleep(8);
		}
		else if(strncmp(g_str_product,"2001",4)==0)
		{	
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}			
			sleep(8);
			Modem_usbtype = USB0_TYPE;
			Eject_Operate(g_str_product);
		}
		else if(strncmp(g_str_product,"2000",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}			
			sleep(6);
			Eject_Operate(g_str_product);
		}
		else if(strncmp(g_str_product,"0015",4)==0)
		{
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"0010",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
				sleep(6);
			}			
			Eject_Operate(g_str_product);
		}
		else if(strncmp(g_str_product,"00a0",4)==0)
		{
			Modem_usbtype = USB0_TYPE;			
		}
		else
		{
			g_simflag = 1;
			Modem_usbtype = ACM0_TYPE;	
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}			
			sleep(6);
			Eject_Operate(g_str_product);
			goto done;
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
	else if(strncmp(g_str_vendor,"1e89",4)==0)
	{
		eval("usb_modeswitch","-v","0x1e89","-p","0x1e16","-R","1");
		sleep(10);
		Modem_usbtype = USB0_TYPE;
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
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}	
			sleep(6);
			eval("eject","/dev/sr0");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
		}			
	}
	else if(strncmp(g_str_vendor,"1bc7",4)==0)
	{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}	
			sleep(6);
			eval("eject","/dev/sr0");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
	}				

#if 1		
	else if(strncmp(g_str_vendor,"1ed1",4)==0)
	{
		Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"201e",4)==0)//James on 2009-05-19, for EPHONE EVDO modem.
	{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}
			sleep(6);
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
				if(IsUsbStorageModule()<=0)
				{
					eval("insmod","usb-storage.ko");
				}
				
				sleep(6);
				eval("eject","/dev/sr0");
				sleep(5);
				Modem_usbtype = USB0_TYPE;
			}			
			
	}
	else if(strncmp(g_str_vendor,"0406",4)==0)//James 2009-01-08
	{
		g_sim_modem=1;
		if(IsUsbStorageModule()<=0)
		{
			eval("insmod","usb-storage.ko");
		}						

		sleep(6);
		Eject_Operate(g_str_product);
		Modem_usbtype = USB0_TYPE;
		File_Set_Modem_Info("modemtype","simmodem");
	}
	else if(strncmp(g_str_vendor,"1ab7",4)==0)//DT Modem James 2009-02-05
	{
		if(strncmp(g_str_product,"1206",4)==0)
		{

			g_DT_modem=1;
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}
			sleep(6);
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
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}	

			sleep(6);
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
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}
			sleep(6);
			eval("eject","/dev/sr0");
			sleep(4);
			Modem_usbtype = USB1_TYPE;
			File_Set_Modem_Info("modemtype","901mmodem");
		}
		else if(strncmp(g_str_product,"2000",4)==0)
		{	

			File_Set_Modem_Info("modemtype","901mmodem");
		}
		else if(strncmp(g_str_product,"5730",4)==0)
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

				g_sim_modem=1;
				if(IsUsbStorageModule()<=0)
				{
					eval("insmod","usb-storage.ko");
				}	
				sleep(6);
				eval("eject","/dev/sr0");
				sleep(6);
				Modem_usbtype = ACM0_TYPE;
				File_Set_Modem_Info("modemtype","simmodem");
			}			
			goto done;
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
		else if(strncmp(g_str_product,"9e00",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"1001",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1c9e", "-p", "0x1001", "-V", "0x1c9e", "-P", "0x6061", "-M", "55534243123456780000000000000606f50402527000000000000000000000");
			sleep(10);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"6061",4)==0)
		{
			Modem_usbtype = USB0_TYPE;
			Modem_AT_Port = USB2_TYPE;
		}
		else
		{
			g_longqi_modem = 1;//longqi modem,需要操作ttyUSB2设备
			Modem_usbtype = USB2_TYPE;
		}		
	}	
	else if(strncmp(g_str_vendor,"0fce",4)==0)//MD300,同上
	{
		g_simflag = 1;
		Modem_usbtype = ACM0_TYPE;

		g_MD300_MODEM = 1;

		eval("udevtrigger");
		
		sleep(3);
		goto done;
	}
	else if(strncmp(g_str_vendor,"0af0",4)==0)//option Modem,该模块操作比较特殊
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
		else if(strncmp(g_str_product,"7211",4)==0)
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
		if(strncmp(g_str_product,"1e16",4)==0)
		{
			//eval("usb_modeswitch", "-v", "0x1e89", "-p", "0x1e16", "-R", "1");
			//sleep(10);
			Modem_usbtype = USB0_TYPE;
		}
		else if(strncmp(g_str_product,"f000",4)==0)
		{
			eval("usb_modeswitch", "-v", "0x1e89", "-p", "0xf000", "-M", "555342435808d884800000008000060619181a207000000000000000000000");
			sleep(8);
			Modem_usbtype = USB1_TYPE;
		}
		else if(strncmp(g_str_product,"1a20",4)==0)
		{
			Modem_usbtype = USB3_TYPE;
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
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
			}	
			sleep(6);
			eval("eject","/dev/sr0");
			sleep(6);
			Modem_usbtype = USB0_TYPE;
		}
	}
	else if(strncmp(g_str_vendor,"0685",4)==0)
	{
		if(strncmp(g_str_product,"6001",4)==0)
			Modem_usbtype = USB0_TYPE;
	}
	else if(strncmp(g_str_vendor,"21f5",4)==0)
	{
		if(IsUsbStorageModule()<=0)
		{
			eval("insmod","usb-storage.ko");
		}	
		sleep(6);
		eval("eject","/dev/sr0");
		sleep(6);
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
	else if(strncmp(g_str_vendor,"6000",4)==0)
	{
		if(strncmp(g_str_product,"1000",4)==0)
		{
			if(IsUsbStorageModule()<=0)
			{
				eval("insmod","usb-storage.ko");
				sleep(8);
			}			
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
		else if(strncmp(g_str_product,"1105",4)==0)
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
	else //default
	{
		if(IsUsbStorageModule()<=0)
		{
			eval("insmod","usb-storage.ko");
			return -1;
		}
	}
#endif	
		
	//eval("insmod","/lib/modules/2.6.8.1/kernel/drivers/usb/serial/option.ko");	

done:
	sleep(2);
	sprintf(usbinfo,"0x%s/0x%s",g_str_vendor,g_str_product);
	//nvram_set("modeminfo",usbinfo);
	File_Set_Modem_Info("modeminfo", usbinfo);
		
	fd = open(devname_other, O_RDWR|O_NONBLOCK);//ttyACM0
	if(fd <=0 )
	{	
		g_simflag = -1;
		switch ( Modem_usbtype )
		{
		case USB0_TYPE:
			fd = open(devname_usb0, O_RDWR|O_NONBLOCK);//ttyUSB0
			break;

		case USB1_TYPE:
			fd = open(devname_usb1,O_RDWR|O_NONBLOCK);
			break;
				
		case USB2_TYPE:
			fd = open(devname_usb2, O_RDWR|O_NONBLOCK);//ttyUSB2
			break;
			
	  case USB3_TYPE:
			fd = open(devname_usb3, O_RDWR|O_NONBLOCK);//ttyUSB3
			break;		

		case USB5_TYPE:
			fd = open(devname_usb5, O_RDWR|O_NONBLOCK);//ttyUSB5
			break;
		case USB9_TYPE:
			fd = open(devname_usb9, O_RDWR|O_NONBLOCK);//ttyUSB9
			break;

		case HS1_TYPE:
			sleep(2);
			fd = open(devname_hs0, O_RDWR|O_NONBLOCK);//ttyHS0
			break;
			
		default: 
			break;
		}
		
		return fd;
	}
	//set_up_tty(fd,1,0);
	g_simflag = 0;
	fd_mod = fd;
	return fd_mod;
}


//打开文件cdma文件后，设置波特率
int open_cdmadev()
{
	int fdother=0,fdusb0=0,ret=-1;
	int count=0;//记录打开ttyACM0的次数

	if(!strncmp(g_str_vendor,"12d1",4)&&!strncmp(g_str_product,"1d09",4))
	{		
		Modem_usbtype = ACM0_TYPE;
		Modem_AT_Port = ACM2_TYPE;
		g_TD_DT901 = 1;
		File_Set_Modem_Info("modemtype","dtmmodem");
		g_simflag = 0;
		fd_mod = 1;
		return 1;
	}

done:	
	fdother = open(devname_other, O_RDWR|O_NONBLOCK);
	if( fdother <= 0)
	{		
		count++;
		if(count<1)//可能ttyACMO设备没有准备好，给足够的时间
		{
			sleep(2);
			goto done;
		}
		else
		{
			count = 0;
			g_simflag = -1;
			goto done1;
		}			
	}	
	//set_up_tty(fdother,1,0);
	//ret = Check_Module_Exist(fdother);
	ret = 0;
	
	if(ret == 0)//说明该模块存在
	{
		g_simflag = 0;
		fd_mod = fdother;

		//模块类型为ttyACM0

		if(!strncmp(g_str_vendor,"12d1",4)&&!strncmp(g_str_product,"1d09",4))
		{
			Modem_usbtype = ACM0_TYPE;
			Modem_AT_Port = ACM2_TYPE;
			g_TD_DT901 = 1;
			File_Set_Modem_Info("modemtype","dtmmodem");
		}
		else
			Modem_usbtype = ACM0_TYPE;

		return fd_mod;
	}
	else
	{
		close(fdother);
		g_simflag = -1;
		fd_mod = -1;
		return fd_mod;
	}

done1:
		fdusb0=Mount_Usb_Module();
		if(fdusb0 <= 0)
		{
			g_simflag = -1;
			fd_mod = -1;			
			return fd_mod;
		}

		if(g_simflag == 0)//说明是ttyACM0
		{
			//ret = Check_Module_Exist(fdusb0);
			ret = 0;
			if(ret == 0)//说明该模块存在
			{
				g_simflag = 0;
				fd_mod = fdusb0;

				//模块类型为ttyACM0
				Modem_usbtype = ACM0_TYPE;
			}
			else
			{
				close(fdusb0);
				g_simflag = -1;
				fd_mod = -1;
			}
			return fd_mod;
		}

	//set_up_tty(fdusb0,1,0);
	//ret = Check_Module_Exist(fdusb0);
	ret = 0;
	if(ret == 0)//说明该模块存在
	{
		if(Modem_usbtype==HS1_TYPE)//option modem
			g_simflag = 3;
		else//ttyusb* modem
			g_simflag = 2;
		fd_mod = fdusb0;
		return fd_mod;
	}
	else//说明读模块出错
	{
		close(fdusb0);
		g_simflag = -1;
		fd_mod = -1;
		return fd_mod;
	}
}


int CheckUSB()
{
	int fd;
	
	memset(&cdma_csq_info,'\0', sizeof(CDMAInfo));

	sim_flag=0;//add by cui 2009.01.15
	fd = open_cdmadev();	
	if( fd <= 0)
	{
		strcpy(cdma_csq_info.Status,"NO MODULE");
		return -1;
	}

	if(g_option_modem == 1)
	{
		if(fd_mod>0)
		{
			sim_flag=1;
			close(fd_mod);
		}
		fd_mod = -1;
	}
	else
	{	
		/*
		if(!g_TD_MU350&&!g_TD_DT901)
		{
			write_str(fd_mod, "at+cpin?", strlen("at+cpin?"));
			if(strstr(rddevbuf, "OK") != 0)
			{
				sim_flag=1;					
			}				
		}
		else
		*/
			sim_flag = 1;	
	}
	return 1;
}


/*
 * create_resolv - create the replacement resolv.conf file
 */
static void
create_resolv(unsigned char *peerdns1, unsigned char *peerdns2)
{
    FILE *f;

    f = fopen("/var/ppp/resolv.conf", "w");
    if (f == NULL) {
	error("Failed to create %s: %m", "/var/ppp/resolv.conf");
	return;
    }

    if (peerdns1)
	fprintf(f, "nameserver %s\n", peerdns1);

    if (peerdns2)
	fprintf(f, "nameserver %s\n", peerdns2);

    if (ferror(f))
	error("Write failed to %s: %m", "/var/ppp/resolv.conf");

    fclose(f);
}

static void
create_ipaddr(char *ipaddr)
{
	FILE *f;

	f = fopen("/proc/var/fyi/wan/ppp0/ipaddress","w");
	if (f == NULL) {
	return;
    }	

	fprintf(f, "%s\n", ipaddr);

	fclose(f);
}

static void
create_dialstatus(int flag)
{
	FILE *f;

	f = fopen("/proc/var/fyi/wan/ppp0/status","w");
	if(f == NULL){
	return;
    }

	if(flag)
		fprintf(f, "%s\n", "6");
	else
		fprintf(f, "%s\n", "1");

	fclose(f);
}


/*
特别针对option模块拨号
*/
void Dial_Option_Modem(char *usrname, char *passwd)
{
	int 		  fd=0, fd_option=0,i=0;
	int     	  wflag=0;//是否写flash的标志
	unsigned char buf[64],ip[16],dns1[16],dns2[16],*pb=NULL,cmd_dns[128];
	static OptionNetS	optionnet;

	memset(buf,'\0',64);
	memset(optionnet.ip,'\0',16);
	memset(optionnet.dns1,'\0',16);
	memset(optionnet.dns2,'\0',16);
	memset(cmd_dns,'\0',128);

	sprintf(buf,"AT$QCPDPP=1,1,\"%s\",\"%s\"",passwd,usrname);	

	write_str(fd, buf, strlen(buf));
	sleep(1);
	write_str(fd, "AT_OWANCALL=1,1,0",strlen("AT_OWANCALL=1,1,0"));
	sleep(5);
	write_str(fd, "AT_OWANDATA=1",strlen("AT_OWANDATA=1"));	
	sleep(6);


	if(pb=strchr(rddevbuf,','))
	{
		pb += 2;
		i = 0;
		while(*pb != ',')
		{
			optionnet.ip[i] = *pb;
			i++;
			pb++;
		}
		optionnet.ip[i] = '\0';

		pb += 11;
		i = 0;
		while(*pb != ',')
		{
			optionnet.dns1[i] = *pb;
			pb++;
			i++;
		}
		optionnet.dns1[i] = '\0';

		pb += 2;
		i = 0;
		while(*pb != ',')
		{
			optionnet.dns2[i] = *pb;
			pb++;
			i++;
		}
		optionnet.dns2[i] = '\0';

		if(strlen(optionnet.ip))
		{
			create_ipaddr(optionnet.ip);
			create_dialstatus(1);
			////led_ctrl(3, 0);//开启
		}			

		sprintf(cmd_dns, "%s%s%s","echo  \"nameserver ", optionnet.dns1, "\" >> /var/fyi/sys/dns");
		system(cmd_dns);
		sleep(1);
		sprintf(cmd_dns, "%s%s%s","echo  \"nameserver ", optionnet.dns2, "\" >> /var/fyi/sys/dns");
		system(cmd_dns);
		sleep(1);

		sprintf(cmd_dns,"ifconfig hso0 %s",optionnet.ip);
		system(cmd_dns);
		usleep(10000);
		sprintf(cmd_dns,"%s","route add default hso0");
		system(cmd_dns);
		
		create_resolv(optionnet.dns1, optionnet.dns2);

		system("killall -9 dnsmasq");
		sleep(1);
		system("dnsmasq -h -i br0 -r /var/ppp/resolv.conf &");		
	}		
}



void WriteWcdmaApn(char *pAPN,char *devname)
{
	int fd=0;
	unsigned char buf[100];
		
	if(devname==NULL)
		return;
	
	fd = open(devname, O_NONBLOCK | O_RDWR, 0);
	if( fd <= 0)
	{
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

	if(strlen(pAPN))
		sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", pAPN);
	else
		sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"", "cmnet");
	
	write_str(fd, buf, strlen(buf));
	if(!g_option_modem)
	{
		close(fd);
		fd = 0;
	}
}


//自己加的一些额外信息，均在次函数中初始化
void InitDeviceInfo()
{	
	int 		ret=0, num=0;
	int     	moduletype;
	char		*choicetype=NULL;
	char		dialapn[32], dialusername[32], dialpassword[32], dialnum[32],usbinfo[32];	

	
	memset(dialapn,'\0',32);
	memset(dialusername,'\0',32);
	memset(dialpassword,'\0',32);
	memset(dialnum,'\0',32);
	memset(usbinfo, '\0',32);

	printf("\r\n in init Deviceinfo\r\n");
	File_Set_Modem_Info("usbmodemflag","yes");//检查到modem
	sprintf(usbinfo,"0x%s/0x%s",g_str_vendor,g_str_product);
	File_Set_Modem_Info("modeminfo", usbinfo);
	
	//nvram_init(RT2860_NVRAM);
 	ret = CheckUSB();
		
	if(ret==-1)
	{
		return ;
	}
	
	if(!sim_flag)//未接入sim卡，或sim卡错误
	{
		if(fd_mod)
			close(fd_mod);
			
		return;	
	}	

	if(g_option_modem!=1)
	{
		if(fd_mod<=0)
			return;
	}

	if(g_simflag<0)
		return ;
	else
	{
		close(fd_mod);
		fd_mod = -1;
	}	

#if 1
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
				eval("ln","-s",STR_USB0,STR_IN_FILE);//LC6311用USB0来拨号
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
	
	//choicetype = nvram_bufget(RT2860_NVRAM,"dialchoicetype");
	if((choicetype==NULL)||strcmp(choicetype,"MANUAL")) //自动获取拨号信息
	{
		//dialnum  = nvram_bufget(RT2860_NVRAM,"dial3gnum_auto");
		//dialusername = nvram_bufget(RT2860_NVRAM,"dial3gusername_auto");
		//dialpassword = nvram_bufget(RT2860_NVRAM,"dial3gpassword_auto");
		//dialapn = nvram_bufget(RT2860_NVRAM,"dial3gapn_auto");
		File_Get_Modem_Info("dial3gnum_auto", dialnum);
		File_Get_Modem_Info("dial3gusername_auto", dialusername);
		File_Get_Modem_Info("dial3gpassword_auto", dialpassword);
		File_Get_Modem_Info("dial3gapn_auto", dialapn);
		
	}		
	else
	{
		//dialnum  = nvram_bufget(RT2860_NVRAM,"dial3gnum");
		//dialusername = nvram_bufget(RT2860_NVRAM,"dial3gusername");
		//dialpassword = nvram_bufget(RT2860_NVRAM,"dial3gpassword");
		//dialapn = nvram_bufget(RT2860_NVRAM,"dial3gapn");
		File_Get_Modem_Info("dial3gnum", dialnum);
		File_Get_Modem_Info("dial3gusername", dialusername);
		File_Get_Modem_Info("dial3gpassword", dialpassword);
		File_Get_Modem_Info("dial3gapn", dialapn);
	}			
	
	if(!g_TD_LC631)
	{
		if(g_option_modem == 1)
			WriteWcdmaApn(dialapn,"/root/ttyHS0");
		//else
			//WriteWcdmaApn(dialapn,"/var/ttyACM0");
	}				
	
	if(g_option_modem == 1)
	{
		Dial_Option_Modem(dialusername,dialpassword);
	}
#endif

	return ;
}

void unDeviceInfo()
{
		
	//create_ipaddr("xxx.xxx.xxx.xxx");
	//create_dialstatus(0);

	FILE *fp = NULL;
	char part[30];	
	
	unlink(STR_IN_FILE);
	unlink("/var/ttyAT");

	//eval("killall","-9","checkat");
	//eval("killall","-15","pppd");
	system("killall -9 checkat 1>/dev/null 2>&1");
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

