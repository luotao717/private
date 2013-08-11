#include 	<stdio.h>
#include 	<stdlib.h>
#include <string.h>
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
#include <signal.h>


#define SIG_MODEM_EXIST	         (SIGUSR1+61)
#define SIG_MODEM_NOEXIST	       (SIGUSR1+62)

static int read_goaheadpid()
{		
	char str_pid[8];
	int pid=0;
	FILE *fp;
	fp = fopen("/var/run/goahead.pid", "r");
	if (NULL == fp)
	{			
		return (-1);		
	}
	fgets(str_pid, 8, fp);
	fclose(fp);
	pid = atoi(str_pid);
	return pid;
}

int main(int argc, char *argv[])
{
	int gopid=0;
	if ( daemon(1,1) < 0 )
			{
		       perror("daemon()");
		       exit(-1);
			}
	while(1)
	{ 

		if(access("/var/hotplugadd",F_OK)==-1)
		{   
			if(access("/var/usbmodem/mode_insert",F_OK)==0)
			{
				//system("gpio l 13 0 4000 0 0 4000");
	 	       		gopid = read_goaheadpid();
				if(gopid > 0)
					kill(gopid,SIG_MODEM_NOEXIST);
			}
		}
		else
		{
			if(access("/var/usbmodem/mode_insert",F_OK)==-1)
			{
				gopid = read_goaheadpid();
				if(gopid > 0)
					kill(gopid,SIG_MODEM_EXIST);
			}
		}
	  sleep(10);
	}
}
