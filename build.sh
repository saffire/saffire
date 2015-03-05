#!/bin/bash

SRC_ROOT_DIR=`pwd`            # full path to the src directory
BUILD_ROOT_DIR="`pwd`/build"  # path to the build directory. See `-b' option
BUILDTYPE="release"           # determines which target(s) has to be built. See `-t' option
CLEAN=0                       # clean the target build directory before build it. See `-c' option
VERBOSE=0                     # make the build process verbose if 1. See `-v' option

if [[ `uname` == 'Darwin' ]]; then
    export ICU_ROOT=$(brew --prefix icu4c)
fi

build_target () {

    target_build=$1;
    target_dir=$2
    clean_dir=$3;
    verbose=$4

    cd $BUILD_ROOT_DIR

    if [ $clean_dir -eq 1 ] ; then
        rm -rf $target_dir
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

    cd $SRC_ROOT_DIR
}

while getopts "hvct:b:" opt ; do
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
        h)
            echo "Usage: build.sh [-h] [-v] [-c] [-t <target>]"
            echo
            echo "-h             Display usage info"
            echo "-v             Make the build process verbose"
            echo "-c             Clean build directory first"
            echo "-t <target>    Target to build (release|debug|all)"
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
    build_target Release release $CLEAN $VERBOSE
fi

if [ $BUILDTYPE = "all" -o $BUILDTYPE = "debug" ] ; then
    build_target Debug debug $CLEAN $VERBOSE
fi
