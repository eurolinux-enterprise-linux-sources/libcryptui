## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = libegg \
	libcryptui \
	po \
	data \
	pixmaps \
	daemon \
	reference

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	COPYING-DOCS \
	COPYING-LIBCRYPTUI \
	ChangeLog \
	HACKING \
	INSTALL \
	MAINTAINERS \
	NEWS \
	README \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	gnome-doc-utils.make

DISTCLEANFILES = \
    intltool-extract \
    intltool-merge \
    intltool-update
    
DISTCHECK_CONFIGURE_FLAGS = \
    --disable-update-mime-database \
    --enable-gtk-doc \
    --disable-debug

CHANGELOG_START = \
	018d8c2c8eccd35281c595228be44d4630310251

dist-hook:
	@if test -d "$(srcdir)/.git"; \
	then \
		echo Creating ChangeLog && \
		( cd "$(top_srcdir)" && \
		  echo '# Generated automatically. Do not edit.'; echo; \
		  $(top_srcdir)/missing --run git log $(CHANGELOG_START).. --stat --date=short ) > ChangeLog.tmp \
		&& mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
		|| ( rm -f ChangeLog.tmp ; \
		     echo Failed to generate ChangeLog >&2 ); \
	else \
		echo A git clone is required to generate a ChangeLog >&2; \
	fi

distuninstallcheck:
	@:
