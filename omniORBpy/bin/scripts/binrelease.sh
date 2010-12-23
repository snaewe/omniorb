#!/bin/sh

# Little script to build a Windows binary release of omniORBpy.

set -e

src=`pwd`
psrc=$src/src/lib/omniORBpy
dest=$src/../omniORBpy

if [ -d $dest ]; then
    echo "Destination $dest exists"
    exit 1
fi

if [ ! -d $psrc ]; then
    echo "$psrc does not exist"
    exit 1
fi

mkdir $dest

# Source files
cp -v $psrc/COPYING.LIB $dest
cp -v $psrc/README* $dest
cp -v $psrc/ReleaseNotes* $dest
cp -v $psrc/THIS_IS* $dest
cp -v $psrc/update.log $dest

cp -rv $psrc/examples $dest

mkdir $dest/doc
cp -rv $psrc/doc/omniORBpy* $dest/doc


# Core source files

cp -rv $src/COPYING $dest
cp -rv $src/sample.* $dest
cp -rv $src/doc/omniNames* $dest/doc
cp -rv $src/doc/utilities* $dest/doc

mkdir $dest/idl
mkdir $dest/idl/COS
cp -v $src/idl/*.idl $dest/idl
cp -v $src/idl/COS/*.idl $dest/idl/COS


# Compiled files

mkdir -p $dest/bin/x86_win32
cp -v $src/bin/x86_win32/catior.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/convertior.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/genior.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/nameclt.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniCodeSets*_rt.dll $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniConnectionMgmt*_rt.dll $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniMapper.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniNames.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniORB*_rt.dll $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omnicpp.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omniidl.exe $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omnisslTP*_rt.dll $dest/bin/x86_win32
cp -v $src/bin/x86_win32/omnithread*_rt.dll $dest/bin/x86_win32

mkdir -p $dest/lib/python
cp -v $src/lib/python/*.py* $dest/lib/python
cp -rv $src/lib/python/CosNaming $dest/lib/python
cp -rv $src/lib/python/CosNaming__POA $dest/lib/python
cp -rv $src/lib/python/omniORB $dest/lib/python
cp -rv $src/lib/python/omniidl $dest/lib/python

mkdir -p $dest/lib/python/omniidl_be
cp -v $src/lib/python/omniidl_be/*.py* $dest/lib/python/omniidl_be

mkdir -p $dest/lib/x86_win32
cp -v $src/lib/x86_win32/*.pyd $dest/lib/x86_win32
