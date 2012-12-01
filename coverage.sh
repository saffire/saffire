#!/bin/sh

#
# Make sure to compile with ./configure --enable-gcov --enable-debug
#
BASEPATH=`pwd`/src

gcov -p src/main/saffire.c 
#lcov --capture --directory --output-file coverage.bas
lcov --capture --initial --directory src --output-file .coverage.base  --base $BASEPATH
lcov --capture --directory src --output-file .coverage.run  --base $BASEPATH
lcov --directory src --add-tracefile .coverage.base --add-tracefile .coverage.run --output-file .coverage.total  --base $BASEPATH
genhtml --no-branch-coverage --output-directory html .coverage.total --prefix $BASEPATH
