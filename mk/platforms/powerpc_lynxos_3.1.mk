#
# powerpc_lynxos_3.1.mk - make variables and rules specific to LynxOS 3.1
#

# ENV_PREFIX is the standard LynxOS variable for the
# location of the Lynx distribution.

# TOOLDIR and TOOLDIR2 are set up for cross-development under
# solaris.  If you build natively under LynxOS, uncomment
# the last TOOLDIRs.
TOOLDIR = $(ENV_PREFIX)/cdk/sunos-xcoff-ppc/bin
TOOLDIR2 = $(ENV_PREFIX)/cdk/sunos-xcoff-ppc/usr/bin
#TOOLDIR = /bin
#TOOLDIR2 = /usr/bin

LynxOS = 1
PowerPCProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
#PYTHON = /usr/local/bin/python


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__powerpc__ -D__lynxos__ -D__OSVERSION__=3

#
# Standard programs
#

AR = $(TOOLDIR)/ar cq

MKDIRHIER = mkdirhier
INSTALL           = $(BASE_OMNI_TREE)/bin/scripts/install-sh -c

CPP = $(TOOLDIR)/cpp

#
#
CC = $(TOOLDIR)/gcc
COPTIONS = -mthreads
CMAKEDEPEND  += -D__GNUC__
DEBUGFLAGS  = -O
CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS)


CXX = $(TOOLDIR2)/g++
CXXOPTIONS = -mthreads
CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2
CXXLINK               = $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

#
# Socket library
#

SOCKET_LIB = -lbsd -lnsl -lstdc++ -lrpc


#
# CORBA stuff
#

omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 \
                          -DPthreadSupportThreadPriority

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

OMNITHREAD_CPPFLAGS = -D_REENTRANT -DUsePthread



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
# Platform specific customerisation.
# everything else is default from unix.mk
#
ifeq ($(notdir $(CXX)),CC)

BuildSharedLibrary = 0       # Disable since they are difficult to build for Lynx 3.1 for PPC

SHAREDLIB_CPPFLAGS = -KPIC

SharedLibraryPlatformLinkFlagsTemplate = -G -h $$soname

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif

ifeq ($(notdir $(CXX)),g++)

BuildSharedLibrary = 0       # Disable

SHAREDLIB_CPPFLAGS = -fPIC

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-h,$$soname

endif

