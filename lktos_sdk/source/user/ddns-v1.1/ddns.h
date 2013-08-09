/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//         The inclde file of ddns module
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
//==============================================================================
//                                    MACROS
//==============================================================================
#define DYNDNS_NAME "dyndns"
#define DYNDNS_DEFAULT_SERVER "members.dyndns.org"
#define DYNDNS_DEFAULT_PORT 80
#define DYNDNS_REQUEST "/nic/update"
#define DYNDNS_STAT_REQUEST "/nic/update"
#define DYNDNS_MAX_INTERVAL (25*24*3600)

#define QDNS_NAME "qdns"
//#define QDNS_DEFAULT_SERVER "members.3322.org"
#define QDNS_DEFAULT_SERVER "members.3322.net"
#define QDNS_DEFAULT_PORT 80
#define QDNS_REQUEST "/dyndns/update"
#define QDNS_STAT_REQUEST "/dyndns/update"
#define QDNS_MAX_INTERVAL (25*24*3600)

#define ORAY_NAME "oray"
#define ORAY_DEFAULT_SERVER "ph002.oray.net"
#define ORAY_DEFAULT_PORT 6060
#define ORAY_REQUEST "/auth/dynamic.html"

#if 0
#define M88IP_NAME "88ip"
#define M88IP_DEFAULT_SERVER "user.dipns.com"
#define M88IP_DEFAULT_PORT 80
#define M88IP_REQUEST "/api/ipaddress.asp"
#endif

#define TZO_NAME "tzo"
#define TZO_DEFAULT_SERVER "cgi.tzo.com"
#define TZO_DEFAULT_PORT 80
#define TZO_REQUEST "/webclient/signedon.html"
#define TZO_SUCCESS 1


#define NOIP_NAME "noip"
#define NOIP_DEFAULT_SERVER "dynupdate.no-ip.com"
#define NOIP_DEFAULT_PORT 80
#define NOIP_REQUEST ""
#define NOIP_STAT_REQUEST ""
#define NOIP_MAX_INTERVAL (25*24*3600)
#define NOIP_ALREADYSET 0
#define NOIP_SUCCESS 1

#define DTDNS_NAME "dtdns"
#define DTDNS_DEFAULT_SERVER "www.dtdns.com"
#define DTDNS_DEFAULT_PORT 80
#define DTDNS_REQUEST "/api/autodns.cfm?"
#define DTDNS_STAT_REQUEST ""
#define DTDNS_MAX_INTERVAL (25*24*3600)

#define CHANGEIP_NAME "changeip"
#define CHANGEIP_DEFAULT_SERVER "www.changeip.com"
#define CHANGEIP_DEFAULT_PORT 80
#define CHANGEIP_REQUEST "https://changeip.com/nic/update"

#define NSTR(x)			inet_ntoa(*(struct in_addr *)&x)

#define MY_IP			SYS_wan_ip
#define MY_IP_STR		NSTR(SYS_wan_ip)

#define DEVICE_NAME		"DDNS Server"
#define FW_VERSION		"V1.1"
#define FW_OS			"LINUX"
#define AUTHOR			"By chenfei"


//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================
struct service_cxy
{
  char *name;
  int (*update_entry)();
  char *default_server;
  unsigned short default_port;
  char *default_request;
};

enum {
  UPDATERES_OK = 0,
  UPDATERES_OK_2,
  UPDATERES_ERROR,
  UPDATERES_SHUTDOWN,
};

struct userInfo_cxy{
	struct service_cxy *service;
	char usrname[30];
	char usrpwd[20];
	char host[30];
	char mx[30];
	char backmx[30];
	char url[30];
	char wildcard;
	int trytime;
	int ip;
	int updated_time;
	char status;
	struct userInfo_cxy *next;
};


