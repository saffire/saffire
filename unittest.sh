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

export PHP_IDE_CONFIG=serverName=Symfony
/usr/bin/php -dxdebug.remote_host=192.168.56.1 -dxdebug.remote_autostart=1 support/unittester/run-saffire-tests.php ${1:-"unittests/tests/"}
