#
# sun4_sosV_5.8.mk - make variables and rules specific to Solaris 8.
#

SunOS = 1
SparcProcessor = 1

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

IMPORT_CPPFLAGS += -D__sparc__ -D__sunos__ -D__OSVERSION__=5


#
# Standard programs
#

AR = ar cq

MKDIRHIER = mkdirhier
INSTALL           = $(BASE_OMNI_TREE)/bin/scripts/install-sh -c 

CPP = /usr/ccs/lib/cpp

#
# To use SunPro compilers, uncomment the following lines:
#
CXX = CC
CXXMAKEDEPEND += -D__SUNPRO_CC -D__cplusplus
CXXDEBUGFLAGS = -O2
CXXMTFLAG     = -mt
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

# CXXLINKOPTIONS += $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS))
# Note: the -R linker option in CXXLINKOPTIONS instruct the Sun linker to
# record the pathname of the shared libraries in the executable.

CC                = cc
CDEBUGFLAGS       = -O2
COPTIONS	  =
CLINK             = $(CC)
#####################################################################

#
# To use gcc, uncomment the following lines:
#
#CPP = gcc
#
#CXX = g++
#CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__
#CXXDEBUGFLAGS = 
#CXXOPTIONS    =  -fhandle-exceptions -Wall -Wno-unused
#CXXMTFLAG     =
#
#CXXLINK		= $(CXX)
#CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)
#
# CXXLINKOPTIONS += $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS))
# Note: the -R linker option in CXXLINKOPTIONS instruct the Sun linker to
# record the pathname of the shared libraries in the executable.
#
#CC                = gcc
#CMAKEDEPEND       += -D__GNUC__
#CDEBUGFLAGS       = -O
#COPTIONS	  = -fpcc-struct-return
#
#CLINK             = $(CC)

#######################################################################

#
# Socket library
#

SOCKET_LIB = -lsocket -lnsl
THREAD_LIB = -lthread $(CXXMTFLAG)


#
# CORBA stuff
#

omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

Solaris_OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) \
			 -lthread -lposix4 $(CXXMTFLAG)
Solaris_OMNITHREAD_CPPFLAGS = -D_REENTRANT $(CXXMTFLAG)

Posix_OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) -lpthread \
		 -lposix4 $(CXXMTFLAG)
Posix_OMNITHREAD_CPPFLAGS = -DUsePthread -D_REENTRANT $(CXXMTFLAG)
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 \
			    -DPthreadSupportThreadPriority

OMNITHREAD_LIB = $($(ThreadSystem)_OMNITHREAD_LIB)
OMNITHREAD_CPPFLAGS = $($(ThreadSystem)_OMNITHREAD_CPPFLAGS)

ThreadSystem = Posix

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

BuildSharedLibrary = 1       # Enable

SHAREDLIB_CPPFLAGS = -KPIC

SharedLibraryPlatformLinkFlagsTemplate = -G -h $$soname

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif

ifeq ($(notdir $(CXX)),g++)

BuildSharedLibrary = 1       # Enable

SHAREDLIB_CPPFLAGS = -fPIC

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-h,$$soname

endif

