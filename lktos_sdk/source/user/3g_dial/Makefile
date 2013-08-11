ifeq ($(USE_TD_MODEM), 0)
CFLAGS += -DNOCURNET
endif

ifeq ("$(PRODUCT_OEM)","chinamobile")
CFLAGS += -DTESTMOBILE
endif

ifeq ("$(PRODUCT_OEM)","totolink")
CFLAGS += -DNOSET_WLAN_MAC
endif

ifeq ("$(PRODUCT_OEM)","emp")
CFLAGS += -DNOSET_WLAN_MAC
endif

ifeq ("$(PRODUCT_OEM)","n3gr")
CFLAGS += -DNOSET_WLAN_MAC
endif

ifeq ("$(PRODUCT_OEM)","totolink")
CFLAGS += -DSET_PASSWORD
endif

ifeq ("$(PRODUCT_OEM)","emp")
CFLAGS += -DSET_PASSWORD
endif

ifeq ("$(PRODUCT_OEM)","n3gr")
CFLAGS += -DSET_PASSWORD
endif

ifeq ($(MODULE_TYPE), 1)
CFLAGS += -DFOREIGN_MODULE
endif

ifeq ($(TEST_SIGNAL), 1)
CFLAGS += -DTEST_SIGNAL
endif

ifeq ($(PCM_VOICE), 1)
 ifeq ($(FOR_LC5740), 1)
 CFLAGS += -DFOR_LC5740
 endif

 ifeq ($(FOR_AD3812), 1)
 CFLAGS += -DFOR_AD3812
 endif

 ifeq ($(FOR_T3G5300), 1)
 CFLAGS += -DFOR_T3G5300
 endif
endif

CFLAGS += -DREALM_STR='"$(REALM_STR)"' -DFIRMWARE_VERSION='"$(FIRMWARE_VERSION)"'

LDLIBS+= -lnvram
CFLAGS	+= -I$(ROOTDIR)/lib/libnvram
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char/pcm
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/rt_rdm -I$(ROOTDIR)/$(LINUXDIR)/include

EXEC_1 = 3gdial
SRCS_1 = 3gdial.o gpio.o

all: clean_inet $(EXEC_1) loopt checkat cmcc cmcc_link netcmd netcall wanflow netwan get_mac check_usb #wphone

clean_inet:
#	rm 3gdial.o checkat.o loopt.o netcmd.o netcall.o uas.o wanflow.o netwan.o get_mac.o check_usb.o

$(EXEC_1): $(SRCS_1)
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

loopt: loopt.o gpio.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

checkat: checkat.o gpio.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

cmcc:	cmcc.o gpio.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

cmcc_link:	cmcc_link.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

netcmd: netcmd.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

wanflow: wanflow.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

netcall: netcall.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

netwan: netwan.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

get_mac: get_mac.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

check_usb: check_usb.o
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

#wphone: uas.o g711.o
#	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ -losip2 -losipparser2 -leXosip2 -lpthread
#	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ -L. -losip2 -losipparser2 -leXosip2 -lpthread
#	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ -L. -leXosip2 -losipparser2 -losip2 -lortp -lpthread $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<


romfs:
	$(ROMFSINST) /bin/3gdial
	$(ROMFSINST) /bin/loopt
	$(ROMFSINST) /bin/ppp-on
	$(ROMFSINST) /bin/checkat
	$(ROMFSINST) /bin/cmcc
	$(ROMFSINST) /bin/cmcc_link
#	$(ROMFSINST) /bin/netcmd
#	$(ROMFSINST) /bin/wanflow
	$(ROMFSINST) /bin/netcall
#	$(ROMFSINST) /bin/wphone
	$(ROMFSINST) /bin/netwan
	$(ROMFSINST) /bin/get_mac
	$(ROMFSINST) /bin/check_usb

clean:
	-rm -f $(EXEC_1) loopt check_usb *.o

