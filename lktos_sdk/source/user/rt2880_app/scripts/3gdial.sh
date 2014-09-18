#!/bin/sh

rm -rf /var/usbmodem
rm -rf /var/config
killall -q hotplugd
killall -q check_usb
killall -q 3gdial
#rmmod usbcore.ko 1>/dev/null 2>&1
rmmod cdc-acm.ko 1>/dev/null 2>&1
#rmmod ehci-hcd.ko 1>/dev/null 2>&1
#rmmod ohci-hcd.ko 1>/dev/null 2>&1
rmmod usbserial.ko 1>/dev/null 2>&1
rmmod option.ko 1>/dev/null 2>&1
umount /proc/bus/usb 1>/dev/null 2>&1


mkdir -m 777 /var/usbmodem
mkdir -m 777 -p /var/config/ppp/peers
hotplugd&
sleep 2
#insmod usbcore.ko 1>/dev/null 2>&1
insmod cdc-acm.ko 1>/dev/null 2>&1
#insmod ehci-hcd.ko 1>/dev/null 2>&1
#insmod ohci-hcd.ko 1>/dev/null 2>&1
insmod usbserial.ko 1>/dev/null 2>&1
insmod option.ko 1>/dev/null 2>&1
mount -t usbfs none /proc/bus/usb 1>/dev/null 2>&1 
sleep 1
check_usb

##call 3gdial
dial3gmode=`nvram_get 2860 dial3gchoicetype`

if [ "$dial3gmode" != "MANUAL" ]; then
	3gdial 0 &
else
	3gdial 1 &
fi


rmmod ehci-hcd.ko
rmohcimod_daemon &
#rmmod ohci-hcd.ko
#sleep 3
#insmod ohci-hcd.ko
#sleep 8
#rmmod ohci-hcd.ko
#sleep 3
#insmod ohci-hcd.ko
exit 0
