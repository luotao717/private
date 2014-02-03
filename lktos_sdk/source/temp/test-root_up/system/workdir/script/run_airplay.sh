#!/bin/sh


var_apssid=`nvram_get 2860 SSID1`

sleep 1
airplay -b 256 --apname=$var_apssid &