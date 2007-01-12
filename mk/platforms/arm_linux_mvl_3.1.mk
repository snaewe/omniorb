#
# ARM Linux MVL 3.1.mk - make variables and rules specific to
#                        MontaVista Linux 3.1 and gcc compiler on a ARM target.
#

ARMProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

# MPT Compiler prefix
compiles=/net/compiler/gcc_mpt_2.00
CCTARGET=armv5teb-hardhat-linux
CCHOST=i586-pc-linux
COMPILER_PREFIX=$(compiles)/bin/$(CCTARGET)

HH_INCLUDES = -I$(TOP)/../../Hardhat_Interface_armv5teb/include \
	      -I$(TOP)/.. \
	      -I$(TOP)/../../TOOL_omniORB_export/$(CCHOST)/include \

CXXFLAGS+= -I$(TOP)/../../Hardhat_Interface_armv5teb/include/c++/3.3.1 \
	   -I$(TOP)/../../Hardhat_Interface_armv5teb/include/c++/3.3.1/armv5teb-hardhat-linux \
           $(HH_INCLUDES) \
	   -DHAVE_STD \
           -O2

CFLAGS+=   $(HH_INCLUDES) \
           -O2

IMPORT_LIBRARY_DIRS = $(TOP)/../../Hardhat_Interface_armv5teb/lib \
		      $(TOP)/../../TOOL_omniORB_export/$(CCTARGET)/lib \
	              $(EXPORT_TREE)/$(LIBDIR)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
PYTHON = /usr/local/bin/python


#
# Include general Linux things
#

include $(THIS_IMPORT_TREE)/mk/linux.mk


#
# Standard programs
#
MKDIRHIER = mkdirhier

RANLIB = $(COMPILER_PREFIX)-ranlib
AR = $(COMPILER_PREFIX)-ar cq
NM = $(COMPILER_PREFIX)-nm

CXX = $(COMPILER_PREFIX)-gcc
CXXMAKEDEPEND = $(HOSTBINDIR)/omkdepend -D__GNUC__
#//willi
#//original CXXFLAGS     += -fno-rtti -mno-sched-prolog -fno-builtin -Wno-unused  -ansi -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXFLAGS     += -mno-sched-prolog -fno-builtin -Wno-unused -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXDEBUGFLAGS = -g

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) \
                -lstdc++ -lomniORB4 -lomnithread \
		$(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS)) \
		$(patsubst %,-Wl$(comma)-rpath-link$(comma)%,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused -g

CC           = $(COMPILER_PREFIX)-gcc
CMAKEDEPEND  = $(HOSTBINDIR)/omkdepend -D__GNUC__
CDEBUGFLAGS  = -g -O2

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

IMPORT_CPPFLAGS +=-D__armv5teb__

#
# CORBA stuff
#

CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

ifndef UseMITthreads
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=10
OMNITHREAD_LIB += -lpthread
else
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=8 
OMNITHREAD_CPPFLAGS += -D_MIT_POSIX_THREADS
OMNITHREAD_LIB += -lpthreads
endif

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
SHAREDLIB_CPPFLAGS = -fPIC   # compiler flag
