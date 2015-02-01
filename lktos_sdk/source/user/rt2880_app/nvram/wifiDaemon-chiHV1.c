#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>

#include "nvram.h"

#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE)
#include "ralink_gpio.h"
#define GPIO_DEV "/dev/gpio"
#endif



static int wifion=1;


/*
 * gpio interrupt handler -
 *   SIGUSR1 - notify goAhead to start WPS (by sending SIGUSR1)
 *   SIGUSR2 - restore default value
 */
static void nvramIrqHandler(int signum)
{
	if (signum == SIGUSR1) 
	{
		if(wifion)
		{
			system("ifconfig ra0 down");
			wifion=0;
		}
		else
		{
			system("ifconfig ra0 up");
			wifion=1;
		}
	}
}

/*
 * init gpio interrupt -
 *   1. config gpio interrupt mode
 *   2. register my pid and request gpio pin 0
 *   3. issue a handler to handle SIGUSR1 and SIGUSR2
 */
int initGpio(void)
{
#if !defined (CONFIG_RALINK_GPIO) && !defined (CONFIG_RALINK_GPIO_MODULE)
	signal(SIGUSR1, nvramIrqHandler);
	signal(SIGUSR2, nvramIrqHandler);
	return 0;
#else
	int fd;
	ralink_gpio_reg_info info;

	info.pid = getpid();
#if defined (CONFIG_RALINK_RT2880)
	info.irq = 0;
#elif defined (CONFIG_RALINK_RT3052) && ((CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE))
	info.irq = 43;
#elif defined (CONFIG_RALINK_RT3883)
	//RT3883 uses gpio 27 for load-to-default
	info.irq = 27;
#elif defined (CONFIG_RALINK_RT6855A)
#if defined (CONFIG_RT6855A_PCIE_PORT0_ENABLE)
	info.irq = 0;	// rt6855 reset default
#else
	info.irq = 2;	// rt6856 reset default
#endif
#elif defined (CONFIG_RALINK_MT7620)
	info.irq = 1;	// MT7620 reset default
#else
	//RT2883, RT3052, RT3352, RT6855 use gpio 10 for load-to-default
	info.irq = 1;//for chi device
#endif	

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	//set gpio direction to input
#if !defined (CONFIG_RALINK_RT6855A)
	if (info.irq < 24) {
		if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, (1<<info.irq)) < 0)
			goto ioctl_err;
	}
#if defined (RALINK_GPIO_HAS_2722)
	else if (22 <= info.irq) {
		if (ioctl(fd, RALINK_GPIO2722_SET_DIR_IN, (1<<(info.irq-22))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_4524)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else {
		if (ioctl(fd, RALINK_GPIO4540_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_5124)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else {
		if (ioctl(fd, RALINK_GPIO5140_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else if (40 <= info.irq && info.irq < 72) {
		if (ioctl(fd, RALINK_GPIO7140_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
	else {
#if defined (RALINK_GPIO_HAS_7224)
		if (ioctl(fd, RALINK_GPIO72_SET_DIR_IN, (1<<(info.irq-72))) < 0)
#else
		if (ioctl(fd, RALINK_GPIO9572_SET_DIR_IN, (1<<(info.irq-72))) < 0)
#endif
			goto ioctl_err;
	}
#endif
#else
	if (info.irq < 16) {
		if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, info.irq) < 0)
			goto ioctl_err;
	} else {
		goto ioctl_err;
	}
#endif
	//enable gpio interrupt
	if (ioctl(fd, RALINK_GPIO_ENABLE_INTP) < 0)
		goto ioctl_err;

	//register my information
	if (ioctl(fd, RALINK_GPIO_REG_IRQ, &info) < 0)
		goto ioctl_err;
	close(fd);

	//issue a handler to handle SIGUSR1 and SIGUSR2
	signal(SIGUSR1, nvramIrqHandler);
	signal(SIGUSR2, nvramIrqHandler);
	return 0;

ioctl_err:
	perror("ioctl");
	close(fd);
	return -1;
#endif
}


int main(int argc,char **argv)
{
	pid_t pid;
	int fd;

	
	if (initGpio() != 0)
		exit(EXIT_FAILURE);
	while (1) {
		pause();
	}

	exit(EXIT_SUCCESS);
}

