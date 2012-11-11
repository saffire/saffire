.SILENT:

all: testsuite

testsuite:
	for i in `find . -name \*.sf`; do echo $$i ; ../saffire/saffire "$$i"; done

.PHONY: testsuite