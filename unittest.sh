#!/bin/sh

export SAFFIRE_TEST_BIN=./src/saffire

if [ ! -e $SAFFIRE_TEST_BIN ] ; then
	echo "Please build saffire before running this test script";
	exit 1;
fi

php unittests/unittester/run-saffire-tests.php ${1:-"unittests/tests/"}
