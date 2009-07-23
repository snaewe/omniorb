omniORBpy 3
===========

This is omniORBpy 3.4.

omniORBpy is a robust high-performance CORBA ORB for Python.

The bindings adhere to the standard IDL to Python mapping which you
may find at

  http://www.omg.org/technology/documents/formal/python_language_mapping.htm


omniORBpy makes use of the C++ omniORB library. omniORBpy version 3.x
can only be used with omniORB 4.1.x.

omniORBpy is currently available for many Unix platforms and Windows.
It has been ported to a number of other operating systems by outside
contributors.

It is a good idea to subscribe to the omniORB mailing list. See

    http://omniorb.sourceforge.net/list.html


Please report any bugs you find to the mailing list.


Building
--------

If you are using a Unix platform, the Autoconf configure script will
probably work for you. Simply create a directory for the build and run
the configure script, followed by make:

  $ cd [omniORBpy directory]
  $ mkdir build
  $ cd build
  $ ../configure [configure options]
  $ make
  $ make install

Run configure --help to get a list of configuration options. Most
options are standard Autoconf ones. The most commonly required is
--prefix, used to select the install location. The default is
/usr/local. To change it, use, for example

  ../configure --prefix=/home/fred/omni_inst

If you use a different prefix to the omniORB install, you must give
the location to omniORB with the --with-omniorb= option.

The configure script tries to figure out the location of the C and C++
compilers and Python. It will always choose gcc over the platform's
native compiler if it is available. To change the choices it makes,
use variables CC, CXX and PYTHON, e.g.:

  ../configure CXX=/usr/bin/platform_c++ PYTHON=/usr/local/bin/python2.3


If you do not have Autoconf support on your platform (e.g. on
Windows), you must install the omniORBpy source in the $TOP/src/lib
directory of a working omniORB 4.1 tree. Make sure the directory is
called omniORBpy -- if you downloaded a release archive you must
rename the directory to remove the version from the directory name.

Once you have the source in the right place, simply do a make export
in the $TOP/src/lib/omniORBpy directory.




Using omniORBpy
---------------

To use omniORBpy, you need to add the directories containing omniORB
to your PYTHONPATH. With an Autoconf build, if you specified the same
installation prefix as was used for Python, you do not need to add
anything to PYTHONPATH; if you used a different installation prefix,
add this path:

  export PYTHONPATH=$PYTHONPATH:$PREFIX/lib/pythonX.Y/site-packages

replacing pythonX.Y with the version of Python you are using,
e.g. python2.4

With the non Autoconf build, you must add two directories to
PYTHONPATH:

  export PYTHONPATH=$PYTHONPATH:$TOP/lib/python:$TOP/lib/$FARCH

where $TOP is the root of your omniORB tree and $FARCH is the platform
name you selected in config.mk. Windows uses semicolons instead of
colons to separate path components:

  set PYTHONPATH=%PYTHONPATH%;%TOP%\lib\python;%TOP%\lib\x86_win32


You should also have the correct bin directory on your path so you can
run the IDL compiler, omniidl. With Autoconf builds, it is in
$PREFIX/bin ; otherwise it is in $TOP/bin/$FARCH.


Full documentation, in a variety of formats, can be found in the doc
directory.
