/*
 * Linux hotplug support
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */
#include  <dirent.h> 
#include "hotplug.h"

typedef struct _fn_table{
	char type[12];
	int (*pFunc)(void);
}fn_table;

extern unsigned char        		g_str_vendor[8],g_str_product[8];

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

// ********* code from syscall.c to kill processes ***************
/***************************************************************************
// Function Name: bcmSystem().
// Description  : launch shell command in the child process.
// Parameters   : command - shell command to launch.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
extern char **environ;
int 
bcmSystem (char *command) {
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

typedef struct proc_s {
	char cmd[16];					/* basename of executable file in call to exec(2) */
	int ruid;						/* real only (sorry) */
	int pid;						/* process id */
	int ppid;						/* pid of parent process */
	char state;						/* single-char code for process state (S=sleeping) */
	unsigned int vmsize;			/* size of process as far as the vm is concerned */
} proc_t;


static int file2str_ex(char *filename, char *ret, int cap)
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


static int parse_proc_status(char *S, proc_t * P)
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
		if ((file2str_ex(path, sbuf, sizeof sbuf)) != -1) {
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
				  {
				  	//PRINTF("name=%s\n", stmp);
					break;
				  }
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

#define READ_BUF_SIZE        128

static void remove_delimitor( char *s)
{
    char *p1, *p2;

    p1 = p2 = s;
    while ( *p1 != '\0' || *(p1+1) != '\0') {
        if (*p1 != '\0') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

}
/* find_pid_by_name()
 *
 *  This finds the pid of the specified process.
 *  Currently, it's implemented by rummaging through
 *  the proc filesystem.
 *
 *  Returns a list of all matching PIDs
 */
static pid_t* find_pid_by_name( char* pidName)
{
        DIR *dir;
        struct dirent *next;
        pid_t* pidList=NULL;
        int i=0;

        /*FILE *status */
        FILE *cmdline;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        /* char name[READ_BUF_SIZE]; */
                
        dir = opendir("/proc");
        if (!dir) {
                printf("cfm:Cannot open /proc");
                return NULL;
        }

        while ((next = readdir(dir)) != NULL) {
                /* re-initialize buffers */
                memset(filename, 0, sizeof(filename));
                memset(buffer, 0, sizeof(buffer));  

                /* Must skip ".." since that is outside /proc */
                if (strcmp(next->d_name, "..") == 0)
                        continue;

                /* If it isn't a number, we don't want it */
                if (!isdigit(*next->d_name))
                        continue;

                /* sprintf(filename, "/proc/%s/status", next->d_name); */
                /* read /porc/<pid>/cmdline instead to get full cmd line */
                sprintf(filename, "/proc/%s/cmdline", next->d_name);
                if (! (cmdline = fopen(filename, "r")) ) {
                        continue;
                }
                if (fgets(buffer, READ_BUF_SIZE-1, cmdline) == NULL) {
                        fclose(cmdline);
                        continue;
                }
                fclose(cmdline);

                /* Buffer should contain a string like "Name:   binary_name" */
                /*sscanf(buffer, "%*s %s", name);*/
                /* buffer contains full commandline params separted by '\0' */
                remove_delimitor(buffer);
                if (strstr(buffer, pidName) != NULL) {
                        pidList=realloc( pidList, sizeof(pid_t) * (i+2));
                        if (!pidList) {
                                printf("cfm: Out of memeory!\n");
				closedir(dir);
                                return NULL;
                        }
                        pidList[i++]=strtol(next->d_name, NULL, 0);
                }
        }
        closedir(dir);

        if (pidList)
                pidList[i]=0;
        else if ( strcmp(pidName, "init")==0) {
                /* If we found nothing and they were trying to kill "init",
                 * guess PID 1 and call it good...  Perhaps we should simply
                 * exit if /proc isn't mounted, but this will do for now. */
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=1;
        } else {
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=-1;
        }
        return pidList;
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

/* ld append */
int hotplug_usb(char *event)
{
	FILE *fp;
	pid_t *pid_t=NULL,pid_self=0;
	int   ret = 0;
	char *devpath = NULL,*action = NULL,*product = NULL, *vid = NULL, *pid = NULL, *ptr = NULL;
	char line[32],tmppid[4],tmpvid[4];

	fp = fopen("/var/usbhotplug", "a");
	if (fp){
		fclose(fp);
	}

	if(!(devpath = getenv("DEVPATH")))
	{
		printf("\nin hotplug_usb DEVPAHT=NULL\n");
	}
	else
		printf("\nin hotplug_usb DEVPAHT=%s\n",devpath);

	action = getenv("ACTION");	
	if(!action)
	{
		printf("\nin hotplug_usb ACTION=NULL\n");
	}
	else
		printf("\nin hotplug_usb ACTION=%s\n",action);

	product = getenv("PRODUCT");
	if(!product)
	{
		printf("\nin hotplug_usb PRODUCT=NULL\n");
	}
	else
		printf("\nin hotplug_usb PRODUCT=%s\n",product);

	if(!strcmp(action,"add"))
	{
		if(product)
		{
			vid = product;
			ptr = strchr(product,'/');
			if ( ptr ) 
			{ 
				*ptr=0; 
				ptr++; 
				pid = ptr;
				ptr = strchr(ptr,'/');
						
				if ( ptr ) *ptr=0;				

				convert_str(g_str_vendor, vid);
				convert_str(g_str_product, pid);
				
				printf("\nvid=%s,pid=%s\n",g_str_vendor,g_str_product);

				if(!strcmp(g_str_vendor,"0000")||(!strcmp(g_str_vendor,"04cc")&&!strcmp(g_str_product,"1520")))
					return 0;
				
				if (0 == access("/var/hotplugadd", F_OK))//说明该文件存在
					return 0;

				pid_t = find_pid_by_name("/sbin/hotplug");
			    if ( pid_t != NULL ) 
				{
			       ret = (int)(*pid_t);
			       free(pid_t);

				   pid_self = getpid();
				   
				   printf("\nhotplug is exist,pid=%d,pid_self=%d\n",ret,pid_self);

				   if(ret==pid_self)
				   		InitDeviceInfo();
				   else if(!strcmp(g_str_product,"2000"))
				   		InitDeviceInfo();				   
			    }
				else
				{
					InitDeviceInfo();
				}
			}			
		}
		else
		{
			g_str_vendor[0] = '\0';
			g_str_product[0] = '\0';
 		}
		
	}
	else if(!strcmp(action,"remove"))
	{
		if (0 == access("/var/hotplugadd", F_OK))//说明该文件存在
			unlink("/var/hotplugadd");
		
		unDeviceInfo();
	}
	
  //return 0;
  exit(0);
}

int hotplug_usb_mark(char *event)
{
	FILE 				*fp;
	
	if (0 != access("/var/usbhotplug", F_OK))
		return 0;

	//zhubo add 2010.01.08
	//fp = fopen("/var/usbhotplug", "w");
	fp = fopen("/var/usbhotplug", "a");
	if (fp){
		fprintf(fp, "%s", event);
		fclose(fp);
	}
  return 0;
}
/* end append */

int
main(int argc, char **argv)
{
    int i;
    char *base = strrchr(argv[0], '/');

#if 0
    fn_table hp_fn_table[] ={	\
    	{"usb", hotplug_usb},	\
		{"block", hotplug_storage_mountpoint},	\
		{"scsi_device", hotplug_storage_mount},	\
		{"scsi", hotplug_storage_umount},	\
        {{0},NULL}		\
    };
#else
    fn_table hp_fn_table[] ={	\
    	{"usb", hotplug_usb},	\
        {{0},NULL}		\
    };
#endif
    base = base ? base + 1 : argv[0];

    if (strstr(base, "hotplug")) {
        if (argc >= 2) {

		//zhubo add 2010.01.08
		//hotplug_usb_mark(argv[1]);

            for(i=0;hp_fn_table[i].type[0] != 0;i++){
                if (!strcmp(argv[1], (char*)(hp_fn_table[i].type)))
                    return hp_fn_table[i].pFunc();
            }			
    } else {
        fprintf(stderr, "usage: hotplug [event]\n");
        return EINVAL;
        }
    }
    return 0;
}
 