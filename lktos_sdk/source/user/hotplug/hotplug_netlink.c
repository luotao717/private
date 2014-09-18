#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
/*add by cui 2009.08.31*/
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "hotplug.h"


/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})

int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);

int mc8780_flag;

/**/

#define DEBUG

//#define ADD_MODULES	0
//#define REMOVE_MODULES	1

#define UEVENT_BUFFER_SIZE 2048

#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/ttyS1", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)

#ifdef DEBUG
#define DEBUGF(fmt, args...)  do { cprintf(fmt, ## args); } while (0)
#else
#define DEBUGF(fmt, args...)
#endif

#define ERROR(fmt, args...)  do { cprintf(fmt, ## args); } while (0)

#define DNOTCHECK_DEV	0
#define CHECK_TTY	1
#define CHECK_BLOCK	2
#define CHECK_DEV_MAX	CHECK_BLOCK + 1

#define USB0_TYPE		"/dev/ttyUSB0"
#define USB1_TYPE		"/dev/ttyUSB1"
#define USB2_TYPE		"/dev/ttyUSB2"
#define USB3_TYPE		"/dev/ttyUSB3"
#define USB4_TYPE		"/dev/ttyUSB4"
#define USB5_TYPE		"/dev/ttyUSB5"
#define USB9_TYPE		"/dev/ttyUSB9"
#define HS1_TYPE		"/dev/ttyHS1"
#define ACM0_TYPE		"/dev/ttyACM0"

static char 	devname[CHECK_DEV_MAX][20]={"","tty","block"};
static int 		checkdev=DNOTCHECK_DEV;
unsigned char   g_str_vendor[8],g_str_product[8];
char			Modem_usbtype[32];
int 			modem_type=2;// 2:no modem;1:option modem;0:not option modem

int				eval_flag=0;

static int init_hotplug_sock(void);

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

int main(int argc, char* argv[])
{	
	int hotplug_sock = init_hotplug_sock();
	int i, len, act=-1;
	char buf[UEVENT_BUFFER_SIZE*2] = {0};
	char *ptr,*tmp;
	char *action, *devpath, *modalias,*product, *subsys;
	int ckdflag = 0;
	char *vid=NULL,*pid=NULL;
	char *usb_flag=NULL;
	int action_flag=0;
	FILE *fp;
	mc8780_flag=0;
	
	if ( hotplug_sock == -1 )
	{
		ERROR("Can't open NetLink socket\n");
		return 0;
	}
	
	if ( daemon(1,1) < 0 )
	{
                perror("daemon()");
               exit(-1);
	}
	
	while(1){
		buf[0]=0;
		len = recv(hotplug_sock, &buf, sizeof(buf), 0); 

		i = 0; ptr=buf;
		action=NULL; devpath=NULL; modalias=NULL;
		product=NULL; subsys=NULL;

		while (i<len) {
			tmp = strchr(ptr, '=' );
			
			if ( tmp ) 
			{
				*tmp = 0;
				
				if ( strcmp(ptr, "ACTION" )==0 ) action = tmp+1;
				else if ( strcmp(ptr, "DEVPATH" )==0 ) devpath = tmp+1;
//				else if ( strcmp(ptr, "MODALIAS" )==0 ) modalias = tmp+1;
				else if ( strcmp(ptr, "PRODUCT" )==0 ) product = tmp+1; 
				else if ( strcmp(ptr, "SUBSYSTEM") == 0 ) subsys = tmp + 1;				
			}
			
			i += strlen(buf+i)+1;
			ptr = buf + i;			
		}

		usb_flag=strstr(devpath,"usb");

		if((usb_flag!=NULL) && !strcmp(action,"remove"))
		{ //is not usb device			
			if(mc8780_flag==0)
			{
				if (0 == access("/var/hotplugadd", F_OK))//说明该文件存�?
				{
					unlink("/var/hotplugadd");
				}
		
				unDeviceInfo();
			}
		}
		else if ( (usb_flag!=NULL) && !strcmp(action,"add") && devpath && product )
		{
			vid = product;
			ptr = strchr(product,'/');
			
			if ( ptr ) 
			{ 
				*ptr=0; 
				ptr++; 
				pid = ptr;
				ptr = strchr(ptr,'/');
				
				if ( ptr ) 
					*ptr=0;

				convert_str(g_str_vendor, vid);
				convert_str(g_str_product, pid);
				
				printf("\nvid=%s,pid=%s\n",g_str_vendor,g_str_product);
				if(!strcmp(g_str_vendor,"0000")||!strcmp(g_str_vendor,"05e3")||(!strcmp(g_str_vendor,"04cc")&&!strcmp(g_str_product,"1520")))
					continue;

				if (0 == access("/var/hotplugadd", F_OK))//说明该文件存�?
				{
					continue;
				}					
				else
				{
					fp = fopen("/var/hotplugadd", "a");
					if (fp){
						fclose(fp);
					}
				}

				InitDeviceInfo();
			}		
			else
			{
				g_str_vendor[0] = '\0';
				g_str_product[0] = '\0';
			}
		}
		
	}
	
	return 0;
}

static int init_hotplug_sock(void)
{
	struct sockaddr_nl snl;
	const int buffersize = 16 * 1024 * 1024;
	int retval;

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;
	int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);

	if (hotplug_sock == -1) {
		ERROR("error getting socket: %s", strerror(errno));
		return -1;
	}

	/* set receive buffersize */
	setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
	retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));

	if (retval < 0) {
		ERROR("bind failed: %s", strerror(errno));
		close(hotplug_sock);
		hotplug_sock = -1;
		return -1;
	}

	return hotplug_sock;
}


