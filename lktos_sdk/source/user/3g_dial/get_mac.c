#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
int get_mac_addr(unsigned char *macbuf)
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
int main(int argc, char *argv[])
{
	char flash_mac[18];
	memset(flash_mac,'\0',sizeof(flash_mac));
	get_mac_addr(flash_mac);
	printf("\nMAC:%s\n",flash_mac);

	return 0;
}