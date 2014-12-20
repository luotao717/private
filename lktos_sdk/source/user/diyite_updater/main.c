#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "upload.h"
#include "http.h"


//当前固件版本
#define SYSTEM_FIIRMWARE_VERSION	101
//查询新版本接口
#define GET_FIRMWARE_VERSION_URL "http://admin.firstte.com:8088/cgi-bin/autoupgrade.cgi"
//查询间隔 默认30分钟
#define TIME_SLEEP_INTERVAL		60*30
//firmware img本地保存路径
#define SD_PATH "/media/sda1"
#define LOCAL_FIRMWARE_FILE_PATH SD_PATH"/update.img"
#define LOG_FILE SD_PATH"/update.log"

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

int check(char *imagefile, int offset, int len, char *err_msg)
{
	struct stat sbuf;

	int  data_len;
	char *data;
	unsigned char *ptr;
	unsigned long checksum;

	image_header_t header;
	image_header_t *hdr = &header;

	int ifd;

	if ((unsigned)len < sizeof(image_header_t)) {
		sprintf (err_msg, "Bad size: \"%s\" is no valid image\n", imagefile);
		return 0;
	}

	ifd = open(imagefile, O_RDONLY);
	if(!ifd){
		sprintf (err_msg, "Can't open %s: %s\n", imagefile, strerror(errno));
		return 0;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		sprintf (err_msg, "Can't stat %s: %s\n", imagefile, strerror(errno));
		return 0;
	}

	ptr = (unsigned char *) mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if ((caddr_t)ptr == (caddr_t)-1) {
		close(ifd);
		sprintf (err_msg, "Can't mmap %s: %s\n", imagefile, strerror(errno));
		return 0;
    }
	ptr += offset;

	/*
	 *  handle Header CRC32
	 */
    memcpy (hdr, ptr, sizeof(image_header_t));

    if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "Bad Magic Number: \"%s\" is no valid image\n", imagefile);
		return 0;
	}

	data = (char *)hdr;

    checksum = ntohl(hdr->ih_hcrc);
    hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

    if (crc32 (0, data, sizeof(image_header_t)) != checksum) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "*** Warning: \"%s\" has bad header checksum!\n", imagefile);
		return 0;
    }

	/*
	 *  handle Data CRC32
	 */
    data = (char *)(ptr + sizeof(image_header_t));
    data_len  = len - sizeof(image_header_t) ;

    if (crc32 (0, data, data_len) != ntohl(hdr->ih_dcrc)) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "*** Warning: \"%s\" has corrupted data!\n", imagefile);
		return 0;
    }

	/*
	 * compare MTD partition size and image size
	 */
#if defined (CONFIG_RT2880_ROOTFS_IN_RAM)
	if(len > getMTDPartSize("\"Kernel\"")){
		munmap(ptr, len);
		close(ifd);
		sprintf(err_msg, "*** Warning: the image file(0x%x) is bigger than Kernel MTD partition.\n", len);
		return 0;
	}
#elif defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
	#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
        if(len > getMTDPartSize("\"Kernel_RootFS\"")){
                munmap(ptr, len);
                close(ifd);
                sprintf(err_msg, "*** Warning: the image file(0x%x) is bigger than Kernel_RootFS MTD partition.\n", len);
                return 0;
        }
  	#else

	if(len < CONFIG_MTD_KERNEL_PART_SIZ){
		munmap(ptr, len);
		close(ifd);
		sprintf(err_msg, "*** Warning: the image file(0x%x) size doesn't make sense.\n", len);
		return 0;
    }

	if((len - CONFIG_MTD_KERNEL_PART_SIZ) > getMTDPartSize("\"RootFS\"")){
		munmap(ptr, len);
		close(ifd);
		sprintf(err_msg, "*** Warning: the image file(0x%x) is bigger than RootFS MTD partition.\n", len - CONFIG_MTD_KERNEL_PART_SIZ);
		return 0;
	}
	#endif
#else
#error "goahead: no CONFIG_RT2880_ROOTFS defined!"
#endif
	munmap(ptr, len);
	close(ifd);

	return 1;
}

int update_firmware(char *filename)
{
	struct stat stat_buf;
	char err_msg[256];

	if(stat(filename, &stat_buf) == -1){
		perror("stat");
		goto err;
	}
	
	// examination
	if(!check(filename, 0, stat_buf.st_size, err_msg) ){
		//printf("Not a valid firmware: %s", err_msg);
		goto err;
	}

    /*
     * write the new image version into flash.
     */
    write_flash_kernel_version(filename, 0);

	// flash write
	if( mtd_write_firmware(filename, 0, stat_buf.st_size) == -1){
		//printf("mtd_write_firmware fatal error! The corrupted image has ruined the flash!!");
		goto err;
	}

	//printf("Done...rebooting");
	
#if defined (UPLOAD_BOOTLOADER_SUPPORT) || defined (CONFIG_RT2880_DRAM_8M)
    system("sleep 3 && reboot &");
#endif

	return 0;
	
err:
	return 1;
}


int getUpgradeVersion(char* firmwareFileUrl)
{
	int i;
	int serverVersion = 0;
	char buf[512] = {0};
	char url[512] = {0};
	sprintf(url, "%s?MAC=%s&BoxVersion=%d", GET_FIRMWARE_VERSION_URL, get_wifi_mac(), SYSTEM_FIIRMWARE_VERSION);
	int ret = http_get(url, buf, 512, 0);
	if (ret > 0 && ret < 512)
	{
		char *pData = buf;
		i = ret - 1;
		while(i >= 0)
		{
			if(pData[i] == 'g' && pData[i-1] == 'm' && pData[i-2] == 'i' && pData[i -3] == '.')
			{
				pData[i+1] = '\0';
				break;
			}
			else {
				i--;
			}
		}

		sscanf(pData,"%d %s", &serverVersion, firmwareFileUrl);
	}

	return serverVersion;
}

const char * getTimeStr()
{
	static char strtime[30];
	char * tok = NULL;
	time_t now = time(NULL);
	sprintf(strtime, ctime(&now));
	tok = strtok(strtime, "\n");

	return tok;
}

void check_logfile()
{
	struct stat file_stat = {0};
	if (!stat(LOG_FILE, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		if (file_stat.st_size > 1*1024*1024)
		{
			// larger than 1M, delete
			unlink(LOG_FILE);
		}
	}
}

int main()
{
	int serverVersion;
	char serverUpgradeFileUrl[512];
	FILE * log_fp = fopen(LOG_FILE, "a+");
	if (!log_fp)
		log_fp = stdout;

	sleep(60*2);
	fprintf(log_fp, "%s Local Mac : %s\n", getTimeStr(), get_wifi_mac());
	fflush(log_fp);

    	while(1)
    	{
		fprintf(log_fp, "%s current firmware version : %d\n", getTimeStr(), SYSTEM_FIIRMWARE_VERSION);
		fflush(log_fp);

		memset(serverUpgradeFileUrl, 0, 512);
		serverVersion = getUpgradeVersion(serverUpgradeFileUrl);
		fprintf(log_fp, "%s server version : %d, file url : %s\n", getTimeStr(), serverVersion, serverUpgradeFileUrl);
		fflush(log_fp);
		if(serverVersion > SYSTEM_FIIRMWARE_VERSION)
		{
			if(strlen(serverUpgradeFileUrl) > 0)
			{
				fprintf(log_fp, "%s downloading ...\n", getTimeStr());
				fflush(log_fp);
				if(http_download(serverUpgradeFileUrl, LOCAL_FIRMWARE_FILE_PATH) >= 0)
				{
					fprintf(log_fp, "%s download firmware success\n", getTimeStr());
					if (update_firmware(LOCAL_FIRMWARE_FILE_PATH) == 0)
					{
						fprintf(log_fp, "%s update firmware success\n", getTimeStr());
						fprintf(log_fp, "%s reboot\n", getTimeStr());
						fflush(log_fp);
						unlink(LOCAL_FIRMWARE_FILE_PATH);
						sleep(1);
		
						system("reboot &");
					}
					else
					{
						fprintf(log_fp, "%s update firmware failed\n", getTimeStr());
						unlink(LOCAL_FIRMWARE_FILE_PATH);
					}
				}
				else 
				{
					fprintf(log_fp, "%s Download Failed.", getTimeStr());
					unlink(LOCAL_FIRMWARE_FILE_PATH);
				}
			}
			else 
			{
				fprintf(log_fp, "%s redir null url.\n", getTimeStr());
			}		
		}
		else
		{
			fprintf(log_fp, "%s Current veriosn is the newest.\n", getTimeStr());
		}	

		fflush(log_fp);
		sleep(TIME_SLEEP_INTERVAL);
    	}

	fclose(log_fp);
    	return 0;
}
