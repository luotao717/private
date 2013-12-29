#!/bin/sh

cd /system/workdir
. ./configenv.sh

ifconfig apcli0 up
var_apclien=`nvram_get 2860 ApCliEnable`
var_ssid=`nvram_get 2860 ApCliSsid`
var_cmd="iwpriv apcli0 set ApCliSsid='${var_ssid}'"
var_auth=`nvram_get 2860 ApCliAuthMode`
var_encry=`nvram_get 2860 ApCliEncrypType`
var_pass=`nvram_get 2860 ApCliWPAPSK`
var_channel=`nvram_get 2860 ApCliChannel`
var_apssid=`nvram_get 2860 SSID1`
var_wepkeyindex=`nvram_get 2860 ApCliWepKeyIndex`
var_wepkey1=`nvram_get 2860 ApCliKey1`
var_wepkey2=`nvram_get 2860 ApCliKey2`
var_wepkey3=`nvram_get 2860 ApCliKey3`
var_wepkey4=`nvram_get 2860 ApCliKey4`
if [ "$var_apclien" = "1" ]; then
	iwpriv apcli0 set ApCliEnable=0
	iwpriv apcli0 set ApCliAuthMode=$var_auth
	iwpriv apcli0 set ApCliEncrypType=$var_encry
	#iwpriv apcli0 set ApCliSsid=${var_ssid1}
	sh -c "${var_cmd}"
	if [ "$var_encry" = "WEP" ]; then
		if [ "$var_wepkeyindex" = "1" ]; then
			iwpriv apcli0 set ApCliKey1=$var_wepkey1
		fi
		if [ "$var_wepkeyindex" = "2" ]; then
			iwpriv apcli0 set ApCliKey2=$var_wepkey2
		fi
		if [ "$var_wepkeyindex" = "3" ]; then
			iwpriv apcli0 set ApCliKey3=$var_wepkey3
		fi
		if [ "$var_wepkeyindex" = "4" ]; then
			iwpriv apcli0 set ApCliKey4=$var_wepkey4
		fi
	else
		iwpriv apcli0 set ApCliWPAPSK=$var_pass
	fi
	iwpriv apcli0 set ApCliEnable=1
	iwpriv ra0 set Channel=$var_channel
fi

brctl addif br0 apcli0

mknod /dev/muart c 250 0
sleep 3

mDNSResponder
sleep 3
var_apssid=`nvram_get 2860 SSID1`
sleep 1
shairairplay -b 256 --apname=$var_apssid &



