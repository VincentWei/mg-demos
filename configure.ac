dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT(mg-demos,1.0.0)
AC_CONFIG_SRCDIR(cbplusui/main.cc)

dnl ========================================================================
dnl Need for cross-compiling
AC_CANONICAL_SYSTEM

dnl ========================================================================
dnl Init automake
AM_INIT_AUTOMAKE

dnl ========================================================================
dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
AC_PROG_RANLIB
AC_PROG_LIBTOOL

dnl ========================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl ========================================================================
dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/time.h unistd.h)

devel_mode="yes"
AC_ARG_ENABLE(develmode,
[  --enable-develmode       developer mode <default=yes>],
devel_mode=$enableval)

if test "x$devel_mode" = "xyes"; then
    CPPFLAGS="$CPPFLAGS -DDEBUG -D_DEBUG"
    if test "$ac_cv_prog_gcc" = "yes"; then
        CPPFLAGS="$CPPFLAGS -Wall -Werror"
    fi
fi

if test "$ac_cv_prog_gcc" = "yes"; then
    CFLAGS="$CFLAGS -Wstrict-prototypes -pipe"
fi

AC_ARG_WITH(softkbdsize,
[  --with-softkbdsize=[480x272/320x240/240x320]])

case "$with_softkbdsize" in
  480x272)
    AC_DEFINE(SOFTKBD_480_272, 1, [Define if keyboard size is 480x272.])
    ;;
  240x320)
    AC_DEFINE(SOFTKBD_240_320, 1, [Define if keyboard size is 240x320.])
    ;;
  320x240)
    AC_DEFINE(SOFTKBD_320_240, 1, [Define if keyboard size is 320x240.])
    ;;
  *)
    AC_DEFINE(SOFTKBD_480_272, 1, [Define if keyboard size is 480x272.])
    ;;
esac

build_animate="yes"
AC_ARG_ENABLE([animate],
[  --enable-animate,    enable animation for softkeyboard <default=yes>],
build_animate=$enableval)

if test "x$build_animate" = "xyes" ; then
    AC_DEFINE(KBD_ANIMATE, 1, [Define if soft keyboard animation is supported.])
fi

dnl ========================================================================
dnl check for installation of MiniGUI
dnl ========================================================================
PKG_CHECK_MODULES([MINIGUI], [minigui >= 3.2.2])
PKG_CHECK_MODULES([MGUTILS], [mgutils >= 1.2.0])
PKG_CHECK_MODULES([MGPLUS], [mgplus >= 1.4.0])
PKG_CHECK_MODULES([MGEFF], [mgeff >= 1.2.0])
PKG_CHECK_MODULES([MGNCS], [mgncs >= 1.2.0])
PKG_CHECK_MODULES([MGNCS4TOUCH], [mgncs4touch >= 1.0.0])

LIBS="$LIBS -lcrypt -lmgncs4touch -lmgncs -lmgeff -lmgplus -lmgutils $MINIGUI_LIBS"

dnl ========================================================================
dnl Write Output

AC_CHECK_DECLS(_MGRM_PROCESSES, minigui_runmode="procs", foo=bar, [#include <minigui/common.h>])
AM_CONDITIONAL(MGRM_PROCESSES, test "x$minigui_runmode" = "xprocs")

AC_OUTPUT(
    Makefile
    common/Makefile
    common/src/Makefile
    softkbd/Makefile
    softkbd/src/Makefile
    cbplusui/Makefile
    cbplusui/res/Makefile
    cbplusui/misc/Makefile
    cbplusui/fakejbus/Makefile
    cbplusui/apps/Makefile
)
