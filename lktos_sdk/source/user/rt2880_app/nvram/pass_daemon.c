#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/autoconf.h>

#include "nvram.h"

struct clientInfo{
	char ip[16];
	char mac[20];
	char control;
	char always;     // 0 is  forever
	unsigned int passtime;
	unsigned int onlinetime;
	struct clientInfo *next;
};

struct clientInfo clientArr[254]={0};

static char *saved_pidfile;

static int get_mib(char *val, char *mib)
{
        FILE *fp;
         char buf[32];

        sprintf(buf, "nvram_get 2860 %s", mib);
            fp = popen(buf, "r");
        if (fp==NULL)
                return -1;

        if (NULL == fgets(buf, sizeof(buf),fp)) {
                pclose(fp);
                return -1;
        }

        //strcpy(val, strstr(buf, "\"")+1);
        strcpy(val, buf);
        val[strlen(val)-1] = '\0';
        pclose(fp);
        return 0;
}


/*
 * gpio interrupt handler -
 *   SIGUSR1 - notify goAhead to start WPS (by sending SIGUSR1)
 *   SIGUSR2 - restore default value
 */
static void nvramIrqHandler(int signum)
{
	char *temptr=NULL;
	char clientIp[16]={0};
	char clientMac[20]={0};
	int passtime=0;
	int i=0;
	FILE *irqFp=NULL;
	
	
	
	if (signum == SIGUSR1)
 {
		irqFp=fopen("/var/tempclientinfo","r");
		if(irqFp != NULL)
		{
			fscanf(irqFp,"%s %d",clientIp,&passtime);
			temptr=strchr(clientIp,'.');
			if(temptr != NULL)
			{
				temptr=strchr(temptr+1,'.');
			}
			if(temptr != NULL)
			{
				temptr=strchr(temptr+1,'.');
			}
			if(temptr != NULL)
			{
			 i=atoi(temptr+1);
			}
			strcpy(clientArr[i-2].ip,clientIp);
			clientArr[i-2].control=1;
			if(passtime != 0)
			{
				clientArr[i-2].always=1;
			}
			clientArr[i-2].passtime=passtime * 60;
                    fclose(irqFp);
		}
	}

}

static void nvramIrqHandler2(int signum)
{
	
	char *temptr=NULL;
		char clientIp[16]={0};
		char clientMac[20]={0};
		int passtime=0;
		int i=0;
		int macExist = 0 ;
		FILE *irqFp=NULL;
		char cmdbuf[256]={0};
						
		if (signum == SIGUSR2)
		 {
			irqFp=fopen("/var/passmacinfo","r");
			if(irqFp != NULL)
			{
				fscanf(irqFp,"%s %s",clientIp,clientMac);
				temptr=strchr(clientIp,'.');
				if(temptr != NULL)
				{
					temptr=strchr(temptr+1,'.');
				}
				if(temptr != NULL)
				{
					temptr=strchr(temptr+1,'.');
				}
				if(temptr != NULL)
				{
				
				 i=atoi(temptr+1);
				}
				// is mac exist ?
				int j=0;
				for (j=0;j<254;j++)
				{
					if (clientArr[j].mac == NULL)
						continue ;
					if  (!strcmp(clientArr[j].mac,clientMac)&&(!strcmp(clientArr[j].ip,clientIp)))
					{						
						fclose(irqFp);
						
						return; 
					}
					if  (!strcmp(clientArr[j].mac,clientMac))
					{
						macExist = 1 ;
						sprintf(cmdbuf,"iptables -D FORWARD -s %s -j ACCEPT",clientArr[j].ip);
						system(cmdbuf);
						sprintf(cmdbuf,"iptables -t nat -D PREROUTING -s %s -j ACCEPT",clientArr[j].ip);						
						system(cmdbuf);						
						strcpy(clientArr[j].ip,clientIp);
						clientArr[j].control = 1;
					}						
				}
				if (0 == macExist)
				{
					strcpy(clientArr[i-2].ip,clientIp);
					strcpy(clientArr[i-2].mac,clientMac);					
					clientArr[i-2].control=1;
				}
				sprintf(cmdbuf,"iptables -I FORWARD -s %s -j ACCEPT", clientIp);
				system(cmdbuf);
				sprintf(cmdbuf,"iptables -t nat -I PREROUTING -s %s -j ACCEPT ", clientIp);	
				system(cmdbuf);				
//				char tmpcmdbuf[256]={0};				
				sprintf(cmdbuf,"echo %s 0 >> /var/tempclientinfoAll",clientIp);
				system(cmdbuf);

				
				fclose(irqFp);
			}
		}
	
}

static void pidfile_delete(void)
{
	if (saved_pidfile) unlink(saved_pidfile);
}

int pidfile_acquire(const char *pidfile)
{
	int pid_fd;
	if (!pidfile) return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) {
		printf("Unable to open pidfile %s: %m\n", pidfile);
	} else {
		lockf(pid_fd, F_LOCK, 0);
		if (!saved_pidfile)
			atexit(pidfile_delete);
		saved_pidfile = (char *) pidfile;
	}
	return pid_fd;
}

void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if (pid_fd < 0) return;

	if ((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

int main(int argc,char **argv)
{
	pid_t pid;
	int fd;
	int i=0;
	struct stat filestat;
	char cmdbuf[256]={0};
	char devMac[36]={0};
	char devSn[48]={0};
	char devUserEmail[64]={0};
       time_t timep;
       struct tm *nowtime=NULL;
	signal(SIGUSR1, nvramIrqHandler);
	signal(SIGUSR2, nvramIrqHandler2);	
	fd = pidfile_acquire("/var/run/passdaemon.pid");
	pidfile_write_release(fd);
	while (1) 
	{
		sleep(10);
		for(i=0;i<254;i++)
		{
			if(clientArr[i].control)
			{
				if(clientArr[i].always)
				{
					if(clientArr[i].onlinetime > clientArr[i].passtime)
					{
                                   
						sprintf(cmdbuf,"iptables -D FORWARD -s %s -j ACCEPT",clientArr[i].ip);
						system(cmdbuf);
						sprintf(cmdbuf,"iptables -t nat -D PREROUTING -s %s -j ACCEPT",clientArr[i].ip);
						system(cmdbuf);
						clientArr[i].control=0;
						clientArr[i].always=0;
						clientArr[i].onlinetime=0;
						clientArr[i].passtime=0;
					}
					else
					{
						clientArr[i].onlinetime+=10;
					}
				}
			}
		}
             time(&timep);
            nowtime=localtime(&timep);
            //printf("\r\n now time = %d:%d:%d",nowtime->tm_hour,nowtime->tm_min,nowtime->tm_sec );
            if(nowtime->tm_hour == 5)
            {
                if((nowtime->tm_min == 0) && (nowtime->tm_sec <= 31))
                {
                    system("reboot");
                }
            }
		
	}
	return 0;
}

