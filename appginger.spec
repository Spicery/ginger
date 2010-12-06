#
# rpm spec file for appginger
#
%define name    appginger
%define version 0.6.1
%define release alpha       
%define _prefix  /usr/local

Summary: AppGinger - a fast interpreter for Ginger
Name:      %{name}
Version:   %{version}
Release:   %{release}
Prefix:    %{_prefix}
Docdir:    %{_prefix}/doc
License: GPL
Group: Development/Languages
Source: http://www.assembla.com/spaces/ginger/documents/appginger-%{version}.tgz
URL: http://www.assembla.com/spaces/ginger
Distribution: sub-Fedora
Vendor: Spice Group
#Icon: appginger.png
BuildRoot: %{_tmppath}/%{name}%{version}-buildroot
Provides: %{name} = %{version}
Requires: boost-devel
ExcludeArch: sparc alpha
ExclusiveArch: i386
ExclusiveOS: linux

%description
Development:
  Stephen Leach <steve@steelypip.com>
  
Design Team:
  Stephen Leach <steve@steelypip.com>
  Chris Dollin <eh@electric-hedgehog.com> 

%prep
%setup -q -n %{name}-%{version}

%build
./configure --prefix=%{_prefix} \
    --bindir=%{_bindir} --mandir=%{_mandir} \
    --localstatedir=%{_localstatedir} --libdir=%{_libdir} \
    --datadir=%{_datadir} --includedir=%{_includedir} \
    --sysconfdir=%{_sysconfdir}

CFLAGS="$RPM_OPT_FLAGS" make \
    prefix=$RPM_BUILD_ROOT%{_prefix} \
    bindir=$RPM_BUILD_ROOT%{_bindir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} \
    libdir=$RPM_BUILD_ROOT%{_libdir} \
    localstatedir=$RPM_BUILD_ROOT%{localstatedir} \
    datadir=$RPM_BUILD_ROOT%{_datadir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} \
    %{?_smp_mflags} DESTDIR=$RPM_BUILD_ROOT all 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
install -m 755 apps/appginger/cpp/appginger $RPM_BUILD_ROOT%{_bindir}/appginger
install -m 755 apps/common2gnx/cpp/common2gnx $RPM_BUILD_ROOT%{_bindir}/common2gnx
install -m 755 apps/fetchgnx/cpp/fetchgnx $RPM_BUILD_ROOT%{_bindir}/fetchgnx
install -m 755 apps/file2gnx/cpp/file2gnx $RPM_BUILD_ROOT%{_bindir}/file2gnx
install -m 755 apps/lisp2gnx/lsp/lisp2gnx $RPM_BUILD_ROOT%{_bindir}/lisp2gnx
mkdir -p $RPM_BUILD_ROOT%{_datadir}/appginger
install -m a=r LICENSE.txt $RPM_BUILD_ROOT%{_datadir}/appginger
install -m a=r *.txt *.odt $RPM_BUILD_ROOT%{_datadir}/appginger
mkdir -p $RPM_BUILD_ROOT%{_datadir}/appginger/examples
install -m a=r examples/* $RPM_BUILD_ROOT%{_datadir}/appginger/examples
mkdir -p $RPM_BUILD_ROOT%{_datadir}/appginger/lisp2gnx
install -m a=r apps/lisp2gnx/lsp/lisp2gnx.lsp $RPM_BUILD_ROOT%{_datadir}/appginger/lisp2gnx

%clean
rm -rf $RPM_BUILD_ROOT

%pre
# Pass

%post
# Pass

%preun
# Pass

%postun
# Pass

%files
%defattr (-, root, root)
%{_bindir}/*
%dir %{_datadir}/appginger/*
%dir %{_datadir}/appginger/examples/*
%dir %{_datadir}/appginger/lisp2gnx/*

%changelog
* Sun Dec 05 2010 Graham Higgins <gjh@bel-epa.com> - 0.1-1
- updated spec
* Sun Aug 09 2010 Graham Higgins <gjh@bel-epa.com> - 0.1-0
- created spec
