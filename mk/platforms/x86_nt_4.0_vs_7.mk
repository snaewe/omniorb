#
# x86_nt_4.0.mk - make variables and rules specific to Windows NT 4.0.
#

WindowsNT = 1
x86Processor = 1

WINVER = 0x0400

BINDIR = bin/x86_win32
LIBDIR = lib/x86_win32

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
PYTHON = /cygdrive/c/Python22/python

MSVC7 = "true"

# Use the following set of flags to build and use multithreaded DLLs
#
MSVC_DLL_CXXNODEBUGFLAGS       = -MD -EHs -GS -GR -Zd -nologo
MSVC_DLL_CXXLINKNODEBUGOPTIONS = -nologo -DEBUG
MSVC_DLL_CNODEBUGFLAGS         = -MD -GS -GR -Zd -nologo
MSVC_DLL_CLINKNODEBUGOPTIONS   = -nologo -DEBUG
#
MSVC_DLL_CXXDEBUGFLAGS         = -MD -EHs -RTC1 -GS -GR -Zd -nologo
MSVC_DLL_CXXLINKDEBUGOPTIONS   = -nologo -DEBUG
MSVC_DLL_CDEBUGFLAGS           = -MD -RTC1 -GS -GR -Zd -nologo
MSVC_DLL_CLINKDEBUGOPTIONS     = -nologo -DEBUG
#
# Or
#
# Use the following set of flags to build and use multithread static libraries
#
MSVC_STATICLIB_CXXNODEBUGFLAGS       = -MD -EHs -GS -GR -Zd -nologo
MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS = -nologo -DEBUG
MSVC_STATICLIB_CNODEBUGFLAGS         = -MD -GS -GR -Zd -nologo
MSVC_STATICLIB_CLINKNODEBUGOPTIONS   = -nologo -DEBUG

MSVC_STATICLIB_CXXDEBUGFLAGS         = -MD -EHs -RTC1 -GS -GR -Zd -nologo
MSVC_STATICLIB_CXXLINKDEBUGOPTIONS   = -nologo -DEBUG
MSVC_STATICLIB_CDEBUGFLAGS           = -MD -RTC1 -GS -GR -Zd -nologo
MSVC_STATICLIB_CLINKDEBUGOPTIONS     = -nologo -DEBUG


ifdef BuildDebugBinary

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKDEBUGOPTIONS)
CXXDEBUGFLAGS  = 
CXXOPTIONS     = $(MSVC_DLL_CXXDEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKDEBUGOPTIONS)
CDEBUGFLAGS    = $(MSVC_DLL_CDEBUGFLAGS)

else

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKNODEBUGOPTIONS)
CXXDEBUGFLAGS  = -O2
CXXOPTIONS     = $(MSVC_DLL_CXXNODEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKNODEBUGOPTIONS)
CDEBUGFLAGS    = -O2
COPTIONS       = $(MSVC_DLL_CNODEBUGFLAGS)

endif

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
#OPEN_SSL_ROOT = /D/openssl
#

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = $(patsubst %,$(LibPathPattern),$(OPEN_SSL_ROOT)/lib) \
               ssleay32.lib libeay32.lib
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
