# x86_freebsd_3.2.mk - make variables and rules specific to FreeBSD 3.2
# started: 1999/06/12 afo
# [Last modified: 1999/06/13 12:42:06 by afo@xxo.zlug.org]

FreeBSD = 1
x86Processor = 1

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

CPP = /usr/libexec/cpp

CXX = egcs++
CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2

CXXLINK		= $(CXX) -pthread -s
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS)

CXXOPTIONS      = -Wall -Wno-unused
EgcsMajorVersion = 1
EgcsMinorVersion = 1        # This is actually a post-1.0 egcs snapshot.

CC           = egcs
CMAKEDEPEND  = $(TOP)/$(BINDIR)/omkdepend -D__GNUC__
CDEBUGFLAGS  = -O2

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS)

INSTALL = install -c

IMPORT_CPPFLAGS += -D__x86__ -D__freebsd__ -D__OSVERSION__=3


omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS =
OMNITHREAD_CPPFLAGS = -D_REENTRANT -D_THREAD_SAFE
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

OMNITHREAD_POSIX_CPPFLAGS += -DUsePthread -DPthreadDraftVersion=10

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

# Uncomment the following if you want all utils and examples build in
# this tree to statically linke with the omniORB libraries.
# build tools semistatic : shared with system libs and static with omniORB libs
#CORBA_LIB	= -Wl,-Bstatic
#CORBA_LIB	+= -lomniORB3 -lomniDynamic2 -ltcpwrapGK -lomnithread
#CORBA_LIB	+= -Wl,-Bdynamic
#OMNITHREAD_LIB = 
#CORBA_LIB_NODYN	= -Wl,-Bstatic
#CORBA_LIB_NODYN	+= -lomniORB3 -ltcpwrapGK -lomnithread
#CORBA_LIB_NODYN	+= -Wl,-Bdynamic
#OMNITHREAD_LIB_NODYN = -lomnithread

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
SHAREDLIB_SUFFIX   = .so

ifdef EgcsMajorVersion

ELF_SHARED_LIBRARY = 1

SHAREDLIB_CPPFLAGS = -fPIC

define MakeCXXSharedLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -shared -Wl,-soname,$$soname -o $@ $(IMPORT_LIBRARY_FLAGS) \
 $(filter-out $(LibSuffixPattern),$^) $$extralibs; \
)
endef


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

endif
