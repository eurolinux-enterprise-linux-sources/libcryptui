dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

dnl ****************************************************************************
dnl MAIN SEAHORSE VERSION (update after release)

AC_INIT(libcryptui, 3.12.2)

dnl ****************************************************************************
dnl LIBCRYPTUI libtool versioning
dnl CURRENT : REVISION : AGE
dnl   +1    :     0    : +1   == new interface that does not break old one.
dnl   +1    :     0    :  0   == removed an interface. Breaks old apps.
dnl    ?    :    +1    :  ?   == internal changes that doesn't break anything.

LIBCRYPTUI_CURRENT=0
LIBCRYPTUI_REVISION=0
LIBCRYPTUI_AGE=0

dnl ****************************************************************************

AC_CONFIG_SRCDIR(daemon/seahorse-daemon.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INTLTOOL([0.35.0])
AM_PROG_LIBTOOL
GTK_DOC_CHECK([1.9])

dnl Get the off_t size right
AC_SYS_LARGEFILE
AC_TYPE_OFF_T
AC_FUNC_FSEEKO

AC_CHECK_FUNCS(strsep)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

GTK_REQUIRED=3.0.0
GLIB_REQUIRED=2.32.0

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(SEAHORSE, gmodule-2.0 gio-2.0 gthread-2.0 >= $GLIB_REQUIRED gtk+-3.0 >= $GTK_REQUIRED)

PKG_CHECK_MODULES(SM, sm ice)

GLIB_GSETTINGS

AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-cygwin*|*-*-mingw*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

if test "$platform_win32" = "yes" ; then
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
else
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
fi
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

dnl **************************************************************
dnl GPG / GPGME CHECKS
dnl **************************************************************

AC_ARG_ENABLE(gpg-check, 
	AC_HELP_STRING([--disable-gpg-check], [check GPG version (default is yes)]),
	DO_CHECK=$enableval, DO_CHECK=yes)

if test	"$DO_CHECK" = "yes"; then
	accepted_versions="1.2 1.4 2.0"
	AC_PATH_PROGS(GNUPG, [gpg gpg2], no)
	AC_DEFINE_UNQUOTED(GNUPG, "$GNUPG", [Path to gpg executable.])
	ok="no"
	if test "$GNUPG" != "no"; then
		AC_MSG_CHECKING(for appropriate GnuPG version)
		gnupg_version=`$GNUPG --version | grep gpg`
		major=`echo $gnupg_version | \
			sed 's/^gpg (GnuPG) \([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
		minor=`echo $gnupg_version | \
			sed 's/^gpg (GnuPG) \([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
		micro=`echo $gnupg_version | \
			sed 's/^gpg (GnuPG) \([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`

		for ver in $accepted_versions; do
			if test "$ver" = "$major.$minor"; then
				AC_DEFINE_UNQUOTED(GPG_MAJOR, [$major], [GPG Major Version])
				AC_DEFINE_UNQUOTED(GPG_MINOR, [$minor], [GPG Minor Version])
				AC_DEFINE_UNQUOTED(GPG_MICRO, [$micro], [GPG Micro Version])
				ok="yes"
				break
			fi
		done
	fi

	if test "$ok" = "yes"; then
		AC_MSG_RESULT([yes])
		have_gpg="$gnupg_version"
	else
		AC_MSG_RESULT([no])
	fi
fi

ok="no"
min_gpgme_version=1.0.0
AC_PATH_PROG(GPGME_CONFIG, gpgme-config, "failed")
if test $GPGME_CONFIG != "failed" ; then
	AC_MSG_CHECKING(for GPGME - version >= $min_gpgme_version)
	req_major=`echo $min_gpgme_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\1/'`
	req_minor=`echo $min_gpgme_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\2/'`
	req_micro=`echo $min_gpgme_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\3/'`
	gpgme_config_version=`$GPGME_CONFIG --version`
	major=`echo $gpgme_config_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
	minor=`echo $gpgme_config_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
	micro=`echo $gpgme_config_version | \
		sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`

	if test "$major" -eq "$req_major"; then
		if test "$minor" -ge "$req_minor"; then
			if test "$micro" -ge "$req_micro"; then
				ok="yes"
			fi
		fi
	fi
fi

if test $ok = "yes"; then
	GPGME_CFLAGS=`$GPGME_CONFIG --cflags`
	GPGME_LIBS=`$GPGME_CONFIG --libs`
	AC_MSG_RESULT(yes)
	have_gpgme="$gpgme_config_version"
else
	AC_MSG_RESULT(no)
fi

if test -z "$have_gpg"; then
	AC_MSG_ERROR([Appropriate version of GnuPG not found. Please install one of versions: $accepted_versions])
fi

if test -z "$have_gpgme"; then
	AC_MSG_ERROR(GPGME $min_gpgme_version or later needed)
fi

SEAHORSE_CFLAGS="$SEAHORSE_CFLAGS $GPGME_CFLAGS"
SEAHORSE_LIBS="$SEAHORSE_LIBS $GPGME_LIBS"

dnl ****************************************************************************
dnl DBUS
dnl ****************************************************************************

DBUS_GLIB_REQUIRED=0.35

PKG_CHECK_MODULES([DBUS], [dbus-glib-1 >= $DBUS_GLIB_REQUIRED], [enable_dbus=yes],[enable_dbus=no])
if test "$enable_dbus" = "no"; then
    AC_MSG_ERROR([dbus or dbus-glib-1 bindings not found.])
fi

AC_SUBST([DBUS_CFLAGS])
AC_SUBST([DBUS_LIBS])

AC_PATH_PROG([DBUS_BINDING_TOOL], [dbus-binding-tool], [no])
if test x$DBUS_BINDING_TOOL = "xno" ; then
    AC_MSG_ERROR([dbus-binding-tool executable not found in your path - should be installed with dbus glib bindings])
fi

SEAHORSE_CFLAGS="$SEAHORSE_CFLAGS $DBUS_CFLAGS"
SEAHORSE_LIBS="$SEAHORSE_LIBS $DBUS_LIBS"

dnl *******************
dnl LIBNOTIFY
dnl *******************

LIBNOTIFY_REQUIRED=0.3

# Allow libnotify to be disabled
AC_ARG_ENABLE(libnotify, 
        AC_HELP_STRING([--disable-libnotify],
        [Don't compile in libnotify support]))   
        
if test "x$enable_libnotify" = "xno"; then
    echo "disabling libnotify support"
else

    PKG_CHECK_MODULES(LIBNOTIFY, [libnotify >= $LIBNOTIFY_REQUIRED], 
                      enable_libnotify=yes, enable_libnotify=no)
    if test "$enable_libnotify" = "yes"; then
        
        AC_DEFINE(HAVE_LIBNOTIFY, 1, [libnotify available])
        
        SEAHORSE_CFLAGS="$SEAHORSE_CFLAGS $LIBNOTIFY_CFLAGS"
        SEAHORSE_LIBS="$SEAHORSE_LIBS $LIBNOTIFY_LIBS"
    fi
fi

dnl ****************************************************************************

# Debug mode
AC_ARG_ENABLE(debug, 
	    AC_HELP_STRING([--enable-debug],
	    [Compile binaries in debug mode]))

if test "$enable_debug" = "yes"; then
  CFLAGS="$CFLAGS -g -O0 -DBONOBO_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED -Wall"
  AC_DEFINE_UNQUOTED(_DEBUG, 1, [In debug mode])
  echo "enabling debug compile mode"
else 
  dnl AC_DEFINE_UNQUOTED(G_DISABLE_ASSERT, 1, [Disable glib assertions])
  echo "disabling debug compile mode"
fi

# Fatal Messages mode
AC_ARG_ENABLE(fatal-messages, 
	    AC_HELP_STRING([--enable-fatal-messages],
	    [Don't make compile and or runtime messages fatal in debug mode.]))

if test "$enable_fatal_messages" = "yes" && test "$enable_debug" = "yes"; then
    CFLAGS="$CFLAGS -Wall -Werror"
    AC_DEFINE_UNQUOTED(FATAL_MESSAGES, 1, [Fatal runtime messages])
    echo "enabling fatal runtime messages"
fi

dnl ****************************************************************************
dnl GNOME-KEYRING
dnl ****************************************************************************

PKG_CHECK_MODULES([GNOME_KEYRING], gnome-keyring-1 >= 2.25.5,
                  [enable_gnome_keyring=yes],[enable_gnome_keyring=no])

if test "$enable_gnome_keyring" = "no"; then
	AC_MSG_ERROR([gnome-keyring library or headers not found.])
fi

SEAHORSE_CFLAGS="$SEAHORSE_CFLAGS $GNOME_KEYRING_CFLAGS"
SEAHORSE_LIBS="$SEAHORSE_LIBS $GNOME_KEYRING_LIBS"

dnl ****************************************************************************
dnl LIBCRYPTUI
dnl ****************************************************************************
   
LIBCRYPTUI_LT_RELEASE=$LIBCRYPTUI_CURRENT:$LIBCRYPTUI_REVISION:$LIBCRYPTUI_AGE
AC_SUBST(LIBCRYPTUI_LT_RELEASE)

LIBCRYPTUI_MAJOR=$LIBCRYPTUI_CURRENT.$LIBCRYPTUI_REVISION
AC_SUBST(LIBCRYPTUI_MAJOR)

PKG_CHECK_MODULES(LIBCRYPTUI, gtk+-3.0 >= $GTK_REQUIRED)

cryptuitargetlib=libcryptui.la
AC_SUBST(cryptuitargetlib)

LIBCRYPTUI_CFLAGS="$LIBCRYPTUI_CFLAGS $DBUS_CFLAGS"
LIBCRYPTUI_LIBS="$LIBCRYPTUI_LIBS $DBUS_LIBS"
AC_SUBST(LIBCRYPTUI_CFLAGS)
AC_SUBST(LIBCRYPTUI_LIBS)

GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
ATK_PREFIX="`$PKG_CONFIG --variable=prefix atk`"
PANGO_PREFIX="`$PKG_CONFIG --variable=prefix pango`"
CAIRO_PREFIX="`pkg-config --variable=prefix cairo`" 

AC_SUBST(GLIB_PREFIX)
AC_SUBST(ATK_PREFIX)
AC_SUBST(PANGO_PREFIX)
AC_SUBST(CAIRO_PREFIX)

dnl ****************************************************************************
dnl GObject Introspection
dnl ****************************************************************************

GOBJECT_INTROSPECTION_CHECK([0.6.4])

dnl ****************************************************************************

AC_SUBST(SEAHORSE_CFLAGS)
AC_SUBST(SEAHORSE_LIBS)

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

AC_ARG_ENABLE(update-mime-database,
	AC_HELP_STRING([--disable-update-mime-database],
	[do not run the update-mime-database utility (mostly useful for package maintainers) ]))
AM_CONDITIONAL(UPDATE_MIME_DATABASE, test "$enable_update_mime_database" != "no")

dnl *******************
dnl ***** gettext *****
dnl *******************

GETTEXT_PACKAGE=cryptui
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext domain.])

AM_GLIB_GNU_GETTEXT

dnl ****************************************************************************
## here we get the flags we'll actually use
PKG_CHECK_MODULES(EGG, gtk+-3.0 >= $GTK_REQUIRED)
AC_SUBST(EGG_LIBS)
AC_SUBST(EGG_CFLAGS)

PKG_CHECK_MODULES(EGG_SMCLIENT, gtk+-3.0)
AC_SUBST(EGG_SMCLIENT_LIBS)
AC_SUBST(EGG_SMCLIENT_CFLAGS)

dnl ****************************************************************************

AC_OUTPUT([
Makefile
libcryptui/cryptui.pc
libcryptui/Makefile
libcryptui/tests/Makefile
libegg/Makefile
po/Makefile.in
data/Makefile
pixmaps/Makefile
pixmaps/22x22/Makefile
pixmaps/48x48/Makefile
pixmaps/scalable/Makefile
daemon/Makefile
reference/Makefile
reference/libcryptui/Makefile
reference/libcryptui/version.xml
])

dnl ****************************************************************************
dnl  SUMMARY
dnl ****************************************************************************

echo "
GnuPG Version:              $have_gpg
GPGME Version:              $have_gpgme
Notification Support:       $enable_libnotify
Build introspection data:   ${enable_introspection}
"
