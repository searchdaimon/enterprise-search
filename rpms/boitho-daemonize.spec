%define name daemonize
%define version 1.5.2
%define release 1

Summary: Boitho daemonize. Creates user and other things that is common for all modules
Name: %{name}
Version: %{version}
Release: %{release}
Source: daemonize-%{version}.tar.gz
Vendor: clapper.org
URL: http://www.clapper.org/software/daemonize/
License: Commercial
Group: Boitho/authentication
Prefix: %{_prefix}
BuildRoot: /var/tmp/%{name}-buildroot
Requires: /usr/sbin/useradd

#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description



%prep

%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure
make

%install
echo "RPM_BUILD_ROOT: "$RPM_BUILD_ROOT

make INSTALL_PREFIX=$RPM_BUILD_ROOT/usr/local install


%clean


%postun

%pre



%post


%files
%defattr(-,root,root)
/usr/local/sbin/daemonize
%doc 



