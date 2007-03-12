%define name #name
%define version #version
%define release 1

Summary: Boitho authentication daemonize.
Name: %{name}
Version: %{version}
Release: %{release}
Source: /home/boitho/redhat/SOURCES/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/authentication
Prefix: %{_prefix}
BuildRoot: /var/tmp/%{name}-buildroot

%description
This library contains C++ utility classes for using IP(sockets).



%prep
%setup

%build
%install
DESTDIR=$RPM_BUILD_ROOT/home/boitho/bin/
rm -rf $DESTDIR
mkdir -p $DESTDIR

echo "RPM_BUILD_ROOT: " $RPM_BUILD_ROOT
echo "DESTDIR: " $DESTDIR
#install -s -m 755 boithoad $DESTDIR/boithoad
#filesinstal

%clean
#rm -rf $RPM_BUILD_ROOT

%post

%postun

%files
%defattr(-,boitho,boitho)
#/home/boitho/bin/boithoad

#fileslist

%doc 



