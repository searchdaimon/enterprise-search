# Test driver. See test/README.md.
#
#   make -C test            build the C tests and run everything through prove
#   make -C test build      only compile the C tests
#   make -C test smoke      end-to-end check against a running appliance (run
#                           inside the container / on the appliance)
#   make -C test smoke-compose   the same from the host, via docker compose exec
#
# Needs BOITHOHOME (defaults to the repo root) and a completed `make dep`.

BOITHOHOME?=	$(abspath ..)
export BOITHOHOME

PROVE?=		prove

all: check

build:
	$(MAKE) -C c

check: build
	$(PROVE) perl/t/*.t c/bin/*
	@if [ -d $(BOITHOHOME)/perlxs/SD-Crawl/blib ]; then \
		(cd $(BOITHOHOME)/perlxs/SD-Crawl && $(PROVE) -b t); \
	else \
		echo "perlxs/SD-Crawl not built; skipping its tests"; \
	fi

smoke:
	bash smoke/smoke.sh

# Same thing from the host, against the compose stack.
smoke-compose:
	docker compose exec -T es bash test/smoke/smoke.sh

clean:
	$(MAKE) -C c clean

.PHONY: all build check smoke smoke-compose clean
