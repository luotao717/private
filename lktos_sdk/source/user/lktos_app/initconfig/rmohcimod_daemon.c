#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int totalCount=0;
	struct stat stat_buf;
	
	while(1)
	{	
		sleep(5);
		if ( stat("/var/usbmodem/simcimi", &stat_buf) < 0 )
		{
			totalCount++;
		}
		else
		{
			totalCount=0;
		}
		if(totalCount>4)
		{
			system("rmmod ohci-hcd.ko");
			system("echo rmmod the ohci mode > /var/rmohcimode");
			sleep(3);
			system("insmod ohci-hcd.ko");
			totalCount=0;
		}
		
	}
}
