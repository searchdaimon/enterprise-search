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


#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description
This library contains C++ utility classes for using IP(sockets).



%prep
%setup

%build
%install
DESTDIR=$RPM_BUILD_ROOT#destdir
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



