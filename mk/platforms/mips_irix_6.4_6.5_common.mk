# mips_irix_6.4_6.5_common.mk - make variables and rules common to IRIX 6.4 
#                                              and IRIX 6.5
#


IRIX = 1
IndigoProcessor = 1

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
# find out what ABI we're meant to be using (ie new 32-bit or 64-bit)
#

ifdef IRIX_n32
 ABIFLAG=-n32
 else
 ifdef IRIX_64
  ABIFLAG=-64
 else
  error Dont know what ABI to use, ie -n32 or -64 for new 32-bit or 64-bit builds
 endif
endif

#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__mips__ -D__irix__ -D__OSVERSION__=6


#
# Standard programs
#

AR = ar cq
RANLIB = true

MKDIRHIER = mkdirhier
INSTALL   = $(TOP)/bin/scripts/install-sh -c

CPP = 'CC -E'

# The cc/CC version 7.2 (mips)
#
CXX = CC
CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__SGI_CC -D__cplusplus
CXXDEBUGFLAGS = -O2 -OPT:Olimit=0
CXXWOFFOPTIONS =  -woff 3303,1110,1182
CXXOPTIONS     =  $(ABIFLAG) -float -ansi -LANG:exceptions=ON $(CXXWOFFOPTIONS)
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

CC                = cc
COPTIONS          = $(ABIFLAG)
CLINKOPTIONS      = $(COPTIONS)
CMAKEDEPEND       = $(TOP)/$(BINDIR)/omkdepend
CLINK             = $(CC)

#
# OMNI thread stuff
#

Posix_OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) -lpthread
Posix_OMNITHREAD_CPPFLAGS = -D_REENTRANT
#
# It is recommended to use -D_POSIX_C_SOURCE=199506L. However, POSIX 1c
# API is enabled by default by _SGI_SOURCE (which is defined by default). 
# Also, defining _POSIX_C_SOURCE remove all the other non-posix functions, 
# such as gettimeofday. This is bad.
# As far as I can tell, the _REENTRANT macro has no effect and could be
# removed.
#
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 \
			    -DPthreadSupportThreadPriority

OMNITHREAD_LIB = $($(ThreadSystem)_OMNITHREAD_LIB)
OMNITHREAD_CPPFLAGS = $($(ThreadSystem)_OMNITHREAD_CPPFLAGS)

ThreadSystem = Posix

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

#
# CORBA stuff
#

omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

# SGI linker has some peculiar requirements on the order in which share
# libraries are specified on the command line.
#    1. Two libraries cross reference each other, as it is the case with 
#       -lomniORB3 and -ltcpwrapGK means that -lomniORB3 has to be
#       repeated after -ltcpwrapGK
#    2. Multi-threaded programs must have -lpthread as the last option
#       on the command line.
# To satisify #2, we arrange in OMNIORB_LIB that -lpthread is the last
# option. So as long as in a dir.mk, the $(CORBA_LIB) is the last one
# in the assignment to libs, this condition is satisfied. 
# E.g.
#      $(eg1): eg1.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
#        @(libs="$(CORBA_LIB)"; $(CXXExecutable))
#

# Here we reset the value of OMNIORB_LIB to meet the above requirements. The
# variable was original set in unix.mk

OMNIORB_LIB = $(patsubst %,$(LibSearchPattern),omniORB3) \
		$(patsubst %,$(LibSearchPattern),omniDynamic3) \
	        $($(omniORBGatekeeperImplementation)_LIB) \
                $(patsubst %,$(LibSearchPattern),omniORB3) \
                $(OMNITHREAD_LIB)

OMNIORB_LIB_NODYN = $(patsubst %,$(LibSearchPattern),omniORB3) \
	        $($(omniORBGatekeeperImplementation)_LIB) \
                $(patsubst %,$(LibSearchPattern),omniORB3) \
                $(OMNITHREAD_LIB)


# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames
