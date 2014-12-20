配置：在main.c文件
//当前固件版本
#define SYSTEM_FIIRMWARE_VERSION	0
//查询新版本接口
#define GET_FIRMWARE_VERSION_URL "http://113.107.235.123:8088/cgi-bin/autoupgrade.cgi"
//查询间隔 默认30分钟
#define TIME_SLEEP_INTERVAL		60*30
//本地保存路径
char g_Default_path[256] = "/media/sda1";


编译:
1. 把diyite_updater文件夹拷贝到/home/5350/RT288x_SDK/source/user目录
2. 修改/home/5350/RT288x_SDK/source/user/Makefile文件
添加一行内容"dir_y   += diyite_updater"
3. 修改/home/5350/RT288x_SDK/source/vendors/Ralink/RT5350/rcS文件，
添加diyite_updater&开机启动
4. 最后回到/home/5350/RT288x_SDK/source执行make

