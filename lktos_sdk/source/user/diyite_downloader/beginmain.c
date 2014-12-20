/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年07月30日 08时41分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <utime.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <http.h>
#include <core_apk_update.h>
#ifndef DEBUG
#include "nvram.h"
#endif

#define TIME_DOWNLOAD_BEGIN   0
#define TIME_DOWNLOAD_END	23
#define TIME_CHEAK_LIST_INTERVAL	60*30
#define TIME_UPLOAD_INTERVAL	60*60*2

#define SERVER_HOST "admin.firstte.com"
#define APK_LIST_BY_MAC_URL "http://admin.firstte.com/firste/boxDownload?boxMac="
#define LOG_RECORD_UPLOAD_URL "http://admin.firstte.com/boxserver/boxUpload"

#define SD_PATH "/media/sda1"
#define APK_PATH SD_PATH"/apk"
#define DOWN_LOAD_FILE SD_PATH"/apk_list.tmp"
#define TEMP_SOFT_LIST SD_PATH"/softlist.tmp"
#define SOFT_LIST SD_PATH"/localsoftlist"
#define OLD_SOFT_LIST SD_PATH"/softlist"
#define LOG_FILE SD_PATH"/log.txt"
#define CONFIG_CACHE SD_PATH"/diyite_download.cfg"
#define LOG_DOWNLOAD_FILE SD_PATH"/apk_download.log"
#define DOWNLOAD_STATUS_FILE SD_PATH"/downloadstatus.ini"

struct apk_info
{
	unsigned long seq;
	char *id;
	char *name;
	unsigned long size;
	char *icon_url;
	char *apk_url;
	char *version;
	long version_code;
	char *package_name;
	char *file_path; 
	char *icon_path;
	time_t update_time;
};

struct array
{
	void * base;
	size_t nmember;
	size_t size;
	size_t capacity;
};

static FILE * log_fp;
static struct array soft_list = { 
	.base = NULL, 
	.nmember = 0,  
	.size = sizeof(struct apk_info), 
	.capacity = 256, 
};

const char * getTimeStr()
{
	static char strtime[30];
	char * tok = NULL;
	time_t now = time(NULL);
	sprintf(strtime, ctime(&now));
	tok = strtok(strtime, "\n");

	return tok;
}

inline int apkcmp(const void * x, const void * y)
{
	return ((const struct apk_info *)x)->seq - ((const struct apk_info *)y)->seq;
}

inline size_t str_replace(char *s, int old, int new)
{
	size_t i = 0;
	size_t count = 0;
	for (i = 0; s[i]; ++i) {
		if (s[i] == old) {
			s[i] = new;
			++count;
		}
	}
	return count;
}

enum download_list_item {
	DOWNLOAD_LIST_SEQ, 
	DOWNLOAD_LIST_ID, 
	DOWNLOAD_LIST_NAME, 
	DOWNLOAD_LIST_SIZE,
	DOWNLOAD_LIST_ICON_URL,
	DOWNLOAD_LIST_APK_URL, 
	DOWNLOAD_LIST_VERSION, 
	DOWNLOAD_LIST_VERSION_CODE, 
	DOWNLOAD_LIST_PACKAGE_NAME, 
};

enum local_list_item {
	LOCAL_LIST_SEQ, 
	LOCAL_LIST_ID, 
	LOCAL_LIST_SIZE, 
	LOCAL_LIST_VERSION, 
	LOCAL_LIST_FILE, 
	LOCAL_LIST_NAME, 
	LOCAL_LIST_ICON, 
	LOCAL_LIST_VERSION_CODE, 
	LOCAL_LIST_PACKAGE_NAME, 
};

inline int check_download_apk_info(struct apk_info *info)
{
	return info->id && info->name && info->icon_url && info->apk_url && info->version; // && info->package_name;
}

inline int check_local_apk_info(struct apk_info *info)
{
	return info->id && info->version && info->file_path && info->name && info->icon_path; // && info->package_name;
}

inline void apk_info_free(struct apk_info *info)
{
	info->seq = 0;
	if (info->id) {
		free(info->id);
		info->id = NULL;
	}
	if (info->name) {
		free(info->name);
		info->name = NULL;
	}
	info->size = 0;
	if (info->icon_url) {
		free(info->icon_url);
		info->icon_url = NULL;
	}
	if (info->apk_url) {
		free(info->apk_url);
		info->apk_url = NULL;
	}
	if (info->version) {
		free(info->version);
		info->version = NULL;
	}
	info->version_code = -1;
	if (info->package_name) {
		free(info->package_name);
		info->package_name = NULL;
	}
	if (info->file_path) {
		free(info->file_path);
		info->file_path = NULL;
	}
	if (info->icon_path) {
		free(info->icon_path);
		info->icon_path = NULL;
	}
	info->update_time = 0;
}

inline int apk_info_copy(struct apk_info *dest, struct apk_info *src)
{
	apk_info_free(dest);
	errno = 0;
	dest->seq = src->seq;
	if (src->id) {
		dest->id = strdup(src->id);
		if (errno)
			goto error;
	}
	if (src->name) {
		dest->name = strdup(src->name);
		if (errno)
			goto error;
	}
	dest->size = src->size;
	if (src->icon_url) {
		dest->icon_url = strdup(src->icon_url);
		if (errno)
			goto error;
	}
	if (src->apk_url) {
		dest->apk_url = strdup(src->apk_url);
		if (errno)
			goto error;
	}
	if (src->version) {
		dest->version = strdup(src->version);
		if (errno)
			goto error;
	}
	dest->version_code = src->version_code;
	if (src->package_name) {
		dest->package_name = strdup(src->package_name);
		if (errno)
			goto error;
	}
	if (src->file_path) {
		dest->file_path = strdup(src->file_path);
		if (errno)
			goto error;
	}
	if (src->icon_path) {
		dest->icon_path = strdup(src->icon_path);
		if (errno)
			goto error;
	}
	dest->update_time = src->update_time;
	return 0;

error:
	return errno;
}

int parse_download_file(struct apk_info *info, char *line)
{
	enum download_list_item item = DOWNLOAD_LIST_SEQ;
	char *tok = NULL;
	char *saveptr = NULL;
	apk_info_free(info);
	errno = 0;
	for(tok = strtok_r(line, "\t\n", &saveptr), item = DOWNLOAD_LIST_SEQ; tok; tok = strtok_r(NULL, "\t\n", &saveptr), ++item) {
		switch(item) {
			case DOWNLOAD_LIST_SEQ:
				info->seq = strtoul(tok, NULL, 0);
				break;
			case DOWNLOAD_LIST_ID:
				info->id = strdup(tok);
				break;
			case DOWNLOAD_LIST_NAME:
				info->name = strdup(tok);
				break;
			case DOWNLOAD_LIST_SIZE:
				info->size = strtoul(tok, NULL, 0);
				break;
			case DOWNLOAD_LIST_ICON_URL:
				info->icon_url = strdup(tok);
				break;
			case DOWNLOAD_LIST_APK_URL:
				info->apk_url = strdup(tok);
				break;
			case DOWNLOAD_LIST_VERSION:
				info->version = strdup(tok);
				break;
			case DOWNLOAD_LIST_VERSION_CODE:
				info->version_code = strtol(tok, NULL, 0);
				break;
			case DOWNLOAD_LIST_PACKAGE_NAME:
				info->package_name = strdup(tok);
				break;
			default:
				if (strcmp(tok, "\n"))
					fprintf(log_fp, "%s %s:%d parse softlist error!\n", getTimeStr(), __FILE__, __LINE__);
				break;
		}
		if (errno)
			goto error;
	}

	// apk file path
	char *tmp = NULL;
	tmp = strdup(info->name);
	if (errno)
		goto error;
	str_replace(tmp, ' ', '_');
	info->file_path = malloc(strlen(APK_PATH) + strlen(info->id) + strlen(tmp) + 7);
	if (errno)
		goto error;
	sprintf(info->file_path, "%s/%s/%s.apk", APK_PATH, info->id, tmp);
	free(tmp);

	// icon file path
	tmp = strdup(info->icon_url);
	if (errno)
		goto error;
	char *last_slash = strrchr(tmp, '/');
	char *icon_name = NULL;
	if (!last_slash)
		icon_name = tmp;
	else 
		icon_name = last_slash + 1;
	info->icon_path = malloc(strlen(APK_PATH) + strlen(info->id) + strlen(icon_name) + 3);
	if (errno)
		goto error;
	sprintf(info->icon_path, "%s/%s/%s", APK_PATH, info->id, icon_name);
	free(tmp);

	info->update_time = time(NULL);

	return 0;

error:
	if (tmp)
		free(tmp);
	
	if (errno)
		fprintf(log_fp, "%s:%d parse download error:%d\n", __FILE__, __LINE__, errno);

	apk_info_free(info);
	return errno;
}

int parse_local_file(struct apk_info *info, char *line)
{
	enum local_list_item item = LOCAL_LIST_SEQ;
	char * tok = NULL;
	char *saveptr = NULL;
	apk_info_free(info);
	errno = 0;
	for(tok = strtok_r(line, "\t\n", &saveptr), item = LOCAL_LIST_SEQ; tok; tok = strtok_r(NULL, "\t\n", &saveptr), ++item) {
		switch(item) {
			case LOCAL_LIST_SEQ:
				info->seq = strtoul(tok, NULL, 0);
				break;
			case LOCAL_LIST_ID:
				info->id = strdup(tok);
				break;
			case LOCAL_LIST_SIZE:
				info->size = strtoul(tok, NULL, 0);
				break;
			case LOCAL_LIST_VERSION:
				info->version = strdup(tok);
				break;
			case LOCAL_LIST_FILE:
				info->file_path = strdup(tok);
				break;
			case LOCAL_LIST_NAME:
				info->name = strdup(tok);
				break;
			case LOCAL_LIST_ICON:
				info->icon_path = strdup(tok);
				break;
			case LOCAL_LIST_VERSION_CODE:
				info->version_code = strtol(tok, NULL, 0);
				break;
			case LOCAL_LIST_PACKAGE_NAME:
				info->package_name = strdup(tok);
				break;
			default:
				if (strcmp(tok, "\n"))
					fprintf(log_fp, "%s %s:%d parse softlist error!\n", getTimeStr(), __FILE__, __LINE__);
				break;
		}
		if (errno)
			goto error;
	}
	info->update_time = time(NULL);
	return 0;

error:
	apk_info_free(info);

	if (errno)
		fprintf(log_fp, "%s:%d parse local error:%d\n", __FILE__, __LINE__, errno);

	return errno;
}

int set_version_code(char *id, long version_code)
{
	char *version_file = NULL;
	FILE *version_fp = NULL;
	struct stat file_stat = {0};
	int ret = 0;

	version_file = malloc(strlen(APK_PATH) + strlen(id) * 2 + 11);
	if (!version_file) {
		ret = errno;
		goto end;
	}
	sprintf(version_file, "%s/%s/%s.version", APK_PATH, id, id);
	if (!stat(version_file, &file_stat) && !S_ISREG(file_stat.st_mode)) {
		ret = -1;
		goto end;
	} else {
		version_fp = fopen(version_file, "w");
		if (version_fp) {
			fprintf(version_fp, "%ld", version_code);
			fclose(version_fp);
			version_fp = NULL;
		} else {
			ret = errno;
			goto end;
		}
	}

end:
	if (version_file)
		free(version_file);

	if (ret)
		fprintf(log_fp, "%s:%d set version code error:%d\n", __FILE__, __LINE__, ret);

	return ret;
}

int get_version_code(char *id, long *version_code)
{
	char *version_file = NULL;
	FILE *version_fp = NULL;
	struct stat file_stat = {0};
	int ret = 0;

	version_file = malloc(strlen(APK_PATH) + strlen(id) * 2 + 11);
	if (!version_file) {
		ret = errno;
		goto end;
	}
	sprintf(version_file, "%s/%s/%s.version", APK_PATH, id, id);
	if (!stat(version_file, &file_stat) && S_ISREG(file_stat.st_mode)) {
		version_fp = fopen(version_file, "r");
		if (version_fp) {
			fscanf(version_fp, "%ld", version_code);
			fclose(version_fp);
			version_fp = NULL;
		} else {
			ret = errno;
			goto end;
		}
	} else {
		ret = -1;
		goto end;
	}

end:
	if (version_file)
		free(version_file);

	if (ret)
		fprintf(log_fp, "%s:%d get version code error:%d\n", __FILE__, __LINE__, ret);

	return ret;
}

int delete_version_file(char *id)
{
	char *version_file = NULL;
	struct stat file_stat = {0};
	int ret = 0;

	version_file = malloc(strlen(APK_PATH) + strlen(id) * 2 + 11);
	if (!version_file) {
		ret = errno;
		goto end;
	}
	sprintf(version_file, "%s/%s/%s.version", APK_PATH, id, id);
	if (!stat(version_file, &file_stat) && S_ISREG(file_stat.st_mode))
		unlink(version_file);

end:		
	if (version_file)
		free(version_file);

	if (ret)
		fprintf(log_fp, "%s:%d delete version file error:%d\n", __FILE__, __LINE__, ret);

	return ret;
}

static int write_local_soft_file()
{
	struct apk_info * apk_list_ptr = (struct apk_info *)soft_list.base;
	size_t soft_list_index = 0;

	qsort(soft_list.base, soft_list.nmember, soft_list.size, apkcmp);
	
	FILE * temp_softlist_fp = fopen(TEMP_SOFT_LIST, "w");
	for (soft_list_index = 0; soft_list_index < soft_list.nmember; ++soft_list_index) {
		fprintf(temp_softlist_fp, "%lu\t%s\t%lu\t%s\t%s\t%s\t%s"
				, apk_list_ptr[soft_list_index].seq
				, apk_list_ptr[soft_list_index].id
				, apk_list_ptr[soft_list_index].size
				, apk_list_ptr[soft_list_index].version
				, apk_list_ptr[soft_list_index].file_path 
				, apk_list_ptr[soft_list_index].name 
				, apk_list_ptr[soft_list_index].icon_path);
		if (apk_list_ptr[soft_list_index].version_code != -1 
				&& apk_list_ptr[soft_list_index].package_name) {
			fprintf(temp_softlist_fp, "\t%d\t%s\n"
					, apk_list_ptr[soft_list_index].version_code
					, apk_list_ptr[soft_list_index].package_name);
		} else
			fprintf(temp_softlist_fp, "\n");
	}
	fclose(temp_softlist_fp);
	rename(TEMP_SOFT_LIST, SOFT_LIST".lst");
}

int init_soft_list()
{
	if (!soft_list.capacity)
		soft_list.capacity = 256;

	if (!soft_list.size)
		soft_list.size = sizeof(struct apk_info);

	if (!soft_list.base)
		soft_list.base = calloc(soft_list.size, soft_list.capacity);
	
	FILE * soft_list_fp = NULL;

	const char *soft_list_file = SOFT_LIST".lst";
	if (access(soft_list_file, F_OK))
		soft_list_file = SOFT_LIST;

	if (soft_list_fp = fopen(soft_list_file, "r")) {
		char * line_buff = NULL;
		size_t line_size = 0;
		while(getline(&line_buff, &line_size, soft_list_fp) != -1) {
			memset(&((struct apk_info *)soft_list.base)[soft_list.nmember], 0, sizeof(struct apk_info));
			if (parse_local_file(&((struct apk_info *)soft_list.base)[soft_list.nmember], line_buff))
				continue;

			if (!check_local_apk_info(&((struct apk_info *)soft_list.base)[soft_list.nmember]))
				continue;

			if (((struct apk_info *)soft_list.base)[soft_list.nmember].version_code == -1) { 
				if (get_version_code(((struct apk_info *)soft_list.base)[soft_list.nmember].id, 
							&((struct apk_info *)soft_list.base)[soft_list.nmember].version_code))
					((struct apk_info *)soft_list.base)[soft_list.nmember].version_code = 0;
				else
					delete_version_file(((struct apk_info *)soft_list.base)[soft_list.nmember].id);
			} else {
				delete_version_file(((struct apk_info *)soft_list.base)[soft_list.nmember].id);
			}

			++soft_list.nmember;
			if (soft_list.nmember == soft_list.capacity) {
				soft_list.capacity *= 2;
				soft_list.base = realloc(soft_list.base, soft_list.size * soft_list.capacity);
			}
		}
		fclose(soft_list_fp);
		if (line_buff)
			free(line_buff);

		if (!access(SOFT_LIST, F_OK)) {
			write_local_soft_file();
			unlink(SOFT_LIST);
		}
	}
}

int remove_overdue_info(time_t timestamp)
{
	size_t soft_list_index = 0;
	struct apk_info * apk_list_ptr = (struct apk_info *)soft_list.base;
	for (soft_list_index = 0; soft_list_index < soft_list.nmember; ++soft_list_index)
	{
		if (apk_list_ptr[soft_list_index].update_time < timestamp)
		{
			fprintf(log_fp, "%s %s:%d delete apkinfo id:%s\n", getTimeStr(), __FILE__, __LINE__, apk_list_ptr[soft_list_index].id);
			apk_info_free(apk_list_ptr + soft_list_index);
			if (soft_list_index != soft_list.nmember - 1)
			{
				memcpy(&apk_list_ptr[soft_list_index], 
						&apk_list_ptr[soft_list.nmember - 1], soft_list.size);
			}
			--soft_list.nmember;
			--soft_list_index;
		}
	}	

	return write_local_soft_file();
}

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
#ifdef DEBUG
	sprintf(if_mac,"22:00:04:AA:BC:38");
#else
	if (getIfMac("ra0", if_mac) == -1)
	{
		sprintf(if_mac,"00:00:00:00:00:00");
	}
#endif
	
	return if_mac;
}

/*
 * description: write WAN MAC address accordingly
 */
/*
const char* get_wan_mac()
{
	//WAN_MAC_ADDR   22:0:4:AA:BC:35 RT2860_NVRAM
	static char mac[18] = {0};
	char *wanMac = NULL;
#ifdef DEBUG
	wanMac = (char *)"22:0:4:AA:BC:35";
#else
	wanMac = nvram_get(RT2860_NVRAM, "WAN_MAC_ADDR");
#endif
	if (wanMac)
	{
		int m1 = 0, m2 = 0, m3 = 0, m4 = 0, m5 = 0, m6 = 0;
		sscanf(wanMac, "%X:%X:%X:%X:%X:%X", &m1, &m2, &m3, &m4, &m5, &m6);
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", m1, m2, m3, m4, m5, m6);
	}
	else
	{
		sprintf(mac,"00:00:00:00:00:00");
	}
	
	return mac;
}
*/

int update_stamp(const char * file)
{
	struct utimbuf buff = {
		.actime = time(NULL), 
		.modtime = time(NULL), 
	};

	char * current_dir = get_current_dir_name();
	char * file_path = strdup(file);

	if (strlen(file_path) && *file_path == '/')
			utime("/", &buff);

	while (strlen(file_path))
	{
		while (file_path[strlen(file_path) - 1] == '/')
			file_path[strlen(file_path - 1)] = '\0';

		utime(file_path, &buff);
		char * file_name = strrchr(file_path, '/');
		if (!file_name)
		{
			break;
		}
		*file_name = '\0';
	}
	chdir(current_dir);
	free(current_dir);
}

/*
int wget(const char * url, const char * file, int continu_flag)
{
	size_t buff_size;
	char * buff;
	if (continu_flag)
	{
		buff_size = strlen("wget -c -q  -O ") + strlen(url) + strlen(file) + 1;
		buff = malloc(buff_size);
		sprintf(buff, "wget -c -q %s -O %s", url, file);
	}
	else
	{
		buff_size = strlen("wget -q  -O ") + strlen(url) + strlen(file) + 1;
		buff = malloc(buff_size);
		sprintf(buff, "wget -q %s -O %s", url, file);
	}

	fprintf(log_fp, "%s\n", buff);

	int ret = system(buff);
	free(buff);
	return WEXITSTATUS(ret);
}
*/

int download_file(const char * url, const char * file)
{
	char * temp_file = malloc(strlen(file) + 5);
	sprintf(temp_file, "%s.tmp", file);
	int ret = http_download(url, temp_file);
	if (ret >= 0)
	{
		rename(temp_file, file);
		ret = 0;
	}
	else
	{
		fprintf(log_fp, "%s %s:%d download_file failed, ret = %d -- unlink %s\n", getTimeStr(), __FILE__, __LINE__, ret, temp_file);
		unlink(temp_file);
		ret = -1;
	}
/*
	int ret = wget(url, temp_file, 1);
	//fprintf(log_fp, "%s:%d download_file -- ret = %d\n", __FILE__, __LINE__, ret);
	switch(ret)
	{
		case 0:
		case 8:
			rename(temp_file, file);
			ret = 0;
			break;
		default:
			fprintf(log_fp, "%s:%d download_file failed, ret = %d -- unlink %s\n", __FILE__, __LINE__, ret, temp_file);
			unlink(temp_file);
			ret = -1;
			break;
	}
*/
	free(temp_file);
	return ret;
}

int check_apk_size(const char* file, unsigned long verify_size)
{
	int ret = -1;
	struct stat file_stat = {0};
	if (!stat(file, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		if (file_stat.st_size == verify_size)
		{
			ret = 0;
		}
		else 
		{
			fprintf(log_fp, "%s %s:%d download size = %d, server size = %d\n", getTimeStr(), __FILE__, __LINE__, file_stat.st_size, verify_size);
			fprintf(log_fp, "%s %s:%d verify apk file failed, ----- %s\n", getTimeStr(), __FILE__, __LINE__, file);
			fflush(log_fp);
		}
	}

	return ret;
}

inline int mkdir_p(const char * dir_path)
{
	char * current_dir = get_current_dir_name();
	if (*dir_path == '/')
		chdir("/");

	char * tok = NULL, * save_ptr = NULL;
	struct stat file_stat = {0};
	char * dir = strdup(dir_path);
	for(tok = strtok_r(dir, "/", &save_ptr); tok; tok = strtok_r(NULL, "/", &save_ptr))
	{
		if (!strlen(tok))
			continue;

		if (!stat(tok, &file_stat))
		{
			if (S_ISDIR(file_stat.st_mode))
				chdir(tok);
			else
			{
				fprintf(log_fp, "%s %s:%d mkdir_p %s error!\n", getTimeStr(), __FILE__, __LINE__, dir);
				break;
			}
		}
		else
		{
			mkdir(tok, S_IRWXU | S_IRWXG | S_IXOTH);
			chdir(tok);
		}
	}
	chdir(current_dir);
	free(current_dir);
	free(dir);
}

inline int remove_dir(const char * dir)
{
	char * current_dir = get_current_dir_name();
	DIR * dir_ptr = opendir(dir);
	if (!dir_ptr)
		return -1;

	chdir(dir);
	struct stat path_stat = {0};
	struct dirent dir_entry, * dir_entry_ptr = &dir_entry;
	while(!readdir_r(dir_ptr, &dir_entry, &dir_entry_ptr) && dir_entry_ptr)
	{
		if (!strcmp(dir_entry.d_name, "..") || !strcmp(dir_entry.d_name, "."))
		{
			continue;
		}
		else if (!stat(dir_entry.d_name, &path_stat) && S_ISDIR(path_stat.st_mode))
		{
			remove_dir(dir_entry.d_name);
		}
		else
		{
			fprintf(log_fp, "%s %s:%d unlink %s\n", getTimeStr(), __FILE__, __LINE__, dir_entry.d_name);
			if(unlink(dir_entry.d_name))
				fprintf(log_fp, "%s %s:%d unlink %s error, errno:%d\n", getTimeStr(), __FILE__, __LINE__, dir_entry.d_name, errno);
		}
	}
	chdir(current_dir);
	free(current_dir);
	closedir(dir_ptr);
	rmdir(dir);
}

inline int update_apk_timestamp(const char * apk_id)
{
	size_t soft_list_index = 0;
	for (soft_list_index = 0; soft_list_index < soft_list.nmember; ++soft_list_index)
	{
		if (!strcmp(((struct apk_info *)soft_list.base)[soft_list_index].id, apk_id))
		{
			((struct apk_info *)soft_list.base)[soft_list_index].update_time = time(NULL);
			break;
		}
	}
}

struct apk_info *find_apk_info(const char *apk_id)
{
	struct apk_info * apk_list_ptr = (struct apk_info *)soft_list.base;
	size_t soft_list_index = 0;
	for (soft_list_index = 0; soft_list_index < soft_list.nmember; ++soft_list_index) {
		if (!strcmp(apk_list_ptr[soft_list_index].id, apk_id))
			return &apk_list_ptr[soft_list_index];
	}
	return NULL;
}

inline int record_local_apk(struct apk_info *find, struct apk_info *info)
{
	if (!find) {
		if (soft_list.nmember + 1 == soft_list.capacity) {
			soft_list.capacity *= 2;
			soft_list.base = realloc(soft_list.base, soft_list.size * soft_list.capacity);
		}
		find = (struct apk_info *)soft_list.base + soft_list.nmember;
		++soft_list.nmember;
	}
	apk_info_copy(find, info);
//	set_version_code(find->id, find->version_code);
	return write_local_soft_file();
}

int update_apk(char *line)
{
	int ret = 0;

	struct apk_info info = {0};
	char *apk_path = NULL;
	if (parse_download_file(&info, line))
		goto end;

	if (!check_download_apk_info(&info))
		goto end;

	apk_path = malloc(strlen(APK_PATH) + strlen(info.id) + 2);
	sprintf(apk_path, "%s/%s", APK_PATH, info.id);
	struct stat file_stat = {0};

	if (!stat(apk_path, &file_stat)) {
		if (S_ISDIR(file_stat.st_mode)) {
			update_stamp(apk_path);
//			update_apk_timestamp(info.id);
		} else {
			unlink(apk_path);
			mkdir_p(apk_path);
		}
	} else
		mkdir_p(apk_path);

	struct apk_info *find = NULL;
	find = find_apk_info(info.id);
	
	if (find && find->version_code >= info.version_code) {
		if (strcmp(find->file_path, info.file_path))
			rename(find->file_path, info.file_path);

		if (strcmp(find->icon_path, info.icon_path)) {
			if (!download_file(info.icon_url, info.icon_path))
				unlink(find->icon_path);
			else
				rename(find->icon_path, info.icon_path);
		}
	} else {
		if (find) {
			unlink(find->file_path);
			unlink(find->icon_path);
		}
		if (!download_file(info.apk_url, info.file_path) 
				&& !check_apk_size(info.file_path, info.size) 
				&& !download_file(info.icon_url, info.icon_path)) {
			fprintf(log_fp, "%s %s:%d update apk, id : %s, name : %s\n", getTimeStr(), __FILE__, __LINE__, info.id, info.name);

		} else {
			ret = -1;
			record_download_status(1);
			unlink(info.file_path);
			unlink(info.icon_path);
			goto end;
		}
	}
	record_local_apk(find, &info);

end:
	apk_info_free(&info);

	if (apk_path)
		free(apk_path);

	fflush(log_fp);
	return ret;
}

int mark_apk(char *line)
{
	struct apk_info info = {0};
	char *apk_path = NULL;

	if (parse_download_file(&info, line))
		goto end;

	if (!check_download_apk_info(&info))
		goto end;

	apk_path = malloc(strlen(APK_PATH) + strlen(info.id) + 2);
	sprintf(apk_path, "%s/%s", APK_PATH, info.id);
	struct stat file_stat = {0};

	if (!stat(apk_path, &file_stat)) {
		if (S_ISDIR(file_stat.st_mode)) {
			update_stamp(apk_path);
			update_apk_timestamp(info.id);
		} else {
			unlink(apk_path);
			mkdir_p(apk_path);
		}
	}

end:
	apk_info_free(&info);

	if (apk_path)
		free(apk_path);
}

int remove_overdue(const char * file, time_t timestamp)
{
	struct stat file_stat = {0};
	if (!stat(file, &file_stat))
	{
		if (file_stat.st_mtime < timestamp)
		{
			if (S_ISDIR(file_stat.st_mode))
			{
				fprintf(log_fp, "%s %s:%d remove dir %s\n", getTimeStr(), __FILE__, __LINE__, file);
				remove_dir(file);
			}
		}
		else if (S_ISDIR(file_stat.st_mode))
		{
			DIR * dir_ptr = opendir(file);
			if (!dir_ptr)
				return -1;

			char * current_dir = get_current_dir_name();
			//fprintf(log_fp, "%s %s:%d enter dir %s\n", getTimeStr(), __FILE__, __LINE__, file);
			chdir(file);
			struct stat subdir_stat = {0};
			struct dirent dir_entry, * dir_entry_ptr = &dir_entry;
			while(!readdir_r(dir_ptr, &dir_entry, &dir_entry_ptr) && dir_entry_ptr)
			{
				if (strcmp(dir_entry.d_name, "..") && strcmp(dir_entry.d_name, ".") && !stat(dir_entry.d_name, &subdir_stat) && S_ISDIR(subdir_stat.st_mode))
				{
					remove_overdue(dir_entry.d_name, timestamp);
				}
			}
			//fprintf(log_fp, "%s %s:%d leave dir %s\n", getTimeStr(), __FILE__, __LINE__, file);
			chdir(current_dir);
			free(current_dir);
			closedir(dir_ptr);
		}
	}
}

int get_apk_list_url(char* url_buf, int buf_size)
{
	const char * mac = get_wifi_mac();
	fprintf(log_fp, "%s %s:%d call get_apk_list_url mac = %s\n", getTimeStr(), __FILE__, __LINE__, mac);
	fflush(log_fp);

	char * url = malloc(strlen(APK_LIST_BY_MAC_URL) + strlen(mac) + 1);
	sprintf(url, "%s%s", APK_LIST_BY_MAC_URL, mac);

	return http_get(url, url_buf, buf_size, 0);
}

int check_apk_list()
{
	FILE * apk_list_fp = fopen(DOWN_LOAD_FILE, "r");
	if (apk_list_fp)
	{
		time_t timestamp = time(NULL);
		sleep(1);

		char * line_buff = NULL;
		size_t line_size = 0;
		while(getline(&line_buff, &line_size, apk_list_fp) != -1)
		{
			mark_apk(line_buff);
		}

		fclose(apk_list_fp);
		if (line_buff)
			free(line_buff);
			
		remove_overdue(APK_PATH, timestamp);
		remove_overdue_info(timestamp);
	}
}

int record_download_status(int flag)
{
	time_t now = time(NULL) + 8 * 60 * 60;
	struct tm *tm = gmtime(&now);
	FILE *fp = fopen(DOWNLOAD_STATUS_FILE, "w");
	fprintf(fp, "%d %04d-%02d-%02d %02d:%02d:%0d", flag, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	fclose(fp);
}

int update()
{
	char url_buf[512] = {0};
	if (get_apk_list_url(url_buf, 512) <= 0)
	{
		fprintf(log_fp, "%s get redir url failed.\n", getTimeStr());
		return -1;
	}
	
	fprintf(log_fp, "%s redir url : %s\n", getTimeStr(), url_buf);
	fflush(log_fp);

	if (http_download(url_buf, DOWN_LOAD_FILE) >= 0)
	{
		//printf("update start\n");
		FILE * apk_list_fp = fopen(DOWN_LOAD_FILE, "r");
		if (apk_list_fp)
		{
			time_t begin = time(NULL);
			FILE * fp = fopen(CONFIG_CACHE, "w+");
			fprintf(fp, "%ld", begin);
			fclose(fp);
			fp = NULL;

			// empty TEMP_SOFT_LIST
//			FILE * temp_local_apk_list_fp = fopen(TEMP_SOFT_LIST, "w+");
//			fclose(temp_local_apk_list_fp);

			// check which apk need to delete
			check_apk_list();

			// update apk
			begin = time(NULL);
			sleep(1);
			char * line_buff = NULL;
			size_t line_size = 0;
			int download_flag = 0;
			while(getline(&line_buff, &line_size, apk_list_fp) != -1)
			{
				//printf("%s\n", line_buff);
				if(update_apk(line_buff) != 0)
				{
					download_flag = 1;
				}
			}
			fclose(apk_list_fp);
			record_download_status(download_flag);
			if (line_buff)
				free(line_buff);
			
			remove_overdue(APK_PATH, begin);
			remove_overdue_info(begin);
			
			fflush(log_fp);

			time_t now = time(NULL);
			fp = fopen(CONFIG_CACHE, "w+");
			fprintf(fp, "%ld", now);
			fclose(fp);
			fp = NULL;
		}
		return 0;
	}
	else
	{
		fprintf(log_fp, "%s get apk list failed.\n", getTimeStr());
		return -1;
	}
}

int convert_old_apklist_path()
{
	struct stat file_stat = {0};
	if (!stat(OLD_SOFT_LIST, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		FILE * apk_list_fp = fopen(OLD_SOFT_LIST, "r");
		if (apk_list_fp)
		{
			char apk_id[10] = {0};
			int force_install = 0;
			int apk_type = 0;
			int apk_subtype = 0;
			int download_times = 0;
			char apk_version[32] = {0}; 
			char apk_filepath[128] = {0};
			char apk_name[32] = {0};
			char apk_iconpath[128] = {0};
			char apk_desc[1024] = {0};

			char new_apk_filepath[128] = {0};
			char new_apk_iconpath[128] = {0};

			struct apk_info info = {
				.id = apk_id, 
				.version = apk_version, 
				.file_path = new_apk_filepath, 
				.name = apk_name, 
				.icon_path = new_apk_iconpath, 
				.version_code = -1, 
			};
			struct apk_info *find = NULL;

			char * line_buff = NULL;
			size_t line_size = 0;
			while(getline(&line_buff, &line_size, apk_list_fp) != -1)
			{
				//fprintf(log_fp, "%s\n", line_buff);
				sscanf(line_buff,"%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\n",
					&info.seq, info.id, &force_install, &apk_type, &apk_subtype, &info.size, &download_times, info.version, apk_filepath, info.name, apk_iconpath, apk_desc);
				if (!stat(apk_filepath, &file_stat) && S_ISREG(file_stat.st_mode) && file_stat.st_size > 0 && !stat(apk_iconpath, &file_stat) && S_ISREG(file_stat.st_mode) && file_stat.st_size > 0)
				{
					char* apk_path = malloc(strlen(APK_PATH) + strlen(info.id) + 2);
					sprintf(apk_path, "%s/%s", APK_PATH, info.id);
					if (!stat(apk_path, &file_stat))
					{
						if (!S_ISDIR(file_stat.st_mode))
						{
							unlink(apk_path);
							mkdir_p(apk_path);
						}
					}
					else
					{
						mkdir_p(apk_path);
					}

					char * temp_name = strdup(apk_name);
					char *tok = strtok(temp_name, " ");
					sprintf(new_apk_filepath, "%s/%s.apk", apk_path, tok);
					rename(apk_filepath, new_apk_filepath);
					sprintf(new_apk_iconpath, "%s/%s.png", apk_path, info.id);
					rename(apk_iconpath, new_apk_iconpath);

					find = find_apk_info(info.id);
					record_local_apk(find, &info);
					fprintf(log_fp, "%s %s:%d convert apk %s\n", getTimeStr(), __FILE__, __LINE__, apk_name);
					free(temp_name);
					free(apk_path);
				}
			}

			fclose(apk_list_fp);
			unlink(OLD_SOFT_LIST);

			// delete unused file
			system("rm -f /media/sda1/*.apk");
			system("rm -f /media/sda1/*.png");
			fprintf(log_fp, "%s convert apk list done.\n", getTimeStr());
			fflush(log_fp);

			if (line_buff)
				free(line_buff);
		}
	}
}

int upload_record()
{
	const char * mac = get_wifi_mac();
	char record_filename[24] = {0};
	sprintf(record_filename, "%s.log", mac);
	//char record_file_path[128] = {0};
	//sprintf(record_file_path, "%s/%s.log", SD_PATH, mac);
	//printf("%s\n", record_file_path);
	
	struct stat file_stat = {0};
	if (!stat(LOG_DOWNLOAD_FILE, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		if (file_stat.st_size > 0)
		{
			if (!http_upload(LOG_RECORD_UPLOAD_URL, LOG_DOWNLOAD_FILE, record_filename))
			{
				fprintf(log_fp, "%s upload record success.\n", getTimeStr());
				unlink(LOG_DOWNLOAD_FILE);
			}
			else
			{
				fprintf(log_fp, "%s upload record failed.\n", getTimeStr());
			}
		}
	}

	fflush(log_fp);
	return 0;
}

int check_system_time()
{
	struct stat file_stat = {0};
	if (!stat(CONFIG_CACHE, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		FILE * fp = fopen(CONFIG_CACHE, "r");
		long last_update_time;
		fscanf(fp, "%ld", &last_update_time);
		fclose(fp);
		fp = NULL;

		time_t cur_time = time(NULL);
		if (last_update_time > cur_time)
		{
			fprintf(log_fp, "%s %s:%d settimeofday %ld\n", getTimeStr(), __FILE__, __LINE__, last_update_time);
			fflush(log_fp);

			struct timeval tv;
			tv.tv_sec = last_update_time;
			tv.tv_usec = 0;
			settimeofday(&tv, NULL);
			sleep(1);
		}
	}
}

int check_server_reachable()
{
	return check_host(SERVER_HOST, 80);
}

void check_logfile()
{
	struct stat file_stat = {0};
	if (!stat(LOG_FILE, &file_stat) && S_ISREG(file_stat.st_mode))
	{
		if (file_stat.st_size > 10*1024*1024)
		{
			// larger than 10M, delete
			unlink(LOG_FILE);
		}
	}
}

void setup_ntp()
{
#ifndef DEBUG
	nvram_bufset(RT2860_NVRAM, "NTPServerIP", "time.nist.gov");
	nvram_bufset(RT2860_NVRAM, "NTPSync", "1");
	nvram_bufset(RT2860_NVRAM, "TZ", "CST_008");
	nvram_commit(RT2860_NVRAM);

	system("ntp.sh");
#endif
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main(int argc, char * argv[])
{
	//wait sd card mount complete 
	sleep(30);

	setup_ntp();
	check_logfile();
	log_fp = fopen(LOG_FILE, "a+");
	if (!log_fp)
		log_fp = stdout;

	fprintf(log_fp, "%s start\n", getTimeStr());
	fflush(log_fp);

	// init
	core_apk_update_init();
	check_system_time();
	init_soft_list();
	convert_old_apklist_path();

	// check network status
	while (check_server_reachable())
	{
		sleep(5);
	}
	fprintf(log_fp, "%s server reachable ok.\n", getTimeStr());
	fflush(log_fp);

	upload_record();
	time_t last_upload_time = time(NULL);

	int must_download = 1;
	while(1)
	{
		if(time(NULL) - last_upload_time > TIME_UPLOAD_INTERVAL)
		{
			upload_record();
			last_upload_time = time(NULL);
		}

		time_t now = time(NULL);
		struct tm * time_message = gmtime(&now);
		int curHour = (time_message->tm_hour + 8) % 24;
		if(must_download || (curHour >= TIME_DOWNLOAD_BEGIN && curHour <= TIME_DOWNLOAD_END) )
		{
			must_download = 0;
			int ret = update();
			fprintf(log_fp, "%s %s:%d update ret = %d\n", getTimeStr(), __FILE__, __LINE__, ret);
			if(ret)
			{
				sleep(60 * 5);//get apk list failed
			}
			else
			{
				sleep(TIME_CHEAK_LIST_INTERVAL);
			}
		}
		else
		{
			sleep(60 * 60);
		}
		fflush(log_fp);
	}

	fprintf(log_fp, "%s end.\n", getTimeStr());
	if (log_fp && log_fp != stdout && log_fp != stderr)
		fclose(log_fp);
	core_apk_update_cleanup();
	return EXIT_SUCCESS;
}
/* ----------  end of function main  ---------- */
