%define name boitho-local-network
%define version 0.1
%define release 1

Summary: Boitho local network package. Sets eth0 to use 192.168.253.0/24
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/network
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Autoreq: 0 

%description



%prep

%setup

%build
%install

%clean


%postun

%pre

sed -i.orig -e 's,10\.0\.0\.,192.168.253.,g' /etc/sysconfig/network-scripts/ifcfg-eth0 /etc/dhcpd.conf
/etc/init.d/network restart
/etc/init.d/dhcpd restart

%post

%files
%defattr(-,boitho,boitho)

%doc 



