#
# powerpc_aix_4.3_xlc5.mk - make variables and rules specific to xlc version 5.0
#                           on PowerPC AIX 4.3
#

AIX = 1
PowerPCProcessor = 1

AIX_MAJOR_VERS := $(shell uname -v)
AIX_MINOR_VERS := $(shell uname -r)

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

IMPORT_CPPFLAGS += -D__aix__ -D__powerpc__ -D__OSVERSION__=${AIX_MAJOR_VERS}

#
# Standard programs
#

AR			= ar cq
RANLIB			= ranlib
MKDIRHIER		= mkdir -p
# INSTALL		= cp         # AIX does not have -p
# or use installbsd
INSTALL_USER	= `id -un`
INSTALL_GROUP	= `id -gn`
INSTALL		= installbsd -c -o $(INSTALL_USER) -g $(INSTALL_GROUP)

CMAKEDEPEND     += -D_AIX
CXXMAKEDEPEND   += -D__cplusplus -D_AIX

############################################################################
# Using xlC_r                                                              #
############################################################################

# xlC_r 3.1.4 & xlC_r 3.6.6

CXX             = xlC_r
CXXDEBUGFLAGS   =
CXXLINK		= xlC_r
CXXOPTIONS      = -qmaxmem=8192

# Use C Set++ to compile your C source.
#
CC		= xlC_r
CLINK           = xlC_r

# Get the compiler version
XLCVERSION := $(shell echo "__xlC__" > /tmp/testAIXCompilerVersion.C )
XLCVERSION := $(shell $(CXX) -+ -E /tmp/testAIXCompilerVersion.C | tail -1')

MAKECPPSHAREDLIB= /usr/ibmcxx/bin/makeC++SharedLib_r

ifeq ($(XLCVERSION),0x0301)
    MAKECPPSHAREDLIB = /usr/lpp/xlC/bin/makeC++SharedLib_r
else
ifeq ($(XLCVERSION),0x0500)
#   Visual Age took over xlC
	MAKECPPSHAREDLIB = /usr/vacpp/bin/makeC++SharedLib_r
endif
endif

############################################################################
# Using gcc 2.95 (built with --enable-threads)                             #
#      Not well tested yet.                                                #
#      Comment out the xlC_r section above and uncomment the following     #
############################################################################
#CXX             = g++
#CXXDEBUGFLAGS   = -g
#CXXOPTIONS      = -mt
#CXXLINK         = g++
#MTFLAGS         = -mthreads
#
#CC              = gcc
#CLINK           = gcc
#############################################################################


# Name all static libraries with -ar.a suffix.
LibPattern = lib%-ar.a
LibSuffixPattern = %-ar.a
LibSearchPattern = -l%-ar

# Name all shared libraries with .a suffix
LibSharedPattern = lib%.a
LibSharedSuffixPattern = %.a
LibSharedSearchPattern = -l%
#
# CORBA stuff
#

# For the moment, gatekeeper feature is disabled with shared library.
# Override the defaults set in unix.mk
#
#omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
#omniORBGatekeeperImplementation = NO_IMPL

#
# Notice that the version number 3.0 is hardwired in OMNIORB_LIB.
#
OMNIORB_LIB = $(patsubst %,$(LibSharedSearchPattern),omniORB30) \
               $(patsubst %,$(LibSharedSearchPattern),omniDynamic30) \
               $(OMNITHREAD_LIB) $(SOCKET_LIB)
lib_depend := $(patsubst %,$(LibSharedPattern),omniORB30) \
              $(patsubst %,$(LibSharedPattern),omniDynamic30)
OMNIORB_LIB_DEPEND1 := $(GENERATE_LIB_DEPEND)
OMNIORB_LIB_DEPEND = $(OMNIORB_LIB_DEPEND1) $(OMNITHREAD_LIB_DEPEND)

OMNIORB_LC_LIB = $(patsubst %,$(LibSharedSearchPattern),omniLC3)

CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#
ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=10
OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_LIB = -lomnithread2 -lpthreads
OMNITHREAD_STATIC_LIB = -lomnithread-ar -lpthreads-ar


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

SharedLibraryFullNameTemplate = lib$$1$$2$$3.$(SHAREDLIB_SUFFIX).$$4
SharedLibrarySoNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate = lib$$1$$2$$3.$(SHAREDLIB_SUFFIX)


ifeq ($(notdir $(CXX)),xlC_r)

BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS =

# MakeCXXSharedLibrary- Build shared library
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  extralibs = <libraries to add to the link line>
#
#  e.g. namespec="COS 3 0 0" --> shared library libCOS300.a
#       extralibs="$(OMNIORB_LIB)"
#

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(MAKECPPSHAREDLIB) \
    -o $$soname $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs \
         -p 40; \
 ar cq $@ $$soname; \
 $(RM) $$soname;
endef

endif

ifeq ($(notdir $(CXX)),g++)

BuildSharedLibrary = 1       # Enable
SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) -shared -mthreads \
     -o $$soname $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs ; \
 ar cq $@ $$soname; \
 $(RM) $$soname;
endef

endif


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
