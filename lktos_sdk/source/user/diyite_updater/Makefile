EXE = diyite_updater
OBJS = main.o crc32.o http_download.o http.o
ZLIB_H  = $(ROOTDIR)/lib/zlib

CFLAGS += -O0 -I$(ROOTDIR)/$(LINUXDIR)

include ../../config/.config

all: $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

main.o: main.c upload.h
	$(CC) $(CFLAGS) -c main.c -o $@

crc32.o: crc32.c
	$(CC) $(CFLAGS) -DUSE_HOSTCC -I$(ZLIB_H) -c crc32.c -o $@
	
http_download.o: http_download.c
	$(CC) $(CFLAGS) -c http_download.c -o $@

http.o: http.c
	$(CC) $(CFLAGS) -c http.c -o $@


romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE)  *.o

