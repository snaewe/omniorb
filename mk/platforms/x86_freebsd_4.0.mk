# x86_freebsd_3.2.mk - make variables and rules specific to FreeBSD 3.2
# started: 1999/06/12 afo
# [Last modified: 1999/06/13 12:42:06 by afo@xxo.zlug.org]

FreeBSD = 1
x86Processor = 1

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter.

#PYTHON = /usr/local/bin/python


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Standard programs
#

AR = ar cq

CPP = /usr/libexec/cpp

CXX = g++295
CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2

CXXLINK		= $(CXX) -pthread -s
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

CXXOPTIONS      = -Wall -Wno-unused
EgcsMajorVersion = 1
EgcsMinorVersion = 1        # This is actually a post-1.0 egcs snapshot.

CC           = gcc295
CMAKEDEPEND  += -D__GNUC__
CDEBUGFLAGS  = -O2

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS)

INSTALL = install -c

IMPORT_CPPFLAGS += -D__x86__ -D__freebsd__ -D__OSVERSION__=4


omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS =
OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

OMNITHREAD_POSIX_CPPFLAGS += -DUsePthread -DPthreadDraftVersion=10

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

#
# Shared Library support.     
#
BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS = -fPIC
#
# everything else is default from unix.mk



# Add the location of the Open SSL library

# To build the SSL transport, OPEN_SSL_ROOT must be defined and points to
# the top level directory of the openssl library. The default is to disable
# the build.
#
#OPEN_SSL_ROOT = /usr/local/openssl
#

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = -L$(OPEN_SSL_ROOT)/lib -lssl -lcrypto
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
