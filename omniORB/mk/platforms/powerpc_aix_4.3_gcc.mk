#
# powerpc_aix_4.3_gcc.mk - make variables and rules specific to gcc 2.95
#                           on PowerPC AIX 4.3
#

AIX = 1
PowerPCProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

AIX_MAJOR_VERS := $(shell uname -v)
AIX_MINOR_VERS := $(shell uname -r)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter.

#PYTHON = /usr/local/bin/python
#PYTHON = $(ABSTOP)/python/bin/python
#PYTHONLIBDIR = $(ABSTOP)/python/lib/python1.5/config

#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk

#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__aix__ -D__powerpc__ -D__OSVERSION__=${AIX_MAJOR_VERS}

#
# Standard programs
#

AR			= ar r
RANLIB			= /bin/true
MKDIRHIER		= mkdir -p
INSTALL		= $(BASE_OMNI_TREE)/bin/scripts/install-sh -c

CMAKEDEPEND     += -D_AIX
CXXMAKEDEPEND   += -D__cplusplus -D_AIX

############################################################################
# Using gcc 2.95 (built with --enable-threads)                             #
#      Not well tested yet.                                                #
############################################################################
CXX             = gcc
CXXDEBUGFLAGS   = -O2
CXXOPTIONS      =
CXXLINK         = gcc
MTFLAGS         =
#
CC              = gcc
CLINK           = gcc
#############################################################################


# Name all static libraries with -ar.a suffix.
LibPattern = lib%-ar.a
LibDebugPattern = lib%-ar.a
LibNoDebugPattern = lib%-ar.a
LibSuffixPattern = %-ar.a
LibSearchPattern = -l%-ar

# Name all shared libraries with .a suffix
LibSharedPattern = lib%.a
LibSharedSuffixPattern = %.a
LibSharedSearchPattern = -l%

#
# OMNI thread stuff
#
ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_LIB = -lomnithread$(OMNITHREAD_MAJOR_VERSION)$(OMNITHREAD_MINOR_VERSION) $(OMNITHREAD_PLATFORM_LIB)
OMNITHREAD_STATIC_LIB = -lomnithread-ar -lpthreads-ar
OMNITHREAD_PLATFORM_LIB = -lpthreads

#
# CORBA stuff
#

CorbaImplementation = OMNIORB

CXXLINKOPTIONS=-Wl,-bbigtoc
CLINKOPTIONS=-Wl,-bbigtoc

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSharedSuffixPattern), $(filter-out $(LibSuffixPattern),$^)) $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSharedSuffixPattern), $(filter-out $(LibSuffixPattern),$^)) $$libs; \
)
endef

# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

##########################################################################
#
# Shared library support stuff
#
# Default setup. Work for most platforms. For those exceptions, override
# the rules in their platform files.
#
SHAREDLIB_SUFFIX   = a

SharedLibraryFullNameTemplate = lib$$1$$2$$3$$4.$(SHAREDLIB_SUFFIX)
SharedLibrarySoNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate = lib$$1$$2$$3.$(SHAREDLIB_SUFFIX)


BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) -shared -Wl,-bbigtoc \
     -o $$soname $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs ; \
 ar cq $@ $$soname; \
 $(RM) $$soname;
endef


# ExportSharedLibrary- export sharedlibrary
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  e.g. namespec = "COS 3 0 0" --> shared library libCOS300.a
#
define ExportSharedLibrary
$(ExportLibrary); \
$(ParseNameSpec); \
 libname=$(SharedLibraryLibNameTemplate); \
 set -x; \
 cd $(EXPORT_TREE)/$(LIBDIR); \
 $(RM) $$libname; \
 ln -s $(<F) $$libname;
endef

OPEN_SSL_ROOT = $(ABSTOP)/../../../openssl/work/src

OPEN_SSL_CPPFLAGS = -I$(OPEN_SSL_ROOT)/include
OPEN_SSL_LIB = -L$(OPEN_SSL_ROOT) -lssl -lcrypto
OMNIORB_SSL_LIB += $(OPEN_SSL_LIB)
OMNIORB_SSL_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)
