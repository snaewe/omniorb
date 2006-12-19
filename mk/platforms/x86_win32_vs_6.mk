#
# x86_win32_vc6.mk - make variables and rules specific to Visual Studio 6
#

WindowsNT = 1
x86Processor = 1

compiler_version_suffix=_vc6

WINVER = 0x0400

BINDIR = bin/x86_win32
LIBDIR = lib/x86_win32

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 or later interpreter. If you
# do not wish to make a complete installation, you may download a
# minimal Python from
# http://sourceforge.net/project/showfiles.php?group_id=51138&package_id=48638
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
#PYTHON = /cygdrive/c/Python24/python


# Use the following set of flags to build and use multithreaded DLLs
#
MSVC_DLL_CXXNODEBUGFLAGS       = -MD -GX -Zi
MSVC_DLL_CXXLINKNODEBUGOPTIONS = -nologo -DEBUG
MSVC_DLL_CNODEBUGFLAGS         = -MD -Zi
MSVC_DLL_CLINKNODEBUGOPTIONS   = -nologo -DEBUG
#
MSVC_DLL_CXXDEBUGFLAGS         = -MDd -GX -Zi -Od 
MSVC_DLL_CXXLINKDEBUGOPTIONS   = -nologo -DEBUG
MSVC_DLL_CDEBUGFLAGS           = -MDd -Zi -Od
MSVC_DLL_CLINKDEBUGOPTIONS     = -nologo -DEBUG
#
# Or
#
# Use the following set of flags to build and use multithread static libraries
#
MSVC_STATICLIB_CXXNODEBUGFLAGS       = -MT -GX
MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS = 
MSVC_STATICLIB_CNODEBUGFLAGS         = -MT
MSVC_STATICLIB_CLINKNODEBUGOPTIONS   = 

MSVC_STATICLIB_CXXDEBUGFLAGS         = -MTd -GX -Z7 -Od 
MSVC_STATICLIB_CXXLINKDEBUGOPTIONS   = -DEBUG
MSVC_STATICLIB_CDEBUGFLAGS           = -MTd -Z7 -Od
MSVC_STATICLIB_CLINKDEBUGOPTIONS     = -DEBUG



#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk



IMPORT_CPPFLAGS += -D__x86__ -D__NT__ -D__OSVERSION__=4


# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = C:\\temp


# Add the location of the Open SSL library

# To build the SSL transport, OPEN_SSL_ROOT must be defined and points to
# the top level directory of the openssl library. The default is to disable
# the build.
#
#OPEN_SSL_ROOT = /cygdrive/c/openssl
#

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = $(patsubst %,$(LibPathPattern),$(OPEN_SSL_ROOT)/lib) \
               ssleay32.lib libeay32.lib
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
