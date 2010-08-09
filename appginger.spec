#
# rpm spec file for appginger
#
%define name    appginger
%define version 0.4
%define release alpha
%define _prefix  /usr/local/ginger

Summary: AppGinger - a fast interpreter for Ginger
Name:      %{name}
Version:   %{version}
Release:   %{release}
Prefix:    %{_prefix}
Docdir:    %{_prefix}/doc
License: GPL
Group: Development/Languages
Source: ftp://ftp.gnomovision.com/pub/cdplayer/appginger-%{version}%{release}.tgz
URL: http://www.assembla.com/spaces/ginger
Distribution: sub-Fedora
Vendor: Spice Group
#Icon: appginger.png
BuildRoot: %{_tmppath}/%{name}%{version}-buildroot
Provides: %{name} = %{version}%{release}
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
%setup -q -n %{name}-%{version}%{release}

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
install -m 755 c/appginger/cpp/appginger \
				$RPM_BUILD_ROOT%{_bindir}/appginger
install -m 755 c/common2gnx/cpp/common2gnx \
				$RPM_BUILD_ROOT%{_bindir}/common2gnx
mkdir -p $RPM_BUILD_ROOT%{_datadir}/instructions
install -m 644 instructions/* $RPM_BUILD_ROOT%{_datadir}/instructions
cp *.txt *.odt $RPM_BUILD_ROOT%{_prefix}
mkdir -p $RPM_BUILD_ROOT%{_datadir}/examples
install -m 644 examples/* $RPM_BUILD_ROOT%{_datadir}/examples

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
%doc %{prefix}/*.txt
%doc %{prefix}/*.odt
%{_bindir}/*
%{_datadir}/examples/*
#%{_libdir}/*
#%{_mandir}/man1/*
%dir %{_datadir}/instructions/*
#%{_libdir}/pkgconfig/3store3.pc
#%{prefix}/include/*

%changelog
* Sun Aug 09 2010 Graham Higgins <gjh@bel-epa.com> - 0.1-0
- created spec
