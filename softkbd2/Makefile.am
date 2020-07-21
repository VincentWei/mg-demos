SUBDIRS = src

AM_CPPFLAGS=-I../common/include/ -Iinclude/

noinst_PROGRAMS=softkbd

APP_LIBS = \
    src/libsoftkeyboard.la \
    ../common/src/libcommon.la

softkbd_SOURCES = \
    SignUpActivity.hh \
    SignUpActivity.cc \
    resource.h \
    main.cc

softkbd_LDADD = $(APP_LIBS)

EXTRA_DIST = README.md MiniGUI.cfg include

