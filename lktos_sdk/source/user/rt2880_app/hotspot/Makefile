LDFLAGS += -lpthread
HS_EXEC = hs
HSCTRL_EXEC = hsctrl

#PLATFORM = x86
PLATFORM = mipsel

ifeq ($(PLATFORM),x86)
CROSS_COMPILE=""
endif

ifeq ($(PLATFORM),mipsel)
CROSS_COMPILE = mipsel-linux-
endif

CFLAGS = -O2 -Wall -g
CC = $(CROSS_COMPILE)gcc

HS_OBJS = hotspot.o driver_wext.o netlink.o eloop.o os_internal.o main.o util.o ctrl_iface_unix.o debug.o

HSCTRL_OBJS = hotspot_ctrl/hotspot_cli.o hotspot_ctrl/hotspot_ctrl.o os_internal.o debug.o

all: $(HS_EXEC) $(HSCTRL_EXEC)

$(HS_EXEC): $(HS_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(HS_EXEC) $(HS_OBJS)

$(HSCTRL_EXEC): $(HSCTRL_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(HSCTRL_EXEC) $(HSCTRL_OBJS)

romfs_dir = $(ROOTDIR)/romfs

romfs_dirs = etc_ro/wifi
	
romfs:
	[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)
	for i in $(romfs_dirs); do \
		[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)/$$i; \
	done 
	$(ROMFSINST) /bin/$(HS_EXEC)
	$(ROMFSINST) /bin/$(HSCTRL_EXEC)
	$(ROMFSINST) /sbin/hotspot.sh
	$(ROMFSINST) /sbin/hotspot_ap.conf
	$(ROMFSINST) /sbin/hotspot_ap_ra0.conf
	$(ROMFSINST) /sbin/hotspot_ap_ra1.conf
	$(ROMFSINST) /sbin/hotspot_ap_ra2.conf
	$(ROMFSINST) /sbin/hotspot_ap_ra3.conf

clean:
	rm -f *.o hotspot_ctrl/*.o $(HS_EXEC) $(HSCTRL_EXEC)
