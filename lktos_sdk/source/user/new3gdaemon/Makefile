
CFLAGS += -DREALM_STR='"$(REALM_STR)"' -DFIRMWARE_VERSION='"$(FIRMWARE_VERSION)"'

LDLIBS+= -lnvram
CFLAGS	+= -I$(ROOTDIR)/lib/libnvram

EXEC_1 = new3g
SRCS_1 = new3g.o

all: $(EXEC_1)

clean_inet:
	rm new3g.o

$(EXEC_1): $(SRCS_1)
	$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<


romfs:
	$(ROMFSINST) /bin/new3g
	
clean:
	-rm -f $(EXEC_1) *.o

