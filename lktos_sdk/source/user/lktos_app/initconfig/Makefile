NVDIR = $(ROOTDIR)/lib/libnvram
CFLAGS += -I$(NVDIR) 
LDFLAGS += -lnvram
LIBNVRAM = $(ROOTDIR)/lib/libnvram/libnvram-0.9.28.so
EXEC = lktos_networkconfig lktos_wificonfig lktos_initconfig rmohcimod_daemon



all: $(EXEC)


lktos_networkconfig: lktNetworkConfig.o networkconfig_main.o

	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
lktos_wificonfig: lktWifiConfig.o wificonfig_main.o

	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
lktos_initconfig: lktInitConfig.o lktNetworkConfig.o lktWifiConfig.o initconfig_main.o

	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

rmohcimod_daemon:rmohcimod_daemon.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

lktNetworkConfig.o: $(LIBNVRAM)

networkconfig_main.o: $(LIBNVRAM)

lktWifiConfig.o: $(LIBNVRAM)

wificonfig_main.o: $(LIBNVRAM)

lktInitConfig.o: $(LIBNVRAM)

initconfig_main.o: $(LIBNVRAM)

clean:
	rm -f *.o $(EXEC)

romfs:
	for i in $(EXEC) ; do $(ROMFSINST) /bin/$$i ; done
	

