%define name boitho-base
%define version 1.3.0
%define release 1

Summary: Boitho base system. Creates user and other things that is common for all modules
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
install -D -m 755 bbdemo.boitho.com.conf $RPM_BUILD_ROOT/etc/httpd/conf/bbdemo.boitho.com.conf
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/logs/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/public_html/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/cgi-bin/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/config/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/var/

%clean


%postun

%pre
#creat boitho user bu running useradd. Settng login to bash, ( should change to nologin )
/usr/sbin/useradd -s /bin/bash "boitho"
# and set executable bit on boitho's home
chmod o+x /home/boitho/

mkdir -p /home/boitho/boithoTools/var
chown boitho /home/boitho/boithoTools/var
chown boitho /boithoData


%post
#including bbdemo.boitho.com.conf in httpd.conf 
echo "Include /etc/httpd/conf/bbdemo.boitho.com.conf" >> /etc/httpd/conf/httpd.conf

#restarting httpd
/etc/rc.d/init.d/httpd restart

%files
%defattr(-,boitho,boitho)
#/home/boitho/bin/boithoad
/etc/httpd/conf/bbdemo.boitho.com.conf
# XXX: http://docs.fedoraproject.org/drafts/rpm-guide-en/ch-packaging-guidelines.html#id2994388
# Skal vi lage en dummy-fil i hver av mappene i stedet?
/home/boitho/boithoTools/logs/
/home/boitho/boithoTools/public_html/
/home/boitho/boithoTools/cgi-bin/
/home/boitho/boithoTools/config/
/home/boitho/boithoTools/var/

%doc 



