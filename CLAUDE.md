# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Searchdaimon Enterprise Search ("ES") — an open-source enterprise/website search engine, ~200K lines, mostly C (~72%) and Perl (~19%) with Yacc/Lex query parsers. It crawls enterprise sources (SMB file shares, Exchange, SharePoint, SQL, websites), stores and indexes documents in custom on-disk structures, and serves ranked, ACL-filtered, faceted search through a web UI. Originally built by Boitho AS, hence the `boitho`/`bb` prefixes throughout.

The product ships as a CentOS-based virtual appliance; the source tree *is* the development environment when checked out on that appliance at `/home/boitho/boithoTools`.

## Building

**The build only works on the appliance (or the provided Docker image), not on a generic dev machine.** `mk/setup.mk` selects compiler and library paths with `$(shell hostname)` and only has branches for hostnames `searchdaimon`, `searchdaimon3`, `bbh-001.boitho.com`, and `bbh-002.boitho.com`. On any other host the build has no MySQL/BDB/Perl/libconfig paths and fails. `Dockerfile` (based on `centos:7`) provisions a working build environment; use it for local work. On an **unrecognized host** (CI runner, Docker build), force the portable `searchdaimon3` profile — which derives everything from `mysql_config`/`pkg-config`/`perl -MExtUtils::Embed` instead of hard-coded paths — by passing it on the command line: `make <target> host=searchdaimon3` (a command-line var overrides the in-makefile `host=$(shell hostname)` and propagates to sub-makes via `MAKEFLAGS`).

Prerequisites before any build:
- `cp config.mk.template config.mk` — sets `WITH_BLACKBOX=yes`, which adds `-DBLACK_BOX` and the `bb` binary suffix. Required; `mk/setup.mk` includes it.
- `export BOITHOHOME=<repo root>` (the appliance uses `/home/boitho/boithoTools`). Every Makefile resolves includes and `lib/`, `bin/`, `cgi-bin/` output dirs relative to this.
- Build/run as user `boitho`.

Commands (run from repo root):
- `make dep` — **run this first.** Builds the shared static libs (`libcommon`, `libds`, `libgetdate`, query lexer, snippet generator, perlembed, etc.) into `lib/`. The daemon builds link against these; they will not build without it.
- `make all` — full appliance rebuild. **Destructive on a live box:** it runs `init.d.stop` (stops all daemons), `dbupdate` (applies `sql/update-cur.sql` to the `boithobb` MySQL database), `dep`, builds every production binary, then `init.d.start`. Don't run casually; build individual targets instead.
- `make build` — **compile-only** target (factored out of `all`): builds `dep` + every production binary but does *not* stop/start services or run `dbupdate`. Use this for container/CI builds where no MySQL or daemons are running. (The Docker image uses `make build host=searchdaimon3`.)
- `make <target>` — build one component (e.g. `make searchdbb`, `make dispatcher_allbb`, `make IndexerLotbb`). Targets and their source dirs are defined in the top-level `Makefile`.

Two target styles coexist in the top `Makefile`:
- **Inline** targets compile directly with `$(CC) ... $(CFLAGS) $(LIBS)*.c ...` (older style; e.g. `boithoad`, most `read*`/`Brank*` tools).
- **Delegating** targets do `(cd src/X && make clean && make)` against a subdir `Makefile` that includes `mk/prog.mk` (newer style; e.g. `searchdbb` → `src/searchkernel`, `dispatcher_allbb` → `src/dispatcher_all`, `boitho-bbdn`, `crawlManager2`, `infoquery`, `Suggest`, `gcAuthoritybb`). For these, rebuild a single component fast with `cd src/<dir> && make`.

**`bb` suffix = the `-DBLACK_BOX` appliance build.** The `all` target builds the `bb` variants (`searchdbb`, `IndexerLotbb`, `dispatcher_allbb`, …); the non-`bb` names are legacy/internal variants of the same source. Prefer the `bb` targets.

There is no automated test suite or `make test`. Testing is ad hoc: `*Test/` source dirs (e.g. `src/boithoadtest`, `src/searchfilterTest`), load/attack Perl scripts in `perl/` (`attack.pl`, `queryHammer.pl`), and one `perlxs/SD-Crawl/t/SD-Crawl.t`.

## Running in Docker

`Dockerfile` + `docker-compose.yml` build and run the whole appliance. `docker compose up -d --build` brings it up; the web UI is on **http://localhost:8080** (`/` landing, `/public` anonymous search, `/admin` panel — default login `admin` / `water66`).

Key facts:
- The image compiles via `make build host=searchdaimon3` (the appliance `all` target's service-stop/`dbupdate`/start steps don't belong in an image build). DB creation/seeding and daemon startup happen at **container runtime**, not build time.
- **systemd vs cgroup v2:** the image's default `CMD` is `/usr/sbin/init` (faithful to the appliance), but CentOS 7's systemd (v219) needs **cgroup v1** and won't boot on a cgroup-v2 host like Docker Desktop ("Failed to get D-Bus connection"). So `docker-compose.yml` overrides the command with `docker/entrypoint.sh`, a **non-systemd** launcher that starts MariaDB → seeds the DB (`blackbox/boithodbsetup`) → `rpcbind` → httpd → the appliance's own `init.d/*` daemons, and `exec tail -F`s the logs to stdout (so `docker logs` / ECS-CloudWatch / k8s capture them). On a real cgroup-v1 host you can instead run the image's systemd `CMD` (the `init.d` services are `chkconfig`-registered).
- **Data** persists in named volumes `boithodata` (`/boithoData`, the index/lots) and `mariadb-data` (`/var/lib/mysql`, the `boithobb` config DB) — keep them together.
- Several appliance-vs-container gaps were fixed in the Dockerfile/entrypoint: create `/run/httpd` before httpd starts; seed `config/.htpasswd` (admin login); `Require all granted` in the vhost (Apache 2.4 default-deny); apache-owned Template-Toolkit cache dirs `var/webclient2_tpl` + `var/webadmin_tmp`.

## Runtime architecture

Four long-running C daemons plus a CGI front controller, all wrapped by `bin/everrun` (a trivial restart-on-crash loop) and managed by `init.d/*` scripts via `daemonize`. Ports are defined in `config/main.conf`:

| Daemon | Source | Port | Role |
|---|---|---|---|
| `searchdbb` | `src/searchkernel/` | 7505 | Search: queries the inverted index, ranks/filters results |
| `boitho-bbdn` | `src/boitho-bbdn/` | 7390 | Document/"lot" daemon: receives crawled docs, stores them in the repository, assigns DocIDs |
| `boithoad` | `src/boithoad/` | 7391 | Authentication: resolves users/groups against LDAP/Active Directory for ACL filtering |
| `crawlManager2` | `src/crawlManager2/` | 7392 | Orchestrates crawlers and collection scheduling |

- `dispatcher_all` / `dispatcher_allbb` (`src/dispatcher_all/`, a CGI in `cgi-bin/`) is the search **front controller**: parses the HTTP query, rewrites it, calls `searchdbb`, checks ACLs via `boithoad`, and emits HTML/XML/JSON/OpenSearch.
- `Suggest` (`src/suggestrpc/` server + `src/suggestwebclient/` CGI) provides autocomplete.
- Web UI is **Perl CGI**, not PHP: `public_html/webclient2/` (search UI) and `cgi-bin/webadmin/` (admin panel). `src/mod_auth_boitho/` is an Apache auth module that gates admin pages by calling `boithoad`.

## Core concepts

- **Lot** — the unit of index partitioning. Documents are bucketed into fixed-size "lots"; `config/lotlist.conf` maps lot ranges to backend nodes, which is how the index scales horizontally across a cluster. Anything named `*Lot*` (e.g. `IndexerLot`, `LotInvertetIndexMaker`, `readLot`, `netlot`) operates on these partitions. See `src/common/lot.c` and `src/common/lotlist.c`.

- **Indexing pipeline** (crawl → searchable):
  `crawlManager2` → crawler connectors → `boitho-bbdn` (writes documents to the *reposetory*, assigns DocIDs) → `IndexerLot` (tokenize, stem, build per-lot reverse index) → `LotInvertetIndexMaker` (merge into the inverted index) → `searchdbb` serves it → `dispatcher_all` → web UI. Intermediate on-disk artifacts include the *reposetory* (compressed document store), *nyeurler* ("new URLs" queue, Norwegian), *revindex*, *DocumentIndex* (per-DocID metadata), and *udfile*.

- **Crawler plugins** — hybrid C + Perl. Performance-critical connectors are C (`src/crawlSMB`, `src/crawlExchange`, `src/crawlPush`). Custom/extensible connectors are Perl modules under `crawlers/` (Alfresco, Sharepoint, Intranet, Wordpress, twitter, …), loaded via an embedded Perl interpreter in `src/crawlManager2/perlcrawl.c`. New connectors are normally written as Perl plugins.

- **MySQL/MariaDB (`boithobb` database) vs. custom files** — the database holds *configuration and state*: connector definitions, collections (`shares` table), user/system mapping, query logs. Document content and all search indexes live in the *custom on-disk binary files* described above, not in SQL. Schema/migrations: `sql/`, `meta/boitho.sql`, `blackbox/boithobb.sql`.

## Conventions and gotchas

- **Norwegian naming.** Comments and many identifiers are Norwegian or Norwegian-spelled: `nyeurler` = new URLs, `reposetory` = repository, `komando`/`kamando` = command, `autentifisering` = authentication, `Brank` = rank. Expect this when grepping.
- **Latin-1 (ISO-8859-1) source files — editing hazard.** Many `.c`/`.h`/`.sql` files and the top-level `Makefile` contain Latin-1 Norwegian comments (`å`/`ø`/`æ` as single high bytes `0xE5`/`0xF8`/`0xE6`). A Read→Edit cycle that normalizes to UTF-8 silently re-encodes those bytes — turning `å` into the replacement char `�`, destroying the text and polluting the diff (even on lines you didn't touch). When editing a file that has any non-ASCII byte, edit **byte-preservingly** (`sed`/`perl` operate on bytes; or Python with `encoding="latin-1"`, which round-trips all 256 byte values) and verify with `git diff --numstat` that only the intended lines changed.
- **`boitho` / `bb`** prefixes are the legacy company name and the BLACK_BOX appliance build — not separate subsystems.
- **Embedded Perl in C** — `crawlManager2` and `boitho-bbdn` link an embedded Perl interpreter (`WANT_PERLEMBED`, `src/perlembed/`). C and Perl are deeply interleaved, not cleanly separated layers.
- **Query parsing** uses Lex/Yacc: `src/query/` (query lexer/stemmer), `src/parser2/` (HTML/CSS parsers), `src/utf8-filter/`. These are prebuilt by `make dep` into `.o`/`.a` artifacts that the daemons link.
- Per-component config lives in `config/*.conf` (`main.conf`, `searchd.conf`, `crawlmanager.conf`, `dispatcher.conf`, `lotlist.conf`, `maplist.conf`).
- **SQL dumps use the removed `TYPE=<engine>` clause.** `blackbox/boithobb.sql` and `sql/update-cur.sql` are MySQL 3.23/4-era dumps (`TYPE=MyISAM`), which MariaDB 5.5+ rejects → tables silently don't get created (e.g. `boithobb.activeUsers` missing → admin panel 500s). `blackbox/boithodbsetup` rewrites `TYPE=`→`ENGINE=` at seed time, so **leave the `.sql` files unedited** (they're also Latin-1 — see above).
- **Case-insensitive-checkout collisions.** On a case-insensitive checkout (macOS/Windows), two repo paths differing only by case collapse to one physical file, producing a spurious `git status` `M` (one path's content lands at the other's path) — not a real edit; don't commit it. The known instance — capital-U `src/readNyeUrlerFil` (Makefile target `readNyeUrlerFil`, `main.c` only) vs an orphaned lowercase `src/readNyeurlerFil` (a different `main.c` + `temp.sh`, no target, referenced nowhere) — was resolved 2026-06-15 (commit `2c26f7f9`): the lowercase orphan was removed, the capital-U dir kept. To clear such a collision safely, `git rm --cached -c core.ignorecase=false` the unwanted path (a plain `git rm <dir>` would also match/delete its sibling on a case-insensitive FS), restore the kept path with `git checkout`, then `git commit` **without a pathspec** (`git commit -- <path>` re-trips the case alias).

## Git commit attribution

Commits in this repository should credit AI assistance with a co-author trailer.
When you create a commit, append the trailer:

    Co-Authored-By: Claude <noreply@anthropic.com>
