
/*
  main function
  lktOS init config moudle for all platform 
  
  v1.0 support mt7620 & ra5350  by luot  20130211
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include	<signal.h>
#include 	<sys/wait.h>
#include <errno.h>

#include	"linux/autoconf.h"  //kernel config
#include	"config/autoconf.h" //user config
#include	"user/busybox/include/autoconf.h" //busybox config

#include "nvram.h"
#include "lktInitConfig.h"


/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})
#define FPRINT_NUM(x) fprintf(fp, #x"=%d\n", atoi(nvram_bufget(ralinkMode, #x)));
#define FPRINT_STR(x) fprintf(fp, #x"=%s\n", nvram_bufget(ralinkMode, #x));
#define FPRINT_CONFIG_STR(x,y) fprintf(fp, #y" %s\n", nvram_bufget(ralinkMode, #x));

static int _eval(char *const argv[], char *path, int timeout, int *ppid)
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

static int get_interface_ralink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,char *wan_if, char *lan_if,char *wan_ppp_if)
{	
	char *opmode=NULL;
	char *wan_mode=NULL;
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		opmode = nvram_bufget(RT2860_NVRAM,"OperationMode");
		strcpy(wan_if,"br0");
		strcpy(wan_ppp_if,"br0");
		strcpy(lan_if,"br0");	/* WAN interface name -> wan_if */
		wan_mode=nvram_bufget(RT2860_NVRAM,"wanConnectionMode");
		if(!strcmp(opmode, "0"))
		{
			strcpy(wan_if,"br0");
		}
		else if(!strcmp(opmode,"1"))
		{
			strcpy(wan_if,"eth2.2");
		}
		else if(!strcmp(opmode,"2"))
		{
			strcpy(wan_if,"ra0");
		}
		else if(!strcmp(opmode,"3"))
		{
			strcpy(wan_if,"apcli0");
		}
		else
		{
		}
		if(!strcmp(wan_mode,"PPPOE") || !strcmp(wan_mode,"L2TP") || !strcmp(wan_mode, "PPTP")|| !strcmp(wan_mode, "3G"))
		{
			strcpy(wan_ppp_if,"ppp0");
		}
		else
		{
			strcpy(wan_ppp_if,wan_if);
		}
		//in ra5350 and mt7620 lan name always br0
		return 1;
	}
	else
	{
		printf("\r\nunknow chip type");
		return 0;
	}
}

static void genDevNodeRalink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform)
{	
	FILE *fp=NULL;
	
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		//system("mount | grep mdev | wc -l");
		eval("mount", "-t", "ramfs", "mdev", "/dev");
		eval("mkdir", "/dev/pts");
		eval("mount", "-t", "devpts", "devpts", "/dev/pts");
		//eval("mdev", "-s");
		eval("mknod", "/dev/video0", "c", "81", "0"); 
		eval("mknod", "/dev/ppp", "c", "108", "0","$cons"); 
		eval("mknod", "/dev/spiS0", "c", "217", "0");
		eval("mknod", "/dev/i2cM0", "c", "218", "0");
		eval("mknod", "/dev/rdm0", "c", "253", "0"); 
		eval("mknod", "/dev/flash0", "c", "200", "0");
		eval("mknod", "/dev/swnat0", "c", "210", "0");
		eval("mknod", "/dev/hwnat0", "c", "220", "0");
		eval("mknod", "/dev/acl0", "c", "230", "0");
		eval("mknod", "/dev/ac0", "c", "240", "0");
		eval("mknod", "/dev/mtr0", "c", "250", "0");
		eval("mknod", "/dev/gpio", "c", "252", "0");
		eval("mknod", "/dev/pcm0", "c", "233", "0"); 
		eval("mknod", "/dev/i2s0", "c", "234", "0");
		eval("mknod", "/dev/cls0", "c", "235", "0");
		eval("mknod", "/dev/nvram", "c", "251", "0");
		    
		fp=fopen("/etc/mdev.conf","w");	
		if(fp==NULL)
			return;	
		fprintf(fp,"# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]\n");
		fprintf(fp,"# The special characters have the meaning:\n");
		fprintf(fp,"# @ Run after creating the device.\n");
		fprintf(fp,"# $ Run before removing the device.\n");
		fprintf(fp,"# * Run both after creating and before removing the device.\n");
		fprintf(fp,"sd[a-z][1-9] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fprintf(fp,"sd[a-z] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fprintf(fp,"mmcblk[0-9]p[0-9] 0:0 0660 */sbin/automount_boot.sh $MDEV\n");
		fprintf(fp,"mmcblk[0-9] 0:0 0660 */sbin/automount_boot.sh $MDEV\n");
	#if defined(CONFIG_USB_SERIAL) || defined(CONFIG_USB_SERIAL_MODULE)
		fprintf(fp,"ttyUSB0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
	#if defined(CONFIG_BLK_DEV_SR) || defined(CONFIG_BLK_DEV_SR_MODULE)
		fprintf(fp,"sr0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
	#if defined(CONFIG_USB_SERIAL_HSO) || defined(CONFIG_USB_SERIAL_HSO_MODULE)
		fprintf(fp,"ttyHS0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
		fclose(fp);
	}
	
}

static void setMDEVRalink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform)
{	
	FILE *fp=NULL;
	
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{    
		fp=fopen("/etc/mdev.conf","w");	
		if(fp==NULL)
			return;	
		fprintf(fp,"# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]\n");
		fprintf(fp,"# The special characters have the meaning:\n");
		fprintf(fp,"# @ Run after creating the device.\n");
		fprintf(fp,"# $ Run before removing the device.\n");
		fprintf(fp,"# * Run both after creating and before removing the device.\n");
		fprintf(fp,"sd[a-z][1-9] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fprintf(fp,"sd[a-z] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fprintf(fp,"mmcblk[0-9]p[0-9] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fprintf(fp,"mmcblk[0-9] 0:0 0660 */sbin/automount.sh $MDEV\n");
	#if defined(CONFIG_USB_SERIAL) || defined(CONFIG_USB_SERIAL_MODULE)
		fprintf(fp,"ttyUSB0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
	#if defined(CONFIG_BLK_DEV_SR) || defined(CONFIG_BLK_DEV_SR_MODULE)
		fprintf(fp,"sr0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
	#if defined(CONFIG_USB_SERIAL_HSO) || defined(CONFIG_USB_SERIAL_HSO_MODULE)
		fprintf(fp,"ttyHS0 0:0 0660 @/sbin/autoconn3G.sh connect\n");	
	#endif
		fclose(fp);
		fp=fopen("/proc/sys/kernel/hotplug","w");
		if(fp==NULL)
			return;	
		fprintf(fp,"/sbin/mdev");
		fclose(fp);
	}
	
}

static void genSysFilesRalink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform)
{	
	char *login=NULL;
	char *pass=NULL;
	char cmd[80];

	FILE *fp=NULL;

	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		login=nvram_get(RT2860_NVRAM,"Login");
		//pass=nvram_get(RT2860_NVRAM,"Password");
		pass="wibacom2010";
		memset(cmd,0,sizeof(cmd));

		if(strcmp(login,"") && strcmp(pass, ""))
		{
			fp=fopen("/etc/passwd","w");
			if(fp==NULL)
				return;
			fprintf(fp,"%s::0:0:Adminstrator:/:/bin/sh",login);
			fclose(fp);

			fp=fopen("/etc/group","w");
			if(fp==NULL)
				return;
			fprintf(fp,"%s:x:0:%s",login,login);
			fclose(fp);
			eval("chpasswd.sh", login, pass); //use the script to do this
		}

		#ifdef CONFIG_PPPOL2TP
			fp=fopen("/etc/services","w");
			if(fp==NULL)
				return;
			fprintf(fp,"l2tp 1701/tcp l2f");
			fprintf(fp,"l2tp 1701/udp l2f");
			fclose(fp);
		#endif
	}
	else
	{
	}
	return;
}

static void set_vlan_map_ralink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,char *para)
{
#ifdef CONFIG_RAETH_QOS_PORT_BASED
	
	int i=0;
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		/* vlan priority tag => skb->priority mapping*/
		for(i=0;i<8;i++)
			eval("vconfig", "set_ingress_map", para, i, i);
		/* skb->priority => vlan priority tag mapping*/	
		for(i=0;i<8;i++)
			eval("vconfig", "set_egress_map", para, i, i);
	}
	else
	{
	}
	return;
#endif
}

static void addRax2Br0Ralink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,char *bssidnum)
{
	int ibssidnum,tmp;
	char cmd[80];
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		memset(cmd, 0, sizeof(cmd));
		ibssidnum=atoi(bssidnum);
		for(tmp=1;tmp<ibssidnum;tmp++)
		{
			if(ibssidnum>8)
				break;
			sprintf(cmd," ra%d",tmp);
			eval("brctl", "addif", "br0", cmd);	
		}
	}
	else
	{
	}
	return;
}

static void addWds2Br0Ralink(T_LKTOS_INITCONFIG_PLATFORM_TYPE_  platform)
{
	char *wds_en=NULL;

	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		wds_en=nvram_bufget(RT2860_NVRAM,"WdsEnable");
		if ( strcmp(wds_en,"0") )
		{
			eval("ifconfig", "wds0", "up");
			//eval("ifconfig", "wds1", "up");
			//eval("ifconfig", "wds2", "up");
			//eval("ifconfig", "wds3", "up");
			eval("brctl", "addif", "br0", "wds0");
			//eval("brctl", "addif", "br0", "wds1");
			//eval("brct", "addif", "br0", "wds2");
			//eval("brctl", "addif", "br0", "wds3");	
		}
	}
	else
	{
	}
	return;
	
}

static int ralinkInitConfigAll(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform)
{
	int ralinkMode=RT2860_NVRAM;

	char *wan_mac=NULL;	
	char *opmode=NULL;	
	char *bssidnum=NULL;
	char *wan_mode=NULL;
	char lan_if[32];
	char wan_if[32];
	char wan_ppp_if[32];
	char para[16], flash_mac[18];
	char tmpbuf[64]={0};
	int i=0,intVal=0;
	int opMode=0;

	
	memset(lan_if, 0, sizeof(lan_if));	
	memset(wan_if, 0, sizeof(wan_if));	
	memset(wan_ppp_if, 0, sizeof(wan_ppp_if));	
	memset(para, 0, sizeof(para));
	opmode=nvram_bufget(RT2860_NVRAM,"OperationMode");	
	bssidnum=nvram_bufget(RT2860_NVRAM,"BssidNum");
	wan_mode=nvram_bufget(RT2860_NVRAM, "wanConnectionMode");

	get_interface_ralink(platform,wan_if,lan_if,wan_ppp_if);
	genDevNodeRalink(platform);
	genSysFilesRalink(platform);
	printf("\r\ninit debug == %s--%s--%s--%s\r\n",wan_mode,wan_if,lan_if,wan_ppp_if);
	eval("insmod", "lm");
	eval("insmod", "dwc_otg");
	eval("ifconfig", "eth2", "0.0.0.0");
	eval("brctl","addbr","br0");
	if(atoi(nvram_bufget(ralinkMode, "stpEnabled")))
	{
		eval("brctl","setfd","br0","15");
		eval("brctl","stp","br0","1");
	}
	else
	{
		eval("brctl","setfd","br0","1");
		eval("brctl","stp","br0","0");
	}
	intVal=lktos_wificonfig_initall_by_filemode(platform,tmpbuf);
	if(!intVal)
	{
		printf("\r\ninit wifi field");
	}
	//nvram_init(ralinkMode);
	eval("vconfig", "add", "eth2", "1");
	sprintf(para,"eth2.1");	
	set_vlan_map_ralink(platform,para);	
	eval("vconfig", "add", "eth2", "2");
	sprintf(para,"eth2.2");
	set_vlan_map_ralink(platform,para);
	eval("ifconfig", "eth2.1", "0.0.0.0");
	eval("ifconfig", "eth2.2", "0.0.0.0");
	eval("ifconfig", "lo", "127.0.0.1");
	eval("iptables", "--flush");
	eval("iptables", "--flush", "-t", "nat");
	eval("iptables", "--flush", "-t", "mangle");
	opMode = atoi(nvram_bufget(ralinkMode, "OperationMode"));
	if(1 == opMode)
	{
	#ifdef CONFIG_WAN_AT_P0
		eval("config-vlan.sh", "2", "WLLLL");
	#else
		eval("config-vlan.sh", "2", "LLLLW");
	#endif
		eval("brctl", "addif", "br0", "eth2.1");
		eval("brctl", "addif", "br0", "ra0");
	}
	else if(0 == opMode)
	{
		eval("brctl", "addif", "br0", "eth2.1");
		eval("brctl", "addif", "br0", "eth2.2");
		eval("brctl", "addif", "br0", "ra0");
		intVal = atoi(nvram_bufget(ralinkMode, "apClient"));
		if(intVal)
		{
			eval("ifconfig", "apcli0", "up");
			eval("brctl", "addif", "br0", "apcli0");
		}
		
	}
	else if(2 == opMode)
	{
		eval("brctl", "addif", "br0", "eth2.1");
	}
	else
	{
		printf("\r\nerror opmode type\r\n");
	}
	bssidnum=nvram_bufget(RT2860_NVRAM,"BssidNum");
#ifdef CONFIG_RT2860V2_AP_MBSS
	if( strcmp(bssidnum, "1" ))
	{
		addRax2Br0Ralink(platform,bssidnum);
	}
#endif
	addWds2Br0Ralink(platform);
	intVal=lktos_networkconfig_init_wan(platform,tmpbuf);
	if(!intVal)
	{
		printf("\r\ninit wan field\r\n");
	}
	intVal=lktos_networkconfig_init_lan(platform,tmpbuf);
	if(!intVal)
	{
		printf("\r\ninit lan field\r\n");
	}
	if(opMode != 0)
	{
		eval("nat.sh");
		system("echo 1 > /proc/sys/net/ipv4/ip_forward");
	}
	eval("route", "add", "-host", "255.255.255.255", "dev", lan_if);
	eval("mdev","-s");
	setMDEVRalink(platform);
	return 1;
}

int lktos_initconfig_init_all(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned char* errormsg)
{
	int resultFlag=0;

	strcpy(errormsg,"nothing!");
	if(errormsg == NULL)
	{
		printf("\r\n the second papa error");
		return resultFlag;
	}
	if((platform <= UNDEFINED) || (platform >= UNKNOW) )
	{
		printf("\r\n the first para(platform type) error");
		return resultFlag;
	}
	switch(platform)
	{
		case RALINK3052_STD:
			
			break;
		case RALINK3050_STD:
			
			break;
		case RALINK5350_STD:
			resultFlag=ralinkInitConfigAll(platform);
			//system("nat.sh");
			break;
		case MTK7620_STD:
			resultFlag=ralinkInitConfigAll(platform);
			//system("nat.sh");
			break;
		default:
			strcpy(errormsg,"wrong palt form!");
			return resultFlag;
			
	}
	return resultFlag;
}



