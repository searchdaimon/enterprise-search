# Tests

Three layers, cheapest first. Everything runs inside the Docker image (or on
the appliance), because that is the only place the C tree builds.

| Layer | What | Where | Run |
|---|---|---|---|
| C unit tests | pure leaf modules in `src/common`, the query lexer, the UTF-8 filter | `test/c/` | `make test` |
| Perl unit tests | `perl -c` of every Perl source, plus behaviour tests for pure modules | `test/perl/t/`, `perlxs/SD-Crawl/t/` | `make test` |
| Smoke test | daemons up, web endpoints answer, a pushed document becomes searchable | `test/smoke/smoke.sh` | `make smoke` |

## Unit tests

The C tests are small programs that print [TAP](https://testanything.org/),
built against the same static libraries (`lib/*.a`) and flex objects the daemons
link. `prove` runs them together with the Perl tests. There is no external test
framework: `test/c/tap.h` is the whole harness.

```sh
# in the container (docker compose exec es bash) or on the appliance:
make dep                       # once; builds lib/*.a
make test                      # = make -C test check
make -C test build SANITIZE=1  # ASan + UBSan build of the C tests
```

In CI the same thing happens in the Dockerfile's `test` stage:

```sh
docker build --target test .
```

Adding a C test: drop `test_foo.c` in `test/c/`, add a rule and a line in
`TESTS` in `test/c/Makefile`. Adding a Perl test: drop a `.t` in `test/perl/t/`.
A Perl file that legitimately cannot compile outside the appliance goes in
`test/perl/syntax-skip.txt` with a reason.

## Smoke test

`test/smoke/smoke.sh` runs inside the appliance and talks to httpd on
localhost. It pushes an HTML document through the REST API
(`POST /documents/<collection>/<url>`), waits for it to show up in a dispatcher
JSON query, loads the anonymous search UI for it (which must show the hit with
no Perl warning and no untranslated `I18N_error` string), then loads every page listed in
`test/smoke/admin-pages.txt` with the admin login and requires HTTP 200 and no
Perl or Apache error page; the `perl -c` sweep proves those CGIs compile, this
proves they run. Only idempotent GETs belong in that list. A page known to be
broken can be prefixed `XFAIL <reason>: ` and is then reported as TODO. New
lines Apache logged from the CGIs during the crawl are printed as diagnostics.

```sh
docker compose up -d
make -C test smoke-compose     # = docker compose exec -T es bash test/smoke/smoke.sh
# (`make smoke` at the top level does the same, but the top-level Makefile needs
# BOITHOHOME and config.mk like every other appliance target)
```

The script is TAP as well, so `prove test/smoke/smoke.sh` works where it runs.
Set `SMOKE_KEEP=1` to leave the `smoketest` collection behind for inspection.

## Gotchas

- Tests must run with `BOITHOHOME` set (the Makefiles default it to the repo
  root) and, for the smoke test, as a user that can read the daemons' pids.
- Keep fixtures ASCII. Many sources are Latin-1 and the Edit tooling will
  corrupt them; see `CLAUDE.md`.
- The `host=searchdaimon3` profile is the only portable build. Pass it the same
  way `make build` does when building the tests on an unknown host.
