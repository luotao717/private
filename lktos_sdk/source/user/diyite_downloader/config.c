/*
 * =====================================================================================
 *
 *       Filename:  config.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/25/2014 12:18:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yang Jun, 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <config.h>

struct global_configure g_config = {
	.core_apk_list_url = CORE_APK_LIST_URL, 
	.core_apk_local_path = CORE_APK_LOCAL_PATH, 
	.core_apk_update_interval = CORE_APK_UPDATE_INTERVAL, 
};
