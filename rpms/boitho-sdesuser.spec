%define name boitho-sdesuser
%define version 1.0.0
%define release 1

Summary: Creates the user sdes
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

install -d -m 700 $RPM_BUILD_ROOT/home/sdes/.ssh/
install -D -m 600 authorized_keys $RPM_BUILD_ROOT/home/sdes/.ssh/authorized_keys


%clean


%postun

%pre

LOGINUSER=sdes
getent passwd $LOGINUSER > /dev/null
if [ $? -eq 2 ]; then
	/usr/sbin/useradd -m -s /bin/bash $LOGINUSER
fi

%post


%files

%defattr(700,sdes,sdes)
/home/sdes/.ssh
%defattr(600,sdes,sdes)
/home/sdes/.ssh/authorized_keys


%doc 



