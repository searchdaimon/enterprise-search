# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Searchdaimon Enterprise Search ("ES") — an open-source enterprise/website search engine, ~200K lines, mostly C (~72%) and Perl (~19%) with Yacc/Lex query parsers. It crawls enterprise sources (SMB file shares, Exchange, SharePoint, SQL, websites), stores and indexes documents in custom on-disk structures, and serves ranked, ACL-filtered, faceted search through a web UI. Originally built by Boitho AS, hence the `boitho`/`bb` prefixes throughout.

The product ships as a CentOS-based virtual appliance; the source tree *is* the development environment when checked out on that appliance at `/home/boitho/boithoTools`.

## Building

**The build only works on the appliance (or the provided Docker image), not on a generic dev machine.** `mk/setup.mk` selects compiler and library paths with `$(shell hostname)` and only has branches for hostnames `searchdaimon`, `searchdaimon3`, `bbh-001.boitho.com`, and `bbh-002.boitho.com`. On any other host the build has no MySQL/BDB/Perl/libconfig paths and fails. `Dockerfile` (based on Amazon Linux 2023) provisions a working build environment; use it for local work. On an **unrecognized host** (CI runner, Docker build), force the portable `searchdaimon3` profile — which derives everything from `mysql_config`/`pkg-config`/`perl -MExtUtils::Embed` instead of hard-coded paths — by passing it on the command line: `make <target> host=searchdaimon3` (a command-line var overrides the in-makefile `host=$(shell hostname)` and propagates to sub-makes via `MAKEFLAGS`).

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

## Testing

Tests live under `test/` (details in `test/README.md`). Three layers:

- **Unit tests** — `make test` (= `make -C test check`) builds TAP-emitting C test programs in `test/c/` against `lib/*.a` and the flex objects (so `make dep` first) and runs them through `prove` together with the Perl tests in `test/perl/t/`. `test/c/tap.h` is the whole C harness; add a test by dropping `test_foo.c` in `test/c/` and adding a rule to its Makefile. `test/perl/t/00-syntax.t` does `perl -c` on every `.pm`/`.pl`/`.cgi` under `cgi-bin/webadmin`, `public_html/webclient2`, `Modules`, `crawlers` and `perlxs`; files that legitimately cannot compile here are listed with a reason in `test/perl/syntax-skip.txt` (currently: `Modules/SD/Lot.pm` needs a nonexistent `SD::Lot::XS`; `ResultParse/SDOld.pm` is legacy; `crawlers/perlSMB` and `crawlers/wikipediadump` need CPAN modules not in the image). `test/perl/lib/SD/Crawl.pm` is a stub so the crawler plugins compile; the real XS module only loads inside crawlManager2, which is also why `perlxs/SD-Crawl/t/SD-Crawl.t` skips itself.
- **Smoke test** — `test/smoke/smoke.sh` runs *inside* the appliance (TAP, exit code = failures). From the host: `make -C test smoke-compose` (= `docker compose exec -T es bash test/smoke/smoke.sh`); the top-level `make smoke` does the same but, like every top-level target, needs `BOITHOHOME` and `config.mk`. It checks the daemons via `pidof`, the landing/search/admin/suggest endpoints and their auth, a dispatcher JSON query, then pushes an HTML document through the REST API (`POST /documents/<coll>/<url>`), waits for it in a dispatcher query, loads the anonymous search UI for it (must show the hit with no `Software error`, `Warning:` or untranslated `I18N_error`), loads every admin page in `test/smoke/admin-pages.txt` (GET only, 200 and no error page; `XFAIL <reason>: ` marks a known-broken page as TODO), and deletes the collection. Each run uses its own collection name so runs can never see each other's data, and the deletion step asserts that no lot or `iindex` directory for the collection survives and that the dispatcher no longer lists it.
- **CI** — `.github/workflows/ci.yml`: `docker build --target test` (the Dockerfile `test` stage = builder + the Perl tree + `config/` + libasan/libubsan, running `make -C test check SANITIZE=1` with `ASAN_OPTIONS=detect_leaks=0`), then the runtime image, `docker compose up`, and the smoke test.

Iterating on C fixes without a full image rebuild: the runtime image has gcc but not `lib/*.a` or the -devel headers, so compile in a `builder`/`test`-stage container, or (old single-stage containers) `docker cp` the source in and run `cd src/<dir> && make clean && make host=searchdaimon3 BDB_INC=... BDB_LIB=...` as `boitho` — `host=` must be explicit because `hostname` is not on PATH under `su -c`. Daemon stdout is lost under `daemonize`; run `stdbuf -oL bin/<daemon>` detached to see it, or `gdb -batch -ex "set follow-fork-mode child" -ex run -ex bt` (searchd forks per query).

The old ad-hoc `*Test/` dirs and `perl/attack.pl`-style load scripts are not wired in.

## Running in Docker

`Dockerfile` + `docker-compose.yml` build and run the whole appliance. `docker compose up -d --build` brings it up; the web UI is on **http://localhost:8080** (`/` landing, `/public` anonymous search, `/admin` panel — default login `admin` / `water66`).

Key facts:
- **Multi-stage image** (`base` → `builder` → `runtime`, plus a `test` stage off `builder`). `base` holds the runtime packages (httpd, MariaDB client, the daemons' shared libs, Perl + modules) **and**, in its own layer, the small toolchain cpanm needs to build XS modules (gcc, gcc-c++, make, perl-devel, ExtUtils::MakeMaker) — which is how SOAP::Lite and HTML::Strip (Sharepoint/Entropysoft connectors, `getacl.pl`) get in. Keep that toolchain line separate from the two big `dnf install` lines: changing those re-downloads 442 packages, and the AL2023 CDN can be very slow from some networks. `builder` adds the toolchain and compiles via `make build host=searchdaimon3` (the appliance `all` target's service-stop/`dbupdate`/start steps don't belong in an image build) — but it receives **only `src/`, `mk/`, `Makefile`, `config.mk.template` and `perlxs/`**, which is everything the Makefiles read. `runtime` copies the artifacts out of `builder` (`bin/`, the C CGIs in `cgi-bin/`, `crawlers/*/*.so`, `usersystems/*/*.so`, `perlxs/`, `setuid/`, `daemonize`, `mod_auth_boitho.so`) and then `COPY .` for the Perl/web side. Consequences: a **Perl-only change rebuilds in seconds** (only the final COPY and the cheap steps after it re-run); a **new C input outside those directories needs a COPY added to the builder stage** or it silently won't be there; likewise anything the Perl tests read at compile time must be COPYed into the `test` stage (that is why it copies `config/`: `webadmin/config.pm` opens `config/setup.txt` while compiling). `.dockerignore` keeps `.git`, installer media and host build outputs out of the context (`.git` alone used to invalidate the compile on every commit).
- **Two containers.** MariaDB runs in its own `db` service (`mariadb:10.11`, latin1, **`--sql-mode=NO_ENGINE_SUBSTITUTION`** — the legacy code relies on non-strict coercion, e.g. admin forms post `""` for int columns). The app finds it via `BOITHO_MYSQL_HOST=db`: the C code reads the env var directly, the entrypoint writes it into `config/setup.txt` for Perl and adds a `SetEnv` to the vhost so daemons started from the admin panel's "Manage services" page inherit it too (without it they fall back to localhost and crash-loop under `everrun`).
- **No systemd.** The image `CMD` is `docker/entrypoint.sh`: wait for the `db` container → seed the DB (`blackbox/boithodbsetup`) → `rpcbind` → httpd → the appliance's own `init.d/*` daemons, then `exec tail -F` the logs to stdout (so `docker logs` / ECS-CloudWatch / k8s capture them). DB seeding and daemon startup happen at **container runtime**, not build time.
- **Data** persists in named volumes `boithodata` (`/boithoData`, the index/lots) and `mariadb-data` (`/var/lib/mysql`, the `boithobb` config DB) — keep them together.
- Appliance-vs-container gaps closed in the Dockerfile/entrypoint: `/run/httpd` created before httpd starts; `config/.htpasswd` seeded (admin login); `Require all granted` in the vhost (Apache 2.4 default-deny); apache-owned Template-Toolkit cache dirs `var/webclient2_tpl` + `var/webadmin_tmp`; the **setuid helpers** the admin panel needs (`setuid/initwrapper`, `yumwrapper`, `configwrite`, `repomodwrap` — on the appliance from the `boithobb-setuid` RPM) built via `make InitServices YumWrapper NetConfig repomodwrap` and installed root `4755`, with the `init.d/*` scripts symlinked into `/etc/init.d/` where `initwrapper` expects them; `procps-ng` (the init scripts need `pidof`/`ps` for status/stop); `/coredumps/saved` for the Crashes page. The network settings page (`settings.cgi?view=network`, and the setup wizard's network step) degrades to an explanation instead of a 500 when the appliance network files (`ifcfg-ethN`, `resolv.conf`, `setuid/configwrite`) are absent, which they are in a container. The phone-home helper (`bin/setuidcaller`) is deliberately not built; `help.cgi` shows "not installed".
- **Live Perl edits** without a rebuild: `docker cp` the file into `enterprise-search:/home/boitho/boithoTools/...` (CGIs are read per request), or bind-mount a sub-path — never the whole tree, which would shadow the built binaries.

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
- **64-bit pointer truncation is a recurring bug class.** The tree was written for 32-bit; several places cast pointers through `int` or let bison's default `int` YYSTYPE carry `char*` token text. Fixed so far: `sfindductype()` in `src/common/bfileutil.c` (crashed every REST push) and `YYSTYPE` in `src/parser2/*_common.h` (crashed searchd on any query with hits). A segfault on an address like `0xffffffff8800....` is this; grep the Latin-1 sources with `grep -a` (plain grep treats them as binary and prints nothing).
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
