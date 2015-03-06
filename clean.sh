#!/bin/sh
# Cleans up the builds
rm -rf build/debug
rm -rf build/release

rm -f src/components/compiler/lex.yy.c
rm -f src/components/compiler/parser.tab.c
rm -f src/components/vm/_generated_vm_opcodes.c
rm -f src/components/objects/_generated_exceptions.inc
rm -f src/components/objects/_generated_interfaces.inc

rm -f include/saffire/compiler/lex.yy.h
rm -f include/saffire/vm/_generated_vm_opcodes.h
rm -f include/saffire/objects/_generated_exceptions.h
rm -f include/saffire/objects/_generated_interfaces.h
rm -f include/saffire/config.h
rm -f include/saffire/gitversion.h
rm -f include/saffire/compiler/parser.tab.h
