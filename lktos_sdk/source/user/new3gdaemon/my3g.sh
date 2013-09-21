#!/bin/sh

rm -rf /var/usbmodem
rm -rf /var/config
killall -q musb
killall -q new3g

#rmmod usbcore.ko 1>/dev/null 2>&1
#rmmod cdc-acm.ko 1>/dev/null 2>&1
#rmmod ehci-hcd.ko 1>/dev/null 2>&1
#rmmod ohci-hcd.ko 1>/dev/null 2>&1
#rmmod usbserial.ko 1>/dev/null 2>&1
#rmmod option.ko 1>/dev/null 2>&1
umount /proc/bus/usb 1>/dev/null 2>&1
insmod usbserial.ko 1>/dev/null 2>&1
insmod option.ko 1>/dev/null 2>&1
mount -t usbfs none /proc/bus/usb 1>/dev/null 2>&1 

mkdir -m 777 /var/usbmodem
mkdir -m 777 -p /var/config/ppp/peers
./musb&
sleep 2
#insmod usbcore.ko 1>/dev/null 2>&1
#insmod cdc-acm.ko 1>/dev/null 2>&1
#insmod ehci-hcd.ko 1>/dev/null 2>&1
#insmod ohci-hcd.ko 1>/dev/null 2>&1

##call 3gdial
dial3gmode=`nvram_get 2860 dial3gchoicetype`
if [ "$dial3gmode" != "MANUAL" ]; then
	./new3g 0
else
	./new3g 1
fi


rmmod ehci-hcd.ko
exit 0
