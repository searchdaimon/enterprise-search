%define name boitho-meta
%define version 0.1
%define release 1

Summary: Boitho meta packages, depend on all the other boitho packages.
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/meta
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Requires: boitho-ad boitho-searchdbb boitho-crawlManager boitho-bbdn boitho-fileFilter
Requires: boitho-base boitho-bbadmin boitho-everrun boitho-infoquery boitho-iindex boitho-webclient
Requires: boithobb-setuid boithobb-bbadminhtaccess boithobb-crawl_watch bbAutoUpdate bb-call-home
Requires: boitho-database daemonize mod_auth_boitho boitho-phonehome boitho-configfiles
Requires: boitho-scripts boitho-dictionary


#Turn off automatic dependency generation
#se http://fedora.redhat.com/docs/drafts/rpm-guide-en/ch-packaging-guidelines.html
Autoreq: 0 

%description



%prep

%setup

%build
%install

%clean


%postun

%pre


%post

%files
%defattr(-,boitho,boitho)

%doc 



