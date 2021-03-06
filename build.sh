#!/bin/bash

SRC_ROOT_DIR=`pwd`            # full path to the src directory
BUILD_ROOT_DIR="`pwd`/build"  # path to the build directory. See `-b' option
BUILDTYPE="release"           # determines which target(s) has to be built. See `-t' option
CLEAN=0                       # clean the target build directory before build it. See `-c' option
VERBOSE=0                     # make the build process verbose if 1. See `-v' option
INSTALL=0                     # install saffire if 1. See `-i' option

if [[ `uname` == 'Darwin' ]]; then
    export ICU_ROOT=$(brew --prefix icu4c)
fi

build_target () {

    target_build=$1;
    target_dir=$2
    clean_dir=$3;
    verbose=$4
    install=$5

    cd $BUILD_ROOT_DIR

    if [ $clean_dir -eq 1 ] ; then

        rm -rf $target_dir

        # temporary fix to issue #214
        # remove generated files from the `include' and `src' dir

        rm -f $SRC_ROOT_DIR/include/saffire/config.h
        rm -f $SRC_ROOT_DIR/include/saffire/gitversion.h
        rm -f $SRC_ROOT_DIR/include/saffire/compiler/lex.yy.h
        rm -f $SRC_ROOT_DIR/include/saffire/compiler/parser.tab.h
        rm -f $SRC_ROOT_DIR/include/saffire/objects/_generated_interfaces.h
        rm -f $SRC_ROOT_DIR/include/saffire/objects/_generated_exceptions.h
        rm -f $SRC_ROOT_DIR/include/saffire/vm/_generated_vm_opcodes.h

        rm -f $SRC_ROOT_DIR/src/components/compiler/lex.yy.c
        rm -f $SRC_ROOT_DIR/src/components/compiler/parser.tab.c
        rm -f $SRC_ROOT_DIR/src/components/vm/_generated_vm_opcodes.c
        rm -f $SRC_ROOT_DIR/src/components/objects/_generated_exceptions.inc
        rm -f $SRC_ROOT_DIR/src/components/objects/_generated_interfaces.inc

    fi

    if [ ! -d $target_dir ] ; then
        mkdir $target_dir
    fi

    cd $target_dir

    if [ $verbose -eq 1 ] ; then
        export VERBOSE=1
    fi

    cmake -DCMAKE_BUILD_TYPE=$target_build $SRC_ROOT_DIR

    if [ $? -ne 0 ] ; then
        echo
        echo "Something went wrong during the configuration process."
        echo
        exit 1
    fi

    make

    if [ $? -ne 0 ] ; then
        echo
        echo "Something went wrong during the build process."
        echo
        exit 1
    fi

    if [ $install -eq 1 ] ; then
        make install
    fi

    cd $SRC_ROOT_DIR
}

while getopts "hvcit:b:" opt ; do
    case "${opt}" in
        v)
            VERBOSE=1
            ;;
        c)
            CLEAN=1
            ;;
        t)
            BUILDTYPE=${OPTARG}
            ;;
        b)
            BUILD_ROOT_DIR=${OPTARG}
            ;;
        i)
            INSTALL=1
            ;;
        h)
            echo "Usage: build.sh [-h] [-v] [-c] [-i] [-t <target>]"
            echo
            echo "-h             Display usage info"
            echo "-v             Make the build process verbose"
            echo "-c             Clean build directory first"
            echo "-i             Install Saffire after build"
            echo "-t <target>    Target to build (release|debug|coverage|all)"
            echo "-b <build-dir> Path to the build directory. \`build' is used as default"
            echo
            echo "When no target is specified, release will be build."
            echo
            exit 1
            ;;
    esac
done

if [ ! -d $BUILD_ROOT_DIR ] ; then
    mkdir -p $BUILD_ROOT_DIR
fi

if [ $BUILDTYPE = "all" -o $BUILDTYPE = "release" ] ; then
    build_target Release release $CLEAN $VERBOSE $INSTALL
fi

if [ $BUILDTYPE = "all" -o $BUILDTYPE = "debug" ] ; then
    build_target Debug debug $CLEAN $VERBOSE 0
fi

if [ $BUILDTYPE = "all" -o $BUILDTYPE = "coverage" ] ; then
    build_target Coverage coverage $CLEAN $VERBOSE 0
    cd $BUILD_ROOT_DIR/coverage
    export SAFFIRE_TEST_BIN=`pwd`/bin/saffire
    make saffire_coverage
    cd $SRC_ROOT_DIR
fi
