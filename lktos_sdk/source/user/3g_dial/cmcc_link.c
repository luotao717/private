#include 	<stdio.h>
#include  	<stdlib.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include 	<sys/stat.h>
#include	<string.h>
#include 	<sys/ioctl.h>


#include 	<sys/socket.h>

#include 	<net/if.h>

#include 	<net/route.h>

#include 	<netinet/in.h>

#include 	<arpa/inet.h>

#include 	<net/if_arp.h>

#include 	<signal.h>
#include	 <dirent.h>
#include	 <fcntl.h>
//#include 	"3gdial.h"

#define PPPDEFGW		0x0A707070
#define PPPDEFIP		0x0A404040
#define _PPPoE_ENABLE        3
#define _CONNECT             1
#define _DISCONNECT          4
#define _LOGIN_IN_PROGRESS   2

#define cprintf(fmt, args...) do { \
        FILE *fp = fopen("/dev/ttyS1", "w"); \
        if (fp) { \
                fprintf(fp, fmt, ## args); \
                fclose(fp); \
        } \
} while (0)

typedef struct cmcc_mem
{
	int	    channel;
	char 	ssid[32];
	char	bssid[32];
	char	security[32];
	char	signal[32];
	char	mode[32];
}tagCMCC;

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


tagCMCC g_cur_cmcc;


/*
 * arguments: ifname  - interface name
 *            if_addr - a 16-byte buffer to store ip address
 * description: fetch ip address, netmask associated to given interface name
 */
static int getIfIp(char *ifname, char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	close(skfd);
	return 0;
}


int find_cmcc_net()
{
	/*
	col[0] : CHANNEL
	col[1] : SSID
	col[2] : BSSID
	col[3] : WEP/TKIP/AES
	col[4] : SIGNAL
	col[5] : MODE
	*/
	char	*scanbt=NULL, col[6][64], line[512], security[24], encry[24];
	int 	count = 0, i = 0, j = 0, k = 0, t = 0, cur_max_signal=0;
	FILE	*fp=NULL;
	
	system("iwpriv apcli0 set SiteSurvey=1");
	sleep(3);
	system("iwpriv apcli0 get_site_survey > /var/site_survey.dat");
	sleep(3);	
	
	fp = fopen("/var/site_survey.dat", "r");
	if(fp != NULL)
	{
		while ( fgets(line, sizeof(line), fp) )
		{
			if ( count++ < 2 ) 
				continue;
			if(strlen(line)>1)
			{					
				memset(col,'\0',6*64);
				strncpy(col[0],line,4);
				strncpy(col[1],line+4,33);
				strncpy(col[2],line+4+33,20);
				strncpy(col[3],line+4+33+20,23);
				strncpy(col[4],line+4+33+20+23,9);
				strncpy(col[5],line+4+33+20+23+9,8);

				{						
					char *p = NULL;
					
					for(j=0; j<6; j++)
					{
						if(j==1)
							continue;
						
						p = strchr(col[j], ' ');
						if(p) 
						{
							*p = '\0';
						}
					}
				}

				if(strncmp(col[1], "CMCC", strlen("CMCC")))
					continue;
				if(strncmp(col[3], "NONE", strlen("NONE")))
					continue;

				if(atoi(col[4]) >= cur_max_signal)
				{
					memset((char *)&g_cur_cmcc, '\0', sizeof(g_cur_cmcc));
					
					cur_max_signal = atoi(col[4]);

					g_cur_cmcc.channel=atoi(col[0]);
					strcpy((char *)&g_cur_cmcc.ssid, 	"CMCC");
					strcpy((char *)&g_cur_cmcc.bssid,	col[2]);
					strcpy((char *)&g_cur_cmcc.security,col[3]);
					strcpy((char *)&g_cur_cmcc.signal,	col[4]);
					strcpy((char *)&g_cur_cmcc.mode,	col[5]);
				}

				i++;
			}				
		}
		fclose(fp);
		unlink("/var/site_survey.dat");
	}

	//cprintf("\n in cmcc_link i=%d,channel=%d,ssid=%s,bssid=%s,security=%s,signal=%s,mode=%s",i,g_cur_cmcc.channel,g_cur_cmcc.ssid,g_cur_cmcc.bssid,g_cur_cmcc.security,g_cur_cmcc.signal,g_cur_cmcc.mode);
	
	if(i > 0)
		return i;
	else
		return 0;
		
}

//zhubo add 2010.10.14

static void SetCMCCParam()
{
	char 	*ssid, *security_mode, *channel,*encrypt_mode, cmd[128];
		
	system("iwpriv apcli0 set ApCliAuthMode=OPEN");
	system("iwpriv apcli0 set ApCliEncrypType=NONE");
	system("iwpriv apcli0 set ApCliSsid=CMCC");
	
	if(channel)
	{
		sprintf(cmd, "iwpriv ra0 set Channel=%d",g_cur_cmcc.channel);
		system(cmd);
	}
	system("iwpriv apcli0 set ApCliEnable=1");

}


int main(int argc, char **argv)
{
	FILE 	* fp;
	char 	if_addr[16];
	int		pid, cmcc_flag=0, nocmcc=0;
	
	if ( daemon(1,1) < 0 )
	{
       perror("daemon()");
       exit(-1);
	}

	while(1)
	{
		if (-1 == getIfIp("apcli0", if_addr)) // apcli0 no ipaddr
		{
			if(find_cmcc_net()) //find CMCC
			{
				cmcc_flag = 1;
				nocmcc = 0;

				system("gpio l 7 10 10 10 0 4000");// find cmcc net ¬˝…¡

				if (0 == access("/var/usbmodem/nocmcc", F_OK))
					unlink("/var/usbmodem/nocmcc");
				
				SetCMCCParam();

				pid = DBgetPid("udhcpc");
				if(pid==0)
					system("udhcpc -i apcli0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &");
			}
			else
			{
				if(access("/var/usbmodem/nocmcc",F_OK)==-1)
				{
					fp=fopen("/var/usbmodem/nocmcc","w");
					if(fp == NULL)
					{
						return -1;
					}		
					close(fp);
				}
				cmcc_flag = 0;
				nocmcc++;
				if(nocmcc>=3)
					system("gpio l 7 1 1 1 0 4000");//NO CMCC NET øÏ…¡
				
			}				
		}

		if(cmcc_flag)
			sleep(40);
		else
			sleep(10);
	}
}
