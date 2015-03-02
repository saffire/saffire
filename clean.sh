#!/bin/sh
# Cleans up the builds
rm -rf build/debug
rm -rf build/release
rm src/include/compiler/lex.yy.h
rm src/components/compiler/lex.yy.c
rm src/components/compiler/parser.tab.c
rm src/components/vm/_generated_vm_opcodes.c
rm src/include/vm/_generated_vm_opcodes.h
rm src/components/objects/_generated_exceptions.inc
rm src/include/objects/_generated_exceptions.h
rm src/components/objects/_generated_interfaces.inc
rm src/include/objects/_generated_interfaces.h
rm src/include/config.h
rm src/include/gitversion.h
rm src/include/compiler/parser.tab.h

true
