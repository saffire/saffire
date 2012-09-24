#!/bin/sh
gcov main.c 
lcov -c -d -o . -o coverage.base
lcov -c -d . -o .coverage.base
lcov -c -d . -o .coverage.run
lcov -d . -a .coverage.base -a .coverage.run -o .coverage.total
genhtml --no-branch-coverage -o html .coverage.total 
