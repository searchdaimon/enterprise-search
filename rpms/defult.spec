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
BuildRoot: %{_tmppath}/%{name}-buildroot
Provides: %{name}


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

%pre
#The %pre script executes just before the package is to be installed. It is the rare package that requires 
#anything to be done prior to installation; none of the 350 packages that comprise Red Hat Linux Linux 4.0 make 
#use of it. %post

#rpm_pre

%post
DESTDIR=$RPM_BUILD_ROOT#destdir
#The %post script executes after the package has been installed. One of the most popular reasons a %post script is 
#needed is to run ldconfig to update the list of available shared libraries after a new one has been installed. Of 
#course, other functions can be performed in a %post script. For example, packages that install shells use the 
#%post script to add the shell name to /etc/shells. 
#
#If a package uses a %post script to perform some function, quite often it will include a %postun script that 
#performs the inverse of the %post script, after the package has been removed. 

#rpm_post

%postun

%files
%defattr(-,boitho,boitho)
#/home/boitho/bin/boithoad

#fileslist

%doc 



