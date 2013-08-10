/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	driver_wext.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot.h"
#include "driver_wext.h"
#include "priv_netlink.h"
#include "wireless_copy.h"
#include "netlink.h"
#include <sys/ioctl.h>


static void event_test(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;

	hs->event_ops->event_test(hs);
}

static void event_anqp_rsp(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;	
	struct anqp_rsp_data *rsp_data = (struct anqp_rsp_data *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(rsp_data->ifindex, ifname);

	hs->event_ops->event_anqp_rsp(hs,
							 	  ifname,
								  rsp_data->peer_mac_addr,					  
							 	  rsp_data->status,
							 	  rsp_data->anqp_rsp,	
							 	  rsp_data->anqp_rsp_len);
}

static void event_anqp_req(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;	
	struct anqp_req_data *req_data = (struct anqp_req_data *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(req_data->ifindex, ifname);
	
	hs->event_ops->event_anqp_req(hs,
							 	  ifname,
								  req_data->peer_mac_addr,					  
							 	  req_data->anqp_req,	
							 	  req_data->anqp_req_len);
}

static void event_hs_onoff(struct driver_wext_data *drv_data, char *buf)
{

	struct hotspot *hs = (struct hotspot *)drv_data->priv;	
	struct hs_onoff *onoff = (struct hs_onoff *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(onoff->ifindex, ifname);

	hs->event_ops->event_hs_onoff(hs,
							 	  ifname,
							 	  onoff->hs_onoff);

}

static void event_btm_query(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;
	struct btm_query_data *query_data = (struct btm_query_data *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(query_data->ifindex, ifname);

	hs->event_ops->event_btm_query(hs,
								   ifname,
								   query_data->peer_mac_addr,
								   query_data->btm_query,
								   query_data->btm_query_len);
}

static void event_btm_rsp(struct driver_wext_data *drv_data, char *buf)
{

	struct hotspot *hs = (struct hotspot *)drv_data->priv;
	struct btm_rsp_data *rsp_data = (struct btm_rsp_data *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(rsp_data->ifindex, ifname);

	hs->event_ops->event_btm_rsp(hs,
								 ifname,
								 rsp_data->peer_mac_addr,
								 rsp_data->btm_rsp,
								 rsp_data->btm_rsp_len);
}

static void event_proxy_arp(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;
	struct proxy_arp_entry *arp_entry = (struct proxy_arp_entry *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(arp_entry->ifindex, ifname);

	if (arp_entry->ip_type == IPV4) {
		hs->event_ops->event_proxy_arp(hs,
									   arp_entry->ifindex,
									   arp_entry->ip_type,
									   arp_entry->from_ds,
									   arp_entry->source_mac_addr,
									   arp_entry->ip_addr,
									   arp_entry->target_mac_addr,
									   arp_entry->ip_addr + 4);
	} else {
		hs->event_ops->event_proxy_arp(hs,
									   arp_entry->ifindex,
									   arp_entry->ip_type,
									   arp_entry->from_ds,
									   arp_entry->source_mac_addr,
									   arp_entry->ip_addr,
									   arp_entry->target_mac_addr,
									   arp_entry->ip_addr + 16);
	}
}

static void event_hs_ap_reload(struct driver_wext_data *drv_data, char *buf)
{
	struct hotspot *hs = (struct hotspot *)drv_data->priv;
	struct hs_onoff *onoff= (struct hs_onoff *)buf;
	char ifname[IFNAMSIZ];

	if_indextoname(onoff->ifindex, ifname);

	hs->event_ops->event_ap_reload(hs, ifname);
}

static void driver_wext_event_wireless(struct driver_wext_data *drv,
                 void *ctx, char *data, int len)
{               
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *buf /*,*assoc_info_buf, *info_pos */;
    
    /* info_pos = NULL; */
	/* assoc_info_buf = NULL; */
    pos = data;
    end = data + len;   
    
    while (pos + IW_EV_LCP_LEN <= end) {
        /* 
 		 * Event data may be unaligned, so make a local, aligned copy
         * before processing. 
         */
        os_memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		
		DBGPRINT(RT_DEBUG_INFO, "cmd = 0x%x len = %d\n", iwe->cmd, iwe->len);
        
		if (iwe->len <= IW_EV_LCP_LEN)
            return;

        custom = pos + IW_EV_POINT_LEN;

        //if (drv->we_version_compiled > 18 && iwe->cmd == IWEVCUSTOM) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            os_memcpy(dpos, pos + IW_EV_LCP_LEN,
                  sizeof(struct iw_event) - dlen);
        //} else {
            //os_memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            //custom += IW_EV_POINT_OFF;
		//}
		
		switch (iwe->cmd) {
        case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
               	return;
           	buf = os_malloc(iwe->u.data.length + 1);
            if (buf == NULL)
                return;
            os_memcpy(buf, custom, iwe->u.data.length);
            buf[iwe->u.data.length] = '\0';

            switch (iwe->u.data.flags) {
			case OID_802_11_HS_TEST:
				event_test(drv, buf); 
				break;
			case OID_802_11_HS_ANQP_REQ:
				event_anqp_req(drv, buf);
				break;
			case OID_802_11_HS_ANQP_RSP:
				event_anqp_rsp(drv, buf);
				break;
			case OID_802_11_HS_ONOFF:
				event_hs_onoff(drv, buf);
				break;
			case OID_802_11_WNM_BTM_QUERY:
				event_btm_query(drv, buf);
				break;
			case OID_802_11_WNM_BTM_RSP:
				event_btm_rsp(drv, buf);
				break;
			case OID_802_11_WNM_PROXY_ARP:
				event_proxy_arp(drv, buf);
				break;
			case OID_802_11_HS_AP_RELOAD:
				event_hs_ap_reload(drv, buf);
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "%s: unkwnon event type(%d)\n", __FUNCTION__, iwe->u.data.flags);
				break; 
			}

           	os_free(buf);
            break;
        }

        pos += iwe->len;
    }
}
static void driver_wext_event_rtm_newlink(void *ctx, struct ifinfomsg *ifi,
                    					  u8 *buf, size_t len)
{       
    struct driver_wext_data *drv = ctx;
    int attrlen, rta_len;
    struct rtattr *attr;
    
    attrlen = len;

    DBGPRINT(RT_DEBUG_TRACE, "attrlen=%d", attrlen);

    attr = (struct rtattr *) buf;
    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen)) {
        DBGPRINT(RT_DEBUG_TRACE, "rta_type=%02x\n", attr->rta_type);
        if (attr->rta_type == IFLA_WIRELESS) {
            driver_wext_event_wireless(
                drv, ctx,
                ((char *) attr) + rta_len,
                attr->rta_len - rta_len);
        }
        attr = RTA_NEXT(attr, attrlen);
    }
}

static int driver_wext_get_we_version_compiled(struct driver_wext_data *drv_wext_data,
												const char *ifname)
{
 	struct iwreq iwr;
    int we_version_compiled = 0;
    
    os_memset(&iwr, 0, sizeof(iwr));
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t) &we_version_compiled;
    iwr.u.data.flags = RT_OID_WE_VERSION_COMPILED;

    if (ioctl(drv_wext_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: failed", __FUNCTION__);
        return -1;
    }

	return we_version_compiled; 
}

static void *driver_wext_init(struct hotspot *hs, 
						      const int opmode,
							  const int drv_mode)
{
	struct driver_wext_data *drv_wext_data;
	struct netlink_config *cfg;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	drv_wext_data = calloc(1, sizeof(*drv_wext_data));
	
	if (!drv_wext_data) {
		DBGPRINT(RT_DEBUG_ERROR, "No avaliable memory for driver_wext_data\n");
		goto err1;

	}

	DBGPRINT(RT_DEBUG_OFF, "Initialize ralink wext interface\n");

	drv_wext_data->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (drv_wext_data->ioctl_sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "socket(PF_INET,SOCK_DGRAM)");
		goto err2;
	}

	cfg = os_zalloc(sizeof(*cfg));

    if (!cfg) {
		DBGPRINT(RT_DEBUG_ERROR, "No avaliable memory for netlink cfg\n");
        goto err3;
    }

	cfg->ctx = drv_wext_data;
	cfg->newlink_cb = driver_wext_event_rtm_newlink;

	drv_wext_data->netlink = netlink_init(cfg);

	if (!drv_wext_data->netlink) {
		DBGPRINT(RT_DEBUG_ERROR, "wext netlink init fail\n");
		goto err3;
	}

	drv_wext_data->opmode = opmode;

	drv_wext_data->drv_mode = drv_mode;

	drv_wext_data->priv = (void *)hs;

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return NULL;
}

static int driver_wext_exit(struct hotspot *hs)
{
	struct driver_wext_data *drv_wext_data = hs->drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	netlink_deinit(drv_wext_data->netlink);
    close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}

static int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t len)    
{
    char *buf;                             
    struct iwreq iwr;
	
    buf = os_zalloc(len);                  

    os_memset(&iwr, 0, sizeof(iwr));       
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.flags = oid;
    iwr.u.data.flags |= OID_GET_SET_TOGGLE;

    if (data)
        os_memcpy(buf, data, len);

	if (buf) {
    	iwr.u.data.pointer = (caddr_t)buf;    
    	iwr.u.data.length = len;
	} else {
    	iwr.u.data.pointer = NULL;    
    	iwr.u.data.length = 0;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: oid=0x%x len (%d) failed",
               __FUNCTION__, oid, len);
        os_free(buf);
        return -1;
    }

    os_free(buf);
    return 0;
}

static int driver_wext_get_oid(struct driver_wext_data *drv_data, const char *ifname,
								   unsigned short oid, char *data, size_t *len)
{
	struct iwreq iwr;
	unsigned char *buf;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);

	if (*len > 4096) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: len = %d\n", __FUNCTION__, *len);
		return -1;
	}

	buf = os_zalloc(4096);


	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.flags = oid;
	iwr.u.data.length = *len;

	if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: oid=0x%x len (%d) failed",
               __FUNCTION__, oid, *len);
		*len = 0;
		os_free(buf);
		return -1;
	}

	if (iwr.u.data.length < *len) {
		os_memcpy(data, buf, iwr.u.data.length);
		*len = iwr.u.data.length;
	} else
		os_memcpy(data, buf, *len);

	os_free(buf);	

	return 0;
}
/*
 * Test Hotspot cmd
 */
static int driver_wext_test(void *drv_data, const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_TEST, NULL, 0);

	return ret;
}

/*
 * Set information element to driver 
 */
static int driver_wext_set_ie(void *drv_data, const char *ifname, char *ie, 
				       		  size_t ie_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_IE, ie, ie_len);

	return ret;
}

/*
 * Send ANQP request to driver
 */
static int driver_wext_send_anqp_req(void *drv_data, const char *ifname, 
							  		 const char *peer_mac_addr, const char *anqp_req, 
							 	 	 size_t anqp_req_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct anqp_req_data *req_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + anqp_req_len;	
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct anqp_req_data *)buf;
	
	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->anqp_req_len = anqp_req_len;
	os_memcpy(req_data->anqp_req, anqp_req, anqp_req_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ANQP_REQ, buf, len);


	os_free(buf);

	return ret;
}

/*
 * Send ANQP response to driver
 */
static int driver_wext_send_anqp_rsp(void *drv_data, const char *ifname, 
							  		 const char *peer_mac_addr, const char *anqp_rsp, 
							  		 size_t anqp_rsp_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct anqp_rsp_data *rsp_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*rsp_data) + anqp_rsp_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	rsp_data = (struct anqp_rsp_data *)buf;

	os_memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	
	rsp_data->anqp_rsp_len = anqp_rsp_len;
	os_memcpy(rsp_data->anqp_rsp, anqp_rsp, anqp_rsp_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ANQP_RSP, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Enable/Disable hotspot feature
 */
static int driver_wext_hotspot_onoff(void *drv_data, const char *ifname, 
					                 int enable, int event_trigger, int event_type)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	struct hs_onoff *onoff;
	char *buf;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	buf = os_zalloc(sizeof(*onoff));

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	onoff = (struct hs_onoff *)buf;

	onoff->hs_onoff = enable;
	onoff->event_trigger  = event_trigger;
	onoff->event_type = event_type;

	//drv_wext_data->we_version_compiled = driver_wext_get_we_version_compiled(drv_wext_data, ifname);
	//ret = driver_wext_set_oid(drv_data, ifname, OID_802_11_HS_IPCTYPE, &drv_wext_data->drv_mode, 1);
		
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ONOFF, buf, sizeof(*onoff));

	os_free(buf);

	return ret;	
}

/*
 * Send BTM query to driver
 */
static int driver_wext_send_btm_query(void *drv_data, const char *ifname, 
							  		  const char *peer_mac_addr, const char *btm_query, 
							  		  size_t btm_query_len)
{
	int ret = 0;


	return ret;
}

/*
 * Send BTM request to driver
 */
static int driver_wext_send_btm_req(void *drv_data, const char *ifname,
									const char *peer_mac_addr, const char *btm_req,
									size_t btm_req_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct btm_req_data *req_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + btm_req_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct btm_req_data *)buf;

	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->btm_req_len = btm_req_len;
	os_memcpy(req_data->btm_req, btm_req, btm_req_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WNM_BTM_REQ, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Send BTM response to driver
 */
static int driver_wext_send_btm_rsp(void *drv_data, const char *ifname,
									const char *peer_mac_addr, const char *btm_rsp,
									size_t btm_rsp_len)
{
	int ret = 0;


	return ret;
}


/*
 * Hotspot 2.0 Parameter Setting
 */
static int driver_wext_param_setting(void *drv_data, const char *ifname,
									 u32 param, u32 value)
{
	int ret;
	struct hs_param_setting param_setting;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	param_setting.param = param;
	param_setting.value = value;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_PARAM_SETTING, 
									(char *)&param_setting, sizeof(param_setting));
	
	return ret;
}

/*
 * WiFi driver verion
 */
static int driver_wext_wifi_version(void *drv_data, const char *ifname,
							   		char *version, size_t *version_len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WIFI_VER, version, version_len);

	return ret;
}						   

/*
 * Get IPv4 Proxy ARP list
 */
static int driver_wext_ipv4_proxy_arp_list(void *drv_data, const char *ifname,
									  	   char *proxy_arp_list, size_t *proxy_arp_list_len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WNM_IPV4_PROXY_ARP_LIST,
									proxy_arp_list, proxy_arp_list_len);

	return ret;
}

/*
 * Get IPv6 Proxy ARP list
 */
static int driver_wext_ipv6_proxy_arp_list(void *drv_data, const char *ifname,
									  	   char *proxy_arp_list, size_t *proxy_arp_list_len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WNM_IPV6_PROXY_ARP_LIST,
									proxy_arp_list, proxy_arp_list_len);

	return ret;
}

static int driver_wext_validate_security_type(void *drv_data, const char *ifname)
{
	
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	struct security_type sec_type;
	size_t security_type_len = sizeof(sec_type);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	os_memset(&sec_type, 0, sizeof(sec_type));

	DBGPRINT(RT_DEBUG_TRACE, "%s: sec_type_len = %d\n", __FUNCTION__, sizeof(sec_type));
	
	driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_SECURITY_TYPE, (char *)&sec_type, &security_type_len);

	DBGPRINT(RT_DEBUG_TRACE, "%s: auth_mode = %d, encryp_type = %d\n", __FUNCTION__, sec_type.auth_mode,
								sec_type.encryp_type);

	if (((sec_type.auth_mode == 6) || (sec_type.auth_mode == 8)) && 
							((sec_type.encryp_type == 6) || (sec_type.encryp_type == 8)))
		return 1;
	else
		return 0;
}

static int driver_wext_reset_resource(void *drv_data, const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_RESET_RESOURCE, NULL, 0);

	return ret;
}

static int driver_wext_get_bssid(void *drv_data, const char *ifname, char *bssid, size_t *bssid_len)
{

	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_HS_BSSID, bssid, bssid_len);

	return ret;
}

const struct hotspot_drv_ops hotspot_drv_wext_ops = {
	.drv_inf_init = driver_wext_init,
	.drv_inf_exit = driver_wext_exit,
	.drv_test = driver_wext_test,
	.drv_set_ie = driver_wext_set_ie,
	.drv_send_anqp_req = driver_wext_send_anqp_req,
	.drv_send_anqp_rsp = driver_wext_send_anqp_rsp,
	.drv_hotspot_onoff = driver_wext_hotspot_onoff,
	.drv_send_btm_query = driver_wext_send_btm_query,
	.drv_send_btm_req = driver_wext_send_btm_req,
	.drv_send_btm_rsp = driver_wext_send_btm_rsp,
	.drv_param_setting = driver_wext_param_setting,
	.drv_wifi_version = driver_wext_wifi_version,
	.drv_ipv4_proxy_arp_list = driver_wext_ipv4_proxy_arp_list,
	.drv_ipv6_proxy_arp_list = driver_wext_ipv6_proxy_arp_list,
	.drv_validate_security_type = driver_wext_validate_security_type,
	.drv_reset_resource = driver_wext_reset_resource,
	.drv_get_bssid = driver_wext_get_bssid,
};
