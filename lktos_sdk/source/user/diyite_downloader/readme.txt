配置：在main.c文件
//设置下载的时间段
#define TIME_DOWNLOAD_BEGIN   2
#define TIME_DOWNLOAD_END	4
//获取apk列表接口
#define APK_LIST_BY_MAC_URL "http://113.107.235.123/firste/boxDownload?boxMac="
//本地SD卡路径
#define SD_PATH "/media/sda1"


编译:
1. 把diyite_downloader文件夹拷贝到/home/5350/RT288x_SDK/source/user目录
2. 修改/home/5350/RT288x_SDK/source/user/Makefile文件
添加一行内容"dir_y   += diyite_downloader"
3. 修改/home/5350/RT288x_SDK/source/vendors/Ralink/RT5350/rcS文件，
添加diyite_downloader&开机启动
4. 最后回到/home/5350/RT288x_SDK/source执行make

注意：
需要使用wget，请在make menuconfig中打开wget
