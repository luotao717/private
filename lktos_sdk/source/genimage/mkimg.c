
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
	struct stat buf;
	
	printf("\r\n make image start!\r\n");
	//if(stat("./web",&buf)== -1)
	//{
	//	printf("no web \r\n");
	//	return -1;
	//}
//	if(stat("./RT2860_default_vlan",&buf)== -1)
	//{
	//	printf("no config file \r\n");
	//	return -1;
	//}
	//system("rm -rf ./test-root_up/sbin/web/");
	//system("cp -rf ./web/ ./test-root_up/sbin/");
	//system("rm -f ./test-root_up/etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
	//system("cp -f ./RT2860_default_vlan ./test-root_up/etc_ro/Wireless/RT2860AP/");
	system("./ltmkfsqs test-root_up ramdiskluoup");
	system("cp ./ltk0115a.bin ./kkkk.bin");
	system("cat ./ramdiskluoup >> ./kkkk.bin");
	system("./ltmkfir -A mips -O linux -T kernel -C lzma -a 80000000 -e 80483000 -k 1522064 -n \"Linux Kernel Image\" -d ./kkkk.bin ./ltwifiaudo_uImage");
	system("rm -f ./ramdiskluoup");
	system("rm -f ./kkkk.bin");
	printf("\r\nsuccess the ltwifiaudo_uImage is fireware!!!\r\n");
	return 0;
}

