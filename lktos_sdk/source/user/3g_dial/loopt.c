
/*
 * system for reset button and led control
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include "gpio.h"

#define RT2860_NVRAM    0

#define SHUTDOW_DETECH_GPIO 1
#define POWEROFF_GPIO		2
#define RESETBUTTON_GPIO	10

#ifdef NOTDDIAL
#define EMERGY_GPIO			11
#define BATTERY_GPIO		12
#else
#define EMERGY_GPIO			18
#define BATTERY_GPIO		19
#endif

static unsigned char g_flash_mac_loopt[18]="00:0C:43:28:80:66";   //70:3A:D8:00:F0:A6

#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/ttyS1", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)

void create_file(char *path)
{
	FILE *fp=NULL;
	
	fp = fopen(path,"w+");
	if(fp == NULL)
		return;

	fclose(fp);
	fp = NULL;
}

void del_file(char *path)
{
	unlink(path);
}


int get_mac_addr_loopt(unsigned char *macbuf)
{
	unsigned int 	n;
	int fd=0,ret=0;

	fd = open("/dev/mtdblock0", O_RDONLY);
	if(fd <= 0)
	{
		printf("\nopen mtdblock0 failed,errno=%d\n",errno);
		return -1;
	}
	lseek(fd, (0x30000 - 32), SEEK_SET);
	ret = read(fd, macbuf, 17);
	if(ret < 0 )
	{
		printf("read id failed,errno=%d\n",errno);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}


void Set_wlan_ssid_loopt()
{
	char 			*nvram_ssid, *nvram_ssid_default=NULL; 
	unsigned char	*tmp=NULL, tmp_value[6], flash_mac[18], reset_ssid[64];
	char			macT[32];
	int i=0;

	memset(tmp_value, '\0', sizeof(tmp_value));
	memset(reset_ssid, '\0', sizeof(reset_ssid));    
	memset(flash_mac,'\0',sizeof(flash_mac));

	//nvram_ssid = nvram_bufget(RT2860_NVRAM, "SSID1");
	get_mac_addr_loopt(flash_mac);
	flash_mac[17] = '\0';

	/*
	nvram_ssid_default = nvram_bufget(RT2860_NVRAM, "SSID1_default");
	if((nvram_ssid_default==NULL)||(strlen(nvram_ssid_default)==0))
	{
		nvram_ssid_default = REALM_STR;
		nvram_bufset(RT2860_NVRAM, "SSID1_default", nvram_ssid_default);
	}
	*/

	nvram_ssid_default = REALM_STR;

	if(flash_mac[2]!=':'||flash_mac[5]!=':'||flash_mac[8]!=':'||flash_mac[11]!=':'||flash_mac[14]!=':')
		tmp = g_flash_mac_loopt;//default value
	else
		tmp = flash_mac;

	sscanf(tmp,"%02x:%02x:%02x:%02x:%02x:%02x\0",&tmp_value[0],&tmp_value[1],&tmp_value[2],&tmp_value[3],&tmp_value[4],&tmp_value[5]);

	memset(macT,'\0',sizeof(macT));	
	sprintf(macT,"%02x%02x",tmp_value[4],tmp_value[5]);	
	for(i=0;i<strlen(macT);i++)	
	{		
		macT[i]=toupper(macT[i]);	
	}

	sprintf(reset_ssid,"%s_%s",nvram_ssid_default,macT);
	
	nvram_bufset(RT2860_NVRAM, "SSID1", reset_ssid);
	nvram_commit(RT2860_NVRAM);
}

int main(int argc, char *argv[])
//int main(int argc, char **argv)
{
	FILE *file;
	int c;
	char wanipaddr[20];
	char wangateway[20];
	char wanifname[12];
	int wan_proto;
	int ip1, ip2, ip3, ip4;
	int count = 0;
	int reset_count = 0, poweroff_count=0, battery_count=0, emergy_low_count=0,emergy_high_count=0;
	int reset_flag = 0, detech_flag = 0, emergy_old_flag=0, emergy_new_flag=0;
	int restart_ntp_count = 0;
	int wan_link_status = 0;
	time_t  t_prev;
	time_t t_now;
	unsigned char mac[6];
	char wanhwaddr[20];
	int wan_keeptime = 0;
	int arpspoof_count = 0;
	int adv_count = 0;
	char sipaddr[20];
	char dipaddr[20];
	int protocol;
	char sportstr[20];
	char dportstr[20];
	char protstr[12];
	char devstr[20];
	int ddns_count = 0;
	int len = 0;
	int check_enable = 0;
	int always_check = 0;
	int version_check = 0;
	static int statsold=0,statsnew=0;
		
	daemon(1, 1);

	gpio_write_output(POWEROFF_GPIO,1);

	system("gpio l 7 0 4000 0 0 4000"); //high
	emergy_old_flag = 1; //default high

	time(&t_prev);	
	reset_flag = 0;
	
	while(1)
	{
		c = gpio_read_input(RESETBUTTON_GPIO);

		if (c == 0 )
			reset_count++;
		else
			reset_count=0;

		if (reset_count >= 3)
			reset_flag = 1;		
		
		if (reset_flag == 1 )
		{
			//reset the modem
			//system("gpio l 7 4000 0 0 0 4000");  //low
			//sleep(60);
			
			// WIFI LED DOWN
			system("ifconfig ra0 down");

			// SIGNAL LED DOWN
			system("gpio l 0 0 4000 0 0 4000"); // LEVEL 1
			system("gpio l 21 0 4000 0 0 4000"); // LEVEL 2
			system("reg w 60 40025d"); // LEVEL 3

			// 3G LED DOWN
			system("gpio l 17 0 4000 0 0 4000");
			system("gpio l 20 0 4000 0 0 4000");
			gpio_set_led(13, 0, 4000, 1, 1, 4000);			

			system("ralink_init clear 2860");
			system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
			sleep(3);
			#ifndef NOSET_WLAN_MAC
				Set_wlan_ssid_loopt();
			#endif
			
			system("reboot");
		}
/***************end the rest gpio*************************************************/		

		#ifndef NOTDDIAL
			c = gpio_read_input(BATTERY_GPIO);
			if( c == 0 )
				battery_count++;
			else
				battery_count = 0;
	
			if(battery_count >= 5)
			{
				//kill(-1,SIGTERM);
				//sync();
				//sleep(1);
				gpio_write_output(POWEROFF_GPIO,0);
				battery_count = 0;
			}
		#endif
/**************end the battery gpio************************************************/		

		/*
		c = gpio_read_input(SHUTDOW_DETECH_GPIO);
		if( c > 0 )
			poweroff_count++;
		else
		{
			if(detech_flag)
				gpio_write_output(POWEROFF_GPIO,0);
			else
				poweroff_count = 0;
		}			

		if(poweroff_count >= 3)
			detech_flag = 1;
		*/	
/**************end the poweroff gpio***********************************************/

		/*
		if(statsold==0)
		{
			//printf("0\n");
			//gpio_set_dir_out(0);
			//gpio_write_output(0,1);
			system("gpio l 0 4000 0 0 0 4000");  //low
			statsold = 1;
		}			
		else
		{
			//printf("1\n");
			//gpio_set_dir_in(0);
			//gpio_write_output(0,0);
			system("gpio l 0 0 4000 0 0 4000"); //high
			statsold = 0;
		}
		*/
		#ifndef NOTDDIAL
			c = gpio_read_input(EMERGY_GPIO);
			if( c == 0 )
			{
				emergy_low_count++;
				emergy_high_count = 0;
			}			
			else
			{
				emergy_high_count++;
				emergy_low_count = 0;
			}
				
	
			if(emergy_low_count >= 3)
			{
				emergy_low_count = 0;
				emergy_new_flag = 0;
	
				if(emergy_old_flag==1)
				{
					system("gpio l 9 4000 0 0 0 4000");  //low
					
					if(access("/var/usbmodem/lowpower",F_OK) == -1)
						create_file("/var/usbmodem/lowpower");
				}				
	
				emergy_old_flag = 0;
			}
	
			if(emergy_high_count >=3)
			{
				emergy_high_count = 0;
				emergy_new_flag = 1;
	
				if(emergy_old_flag==0)
				{
					system("gpio l 9 0 4000 0 0 4000"); //high
	
					if(access("/var/usbmodem/lowpower",F_OK) == 0)
						del_file("/var/usbmodem/lowpower");
				}				
	
				emergy_old_flag = 1;
			}
		#endif

/**************end the emergy gpio***********************************************/
				
		sleep(1);
	}
	
	return 0;
}
