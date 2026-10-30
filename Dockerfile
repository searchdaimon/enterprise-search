# syntax=docker/dockerfile:1
#
# Multi-stage build:
#
#   base     - Amazon Linux 2023 + everything the appliance needs at *runtime*
#              (httpd, the MariaDB client, the C daemons' shared libraries, Perl
#              and the Perl modules the web UI / crawlers use), plus gcc, make
#              and perl-devel so cpanm can build XS modules in this stage.
#   builder  - base + the C toolchain and -devel packages, the from-source build
#              deps (Berkeley DB, flex, bison, daemonize), and the compile itself.
#              It receives ONLY the C sources (src/, mk/, Makefile, perlxs/), so a
#              change to the Perl/web side never invalidates it.
#   runtime  - base + the compiled artifacts copied out of builder + the full
#              source tree (the Perl CGIs, templates, config, init scripts...).
#              A Perl-only change rebuilds from the final COPY onwards: seconds.
#
# The appliance semantics are kept: the tree lives at /home/boitho/boithoTools and
# is both the install location and $BOITHOHOME.

ARG BASE_IMAGE=amazonlinux:2023.12.20261021.0

##############################################################################
# base: runtime packages
##############################################################################
FROM ${BASE_IMAGE} AS base

ENV container=docker
ENV BOITHOHOME=/home/boitho/boithoTools

# Amazon Linux 2023 uses dnf. The MariaDB *server* is not installed here -- it
# runs in its own compose container (see docker-compose.yml); this image only
# needs the client (mysql/mysqladmin) to seed the remote DB.
#
# Runtime libraries are the ones `ldd` reports for bin/*, the crawler/usersystem
# .so plugins and the Apache module: libsmbclient/samba-client-libs, libconfig,
# libxml2, libcurl, openldap, openssl, libtirpc (Sun RPC for Suggest),
# mariadb-connector-c (pulled in by the client) and perl-libs (the embedded
# interpreter in crawlManager2 / boitho-bbdn).
#
# procps-ng: the init.d scripts use pidof/ps for status and stop. Without them
# the admin panel's "Manage services" page reports every daemon as stopped and
# cannot stop or restart them.
RUN dnf -y update && dnf -y install \
        httpd httpd-tools \
        mariadb105 \
        libsmbclient samba-client libconfig libxml2 libcurl-minimal \
        openldap openssl-libs libtirpc \
        rpcbind ImageMagick aspell chkconfig procps-ng \
        which findutils tar gzip sudo \
        perl perl-libs \
    && dnf clean all

# Perl modules: everything AL2023 packages natively (fast, no compiler/test
# flakiness), then bootstrap cpanm and fill the gaps not in the distro repos.
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

# gcc/gcc-c++/make/perl-devel/MakeMaker: the minimum to build XS Perl modules
# with cpanm. Several connector dependencies are XS (HTML::Strip, and on the
# appliance Filesys::SmbClient), and the appliance itself ships a compiler. Its
# own layer, so a toolchain change does not invalidate the two large (and slow
# to download) package layers above.
RUN dnf -y install gcc gcc-c++ make perl-devel perl-ExtUtils-MakeMaker && dnf clean all

# cpanm itself isn't packaged on AL2023; the cpanmin.us script is a self-contained
# (fatpacked) installer that needs only the system perl.
RUN curl -sL https://cpanmin.us -o /usr/local/bin/cpanm && chmod +x /usr/local/bin/cpanm

# Modules not in the AL2023 repos; app/admin runtime deps. MIME::Tools (MIME::Entity / MIME::Parser),
# together with the distro perl-OLE-Storage_Lite above, are what the .msg file
# filter needs to index Outlook messages (upstream issues #18 / #27).
RUN cpanm --notest \
        XML::SimpleObject \
        Net::IP \
        Locale::Maketext::Lexicon \
        Apache::Htpasswd \
        HTTP::Request::AsCGI \
        SQL::Abstract \
        MIME::Tools

RUN useradd boitho && chmod 701 /home/boitho

##############################################################################
# builder: toolchain + compile
##############################################################################
FROM base AS builder

WORKDIR /tmp

RUN dnf -y install \
        gcc gcc-c++ make autoconf automake libtool patch m4 help2man \
        wget git symlinks \
        httpd-devel \
        mariadb105-devel mariadb-connector-c-devel \
        libsmbclient-devel libconfig-devel \
        openldap-devel libxml2-devel libcurl-devel openssl-devel \
        zlib-devel popt-devel expat-devel \
        perl-devel perl-ExtUtils-Embed perl-ExtUtils-MakeMaker \
        rpcgen libtirpc-devel \
    && dnf clean all

# --- Berkeley DB (built from source) ----------------------------------------
# RHEL 9 / Amazon Linux 2023 dropped the Berkeley DB -devel package, but the
# search kernel and several index tools link libdb (db_create, DB_BTREE, ...).
# Build 5.3.28 statically and link it in -- same "from source" approach the image
# already uses for flex/bison. 5.3.28 matches the API the code was written against.
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

# --- flex (built from source) -----------------------------------------------
# AL2023 packages a usable flex, but the search/parser grammars were developed
# against 2.6.4; build it from the release tarball (it ships a ready ./configure,
# so no autogen step is needed).
WORKDIR /tmp
RUN wget -q "https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz" \
 && tar xf flex-2.6.4.tar.gz
WORKDIR /tmp/flex-2.6.4
RUN ./configure && make -j"$(nproc)" && make install

# --- bison (built from source) ----------------------------------------------
# Pinned to 2.4.1: bison 3.x made breaking changes (removed YYPARSE_PARAM,
# %name-prefix semantics, etc.) that the legacy .y grammars rely on.
WORKDIR /tmp
RUN wget -q "http://ftp.gnu.org/gnu/bison/bison-2.4.1.tar.gz" \
 && tar xf bison-2.4.1.tar.gz
WORKDIR /tmp/bison-2.4.1
RUN ./configure && make -j"$(nproc)" && make install

# --- daemonize (built from source; used by the init.d scripts) ---------------
COPY src/daemonize /tmp/daemonize/
WORKDIR /tmp/daemonize
RUN ./configure && make && make install

# --- The C sources ----------------------------------------------------------
# Only what the compile reads. Nothing under cgi-bin/, public_html/, Modules/,
# perl/, crawlers/ or config/ is referenced by the Makefiles, so the Perl/web
# side is deliberately NOT copied into this stage.
WORKDIR ${BOITHOHOME}
COPY Makefile config.mk.template ./
COPY mk/ mk/
COPY src/ src/
COPY perlxs/ perlxs/
# config.mk selects the BLACK_BOX appliance build (-DBLACK_BOX, `bb` suffix).
# bin/ and lib/ are the output dirs the Makefiles write into.
RUN cp config.mk.template config.mk && mkdir -p bin lib cgi-bin crawlers usersystems

# --- mod_auth_boitho (Apache auth module gating the admin panel) -------------
# Compile only; the runtime stage installs the .so and the LoadModule line
# (the Makefile's `install` uses apxs -i -a, which needs httpd-devel).
RUN (cd src/mod_auth_boitho && make Apache2)

# --- Build Searchdaimon ------------------------------------------------------
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
# here; owner/mode are set in the runtime stage.
RUN make InitServices YumWrapper NetConfig repomodwrap host=searchdaimon3

##############################################################################
# runtime: artifacts + the source tree
##############################################################################
FROM base AS runtime

# Compiled artifacts. The Makefiles write into these directories of the tree:
# bin/ (daemons and tools), cgi-bin/ (the C CGIs next to the Perl ones),
# crawlers/<name>/*.so and usersystems/<name>/*.so (plugins), perlxs/ (the
# SD::Crawl XS module, used in place from blib/), setuid/ (helpers). src/ and
# lib/ (static .a) are build-time only and are left behind.
COPY --from=builder /usr/local/sbin/daemonize /usr/local/sbin/daemonize
COPY --from=builder ${BOITHOHOME}/bin/ ${BOITHOHOME}/bin/
COPY --from=builder ${BOITHOHOME}/cgi-bin/ ${BOITHOHOME}/cgi-bin/
COPY --from=builder ${BOITHOHOME}/crawlers/ ${BOITHOHOME}/crawlers/
COPY --from=builder ${BOITHOHOME}/usersystems/ ${BOITHOHOME}/usersystems/
COPY --from=builder ${BOITHOHOME}/perlxs/ ${BOITHOHOME}/perlxs/
COPY --from=builder ${BOITHOHOME}/setuid/ ${BOITHOHOME}/setuid/

# Apache auth module: what `apxs -i -a -n auth_boitho` would have done.
COPY --from=builder ${BOITHOHOME}/src/mod_auth_boitho/.libs/mod_auth_boitho.so \
        /usr/lib64/httpd/modules/mod_auth_boitho.so
RUN echo 'LoadModule auth_boitho_module modules/mod_auth_boitho.so' \
        > /etc/httpd/conf.modules.d/10-auth_boitho.conf

# The whole source tree: Perl CGIs, templates, Modules, crawler plugins, config,
# init scripts, SQL. Merged over the artifacts above (no name collisions: the
# tracked tree holds no binaries). This is the layer a Perl change invalidates.
COPY . ${BOITHOHOME}/
COPY ./config.mk.template ${BOITHOHOME}/config.mk
COPY ./config.mk.template /config.mk
COPY ./public_html/webclient2/config.pm.template ${BOITHOHOME}/public_html/webclient2/config.pm
COPY ./blackbox/bbdemo.boitho.com.conf /etc/httpd/conf.d/bbdemo.boitho.com.conf

WORKDIR ${BOITHOHOME}

# Runtime data dirs the daemons write to, then fix ownership: the tree was
# copied in as root but the daemons run as user 'boitho'.
RUN mkdir -p ${BOITHOHOME}/var ${BOITHOHOME}/logs /boithoData/lot \
 && chown -R boitho:boitho ${BOITHOHOME} /boithoData \
 && touch ${BOITHOHOME}/logs/bbdocumentWebAdd.log \
 && chown apache:apache ${BOITHOHOME}/logs/bbdocumentWebAdd.log \
                        ${BOITHOHOME}/crawlers \
 && install -d -o apache -g apache ${BOITHOHOME}/var/webclient2_tpl \
                                  ${BOITHOHOME}/var/webadmin_tmp \
 && install -o apache -g apache -m 600 ${BOITHOHOME}/cgi-bin/webadmin/.htpasswd \
                                       ${BOITHOHOME}/config/.htpasswd

# Install the setuid helpers as root 4755 (what the boithobb-setuid RPM does on
# the appliance). initwrapper runs /etc/init.d/<service>, so link the appliance
# init scripts there for the services it knows about (phonehome is left out on
# purpose: help.cgi treats its presence as "phone home installed").
RUN chown root:root ${BOITHOHOME}/setuid/* \
 && chmod 4755 ${BOITHOHOME}/setuid/* \
 && for s in boithoad boitho-bbdn searchdbb crawlManager suggest crawl_watch bbAutoUpdate; do \
      ln -s ${BOITHOHOME}/init.d/$s /etc/init.d/$s; \
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
