#CC	= gcc
#AR	= ar
#STRIP	= strip
CC	= /opt/buildroot-gcc342/bin/mipsel-linux-gcc
#AR	= mips-uclibc-ar
STRIP	= /opt/buildroot-gcc342/bin/mipsel-linux-strip
# LIBS=-L. -L/usr/local -lpthread
all:yddns
ALL_OBJS = ddns_main.o provider.o oray.o md5.o base64.o blowfish.o

OUTFILE=$(OUTDIR)/$(TARGET_ARCH)/yddns
yddns: $(ALL_OBJS)
# $(CC) -o yddns $(ALL_OBJS) $(LIBS) 
	$(CC) -o yddns $(ALL_OBJS)  
	$(STRIP) yddns
ddns_main.o: ddns_main.c 
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c ddns_main.c
provider.o:   provider.c
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c provider.c
oray.o:   oray.c
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c oray.c
md5.o:	md5.c
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c md5.c
base64.o:	base64.c
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c base64.c
blowfish.o:	blowfish.c
	$(CC) $(INCLUDE) $(FLAGS)  -O2 -c blowfish.c
    
clean:  
	rm -f yddns *.o

romfs:
	$(ROMFSINST) -e CONFIG_USER_DDNS /bin/yddns

