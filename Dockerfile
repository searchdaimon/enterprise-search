FROM amazonlinux:2023.12.20261021.0

ENV container=docker
WORKDIR /tmp

##############################################################################
# System + build dependencies
##############################################################################
# Amazon Linux 2023 uses dnf. Unlike the old CentOS 7 image there is no EOL
# mirror dance: AL2023 repos are live. The MariaDB *server* is no longer
# installed here -- it runs in its own compose container (see docker-compose.yml);
# this image only needs the client (mysql/mysqladmin) and the connector dev
# package (mysql_config / headers) to build and to seed the remote DB.
RUN dnf -y update && dnf -y install \
        gcc gcc-c++ make autoconf automake libtool patch m4 help2man \
        tar gzip wget git symlinks sudo which findutils \
        httpd httpd-devel httpd-tools \
        mariadb105 mariadb105-devel mariadb-connector-c-devel \
        libsmbclient libsmbclient-devel samba-client \
        libconfig libconfig-devel \
        openldap-devel libxml2-devel libcurl-devel openssl-devel \
        zlib-devel popt-devel expat-devel \
        rpcbind ImageMagick aspell chkconfig \
        perl perl-devel perl-ExtUtils-Embed perl-ExtUtils-MakeMaker \
    && dnf clean all

##############################################################################
# Perl modules
##############################################################################
# Install everything AL2023 packages natively (fast, no compiler/test flakiness),
# then bootstrap cpanm and fill the gaps not in the distro repos.
RUN dnf -y install \
        perl-DBI perl-DBD-MySQL perl-DBD-MariaDB perl-Template-Toolkit \
        perl-XML-Parser perl-XML-Writer perl-XML-LibXML perl-XML-SAX \
        perl-XML-NamespaceSupport perl-IO-String perl-Readonly perl-Params-Validate \
        perl-File-ReadBackwards perl-JSON-XS perl-JSON perl-DateTime perl-Switch \
        perl-File-Copy-Recursive perl-Text-Iconv perl-CGI perl-libwww-perl \
        perl-LWP-Protocol-https perl-URI perl-Config-Tiny perl-Archive-Tar \
        perl-Time-HiRes perl-OLE-Storage_Lite perl-Digest-MD5 perl-Storable \
        perl-Test-Simple perl-Encode \
    && dnf clean all

# cpanm itself isn't packaged on AL2023; the cpanmin.us script is a self-contained
# (fatpacked) installer that needs only the system perl.
RUN curl -sL https://cpanmin.us -o /usr/local/bin/cpanm && chmod +x /usr/local/bin/cpanm

# Modules not in the AL2023 repos; app/admin runtime deps. MIME::Tools
# (MIME::Entity / MIME::Parser), together with the distro perl-OLE-Storage_Lite
# above, are what the .msg file filter needs to index Outlook messages
# (upstream issues #18 / #27).
RUN cpanm --notest \
        XML::SimpleObject \
        Net::IP \
        Locale::Maketext::Lexicon \
        Apache::Htpasswd \
        HTTP::Request::AsCGI \
        SQL::Abstract \
        MIME::Tools

##############################################################################
# Berkeley DB (built from source)
##############################################################################
# RHEL 9 / Amazon Linux 2023 dropped the Berkeley DB -devel package, but the
# search kernel and several index tools link libdb (db_create, DB_BTREE, ...).
# Build 5.3.28 statically and link it in -- same "from source" approach the image
# already uses for flex/bison. 5.3.28 matches the API the code was written against.
WORKDIR /tmp
RUN wget -q "https://download.oracle.com/berkeley-db/db-5.3.28.tar.gz" \
 && tar xf db-5.3.28.tar.gz
# Modern gcc ships an __atomic_compare_exchange builtin that clashes with BDB's
# own static inline of the same name -- rename BDB's to let it compile.
RUN sed -i 's/__atomic_compare_exchange/__atomic_compare_exchange_db/g' \
        /tmp/db-5.3.28/src/dbinc/atomic.h
WORKDIR /tmp/db-5.3.28/build_unix
# Build position-independent objects (-fPIC / --with-pic): modern gcc links
# executables as PIE by default, and a non-PIC static libdb.a cannot be embedded
# into a PIE (relocation R_X86_64_32 ... can not be used when making a PIE object),
# which breaks the perl-embedded daemons (crawlManager2, boitho-bbdn).
RUN ../dist/configure --prefix=/usr/local/BerkeleyDB --enable-static --disable-shared \
        --with-pic CFLAGS="-fPIC -O2" \
 && make -j"$(nproc)" \
 && make install

##############################################################################
# flex (built from source)
##############################################################################
# AL2023 packages a usable flex, but the search/parser grammars were developed
# against 2.6.4; build it from the release tarball (it ships a ready ./configure,
# so no autogen step is needed).
WORKDIR /tmp
RUN wget -q "https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz" \
 && tar xf flex-2.6.4.tar.gz
WORKDIR /tmp/flex-2.6.4
RUN ./configure && make -j"$(nproc)" && make install

##############################################################################
# bison (built from source)
##############################################################################
# Pinned to 2.4.1: bison 3.x made breaking changes (removed YYPARSE_PARAM,
# %name-prefix semantics, etc.) that the legacy .y grammars rely on.
WORKDIR /tmp
RUN wget -q "http://ftp.gnu.org/gnu/bison/bison-2.4.1.tar.gz" \
 && tar xf bison-2.4.1.tar.gz
WORKDIR /tmp/bison-2.4.1
RUN ./configure && make -j"$(nproc)" && make install

##############################################################################
# daemonize (built from source; used by the init.d scripts)
##############################################################################
COPY src/daemonize /tmp/daemonize/
WORKDIR /tmp/daemonize
RUN ./configure && make && make install

##############################################################################
# Sun RPC toolchain (for the Suggest autocomplete service)
##############################################################################
# glibc removed the built-in Sun RPC implementation and rpcgen; the Suggest
# server/client use RPC, so pull rpcgen plus libtirpc (the replacement runtime
# and rpc/*.h headers). Kept in its own late layer so the from-source builds
# above stay cached when this changes.
RUN dnf -y install rpcgen libtirpc libtirpc-devel && dnf clean all

# procps-ng: the init.d scripts use pidof/ps for status and stop. Without them
# the admin panel's "Manage services" page reports every daemon as stopped and
# cannot stop or restart them (start still works, which is why the entrypoint
# got by without it).
RUN dnf -y install procps-ng && dnf clean all

##############################################################################
# Main setup
##############################################################################
RUN useradd boitho && chmod 701 /home/boitho

COPY . /home/boitho/boithoTools/
COPY ./config.mk.template /home/boitho/boithoTools/config.mk
COPY ./config.mk.template /config.mk
COPY ./public_html/webclient2/config.pm.template /home/boitho/boithoTools/public_html/webclient2/config.pm
COPY ./blackbox/bbdemo.boitho.com.conf /etc/httpd/conf.d/bbdemo.boitho.com.conf

##############################################################################
# mod_auth_boitho (Apache auth module gating the admin panel)
##############################################################################
WORKDIR /home/boitho/boithoTools/src/mod_auth_boitho
RUN make Apache2 && make install

##############################################################################
# Build Searchdaimon
##############################################################################
WORKDIR /home/boitho/boithoTools/
ENV BOITHOHOME=/home/boitho/boithoTools

# Compile only. DB setup and daemon startup happen at container runtime, so we
# avoid the appliance `all` target (which also runs dbupdate + init.d start/stop).
# host=searchdaimon3 selects the portable toolchain profile in mk/setup.mk
# (mysql_config / pkg-config / perl -MExtUtils::Embed). BDB_INC/BDB_LIB are passed
# as command-line overrides (they beat the in-makefile defaults and propagate to
# sub-makes via MAKEFLAGS) to point at the from-source Berkeley DB above.
RUN make build host=searchdaimon3 \
        BDB_INC="-I/usr/local/BerkeleyDB/include" \
        BDB_LIB="/usr/local/BerkeleyDB/lib/libdb.a -lpthread"

# The setuid helpers the admin panel (running as apache) uses to control the
# daemons, run yum and switch the package repo. The appliance ships them in the
# boithobb-setuid RPM, which `make build` does not cover. Built unprivileged
# here; owner/mode are set in the runtime section below, after the chown -R.
RUN make InitServices YumWrapper NetConfig repomodwrap host=searchdaimon3

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

# Install the setuid helpers as root 4755 (what the boithobb-setuid RPM does on
# the appliance). initwrapper runs /etc/init.d/<service>, so link the appliance
# init scripts there for the services it knows about (phonehome is left out on
# purpose: help.cgi treats its presence as "phone home installed").
RUN chown root:root /home/boitho/boithoTools/setuid/* \
 && chmod 4755 /home/boitho/boithoTools/setuid/* \
 && for s in boithoad boitho-bbdn searchdbb crawlManager suggest crawl_watch bbAutoUpdate; do \
      ln -s /home/boitho/boithoTools/init.d/$s /etc/init.d/$s; \
    done

# Where the admin panel's "Crashes" page looks for core dumps (the appliance
# kernel core_pattern writes there). Empty in a container, but it must exist.
RUN install -d -o apache -g apache /coredumps/saved

# The compose entrypoint (docker/entrypoint.sh) starts the stack without systemd
# (cgroup-v2 friendly) and is mounted in at run time. Copy it in too so a plain
# `docker run` of this image also works.
COPY docker/entrypoint.sh /usr/local/bin/es-entrypoint.sh
RUN chmod +x /usr/local/bin/es-entrypoint.sh
CMD ["bash", "/usr/local/bin/es-entrypoint.sh"]
