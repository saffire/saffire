#!/bin/sh

build_target () {
    target_build=$1;
    target_dir=$2
    clean_dir=$3;
    verbose=$4

    if [ $clean_dir -eq 1 -a -d $target_dir ] ; then rm -rf $target_dir; fi
    if [ ! -d $target_dir ] ; then mkdir $target_dir ; fi

    cd $target_dir
    cmake -B.  -D CMAKE_BUILD_TYPE=$target_build ../..

    if [ $verbose -eq 1 ] ; then
      VERBOSE=1 make
    else
      make
    fi

    cd ..
}




BUILDTYPE="all"
CLEAN=0
VERBOSE=0

while getopts "hvct:" opt ; do
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
        h)
            echo "Usage: build.sh [-h] [-c] [-t <target>]"
            echo
            echo "-h             Display usage info"
            echo "-c             Clean build directory first"
            echo "-t <target>    Target to build (release|debug)."
            echo
            echo "When no target is specified, all targets will be build."
            echo
            exit 1
            ;;
    esac
done


if [ $BUILDTYPE = "all" -o $BUILDTYPE = "release" ] ; then
    build_target Release release $CLEAN $VERBOSE
fi
if [ $BUILDTYPE = "all" -o $BUILDTYPE = "debug" ] ; then
    build_target Debug debug $CLEAN $VERBOSE
fi

