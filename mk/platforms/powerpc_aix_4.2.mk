#
# powerpc_aix_4.2.mk - make variables and rules specific to AIX 4.2 on 
#                      PowerPC.
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

IMPORT_CPPFLAGS = -D__aix__ -D__powerpc__ -D__OSVERSION__=4

#
# Standard programs
#

AR              = ar cq
RANLIB		= ranlib
MKDIRHIER	= /usr/bin/X11/mkdirhier
# INSTALL         = cp         # AIX does not have -p
# or use installbsd
INSTALL_USER  = `id -un`
INSTALL_GROUP = `id -gn`
INSTALL       = installbsd -c -o $(INSTALL_USER) -g $(INSTALL_GROUP)

CMAKEDEPEND     = $(TOP)/$(BINDIR)/omkdepend -D_AIX
CXXMAKEDEPEND   = $(TOP)/$(BINDIR)/omkdepend -D__cplusplus -D_AIX

############################################################################
# Using xlC_r                                                              #
############################################################################

# xlC_r 3.1.4 & xlC_r 3.6.6

CXX             = xlC_r
CXXDEBUGFLAGS   = -O
CXXLINK		= xlC_r

# Use C Set++ to compile your C source.
#
CC              = xlC_r
CLINK           = xlC_r

# Get the compiler version
XLCVERSION := $(shell echo "__xlC__" > /tmp/testAIXCompilerVersion.C )
XLCVERSION := $(shell $(CXX) -+ -E /tmp/testAIXCompilerVersion.C | tail -1')
  
MAKECPPSHAREDLIB= /usr/ibmcxx/bin/makeC++SharedLib_r
 
ifeq ($(XLCVERSION),0x0301)
    MAKECPPSHAREDLIB = /usr/lpp/xlC/bin/makeC++SharedLib_r
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

OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep -DPthreadDraftVersion=8
OMNITHREAD_CPPFLAGS = -I$(TOP)/include -D_REENTRANT -D_THREAD_SAFE
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
SHAREDLIB_SUFFIX   = a

SharedLibraryFullName = $(shell fn() {\
                                  echo lib$$1$$2$$3.$(SHAREDLIB_SUFFIX).$$4; \
                                 }; \
                                fn $(subst ., ,$(SharedLibraryNameSpec)))

SharedLibrarySoName = $(shell fn() {\
                                 echo lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3; \
                              }; \
                              fn $(subst ., ,$(SharedLibraryNameSpec)))

SharedLibraryName = $(shell fn() {\
                                echo lib$$1$$2$$3.$(SHAREDLIB_SUFFIX); \
                            }; \
                            fn $(subst ., ,$(SharedLibraryNameSpec)))

ifeq ($(notdir $(CXX)),xlC_r)

ELF_SHARED_LIBRARY = 1
SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
(set -x; \
 $(RM) $@; \
 $(MAKECPPSHAREDLIB) \
    -o $$soname $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs \
         -p 40; \
         ar cq $@ $$soname; \
         $(RM) $$soname; \
       )
endef

endif

ifeq ($(notdir $(CXX)),g++)

ELF_SHARED_LIBRARY = 1
SHAREDLIB_CPPFLAGS =

define MakeCXXSharedLibrary
(set -x; \
 $(RM) $@; \
   $(CXXLINK) -shared -mthreads \
     -o $$soname $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$extralibs ; \
         ar cq $@ $$soname; \
         $(RM) $$soname; \
       )
endef

endif


define ExportSharedLibrary
$(ExportLibrary); \
(set -x; \
   cd $(EXPORT_TREE)/$(LIBDIR); \
    $(RM) $$libname; \
    ln -s $^ $$libname; \
  )
endef
