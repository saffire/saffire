.SILENT :

all:	saffire

debug:
	cd src && $(MAKE) $@

saffire:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@

test:
	for i in ./tests/*.sf; do \
	  ./sf-unittest.sh $$i; \
	done

.PHONY:	all debug saffire clean test
