#
# hppa_hpux_10.20.mk - make variables and rules specific to HPUX 10.20
#

HPUX = 1
HppaProcessor = 1

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

IMPORT_CPPFLAGS += -D__hppa__ -D__hpux__ -D__OSVERSION__=10


#
# Standard programs
#

AR = ar cq
RANLIB = ranlib
MKDIRHIER = mkdir -p
INSTALL		= cp -f
INSTLIBFLAGS	= 
INSTEXEFLAGS	= 

CPP = /lib/cpp

#############################################################################
# To use aCC uncomment the following lines:                                 #
#############################################################################
# Tested on:
#        HP aC++ B3910B A.01.04
#        HP aC++ B3910B A.01.01 Support Library
#
CXX = aCC
CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__cplusplus
CXXDEBUGFLAGS = -O2
CXXOPTIONS   += -I /opt/aCC/include +inst_v +DAportable -D_CMA_NOWRAPPERS_ 
CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -Wl,+s

CC                = cc
CMAKEDEPEND       = $(TOP)/$(BINDIR)/omkdepend
CDEBUGFLAGS       = -O2
COPTIONS	  = -Aa -D_HPUX_SOURCE +DAportable
CLINKOPTIONS      = -Wl,+s
CLINK             = $(CC)

# In order to compile Templates
# Override the default StaticLinkLibrary in unix.mk
#
define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(CXX) -c +inst_close $^; \
 $(AR) $@ $^; \
 $(RANLIB) $@; \
)
endef

############################################################################
# To use gcc uncomment the following lines:                                #
############################################################################
#CXX = g++
#CXXMAKEDEPEND = $(TOP)/$(BINDIR)/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
#CXXDEBUGFLAGS = 
# -D_CMA_NOWRAPPERS_ is needed otherwise linking omniNames results in
#                    /opt/aCC/lbin/ld: Unsatisfied symbols:
#                    fstreambase::cma_close(void)(code)
#CXXOPTIONS    =  -fhandle-exceptions -Wall -Wno-unused \
#                 -D_CMA_NOWRAPPERS_
#CXXLINK		= $(CXX)
# 
# Is GCCEXTRALINKOPTIONS necessary in a normal gcc setup? - SLL
#
#GCCEXTRALINKOPTIONS = -L/opt/gcc/lib/gcc -lg++
#CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(GCCEXTRALINKOPTIONS)
#
#PURIFY  = purify -g++ -collector=/opt/gcc/lib/gcc-lib/hppa1.1-hp-hpux10.20/2.7.2.1/ld \
#          -ignore-unknown-subspaces=yes -chain-length=18 \
#			 -show-directory=yes -show-pc=yes -show-pc-offset=yes
#
#CC                = gcc
#CMAKEDEPEND       = $(TOP)/$(BINDIR)/omkdepend __GNUC__
#CDEBUGFLAGS       = -O
#COPTIONS          = $(CDEBUGFLAGS) $(COPTIONS) \
#              $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))
#CLINKOPTIONS      = 
#CLINK             = $(CC)

#
# Socket library
#

SOCKET_LIB = 
THREAD_LIB = 


#
# CORBA stuff
#
#omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK
omniORBGatekeeperImplementation = OMNIORB_DUMMYGK
CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#


ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) -ldce -lcma
HPTHREADLIBS = -ldce -lcma

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


#
# OMNI ParTcl stuff
#

TCLTK_CPPFLAGS = -I/usr/local/include/Tcl7.4Tk4.0
TCLTK_LIB = -L/usr/local/lib -ltk4.0 -ltcl7.4 -lm -R /usr/local/lib
X11_CPPFLAGS = -I/usr/openwin/include
X11_LIB = -L/usr/openwin/lib -lX11 -R /usr/openwin/lib
WISH4 = /usr/local/bin/wish4.0

OMNIPARTCL_CPPFLAGS = $(TCLTK_CPPFLAGS) $(X11_CPPFLAGS) $(OMNITHREAD_CPPFLAGS)
OMNIPARTCL_LIB = $(patsubst %,$(LibSearchPattern),omniParTcl) $(TCLTK_LIB) \
		 $(X11_LIB) $(OMNITHREAD_LIB)
lib_depend := $(patsubst %,$(LibPattern),omniParTcl)
OMNIPARTCL_LIB_DEPEND := $(GENERATE_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)

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

SHAREDLIB_SUFFIX   = .sl

ifeq ($(notdir $(CXX)),aCC)

ELF_SHARED_LIBRARY = 1

SHAREDLIB_CPPFLAGS += +Z

define MakeCXXSharedLibrary
(set -x; \
$(RM) $@; \
  $(CXX) -b -Wl,+h$$soname -o $@  $(IMPORT_LIBRARY_FLAGS) \
      $(patsubst %,-L %,$(IMPORT_LIBRARY_DIRS)) \
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
