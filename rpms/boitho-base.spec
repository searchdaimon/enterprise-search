%define name boitho-base
%define version 1.3.1
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
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/logs/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/public_html/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/cgi-bin/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/config/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/var/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/script/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/data/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/blackbox/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/perl/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/init.d/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/sql/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/sysconfig/
mkdir -p $RPM_BUILD_ROOT/home/boitho/boithoTools/bin/



%clean


%postun

%pre
#creat boitho user by running useradd. Settng login to bash, ( should change to nologin )
/usr/sbin/useradd -s /bin/bash "boitho"
# and set executable bit on boitho's home
chmod o+x /home/boitho/

mkdir -p /home/boitho/boithoTools/var
chown boitho /home/boitho/boithoTools/var

mkdir -p /home/boitho/boithoTools/logs
chown boitho /home/boitho/boithoTools/logs

mkdir -p /boithoData
chown boitho /boithoData


%post

SSHDCONFIG=/etc/ssh/sshd_config

# Do not permit root to log directly in
egrep "^PermitRootLogin no$" $SSHDCONFIG > /dev/null  || echo "PermitRootLogin no" >> $SSHDCONFIG
# Do not allow password logins
egrep "^PasswordAuthentication yes$" $SSHDCONFIG >/dev/null && sed -i.bak -e 's/^PasswordAuthentication yes$/PasswordAuthentication no/' $SSHDCONFIG 

/etc/init.d/sshd restart

%files
%defattr(-,boitho,boitho)
#/home/boitho/bin/boithoad
# XXX: http://docs.fedoraproject.org/drafts/rpm-guide-en/ch-packaging-guidelines.html#id2994388
# Skal vi lage en dummy-fil i hver av mappene i stedet?
/home/boitho/boithoTools/logs/
/home/boitho/boithoTools/public_html/
/home/boitho/boithoTools/cgi-bin/
/home/boitho/boithoTools/config/
/home/boitho/boithoTools/var/
/home/boitho/boithoTools/script
/home/boitho/boithoTools/data
/home/boitho/boithoTools/blackbox
/home/boitho/boithoTools/perl
/home/boitho/boithoTools/init.d
/home/boitho/boithoTools/sql
/home/boitho/boithoTools/sysconfig
/home/boitho/boithoTools/bin


#/etc/yum.repos.d/boitho.repo

%doc 



