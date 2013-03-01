%define name boitho-syslogconf
%define version 1.0.3
%define release 1

Summary: Boitho 
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/logging
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Requires: rsyslog

#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description



%prep

%setup

%build
%install
install -D -m 755 bb.rsyslog.conf $RPM_BUILD_ROOT/etc/bb.rsyslog.conf

%clean


%postun

%pre

%post

# eirik 07.10.09: no longer needed, just let the log files stay root owned.

# Match up against logs from bb.rsyslog.conf
#for i in searchd test; do
#	if [ ! -f /home/boitho/boithoTools/logs/$i.log ]; then
#		touch /home/boitho/boithoTools/logs/$i.log
#	fi
#	chown boitho:boitho /home/boitho/boithoTools/logs/$i.log
#done

cp /etc/rsyslog.conf /etc/rsyslog.conf.orig
cp /etc/bb.rsyslog.conf /etc/rsyslog.conf
/etc/rc.d/init.d/rsyslog restart

for i in searchdbb_stderr searchdbb_stdout crawlManager_access crawlManager_error; do
	if [ -f /home/boitho/boithoTools/logs/$i ]; then
		rm -f /home/boitho/boithoTools/logs/$i
	fi
done

%files
%defattr(-,boitho,boitho)
/etc/bb.rsyslog.conf

%doc 

