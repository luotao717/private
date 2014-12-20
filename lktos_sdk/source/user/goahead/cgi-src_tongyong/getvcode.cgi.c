#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>

#include "nvram.h"

#define GET_VCODE_URL "http://admin.firstte.com/firste/getboxcode"
#define SD_PATH "/media/sda1"
#define VCODE_FILE SD_PATH"/vcode.txt"

int http_post(const char *server, char *buf, size_t count);


/*
 * arguments: ifname  - interface name
 *            if_addr - a 18-byte buffer to store mac address
 * description: fetch mac address according to given interface name
 */
int getIfMac(char *ifname, char *if_hw)
{
	struct ifreq ifr;
	char *ptr;
	int skfd;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		//error(E_L, E_LOG, T("getIfMac: open socket error"));
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		//error(E_L, E_LOG, T("getIfMac: ioctl SIOCGIFHWADDR error for %s"), ifname);
		return -1;
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

	close(skfd);
	return 0;
}

/*
 * description: write WAN MAC address accordingly
 */
const char* get_wifi_mac()
{
	static char if_mac[18];
	if (getIfMac("ra0", if_mac) == -1)
	{
		sprintf(if_mac,"00:00:00:00:00:00");
	}
	
	return if_mac;
}

int main (int argc, char *argv[])
{
	char buf[512] = {0};
	sprintf(buf, "mac=%s", get_wifi_mac());
	int ret = http_post(GET_VCODE_URL, buf, 512);
	if (ret > 0 && ret < 512)
	{
		buf[ret] = '\0';
		printf("Server: %s\n"
			"Pragma: no-cache\n"
			"Content-type: text/html\n"
			"Content-Length: %d\n\n", getenv("SERVER_SOFTWARE"), ret);
		printf("%s\n", buf);

		//local cache
		FILE * fp = fopen(VCODE_FILE, "w+");
		fprintf(fp, "%s", buf);
		fclose(fp);
		fp = NULL;

		exit(0);
	}
	else
	{
		exit(-1);
	}	
}
