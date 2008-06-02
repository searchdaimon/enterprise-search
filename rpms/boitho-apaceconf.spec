%define name boitho-apaceconf
%define version 1.3.0
%define release 1

Summary: Boitho 
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/authentication
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Requires: /usr/sbin/useradd

#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description



%prep

%setup

%build
%install
install -D -m 755 bbdemo.boitho.com.conf $RPM_BUILD_ROOT/etc/httpd/conf.d/bbdemo.boitho.com.conf

%clean


%postun

%pre

%post
#including bbdemo.boitho.com.conf in httpd.conf 
sed -e 's,^,#,' -i.orig /etc/httpd/conf.d/welcome.conf
#echo "Include /etc/httpd/conf/bbdemo.boitho.com.conf" >> /etc/httpd/conf/httpd.conf

#restarting httpd
#/etc/rc.d/init.d/httpd restart
/etc/rc.d/init.d/httpd graceful

%files
%defattr(-,boitho,boitho)
/etc/httpd/conf.d/bbdemo.boitho.com.conf

%doc 



