%define name boitho-phonehome
%define version 0.13
%define release 1

Summary: Boitho phone home, includes ssh keys for password less login and config files.
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{_sourcedir}/%{name}-%{version}.tar.gz
Vendor: Search Daimon AS
URL: http://www.searchdaimon.com/
License: Commercial
Group: Boitho/admin
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-buildroot
Provides: boitho-phonehome

%description



%prep

%setup

%build
%install

DESTDIR=$RPM_BUILD_ROOT/home/boitho/boithoTools
rm -rf $DESTDIR
mkdir -p $DESTDIR
mkdir -p $DESTDIR/bin
mkdir -p $DESTDIR/config

SSHDIR=$RPM_BUILD_ROOT/home/phonehome/.ssh
mkdir -p $SSHDIR

echo "RPM_BUILD_ROOT: " $RPM_BUILD_ROOT
echo "DESTDIR: " $DESTDIR
cp bb-client.pl $DESTDIR/bin/
cp bbph-keep-alive.pl $DESTDIR/bin/
cp bb-phone-home-client.conf $DESTDIR/config/
install -D -m 755  init.d/phonehome $RPM_BUILD_ROOT/etc/init.d/phonehome

cp id_rsa $SSHDIR
cp ssh_config $SSHDIR/config

%clean


%postun

%pre

if [ -f /etc/init.d/phonehome ] ; then
	sh /etc/init.d/phonehome stop
fi

/usr/sbin/useradd -s /bin/bash -d /home/phonehome -m "phonehome"
mkdir -p /home/phonehome/.ssh
chown phonehome /home/phonehome/.ssh
chmod 700 /home/phonehome/.ssh
#chown phonehome /home/phonehome/.ssh/*


%post

if [ ! -f /home/boitho/boithoTools/var/phonehome.state ]; then
	echo dead > /home/boitho/boithoTools/var/phonehome.state
fi
chown phonehome /home/boitho/boithoTools/var/phonehome.state

touch /home/boitho/boithoTools/var/bb-phone-home-keepalive-pid-file
chown phonehome /home/boitho/boithoTools/var/bb-phone-home-keepalive-pid-file

chkconfig --add phonehome
sh /etc/init.d/phonehome start


%files
%defattr(-,boitho,boitho)

/home/boitho/boithoTools/bin/bb-client.pl
/home/boitho/boithoTools/bin/bbph-keep-alive.pl
/home/boitho/boithoTools/config/bb-phone-home-client.conf
/etc/init.d/phonehome

%attr(600, phonehome, phonehome) /home/phonehome/.ssh/id_rsa
%attr(600, phonehome, phonehome) /home/phonehome/.ssh/config

%doc 



