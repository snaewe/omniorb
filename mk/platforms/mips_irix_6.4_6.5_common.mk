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
INSTALL   = $(BASE_OMNI_TREE)/bin/scripts/install-sh -c

CPP = 'CC -E'

# The cc/CC version 7.2 (mips)
#
CXX = CC
CXXMAKEDEPEND += -D__SGI_CC -D__cplusplus
CXXDEBUGFLAGS = -O2 -OPT:Olimit=0
CXXWOFFOPTIONS =  -woff 3303,1110,1182
CXXOPTIONS     =  $(ABIFLAG) -float -ansi -LANG:exceptions=ON $(CXXWOFFOPTIONS)
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

CC                = cc
COPTIONS          = $(ABIFLAG)
CLINKOPTIONS      = $(COPTIONS)
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


#
# Shared Library support.     
#
# Platform specific customerisation.
# everything else is default from unix.mk
#
ifeq ($(notdir $(CXX)),CC)

BuildSharedLibrary = 1       # Enable

SHAREDLIB_CPPFLAGS = -KPIC

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 libname=$(SharedLibraryLibNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(LINK.cc) -KPIC -shared -Wl,-h,$$libname -Wl,-set_version,$$soname -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) \
 $$extralibs $(LDLIBS); \
endef

endif
