#!/bin/bash

# Update
yum -y update

# Make sure all basic packages are installed
yum -y install rootfiles ethtool bash coreutils device-mapper binutils yum-metadata-parser ntp yum-utils httpd man samba setup grep openssl vim-minimal sysvinit php-mysql traceroute rpm initscripts samba-common samba-client man-pages tar httpd-tools wget symlinks sudo mysql mysql-server grub gzip dhclient openssh-clients filesystem crontabs openssh openssh-server perl php ImageMagick.x86_64 nmap aspell php-cli


# enable Apache to run on start-up
/sbin/chkconfig --levels 345 httpd on
# enable mysqld to run on start-up
/sbin/chkconfig --levels 345 mysqld on
# enable ntpd to run on start-up
/sbin/chkconfig --levels 345 ntpd on
# Add the -x so ntpdate is run on each boot
echo "OPTIONS=\"-u ntp:ntp -p /var/run/ntpd.pid -x\"" >> /etc/sysconfig/ntpd

/etc/init.d/mysqld start

# Grab boitho-base
wget -O /tmp/boitho-base.rpm http://repo.searchdaimon.com/closed/CentOSRepository/boitho/6.8/updates/x86_64/boitho-base-1.3.1-1.x86_64.rpm
rpm -ivh /tmp/boitho-base.rpm

# Get the EPEL repository
wget -O /tmp/epel-release-6-8.noarch.rpm http://www.searchdaimon.com/es/v3/epel-release-6-8.noarch.rpm
rpm -ivh /tmp/epel-release-6-8.noarch.rpm


wget -O /tmp/boitho-public-htaccess.rpm http://www.searchdaimon.com/es/boitho-public-htaccess-0.1.0-1.i386.rpm
rpm -ivh /tmp/boitho-public-htaccess.rpm

# Disable all existing yum reposetorys
#sed -i 's/enabled=1/enabled=0/g' /etc/yum.repos.d/*
# Download the pached version of the CentOS you repo
#wget -O /etc/yum.repos.d/CentOS-Base.repo http://www.searchdaimon.com/es/v3/CentOS-Base.repo
# Setup Yum repos to also use Searchdaimon
wget -O /etc/yum.repos.d/searchdaimon.repo http://www.searchdaimon.com/es/v3/searchdaimon.repo
yum clean all

# Install perl modules we need
yum -y install perl-Template-Toolkit perl-XML-NamespaceSupport perl-XML-SimpleObject perl-XML-LibXML perl-XML-Parser perl-XML-LibXML-Common perl-XML-SAX perl-Net-IP perl-XML-Writer perl-IO-String perl-Apache-Htpasswd perl-DBI perl-Params-Validate perl-HTTP-Request-AsCGI perl-JSON-XS perl-DateTime perl-ExtUtils-Embed perl-Time-HiRes perl-DBD-MySQL perl-Template-Toolkit perl-XML-Parser perl-XML-Writer perl-IO-String perl-Net-IP perl-Text-Iconv.x86_64

# Install catdoc and abiword
yum -y install catdoc
yum -y install abiword

# Instal the setup tool with network and firewall utiletis
yum -y install system-config-firewall-tui system-config-network-tui setuptool

# Rest of the boitho packages
yum -y install boitho-meta

