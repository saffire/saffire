#!/bin/sh

export SAFFIRE_TEST_BIN=./build/release/saffire

if [ ! -e $SAFFIRE_TEST_BIN ] ; then
	echo "Please build saffire before running this test script";
	exit 1;
fi

DEBUG=`$SAFFIRE_TEST_BIN version --long | grep "debug"`
if [ -n "$DEBUG" ] ; then
	echo "Please build saffire without debug settings before running this test script";
	exit 1;
fi

/usr/bin/php support/unittester/run-saffire-tests.php ${1:-"unittests/tests/"}
