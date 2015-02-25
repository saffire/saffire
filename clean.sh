#!/bin/sh
# Cleans up the builds
rm -rf build/debug
rm -rf build/release
rm src/include/compiler/lex.yy.h
rm src/components/compiler/lex.yy.c
rm src/components/compiler/parser.tab.c
