.SILENT :

all:	saffire

debug:
	cd src && $(MAKE) debug 

saffire:
	cd src && $(MAKE) saffire

clean:
	cd src && $(MAKE) clean

test:
	for i in ./tests/*.sf; do \
	  ./tests/sf-unittest.sh $$i; \
	done

.PHONY:	all debug saffire clean test
