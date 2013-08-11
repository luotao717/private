#ifndef __CMD_H
#define __CMD_H

#define CMD_PORT		9001
#define SEND_PORT		9002
#define RCV_PORT		9003
#define CALL_PORT		9004

#define MAX_DHCP_CLIENTS	6

#define SEND_OK			0
#define CONNECTING		1
#define SEND_FAIL		2

#define CMD_FLAGS		"WIBA"

#define NETCOM_DIAL_UP		0x40
#define NETCOM_HUNG_UP  	0x41
#define NETCOM_CALLIN		0x42
#define NETCOM_CALLIN_HUNG	0x43
#define NETCOM_LINE_ON		0x44
#define NETCOM_LINE_OFF		0x45

//SET WAN
#define NETCOM_SET_WAN		0x50
#define NETCOM_GET_WAN		0x51

//SET LAN
#define NETCOM_SET_LAN		0x52
#define NETCOM_GET_LAN		0x53

//SET WIRELESS
#define NETCOM_SET_WIRELESS 0x54
#define NETCOM_GET_WIRELESS	0x55

//SET USER ACCOUNT
#define NETCOM_SET_ACCOUNT	0x56
#define NETCOM_GET_ACCOUNT	0x57

//SET FIREWALL FILTER
#define NETCOM_SET_FILTER		0x58
#define NETCOM_GET_FILTER_LIST	0x59
#define NETCOM_DEL_FILTER		0x5A

//SET NAT
#define NETCOM_SET_NAT			0x5B
#define	NETCOM_GET_NAT_LIST		0x5C
#define NETCOM_DEL_NAT			0x5D

//SET DMZ
#define NETCOM_SET_DMZ			0x5E
#define NETCOM_GET_DMZ			0x5F

//GET DEVICE INFORMATION
#define NETCOM_GET_DEVIEC_INFO	0x60

//GET CURRENT SIGNAL FOR 3G
#define NETCOM_GET_SIGNAL		0x61

//GET FLOW RATE
#define NETCOM_GET_FLOW			0x62

//RESET TOTAL FLOW RATE
#define NETCOM_RESET_TOTAL_FLOW	0x63

//GET BATTRY EMERGY
#define NETCOM_GET_BATTRY_EMERGY 0x64

//LOGIN
#define NETCOM_LOGIN			0x65

//SET DEFAULT
#define NETCOM_SET_DEFAULT	0x70

//SET REBOOT
#define NETCOM_SET_REBOOT	0x71

typedef struct  tagMySockInfo
{
  int		fd;
  int		sinlen;
	struct sockaddr_in sin;	
}MySockInfo,*PMySockInfo;

typedef struct tagPhoneOpera
{
	unsigned char flag[4]; //WIBA
	unsigned char cmd;
	unsigned char phonenum[16];
	unsigned char res[32];
}__attribute__((packed)) PhoneOpera, *PPhoneOpera;

typedef struct tagWanParam
{
	unsigned char 	flag[4]; //WIBA
	unsigned char 	cmd;
	unsigned char 	res[3];
	unsigned int  	wantype; // 0 :STATIC 1 :DHCP 2 :PPPOE 3 :3G 4 :CMCC 

	//static
	unsigned char 	static_ip[32];
	unsigned char 	static_mask[32];
	unsigned char 	static_gw[32];
	unsigned char 	static_pdns[32];
	unsigned char 	static_sdns[32];
	unsigned char 	static_res[32];

	//dhcp
	unsigned char 	dhcp_name[32]; //optional
	unsigned char 	dhcp_res[32];

	//pppoe
	unsigned char 	pppoe_usrname[32];
	unsigned char 	pppoe_passwd[32];
	unsigned char 	pppoe_res[32];

	// 3G
	unsigned int  	g3_dialparam; // 0 :auto 1 :manual
	unsigned char 	g3_dialnum[32];
	unsigned char 	g3_usrname[16];
	unsigned char 	g3_passwd[16];
	unsigned char 	g3_apn[16];
	unsigned char 	g3_simpin[4];
	unsigned char 	g3_res[32];

	//CMCC
	unsigned char 	cmcc_phonenum[16];
	unsigned char 	cmcc_passwd[16];
	unsigned char 	cmcc_res[32];
}__attribute__((packed))WanParam, *PWanParam;

typedef struct tagLanParam
{
	unsigned char 	flag[4]; //WIBA
	unsigned char 	cmd;
	unsigned char 	res[3];
	unsigned char 	lanip[32];
	unsigned char 	lanmask[32];
	unsigned int  	dhcp_en; // 0 :disable 1 :enable
	unsigned char 	dhcp_start[32];
	unsigned char 	dhcp_end[32];
	unsigned char 	lan_res[32];
}__attribute__((packed))LanParam, *PLanParam;

typedef struct tagSetWireless
{
	unsigned char 	flag[4]; //WIBA
	unsigned char 	cmd;
	unsigned char 	res1[3];
	unsigned char 	ssid[32];
	unsigned int  	channel; // 0 :auto  1-14
	unsigned int	securitymode; // 0 :disable 1 :wpapsk  2 :wpa2psk  3 :wpapskwpa2psk
	unsigned int	wpaalg;			// 0 :TKIP  1 :AES   2 :TKIPAES
	unsigned char 	passwd[64];    // at least 8 character
	unsigned char	res2[32];
}__attribute__((packed))WirelessParam, *PWirelessParam;

typedef struct tagAccountParam
{
	unsigned char 	flag[4];
	unsigned char 	cmd;
	unsigned char 	res[3];
	unsigned char	account_name[32];
	unsigned char	account_passwd[32];
	unsigned char	account_res[32];
}__attribute__((packed))AccountParam, *PAccountParam;

typedef struct tagSetFilter
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	enable;		// 0 : diasble filter  1 : enable filter
	unsigned char	mac_addr[32];
	unsigned char	dest_ip[32];
	unsigned char	source_ip[32];
	unsigned char	protocol;	// 0 : UNKNONW  1 : TCP  2 : UDP  3 : TCP&UDP  4 : ICMP   5 : NONE
	unsigned char	res2[3];
	unsigned int	dport_start;
	unsigned int	dport_end;
	unsigned int	sport_start;
	unsigned int	sport_end;
	unsigned char	action; 	// 0 : drop 	
	unsigned char	res3[15];
}__attribute__((packed))SetFilter, *PSetFilter;

typedef struct tagGetFilterList
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	enable; // 0 : diasble filter  1 : enable filter
	unsigned char	rules[1024];
}__attribute__((packed))GetFilterList, *PGetFilterList;

typedef struct tagDelFilter
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	del[MAX_DHCP_CLIENTS]; //  max lists 6   0 : reserve  1 : delete
}__attribute__((packed))DelFilter, *PDelFilter;


typedef struct tagSetNat
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	enable; // 0 : diasble nat  1 : enable nat
	unsigned char	ipaddr[32];
	unsigned int	port_start;
	unsigned int	port_end;
	unsigned int	protocol;	//  1 :tcp  2 :udp 3 :all 
	unsigned char	res2[32];
}__attribute__((packed))SetNat, *PSetNat;


typedef struct tagGetNatList
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	enable; // 0 : diasble nat  1 : enable nat
	unsigned char	rules[1024];
}__attribute__((packed))GetNatList, *PGetNatList;


typedef struct tagDelNat
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	del[MAX_DHCP_CLIENTS]; //  max lists 6   0 : reserve  1 : delete
}__attribute__((packed))DelNat, *PDelNat;


typedef struct tagSetDMZ
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res[3];
	unsigned int	enable;		// 0 : disable dmz  1 : enable dmz
	unsigned char	dmz_addr[32];
}__attribute__((packed))SetDMZ, *PSetDMZ;

typedef struct tagGetDMZ
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res[3];
	unsigned int	enable; // 0 : diasble dmz  1 : enable dmz
	unsigned char	dmz_addr[32];
}__attribute__((packed))GetDMZ, *PGetDMZ;


typedef struct tagGetDevInfo
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res[3];
	unsigned char	soft_ver[32];
	unsigned int  	wantype; // 0 :STATIC 1 :DHCP 2 :PPPOE 3 :3G 4 :CMCC 

	//WAN
	unsigned char	wan_ip[32];
	unsigned char	wan_mask[32];
	unsigned char	wan_gw[32];
	unsigned char	wan_pdns[32];
	unsigned char	wan_sdns[32];
	unsigned char	ispname[32]; // for 3G 

	//LAN
	unsigned char	lan_ip[32];
	unsigned char	lan_mask[32];
	unsigned char	lan_mac[32];

	//DHCP CLIENTS
	unsigned int	dhcp_num;
	unsigned char	dhcp_devname[MAX_DHCP_CLIENTS][32];
	unsigned char	dhcp_devmac[MAX_DHCP_CLIENTS][32];
	unsigned char	dhcp_devip[MAX_DHCP_CLIENTS][32];
	unsigned char	dhcp_devtime[MAX_DHCP_CLIENTS][32];
}__attribute__((packed))GetDevInfo, *PGetDevInfo;

typedef struct tagGetSignal
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned char	wan_type[16]; // EDGE  WCDMA TDSCDMA
	int				signal_level; // 0--4   0 : no signal
	unsigned char	res2[16];
}__attribute__((packed))GetSignal, *PGetSignal;

typedef struct tagGetFlowRate
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	long long		cur_flow_up;
	long long		cur_flow_down;
	long long		his_flow_up;
	long long		his_flow_down;
	unsigned char	res2[32];
}__attribute__((packed))GetFlowRate, *PGetFlowRate;


typedef struct tagBattryEmergy
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned int	emergy_flag;	// 0 : normal  1 : low 
	unsigned char	res2[32];
}__attribute__((packed))BattryEmergy, *PBattryEmergy;

typedef struct tagLoginParam
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	res1[3];
	unsigned char	user[32];
	unsigned char	passwd[32];
}__attribute__((packed))LoginParam,*PLoginParam;

typedef struct tagResult2Login
{
	unsigned char	flag[4];
	unsigned char	cmd;
	unsigned char	result;	// 1 : login success .  else : login failed
}__attribute__((packed))Result2Login, *PResult2Login;

typedef struct tagSetSingleCmd
{
	unsigned char 	flag[4];
	unsigned char	cmd;	//  0x61 reset flow rate  0x70 load -default  0x71 reboot
	unsigned char	res[3];
}__attribute__((packed))SetSingleCmd, *PSetSingleCmd;

typedef struct tagCmdResult
{
	unsigned char flag[4];
	unsigned char cmd;
	unsigned char result;
}__attribute__((packed)) CmdResult, *PCmdResult;
#endif
