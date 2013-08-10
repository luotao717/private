/*
  USB Driver for GSM modems

  Copyright (C) 2005  Matthias Urlichs <smurf@smurf.noris.de>

  This driver is free software; you can redistribute it and/or modify
  it under the terms of Version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Portions copied from the Keyspan driver by Hugh Blemings <hugh@blemings.org>

  History: see the git log.

  Work sponsored by: Sigos GmbH, Germany <info@sigos.de>

  This driver exists because the "normal" serial driver doesn't work too well
  with GSM modems. Issues:
  - data loss -- one single Receive URB is not nearly enough
  - nonstandard flow (Option devices) control
  - controlling the baud rate doesn't make sense

  This driver is named "option" because the most common device it's
  used for is a PC-Card (with an internal OHCI-USB interface, behind
  which the GSM interface sits), made by Option Inc.

  Some of the "one port" devices actually exhibit multiple USB instances
  on the USB bus. This is not a bug, these ports are used for different
  device features.
*/

#define DRIVER_VERSION "v0.7.1"
#define DRIVER_AUTHOR "Matthias Urlichs <smurf@smurf.noris.de>"
#define DRIVER_DESC "USB Driver for GSM modems"

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  option_open(struct usb_serial_port *port, struct file *filp);
static void option_close(struct usb_serial_port *port, struct file *filp);
static int  option_startup(struct usb_serial *serial);
static void option_shutdown(struct usb_serial *serial);
static void option_rx_throttle(struct usb_serial_port *port);
static void option_rx_unthrottle(struct usb_serial_port *port);
static int  option_write_room(struct usb_serial_port *port);

static void option_instat_callback(struct urb *urb);

static int option_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  option_chars_in_buffer(struct usb_serial_port *port);
static int  option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void option_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void option_break_ctl(struct usb_serial_port *port, int break_state);
static int  option_tiocmget(struct usb_serial_port *port, struct file *file);
static int  option_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  option_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define OPTION_VENDOR_ID			0x0AF0
#define OPTION_PRODUCT_COLT			0x5000
#define OPTION_PRODUCT_RICOLA			0x6000
#define OPTION_PRODUCT_RICOLA_LIGHT		0x6100
#define OPTION_PRODUCT_RICOLA_QUAD		0x6200
#define OPTION_PRODUCT_RICOLA_QUAD_LIGHT	0x6300
#define OPTION_PRODUCT_RICOLA_NDIS		0x6050
#define OPTION_PRODUCT_RICOLA_NDIS_LIGHT	0x6150
#define OPTION_PRODUCT_RICOLA_NDIS_QUAD		0x6250
#define OPTION_PRODUCT_RICOLA_NDIS_QUAD_LIGHT	0x6350
#define OPTION_PRODUCT_COBRA			0x6500
#define OPTION_PRODUCT_COBRA_BUS		0x6501
#define OPTION_PRODUCT_VIPER			0x6600
#define OPTION_PRODUCT_VIPER_BUS		0x6601
#define OPTION_PRODUCT_GT_MAX_READY		0x6701
#define OPTION_PRODUCT_GT_MAX			0x6711
#define OPTION_PRODUCT_FUJI_MODEM_LIGHT		0x6721
#define OPTION_PRODUCT_FUJI_MODEM_GT		0x6741
#define OPTION_PRODUCT_FUJI_MODEM_EX		0x6761
#define OPTION_PRODUCT_FUJI_NETWORK_LIGHT	0x6731
#define OPTION_PRODUCT_FUJI_NETWORK_GT		0x6751
#define OPTION_PRODUCT_FUJI_NETWORK_EX		0x6771
#define OPTION_PRODUCT_KOI_MODEM		0x6800
#define OPTION_PRODUCT_KOI_NETWORK		0x6811
#define OPTION_PRODUCT_SCORPION_MODEM		0x6901
#define OPTION_PRODUCT_SCORPION_NETWORK		0x6911
#define OPTION_PRODUCT_ETNA_MODEM		0x7001
#define OPTION_PRODUCT_ETNA_NETWORK		0x7011
#define OPTION_PRODUCT_ETNA_MODEM_LITE		0x7021
#define OPTION_PRODUCT_ETNA_MODEM_GT		0x7041
#define OPTION_PRODUCT_ETNA_MODEM_EX		0x7061
#define OPTION_PRODUCT_ETNA_NETWORK_LITE	0x7031
#define OPTION_PRODUCT_ETNA_NETWORK_GT		0x7051
#define OPTION_PRODUCT_ETNA_NETWORK_EX		0x7071
#define OPTION_PRODUCT_ETNA_KOI_MODEM		0x7100
#define OPTION_PRODUCT_ETNA_KOI_NETWORK		0x7111
#define OPTION_PRODUCT_GTM380        		0x7211		//jiangpan add 2011-11-4


/* NOVATELWIRELESS Products */
#define NOVATELWIRELESS_VENDOR_ID           0x1410

	/* MERLIN EVDO PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_V640		0x1100
#define NOVATELWIRELESS_PRODUCT_V620		0x1110
#define NOVATELWIRELESS_PRODUCT_V740		0x1120
#define NOVATELWIRELESS_PRODUCT_V720		0x1130
	/* MERLIN HSDPA/HSPA PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_U730		0x1400
#define NOVATELWIRELESS_PRODUCT_U740		0x1410
#define NOVATELWIRELESS_PRODUCT_U870		0x1420
#define NOVATELWIRELESS_PRODUCT_XU870		0x1430
#define NOVATELWIRELESS_PRODUCT_X950D		0x1450
	/* EXPEDITE PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_EV620		0x2100
#define NOVATELWIRELESS_PRODUCT_ES720		0x2110
#define NOVATELWIRELESS_PRODUCT_E725		0x2120
#define NOVATELWIRELESS_PRODUCT_ES620		0x2130
#define NOVATELWIRELESS_PRODUCT_EU730		0x2400
#define NOVATELWIRELESS_PRODUCT_EU740		0x2410
#define NOVATELWIRELESS_PRODUCT_EU870D		0x2420
	/* OVATION PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_MC727		0x4100
#define NOVATELWIRELESS_PRODUCT_MC950D		0x4400
#define NOVATELWIRELESS_PRODUCT_MC930D      0x4400		//Supported on 2008-12-30
#define NOVATELWIRELESS_PRODUCT_6002		0x6002
	/* FUTURE NOVATEL PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_EVDO_1		0x6000
#define NOVATELWIRELESS_PRODUCT_HSPA_1		0x7000
#define NOVATELWIRELESS_PRODUCT_EMBEDDED_1	0x8000
#define NOVATELWIRELESS_PRODUCT_GLOBAL_1	0x9000
#define NOVATELWIRELESS_PRODUCT_EVDO_2		0x6001
#define NOVATELWIRELESS_PRODUCT_HSPA_2		0x7001
#define NOVATELWIRELESS_PRODUCT_EMBEDDED_2	0x8001
#define NOVATELWIRELESS_PRODUCT_GLOBAL_2	0x9001

/* HUAWEI Products */
#define HUAWEI_VENDOR_ID			0x12D1

#define HUAWEI_PRODUCT_E169G			0x1001				//Supported on 2008-12-30
#define HUAWEI_PRODUCT_E220			0x1003				//Supported on 2008-12-30
#define HUAWEI_PRODUCT_770W			0x1404				//zhubo add 2010.08.23
#define HUAWEI_PRODUCT_EC121			0x1411				//Supported on 2009-4-1
#define HUAWEI_PRODUCT_EC168C			0x1412				//zhubo add 2010.11.11
#define HUAWEI_PRODUCT_E180			0x1414				//zhubo add 2009.10.14
#define HUAWEI_PRODUCT_E1692			0x1446				//zhubo add 2009.10.14
#define HUAWEI_PRODUCT_K3675			0x1465				//zhubo add 2009.11.03
#define HUAWEI_PRODUCT_K4505			0x1464				//zhubo add 2010.01.04
#define HUAWEI_PRODUCT_E1820			0x14ac				//zhubo add 2010.01.04 E270+ E1762 E1820
#define HUAWEI_PRODUCT_EC1260			0x140b				//zhubo add 2010.04.19
#define HUAWEI_PRODUCT_E1690			0x140c				//zhubo add 2010.01.04 E1690 E1692 E1762
#define HUAWEI_PRODUCT_E1612			0x1406				//zhubo add 2010.01.04 E1612
#define HUAWEI_PRODUCT_E1752			0x141b				//zhubo add 2010.03.22 E1752
#define HUAWEI_PRODUCT_K4505			0x1464				//zhubo add 2010.07.02 for HSPA+
#define HUAWEI_PRODUCT_EM820W			0x1404				//jiangpan add 2011.04.14
#define HUAWEI_PRODUCT_E303				0x1506

/* AMOI PRODUCTS */
#define AMOI_VENDOR_ID				0x1614
#define AMOI_PRODUCT_H01			0x0800				//Supported on 2008-12-30
#define AMOI_PRODUCT_H01A			0x7002
#define AMOI_PRODUCT_H02			0x0802

/* ANYDATA PRODUCTS */
#define ANYDATA_VENDOR_ID			0x16d5
#define ANYDATA_PRODUCT_ADU_620UW	0x6202
#define ANYDATA_PRODUCT_ADU_E100A	0x6501
#define ANYDATA_PRODUCT_ADU_500A	0x6502

/* AXESSTEL PRODUCTS */
#define AXESSTEL_VENDOR_ID			0x1726
#define AXESSTEL_PRODUCT_MV110H		0x1000

/* ONDA PRODUCTS */
/*
#define ONDA_VENDOR_ID				0x19d2
#define ONDA_PRODUCT_MSA501HS		0x0001	//MF622
#define ONDA_PRODUCT_ET502HS		0x0002
*/
/* ZTE PRODUCTS */
#define ZTE_VENDOR_ID				0x19d2

//HSPA
#define ZTE_PRODUCT_HSPA1			0x0001				//MF622, ONDA MSA501HS
#define ZTE_PRODUCT_HSPA2			0x0002				//MF622, ONDA ET502HS
#define ZET_PRODUCT_TDMU350                     0x0003                          //MU350 TD zhubo add 2009.05.15
#define ZTE_PRODUCT_HSPA15			0x0015				//MF628
#define ZTE_PRODUCT_HSPA16			0x0016
#define ZTE_PRODUCT_HSPA17			0x0017				
#define ZTE_PRODUCT_HSPA18		        0x0018				//MF626/MF636
#define ZTE_PRODUCT_HSPA19		        0x0019				//MF626/MF636
#define ZTE_PRODUCT_HSPA31		        0x0031				//MF626/MF636, USB2 type confirmed by James on 2009-01-15
#define ZTE_PRODUCT_HSPA33		        0x0033				//MF626/MF636
#define ZTE_PRODUCT_HSPA37		        0x0037				//MF626/MF636/MF628
#define ZTE_PRODUCT_HSPA39		        0x0039				//MF626/MF636
#define ZTE_PRODUCT_HSPA42		        0x0042				//MF626/MF636
#define ZTE_PRODUCT_HSPA48		        0x0048				//MF626/MF636
#define ZTE_PRODUCT_AC580		        0x0094				//AC580 zhubo add 2009.07.30
#define ZTE_PRODUCT_HSPA51			0x0051				//MF628
#define ZTE_PRODUCT_HSPA6535			0x0052				//ZTE 6535 zhubo add 2009.10.14
#define ZTE_PRODUCT_MF633R			0x0053				//ZTE 6535 zhubo add 2010.06.03
#define ZTE_PRODUCT_K3520			0x0055				//ZTE K3520 zhubo add 2009.10.14
#define ZTE_PRODUCT_HSPA57		        0x0057				//MF626/MF636, USB2 type confirmed by James on 2009-01-15
#define ZTE_PRODUCT_K3565			0x0063				//K3565-Z zhubo add 2009.11.02
#define ZTE_PRODUCT_AC560			0x0070				//AC560 zhubo add 2009.11.02
#define ZTE_PRODUCT_AC560_OTHER			0x0073				//AC560 zhubo add 2010.03.19
#define ZTE_PRODUCT_A356			0x0079				//A356 zhubo add 2010.10.08
#define ZTE_PRODUCT_MF200			0x0117				//MF200 jiangpan add 2010.12.28
#define ZTE_PRODUCT_MF627			0x2003				//MF627 jiangpan add 2011.03.30
#define ZTE_PRODUCT_MU301			0x1303				//MU301 jiangpan add 2011.06.17
//EVDO
#define ZTE_PRODUCT_CDMA_TECH			0xfffe
#define ZTEMT_PRODUCT_AC2710			0xffff				//Supported on 2008-12-30
#define ZTE_PRODUCT_EVDO_TECH			0x0054				//Supported on 2009-01-08
#define ZTE_PRODUCT_EVDO_2736			0xfff1        //zhubo add 2009.04.20
#define ZTE_PRODUCT_EVDO_2726			0xfff5        //zhubo add 2009.04.20
#define ZTE_PRODUCT_WCDWA_K3765Z		0x2002	      //zhubo add 2009.10.13
#define ZTE_PRODUCT_MC2716			0xffed	      //zhubo add 2010.05.13
#define ZTE_PRODUCT_3812			0xffeb	      //zhubo add 2010.05.13
#define ZTE_PRODUCT_MU301			0x1303	      //zhubo add 2011.01.10
#define ZTE_PRODUCT_MF200			0x0117		//MF200 jiangpan add 2010.12.28

/* Standard QUALCOMM Products */
#define QUALCOMM_VENDOR_ID			0x05C6
#define QUALCOMM_PRODUCT_PHu9			0x9000
#define QUALCOMM_PRODUCT_9003      		0X9003
#define QUALCOMM_VENDOR_ID1			0x05c6	//jiangpan add 2010.12.27
#define QUALCOMM_PRODUCT_UNKNOW2		0x6000	//jiangpan add 2010.12.27

/* KYOCERA PRODUCTS */
#define KYOCERA_VENDOR_ID			0x0c88
#define KYOCERA_PRODUCT_KPC650			0x17da
#define KYOCERA_PRODUCT_KPC680			0x180a

/* ELSE */
#define MAXON_VENDOR_ID				0x16d8

#define TELIT_VENDOR_ID				0x1bc7
#define TELIT_PRODUCT_UC864E			0x1003

#define SIMCOM_VENDOR_ID			0x1ab7//James 2009-01-08
#define SIMCOM_PRODUCT_1			0x2000
#define SIMCOM_PRODUCT_2			0x1206//zhubo add 2009.06.22
#define SIMCOM_PRODUCT_3			0x1200//zhubo add 2009.06.22
#define TD6311_PRODUCT				0x6000

#define TD_LIANXIN_PRODUCT			0x5730
#define TD_LIANXIN_PRODUCT_5740			0x5740
#define TD_PRODUCT_5731				0x5731

#define ChinaTelcom_VENDOR_ID			0x1ed1
#define ChinaTelcom_PRODUCT			0x0090//James 2009-02-24

#define EPHONE_VENDOR_ID			0x201e
#define EPHONE_PRODUCT				0x3000//James 2009-05-19

/*LongQi PRODUCTS */
#define LQ_VENDOR_ID				0x1c9e
#define LQ_PRODUCT_6061				0x6061				//Supported on 2008-12-30
#define LQ_PRODUCT_6000				0x6000				//zhubo add 2009.09.08
#define LQ_PRODUCT_9603				0x9603				//zhubo add 2009.09.22
#define LQ_PRODUCT_9E00				0x9e00				//jiangpan add 2011.01.11

/*Rway Modem*/
#define RWAY_VENDOR_ID				0x2073//zhubo add 2009.05.25
#define RWAY_PRODUCT_ID				0x0001

/*unknow modem*/
#define UNDEFINE_MODEM_0_VENDOR_ID		0x1e0e//zhubo add 2009.07.03
#define UNDEFINE_MODEM_0_PRODUCT_ID		0xcefe
#define DLINK_MODEM_PRODUCT_ID			0xce16
#define UNKNOW_MODEM_1_VENDOR_ID		0x1e89
#define UNKNOW_MODEM_1_PRODUCT_ID		0x1e16
#define UNKNOW_MODEM_20_PRODUCT_ID		0x1a16
#define UNKNOW_MODEM_PRODUCT_ID_20		0x1a20  //jiangpan add 2011.04.07
#define UNKNOW_MODEM_105_VENDOR_ID		0x8848	//jiangpan add 2011.04.19
#define UNKNOW_MODEM_105_PRODUCT_ID		0x3721	

/*NOKIA*/
#define NOKIA_MODEM_0_VENDOR_ID			0x0421 //zhubo add 2009.10.19
#define NOKIA_MODEM_0_PRODUCT_ID		0x0612 //zhubo add 2009.10.19

/*SIERRA*/
#define SIERRA_VENDOR_306_ID			0x1199 //zhubo add 2009.10.29
#define SIERRA_PRODUCT_306_ID			0x68a3 //zhubo add 2009.10.29
#define SIERRA_PRODUCT_MC8781_ID		0x6856
#define SIERRA_PRODUCT_MC8780_32ID		0x6832
#define SIERRA_PRODUCT_MC8785_ID		0x683b //jiangpan add 2011.04.19

/*ALCATEL*/
#define ALCATEL_VENDOR_X200_ID			0x1bbb //zhubo add 2009.10.29
#define ALCATEL_PRODUCT_X200_ID			0x0000 //zhubo add 2009.10.29

/*Philips TalkTalk*/
#define PHI_VENDOR_TALK_ID						0x0471 //zhubo add 2010.03.08
#define PHI_PRODUCT_TALK_ID						0x1234

/*Motorola */
#define MOTO_VENDOR_ID								0x148f
#define MOTO_PRODUCT_ID								0x9021

/*Zydas*/
#define ZYDAS_VENDOR_ID								0x0ace
#define	ZYDAS_PRODUCT_ID_1						0x2011
#define ZYDAS_PRODUCT_ID_2						0x20ff

/*INFOCERT*/
#define	INFOCERT_VENDOR_ID						0x072f
#define INFOCERT_PRODUCT_ID						0x90cc

/*UTSTARCOM*/
#define UT_UM185E_VENDOR							0x106c
#define UT_UM185E_PRODUCT							0x3717

/* Sprocomm products */
#define SPRO_VENDOR_ID				0x1782
#define SPRO_PRODUCT_ID				0x3D00
#define SPRO_PRODUCT_686_ID                     0x4d00

#define UNKNOW_MODEM_2_VENDOR_ID		0x15eb
#define UNKNOW_MODEM_2_PRODUCT_ID		0x0001
#define VIA_MODEM_2_PRODUCT_ID			0x1231
#define TENDA_MODEM_PORDUCT_ID			0x7152 //zhubo add 2010.05.08

#define UNKNOW_MODEM_3_VENDOR_ID		0x1bc7
#define UNKNOW_MODEM_3_PRODUCT_ID		0x1002
#define UNKNOW_MODEM_4_VENDOR_ID		0x1d09
#define UNKNOW_MODEM_4_PRODUCT_ID		0xaef4
#define UNKNOW_MODEM_5_VENDOR_ID		0x20b9
#define UNKNOW_MODEM_5_PRODUCT_ID		0x1682

#define NEW_MODEM_0_VENDOR_ID			0x1782
#define NEW_MODEM_0_PRODUCT_ID			0x3d01

#define HEXING_MODEM_0_VENDOR_ID		0xd324
#define HEXING_MODEM_0_PRODUCT_ID		0x9002

#define XUNYOU_MODEM_0_VENDOR_ID                0x04cc
#define XUNYOU_MODEM_0_PRODUCT_ID               0x2259

#define UNKNOW_MODEM_6_VENDOR_ID		0x1e0e
#define UNKNOW_MODEM_6_PRODUCT_ID		0xdeff
#define UNKNOW_MODEM_7_VENDOR_ID		0x1199
#define UNKNOW_MODEM_7_PRODUCT_ID		0x6812
#define UNKNOW_MODEM_8_PRODUCT_ID		0x0120
#define UNKNOW_MODEM_9_VENDOR_ID		0x21f5
#define UNKNOW_MODEM_9_PRODUCT_ID		0x2008
#define UNKNOW_MODEM_10_VENDOR_ID		0x028a
#define UNKNOW_MODEM_10_PRODUCT_ID		0x1006
#define UNKNOW_MODEM_11_VENDOR_ID		0x04e8
#define UNKNOW_MODEM_11_PRODUCT_ID		0x6772
#define UNKNOW_MODEM_12_VENDOR_ID		0x0930
#define UNKNOW_MODEM_12_PRODUCT_ID		0x1302

static struct usb_device_id option_ids[] = {
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COLT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_QUAD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_QUAD_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_QUAD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_QUAD_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA_BUS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_VIPER) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_VIPER_BUS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GT_MAX_READY) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GT_MAX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_NETWORK_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_NETWORK_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_NETWORK_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_KOI_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_KOI_NETWORK) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_SCORPION_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_SCORPION_NETWORK) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_NETWORK) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_LITE) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_NETWORK_LITE) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_NETWORK_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_NETWORK_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_KOI_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_KOI_NETWORK) },
	{ USB_DEVICE(OPTION_VENDOR_ID, 0xd055) },//IC0N505 zhubo add 2010.03.19
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GTM380) },
	
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V640) }, /* Novatel Merlin V640/XV620 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V620) }, /* Novatel Merlin V620/S620 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V740) }, /* Novatel Merlin EX720/V740/X720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V720) }, /* Novatel Merlin V720/S720/PC720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U730) }, /* Novatel U730/U740 (VF version) */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U740) }, /* Novatel U740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U870) }, /* Novatel U870 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_XU870) }, /* Novatel Merlin XU870 HSDPA/3G */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_X950D) }, /* Novatel X950D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EV620) }, /* Novatel EV620/ES620 CDMA/EV-DO */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_ES720) }, /* Novatel ES620/ES720/U720/USB720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_E725) }, /* Novatel E725/E726 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_ES620) }, /* Novatel Merlin ES620 SM Bus */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU730) }, /* Novatel EU730 and Vodafone EU740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU740) }, /* Novatel non-Vodafone EU740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU870D) }, /* Novatel EU850D/EU860D/EU870D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_MC950D) }, /* Novatel MC930D/MC950D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_MC727) }, /* Novatel MC727/U727/USB727 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_1) }, /* Novatel EVDO product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_1) }, /* Novatel HSPA product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EMBEDDED_1) }, /* Novatel Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_GLOBAL_1) }, /* Novatel Global product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_2) }, /* Novatel EVDO product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_2) }, /* Novatel HSPA product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EMBEDDED_2) }, /* Novatel Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_GLOBAL_2) }, /* Novatel Global product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_MC930D) },
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_6002) },												/*Novatel MC760 3G  zhubo add 2010.03.08*/

	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E169G) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E220) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_770W) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_EC121) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_EC168C) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E180) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1692) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_K3675) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_K4505) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_EC1260) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1820) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1690) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1612) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1752) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_K4505) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_EM820W) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E303) },

	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H01) },
	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H01A) },
	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H02) },

	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_E100A) },
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_500A) },
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_620UW) },
	
	{ USB_DEVICE(AXESSTEL_VENDOR_ID, AXESSTEL_PRODUCT_MV110H) },

//	{ USB_DEVICE(ONDA_VENDOR_ID, ONDA_PRODUCT_MSA501HS) },
	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA1) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA2) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZET_PRODUCT_TDMU350) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA15) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA16) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA17) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA18) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA19) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA31) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA33) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA37) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA39) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA42) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA48) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_AC580) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_K3565) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_AC560) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_AC560_OTHER) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA51) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA6535) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MF633R) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_K3520) },	
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_HSPA57) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_EVDO_TECH) },					
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_CDMA_TECH) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTEMT_PRODUCT_AC2710) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_EVDO_2736) }, 
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_EVDO_2726) }, 
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_WCDWA_K3765Z) }, 
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MC2716) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_A356) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_3812) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MF200) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MF627) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MU301) },

	{ USB_DEVICE(QUALCOMM_VENDOR_ID, QUALCOMM_PRODUCT_PHu9)}, /* James on 2009-02-25 */
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x6000)}, /* ZTE EVDO AC8700 */
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x6613)}, /* Onda H600/ZTE MF330 */
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x6280)}, /* Onda H600/ZTE MF330 */
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x0015)},
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x1000)},
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x00a0)},
	{ USB_DEVICE(QUALCOMM_VENDOR_ID1, QUALCOMM_PRODUCT_UNKNOW2)},
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, QUALCOMM_PRODUCT_9003)},

	{ USB_DEVICE(KYOCERA_VENDOR_ID, KYOCERA_PRODUCT_KPC650) },
	{ USB_DEVICE(KYOCERA_VENDOR_ID, KYOCERA_PRODUCT_KPC680) },

	{ USB_DEVICE(MAXON_VENDOR_ID, 0x6280) }, /* BP3-USB & BP3-EXT HSDPA */
	{ USB_DEVICE(TELIT_VENDOR_ID, TELIT_PRODUCT_UC864E) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, SIMCOM_PRODUCT_1) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, SIMCOM_PRODUCT_2) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, SIMCOM_PRODUCT_3) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, TD6311_PRODUCT) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, TD_LIANXIN_PRODUCT) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, TD_LIANXIN_PRODUCT_5740) },
	{ USB_DEVICE(SIMCOM_VENDOR_ID, TD_PRODUCT_5731) },
	{ USB_DEVICE(ChinaTelcom_VENDOR_ID, ChinaTelcom_PRODUCT) },

	{ USB_DEVICE(LQ_VENDOR_ID, LQ_PRODUCT_6061) },
	{ USB_DEVICE(LQ_VENDOR_ID, 0x6000) },
	{ USB_DEVICE(LQ_VENDOR_ID, LQ_PRODUCT_9603) },
	{ USB_DEVICE(LQ_VENDOR_ID, LQ_PRODUCT_9E00) },
	{ USB_DEVICE(RWAY_VENDOR_ID, RWAY_PRODUCT_ID) },
	{ USB_DEVICE(EPHONE_VENDOR_ID, EPHONE_PRODUCT) },
	{ USB_DEVICE(UNDEFINE_MODEM_0_VENDOR_ID, UNDEFINE_MODEM_0_PRODUCT_ID) },
	{ USB_DEVICE(UNDEFINE_MODEM_0_VENDOR_ID, DLINK_MODEM_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_1_VENDOR_ID, UNKNOW_MODEM_1_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_1_VENDOR_ID, UNKNOW_MODEM_20_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_1_VENDOR_ID, UNKNOW_MODEM_PRODUCT_ID_20) },
	{ USB_DEVICE(NOKIA_MODEM_0_VENDOR_ID, NOKIA_MODEM_0_PRODUCT_ID) },
	{ USB_DEVICE(SIERRA_VENDOR_306_ID, SIERRA_PRODUCT_306_ID) },
	{ USB_DEVICE(SIERRA_VENDOR_306_ID, SIERRA_PRODUCT_MC8780_32ID) },
	{ USB_DEVICE(SIERRA_VENDOR_306_ID, SIERRA_PRODUCT_MC8785_ID) },
	{ USB_DEVICE(ALCATEL_VENDOR_X200_ID, ALCATEL_PRODUCT_X200_ID) },
	
	{ USB_DEVICE(PHI_VENDOR_TALK_ID, PHI_PRODUCT_TALK_ID) },
	
	{ USB_DEVICE(MOTO_VENDOR_ID, MOTO_PRODUCT_ID) },
	
	{ USB_DEVICE(ZYDAS_VENDOR_ID, ZYDAS_PRODUCT_ID_1) },
	{ USB_DEVICE(ZYDAS_VENDOR_ID, ZYDAS_PRODUCT_ID_2) },
	
	{ USB_DEVICE(INFOCERT_VENDOR_ID, INFOCERT_PRODUCT_ID) },
	
	{ USB_DEVICE(UT_UM185E_VENDOR, UT_UM185E_PRODUCT) },
	{ USB_DEVICE(SPRO_VENDOR_ID, SPRO_PRODUCT_ID) },
        { USB_DEVICE(SPRO_VENDOR_ID, SPRO_PRODUCT_686_ID) },


	{ USB_DEVICE(UNKNOW_MODEM_2_VENDOR_ID, UNKNOW_MODEM_2_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_2_VENDOR_ID, VIA_MODEM_2_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_2_VENDOR_ID, TENDA_MODEM_PORDUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_3_VENDOR_ID, UNKNOW_MODEM_3_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_4_VENDOR_ID, UNKNOW_MODEM_4_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_5_VENDOR_ID, UNKNOW_MODEM_5_PRODUCT_ID) },
	{ USB_DEVICE(NEW_MODEM_0_VENDOR_ID, NEW_MODEM_0_PRODUCT_ID) },
	{ USB_DEVICE(HEXING_MODEM_0_VENDOR_ID, HEXING_MODEM_0_PRODUCT_ID) },
        { USB_DEVICE(XUNYOU_MODEM_0_VENDOR_ID, XUNYOU_MODEM_0_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_6_VENDOR_ID, UNKNOW_MODEM_6_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_7_VENDOR_ID, UNKNOW_MODEM_7_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_7_VENDOR_ID, UNKNOW_MODEM_8_PRODUCT_ID) },
	{ USB_DEVICE(0x0685, 0x6001) },
	{ USB_DEVICE(UNKNOW_MODEM_9_VENDOR_ID, UNKNOW_MODEM_9_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_10_VENDOR_ID, UNKNOW_MODEM_10_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_11_VENDOR_ID, UNKNOW_MODEM_11_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_12_VENDOR_ID, UNKNOW_MODEM_12_PRODUCT_ID) },
	{ USB_DEVICE(UNKNOW_MODEM_105_VENDOR_ID, UNKNOW_MODEM_105_PRODUCT_ID) },
	{ USB_DEVICE(0x20a6, 0x1105) },
	{ } /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, option_ids);

static struct usb_driver option_driver = {
	.name       = "option",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = option_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver option_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"option1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &option_driver,
	.id_table          = option_ids,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = option_open,
	.close             = option_close,
	.write             = option_write,
	.write_room        = option_write_room,
	.chars_in_buffer   = option_chars_in_buffer,
	.throttle          = option_rx_throttle,
	.unthrottle        = option_rx_unthrottle,
	.ioctl             = option_ioctl,
	.set_termios       = option_set_termios,
	.break_ctl         = option_break_ctl,
	.tiocmget          = option_tiocmget,
	.tiocmset          = option_tiocmset,
	.attach            = option_startup,
	.shutdown          = option_shutdown,
	.read_int_callback = option_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct option_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	char in_buffer[N_IN_URB][IN_BUFLEN];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	char out_buffer[N_OUT_URB][OUT_BUFLEN];

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init option_init(void)
{
	int retval;
	retval = usb_serial_register(&option_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&option_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&option_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit option_exit(void)
{
	usb_deregister (&option_driver);
	usb_serial_deregister (&option_1port_device);
}

module_init(option_init);
module_exit(option_exit);

static void option_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void option_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void option_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void option_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __FUNCTION__);

	option_send_setup(port);
}

static int option_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct option_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int option_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct option_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return option_send_setup(port);
}

static int option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int option_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct option_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (this_urb->status == -EINPROGRESS) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void option_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

	if (urb->status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __FUNCTION__, urb->status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && urb->status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __FUNCTION__, err);
		}
	}
	return;
}

static void option_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;

	usb_serial_port_softint(port);
}

static void option_instat_callback(struct urb *urb)
{
	int err;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __FUNCTION__);
	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (urb->status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __FUNCTION__, urb->status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (urb->status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__FUNCTION__, err);
	}
}

static int option_write_room(struct usb_serial_port *port)
{
	struct option_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status != -EINPROGRESS)
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_chars_in_buffer(struct usb_serial_port *port)
{
	struct option_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status == -EINPROGRESS)
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_open(struct usb_serial_port *port, struct file *filp)
{
	struct option_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	option_send_setup(port);

	return (0);
}

static inline void stop_urb(struct urb *urb)
{
	if (urb && urb->status == -EINPROGRESS)
		usb_kill_urb(urb);
}

static void option_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		option_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			stop_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			stop_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by option_setup_urbs */
static struct urb *option_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void option_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = option_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, option_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = option_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, option_outdat_callback);
		}
	}
}

static int option_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int option_startup(struct usb_serial *serial)
{
	int i, err;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for option_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}

	option_setup_urbs(serial);

	return (0);
}

static void option_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			stop_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			stop_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

