include $(top_srcdir)/Makefile.decl

localedir = $(datadir)/locale
pixmapsdir = $(datadir)/pixmaps/cryptui/
uidir = $(datadir)/cryptui/ui/

INCLUDES = -I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libcryptui \
	-I$(top_srcdir)/libegg \
	-I$(top_srcdir)/libseahorse \
	$(SEAHORSE_CFLAGS) \
	-DDATA_DIR=\""$(datadir)"\" \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DPIXMAPSDIR=\""$(pixmapsdir)"\" \
	-DLIBCRYPTUI_API_SUBJECT_TO_CHANGE \
	-DAUTOSTARTDIR=\""$(sysconfdir)/xdg/autostart"\" \
	-DSEAHORSE_UIDIR=\""$(uidir)"\"

bin_PROGRAMS = seahorse-daemon

MARSHAL_SRCS = \
	seahorse-marshal.c seahorse-marshal.h

BUILT_SOURCES = \
	seahorse-service-bindings.h \
	seahorse-service-keyset-bindings.h \
	seahorse-service-crypto-bindings.h \
	$(MARSHAL_SRCS)

seahorse_daemon_SOURCES = \
	seahorse-daemon.c seahorse-daemon.h \
	seahorse-dbus-server.c \
	seahorse-gpgme.c seahorse-gpgme.h \
	seahorse-gpgme-data.c seahorse-gpgme-data.h \
	seahorse-gpgme-generate.c seahorse-gpgme-generate.h \
	seahorse-gpgme-key.c seahorse-gpgme-key.h \
	seahorse-gpgme-key-op.c seahorse-gpgme-key-op.h \
	seahorse-gpgme-operation.c seahorse-gpgme-operation.h \
	seahorse-gpgme-source.c seahorse-gpgme-source.h \
	seahorse-gpgme-subkey.c seahorse-gpgme-subkey.h \
	seahorse-gpgme-uid.c seahorse-gpgme-uid.h \
	seahorse-gpg-op.h seahorse-gpg-op.c \
	seahorse-gpg-options.c seahorse-gpg-options.h \
	seahorse-object-list.c seahorse-object-list.h \
	seahorse-pgp.c seahorse-pgp.h \
	seahorse-pgp-key.c seahorse-pgp-key.h \
	seahorse-pgp-module.c seahorse-pgp-module.h \
	seahorse-pgp-signature.c seahorse-pgp-signature.h \
	seahorse-pgp-subkey.c seahorse-pgp-subkey.h \
	seahorse-pgp-uid.c seahorse-pgp-uid.h \
	seahorse-secure-buffer.c seahorse-secure-buffer.h \
	seahorse-service.c seahorse-service.h \
	seahorse-service-keyset.c seahorse-service-crypto.c \
	seahorse-context.c seahorse-context.h \
	seahorse-gtkstock.c seahorse-gtkstock.h \
	seahorse-libdialogs.h \
	seahorse-notification.c \
	seahorse-object.c seahorse-object.h \
	seahorse-operation.c seahorse-operation.h \
	seahorse-passphrase.c seahorse-passphrase.h \
	seahorse-progress.c seahorse-progress.h \
	seahorse-secure-memory.c seahorse-secure-memory.h \
	seahorse-set.c seahorse-set.h \
	seahorse-source.c seahorse-source.h \
	seahorse-transfer-operation.c seahorse-transfer-operation.h \
	seahorse-types.c seahorse-types.h \
	seahorse-unix-signal.c seahorse-unix-signal.h \
	seahorse-unknown.c seahorse-unknown.h \
	seahorse-unknown-source.c seahorse-unknown-source.h \
	seahorse-util.c seahorse-util.h \
	seahorse-validity.c seahorse-validity.h \
	seahorse-widget.c seahorse-widget.h \
	$(BUILT_SOURCES)

seahorse_daemon_LDADD = \
	$(top_builddir)/libcryptui/libcryptui.la \
	$(top_builddir)/libegg/libeggdatetime.la \
	$(top_builddir)/libegg/libeggsmclient.la \
	$(SEAHORSE_LIBS)

man_MANS = seahorse-daemon.1

EXTRA_DIST = \
	$(man_MANS) \
	seahorse-service.xml \
	seahorse-service-keyset.xml \
	seahorse-service-crypto.xml \
	seahorse-service-bindings.h \
	seahorse-service-keyset-bindings.h \
	seahorse-service-crypto-bindings.h \
	org.gnome.seahorse.service.in \
	$(ui_DATA) \
	seahorse-marshal.list

CLEANFILES = *.bak \
	org.gnome.seahorse.service

# DBUS binding files
seahorse-service-bindings.h: seahorse-service.xml
	dbus-binding-tool --prefix=seahorse_service --mode=glib-server --output=$@ $<
seahorse-service-keyset-bindings.h: seahorse-service-keyset.xml
	dbus-binding-tool --prefix=seahorse_service_keyset --mode=glib-server --output=$@ $<
seahorse-service-crypto-bindings.h: seahorse-service-crypto.xml
	dbus-binding-tool --prefix=seahorse_service_crypto --mode=glib-server --output=$@ $<

# Dbus service file
servicedir = $(datadir)/dbus-1/services/
service_in_files = org.gnome.seahorse.service.in
service_DATA = $(service_in_files:.service.in=.service)

seahorse-marshal.h: seahorse-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=seahorse_marshal > $@

seahorse-marshal.c: seahorse-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"seahorse-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=seahorse_marshal >> $@

ui_DATA = \
	seahorse-progress.xml \
	seahorse-notify.xml \
	seahorse-pgp-generate.xml
