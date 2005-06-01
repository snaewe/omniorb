#
# hppa_hpux_11.23.mk - make variables and rules specific to HPUX 11.23 on PA-RISC 64
#

HPUX = 1
HppaProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
PYTHON = /usr/local/aCCPython/bin/python
 
#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -ldir $< -o $@

#               -ldir is for compiling templates
#


#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__hppa__ -D__hpux__ -D__OSVERSION__=11


#
# Standard programs
#

AR = ar cq
RANLIB = ranlib
MKDIRHIER = mkdir -p
INSTALL         = $(BASE_OMNI_TREE)/bin/scripts/install-sh -c
INSTLIBFLAGS    = -m 0755    # shared library must have executable flag set.

CPP = /lib/cpp

#############################################################################
# To use aCC uncomment the following lines:                                 #
#############################################################################
# Tested on:
#      aCC: HP ANSI C++ B3910B A.03.58.%IZ2
#

CPPFLAGS      += +DD64 
CXX = aCC
CXXMAKEDEPEND += -D__cplusplus
CXXDEBUGFLAGS += 
CXXOPTIONS   += -AA -mt +DD64
CXXLINK		= $(CXX) 
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) +DD64

CC                = cc
CDEBUGFLAGS       = 
COPTIONS	  += -Ae -mt +DD64 
CLINKOPTIONS      += -mt +DD64
CLINK             = $(CC)

# In order to compile Templates
# Override the default StaticLinkLibrary in unix.mk
#
define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -c $^; \
 $(AR) $@ $^; \
 $(RANLIB) $@; \
)
endef


#
# Socket library
#

SOCKET_LIB = 
THREAD_LIB = 


#
# CORBA stuff
#
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#


ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = +DD64 
HPTHREADLIBS = -lpthread
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) $(HPTHREADLIBS)

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
SHAREDLIB_SUFFIX   = sl

ifeq ($(notdir $(CXX)),aCC)

BuildSharedLibrary = 1

SHAREDLIB_CPPFLAGS += +z 

SharedLibraryPlatformLinkFlagsTemplate = -b -Wl,-E -Wl,-s -mt +DD64

# May need  $(patsubst %,-L %,$(IMPORT_LIBRARY_DIRS))

endif


