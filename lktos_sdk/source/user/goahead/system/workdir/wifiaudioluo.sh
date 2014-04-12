#!/bin/sh

cd /system/workdir
. ./evn.sh

sleep 3


#mount JFFS2 to mtdblock8 
mount -t jffs2 /dev/mtdblock8 /mnt

cp -rf /system/workdir/misc/general.dat /tmp/
cp -rf /system/workdir/script/hosts /etc/
cp -rf /system/workdir/script/dnsmasq.conf /etc/
mknod /dev/muart c 250 0
mknod /dev/enc c 248 0
#iwevent &
sleep 3

#for melody
MELODY=`grep "melody" /system/workdir/misc/general.dat | sed 's/melody[ \t]*=[ \t]*//' | sed  's/\r//g'`
if [ "$MELODY" = "1" ]; then
echo "melody on"
mplayer /system/workdir/misc/melody.mp3 &
mplayer /system/workdir/misc/melody.mp3 &
mplayer /system/workdir/misc/melody.mp3 &
mplayer /system/workdir/misc/melody.mp3 &
mplayer /system/workdir/misc/melody.mp3 &
else
echo "melody off"
fi

mDNSResponder
var_apssid=`nvram_get 2860 SSID1`
sleep 1
airplay -b 256 --apname=$var_apssid &
sleep 2
upnprender_out &
sleep 4
a01controller &
#a01controller &
#gmediarender &
sleep 4
#upnprender_out &
dnsmasq &

#mv_guard &
#mv_netguard &
iwevent &

#m1transerver_nietech &
#m1transerver  &
#myuartsocket &
#iperf -s -i 5 -w 1M &
