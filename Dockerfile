FROM centos:7

ENV container docker

WORKDIR /tmp

RUN sed -i 's/mirror\.centos\.org/vault.centos.org/g' /etc/yum.repos.d/CentOS-*.repo
RUN sed -i 's/^#.*baseurl=http/baseurl=http/g' /etc/yum.repos.d/CentOS-*.repo
RUN sed -i 's/^mirrorlist=http/#mirrorlist=http/g' /etc/yum.repos.d/CentOS-*.repo

RUN yum update -y && yum install -y epel-release && yum install --enablerepo="epel" install -y \
    rootfiles \
    ethtool \
    bash \
    coreutils \
    device-mapper \
    binutils \
    yum-metadata-parser \
    ntp \
    yum-utils \
    httpd \
    man \
    samba \
    setup \
    grep \
    openssl \
    vim-minimal \
    sysvinit \
    php-mysql \
    traceroute \
    rpm \
    initscripts \
    samba-common \
    samba-client \
    man-pages \
    tar \
    httpd-tools  \
    wget \
    symlinks \
    sudo \
    mariadb \
    mariadb-server \
    grub \
    gzip \
    dhclient \
    openssh-clients \
    filesystem \
    crontabs \
    openssh \
    openssh-server \
    perl \
    perl-DBD-MySQL \
    perl-Template-Toolkit \
    perl-XML-Parser \
    perl-XML-Writer \
    perl-IO-String \
    perl-Net-IP \
    php \
    ImageMagick \
    nmap \
    aspell \
    rpcbind \
    perl-Readonly \
    perl-Locale-Maketext \
    perl-App-cpanminus \
    perl-Test-Simple \
    perl-Test-Exception \
    perl-Test-Warn \
    perl-Test-Deep \
    perl-Test-Requires \
    perl-Test-Fatal \
    perl-Params-Validate \
    perl-File-ReadBackwards \
    perl-JSON-XS \
    perl-DateTime \
    perl-Template-Toolkit \
    perl-XML-NamespaceSupport \
    perl-XML-SimpleObject \
    perl-XML-LibXML \
    perl-XML-Parser \
    perl-XML-LibXML-Common \
    perl-XML-SAX \
    perl-Net-IP \
    perl-XML-Writer \
    perl-IO-String \
    perl-Apache-Htpasswd \
    perl-DBI \
    perl-Params-Validate \
    perl-HTTP-Request-AsCGI \
    perl-JSON-XS perl-DateTime \
    perl-ExtUtils-Embed \
    perl-Time-HiRes \
    perl-DBD-MySQL \
    perl-Template-Toolkit \
    perl-XML-Parser \
    perl-XML-Writer \
    perl-IO-String \
    perl-Net-IP \
    perl-Switch \
    perl-Archive-Tar \
    perl-File-Copy-Recursive \
    perl-Locale-Maketext-Lexicon \
    perl-Text-Iconv \
&& yum clean all && rm -rf /var/cache/yum


RUN cpanm SQL::Abstract
RUN cpanm XML::SimpleObject

# Development tools
RUN yum install -y \
    rpm-build \
    valgrind \
    gdb \
    gcc-c++ \
    kernel-devel \
    kernel-headers \
    gcc \
    db4-devel \
    glibc-devel \
    libconfig \
    libconfig-devel \
    mariadb-devel \
    perl-devel \
    libsmbclient-devel \
    libxml2-devel \
    popt-devel \
    git \
    openldap-devel \
    libtool \
    curl-devel \
    zlib-devel \
    gettext-autopoint \
    xsltproc \
    samba-devel \
    perl-ExtUtils-Embed \
    httpd-devel \
    openssl-devel \
&& yum clean all && rm -rf /var/cache/yum


RUN (cd /lib/systemd/system/sysinit.target.wants/; for i in *; do [ $i == systemd-tmpfiles-setup.service ] || rm -f $i; done); \
rm -f /lib/systemd/system/multi-user.target.wants/*; \
rm -f /etc/systemd/system/*.wants/*; \
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
rm -f /lib/systemd/system/basic.target.wants/*; \
rm -f /lib/systemd/system/anaconda.target.wants/*;
# Without this, init won't start the enabled services and exec'ing and starting
# them reports "Failed to get D-Bus connection: Operation not permitted".
VOLUME [ "/run" ]
VOLUME [ "/sys/fs/cgroup" ]

# enable apache
RUN systemctl enable httpd
# enable mysqld
RUN systemctl enable mariadb
# enable ntpd
RUN systemctl enable ntpd
# Add the -x so ntpdate is run on each boot
RUN echo "OPTIONS=\"-u ntp:ntp -p /var/run/ntpd.pid -x\"" >> /etc/sysconfig/ntpd

#RUN /etc/init.d/mysql start

##############################################################################
# Make flex
##############################################################################
WORKDIR /tmp
RUN wget "https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz"
RUN gunzip flex-2.6.4.tar.gz
RUN tar xf flex-2.6.4.tar
WORKDIR /tmp/flex-2.6.4

RUN ./autogen.sh
RUN ./configure
RUN make
RUN make install

##############################################################################
# Make bison
##############################################################################
WORKDIR /tmp
RUN wget "http://ftp.gnu.org/gnu/bison/bison-2.4.1.tar.gz"
RUN gunzip bison-2.4.1.tar.gz
RUN tar xf bison-2.4.1.tar
WORKDIR /tmp/bison-2.4.1

RUN ./configure
RUN make
RUN make install

##############################################################################
# Make daemonize
##############################################################################
COPY src/daemonize /tmp/daemonize/
WORKDIR /tmp/daemonize
RUN ./configure
RUN make
RUN make install



##############################################################################
# Main setup
##############################################################################
RUN useradd boitho
RUN chmod 701 /home/boitho


COPY . /home/boitho/boithoTools/
COPY ./config.mk.template /home/boitho/boithoTools/config.mk
COPY ./config.mk.template /config.mk
COPY ./public_html/webclient2/config.pm.template /home/boitho/boithoTools/public_html/webclient2/config.pm
COPY ./blackbox/bbdemo.boitho.com.conf /etc/httpd/conf.d/bbdemo.boitho.com.conf

##############################################################################
# Make Mode auth boitho
##############################################################################
WORKDIR /home/boitho/boithoTools/src/mod_auth_boitho
RUN make Apache2
RUN make install

##############################################################################
# Make Searchdaimon
##############################################################################
WORKDIR /home/boitho/boithoTools/

ENV BOITHOHOME=/home/boitho/boithoTools

# Compile only. DB setup and daemon startup happen at container runtime, so we
# avoid the appliance `all` target (which also runs dbupdate + init.d start/stop).
# host=searchdaimon3 selects the portable toolchain profile in mk/setup.mk
# (mysql_config / pkg-config / perl -MExtUtils::Embed); the in-build container
# hostname matches none of the hard-coded build hosts otherwise.
RUN make build host=searchdaimon3

##############################################################################
# Runtime services
##############################################################################
# Runtime data dirs the daemons write to, then fix ownership: the tree was
# built as root but the daemons run as user 'boitho'.
RUN mkdir -p /home/boitho/boithoTools/var /boithoData/lot \
 && chown -R boitho:boitho /home/boitho/boithoTools /boithoData \
 && touch /home/boitho/boithoTools/logs/bbdocumentWebAdd.log \
 && chown apache:apache /home/boitho/boithoTools/logs/bbdocumentWebAdd.log \
                        /home/boitho/boithoTools/crawlers \
 && install -d -o apache -g apache /home/boitho/boithoTools/var/webclient2_tpl \
                                  /home/boitho/boithoTools/var/webadmin_tmp \
 && install -o apache -g apache -m 600 /home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd \
                                       /home/boitho/boithoTools/config/.htpasswd

# suggest_server registers over SunRPC, so it needs the portmapper running.
RUN yum install -y rpcbind && systemctl enable rpcbind

# Register the appliance's own init.d services with systemd (via the
# sysv-generator). boithodbsetup seeds the DB; the daemons are everrun-supervised
# and retry until the DB and their peers are up, so no explicit ordering is
# needed beyond boithodbsetup waiting for MariaDB (handled in blackbox/boithodbsetup).
RUN for s in boithodbsetup boithoad boitho-bbdn searchdbb crawlManager suggest crawl_watch; do \
        install -m 755 /home/boitho/boithoTools/init.d/$s /etc/rc.d/init.d/$s && \
        chkconfig --add $s ; \
    done

#ENTRYPOINT ["/usr/sbin/init"]
#CMD ["systemctl"]
CMD ["/usr/sbin/init"]

