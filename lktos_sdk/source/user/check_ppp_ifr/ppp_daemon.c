#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "nvram.h"

#define TRUE 		0
#define FALSE		-1

#define PS_LOG_PATH     	"/var/log/ps.log"
#define DEV_LOG_PATH    	"/var/log/dev.log"
#define OPENL2TPD_PID_PATH	"/var/run/openl2tpd.pid"
#define PPPD_PID_PATH		"/var/run/ppp0.pid"	
#define PPPD_OPTIONS_FILE	"/etc/options.pptp"

char *dev = "ppp0";
char *l2tp = "openl2tpd";
char *pppd = "pppd";

int search_str(const char *dst, const char *src)
{	
    while ('\0' != *src)
    {
        if (0 == strncmp(dst, src, strlen(dst)))
            return TRUE;

		src ++;
    }
 
    return FALSE;
}

int check_dev(char *dev)
{   
	int nRet;
    char dev_info[200];
    char cmd[50];
    FILE *fp;
    
    sprintf(cmd, "ifconfig | grep %s >"DEV_LOG_PATH, dev);
    system(cmd),
    
    fp = fopen(DEV_LOG_PATH, "r");
    if (NULL == fp)
        return FALSE;

    nRet = fread(dev_info, 1, sizeof(dev_info), fp);
	if (nRet == 0)
	{
		fclose(fp);
		return FALSE;
		
	}
	
    fclose(fp);
	return search_str(dev, dev_info);
}

#define MAX_BUF_SIZE		256

int check_ps(char *ps)
{
	int nRet;
    char ps_info[MAX_BUF_SIZE];
    char cmd[100];
    FILE *fp;

    sprintf(cmd, "ps -ef | grep %s | grep -v ""grep"" >"PS_LOG_PATH, ps);
    system(cmd);

    fp = fopen(PS_LOG_PATH, "r");
    if (NULL == fp)
        return FALSE;
	
   	nRet = fread(ps_info, 1, sizeof(ps_info), fp);
	if (nRet <= 0)
	{
		fclose(fp);
		return FALSE;
	}
	
    fclose(fp);
	return search_str(ps, ps_info);
}

int main(int argc, char **argv)
{
	char *cmd[50];
	char *wan_mode;
	
	while(1)
	{		
		if (TRUE != check_dev(dev))
		{	
			wan_mode = nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
			
			if (0 == strncmp(wan_mode, "L2TP", 5))
			{
				if (FALSE != access(OPENL2TPD_PID_PATH, F_OK))
				{
					system("rm "OPENL2TPD_PID_PATH);
				}
				
	            if (TRUE == check_ps(l2tp))
	            {
					sprintf(cmd, "killall %s", l2tp);
					usleep(100 * 1000);
					system(cmd);
	            }
	            else
	            {
					system(l2tp);
	            }
			}
			else if (0 == strncmp(wan_mode, "PPTP", 5))
			{
				if (FALSE != access(PPPD_PID_PATH, F_OK))
				{
					system("rm "PPPD_PID_PATH);
				}
				
	            if (TRUE == check_ps(pppd))
	            {
					sprintf(cmd, "killall %s", pppd);
					usleep(100 * 1000);
					system(cmd);
	            }
	            else
	            {
	            	sprintf(cmd, "%s file "PPPD_OPTIONS_FILE, pppd);
					system(cmd);
	            }
			}
		}	
		usleep(20 * 1000 * 1000);
	}
}
