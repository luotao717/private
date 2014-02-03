#!/bin/sh

USBLOGFILENAME=factory.log
USBUPSCRIPTFILENAME=usbupfirmware.sh
SDTESTFILE=/media/sdb/sd.log
SDTESTRESULT=1
USBTESTRESULT=1
GPIOTESTRESULT=1
USBUPRESULT=0


pass()
{
echo  pass!!!
echo  "#######################################################################" >>${USBLOGFILE}
echo  "##                                                                   ##" >>${USBLOGFILE}
echo  "##                 ########     ###     ######   ######              ##" >>${USBLOGFILE}
echo  "##                 ##     ##   ## ##   ##    ## ##    ##             ##" >>${USBLOGFILE}
echo  "##                 ##     ##  ##   ##  ##       ##                   ##" >>${USBLOGFILE}
echo  "##                 ########  ##     ##  ######   ######              ##" >>${USBLOGFILE}
echo  "##                 ##        #########       ##       ##             ##" >>${USBLOGFILE}
echo  "##                 ##        ##     ## ##    ## ##    ##             ##" >>${USBLOGFILE}
echo  "##                 ##        ##     ##  ######   ######              ##" >>${USBLOGFILE} 
echo  "##                                                                   ##" >>${USBLOGFILE}
echo  "#######################################################################" >>${USBLOGFILE}
}

fail()
{
echo  fail!!!
echo  "#######################################################################" >>${USBLOGFILE}
echo  "##                                                                   ##" >>${USBLOGFILE}
echo  "##                 ########     ###       ##    ##                   ##" >>${USBLOGFILE}
echo  "##                 ##          ## ##            ##                   ##" >>${USBLOGFILE}
echo  "##                 ##         ##   ##    ####   ##                   ##" >>${USBLOGFILE}
echo  "##                 ########  ##     ##    ##    ##                   ##" >>${USBLOGFILE}
echo  "##                 ##        #########    ##    ##                   ##" >>${USBLOGFILE}
echo  "##                 ##        ##     ##    ##    ##                   ##" >>${USBLOGFILE}
echo  "##                 ##        ##     ##   ####   ########             ##" >>${USBLOGFILE} 
echo  "##                                                                   ##" >>${USBLOGFILE}
echo  "#######################################################################" >>${USBLOGFILE}
}

USBdetect()
{
UDISK=`cat /proc/mounts| grep "/media/" | sed -n 's|.*\(/media/.*[[:space:]]\).*|\1|p' | sed 's/^[ \t]\+//;s/[ \t].*//'`
echo UDISK is $UDISK
}

USBtest()
{
USBTESTRESULT=0

if test "$UDISK" == ""
then
	echo no UDisk!!!
	fail
	USBTESTRESULT=0
	return
fi

for i in $UDISK
do
	USBLOGFILE=$i/${USBLOGFILENAME}
done

if test -e ${USBLOGFILE}
then
	echo ${USBLOGFILE} exist, rm it!
	rm -rf ${USBLOGFILE}
	if test -e ${USBLOGFILE}
	then
		echo rm ${USBLOGFILE} failed!
	fi
fi

echo =======================================================================	>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo USB TEST START!!!															>>${USBLOGFILE}
echo USB TEST will save this message into factory.log 							>>${USBLOGFILE}
echo USB TEST END!!!															>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}          
if test -e ${USBLOGFILE}
then
	pass                                                        					
	USBTESTRESULT=1
else
	fail
	USBTESTRESULT=0
fi		                                                        					
echo                     														>>${USBLOGFILE}          
echo                     														>>${USBLOGFILE}
echo =======================================================================	>>${USBLOGFILE}
}

SDtest()
{
if test -e ${SDTESTFILE}
then
	echo ${SDTESTFILE} exit, rm it!
	rm -rf ${SDTESTFILE}
fi
ramdnum=$RANDOM
echo ramdnum=$ramdnum                                       					>>${SDTESTFILE}
if test -e ${SDTESTFILE}
then
	temp=`grep "ramdnum" ${SDTESTFILE} | sed 's/ramdnum[ \t]*=[ \t]*//' | sed  's/\r//g'`
	if test ${temp} -eq ${ramdnum}
	then
		SDTESTRESULT=1
	else
		SDTESTRESULT=0
	fi		
else
	SDTESTRESULT=0
fi

echo =======================================================================	>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo SD TEST START!!!															>>${USBLOGFILE}
echo SD TEST END!!!								     							>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}          
if test $SDTESTRESULT -eq 0
then
	fail
else
	pass
fi		                                                        					
echo                     														>>${USBLOGFILE}          
echo                     														>>${USBLOGFILE}
echo =======================================================================	>>${USBLOGFILE}
}

GPIOtest()
{
echo =======================================================================	>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}
echo GPIO TEST START!!!															>>${USBLOGFILE}
GPIOTESTRESULT=`factory_gpio f1`
echo $GPIOTESTRESULT                                                            >>${USBLOGFILE}
echo GPIO TEST END!!!								     						>>${USBLOGFILE}
echo                     														>>${USBLOGFILE}          
GPIOTESTRESULT=`factory_gpio f1 | grep 'TestResult' | sed 's/TestResult:*/\1/g'` 
if test $GPIOTESTRESULT -eq 0
then
	fail
else
	pass
fi		                                                        					
echo                     														>>${USBLOGFILE}          
echo                     														>>${USBLOGFILE}
echo =======================================================================	>>${USBLOGFILE}
}

#setup environment
. ./evn.sh

#factory_gpio f6

#for WIFI test
ated &
pkill udhcpc
ifconfig eth2 192.168.1.10

a01controller &



USBdetect
for i in $UDISK
do
	if test -e $i/${USBUPSCRIPTFILENAME}
	then
		echo ${USBUPSCRIPTFILENAME} exist, USB update!!!
		cd $i
		pwd
		#start burn
		factory_gpio f7 &
		#sleep for a while to wait system ready
		sleep 2
		./${USBUPSCRIPTFILENAME}
		USBSCRIPT=$?
		echo return is $USBSCRIPT
		pkill factory_gpio
	    sleep 1		
		if [ "$USBSCRIPT" == "0" ]
		then
			#success
			factory_gpio f8
			USBUPRESULT=1
		else
			#fail since image wrong
			factory_gpio f5 &			
			USBUPRESULT=2
		fi
	else
		echo ${USBUPSCRIPTFILENAME} not found!
		USBUPRESUL=0
	fi
done


#have not do the USB update firmware, do the factory test
if [ "${USBUPRESULT}" == "0" ]
then

	echo start factory test!!!
	#for USB test
	USBtest
	
	#for SD test
	#SDtest

	#for GPIO test
	GPIOtest
	
	sync
	
	#for display the result
	sleep 1
	if  [ "$GPIOTESTRESULT" = "1" -a "$SDTESTRESULT" = "1" -a "$USBTESTRESULT" = "1" ]
	then
	        factory_gpio f2
	else
	        factory_gpio f5 &
	fi

	#for audio test
	factory_audio &
fi

