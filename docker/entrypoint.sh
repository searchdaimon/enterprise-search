#!/bin/bash
#
# Container entrypoint: bring up the Searchdaimon appliance WITHOUT systemd.
#
# Modern hosts (Amazon Linux 2023, Docker Desktop) use cgroup v2, on which the
# CentOS-style systemd init won't boot. This script starts the same services
# systemd would, in the same order, so the image runs anywhere. It reuses the
# appliance's own init.d scripts to launch (and everrun-supervise) the daemons;
# only the orchestration differs.
#
# The MariaDB server runs in its own container (service `db` in
# docker-compose.yml); this script waits for it, points the app at it, and seeds
# the schema. BOITHO_MYSQL_HOST selects the DB host (default: localhost).

export BOITHOHOME=/home/boitho/boithoTools
export BOITHO_MYSQL_HOST="${BOITHO_MYSQL_HOST:-localhost}"
log() { echo "[entrypoint] $*"; }

log "using database host: $BOITHO_MYSQL_HOST"

# Point the Perl side at the database host. The C daemons read BOITHO_MYSQL_HOST
# directly; the Perl modules read the server from config/setup.txt, so rewrite it
# here to match.
if [ -f "$BOITHOHOME/config/setup.txt" ]; then
	sed -ri "s/^server=.*/server=${BOITHO_MYSQL_HOST}/" "$BOITHOHOME/config/setup.txt"
fi

log "waiting for MariaDB at $BOITHO_MYSQL_HOST"
for i in $(seq 1 60); do
	mysqladmin ping -h "$BOITHO_MYSQL_HOST" --silent 2>/dev/null && break
	sleep 1
done
log "MariaDB: $(mysqladmin ping -h "$BOITHO_MYSQL_HOST" 2>&1 | head -1)"

log "starting rpcbind (required by suggest_server's SunRPC registration)"
/sbin/rpcbind 2>/dev/null || rpcbind 2>/dev/null || true

log "seeding database (blackbox/boithodbsetup)"
sh "$BOITHOHOME/blackbox/boithodbsetup" || true

log "starting httpd"
# Route Apache's logs to files (the appliance vhost logs to syslog:local6, which
# is lost in a container without rsyslog) so they can be streamed to stdout
# below. This rewrites only the in-container config copy; the image/source vhost
# is unchanged.
sed -ri 's#^[[:space:]]*ErrorLog .*#ErrorLog /var/log/httpd/error_log#; s#^[[:space:]]*CustomLog .*#CustomLog /var/log/httpd/access_log common#' \
	/etc/httpd/conf.d/bbdemo.boitho.com.conf
# Daemons started from the admin panel's "Manage services" page inherit Apache's
# CGI environment, so the database host has to be visible there as well: the C
# daemons read BOITHO_MYSQL_HOST and otherwise fall back to localhost, exit, and
# everrun respawns them forever while the page reports them as running.
grep -q '^SetEnv BOITHO_MYSQL_HOST ' /etc/httpd/conf.d/bbdemo.boitho.com.conf \
	|| sed -i "/^SetEnv BOITHOHOME /a SetEnv BOITHO_MYSQL_HOST ${BOITHO_MYSQL_HOST}" \
		/etc/httpd/conf.d/bbdemo.boitho.com.conf
# httpd needs its runtime dir for the pid/lock; with no systemd-tmpfiles here it
# is not created automatically, and httpd then fails to start. (Don't hide the
# error — let it surface in the streamed logs.)
mkdir -p /run/httpd
/usr/sbin/httpd -k start || log "WARNING: httpd failed to start"

log "starting Searchdaimon daemons"
for s in boithoad boitho-bbdn searchdbb crawlManager suggest crawl_watch; do
	"$BOITHOHOME/init.d/$s" start || true
done

log "startup complete; streaming logs to stdout (docker logs / CloudWatch)."
# Stream the key logs to PID 1's stdout so they appear in `docker logs` and on
# any platform that captures container stdout (AWS ECS awslogs -> CloudWatch,
# Kubernetes, etc.) — no files to scrape. -F retries files not present yet;
# daemon logs land under $BOITHOHOME/logs and $BOITHOHOME/var.
mkdir -p /var/log/httpd
touch /var/log/httpd/error_log /var/log/httpd/access_log
LOGS="/var/log/httpd/error_log /var/log/httpd/access_log"
for f in "$BOITHOHOME"/logs/*.log "$BOITHOHOME"/var/*.log; do [ -f "$f" ] && LOGS="$LOGS $f"; done
exec tail -n0 -F $LOGS 2>/dev/null
