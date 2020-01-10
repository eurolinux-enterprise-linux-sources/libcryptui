Name: libcryptui
Version: 3.8.0
Release: 3%{?dist}
Summary: Interface components for OpenPGP

Group: System Environment/Libraries
License: LGPLv2+
URL: http://projects.gnome.org/seahorse/
Source0: http://download.gnome.org/sources/libcryptui/3.8/%{name}-%{version}.tar.xz

BuildRequires: dbus-glib-devel
BuildRequires: gnome-doc-utils
BuildRequires: libgnome-keyring-devel
BuildRequires: gobject-introspection-devel
BuildRequires: gpgme-devel
BuildRequires: gtk3-devel
BuildRequires: intltool
BuildRequires: libnotify-devel
BuildRequires: libtool
BuildRequires: libSM-devel

%description
libcryptui is a library used for prompting for PGP keys.

%package devel
Summary: Header files required to develop with libcryptui
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig

%description devel
The libcryptui-devel package contains the header files and developer
documentation for the libcryptui library.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT
%find_lang cryptui --with-gnome --all-name

find ${RPM_BUILD_ROOT} -type f -name "*.a" -exec rm -f {} ';'
find ${RPM_BUILD_ROOT} -type f -name "*.la" -exec rm -f {} ';'

%post -p /sbin/ldconfig

%postun
/sbin/ldconfig
if [ $1 -eq 0 ]; then
  glib-compile-schemas %{_datadir}/glib-2.0/schemas &>/dev/null || :
fi

%posttrans
glib-compile-schemas %{_datadir}/glib-2.0/schemas &>/dev/null || :

%files -f cryptui.lang
%doc AUTHORS COPYING-LIBCRYPTUI NEWS README
%{_bindir}/*
%{_mandir}/man1/*.gz
%{_datadir}/cryptui
%{_libdir}/libcryptui.so.*
%{_datadir}/dbus-1/services/*
%{_datadir}/pixmaps/cryptui
%{_libdir}/girepository-1.0/*
%{_datadir}/GConf/gsettings/org.gnome.seahorse.recipients.convert
%{_datadir}/glib-2.0/schemas/org.gnome.seahorse.recipients.gschema.xml

%files devel
%{_libdir}/libcryptui.so
%{_libdir}/pkgconfig/*
%{_includedir}/libcryptui
%{_datadir}/gtk-doc/html/libcryptui
%{_datadir}/gir-1.0/*

%changelog
* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 3.8.0-3
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 3.8.0-2
- Mass rebuild 2013-12-27

* Tue Mar 26 2013 Kalev Lember <kalevlember@gmail.com> - 3.8.0-1
- Update to 3.8.0

* Wed Feb 06 2013 Kalev Lember <kalevlember@gmail.com> - 3.7.5-1
- Update to 3.7.5

* Tue Sep 25 2012 Matthias Clasen <mclasen@redhat.com> -3.6.0-1
- Update to 3.6.0

* Wed Sep 19 2012 Tomas Bzatek <tbzatek@redhat.com> - 3.5.92-1
- Update to 3.5.92

* Tue Jul 17 2012 Richard Hughes <hughsient@gmail.com> - 3.5.4-1
- Update to 3.5.4

* Tue Apr 24 2012 Kalev Lember <kalevlember@gmail.com> - 3.4.1-2
- Silence rpm scriptlet output

* Mon Apr 16 2012 Richard Hughes <hughsient@gmail.com> - 3.4.1-1
- Update to 3.4.1

* Tue Mar 27 2012 Kalev Lember <kalevlember@gmail.com> - 3.4.0-1
- Update to 3.4.0

* Fri Mar  9 2012 Matthias Clasen <mclasen@redhat.com> - 3.3.91-1
- Update to 3.3.91

* Tue Feb  7 2012 Matthias Clasen <mclasen@redhat.com> - 3.3.5-1
- Update to 3.3.5

* Thu Jan 26 2012 Tomas Bzatek <tbzatek@redhat.com> - 3.2.2-3
- Fix BuildRequires

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.2.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Thu Nov 24 2011 Matthias Clasen <mclasen@redhat.com> - 3.2.2-1
- Update to 3.2.2

* Fri Nov 18 2011 Matthew Barnes <mbarnes@redhat.com> - 3.2.0-2
- Remove gtk-doc req in devel subpackage (RH bug #754500).

* Wed Sep 28 2011 Matthias Clasen <mclasen@redhat.com> - 3.2.0-1
- Update to 3.2.0

* Tue Sep  6 2011 Matthias Clasen <mclasen@redhat.com> - 3.1.91-1
- Update to 3.1.91

* Wed Jul 27 2011 Matthew Barnes <mbarnes@redhat.com> - 3.1.4-3
- Add upstream patch to avoid file conflicts with seahorse.

* Tue Jul 26 2011 Matthew Barnes <mbarnes@redhat.com> - 3.1.4-2
- Package review changes.

* Mon Jul 25 2011 Matthew Barnes <mbarnes@redhat.com> - 3.1.4-1
- Initial packaging.
