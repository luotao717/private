EXEC = gpio
EXEC_TEST = reloadfac
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char

all: $(EXEC) $(EXEC_TEST)

$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)

$(EXEC_TEST): $(EXEC_TEST).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) $(EXEC_TEST) *.elf *.gdb *.o

