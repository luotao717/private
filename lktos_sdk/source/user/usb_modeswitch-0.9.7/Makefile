LIBDIR = 
CFLAGS =
LDLIBS =
EXEC = usb_modeswitch
SRCS = usb_modeswitch.c descriptors.c  error.c  linux.c  usb.c

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) usb_modeswitch.conf /etc/
clean:
	-rm -f $(EXEC) *.o
