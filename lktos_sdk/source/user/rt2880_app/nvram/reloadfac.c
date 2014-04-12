#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <linux/autoconf.h>
#include "ralink_gpio.h"
#define GPIO_DEV	"/dev/gpio"
#include <time.h>
#if 0
#define DEBUG_PRINT printf

#else
#define DEBUG_PRINT
#endif

static int gpio_get_data(int irq)
{
	int fd;
	int ledon=0;
	int rData=0;
	int gpioValue=0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return;
	}
	//if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, 1<<9) < 0) {
	if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, 1<<irq) < 0) {//for test
		perror("ioctl");
		close(fd);
		return;
	}
	if (ioctl(fd, RALINK_GPIO_READ, &rData) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	gpioValue=rData&(1<<irq);
	close(fd);
	if(gpioValue)
		return 0;
	else
		return 1;
	//printf("\r\ntest gpio value =%0x",rData);
}

int main(int argc, char *argv[])
{
	
	int gpio_state=0;
	int gpioCount=0;
	while(1)
	{
		gpio_state=gpio_get_data(7);
		if(gpio_state)
		{
			printf("\r\ncount = %d",gpioCount);
			if(gpioCount>2)
			{
				printf("\r\nthen load default and reboot");
				system("gpio l 8 1 1 4000 1 1");
				system("gpio l 21 1 1 4000 1 1");
				system("ralink_init clear 2860");
				system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
				sleep(2);
				system("reboot");
			}
			else
			{
				gpioCount++;
			}
		}
		else
		{
			gpioCount=0;
		}  	  
		
		sleep(2);
	}
	return 0;
}
	
