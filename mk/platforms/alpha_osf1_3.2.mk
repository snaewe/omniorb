#
# alpha_osf1_3.2.mk - make variables and rules specific to Digital Unix
# (i.e. OSF1) 3.2.
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

IMPORT_CPPFLAGS += -D__alpha__ -D__osf1__ -D__OSVERSION__=3


#
# Standard programs
#

AR = ar clq

CXX = /usr/bin/cxx
CXXMAKEDEPEND += -D__DECCXX -D__cplusplus
CXXDEBUGFLAGS = -O

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -call_shared

CC = gcc
CMAKEDEPEND += -D__GNUC__
CDEBUGFLAGS = -O

CLINK = $(CC)


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

omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4 -DNoNanoSleep

OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) \
		 -lpthreads -lmach -lc_r
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
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs; \
  -lcxxstd -lcxx -lexc -lots -lc;
endef

ifeq ($(notdir $(CC)),gcc)
SHAREDLIB_CFLAGS = -fPIC
endif

endif
