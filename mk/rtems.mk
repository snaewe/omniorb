###########################################################################
#
# $Header$
#
# Copyright (c) 1999-2000 ConnectTel, Inc. All Rights Reserved.
#  
# MODULE DESCRIPTION: rtems.mk
# 
#  by: Rosimildo da Silva:
#      rdasilva@connecttel.com
#      http://www.connecttel.com
#
# MODIFICATION/HISTORY:
#
# $Log$
# Revision 1.4  2001/02/21 14:12:24  dpg1
# Merge from omni3_develop for 3.0.3 release.
#
# Revision 1.1.2.4  2000/10/02 16:35:41  dpg1
# Updates for 3.0.2 release
#
# Revision 1.1.2.3  2000/08/18 11:15:43  sll
# Make current tree into omniORB 3.0.1 release.
# Turn on <top>/src/services build by default.
#
# Revision 1.1.2.2  2000/08/18 09:53:01  sll
# Latest RTEMS updates.
#
# Revision 1.1.2.1  2000/08/17 15:37:51  sll
# Merged RTEMS port.
#
#
###########################################################################

# TOOLSET -- configuration -- these macros needs to be localized to match
#            the locaction of the installation of your toolset on your host.
RTEMS_BUILD         = /opt/rtems
RTEMS_INSTALL_POINT = /opt/rtems
RTEMS_CC_PREFIX     = i386-rtems-
RTEMS_BSP           = pc486
PROJECT_RELEASE     = $(RTEMS_BUILD)/$(RTEMS_BSP)
PROJECT_TOOLS       = $(RTEMS_BUILD)/$(RTEMS_BSP)/build-tools
RTEMS               = 1
x86Processor        = 1

# Define OrbCoreOnly if only want to build the runtime with no support 
# for dynamic interfaces, e.g. DII, DSI, Any, Typecode etc.
#OrbCoreOnly = 1

# Define EmbeddedSystem causes the build process to only build src/lib.
#
EmbeddedSystem = 1

# RTEMS uses similar make/build structure as Unix, so turn this on
UnixPlatform = 1

# No gatekeeper
NoGateKeeper = 1

#
# Global defines to build the ORB runtime libraries.
#
# IMPORT_CPPFLAGS += -D__rtems__
# Not necessary because __rtems__ is builtin to the cross compiler


#
OMNITHREAD_CPPFLAGS += -DPthreadDraftVersion=10 -DPthreadSupportThreadPriority \
                        -DThreadStackSize=4096*4

#
# The HOSTBINDIR variable is used to locate the HOST IDL compiler.
# This needs to be changed if your host it is NOT CygWin.
#
# NOTE: ******* >>> Please change this to match your host  <<< **************
#
HOSTBINDIR = bin/x86_win32


# BINDIR and LIBDIR contain the binaries and libraries of the target.
BINDIR     = $(HOSTBINDIR)
LIBDIR     = lib/$(platform)
#
# Under Windows, when Microsoft VC+compiler is used, extra flags needs 
# to be defined. Undefine this macro when compiling under an UNIX or LINUX host.
HOST_IDL_FLAGS = -h .hh -s SK.cc

LibPattern=%lib.a
ThreadSystem=Posix

CFLAGS = 
CPPFLAGS = 
DEFS = -DHAVE_CONFIG_H
LDFLAGS =
LIBS = 
CC_FOR_BUILD = gcc

CC = 		   $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)gcc
AS = 		   $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)as
AR = 		   $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)ar
NM = 		   $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)nm
LD = 		   $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)ld
SIZE = 		$(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)size
OBJCOPY = 	$(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)objcopy
RANLIB = 	$(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)ranlib
CXX =       $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)g++

export CC
export AS
export LD
export NM
export AR
export SIZE
export OBJCOPY
export CXX


MKDIRHIER	= mkdir -p
INSTALL		= install -c
INSTLIBFLAGS	= -m 0644
INSTEXEFLAGS	= -m 0755

CP		 = cp
MV		 = mv -f
CPP = $(RTEMS_INSTALL_POINT)/bin/cpp
CXXLINK  = $(RTEMS_INSTALL_POINT)/bin/$(RTEMS_CC_PREFIX)g++ --pipe
CFLAGS = -O4 -Wall -Wno-unused -fasm -g \
-B$(RTEMS_BUILD)/$(RTEMS_BSP)/lib/ -specs bsp_specs -qrtems $(IMPORT_CPPFLAGS) \
$(DIR_CPPFLAGS) -I$(RTEMS_BUILD)/$(RTEMS_BSP)/lib/include \
-I$(RTEMS_BUILD)/$(RTEMS_BSP)/lib/include/networking \
-I$(RTEMS_BUILD)/$(RTEMS_BSP)/lib/include/sys

CXXFLAGS = $(CFLAGS)

ARFLAGS  = -ruv

OMKDEPEND = $(BASE_OMNI_TREE)/$(HOSTBINDIR)/omkdepend

CXXMAKEDEPEND   = $(OMKDEPEND) -D__cplusplus -D__GNUG__ -D__GNUC__
CMAKEDEPEND     = $(OMKDEPEND) -D__GNUC__

#
# General rules for cleaning.
#

define CleanRule
$(RM) *.o *.a *.class *.num *.bt *.bin *.exe
endef

define VeryCleanRule
$(RM) *.d
$(RM) $(CORBA_STUB_FILES)
endef

#
# Patterns for various file types
#

LibPattern = lib%.a
LibSuffixPattern = %.a
LibSearchPattern = -l%
BinPattern = %
TclScriptPattern = %


#
# Stuff to generate statically-linked libraries.
#

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(AR) $(ARFLAGS) $@ $^; \
 $(RANLIB) $@; \
)
endef

ifdef EXPORT_TREE
define ExportLibrary
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportFileToDir); \
 done; \
)
endef
endif


#
# Stuff to generate executable binaries.
#
# These rules are used like this
#
# target: objs lib_depends
#         @(libs="libs"; $(...Executable))
#
# The command we want to generate is like this
#
# linker -o target ... objs libs
# i.e. we need to filter out the lib_depends from the command
#

IMPORT_LIBRARY_FLAGS = $(patsubst %,-L%,$(IMPORT_LIBRARY_DIRS))

ifdef EXPORT_TREE
define ExportExecutable
(dir="$(EXPORT_TREE)/$(BINDIR)"; \
 files="$^"; \
 for file in $$files; do \
   $(ExportExecutableFileToDir); \
 done; \
)
endef
endif


OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)
lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


#
# CORBA stuff
#
CorbaImplementation = OMNIORB

OMNIORB_VERSION = 3.0.2
OMNIORB_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_VERSION)))

lib_depend := $(patsubst %,$(LibPattern),omniORB$(OMNIORB_MAJOR_VERSION))
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(patsubst %,$(LibPattern),omniDynamic$(OMNIORB_MAJOR_VERSION))
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

OMNIORB_IDL_ONLY = $(BASE_OMNI_TREE)/$(BINDIR)/omniidl -bcxx
ifndef OrbCoreOnly
OMNIORB_IDL_ANY_FLAGS = -Wba
endif
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB$(OMNIORB_MAJOR_VERSION)__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)
OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%

OMNIORB_LIB_NODYN = $(patsubst %,$(LibSearchPattern),omniORB$(OMNIORB_MAJOR_VERSION))
ifdef OrbCoreOnly
OMNIORB_LIB = $(OMNIORB_LIB_NODYN)
else
OMNIORB_LIB = $(OMNIORB_LIB_NODYN) \
       $(patsubst %,$(LibSearchPattern),omniDynamic$(OMNIORB_MAJOR_VERSION))
endif

OMNIORB_LIB_NODYN_DEPEND = $(omniORB_lib_depend)
OMNIORB_LIB_DEPEND = $(omniORB_lib_depend) $(omniDynamic_lib_depend)

OMNIORB_STATIC_STUB_OBJS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%SK.o)
OMNIORB_STATIC_STUB_SRCS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%SK.cc)
OMNIORB_DYN_STUB_OBJS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%DynSK.o)
OMNIORB_DYN_STUB_SRCS = \
	$(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%DynSK.cc)

OMNIORB_STUB_SRCS = $(OMNIORB_STATIC_STUB_SRCS) $(OMNIORB_DYN_STUB_SRCS)
OMNIORB_STUB_OBJS = $(OMNIORB_STATIC_STUB_OBJS) $(OMNIORB_DYN_STUB_OBJS)

OMNIORB_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%SK.cc
OMNIORB_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%SK.o
OMNIORB_DYN_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%DynSK.cc
OMNIORB_DYN_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%DynSK.o
OMNIORB_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh


# omniORB access control policy modules

OMNIORB_DUMMYGK_LIB = $(patsubst %,$(LibSearchPattern),omniGK_stub)
lib_depend := $(patsubst %,$(LibPattern),omniGK_stub)
OMNIORB_DUMMYGK_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

OMNIORB_TCPWRAPGK_LIB = $(patsubst %,$(LibSearchPattern),tcpwrapGK)
lib_depend := $(patsubst %,$(LibPattern),tcpwrapGK)
OMNIORB_TCPWRAPGK_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

omniORBGatekeeperImplementation = OMNIORB_DUMMYGK

OMNIORB_LIB += $($(omniORBGatekeeperImplementation)_LIB)
OMNIORB_LIB_NODYN += $($(omniORBGatekeeperImplementation)_LIB)
OMNIORB_LIB_DEPEND += $($(omniORBGatekeeperImplementation)_LIB_DEPEND)
OMNIORB_LIB_NODYN_DEPEND += $($(omniORBGatekeeperImplementation)_LIB_DEPEND)

# thread libraries required by omniORB. Make sure this is the last in
# the list of omniORB related libraries

OMNIORB_LIB += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_NODYN += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_DEPEND += $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_NODYN_DEPEND += $(OMNITHREAD_LIB_DEPEND)

# LifeCycle stuff

OMNIORB_IDL_LC_FLAGS = -l
OMNIORB_LC_LIB = $(patsubst %,$(LibSearchPattern),omniLC)

# Pattern rules to build  objects files for static.
# The convention is to build the static library in the subdirectoy "static"
# The pattern rules below ensured that the right compiler flags are used
# to compile the source for the library.

static/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<

static/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<


