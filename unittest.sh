#!/bin/sh

CPWD=`pwd`

cd `dirname $0`

: ${SAFFIRE_TEST_BIN:=./build/release/bin/saffire}
export SAFFIRE_TEST_BIN

if [ ! -e $SAFFIRE_TEST_BIN ] ; then
    echo "Cannot find the test binary '$SAFFIRE_TEST_BIN'";
    exit 1;
fi

DEBUG=`$SAFFIRE_TEST_BIN version --long | grep "debug"`
if [ -n "$DEBUG" ] ; then
    echo "Please build saffire without debug settings before running this test script";
    exit 1;
fi

`dirname ${SAFFIRE_TEST_BIN}`/utmain
/usr/bin/php support/unittester/run-saffire-tests.php ${1:-"unittests/tests/"}

cd $CPWD
