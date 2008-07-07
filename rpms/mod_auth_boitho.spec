Name: mod_auth_boitho
Version: 1.2
Release: 1
Source: /home/eirik/redhat/SOURCES/%{name}-%{version}.tar.gz
License: GPL
Group: System Environment/Daemons
Summary: An Apache module for authentication using Boitho auth
BuildRoot: %{_tmppath}/%{name}-rpmroot
Provides: mod_auth_boitho
%description

%prep
%setup

%build
%install
mkdir -p $RPM_BUILD_ROOT/usr/lib/httpd/modules
install mod_auth_boitho.so $RPM_BUILD_ROOT/usr/lib/httpd/modules

%post
( echo 'LoadModule auth_boitho_module /usr/lib/httpd/modules/mod_auth_boitho.so'
  ) >>/etc/httpd/conf.d/mod_boitho_auth.conf

%preun
perl -ni -le 'print unless /mod_auth_boitho/' /etc/httpd/conf/httpd.conf

%changelog
%files
%defattr(-,root,root)
/usr/lib/httpd/modules/mod_auth_boitho.so

