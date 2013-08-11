#include 	<stdio.h>
#include <dirent.h>
#include <linux/if.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include 	<sys/stat.h>
#include 	"rdm.h"

#define IFC_STATUS_UNKNOWN          0
#define IFC_STATUS_NEW              1
#define IFC_STATUS_AUTH_ERR         2
#define IFC_STATUS_DOWN             3
#define IFC_STATUS_DHCPC_PROGRESS   4
#define IFC_STATUS_DHCPC_FAIL       5
#define IFC_STATUS_UP               6

#define PPPDEFGW		0x0A707070
#define PPPDEFIP		0x0A404040
#define _PPPoE_ENABLE        3
#define _CONNECT             1
#define _DISCONNECT          4
#define _LOGIN_IN_PROGRESS   2

#define PPP_PAP_FILE	        "/var/config/ppp/pap-secrets"
#define PPP_CHAP_FILE	        "/var/config/ppp/chap-secrets"
#define PPP_3G_FILE				"/var/config/ppp/peers/3g"
#define PPP_3G_CONN_SCR			"/var/config/ppp/peers/Generic_conn.scr"
#define PPP_3G_DISCON_SCR		"/var/config/ppp/peers/Generic_disconn.scr"

#ifdef NOTDDIAL
#define GPIO_PORT_3G	13
#else
#ifdef SET_PASSWORD
#define GPIO_PORT_3G	13
#else
#define GPIO_PORT_3G	17
#endif
#endif

#define GPIO_3G_LED2		20

#define GPIO_ONE_STEP		0
#define GPIO_TWO_STEP		21
#define GPIO_THREE_STEP		27

#define DEBUG
#ifdef DEBUG
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/ttyS1", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
#else
#define cprintf(fmt, args...)
#endif

typedef struct proc_s {
	char cmd[16];					/* basename of executable file in call to exec(2) */
	int ruid;						/* real only (sorry) */
	int pid;						/* process id */
	int ppid;						/* pid of parent process */
	char state;						/* single-char code for process state (S=sleeping) */
	unsigned int vmsize;			/* size of process as far as the vm is concerned */
} proc_t;

typedef struct  tagCDMAInfo
{
	unsigned int 			ModuleType;
  	int 					csq;
	int 					mobileNet;
	char 					Status[16];
	char 					operatorName[16]; 
	char 					moduleVersion[128]; 
}CDMAInfo,*PCDMAInfo;

typedef struct tagOptInfo
{
	unsigned char			*ID;			
	unsigned char			*optname;				//运营商名称
	unsigned char			*username;			//用户名
	unsigned char			*passwd;			//密码
	unsigned char			*papn;				//该运营商所对应的APN
	unsigned char 			*dialnum;
}OptInfo, *POptInfo;

typedef struct tagOptionNetS
{
	unsigned char 	ip[16];
	unsigned char 	dns1[16];
	unsigned char 	dns2[16];
	unsigned char 	res[16];
}OptionNetS,*POptionNetS;


extern void display_signal(int sms,int signal);
extern void led_signal_ctl(int gpio, int on);
extern void gpio27_ctl(int type);

extern int DBgetPid(char *progname);
extern int DBgetPPPStatus( char *pppname, char *subpty );
extern int file2str_ex(char *filename, char *ret, int cap);
extern int parse_proc_status(char *S, proc_t * P);
void Dial_Option_Modem(void);

#if 0  //remove by hs, because our 3G don`t care led
/*
	@param: int type	2 use system register
						1 gpio27 high
						0 gpio27 low
*/
void gpio27_ctl(int type)
{
	int fd, method, offset = 0, value = 0;

	fd = open("/dev/rdm0", O_RDONLY);
	if (fd < 0)
	{
		printf("Open pseudo device failed\n");
		return 0;
	}

	switch(type)
	{
		case 0:
			method = RT_RDM_CMD_READ;
			offset = 0x60;
			ioctl(fd, method, &offset);
			cprintf("type=%d,0x%x\n", type,offset);
			value = offset|0x00400000;
			method = RT_RDM_CMD_WRITE;
			method = (method | (0x60 << 16));
			ioctl(fd, method, &value);
			break;
		case 1:
			method = RT_RDM_CMD_READ;
			offset = 0x60;
			ioctl(fd, method, &offset);
			cprintf("type=%d,0x%x\n", type,offset);
			value = offset&0xffbfffff;
			method = RT_RDM_CMD_WRITE;
			method = (method | (0x60 << 16));
			ioctl(fd, method, &value);
			break;
		case 2:
			method = RT_RDM_CMD_SET_BASE_SYS;
			offset = 0;
			ioctl(fd, method, offset);
			break;
	}
	close(fd);
}


/*
	@param: int gpio	gpio number
	@param	int on		0:dark 1:right
*/
void led_signal_ctl(int gpio, int on)
{
	unsigned char cmd[128];
	static	int  switchcount=0;

	memset(cmd, '\0', sizeof(cmd));

	if(27==gpio)// GPIO27 
	{
		if(0==switchcount)
		{
			//system("reg s 0"); // switch current base addr to 0xb0000000
			gpio27_ctl(2);
			switchcount = 1;
		}			
	}
	
	if(on)
	{
		if(27==gpio)
		{
			//sprintf(cmd, "reg w 60 25d"); // SPI_CS1 model
			gpio27_ctl(1);
		}			
		else
		{
			sprintf(cmd, "gpio l %d 4000 0 0 0 4000",gpio); //hign
			system(cmd);
		}			
	}
	else
	{
		if(27==gpio)
		{
			//sprintf(cmd, "reg w 60 40025d"); // GPIO model
			gpio27_ctl(0);
		}			
		else
		{
			sprintf(cmd, "gpio l %d 0 4000 0 0 4000",gpio); //low
			system(cmd);
		}			
	}
}


/*
	@param: int type 网络类型 2G 或 3G
	type = 2	2G  green
	type = 3 	3G	blue
*/
void display_net_led(int type)
{
	static int pre_net = -1, now_net = -1;
	int ret;

	ret = DBgetPPPStatus( "ppp0", "pppd" );
	if(ret != _CONNECT)
		return;

	if(type == 2)
	{
		now_net = 2;
		if(now_net != pre_net)
		{
			led_signal_ctl(GPIO_PORT_3G, 1);
			led_signal_ctl(GPIO_3G_LED2, 0);
		}
	}
	else if(type == 3)
	{
		now_net = 3;
		if(now_net != pre_net)
		{
			led_signal_ctl(GPIO_PORT_3G, 0);
			led_signal_ctl(GPIO_3G_LED2, 1);
		}
	}
	
	pre_net = now_net;
}

/*
	@param: int signal 信号强度的数值
	signal == 0			0 step
	0<signal<120 		1 step
	120<= singal <140 	2 step
	140 <= signal 		3 step
*/
void display_signal(int sms,int signal)
{
	static int pre_status=-1, now_status=-1;
	int i=0;

	if(sms) //have sms
	{
		for(i=0; i<3; i++)
		{
			//led_signal_ctl(GPIO_ONE_STEP, 0);
			led_signal_ctl(GPIO_TWO_STEP, 0);
			led_signal_ctl(GPIO_THREE_STEP, 0);
			sleep(1);

			//led_signal_ctl(GPIO_ONE_STEP, 1);
			led_signal_ctl(GPIO_TWO_STEP, 1);
			led_signal_ctl(GPIO_THREE_STEP, 1);
			sleep(2);
		}

		return;
	}
		
	
	if((99 == signal)||(0 == signal)||(199 == signal)||(100 == signal))
		signal = 0;

	if(signal <= 0 )
	{
		now_status = 0;
		if(now_status != pre_status)
		{
			//led_signal_ctl(GPIO_ONE_STEP, 0);
			led_signal_ctl(GPIO_TWO_STEP, 0);
			led_signal_ctl(GPIO_THREE_STEP, 0);
		}		
	}
	else if(signal < 99)
	{
		if(signal < 10)
		{
			now_status = 1;
			if(now_status != pre_status)
			{
				//led_signal_ctl(GPIO_ONE_STEP, 1);
				led_signal_ctl(GPIO_TWO_STEP, 0);
				led_signal_ctl(GPIO_THREE_STEP, 0);
			}
		}
		else if(signal < 21)
		{
			now_status = 2;
			if(now_status != pre_status)
			{
				//led_signal_ctl(GPIO_ONE_STEP, 1);
				led_signal_ctl(GPIO_TWO_STEP, 1);
				led_signal_ctl(GPIO_THREE_STEP, 0);
			}
		}
		else
		{
			now_status = 3;
			if(now_status != pre_status)
			{
				//led_signal_ctl(GPIO_ONE_STEP, 1);
				led_signal_ctl(GPIO_TWO_STEP, 1);
				led_signal_ctl(GPIO_THREE_STEP, 1);
			}
		}
	}
	else if(signal < 120)
	{
		now_status = 1;
		if(now_status != pre_status)
		{
			//led_signal_ctl(GPIO_ONE_STEP, 1);
			led_signal_ctl(GPIO_TWO_STEP, 0);
			led_signal_ctl(GPIO_THREE_STEP, 0);
		}
	}
	else if((signal>=120)&&(signal<140))
	{
		now_status = 2;
		if(now_status != pre_status)
		{
			//led_signal_ctl(GPIO_ONE_STEP, 1);
			led_signal_ctl(GPIO_TWO_STEP, 1);
			led_signal_ctl(GPIO_THREE_STEP, 0);
		}
	}
	else if(signal>=140)
	{
		now_status = 3;
		if(now_status != pre_status)
		{
			//led_signal_ctl(GPIO_ONE_STEP, 1);
			led_signal_ctl(GPIO_TWO_STEP, 1);
			led_signal_ctl(GPIO_THREE_STEP, 1);
		}
	}
	pre_status = now_status;
}
#else

void gpio27_ctl(int type)
{
	printf("type is %d\n", type);
}

void led_signal_ctl(int gpio, int on)
{
	printf("gpio is %d\n", gpio);
	printf("on is %d\n", on);
}

void display_net_led(int type)
{
	printf("type is %d\n", type);
}

void display_signal(int sms,int signal)
{
	printf("sms is %d\n", sms);
	printf("signal is %d\n", signal);
}
#endif

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



//add 2009.04.30 by lei 
OptInfo OptName[]={
{"20201",    "COSMOTE", NULL,  NULL, "internet", "*99***1#"},
{"20205",    "vodafone GR", NULL,  NULL, "internet.vodafone.gr", "*99***1#"},
{"20209",    "Q-TELECOM", NULL,  NULL, NULL, "*99***1#"},
{"20210",    "Telestet", NULL,  "24680", "gnet.b-online.gr", "*99***1#"},
{"20210",    "TIM", "web",  "web", "gint.b-online.gr", "*99***1#"},
{"20420",    "Orange Nederland N.V.", NULL,  NULL, NULL, "*99***1#"},
{"20408",    "KPN Mobile Netherlands"  ,"KPN",  "gprs",  "internet" , "*99***1#"},
{"20412",    "O2 Netherlands"  ,NULL,  NULL,  "internet" , "*99***1#"},
{"20451",    "Telfort Netherlands"  ,"telfortnl",  "password",  "internet" , "*99***1#"},
{"20416",    "T-Mobile (Ben) Netherlands"  ,NULL,  NULL,  "internet-act" , "*99***1#"},
{"20404",    "Vodafone Netherlands"  ,"vodafone",  "vodafone",  "web.vodafone.nl" , "*99***1#"},
{"20404",    "Vodafone (Business) Netherlands"  ,"vodafone",  "vodafone",  "office.vodafone.nl" , "*99***1#"},
{"20404",    "Vodafone (Live) Netherlands"  ,"vodafone",  "vodafone",  "live.vodafone.com", "*99***1#" },
{"20610",    "Mobistar Belgium"  ,"mobistar ",  "mobistar",  "web.pro.be" , "*99***1#"}, 
{"20601",    "Proximus Belgium"  ,NULL,  NULL,  "internet.proximus.be" , "*99***1#"},
{"20620",    "BASE(orange)", NULL,  NULL,  "orangeinternet" , "*99***1#"},
{"20620",    "BASE(Pro)", "BusinessInternet",  "busint", "busint.base.be", "*99***1#"},
{"20801",    "Orange France"  ,"orange",  "orange",  "orange.fr" , "*99***1#"},
{"20801",    "Orange France Prépayé" , NULL,  NULL, NULL, "*99***1#"},
{"20801",    "Orange MIB France","mportail","mib","orange-mib" , "*99***1#"}, 
{"20801",    "Orange France(orange.ie)"  ,"orange",  "orange",  "orange.ie", "*99***1#" },
{"20801",    "Orange France(enterprise)"  ,"orange",  "orange",  "internet-entreprise", "*99***1#" },
{"20810",    "SFR France" , NULL,  NULL, "websfr" , "*99***1#"}, 
{"20810",    "SFR France(EEEPC)" , NULL,  NULL, "slsfr", "*99***1#" }, 
{"20820",    "Bouygues Telecom France" , NULL,  NULL, "ebouygtel.com" , "*99***1#"},
{"20820",    "B2BouygTel France" , NULL,  NULL, "b2bouygtel.com" , "*99***1#"},
{"20820",    "A2BouygTel France" , NULL,  NULL, "a2bouygtel.com", "*99***1#" },
{"20820",    "FIPBouygTel France" , NULL,  NULL, "fipbouygtel.com", "*99***1#" },
{"20820",    "VPNBouygTel France" , NULL,  NULL, "vpnbouygtel.com", "*99***1#" },
{"20813",    "SFR", NULL,  NULL, NULL, "*99***1#"},
{"21210",    "Monaco Telecom", NULL,  NULL, NULL, "*99***1#"},
{"21303",    "MOBILAND", NULL,  NULL, NULL,"*99***1#"},
{"21403",    "Orange Amena Spain"  ,"CLIENTE",  "AMENA",  "internet" , "*99***1#"},
{"21407",    "Telefonica Movistar Spain"  ,"movistar",  "movistar",  "movistar.es" , "*99***1#"},
{"21401",    "Vodafone Airtel Spain"  ,"vodafone",  "vodafone",  "airtelnet.es" , "*99***1#"},
{"21404",    "Yoigo", NULL,  NULL, NULL, "*99***1#"},
{"21601",    "Pannon(contract)", NULL,  NULL, "next","*99***1#"},
{"21601",    "Pannon(flat rate)", NULL,  NULL, "next","*99***1#"},
{"21601",    "Pannon(compressed)", NULL,  NULL, "snext","*99***1#"},
{"21630",    "Westel Tmobile(contract)", NULL,  NULL, "internet","*99***1#"},
{"21670",    "Vodafone(contract,compressed)", NULL,  NULL, "internet.vodafone.net", "*99***1#"},
{"21670",    "Vodafone(contract,uncompressed)", NULL,  NULL, "standardnet.vodafone.net", "*99***1#"},
{"21670",    "Vodafone(pre-pay,compressed)", NULL,  NULL, "vitamax.internet.vodafone.net", "*99***1#"},
{"21670",    "Vodafone(pre-pay,uncompressed)", NULL,  NULL, "vitamax.snet.vodafone.net", "*99***1#"},
{"21803",    "ERONET", NULL,  NULL, NULL,"*99***1#"},
{"21805",    "m:tel", NULL,  NULL, NULL,"*99***1#"},
{"21890",    "GSMBIH", NULL,  NULL, NULL,"*99***1#"},
{"21901",    "T-Mobile", "38591",  "38591", "web.htgpr","*99***1#"},
{"21910",    "VIPNET Start", "38591",  "38591", "gprs0.vipnet.hr", "*99***1#"},
{"21910",    "VIPNET Pro", "38591",  "38591", "gprs5.vipnet.hr", "*99***1#"},
{"21910",    "VIPNET 3G", "38591",  "38591", "3g.vip.hr", "*99***1#"},
{"22001",    "Mobtel Srbija", "mobtel",  "gprs", "internet", "*99***1#"},
{"22002",    "PROMONTE GSM", NULL,  NULL, NULL,"*99***1#"},
{"22003",    "Telekom Srbija", "mts",  "64", "gprsinternet", "*99***1#"},
{"22004",    "T-Mobile", NULL,  NULL, NULL,"*99***1#"},
{"22005",    "Vip SRB", NULL,  NULL, NULL,"*99***1#"},
{"22298",    "Blu (Contratto) Italy"  ,NULL,  NULL,  "INTERNET", "*99***1#" },
{"22298",    "Blu (Prepagata) Italy"  ,NULL,  NULL,  "PINTERNET", "*99***1#" },
{"22299",    "H3G Italy"  ,NULL,  NULL,  "tre.it" ,"*99***1#"},
{"22210",    "Vodafone (Omnitel) Italy"  ,NULL,  NULL,  "web.omnitel.it" ,"*99***1#"},
{"22201",    "TIM Italy"  ,NULL,  NULL,  "ibox.tim.it" ,"*99***1#"},
{"22288",    "Wind Italy"  ,"wind",  "wind",  "internet.wind" ,"*99***1#"},
{"22601",    "Connex Vodafone Romania"  ,"internet.connex.ro",  "connex",  "internet.connex.ro" ,"*99***1#"},
{"22610",    "Orange Romania"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"22603",    "COSMOTE", NULL,  NULL, NULL,"*99***1#"},
{"22803",    "Orange CH Switzerland"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"22802",    "Sunrise CH Switzerland"  ,"internet",  "internet",  "internet" ,"*99***1#"},
{"22801",    "Swisscom Switzerland"  ,NULL,  NULL,  "gprs.swisscom.ch" ,"*99***1#"},
{"22807",    "in&phone", NULL,  NULL, NULL,"*99***1#"},
{"22808",    "Tele2 Switzerland", NULL,  NULL, NULL,"*99***1#"},
{"23001",    "T-Mobile", NULL,  NULL, "internet.t-mobile.cz", "*99***1#"},
{"23002",    "Telefonica(contract)", NULL,  NULL, "internet", "*99***1#"},
{"23002",    "Telefonica(Go)", NULL,  NULL, "gointernet", "*99***1#"},
{"23003",    "OSKAR", NULL,  NULL, NULL,"*99***1#"},
{"23099",    "Vodafone(contract)", NULL,  NULL, "internet", "*99***1#"},
{"23099",    "Vodafone(prepaid)", NULL,  NULL, "ointernet", "*99***1#"},
{"23101",    "Orange Globtel", NULL,  NULL, "internet", "*99***1#"},
{"23102",    "Eurotel", NULL,  NULL, "internet", "*99***1#"},
{"23106",    "O2", NULL,  NULL, NULL, "*99***1#"},
{"23212",    "one", NULL,  NULL, NULL,"*99***1#"},
{"23210",    "Drei Austria"  ,NULL,  NULL,  "drei.at", "*99***1#" },
{"23212",    "Max Online Metro Austria"  ,"GPRS",  NULL,  "gprsmetro" ,"*99***1#"},
{"23203",    "T-Mobile A (Max Online) Austria"  ,"GPRS",  NULL,  "gprsinternet " ,"*99***1#"},
{"23203",    "T-Mobile A (Business) Austria"  ,"GPRS",  NULL,  "business.gprsinternet" ,"*99***1#"},
{"23201",    "Mobilkom A1 Austria"  ,"ppp@a1plus.at",  "ppp",  "A1.net" ,"*99***1#"},
{"23205",    "OneNet Austria"  ,"web",  "web",  "web.one.at" ,"*99***1#"},
{"23207",    "tele.ring Austria"  ,"web@telering.at",  "web",  "web" ,"*99***1#"},
{"23420",    "3 UK"  ,NULL,  NULL,  "three.co.uk" ,"*99***1#"},
{"23450",    "Jersey Telecom UK"  ,"abc",  "abc",  "pepper" ,"*99***1#"},
{"23410",    "O2 UK"  ,"web",  "password",  "mobile.o2.co.uk" ,"*99***1#"},
{"23410",    "O2 Payandgo UK"  ,"faster",  "password",  "mobile.o2.co.uk" ,"*99***1#"},
{"23410",    "O2 Prepaid UK"  ,"payandgo",  "payandgo",  "payandgo.o2.co.uk", "*99***1#" },
{"23411",    "O2 Payandgo UK"  ,"faster",  "password",  "mobile.o2.co.uk" ,"*99***1#"},
{"23433",    "Orange UK"  ,"user",  "pass",  "orangeinternet" ,"*99***1#"},
{"23433",    "Orange Payandgo UK"  ,"Multimedia",  "Orange",  "orangewap" ,"*99***1#"},
{"23432",    "T-Mobile UK"  ,"user",  "pass",  "general.t-mobile.uk" ,"*99***1#"},
{"23432",    "Virgin Mobile UK"  ,"user",  NULL,  "goto.virginmobile.com" ,"*99***1#"},
{"23415",    "Vodafone UK"  ,"web",  "web",  "internet" ,"*99***1#"},
{"23415",    "Vodafone UK(prepaid)"  ,"web",  "web",  "pp.vodafone.co.uk", "*99***1#" },
{"23430",    "T-Mobile UK", "user",  "pass", "general.t-mobile.uk", "*99***1#"},
{"23401",    "Virgin Mobile UK", "user",  NULL, "goto.virginmobile.com", "*99***1#"},
{"23455",    "Cable & Wireless Guernsey", NULL,  NULL, NULL,"*99***1#"},
{"23458",    "Pronto GSM", NULL,  NULL, NULL,"*99***1#"},
{"23806",    "3 Denmark"  ,NULL,  NULL,  NULL ,"*99***1#"},
{"23830",    "Orange Denmark"  ,NULL,  NULL,  "web.orange.dk" ,"*99***1#"},
{"23802",    "Sonofon Denmark"  ,NULL,  NULL,  NULL ,"*99***1#"},
{"23801",    "TDC Denmark"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"23820",    "Telia DK", NULL,  NULL, NULL,"*99***1#"},
{"24007",    "Tele2 Comviq Sweden"  ,"gprs",  "internet",  "isplnk1.swip.net" ,"*99***1#"},
{"24007",    "Tele2 3G Sweden"  ,NULL,  NULL,  "internet.tele2.se" ,"*99***1#"},
{"24001",    "Telia Sweden"  ,NULL,  NULL,  "online.telia.se" ,"*99***1#"},
{"24002",    "Tre (3G) Sweden"  ,NULL,  NULL,  "data.tre.se" ,"*99***1#"},
{"24008",    "Vodafone (GPRS) Sweden"  ,NULL,  NULL,  "internet.vodafone.net" ,"*99***1#"},
{"24008",    "Vodafone (3G) Sweden"  ,NULL,  NULL,  "Services.vodafone.net" ,"*99***1#"},
{"24010",    "Swefour AB", NULL,  NULL, NULL,"*99***1#"},
{"24202",    "Netcom Norway"  ,NULL,  NULL,  "internet.netcom.no" ,"*99***1#"},
{"24205",    "Talkmore Norway"  ,NULL,  NULL,  "internet.netcom.no" ,"*99***1#"},
{"24206",    "TMN Portugal"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"24201",    "Telenor Mobil Norway"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"24201",    "Vodafone PT Portugal"  ,NULL,  NULL,  "internet.vodafone.pt" ,"*99***1#"},
{"24201",    "TELENOR", NULL,  NULL, NULL,"*99***1#"},
{"24203",    "Teletopia", NULL,  NULL, NULL,"*99***1#"},
{"24403",    "Dna Finland"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"24421",    "Elisa Finland"  ,"rlnet",  "internet",  "internet" ,"*99***1#"},
{"24405",    "Radiolinja Finland"  ,"rlnet",  "internet",  "internet" ,"*99***1#"},
{"24491",    "Sonera Finland"  ,NULL,  NULL,  "internet" ,"*99***1#"},
{"24491",    "Song Finland"  ,"song@internet",  "songnet",  "internet.song.fi" ,"*99***1#"},
{"24421",    "Saunalahti Finland"  ,NULL,  NULL,  "internet.saunalahti" ,"*99***1#"},
{"24412",    "dna", NULL,  NULL, NULL,"*99***1#"},
{"24414",    "Alands Mobiltelefon AB", NULL,  NULL, NULL,"*99***1#"},
{"24601",    "Omnitel(contract)", NULL,  NULL, "gprs.omnitel.net", "*99***1#"},
{"24601",    "Omnitel(no contract)", "omni",  "omni", "gprs.startas.lt", "*99***1#"},
{"24602",    "Bite GSM", NULL,  NULL, "banga", "*99***1#"},
{"24603",    "TELE2", NULL,  NULL, NULL,"*99***1#"},
{"24701",    "LMT", NULL,  NULL, "internet.lmt.lv", "*99***1#"},
{"24702",    "Tele2", "gprs",  "internet", "internet.tele2.lv", "*99***1#"},
{"24705",    "Bite Latvija", NULL,  NULL, NULL,"*99***1#"},
{"24801",    "EMT", NULL,  NULL, "internet.emt.ee", "*99***1#"},
{"24802",    "RLE", NULL,  NULL, "internet", "*99***1#"},
{"24803",    "Tele2", NULL,  NULL, NULL,"*99***1#"},
{"25001",    "MTS", "mts",  "mts", "internet.mts.ru", "*99***1#"},
{"25002",    "MegaFon(dv)", NULL,  NULL, "internet.dv", "*99***1#"},
{"25002",    "MegaFon(kvk)", NULL,  NULL, "internet.kvk", "*99***1#"},
{"25002",    "MegaFon(ltmsk)", NULL,  NULL, "internet.ltmsk", "*99***1#"},
{"25002",    "MegaFon(mc)", NULL,  NULL, "internet.mc", "*99***1#"},
{"25002",    "MegaFon(NWGSM)", NULL,  NULL, "internet.nw", "*99***1#"},
{"25002",    "MegaFon(Siberia)", NULL,  NULL, "internet.sib", "*99***1#"},
{"25002",    "MegaFon(UGSM)", NULL,  NULL, "internet.ugsm", "*99***1#"},
{"25002",    "MegaFon(usi)", NULL,  NULL, "internet.usi.ru", "*99***1#"},
{"25002",    "MegaFon(Volga)", NULL,  NULL, "internet.volga", "*99***1#"},
{"25003",    "NCC", "ncc",  NULL, "internet", "*99***1#"},
{"25004",    "SIBCHALLENGE", NULL,  NULL, NULL,"*99***1#"},
{"25005",    "Sayantelecom LLC", NULL,  NULL, NULL,"*99***1#"},
{"25007",    "BM Telecom", NULL,  NULL, NULL,"*99***1#"},
{"25007",    "SMARTS", NULL,  NULL, NULL,"*99***1#"},
{"25010",    "DTC", NULL,  NULL, NULL,"*99***1#"},
{"25012",    "AKOS", NULL,  NULL, NULL,"*99***1#"},
{"25012",    "Sakhalin GSM", NULL,  NULL, NULL,"*99***1#"},
{"25012",    "ULAN-UDE CELLULAR NETWORK", NULL,  NULL, NULL,"*99***1#"},
{"25012",    "Sibintertelecom", NULL,  NULL, NULL,"*99***1#"},
{"25013",    "Kuban-GSM", NULL,  NULL, NULL,"*99***1#"},
{"25015",    "SMARTS-Ufa", NULL,  NULL, NULL,"*99***1#"},
{"25016",    "NTC", NULL,  NULL, "internet.ntc", "*99***1#"},
{"25017",    "Utel (RUS17)", NULL,  NULL, NULL,"*99***1#"},
{"25019",    "INDIGO", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "Cellular Communications of Udmurtia", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "Tele2-Chelyabinsk", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "TELE2", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "MOTIV", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "TELE2-OMSK", NULL,  NULL, NULL,"*99***1#"},
{"25020",    "Votek Mobile", NULL,  NULL, NULL,"*99***1#"},
{"25028",    "EXTEL", NULL,  NULL, NULL,"*99***1#"},
{"25035",    "MOTIV", "beeline",  "beeline", "internet.beeline.ru", "*99***1#"},
{"25039",    "Utel (RUSUT)", NULL,  NULL, NULL,"*99***1#"},
{"25039",    "Uraltel", NULL,  NULL, NULL,"*99***1#"},
{"25092",    "PrimTel", NULL,  NULL, "internet.primtel.ru", "*99***1#"},
{"25099",    "Beeline", "beeline",  "beeline", "internet.beeline.ru", "*99***1#"},
{"25501",    "MTS UKR", NULL,  NULL, NULL,"*99***1#"},
{"25502",    "Beeline UA", NULL,  NULL, NULL,"*99***1#"},
{"25503",    "KYIVSTAR", NULL,  NULL, NULL,"*99***1#"},
{"25505",    "GOLDEN TELECOM", NULL,  NULL, NULL,"*99***1#"},
{"25506",    "life:)", NULL,  NULL, NULL,"*99***1#"},
{"25701",    "VELCOM", "web",  "web", "web.velcom.by", "*99***1#"},
{"25702",    "MTS", NULL,  NULL, NULL,"*99***1#"},
{"25704",    "BeST", NULL,  NULL, NULL,"*99***1#"},
{"25901",    "VoXtel S.A", NULL,  NULL, NULL,"*99***1#"},
{"25902",    "MOLDCELL", "gprs",  "gprs", "internet", "*99***1#"},
{"25904",    "Eventis Mobile S.R.L.", NULL,  NULL, NULL,"*99***1#"},
{"26001",    "Polkomtel", NULL,  NULL, "www.plusgsm.pl", "*99***1#"},
{"26002",    "Era", "erainternet",  "erainternet", "erainternet", "*99***1#"},
{"26002",    "Heyah", "heyah",  "heyah", "heyah.pl", "*99***1#"},
{"26003",    "Idea", "idea",  "idea", "www.idea.pl", "*99***1#"},
{"26003",    "Orange", "internet",  "internet", "internet", "*99***1#"},
{"26201",    "T-Mobile Germany"  ,"td1 ",  "gprs",  "internet.t-d1.de" ,"*99***1#"},
{"26202",    "Vodafone.de Germany (Web)" , NULL,  NULL, "web.vodafone.de" ,"*99***1#"}, 
{"26202",    "Vodafone.de Germany (Event)" , NULL,  NULL, "event.vodafone.de" ,"*99***1#"}, 
{"26203",    "E-Plus Germany"  ,"eplus ",  "gprs",  "internet.eplus.de" ,"*99***1#"}, 
{"26208",    "O2 GPRS Germany"  ,NULL,  NULL,  "internet " ,"*99***1#"},  
{"26208",    "O2 Prepaid Germany"  ,NULL,  NULL,  "pinternet.interkom.de" ,"*99***1#"}, 
{"26208",    "O2 3G Germany"  ,NULL,  NULL,  "surfo2 " ,"*99***1#"}, 
{"26207",    "O2 (Germany) GmbH & Co. OHG", NULL,  NULL, NULL,"*99***1#"},
{"26601",    "GIBTEL", NULL,  NULL, NULL,"*99***1#"},
{"26803",    "Optimus Portugal"  ,NULL,  NULL,  "internet " ,"*99***1#"},
{"26801",    "vodafone PT Portugal", NULL,  NULL, "internet.vodafone.pt", "*99***1#"},
{"26806",    "TMN Portugal", NULL,  NULL, "internet", "*99***1#"},
{"27001",    "LUXGSM Luxemburg"  ,NULL,  NULL,  "web.pt.lu" ,"*99***1#"},
{"27077",    "Tango Luxemburg"  ,"tango ",  "tango",  "internet" ,"*99***1#"},
{"27077",    "Tango Luxemburg(HSPA)"  ,"tango ",  "tango",  "HSPA", "*99***1#" },
{"27099",    "VOXmobile  Luxemburg"  ,NULL,  NULL,  "vox.lu" ,"*99***1#"},
{"27202",    "O2 Ireland" , "gprs",  "gprs",  "open.internet " ,"*99***1#"},
{"27202",    "O2 Prepaid Ireland" , "gprs",  "gprs",  "pp.internet" ,"*99***1#"},
{"27201",    "Vodafone Ireland" , "vodafone",  "vodafone",  "isp.vodafone.ie" ,"*99***1#"},
{"27201",    "Vodafone Prepaid Ireland" , "vodafone",  "vodafone",  "live.codafone.com" ,"*99***1#"},
{"27203",    "Meteor", NULL,  NULL, NULL,"*99***1#"},
{"27205",    "Hutchison 3G Ireland", NULL,  NULL, NULL,"*99***1#"},
{"27401",    "Orange", NULL,  NULL, "internet", "*99***1#"},
{"27402",    "Vodafone Iceland", NULL,  NULL, NULL,"*99***1#"},
{"27403",    "Vodafone Iceland", NULL,  NULL, NULL,"*99***1#"},
{"27404",    "Viking wireless", NULL,  NULL, NULL,"*99***1#"},
{"27408",    "On-waves", NULL,  NULL, NULL,"*99***1#"},
{"27411",    "Nova ehf.", NULL,  NULL, NULL,"*99***1#"},
{"27601",    "A M C  MOBIL", NULL,  NULL, NULL,"*99***1#"},
{"27602",    "vodafone", NULL,  NULL, NULL,"*99***1#"},
{"27801",    "vodafone", NULL,  NULL, NULL,"*99***1#"},
{"27821",    "go mobile(contract)", NULL,  NULL, "gosurfing", "*99***1#"},
{"27821",    "go mobile(pre.pay)", NULL,  NULL, "rtgsurfing", "*99***1#"},
{"28001",    "Cytamobile-Vodafone", NULL,  NULL, NULL,"*99***1#"},
{"28010",    "Areeba", NULL,  NULL, NULL,"*99***1#"},
{"28201",    "GEOCELL", NULL,  NULL, NULL,"*99***1#"},
{"28202",    "MAGTI GSM", NULL,  NULL, NULL,"*99***1#"},
{"28204",    "Mobitel", NULL,  NULL, NULL,"*99***1#"},
{"28301",    "ARMGSM", NULL,  NULL, NULL,"*99***1#"},
{"28305",    "VivaCell", NULL,  NULL, NULL,"*99***1#"},
{"28401",    "M-Tel BG", NULL,  NULL, NULL,"*99***1#"},
{"28403",    "vivatel", NULL,  NULL, NULL,"*99***1#"},
{"28405",    "GLOBUL", "globul",  NULL, "internet.globul.gl", "*99***1#"},
{"28601",    "Turkcell", "gprs",  "gprs", "internet", "*99***1#"},
{"28602",    "Telsim", "telsim",  "telsim", "telsim", "*99***1#"},
{"28603",    "AVEA", NULL,  NULL, NULL,"*99***1#"},
{"28604",    "Aycell", NULL,  NULL, "aycell", "*99***1#"},
{"28801",    "Faroese Telecom GSM", NULL,  NULL, NULL,"*99***1#"},
{"28802",    "KALL-GSM", NULL,  NULL, NULL,"*99***1#"},
{"29001",    "TELE Greenland", NULL,  NULL, NULL,"*99***1#"},
{"29266",    "San Marino Telecom", NULL,  NULL, NULL,"*99***1#"},
{"29340",    "Simobil", NULL,  NULL, "internet.si.mobil", "*99***1#"},
{"29341",    "MOBITEL(Internet)", "mobitel",  "internet", "internet", "*99***1#"},
{"29341",    "MOBITEL(Internet Pro)", "mobitel",  "internet", "internetpro", "*99***1#"},
{"29370",    "Tusmobil", NULL,  NULL, NULL,"*99***1#"},
{"29401",    "T-Mobile Macedonia", NULL,  NULL, NULL,"*99***1#"},
{"29402",    "COSMOFON", NULL,  NULL, NULL,"*99***1#"},
{"29501",    "FL GSM", NULL,  NULL, NULL,"*99***1#"},
{"29502",    "Orange FL", NULL,  NULL, NULL,"*99***1#"},
{"29505",    "FL1", NULL,  NULL, NULL,"*99***1#"},
{"29577",    "Tele 2 AG", NULL,  NULL, NULL,"*99***1#"},
{"30237",    "Microcell", "fido",  "fido", "internet.fido.ca", "*99***1#"},
{"30272",    "Rogers Wireless", NULL,  NULL, NULL,"*99***1#"},
{"30801",    "AMERIS", NULL,  NULL, NULL,"*99***1#"},
{"30802",    "Prosodie", NULL,  NULL, NULL,"*99***1#"},
{"31041",    "Cingular USA"  ,"isdpa@cingulargprs.com ",  "CINGULAR1",  "isp.cingular" ,"*99***1#"},
{"31080",    "T-Mobile USA"  ,NULL,  NULL,  "internet2.voicestream.com" ,"*99***1#"},
{"31020",    "Union Telephone Company", NULL,  NULL, NULL,"*99***1#"},
{"31026",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31030",    "Centennial Communications", NULL,  NULL, NULL,"*99***1#"},
{"31031",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31032",    "IT&E Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31040",    "Concho Cellular Telephone Co.", NULL,  NULL, NULL,"*99***1#"},
{"31046",    "SIMMETRY", NULL,  NULL, NULL,"*99***1#"},
{"31080",    "Corr Wireless Communications", NULL,  NULL, NULL,"*99***1#"},
{"31090",    "Edge Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31010",    "Plateau Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31015",    "Cingular, now the new AT&T", NULL,  NULL, NULL,"*99***1#"},
{"31016",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31017",    "Cingular, now the new AT&T", NULL,  NULL, NULL,"*99***1#"},
{"31019",    "Dutch Harbor", NULL,  NULL, NULL,"*99***1#"},
{"31020",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31021",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31022",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31023",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31024",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31025",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31026",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31027",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31032",    "CellularOne", NULL,  NULL, NULL,"*99***1#"},
{"31040",    "Westlink Communications", NULL,  NULL, NULL,"*99***1#"},
{"31080",    "Cingular, now the new AT&T", NULL,  NULL, NULL,"*99***1#"},
{"31090",    "Cellular One of East Texas", NULL,  NULL, NULL,"*99***1#"},
{"31040",    "i CAN_GSM", NULL,  NULL, NULL,"*99***1#"},
{"31041",    "Cingular, now the new AT&T", NULL,  NULL, NULL,"*99***1#"},
{"31045",    "Viaero Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31053",    "West Virginia Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31056",    "Dobson", NULL,  NULL, NULL,"*99***1#"},
{"31057",    "Chinook Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31059",    "Alltel", NULL,  NULL, NULL,"*99***1#"},
{"31063",    "AmeriLink PCS", NULL,  NULL, NULL,"*99***1#"},
{"31064",    "Airadigm Communications", NULL,  NULL, NULL,"*99***1#"},
{"31065",    "Jasper", NULL,  NULL, NULL,"*99***1#"},
{"31066",    "T-Mobile USA", NULL,  NULL, NULL,"*99***1#"},
{"31069",    "Immix Wireless", NULL,  NULL, NULL,"*99***1#"},
{"31070",    "BIG FOOT", NULL,  NULL, NULL,"*99***1#"},
{"31071",    "Artic Slope Telephone Association Cooperative", NULL,  NULL, NULL,"*99***1#"},
{"31074",    "TELEMETRIX", NULL,  NULL, NULL,"*99***1#"},
{"31077",    "Iowa Wireless Services", NULL,  NULL, NULL,"*99***1#"},
{"31079",    "PinPoint Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31087",    "PinPoint Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31088",    "Advantage", NULL,  NULL, NULL,"*99***1#"},
{"31089",    "Wireless Alliance",  NULL,  NULL, NULL,"*99***1#"},
{"31090",    "Texas Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"31091",    "FC",  NULL,  NULL, NULL,"*99***1#"},
{"31095",    "Texas RSA 1 Ltd Partnership DBA XIT Cell", NULL,  NULL, NULL,"*99***1#"},
{"31100",    "Mid-Tex Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"31100",    "Via Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31101",    "Via Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31105",    "Via Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31130",    "Indigo Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31140",    "Commnet",  NULL,  NULL, NULL,"*99***1#"},
{"31170",    "Easterbrooke",  NULL,  NULL, NULL,"*99***1#"},
{"31180",    "Pine Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"31190",    "Long Lines Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31111",    "High Plains Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"31113",    "Cellular One of Amarillo",  NULL,  NULL, NULL,"*99***1#"},
{"31117",    "PetroCom",  NULL,  NULL, NULL,"*99***1#"},
{"31119",    "Cellular One of East Central Illinois",  NULL,  NULL, NULL,"*99***1#"},
{"31121",    "Farmers",  NULL,  NULL, NULL,"*99***1#"},
{"31124",    "Cordova Wireless Communications",  NULL,  NULL, NULL,"*99***1#"},
{"31125",    "i CAN_GSM",  NULL,  NULL, NULL,"*99***1#"},
{"31131",    "Lamar County Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"31136",    "Stelera Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"33402",    "Telcel",  "webgprs",  "webgprs2002", "internet.itelcel.com", "*99***1#"},
{"33403",    "movistar",  NULL,  NULL, NULL,"*99***1#"},
{"33805",    "Digicel Jamaica",  NULL,  NULL, NULL,"*99***1#"},
{"33818",    "Cable and Wireless Jamaica Limited",  NULL,  NULL, NULL,"*99***1#"},
{"34001",    "Orange",  NULL,  NULL, NULL,"*99***1#"},
{"34002",    "Outremer",  NULL,  NULL, NULL,"*99***1#"},
{"34008",    "AMIGO GSM",  NULL,  NULL, NULL,"*99***1#"},
{"34020",    "DIGICEL F",  NULL,  NULL, NULL,"*99***1#"},
{"34260",    "Cable & Wireless (Barbados) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"34275",    "Digicel",  NULL,  NULL, NULL,"*99***1#"},
{"34430",    "APUA PCS",  NULL,  NULL, NULL,"*99***1#"},
{"34492",    "Digicel Antigua & Barbuda",  NULL,  NULL, NULL,"*99***1#"},
{"31136",    "Cable & Wireless (Cayman Islands) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"34614",    "Cable & Wireless (Cayman Islands) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"34817",    "CCT",  NULL,  NULL, NULL,"*99***1#"},
{"34857",    "MOBILITY LTD.",  NULL,  NULL, NULL,"*99***1#"},
{"35002",    "Digicel Bermuda",  NULL,  NULL, NULL,"*99***1#"},
{"35230",    "Digicel",  NULL,  NULL, NULL,"*99***1#"},
{"35211",    "Cable & Wireless Grenada Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"35486",    "Cable & Wireless West Indies (Montserrat)",  NULL,  NULL, NULL,"*99***1#"},
{"35611",    "Cable & Wireless St Kitts & Nevis Limited",  NULL,  NULL, NULL,"*99***1#"},
{"35611",    "Cable & Wireless St Kitts & Nevis Limited",  NULL,  NULL, NULL,"*99***1#"},
{"35850",    "Digicel (St Lucia) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"35811",    "Cable & Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"36070",    "Digicel (St. Vincent and Grenadines) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"36011",    "Cable & Wireless Caribbean Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"36251",    "Telcell N.V.",  NULL,  NULL, NULL,"*99***1#"},
{"36269",    "Digicel Netherlands Antilles",  NULL,  NULL, NULL,"*99***1#"},
{"362951",   "UTS Wireless Curacao",  NULL,  NULL, NULL,"*99***1#"},
{"36301",    "SETAR GSM",  NULL,  NULL, NULL,"*99***1#"},
{"36320",    "Digicel",  NULL,  NULL, NULL,"*99***1#"},
{"36439",    "The Bahamas Telecommunications Company Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"36584",    "Cable & Wireless (West Indies) Ltd. Anguilla",  NULL,  NULL, NULL,"*99***1#"},
{"36620",    "Digicel Dominica",  NULL,  NULL, NULL,"*99***1#"},
{"36611",    "Cable & Wireless Dominica Ltd.",  NULL,  NULL, NULL,"*99***1#"},
{"36801",    "ETECSA",  NULL,  NULL, NULL,"*99***1#"},
{"37001",    "ORANGE",  NULL,  NULL, "orangenet.com.do", "*99***1#"},
{"37002",    "CLARO GSM",  NULL,  NULL, NULL,"*99***1#"},
{"37201",    "Comcel",  NULL,  NULL, NULL,"*99***1#"},
{"37412",    "TSTT",  NULL,  NULL, NULL,"*99***1#"},
{"37635",    "Cable & Wireless West Indies Ltd ",  NULL,  NULL, NULL,"*99***1#"},
{"37635",    "Islandcom",  NULL,  NULL, NULL,"*99***1#"},
{"40001",    "AZERCELL",  NULL,  NULL, "internet", "*99***1#"},
{"40002",    "BAKCELL GSM 2000",  NULL,  NULL, NULL,"*99***1#"},
{"40004",    "Nar Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"40101",    "Beeline",  "internet.beeline",  NULL, "internet.beeline.kz", "*99***1#"},
{"40102",    "K'CELL",  NULL,  NULL, NULL,"*99***1#"},
{"40177",    "Mobile Telecom Service",  NULL,  NULL, NULL,"*99***1#"},
{"40211",    "B-Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"40401",    "Hutch-Haryana",  NULL,  NULL, NULL,"*99***1#"},
{"40402",    "AirTel",  NULL,  NULL, "airtelgprs.com", "*99***1#"},
{"40403",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40404",    "IDEA CELLULAR - Delhi",  NULL,  NULL, NULL,"*99***1#"},
{"40405",    "Hutch-Gujarat",  NULL,  NULL, NULL,"*99***1#"},
{"40407",    "IDEA Cellular - Andhra Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40410",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40411",    "Hutch-Delhi",  NULL,  NULL, NULL,"*99***1#"},
{"40412",    "Idea Cellular",  NULL,  NULL, "internet", "*99***1#"},
{"40413",    "Hutch-Andhra Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40414",    "Spice Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"40415",    "Hutch-UP East",  NULL,  NULL, NULL,"*99***1#"},
{"40417",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40419",    "Idea (Escotel) Kerala",  NULL,  NULL, NULL,"*99***1#"},
{"40420",    "Orange",  NULL,  NULL, "portalnmms", "*99***1#"},
{"40421",    "BPL  MOBILE",  NULL,  NULL, NULL,"*99***1#"},
{"40422",    "IDEA Cellular - Maharashtra",  NULL,  NULL, NULL,"*99***1#"},
{"40424",    "IDEA Cellular - Gujarat",  NULL,  NULL, NULL,"*99***1#"},
{"40425",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40427",    "BPL",  "bplmobile",  NULL, "bplgprs.com", "*99***1#"},
{"40428",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40429",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40430",    "Hutch-Kolkata",  NULL,  NULL, NULL,"*99***1#"},
{"40431",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40433",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40434",    "CellOne Haryana",  NULL,  NULL, NULL,"*99***1#"},
{"40435",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40437",    "Aircel",  NULL,  NULL, NULL,"*99***1#"},
{"40438",    "CellOne Assam",  NULL,  NULL, NULL,"*99***1#"},
{"40440",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40441",    "Aircel Cellular Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"40442",    "AIRCEL",  NULL,  NULL, NULL,"*99***1#"},
{"40444",    "Spice Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"40445",    "Airtel",  NULL,  NULL, NULL,"*99***1#"},
{"40449",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40451",    "CellOne Himachal Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40453",    "CellOne Punjab",  NULL,  NULL, NULL,"*99***1#"},
{"40454",    "CellOne Uttar Pradesh (West)",  NULL,  NULL, NULL,"*99***1#"},
{"40455",    "CellOne Uttar Pradesh (East)",  NULL,  NULL, NULL,"*99***1#"},
{"40456",    "Idea (Escotel) UP West",  NULL,  NULL, NULL,"*99***1#"},
{"40457",    "CellOne Gujarat",  NULL,  NULL, NULL,"*99***1#"},
{"40458",    "CellOne Madhya Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40459",    "CellOne Rajasthan",  NULL,  NULL, NULL,"*99***1#"},
{"40460",    "Hutch-Rajasthan",  NULL,  NULL, NULL,"*99***1#"},
{"40462",    "CellOne Jammu & Kashmir",  NULL,  NULL, NULL,"*99***1#"},
{"40464",    "CellOne Chennai",  NULL,  NULL, NULL,"*99***1#"},
{"40466",    "CellOne Maharashtra",  NULL,  NULL, NULL,"*99***1#"},
{"40468",    "Mahanagar Telephone Nigam Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"40469",    "Mahanagar Telephone Nigam Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"40470",    "AIRTEL",  NULL,  NULL, NULL,"*99***1#"},
{"40471",    "CellOne Karnataka",  NULL,  NULL, NULL,"*99***1#"},
{"40472",    "CellOne Kerala",  NULL,  NULL, NULL,"*99***1#"},
{"40473",    "CellOne Andhra Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40474",    "CellOne West Bengal",  NULL,  NULL, NULL,"*99***1#"},
{"40475",    "CellOne Bihar",  NULL,  NULL, NULL,"*99***1#"},
{"40476",    "CellOne Orissa",  NULL,  NULL, NULL,"*99***1#"},
{"40477",    "CellOne North East",  NULL,  NULL, NULL,"*99***1#"},
{"40478",    "IDEA Cellular - Madhya Pradesh",  NULL,  NULL, NULL,"*99***1#"},
{"40479",    "CellOne A&N",  NULL,  NULL, NULL,"*99***1#"},
{"40480",    "CellOne Tamil Nadu",  NULL,  NULL, NULL,"*99***1#"},
{"40481",    "CellOne Kolkata",  NULL,  NULL, NULL,"*99***1#"},
{"40482",    "IDEA",  NULL,  NULL, NULL,"*99***1#"},
{"40484",    "Hutch-Chennai",  NULL,  NULL, NULL,"*99***1#"},
{"40486",    "Hutch-Karnataka",  NULL,  NULL, NULL,"*99***1#"},
{"40487",    "IDEA",  NULL,  NULL, NULL,"*99***1#"},
{"40488",    "Hutch-Punjab",  NULL,  NULL, NULL,"*99***1#"},
{"40489",    "IDEA",  NULL,  NULL, NULL,"*99***1#"},
{"40490",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40492",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40493",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40494",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40495",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40496",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40497",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40498",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40550",    "Reliance Telecom Private",  NULL,  NULL, NULL,"*99***1#"},
{"40551",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40552",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40553",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40554",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40555",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40556",    "AirTel",  NULL,  NULL, NULL,"*99***1#"},
{"40566",    "Hutch- UP West",  NULL,  NULL, NULL,"*99***1#"},
{"40567",    "Hutch-West Bengal",  NULL,  NULL, NULL,"*99***1#"},
{"41001",    "Mobilink",  NULL,  NULL, NULL,"*99***1#"},
{"41003",    "Ufone",  "ufone",  "ufone", "ufone.internet", "*99***1#"},
{"41004",    "PAKTEL",  NULL,  NULL, NULL,"*99***1#"},
{"41006",    "Telenor Pakistan (Pvt) Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"41007",    "Warid Telecom (PVT) Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"41201",    "AWCC",  NULL,  NULL, NULL,"*99***1#"},
{"41220",    "ROSHAN",  NULL,  NULL, NULL,"*99***1#"},
{"41240",    "Areeba",  NULL,  NULL, NULL,"*99***1#"},
{"41250",    "Etisalat Afghanistan",  NULL,  NULL, NULL,"*99***1#"},
{"41301",    "Mobitel",  NULL,  NULL, NULL,"*99***1#"},
{"41302",    "DIALOG",  NULL,  NULL, NULL,"*99***1#"},
{"41303",    "Tigo",  NULL,  NULL, NULL,"*99***1#"},
{"41308",    "Hutchison Telecommunications Lanka (Pvt) Limited",  NULL,  NULL, NULL,"*99***1#"},
{"41401",    "MPT GSM Network",  NULL,  NULL, NULL,"*99***1#"},
{"41501",    "Cellis FTML",  "plugged",  "plugged", "internet.ftml.com.lb", "*99***1#"},
{"41503",    "mtc touch",  NULL,  NULL, NULL,"*99***1#"},
{"41601",    "Fastlink",  NULL,  NULL, NULL,"*99***1#"},
{"41603",    "Umniah",  NULL,  NULL, NULL,"*99***1#"},
{"41677",    "MOBILECOM",  NULL,  NULL, NULL,"*99***1#"},
{"41701",    "SYRIATEL",  NULL,  NULL, NULL,"*99***1#"},
{"41702",    "MTN Syria",  NULL,  NULL, NULL,"*99***1#"},
{"41709",    "MOBILE SYRIA",  NULL,  NULL, NULL,"*99***1#"},
{"41800",    "Asia Cell Telecommunications Company",  NULL,  NULL, NULL,"*99***1#"},
{"41802",    "SanaTel",  NULL,  NULL, NULL,"*99***1#"},
{"41805",    "Asia Cell Telecommunications Company",  NULL,  NULL, NULL,"*99***1#"},
{"41808",    "SanaTel",  NULL,  NULL, NULL,"*99***1#"},
{"41820",    "mtc atheer",  NULL,  NULL, NULL,"*99***1#"},
{"41830",    "IRAQNA",  NULL,  NULL, NULL,"*99***1#"},
{"41902",    "ZAIN KWT",  NULL,  NULL, NULL,"*99***1#"},
{"41903",    "Wataniya Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"42001",    "AL JAWAL",  NULL,  NULL, NULL,"*99***1#"},
{"42003",    "Mobily",  NULL,  NULL, NULL,"*99***1#"},
{"42101",    "Yemen Company for Mobile Telephony",  NULL,  NULL, NULL,"*99***1#"},
{"42102",    "MTN",  NULL,  NULL, NULL,"*99***1#"},
{"42202",    "OMAN MOBILE",  NULL,  NULL, NULL,"*99***1#"},
{"42203",    "nawras",  NULL,  NULL, NULL,"*99***1#"},
{"42401",    "ETISALAT",  "mnet",  "mnet", "mnet", "*99***1#"},
{"42402",    "ETISALAT",  "mnet",  "mnet", "mnet", "*99***1#"},
{"42403",    "du",  NULL,  NULL, NULL,"*99***1#"},
{"42501",    "Orange",  NULL,  NULL, "internet", "*99***1#"},
{"42502",    "Cellcom",  NULL,  NULL, "internetg", "*99***1#"},
{"42502",    "Cellcom",  NULL,  NULL, "internetg", "*99***1#"},
{"42503",    "Pelephone lL",  "pcl@3g",  "pcl", "internet.pelephone.net.il", "*99***1#"},
{"42505",    "Palestine Telecommunications Co. P.L.C",  NULL,  NULL, NULL,"*99***1#"},
{"42601",    "BATELCO",  NULL,  NULL, NULL,"*99***1#"},
{"42602",    "MTC VODAFONE Bahrain",  NULL,  NULL, NULL,"*99***1#"},
{"42602",    "MTC Vodafone (Bahrain) B.S.C",  NULL,  NULL, NULL,"*99***1#"},
{"42701",    "Qatarnet",  NULL,  NULL, NULL,"*99***1#"},
{"42888",    "Unitel",  NULL,  NULL, NULL,"*99***1#"},
{"42899",    "MobiCom",  NULL,  NULL, NULL,"*99***1#"},
{"43211",    "MCI",  NULL,  NULL, NULL,"*99***1#"},
{"43214",    "TKC",  NULL,  NULL, NULL,"*99***1#"},
{"43219",    "MTCE",  NULL,  NULL, NULL,"*99***1#"},
{"43232",    "Taliya",  NULL,  NULL, NULL,"*99***1#"},
{"43404",    "Beeline",  NULL,  NULL, NULL,"*99***1#"},
{"43405",    "COSCOM",  NULL,  NULL, NULL,"*99***1#"},
{"43407",    "Uzdunrobita",  "user",  "pass", "net.urd.uz", "*99***1#"},
{"43601",    "JV Somoncom",  NULL,  NULL, NULL,"*99***1#"},
{"43602",    "Indigo Tajikistan",  NULL,  NULL, NULL,"*99***1#"},
{"43603",    "Mobile Lines of Tajikistan",  NULL,  NULL, NULL,"*99***1#"},
{"43604",    "Babilon-M",  NULL,  NULL, NULL,"*99***1#"},
{"43605",    "BEELINE TJ",  NULL,  NULL, NULL,"*99***1#"},
{"43612",    "Indigo Tajikistan",  NULL,  NULL, NULL,"*99***1#"},
{"43701",    "BITEL",  NULL,  NULL, NULL,"*99***1#"},
{"43705",    "MEGACOM",  NULL,  NULL, NULL,"*99***1#"},
{"43801",    "MTS Turkmenistan",  NULL,  NULL, NULL,"*99***1#"},
{"44000",    "eMobile",  NULL,  NULL, NULL,"*99***1#"},
{"44010",    "NTT DoCoMo, Inc",  NULL,  NULL, NULL,"*99***1#"},
{"44020",    "J-Phone",  NULL,  NULL, NULL,"*99***1#"},
{"45002",    "KTF",  NULL,  NULL, NULL,"*99***1#"},
{"45005",    "SK Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"45008",    "KTF",  NULL,  NULL, NULL,"*99***1#"},
{"45201",    "Mobifone",  NULL,  NULL, NULL,"*99***1#"},
{"45202",    "Mobifone",  NULL,  NULL, NULL,"*99***1#"},
{"45204",    "Viettel Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"45400",    "HK CSL",  NULL,  NULL, "HKCSL","*99***1#"},
{"45402",    "HK CSL",  NULL,  NULL, "HKCSL","*99***1#"},
{"45403",    "3 HK",  NULL,  NULL, "ipc.three.com.hk", "*99***1#"},
{"45404",    "3(2G)",  NULL,  NULL, NULL,"*99***1#"},
{"45406",    "SmarToneVodafone",  NULL,  NULL, "internet", "*99***1#"},
{"45410",    "New World Mobility",  NULL,  NULL, NULL,"*99***1#"},
{"45412",    "PEOPLES",  NULL,  NULL, "Peoples.net","*99***1#"},
{"45415",    "SmarToneVodafone",  NULL,  NULL, NULL,"*99***1#"},
{"45416",    "PCCW",  NULL,  NULL, NULL,"*99***1#"},
{"45418",    "HK CSL",  NULL,  NULL, "hkcsl","*99***1#"},
{"45419",    "PCCW Mobile(Sunday)",  NULL,  NULL, "sunday", "*99***1#"},
{"45500",    "SmarTone",  NULL,  NULL, NULL,"*99***1#"},
{"45501",    "CTM",  NULL,  NULL, NULL,"*99***1#"},
{"45503",    "Hutchison Telecom Macau",  NULL,  NULL, NULL,"*99***1#"},
{"45504",    "CTM",  NULL,  NULL, NULL,"*99***1#"},
{"45601",    "MOBITEL",  NULL,  NULL, NULL,"*99***1#"},
{"45602",    "Hello GSM",  NULL,  NULL, NULL,"*99***1#"},
{"45605",    "StarCell",  NULL,  NULL, NULL,"*99***1#"},
{"45618",    "CAMBODIA SHINAWATRA",  NULL,  NULL, NULL,"*99***1#"},
{"45701",    "LAO TELECOMMUNICATIONS",  NULL,  NULL, NULL,"*99***1#"},
{"45702",    "ETL MOBILE",  NULL,  NULL, NULL,"*99***1#"},
{"45703",    "LAT",  NULL,  NULL, NULL,"*99***1#"},
{"45708",    "Tigo",  NULL,  NULL, NULL,"*99***1#"},
{"46000",    "CHINA MOBILE",  "card",  "card", "cmnet","*99***1#"},
{"46001",    "CHINA UNICOM",  "card",  "card", "3gnet","*99***1#"},
{"46002",    "CHINA MOBILE",  "card",  "card", "cmnet","*99***1#"},
{"46003",    "CHINA TELECOM",  "card",  "card", NULL,"#777"},
{"46007",    "CHINA MOBILE",  "card",  "card", "cmnet","*99***1#"},
{"46602",    "Far EasTone",  NULL,  NULL, NULL,"*99***1#"},
{"46668",    "ACeS Taiwan - ACeS Taiwan Telecommunications Co Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"46688",    "KG Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"46689",    "VIBO",  NULL,  NULL, NULL,"*99***1#"},
{"46692",    "Chunghwa Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"46693",    "MOBITAI",  NULL,  NULL, NULL,"*99***1#"},
{"46697",    "Taiwan Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"46699",    "TransAsia Telecom GSM",  NULL,  NULL, NULL,"*99***1#"},
{"46699",    "TransAsia Telecommunications",  NULL,  NULL, NULL,"*99***1#"},
{"46703",    "SUN NET",  NULL,  NULL, NULL,"*99***1#"},
{"47001",    "GrameenPhone",  NULL,  NULL, NULL,"*99***1#"},
{"47002",    "AKTEL",  NULL,  NULL, NULL,"*99***1#"},
{"47003",    "Banglalink",  NULL,  NULL, NULL,"*99***1#"},
{"47004",    "Teletalk",  NULL,  NULL, NULL,"*99***1#"},
{"47201",    "DHIMOBILE",  NULL,  NULL, NULL,"*99***1#"},
{"47202",    "WMOBILE",  NULL,  NULL, NULL,"*99***1#"},
{"50200",    "TIME3G",  NULL,  NULL, NULL,"*99***1#"},
{"50201",    "TIME3G",  NULL,  NULL, NULL,"*99***1#"},
{"50212",    "MMS & MB",  NULL,  NULL, NULL,"*99***1#"},
{"50213",    "Celcom Malaysia",  NULL,  NULL, NULL,"*99***1#"},
{"50216",    "DiGi",  NULL,  NULL, NULL,"*99***1#"},
{"50219",    "CELCOM GSM",  NULL,  NULL, NULL,"*99***1#"},
{"50501",    "Telstra",  NULL,  NULL, "telstra.internet", "*99***1#"},
{"50512",    "Three",  "a",  "a", "3netaccess", "*99***1#"},
{"50502",    "OPTUS",  NULL,  NULL, "internet", "*99***1#"},
{"50503",    "vodafone",  NULL,  NULL, NULL,"*99***1#"},
{"50506",    "Vodafone",  NULL,  NULL, "vfinternet.au", "*99***1#"},
{"51000",    "ACeS",  NULL,  NULL, NULL,"*99***1#"},
{"51001",    "INDOSAT",  NULL,  NULL, NULL,"*99***1#"},
{"51008",    "Lippo Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"51010",    "TELKOMSEL",  NULL,  NULL, NULL,"*99***1#"},
{"51011",    "IND XL",  NULL,  NULL, NULL,"*99***1#"},
{"51021",    "INDOSAT",  NULL,  NULL, NULL,"*99***1#"},
{"51089",    "3",  NULL,  NULL, NULL,"*99***1#"},
{"51402",    "Timor Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"51501",    "Islacom",  NULL,  NULL, NULL,"*99***1#"},
{"51502",    "Globe Telecom",  "globe",  "globe", "internet.globe.com.ph", "*99***1#"},
{"51503",    "SMART",  "witsductoor",  "banonoy", "internet", "*99***1#"},
{"51505",    "Sun Cellular",  NULL,  NULL, "minternet", "*99***1#"},
{"51511",    "ACeS Philippines - ACeS Philippines Cellular Corp",  NULL,  NULL, NULL,"*99***1#"},
{"52001",    "AIS GSM",  NULL,  NULL, NULL,"*99***1#"},
{"52015",    "ACT GSM 1900",  NULL,  NULL, NULL,"*99***1#"},
{"52018",    "DTAC",  NULL,  NULL, NULL,"*99***1#"},
{"52020",    "ACeS Thailand - ACeS Regional Services Co Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"52023",    "GSM 1800",  NULL,  NULL, NULL,"*99***1#"},
{"52099",    "True Move",  NULL,  NULL, NULL,"*99***1#"},
{"52501",    "SingTel",  NULL,  NULL, NULL,"*99***1#"},
{"52502",    "SingTel-G18",  NULL,  NULL, NULL,"*99***1#"},
{"52503",    "MOBILEONE",  NULL,  NULL, NULL,"*99***1#"},
{"52505",    "StarHub Pte Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"52505",    "StarHub Mobile Pte Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"52802",    "b-mobile",  NULL,  NULL, NULL,"*99***1#"},
{"52811",    "DTSCom",  NULL,  NULL, NULL,"*99***1#"},
{"53001",    "vodafone",  NULL,  NULL, "www.vodafone.net.nz", "*99***1#"},
{"53701",    "Bee Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"53730",    "Digicel PNG Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"53901",    "U-CALL",  NULL,  NULL, NULL,"*99***1#"},
{"54001",    "BREEZE",  NULL,  NULL, NULL,"*99***1#"},
{"54101",    "SMILE",  NULL,  NULL, NULL,"*99***1#"},
{"54201",    "VODAFONE",  NULL,  NULL, NULL,"*99***1#"},
{"54509",    "Kiribati Frigate",  NULL,  NULL, NULL,"*99***1#"},
{"54601",    "MOBILIS",  NULL,  NULL, NULL,"*99***1#"},
{"54720",    "VINI",  NULL,  NULL, NULL,"*99***1#"},
{"54801",    "Telecom Cook Islands",  NULL,  NULL, NULL,"*99***1#"},
{"54927",    "Samoatel Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"55001",    "FSM Telecommunications Corporation",  NULL,  NULL, NULL,"*99***1#"},
{"55280",    "Palau Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"60201",    "MoBiNiL",  NULL,  NULL, "mobinilweb", "*99***1#"},
{"60202",    "Click vodafone",  "internet",  "internet", "internet.vodafone.net", "*99***1#"},
{"60203",    "Etisalat",  NULL,  NULL, NULL,"*99***1#"},
{"60301",    "Mobilis",  NULL,  NULL, NULL,"*99***1#"},
{"60302",    "Djezzy",  NULL,  NULL, NULL,"*99***1#"},
{"60303",    "Nedjma",  NULL,  NULL, NULL,"*99***1#"},
{"60400",    "MEDI TELECOM",  NULL,  NULL, NULL,"*99***1#"},
{"60401",    "IAM",  NULL,  NULL, NULL,"*99***1#"},
{"60502",    "TUNTEL",  NULL,  NULL, NULL,"*99***1#"},
{"60503",    "TUNISIANA",  NULL,  NULL, NULL,"*99***1#"},
{"60600",    "Libyana Mobile Phone",  NULL,  NULL, NULL,"*99***1#"},
{"60601",    "Libya Al Madar",  NULL,  NULL, NULL,"*99***1#"},
{"60701",    "GAMCELL",  NULL,  NULL, NULL,"*99***1#"},
{"60702",    "AFRICELL",  NULL,  NULL, NULL,"*99***1#"},
{"60703",    "Comium Gambia",  NULL,  NULL, NULL,"*99***1#"},
{"60801",    "Alize",  NULL,  NULL, NULL,"*99***1#"},
{"60802",    "SENTEL",  NULL,  NULL, NULL,"*99***1#"},
{"60901",    "MALITEL",  NULL,  NULL, NULL,"*99***1#"},
{"61001",    "MALITEL",  NULL,  NULL, NULL,"*99***1#"},
{"61002",    "Orange MALI",  NULL,  NULL, NULL,"*99***1#"},
{"61101",    "Orange Guinee SA",  NULL,  NULL, NULL,"*99***1#"},
{"61102",    "LAGUI",  NULL,  NULL, NULL,"*99***1#"},
{"61104",    "Areeba",  NULL,  NULL, NULL,"*99***1#"},
{"61202",    "A-Cell",  NULL,  NULL, NULL,"*99***1#"},
{"61203",    "Orange CI",  NULL,  NULL, NULL,"*99***1#"},
{"61201",    "KoZ",  NULL,  NULL, NULL,"*99***1#"},
{"61205",    "MTN",  NULL,  NULL, NULL,"*99***1#"},
{"61302",    "Celtel Burkina Faso",  NULL,  NULL, NULL,"*99***1#"},
{"61401",    "SahelCom",  NULL,  NULL, NULL,"*99***1#"},
{"61402",    "Celtel",  NULL,  NULL, NULL,"*99***1#"},
{"61403",    "Telecel Niger",  NULL,  NULL, NULL,"*99***1#"},
{"61501",    "TOGO CELL",  NULL,  NULL, NULL,"*99***1#"},
{"61503",    "Telecel Togo",  NULL,  NULL, NULL,"*99***1#"},
{"61602",    "TELECEL BENIN",  NULL,  NULL, NULL,"*99***1#"},
{"61603",    "Areeba",  NULL,  NULL, NULL,"*99***1#"},
{"61604",    "Bell Benin Communication BBCOM",  NULL,  NULL, NULL,"*99***1#"},
{"61701",    "CELLPLUS",  NULL,  NULL, NULL,"*99***1#"},
{"61710",    "EMTEL",  NULL,  NULL, NULL,"*99***1#"},
{"61801",    "Lonestar Cell",  NULL,  NULL, NULL,"*99***1#"},
{"61802",    "LIBERCELL",  NULL,  NULL, NULL,"*99***1#"},
{"61807",    "Celcom Telecommunications",  NULL,  NULL, NULL,"*99***1#"},
{"61901",    "CELTEL",  NULL,  NULL, NULL,"*99***1#"},
{"61902",    "MILLICOM",  NULL,  NULL, NULL,"*99***1#"},
{"62001",    "MTN",  NULL,  NULL, NULL,"*99***1#"},
{"62002",    "Ghana Telecom Mobile GSM",  NULL,  NULL, NULL,"*99***1#"},
{"62003",    "tiGO",  NULL,  NULL, NULL,"*99***1#"},
{"62120",    "Celtel Nigeria Limited",  NULL,  NULL, NULL,"*99***1#"},
{"62130",    "MTN Nigeria",  NULL,  NULL, NULL,"*99***1#"},
{"62140",    "Mtel",  NULL,  NULL, NULL,"*99***1#"},
{"62150",    "Glo Mobile",  "wap",  NULL, "gprs", "*99***1#"},
{"62201",    "CELTEL",  NULL,  NULL, NULL,"*99***1#"},
{"62302",    "TELECEL CENTRAFRIQUE",  NULL,  NULL, NULL,"*99***1#"},
{"62401",    "MTN Cameroon",  NULL,  NULL, NULL,"*99***1#"},
{"62402",    "Orange",  NULL,  NULL, NULL,"*99***1#"},
{"62501",    "CVMOVEL",  NULL,  NULL, NULL,"*99***1#"},
{"62502",    "T+ TELECOMUNICA??ES S.A",  NULL,  NULL, NULL,"*99***1#"},
{"62601",    "CSTmovel",  NULL,  NULL, NULL,"*99***1#"},
{"62701",    "Orange GQ",  NULL,  NULL, NULL,"*99***1#"},
{"62801",    "LIBERTIS",  NULL,  NULL, NULL,"*99***1#"},
{"62802",    "TELECEL",  NULL,  NULL, NULL,"*99***1#"},
{"62803",    "Celtel",  NULL,  NULL, NULL,"*99***1#"},
{"62901",    "CelTel Congo",  NULL,  NULL, NULL,"*99***1#"},
{"62910",    "Libertis Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"63001",    "VODACOM CONGO",  NULL,  NULL, NULL,"*99***1#"},
{"63002",    "CelTel Congo SA",  NULL,  NULL, NULL,"*99***1#"},
{"63005",    "Supercell Sprl",  NULL,  NULL, NULL,"*99***1#"},
{"63086",    "Congo Chine Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"63089",    "SAIT Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"63102",    "UNITEL",  NULL,  NULL, NULL,"*99***1#"},
{"63203",    "Orange Bissau",  NULL,  NULL, NULL,"*99***1#"},
{"63207",    "Guinetel",  NULL,  NULL, NULL,"*99***1#"},
{"63301",    "CABLE & WIRELESS",  NULL,  NULL, NULL,"*99***1#"},
{"63310",    "AIRTEL",  NULL,  NULL, NULL,"*99***1#"},
{"63401",    "MobiTel",  NULL,  NULL, NULL,"*99***1#"},
{"63402",    "MTN",  NULL,  NULL, NULL,"*99***1#"},
{"63510",    "Rwandacell",  NULL,  NULL, NULL,"*99***1#"},
{"63601",    "ETMTN",  NULL,  NULL, NULL,"*99***1#"},
{"63701",    "TELSOM",  NULL,  NULL, NULL,"*99***1#"},
{"63704",    "SOMAFONE",  NULL,  NULL, NULL,"*99***1#"},
{"63730",    "Golis",  NULL,  NULL, NULL,"*99***1#"},
{"63782",    "Telsom Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"63801",    "Evatis",  NULL,  NULL, NULL,"*99***1#"},
{"63902",    "Safaricom",  NULL,  NULL, NULL,"*99***1#"},
{"63903",    "Celtel",  NULL,  NULL, NULL,"*99***1#"},
{"64002",    "MOBITEL",  NULL,  NULL, NULL,"*99***1#"},
{"64003",    "ZANTEL",  NULL,  NULL, NULL,"*99***1#"},
{"64004",    "Vodacom Tanzania Limited",  NULL,  NULL, NULL,"*99***1#"},
{"64005",    "celtel",  NULL,  NULL, NULL,"*99***1#"},
{"64101",    "CelTel Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"64110",    "MTN-UGANDA",  NULL,  NULL, NULL,"*99***1#"},
{"64111",    "Uganda Telecom",  NULL,  NULL, NULL,"*99***1#"},
{"64201",    "Spacetel - Burundi",  NULL,  NULL, NULL,"*99***1#"},
{"64202",    "SAFARIS",  NULL,  NULL, NULL,"*99***1#"},
{"64203",    "ONATEL",  NULL,  NULL, NULL,"*99***1#"},
{"64282",    "Telecel-Burundi Company",  NULL,  NULL, NULL,"*99***1#"},
{"64301",    "mCel",  NULL,  NULL, NULL,"*99***1#"},
{"64304",    "Vodacom Mozambique",  NULL,  NULL, NULL,"*99***1#"},
{"64501",    "CELTEL",  NULL,  NULL, NULL,"*99***1#"},
{"64502",    "MTN ZAMBIA",  NULL,  NULL, NULL,"*99***1#"},
{"64601",    "Madacom",  NULL,  NULL, NULL,"*99***1#"},
{"64602",    "Orange Madagascar",  NULL,  NULL, NULL,"*99***1#"},
{"64604",    "Telma Mobile",  NULL,  NULL, NULL,"*99***1#"},
{"64700",    "Orange reunion",  NULL,  NULL, NULL,"*99***1#"},
{"64702",    "OUTREMER TELECOM",  NULL,  NULL, NULL,"*99***1#"},
{"64710",    "SFR REUNION",  NULL,  NULL, NULL,"*99***1#"},
{"64801",    "Net*One Cellular (Pvt) Ltd",  NULL,  NULL, NULL,"*99***1#"},
{"64803",    "TELECEL",  NULL,  NULL, NULL,"*99***1#"},
{"64804",    "ECONET",  NULL,  NULL, NULL,"*99***1#"},
{"64901",    "MTC",  NULL,  NULL, NULL,"*99***1#"},
{"65001",    "TELEKOM",  NULL,  NULL, NULL,"*99***1#"},
{"65010",    "Celtel",  NULL,  NULL, NULL,"*99***1#"},
{"65101",    "VODACOM LESOTHO",  NULL,  NULL, NULL,"*99***1#"},
{"65102",    "Econet Ezi-Cel",  NULL,  NULL, NULL,"*99***1#"},
{"65201",    "MASCOM",  NULL,  NULL, NULL,"*99***1#"},
{"65202",    "ORANGE",  NULL,  NULL, NULL,"*99***1#"},
{"65310",    "Swazi MTN Limited",  NULL,  NULL, NULL,"*99***1#"},
{"65401",    "HURI",  NULL,  NULL, NULL,"*99***1#"},
{"65501",    "VodaCom",  NULL,  NULL, "internet", "*99***1#"},
{"65507",    "Cell-C",  NULL,  NULL, "internet", "*99***1#"},
{"65510",    "MTN",  NULL,  NULL, "internet", "*99***1#"},
{"70267",    "Belize Telecommunications",  NULL,  NULL, NULL,"*99***1#"},
{"70401",    "CLARO",  NULL,  NULL, NULL,"*99***1#"},
{"70402",    "COMCEL GUATEMALA",  NULL,  NULL, NULL,"*99***1#"},
{"70403",    "Telefonica",  NULL,  NULL, NULL,"*99***1#"},
{"70601",    "CTE Telecom Personal SA de CV",  NULL,  NULL, NULL,"*99***1#"},
{"70602",    "Digicel",  NULL,  NULL, NULL,"*99***1#"},
{"70603",    "Telemovil EL Salvador",  NULL,  NULL, NULL,"*99***1#"},
{"70604",    "Telefonica",  NULL,  NULL, NULL,"*99***1#"},
{"70801",    "CLARO GSM",  NULL,  NULL, NULL,"*99***1#"},
{"70802",    "CELTEL",  NULL,  NULL, NULL,"*99***1#"},
{"70830",    "Empresa Hondurena de Telecomunicaciones",  NULL,  NULL, NULL,"*99***1#"},
{"71021",    "CLARO NIC",  NULL,  NULL, NULL,"*99***1#"},
{"71073",    "CLARO NIC",  NULL,  NULL, NULL,"*99***1#"},
{"71030",    "Telefonia Celular de Nicaragua S.A",  NULL,  NULL, NULL,"*99***1#"},
{"71201",    "IceRegular",  NULL,  NULL, "iceregular", "*99***1#"},
{"71401",    "Cable & Wireless",  NULL,  NULL, "apn01.cwpanama.com.pa", "*99***1#"},
{"71420",    "Movistar",  NULL,  NULL, NULL,"*99***1#"},
{"71606",    "Movistar Peru",  "movistar@datos",  "movistar", "movistar.pe","*99***1#"},
{"71610",    "CLARO(TIM)",  "tim",  "tulibertad", "tim.pe", "*99***1#"},
{"72207",    "Movistar",  NULL,  NULL, NULL,"*99***1#"},
{"72234",    "Personal",  NULL,  "adgj", "gprs.personal.com", "*99***1#"},
{"72231",    "CTI Movil",  NULL,  NULL, NULL,"*99***1#"},
{"72402",    "TIM",  "tim",  "tim", "tim.br", "*99***1#"},
{"72405",    "Claro",  "claro",  "claro", "claro.com.br", "*99***1#"},
{"72406",    "Vivo",  NULL,  NULL, NULL,"*99***1#"},
{"72410",    "Vivo",  NULL,  NULL, NULL,"*99***1#"},
{"72411",    "Vivo",  NULL,  NULL, NULL,"*99***1#"},
{"72415",    "SERCOMTEL",  NULL,  NULL, NULL,"*99***1#"},
{"72416",    "Brasil Telecom Celular",  NULL,  NULL, NULL,"*99***1#"},
{"72423",    "Telemig Celular S/A",  NULL,  NULL, NULL,"*99***1#"},
{"72424",    "Amazonia Celular S/A",  NULL,  NULL, NULL,"*99***1#"},
{"72431",    "Oi",  NULL,  NULL, "gprs.oi.com.br", "*99***1#"},
{"72432",    "CTBC Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"72433",    "CTBC Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"72434",    "CTBC Cellular",  NULL,  NULL, NULL,"*99***1#"},
{"73001",    "Entel PCS",  "entelpcs",  "entelpcs", "imovil.entelpcs.cl", "*99***1#"},
{"73002",    "TELEFONICA",  "web",  "web", "web.tmovil.cl","*99***1#"},
{"73003",    "CLARO CHILE SA",  NULL,  NULL, NULL,"*99***1#"},
{"73010",    "Telefonica Movil",  "web",  "web", "web.tmovil.cl", "*99***1#"},
{"732101",    "Comunicacion Celular SA Comcel SA",  NULL,  NULL, NULL,"*99***1#"},
{"732103",    "Tigo",  NULL,  NULL, NULL,"*99***1#"},
{"732111",    "Tigo",  NULL,  NULL, NULL,"*99***1#"},
{"732111",    "Colombia Telecomunicaciones S.A. ESP",  NULL,  NULL, NULL,"*99***1#"},
{"732123",    "movistar",  "movistar",  "movistar", "internet.movistar.com.co","*99***1#"},
{"73401",    "DIGITEL GSM",  NULL,  NULL, NULL,"*99***1#"},
{"73402",    "DIGITEL TIM",  NULL,  NULL, "gprsweb.digitel.ve", "*99***1#"},
{"73403",    "DIGITEL GSM",  NULL,  NULL, NULL,"*99***1#"},
{"73404",    "Movistar",  NULL,  NULL, "internet.movistar.ve","*99***1#"},
{"73601",    "uevatel PCS De Bolivia SA",  NULL,  NULL, NULL,"*99***1#"},
{"73602",    "Entel",  NULL,  NULL, NULL,"*99***1#"},
{"73603",    "TELECEL BOLIVIA",  NULL,  NULL, NULL,"*99***1#"},
{"73801",    "Digicel Guyana",  NULL,  NULL, NULL,"*99***1#"},
{"73802",    "Cellink Plus",  NULL,  NULL, NULL,"*99***1#"},
{"74000",    "MOVISTAR",  NULL,  NULL, NULL,"*99***1#"},
{"74001",    "Conecel S.A",  NULL,  NULL, NULL,"*99***1#"},
{"74401",    "VOX",  NULL,  NULL, NULL, "*99***1#"},
{"74402",    "AMX PARAGUAY S.A",  NULL,  NULL, NULL, "*99***1#"},
{"74404",    "Telecel Paraguay",  NULL,  NULL, NULL, "*99***1#"},
{"74405",    "Personal",  NULL,  NULL, NULL,"*99***1#"},
{"74602",    "TELESUR.GSM",  NULL,  NULL, NULL,"*99***1#"},
{"74801",    "ANCEL",  NULL,  NULL, "gprs.ancel", "*99***1#"},
{"74807",    "MOVISTAR",  NULL,  NULL, NULL,"*99***1#"},
{"74810",    "AM Wireless Uruguay S.A",  NULL,  NULL, NULL,"*99***1#"},
{"75001",    "Cable and Wireless",  NULL,  NULL, NULL,"*99***1#"},
{"79502",    "Altyn Asyr",  NULL,  NULL, NULL,"*99***1#"},
{"90105",    "Thuraya Satellite Telecommunications Co",  NULL,  NULL, NULL,"*99***1#"},
{"90106",    "Thuraya Satellite Telecommunications Co",  NULL,  NULL, NULL,"*99***1#"},
{"90112",    "Maritime Communications Partner AS",  NULL,  NULL, NULL,"*99***1#"},
{"90114",    "AeroMobile",  NULL,  NULL, NULL,"*99***1#"},
{"90115",    "ONAIR",  NULL,  NULL, NULL,"*99***1#"},
{"90117",    "Navitas",  NULL,  NULL, NULL,"*99***1#"},
{NULL, NULL, NULL, NULL, NULL}
};

