#
# x86_ets.mk - make variables and rules specific to Phar Lap Software
#              Realtime ETS kernel
#

ETSKernel = 1
x86Processor = 1

WINVER = 0x0400

# Define EmbeddedSystem causes the build process to only build
#    src/lib.
#
EmbeddedSystem = 1

HOSTBINDIR = bin/x86_win32
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
#PYTHON = //c/progra~1/Python/python


#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk


# Override settings in win32.mk.
IMPORT_CPPFLAGS += -D__x86__ -D__NT__ -D__OSVERSION__=4 -D__ETS_KERNEL__

BuildSharedLibrary =

LibNoDebugPattern = %ETS.lib
LibDebugPattern = %ETSd.lib
LibNoDebugSearchPattern = %ETS.lib
LibDebugSearchPattern = %ETSd.lib

# Default location of the omniORB configuration file.
#
#CONFIG_DEFAULT_LOCATION = C:\\OMNIORB.CFG

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
