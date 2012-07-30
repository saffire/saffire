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
	  echo "*** Testing: $$i" ; \
	  ./src/saffire $$i | php interpreter/interpreter.php ; \
	  echo "" ; \
	  echo "" ; \
	done

.PHONY:	all debug saffire clean test
