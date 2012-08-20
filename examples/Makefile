.SILENT:

all: testsuite

testsuite:
	for i in `find . -name \*.sf`; do echo $$i ; ../src/saffire "$$i"; done

.PHONY: testsuite