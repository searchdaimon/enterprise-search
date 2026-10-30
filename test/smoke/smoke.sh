#!/bin/bash
#
# End-to-end smoke test for a running Searchdaimon appliance.
#
# Runs INSIDE the appliance (the Docker container or a real box): it checks the
# daemons with pidof and talks to httpd on localhost. Output is TAP, so it can be
# run under prove as well as on its own. Exit status is the number of failures.
#
#   docker compose exec -T es bash test/smoke/smoke.sh      # from the host
#   make smoke                                              # same thing
#
# What it exercises, in order:
#   1. every daemon the init scripts start is alive
#   2. the landing page, anonymous search page and admin panel respond, and the
#      admin panel and suggest endpoint demand authentication
#   3. the dispatcher answers a JSON query (which needs searchd + the config DB)
#   4. a document pushed through the REST API (/documents/<coll>/<url>) becomes
#      searchable: bbdocumentWebAdd -> boitho-bbdn -> repository -> searchd ->
#      dispatcher_allbb
#   5. the anonymous search UI renders that hit without Perl warnings or
#      untranslated strings
#   6. every page of the admin panel renders (GET, see admin-pages.txt)
#   7. the test collection can be deleted again
#
# Environment overrides:
#   SMOKE_BASE_URL     default http://127.0.0.1
#   SMOKE_ADMIN_USER   default admin
#   SMOKE_ADMIN_PASS   default water66
#   SMOKE_TIMEOUT      seconds to wait for the pushed document (default 120)
#   SMOKE_KEEP         set to 1 to leave the test collection behind

BASE=${SMOKE_BASE_URL:-http://127.0.0.1}
ADMIN_USER=${SMOKE_ADMIN_USER:-admin}
ADMIN_PASS=${SMOKE_ADMIN_PASS:-water66}
TIMEOUT=${SMOKE_TIMEOUT:-120}
# Letters only: the query tokenizer may split digits from letters. The token
# also names the collection, so a run never sees another run's leftovers.
token="smoke$(date +%s | tr '0-9' 'a-j')"
COLL="$token"

ran=0; failed=0
ok()     { ran=$((ran+1)); echo "ok $ran - $1"; }
not_ok() { ran=$((ran+1)); failed=$((failed+1)); echo "not ok $ran - $1"; shift; for l in "$@"; do echo "#   $l"; done; }
check()  { if [ "$1" -eq 0 ]; then ok "$2"; else shift; not_ok "$@"; fi; }
diag()   { echo "# $*"; }

http_code() { curl -s -o /dev/null -w '%{http_code}' "$@"; }

# Fully buffered pidof output lands in CI logs; keep it simple.
# --- 1. daemons --------------------------------------------------------------
for p in httpd searchdbb boitho-bbdn boithoad crawlManager2 suggest_server; do
	if pidof -x "$p" >/dev/null 2>&1; then ok "daemon $p is running"; else not_ok "daemon $p is running" "pidof $p found nothing"; fi
done

# --- 2. web endpoints --------------------------------------------------------
code=$(http_code "$BASE/")
case "$code" in 200|301|302) ok "landing page / answers ($code)";; *) not_ok "landing page / answers" "got HTTP $code";; esac

code=$(http_code "$BASE/webclient2/indexanon.pl")
[ "$code" = 200 ] && ok "anonymous search page answers 200" || not_ok "anonymous search page answers 200" "got HTTP $code"

code=$(http_code "$BASE/cgi-bin/webadmin/overview.cgi")
[ "$code" = 401 ] && ok "admin panel requires authentication" || not_ok "admin panel requires authentication" "got HTTP $code, expected 401"

code=$(http_code -u "$ADMIN_USER:$ADMIN_PASS" "$BASE/cgi-bin/webadmin/overview.cgi")
[ "$code" = 200 ] && ok "admin panel overview answers 200 with the default login" || not_ok "admin panel overview answers 200 with the default login" "got HTTP $code"

code=$(http_code "$BASE/cgi-bin/suggest_webclient?q=te")
[ "$code" = 401 ] && ok "suggest endpoint requires authentication" || not_ok "suggest endpoint requires authentication" "got HTTP $code, expected 401"

# --- 3. dispatcher JSON ------------------------------------------------------
# Parse with perl's JSON module (installed for the web UI); prints total or "ERR".
json_total() {
	perl -MJSON -e '
		local $/; my $t = <STDIN>;
		my $j = eval { decode_json($t) };
		if (!$j || !ref $j->{results_info}) { print "ERR\n"; exit 0 }
		print $j->{results_info}{total}, "\n";
	'
}
query_url() { echo "$BASE/cgi-bin/dispatcher_allbb?query=$1&anonymous=1&outformat=json"; }

body=$(curl -s "$(query_url zzqqnothingmatches)")
total=$(printf '%s' "$body" | json_total)
if [ "$total" = "ERR" ]; then
	not_ok "dispatcher answers a JSON query" "response was not dispatcher JSON:" "$(printf '%s' "$body" | head -c 300 | tr '\n' ' ')"
else
	ok "dispatcher answers a JSON query (total=$total)"
fi

# --- 4. push a document and find it ------------------------------------------
docurl="http://smoke.test/$token.html"
doc="<html><head><title>Smoke test $token</title></head><body><p>This page was pushed by the smoke test. Marker: $token.</p></body></html>"

resp=$(curl -s -w '\n%{http_code}' -X POST -H 'Content-Type: text/html' --data-binary "$doc" "$BASE/documents/$COLL/$docurl")
code=$(printf '%s' "$resp" | tail -n1)
if [ "$code" = 200 ]; then
	ok "REST push of $docurl into collection $COLL returns 200"
else
	not_ok "REST push of $docurl into collection $COLL returns 200" "got HTTP $code" "$(printf '%s' "$resp" | head -n -1 | head -c 300 | tr '\n' ' ')"
fi

found=0; waited=0
while [ $waited -lt "$TIMEOUT" ]; do
	body=$(curl -s "$(query_url "$token")")
	total=$(printf '%s' "$body" | json_total)
	if [ "$total" != "ERR" ] && [ "$total" -ge 1 ] 2>/dev/null; then found=1; break; fi
	sleep 2; waited=$((waited+2))
done
if [ $found = 1 ]; then
	ok "pushed document is searchable after ${waited}s"
	if printf '%s' "$body" | grep -qF "$docurl"; then
		ok "search hit carries the pushed URL"
	else
		not_ok "search hit carries the pushed URL" "$(printf '%s' "$body" | head -c 500 | tr '\n' ' ')"
	fi
else
	not_ok "pushed document is searchable within ${TIMEOUT}s" "last total=$total" "$(printf '%s' "$body" | head -c 300 | tr '\n' ' ')"
	not_ok "search hit carries the pushed URL" "skipped: document never showed up"
fi

# --- 4b. the search UI renders the hit without Perl warnings -----------------
# index-common.pm turns every Perl warning into a visible "Software error" block
# and every missing translation into the literal I18N_error, so a clean page is a
# real assertion. Checked in the default language and in English.
for lang in "" "en"; do
	q="$BASE/webclient2/indexanon.pl?query=$token"; [ -n "$lang" ] && q="$q&lang=$lang"
	label="search UI page for the pushed document${lang:+ (lang=$lang)}"
	body=$(curl -s -m 60 "$q")
	if ! printf '%s' "$body" | grep -qF "$docurl"; then
		not_ok "$label shows the hit" "pushed URL not in the page"
	elif printf '%s' "$body" | grep -qE 'Software error|<pre>Warning: |I18N_error'; then
		not_ok "$label renders without errors" "$(printf '%s' "$body" | grep -oE '<pre>Warning: [^<]{0,160}|I18N_error' | sort -u | head -n 3 | tr '\n' ';')"
	else
		ok "$label renders the hit without errors"
	fi
done

# --- 6. every admin page renders -----------------------------------------------
# The perl -c sweep in test/perl proves the admin CGIs compile; this proves they
# run. GET only, see admin-pages.txt. The Apache error log is diffed around the
# crawl and new stderr lines are shown as diagnostics (not failures: the legacy
# code warns a lot).
PAGES=$(dirname "$0")/admin-pages.txt
if [ -r "$PAGES" ]; then
	errlog=/var/log/httpd/error_log
	errstart=$( [ -r $errlog ] && wc -l < $errlog || echo 0 )
	while IFS= read -r line; do
		case "$line" in \#*|"") continue;; esac
		xfail=""; path="$line"
		case "$line" in XFAIL*) xfail=$(printf '%s' "$line" | sed 's/^XFAIL \(.*\): .*/\1/'); path=$(printf '%s' "$line" | sed 's/^XFAIL .*: //');; esac
		resp=$(curl -s -m 60 -w '\n%{http_code}' -u "$ADMIN_USER:$ADMIN_PASS" "$BASE/cgi-bin/webadmin/$path")
		code=$(printf '%s' "$resp" | tail -n1)
		body=$(printf '%s' "$resp" | sed '$d')
		problem=""
		[ "$code" = 200 ] || problem="HTTP $code"
		if [ -z "$problem" ] && printf '%s' "$body" | grep -qE '<h1>Software error:|<pre>Warning: |<title>500 Internal Server Error|Template .* error|I18N_error'; then
			problem="error page: $(printf '%s' "$body" | grep -oE '<pre>[^<]{0,160}|I18N_error' | head -n1 | sed 's/<pre>//')"
		fi
		if [ -n "$xfail" ]; then
			if [ -z "$problem" ]; then ok "admin page $path renders # TODO passes now, remove XFAIL ($xfail)"; else ok "admin page $path renders # TODO $xfail: $problem"; fi
		elif [ -z "$problem" ]; then
			ok "admin page $path renders"
		else
			not_ok "admin page $path renders" "$problem"
		fi
	done < "$PAGES"
	if [ -r $errlog ]; then
		new=$(tail -n +$((errstart+1)) $errlog | grep -c "stderr from" || true)
		diag "$new new stderr line(s) from the admin CGIs during the crawl"
		tail -n +$((errstart+1)) $errlog | grep "stderr from" | sed 's/.*stderr from [^:]*: //' | sort | uniq -c | sort -rn | head -n 8 | while read -r l; do diag "  $l"; done
	fi
else
	diag "admin-pages.txt not found next to $0; skipping the admin page crawl"
fi

# --- 7. clean up -------------------------------------------------------------
if [ "${SMOKE_KEEP:-0}" = 1 ]; then
	diag "SMOKE_KEEP=1: leaving collection $COLL in place"
	ok "collection cleanup skipped on request"
else
	code=$(http_code -X DELETE "$BASE/documents/$COLL/")
	[ "$code" = 200 ] && ok "test collection $COLL deleted" || not_ok "test collection $COLL deleted" "got HTTP $code"
	body=$(curl -s "$(query_url zzqqnothingmatches)")
	if printf '%s' "$body" | grep -qF "\"$COLL\""; then not_ok "dispatcher no longer lists the deleted collection" "still in the collections list"; else ok "dispatcher no longer lists the deleted collection"; fi
	if [ -d /boithoData ]; then
		left=$(find /boithoData -maxdepth 4 -name "$COLL" 2>/dev/null)
		if [ -z "$left" ]; then ok "deleting the collection removed its lot and iindex directories"; else not_ok "deleting the collection removed its lot and iindex directories" "left behind:" $left; fi
	fi
fi

echo "1..$ran"
[ $failed -gt 0 ] && echo "# $failed of $ran checks failed"
exit $failed
