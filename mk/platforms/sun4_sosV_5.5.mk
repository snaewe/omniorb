#
# sun4_sosV_5.5.mk - make variables and rules specific to Solaris 2.5.
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
INSTALL           = $(TOP)/bin/scripts/install-sh -c 

CPP = /usr/ccs/lib/cpp

#
# To use SunPro compilers, uncomment the following lines:
#
CXX = CC
CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__SUNPRO_CC -D__cplusplus
CXXDEBUGFLAGS = -O2
CXXMTFLAG     = -mt
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

# CXXLINKOPTIONS += $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS))
# Note: the -R linker option in CXXLINKOPTIONS instruct the Sun linker to
# record the pathname of the shared libraries in the executable.

#CC                = cc
#CMAKEDEPEND       = $(TOP)/$(BINDIR)/omkdepend
#CDEBUGFLAGS       = -O
#COPTIONS	  =
#CLINK             = $(CC)
#####################################################################

#
# To use gcc, uncomment the following lines:
#
#CPP = gcc
#
#CXX = g++
#CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
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

CC                = gcc
CMAKEDEPEND       = $(TOP)/$(BINDIR)/omkdepend -D__GNUC__
CDEBUGFLAGS       = -O
COPTIONS	  = -fpcc-struct-return

CLINK             = $(CC)

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

# MakeCXXSharedLibrary- Build shared library
#     Expect shell varables:
#       soname  = soname to be inserted into the library (e.g. libfoo.so.1)
#       libname = shared library name (e.g. libfoo.so)
#
# ExportSharedLibrary- export sharedlibrary
#     Expect shell varables:
#       soname  = soname to be inserted into the library (e.g. libfoo.so.1)
#       libname = shared library name (e.g. libfoo.so)
#      
ELF_SHARED_LIBRARY = 1

SHAREDLIB_SUFFIX   = .so

ifeq ($(notdir $(CXX)),CC)

ELF_SHARED_LIBRARY = 1

SHAREDLIB_CPPFLAGS = -KPIC

define MakeCXXSharedLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -G -o $@ -h $$soname $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $$extralibs; \
)
endef

endif

ifeq ($(notdir $(CXX)),g++)

ELF_SHARED_LIBRARY = 1

SHAREDLIB_CPPFLAGS = -fPIC

define MakeCXXSharedLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -shared -Wl,-h,$$soname -o $@ $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs; \
)
endef

endif

define ExportSharedLibrary
$(ExportLibrary); \
(set -x; \
   cd $(EXPORT_TREE)/$(LIBDIR); \
   $(RM) $$soname; \
   ln -s $^ $$soname; \
    $(RM) $$libname; \
    ln -s $$soname $$libname; \
  )
endef
