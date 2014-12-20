/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/25/2014 12:20:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yang Jun, 
 *   Organization:  
 *
 * =====================================================================================
 */

#define CORE_APK_LIST_URL "http://admin.firstte.com:8088/cgi-bin/getapp.cgi"
#define CORE_APK_LOCAL_PATH "/media/sda1/apk/firstteassitant.apk"
#define CORE_APK_UPDATE_INTERVAL (30 * 60)

struct global_configure {
	const char *core_apk_list_url;
	const char *core_apk_local_path;
	const unsigned long core_apk_update_interval;
};

extern struct global_configure g_config;
