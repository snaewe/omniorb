#
# powerpc_darwin_1.3.mk - make variables and rules specific to powerpc darwin 1.3
#

Darwin = 1
PowerPCProcessor = 1

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
CPP = /usr/bin/cpp

CXX             = /usr/bin/c++
CXXMAKEDEPEND  += -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS   = -O2
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)
CXXOPTIONS      = -Wall -Wno-unused

CC           = /usr/bin/cc
CMAKEDEPEND += -D__GNUC__
CDEBUGFLAGS  = -O
CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS)
COPTIONS     = 

INSTALL = install -c

IMPORT_CPPFLAGS += -D__powerpc__ -D__darwin__ -D__OSVERSION__=1
MKDIRHIER = mkdir -p

#
# CORBA stuff
#

omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_CPPFLAGS =
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DPthreadSupportThreadPriority -DNoNanoSleep
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)
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

BuildSharedLibrary = 1           # Enable
SHAREDLIB_CPPFLAGS = -fno-common # compiler flag
SHAREDLIB_SUFFIX = dylib
SharedLibraryFullNameTemplate = lib$$1$$2.$$3.$$4.$(SHAREDLIB_SUFFIX)
SharedLibrarySoNameTemplate = lib$$1$$2.$$3.$(SHAREDLIB_SUFFIX)
SharedLibraryLibNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX)
SharedLibraryPlatformLinkFlagsTemplate = -dynamiclib -undefined suppress

# Re-define 'ExportLibrary' to run 'ranlib' after the file is copied,
# for static libraries as otherwise the linker complains: "table of 
# contents for archive: ???? is out of date; rerun ranlib(1) (can't
# load from it)"
#
ifdef EXPORT_TREE
define ExportLibrary
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportFileToDir); \
   base=`basename $$file`; \
   if [ $${base%.a} != $$base ]; then (set -x; $(RANLIB) $$dir/$$base); fi; \
 done; \
)
endef
endif


#
# everything else is default from unix.mk
