%define name boitho-dummy
%define version 0.1
%define release 1

Summary: Install perl modules we need from dummy
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/admin
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Provides: boitho-dummy

%description



%prep

%setup

%build
%install

DESTDIR=$RPM_BUILD_ROOT/home/boitho/boithoTools
rm -rf $DESTDIR
mkdir -p $DESTDIR

%clean


%postun

%pre

echo "Det fungerte!" > /root/dummytest

%post


%files
%defattr(-,boitho,boitho)



