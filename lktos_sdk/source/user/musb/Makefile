#
# hotplug: linux kernel hotplug entry program
#          currently bound with 3G for now
PROGRAM_NAME:= musb

CFLAGS += -I$(ROOTDIR)/lib/libnvram 
LDFLAGS += -lnvram

HOTPLUGOBJS = musb.o

OBJS = $(HOTPLUGOBJS)

all: clean_inet $(PROGRAM_NAME)

clean_inet:
	#rm hotplug_3gmodem.o

$(PROGRAM_NAME): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)
	$(STRIP) --remove-section=.note --remove-section=.comment $@

romfs:
	$(ROMFSINST) /bin/$(PROGRAM_NAME)

clean:
	-rm -f $(PROGRAM_NAME) *.o

