Name: mod_auth_boitho
Version: 1.1
Release: 1
Source: ftp://mod-auth-shadow.sourceforge.net/pub/mod-auth-shadow/%{name}-%{version}.tar.gz
License: GPL
Group: System Environment/Daemons
Summary: An Apache module for authentication using Boitho aut
BuildRoot: /var/tmp/%{name}-rpmroot
%description

%prep
%setup

%build
make all

%install
mkdir -p $RPM_BUILD_ROOT/usr/{sbin,lib/httpd/modules}
install mod_auth_boitho.so $RPM_BUILD_ROOT/usr/lib/httpd/modules

%post
( echo 'LoadModule auth_shadow_boitho  /usr/lib/httpd/modules/mod_auth_boitho.so'
  ) >>/etc/httpd/conf/httpd.conf

%preun
perl -ni -le 'print unless /mod_auth_boitho/' /etc/httpd/conf/httpd.conf

%changelog
%files
%defattr(-,root,root)
/usr/lib/httpd/modules/*
%doc CHANGES INSTALL README makefile
