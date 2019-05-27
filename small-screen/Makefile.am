AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS=-I../common/include/ -Imisc/

SUBDIRS=res misc apps

noinst_PROGRAMS=small-screen

APP_LIBS = \
    ../common/src/libcommon.la \
    misc/libmisc.la \
    apps/libapps.la

small_screen_SOURCES = \
    resource.h \
    main.cc

small_screen_LDADD = $(APP_LIBS) -lefence

EXTRA_DIST = README.md

#install-data-local:
#	$(INSTALL_DATA) MiniGUI.cfg $(prefix)/etc/MiniGUI.cfg
#	$(INSTALL_DATA) mgncs.cfg $(prefix)/etc/mgncs.cfg
#	$(INSTALL_DATA) mgncs4touch.cfg $(prefix)/etc/mgncs4touch.cfg
#	$(INSTALL_DATA) small-screen.cfg $(prefix)/etc/small-screen.cfg
