/*
 * hotplug_net_nas.c - hotplug handler for nas
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
#include "hotplug.h"

/* Print directly to the console */
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)


/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
static char * fd2str(int fd)
{
	char *buf = NULL;
	size_t count = 0, n;

	do {
		buf = realloc(buf, count + 512);
		n = read(fd, buf + count, 512);
		if (n < 0) {
			free(buf);
			buf = NULL;
		}
		count += n;
	} while (n == 512);

	close(fd);
	if (buf)
		buf[count] = '\0';
	return buf;
}

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
static char * file2str(const char *path)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd);
}

int
notify_nas(char *type, char *ifname, char *action)
{


	char *argv[] = {"nas4not", type, ifname, action, 
			NULL,	/* role */
			NULL,	/* crypto */
			NULL,	/* auth */
			NULL,	/* passphrase */
			NULL,	/* ssid */
			NULL};

	char *str=NULL,*str_role=NULL, *str_crypto=NULL, *str_auth=NULL, *str_key=NULL,*str_ssid=NULL;
	int retries = 30;
	int retval=0;
#ifdef BCMDBG
	char cmd[128];
#endif

	/* wait till nas is started, wait for the main nas process */
	while (retries -- > 0 && !(str = file2str("/var/nas.lan0.pid"))){	
		sleep(1);
	}
			
	if (str) {
		
		int pid;
		free(str);		
		
		/* check if we use wpa and require to notify nas*/								
		str_role=file2str("/var/notify_nas_role");	argv[4]=str_role;
		str_crypto=file2str("/var/notify_nas_crypto");  argv[5]=str_crypto;
		str_auth=file2str("/var/notify_nas_auth");  	argv[6]=str_auth;
		str_key=file2str("/var/notify_nas_key");  	argv[7]=str_key;
		str_ssid=file2str("/var/notify_nas_ssid");   	argv[8]=str_ssid;
#ifdef BCMDBG
		
		sprintf(cmd,"echo argv[0]=%s > /var/nas_dbg",argv[0]); 	bcmSystem(cmd);			
		sprintf(cmd,"echo argv[1]=%s >> /var/nas_dbg ",argv[1]);bcmSystem(cmd);
		sprintf(cmd,"echo argv[2]=%s >> /var/nas_dbg",argv[2]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[3]=%s >> /var/nas_dbg",argv[3]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[4]=%s >> /var/nas_dbg",argv[4]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[5]=%s >> /var/nas_dbg",argv[5]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[6]=%s >> /var/nas_dbg",argv[6]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[7]=%s >> /var/nas_dbg",argv[7]);	bcmSystem(cmd);
		sprintf(cmd,"echo argv[8]=%s >> /var/nas_dbg",argv[8]);	bcmSystem(cmd);

#endif
		retval= _eval(argv, ">/dev/console", 0, &pid);	

		free(str_role);free(str_crypto);free(str_auth);free(str_key);free(str_ssid);
		
		return retval;
	}	
	return -1;	
}
