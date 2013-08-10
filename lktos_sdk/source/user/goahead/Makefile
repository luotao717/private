.EXPORT_ALL_VARIABLES:

ifeq ("$(CONFIG_USER_MATRIXSSL)", "y")
DIRS = ../matrixssl-1.8.3 src cgi-src
else
DIRS = src cgi-src
endif

ROOT_DIRECTORY = /etc_ro/web
ifeq ("$(CONFIG_RT_FIRST_CARD)", "2860")
FIRST_CARD=RT$(CONFIG_RT_FIRST_CARD)V2
else
ifeq ("$(CONFIG_RT_FIRST_CARD)", "7610")
FIRST_CARD=MT$(CONFIG_RT_FIRST_CARD)
else
endif
FIRST_CARD=RT$(CONFIG_RT_FIRST_CARD)
endif
FIRST_WDS=CONFIG_$(FIRST_CARD)_AP_WDS
SECOND_WDS=CONFIG_$(CONFIG_RT_SECOND_CARD)_AP_WDS
FIRST_APCLI=CONFIG_$(FIRST_CARD)_AP_APCLI

all:
#if some product have different web src code,modify here
ifeq ("$(PRODUCT_OEM)","")
	ln -sf web_original web
	ln -sf src_original src
	ln -sf cgi-src_original cgi-src
else
ifeq ("$(PRODUCT_OEM)","original")
	ln -sf src_$(PRODUCT_OEM) src
	ln -sf cgi-src_$(PRODUCT_OEM) cgi-src
else
	ln -sf src_tongyong src
	ln -sf cgi-src_tongyong cgi-src
endif
	ln -sf web_$(PRODUCT_OEM) web
endif
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

romfs:
	$(ROMFSINST) $(ROOT_DIRECTORY)
ifeq ("$(CONFIG_USER_PPPOECD)", "y")
	mv $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan_simple.asp $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan.asp
else
	rm -rf $(ROMFSDIR)$(ROOT_DIRECTORY)/internet/wan_simple.asp
endif
	for i in $(DIRS) ; do make -C $$i $@ ; done
clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

