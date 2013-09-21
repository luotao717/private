#ifndef _hotplug_3gmodem_h_
#define _hotplug_3gmodem_h_

#define 								ACM0_TYPE 0
#define 								USB0_TYPE 1
#define 								USB2_TYPE 2
#define 								HS0_TYPE  3	//option类型
#define 								HS1_TYPE  4
#define 								HS2_TYPE  5
#define 								USB1_TYPE 6 //MF628(ONDA)
#define 								USB5_TYPE 7
#define 								USB3_TYPE 8
#define									ACM1_TYPE 9
#define									ACM2_TYPE 10
#define 								USB4_TYPE 11
#define 								USB9_TYPE 12


#define USBMODEMPATH	"/var/usbmodem/"
#define STR_ACM0 		"/dev/ttyACM0"
#define STR_ACM1		"/dev/ttyACM1"
#define STR_ACM2		"/dev/ttyACM2"
#define STR_USB0 		"/dev/ttyUSB0"
#define STR_USB1        "/dev/ttyUSB1"
#define STR_USB2 		"/dev/ttyUSB2"
#define STR_USB3		"/dev/ttyUSB3"
#define STR_USB4		"/dev/ttyUSB4"
#define STR_USB5 		"/dev/ttyUSB5"
#define STR_USB9 		"/dev/ttyUSB9"
#define STR_HS0        	"/dev/ttyHS0"
#define STR_HS1        	"/dev/ttyHS1"
#define STR_HS2        	"/dev/ttyHS2"
#define STR_IN_FILE		"/var/ttyACM0"

typedef struct  tagCDMAInfo
{
	unsigned int 			ModuleType;
  	int 					csq;
	int 					mobileNet;
	char 					Status[16];
	char 					operatorName[16]; 
	char 					moduleVersion[128]; 
}CDMAInfo,*PCDMAInfo;

typedef struct tagOptInfo
{
	unsigned char			*ID;			
	unsigned char			*optname;				//运营商名祿
	unsigned char			*username;			//用户县
	unsigned char			*passwd;			//密码
	unsigned char			*papn;				//该运营商所对应的APN
	unsigned char 			*dialnum;
}OptInfo, *POptInfo;

typedef struct tagOptionNetS
{
	unsigned char 	ip[16];
	unsigned char 	dns1[16];
	unsigned char 	dns2[16];
	unsigned char 	res[16];
}OptionNetS,*POptionNetS;


#endif
