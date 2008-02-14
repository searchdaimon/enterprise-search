%define name tlcd-gt14
%define version 0.0.2
%define release 1

Summary: tlcd-gt14. 
Name: %{name}
Version: %{version}
Release: %{release}
Source: tlcd-gt14-%{version}.tar.gz
Vendor: tyan.com
URL: http://www.tyan.com
License: Commercial
Group: Boitho/authentication
Prefix: %{_prefix}
BuildRoot: /var/tmp/%{name}-buildroot
#Requires: compat-libstdc++-296-2.96-139.i386 compat-libstdc++-33-3.2.3-62.i386 compat-libstdc++-33-3.2.3-62.x86_64
Prereq: libstdc++.so.5

#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description



%prep

%setup

%build

%install

install -D -m 755 init.d/tlcd $RPM_BUILD_ROOT/etc/init.d/tlcd
install -D -m 644 tlcd.conf $RPM_BUILD_ROOT/etc/tlcd.conf

install -D -m 755 bin/tlcd $RPM_BUILD_ROOT/usr/bin/tyan/tlcd/tlcd
install -D -m 755 bin/tysmdll.so $RPM_BUILD_ROOT/usr/bin/tyan/tlcd/tysmdll.so
install -D -m 644 bin/mbconf $RPM_BUILD_ROOT/usr/bin/tyan/tlcd/mbconf
install -D -m 644 bin/tlcd.lock $RPM_BUILD_ROOT/usr/bin/tyan/tlcd/tlcd.lock
install -D -m 644 bin/top.log $RPM_BUILD_ROOT/usr/bin/tyan/tlcd/top.log


%clean


%postun

%pre



%post
#run chkconfig to add it to rc
chkconfig --add tlcd

#start it
sh /etc/init.d/tlcd start


%files
%defattr(-,root,root)
/etc/init.d/tlcd
/etc/tlcd.conf
/usr/bin/tyan/tlcd/tlcd
/usr/bin/tyan/tlcd/tysmdll.so
/usr/bin/tyan/tlcd/mbconf
/usr/bin/tyan/tlcd/tlcd.lock
/usr/bin/tyan/tlcd/top.log

%doc 



