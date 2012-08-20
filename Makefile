.SILENT:

all:
	echo "Welcome to the Saffire Makefile"
	echo "Usage: make <saffire|debug|docs|tests>"
	echo
	echo "make clean        Removes temporary objects and files"
	echo "make saffire      Creates the saffire binary"
	echo "make debug        Creates a debug version of the saffire binary"
	echo "make docs         Generates (html) documentation"
	echo "make tests        Runs the saffire tests suite"
	echo

docs:
	make -C docs/spec html

clean:
	make -C src clean

saffire:
	make -C src saffire

debug:
	make -C src debug

tests:
	make -C examples all

.PHONY: docs clean tests
