EXE = ppp_daemon
OBJS = ppp_daemon.o

#stdlibc++ known issue: 
#If your program  needs libstdc++, please turn off code optimization
CFLAGS += -O0
CFLAGS += -I$(ROOTDIR)/lib/libnvram
LDLIBS = -lnvram

all: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LDFLAGS) $(LDLIBS)

.o:
	$(CC) $(CFLAGS) -c $< 

romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE)  *.o

