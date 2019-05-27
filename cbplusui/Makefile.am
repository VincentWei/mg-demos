AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS=-I../common/include/ -I../fakejbus/ -Imisc/

SUBDIRS=res misc apps

noinst_PROGRAMS=cbplusui

APP_LIBS = \
    apps/libapps.la \
    misc/libmisc.la \
    ../fakejbus/libjbus.la \
    ../common/src/libcommon.la

cbplusui_SOURCES = \
    resource.h \
    main.cc

cbplusui_LDADD = $(APP_LIBS) -lefence

EXTRA_DIST = README.md

#install-data-local:
#	$(INSTALL_DATA) MiniGUI.cfg $(prefix)/etc/MiniGUI.cfg
#	$(INSTALL_DATA) mgncs.cfg $(prefix)/etc/mgncs.cfg
#	$(INSTALL_DATA) mgncs4touch.cfg $(prefix)/etc/mgncs4touch.cfg
#	$(INSTALL_DATA) cbplusui.cfg $(prefix)/etc/cbplusui.cfg
