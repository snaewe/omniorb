#
# alpha_osf1_5.0.mk - make variables and rules specific to Tru64 5.0
#

OSF1 = 1
AlphaProcessor = 1


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

IMPORT_CPPFLAGS += -D__alpha__ -D__osf1__ -D__OSVERSION__=5


#
# Standard programs
#

AR = ar clq

CXX = /usr/bin/cxx
# For DEC C++ 6.2
CXXOPTIONS = -ptr $(TOP)/cxx_respository
#
DecCxxMajorVersion = 6
DecCxxMinorVersion = 2
#
# For DEC C++ 6.2
# Uncommment the following line to speed up the compilation, but may require
# manually deleted some .pch and cxx_respository/TIMESTAMP files to pick
# up changes in templates or the order of -I flags.
#
# CXXOPTIONS += -ttimestamp -pch

CXXMAKEDEPEND += -D__DECCXX -D__DECCXX_VER=60290024 -D__cplusplus
CXXDEBUGFLAGS = -O

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -call_shared

CC = cc
CDEBUGFLAGS = -g 

CLINK = $(CC) -g


#
# When specifying the "rpath" (directories which the run-time linker should
# search for shared libraries) we unfortunately need to do it in a single
# argument.  For this reason we override the default unix CXXExecutable and
# CExecutable rules.  Any -L flags given in $$libs results in another element
# being added to the rpath and we then give the whole rpath at the end of the
# link command line.
#

RPATH = $(subst $(space),:,$(strip $(IMPORT_LIBRARY_DIRS)))

define CXXExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef

define CExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef


#
# CORBA stuff
#

CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DNoNanoSleep
OMNITHREAD_CPPFLAGS = -D_REENTRANT -pthread

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) \
		 -lpthread -lexc

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

ifeq ($(notdir $(CXX)),cxx)

BuildSharedLibrary = 1       # Enable

SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
  ld -shared -soname $$soname -set_version $$soname -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs \
  -lcxxstd -lcxx -lexc -lots -lc;
endef

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif
