.SILENT :

all:	saffire

debug:
	cd src && $(MAKE) $@

saffire:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@

test:
	./src/saffire ./tests/001.sf
	./src/saffire ./tests/002.sf
	./src/saffire ./tests/003.sf

.PHONY:	all debug saffire clean test