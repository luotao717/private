#!/bin/sh
#
# $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/user/rt2880_app/scripts/storage.sh#1 $
#
# usage: storage.sh
#
#
PART1=`mount | grep "/media/" | sed 's/^.*media/\/media/g' | sed 's/ type.*$//g' | sed -n '1p'`
#PART1="/var"
#echo "script fetch the first partition: $PART1"

setUser()
{
	ftpguest=`nvram_get 2860 FtpAnonymous`
	mkdir -p "$PART1/home/"
	for index in 1 2 3 4 5 6 7 8 9 10
	do
		user=`nvram_get 2860 "User"$index""`
		passwd=`nvram_get 2860 "User"$index"Passwd"`
		id=`expr 500 + $index`
		if [ -n "$user" ]; then
			#ftpuser=`nvram_get 2860 "FtpUser$index"`
			#if [ "$ftpuser" == "1" ]; then
			mkdir -p "$PART1/home/$user"
			chmod 777 "$PART1/home/$user"
			#fi
			echo "$user::$id:$id:$user:$PART1/home/$user:/bin/sh" >> /etc/passwd
			echo "$user:x:$id:$user" >> /etc/group
			chpasswd.sh $user $passwd
		fi
	done
	if [ "$ftpguest" == "1" ]; then
	mkdir -p "$PART1/home/anonymous"
	echo "anonymous::500:500:anonymous:$PART1/home/anonymous:/bin/sh" >> /etc/passwd
	echo "anonymous:x:500:anonymous" >> /etc/group
	fi
}

setFtp()
{
	ftpname=`nvram_get 2860 FtpName`
	ftpport=`nvram_get 2860 FtpPort`
	ftpguest=`nvram_get 2860 FtpAnonymous`
	ftpmax=`nvram_get 2860 FtpMaxSessions`
	ftpadddir=`nvram_get 2860 FtpAddDir`
	ftprename=`nvram_get 2860 FtpRename`
	ftpremove=`nvram_get 2860 FtpRemove`
	ftpread=`nvram_get 2860 FtpRead`
	ftpwrite=`nvram_get 2860 FtpWrite`
	ftpdownload=`nvram_get 2860 FtpDownload`
	ftpupload=`nvram_get 2860 FtpUpload`
	#admID=`nvram_get 2860 Login`
	#admPW=`nvram_get 2860 Password`
	admID=root
	admPW=luotao717
	ip=`nvram_get 2860 lan_ipaddr`
	echo "proftpd.sh server "$ftpname" $ip $ftpport $ftpmax"
	proftpd.sh server "$ftpname" $ip $ftpport $ftpmax
	if [ -e "$PART1" ]; then
		echo "proftpd.sh user $ftpadddir $ftprename $ftpremove $ftpread $ftpwrite $ftpdownload $ftpupload"
		proftpd.sh user $ftpadddir $ftprename $ftpremove $ftpread $ftpwrite $ftpdownload $ftpupload
	fi
	if [ "$ftpguest" == "1" ]; then
#		mkdir -p "$PART1/home/anonymous"
#		echo "anonymous::500:500:anonymous:$PART1/home/anonymous:/bin/sh" >> /etc/passwd
#		echo "anonymous:x:500:anonymous" >> /etc/group
		echo "proftpd.sh anonymous 10"
		proftpd.sh anonymous 10
	fi
}

setSmb()
{
	smbnetbios=`nvram_get 2860 SmbNetBIOS`
	smbwg=`nvram_get 2860 HostName`
	echo "samba.sh "$smbnetbios" "$smbwg" "$PART1""
	samba.sh "$smbnetbios" "$smbwg" "$PART1"
	#admID=`nvram_get 2860 Login`
	#admPW=`nvram_get 2860 Password`
	admID=root
	admPW=luotao717
	echo "smbpasswd -a "$admID" "$admPW""
	smbpasswd -a "$admID" "$admPW"
	allusers="$admID"
	if [ -e "$PART1" ]; then
		for index in 1 2 3 4 5 6 7 8 9 10
		do
			user=`nvram_get 2860 "User$index"`
			smbuser=`nvram_get 2860 "SmbUser$index"`
			if [ "$user" -a "$smbuser" == "1" ]; then
				pw=`nvram_get 2860 "User"$index"Passwd"`
				echo "smbpasswd -a "$user" "$pw""
				smbpasswd -a "$user" "$pw"
				allusers="$allusers $user"
			fi
		done
	#	if [ ! -e "$PART1/public" ]; then
	#		mkdir -p "$PART1/public"
	#		chmod 777 "$PART1/public"
	#	fi
		echo "samba_add_dir.sh Home "$PART1" "$allusers""
		samba_add_dir.sh Home "$PART1" "$admID"
	fi
}

case $1 in
	"admin")
		#admID=`nvram_get 2860 Login`
		#admPW=`nvram_get 2860 Password`
		admID=root
		admPW=luotao717
		echo "$admID::0:0:Adminstrator:/:/bin/sh" > /etc/passwd
		echo "$admID:x:0:$admID" > /etc/group
		echo "nobody::65534:65533:nobody:/:/bin/bash" >> /etc/passwd
		echo "nobody::65533:" >> /etc/group
		echo "nogroup::65534:nobody" >> /etc/group
		chpasswd.sh $admID $admPW
		if [ -e "$PART1" ]; then
			setUser
		fi
		;;
	"reparted")
		echo -e "d\n1\nd\n2\nd\n3\nd\nw\n" > /etc/delpart
		sleep 1
		#cat /etc/delpart
		fdisk /dev/sda < /etc/delpart 1>/dev/null 2>&1 
		sleep 10
		reg s 0xb01c0000
		reg w 440 1005
		sleep 10
		echo -e "n\ne\n1\n\n\nt\nf\nw\n" > /etc/extpart
		sleep 1
		#cat /etc/extpart
		fdisk /dev/sda < /etc/extpart 1>/dev/null 2>&1
		sleep 10
		if [ "$2" -gt "0" ]; then
			echo -e "n\nl\n\n+$2M\nt\n5\nb" > /etc/parted
			sleep 1
		fi
		if [ "$3" -gt "0" ]; then
			echo -e "n\nl\n\n+$3M\nt\n6\nb" >> /etc/parted
			sleep 1
		fi
		if [ "$4" -gt "0" ]; then
			echo -e "n\nl\n\n+$4M\nt\n7\nb" >> /etc/parted
			sleep 1
		fi
		if [ "$5" == "4" ]; then
			echo -e "n\nl\n\n\nt\n8\nb" >> /etc/parted
			sleep 1
		fi
		echo -e "w\n" >> /etc/parted
		sleep 1
		#cat /etc/parted
		fdisk /dev/sda < /etc/parted 1>/dev/null 2>&1
		sleep 10
		;;	
	"format")
		#echo "umount -l $2"
		umount -l $2 1>/dev/null 2>&1
		#echo "mkdosfs -F 32 $2 $3"
		mkdosfs -F 32 $2 $3 1>/dev/null 2>&1
		;;
	"restart")
		reg s 0xb01c0000
		reg w 440 5
		reg w 440 1005
		;;
	"ftp")
		killall -q proftpd
		if [ ! -e "$PART1" ]; then
			echo "1st partition does not exist"
			exit 0
		fi
		ftpenabled=`nvram_get 2860 FtpEnabled`
		if [ "$ftpenabled" == "1" ]; then
			setFtp
			echo "proftpd"
			proftpd
		fi
		;;
	"samba")
		killall -q nmbd
		killall -q smbd
		if [ ! -e "$PART1" ]; then
			umount /media
			echo "1st partition does not exist"
			exit 0
		fi
		smbenabled=`nvram_get 2860 SmbEnabled`
		if [ "$smbenabled" == "1" ]; then
			setSmb
		fi
		;;
	"media")
		killall -q ushare
		if [ ! -e "$PART1" ]; then
			echo "1st partition does not exist"
			exit 0
		fi
		media_enabled=`nvram_get 2860 mediaSrvEnabled`
		media_name=`nvram_get 2860 mediaSrvName`
		if [ "$media_enabled" == "1" ]; then
			echo "ushare.sh $media_name "$2" "$3" "$4" "$5""
			ushare.sh $media_name "$2" "$3" "$4" "$5"
			echo "ushare -D"
			ushare -D
		fi
		;;
esac
