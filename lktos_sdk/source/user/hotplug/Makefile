#
# hotplug: linux kernel hotplug entry program
#          currently bound with NAS for now
PROGRAM_NAME:= hotplugd

CFLAGS += -I$(ROOTDIR)/lib/libnvram 
LDFLAGS += -lnvram

ifeq ("$(PRODUCT_OEM)","mastone")
CFLAGS += -DNOTDDIAL
endif

ifeq ("$(PRODUCT_OEM)","mastone_zx")
CFLAGS += -DNOTDDIAL
endif

HOTPLUGOBJS = hotplug_3gmodem.o hotplug_netlink.o

OBJS = $(HOTPLUGOBJS)

all: clean_inet $(PROGRAM_NAME)

clean_inet:
	#rm hotplug_3gmodem.o

$(PROGRAM_NAME): $(OBJS)
	#$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)
	$(STRIP) --remove-section=.note --remove-section=.comment $@

romfs:
	$(ROMFSINST) /sbin/$(PROGRAM_NAME)

clean:
	-rm -f $(PROGRAM_NAME) *.o

