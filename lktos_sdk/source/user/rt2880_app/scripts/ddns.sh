#!/bin/sh
#
# $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/user/rt2880_app/scripts/ddns.sh#1 $
#
# usage: ddns.sh
#

ddns_enable=`nvram_get 2860 DDNSEnabled`
srv=`nvram_get 2860 DDNSProvider`
ddns=`nvram_get 2860 DDNS`
u=`nvram_get 2860 DDNSAccount`
pw=`nvram_get 2860 DDNSPassword`
wanmode=`nvram_get 2860 wanConnectionMode`

killall -q yddns

if [ "$ddns_enable" = "" ]; then
	exit 0
fi
if [ "$ddns" = "" -o "$u" = "" -o "$pw" = "" ]; then
	exit 0
fi

# debug
echo "enable=$ddns_enable"
echo "srv=$srv"
echo "ddns=$ddns"
echo "u=$u"
echo "pw=$pw"

#add by chenfei for get wan ip
if [ "$wanmode" = "PPPOE" ]; then
	s1=`ifconfig ppp0 | grep "inet addr"`
	while [ "$s1" = "" ]
	do
		s1=`ifconfig ppp0 | grep "inet addr"`
		sleep 2
	done

	s2=`echo $s1 | cut -f2 -d:`
	wan_ip_addr=`echo $s2 | cut -f1 -d " "`
elif [ "$wanmode" = "DHCP" ]; then
	s3=`ifconfig eth2.2 | grep "inet addr"`
	while [ "$s3" = "" ]
	do
		s3=`ifconfig eth2.2 | grep "inet addr"`
		sleep 2
	done

	s4=`echo $s3 | cut -f2 -d:`
	wan_ip_addr=`echo $s4 | cut -f1 -d " "`
elif [ "$wanmode" = "STATIC" ]; then
	s3=`ifconfig eth2.2 | grep "inet addr"`
	while [ "$s3" = "" ]
	do
		s3=`ifconfig eth2.2 | grep "inet addr"`
		sleep 2
	done

	s4=`echo $s3 | cut -f2 -d:`
	wan_ip_addr=`echo $s4 | cut -f1 -d " "`
else
	wan_ip_addr=`echo 172.1.1.1`
fi

if [ "$srv" = "dyndns.org" ]; then
	yddns -s dyndns $ddns $u $pw $wan_ip_addr
elif [ "$srv" = "no-ip.com" ]; then
	yddns -s noip $ddns $u $pw $wan_ip_addr
elif [ "$srv" = "3322.org" ]; then
	yddns -s qdns $ddns $u $pw $wan_ip_addr
elif [ "$srv" = "tzo.org" ]; then
	yddns -s tzo $ddns $u $pw $wan_ip_addr
else
	echo "$0: unknown DDNS provider: $srv"
	exit 1
fi

