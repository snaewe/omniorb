#
# sgi_irix_6.2.mk - make variables and rules specific to SGI Irix 6.2
#

IRIX = 1
IRIX_n32 = 1
IndigoProcessor = 1


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
CXXDEBUGFLAGS = 
CXXWOFFOPTIONS =  -woff 3303,1110,1182
CXXOPTIONS     =  -n32 -float -ansi -LANG:exceptions=ON $(CXXWOFFOPTIONS)
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

CC                = cc
COPTIONS          = -n32
CLINKOPTIONS      = $(COPTIONS)
CLINK             = $(CC)

#
# OMNI thread stuff
#

Posix_OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) -lpthread
Posix_OMNITHREAD_CPPFLAGS = -DUsePthread -D_REENTRANT 
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

CorbaImplementation = OMNIORB

# It is known that the linker on Irix 6.4 and 6.5 has some peculiar
# requirements on the order in which share libraries are specified on the 
# command line (see mips_irix_6.5_n32.mk).
#
# The same may apply to Irix 6.2 and 6.3.
#
# Here we reset the value of OMNIORB_LIB to meet the requirements. The
# variable was original set in unix.mk
#
# "Nilo Stolte" <nilo_stolte@my-dejanews.com> has confirmed that this setup
# works on Irix 6.3 with 7.2.1 C++ compiler + patches to compiler, posix
# threads etc.
#
OMNIORB_LIB = $(patsubst %,$(LibSearchPattern),omniORB3) \
		$(patsubst %,$(LibSearchPattern),omniDynamic2) \
                $(patsubst %,$(LibSearchPattern),omniORB3) \
                $(OMNITHREAD_LIB)

OMNIORB_LIB_NODYN = $(patsubst %,$(LibSearchPattern),omniORB3) \
                $(patsubst %,$(LibSearchPattern),omniORB3) \
                $(OMNITHREAD_LIB)


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

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif
