#
# Standard make variables and rules for all NT platforms.
#

NTArchitecture = 1

#
# Standard "unix" programs.  Anything here not provided by the GNU-WIN32
# system is likely to need a wrapper around it to perform filename translation.
#

MKDIRHIER	= mkdir -p

INSTALL		= install -c
INSTLIBFLAGS	= -m 0644
INSTEXEFLAGS	= -m 0755

CP = cp
MV = mv -f

AR = libwrapper

CXX = clwrapper
CXXDEBUGFLAGS =
CXXOPTIONS = -MD -GX
CXXMAKEDEPEND = omkdepend -D_WIN32 -D_MT -D__cplusplus \
		-DMSC_VER=1000 -D_M_IX86=400

CXXLINK	= linkwrapper
CXXLINKOPTIONS = -libpath:/msdev/lib

CC = clwrapper
COPTIONS = -MD
CMAKEDEPEND = omkdepend -D_WIN32 -D_MT -DMSC_VER=1000 -D_M_IX86=400

CLINK = linkwrapper
CLINKOPTIONS = -libpath:/msdev/lib

IMPORT_CPPFLAGS += -D__NT__


#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -Fo$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<


#
# Rule to generate dependency files
#

define GenerateCDependencies
$(CMAKEDEPEND) $(CPPFLAGS) -I/msdev/include -f $@ $<
endef

define GenerateCXXDependencies
$(CXXMAKEDEPEND) $(CPPFLAGS) -I/msdev/include -f $@ $<
endef


#
# General rule for cleaning.
#

define CleanRule
$(RM) *.o *.lib
endef

define VeryCleanRule
$(RM) *.d
$(RM) $(CORBA_STUB_FILES)
endef


#
# Patterns for various file types
#

LibPattern = %.lib
DLLPattern = %_rt.lib
LibSearchPattern = %.lib
DLLSearchPattern = %_rt.lib
BinPattern = %.exe


#
# Stuff to generate statically-linked libraries.
#

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(AR) $@ $^; \
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

IMPORT_LIBRARY_FLAGS = $(patsubst %,-libpath:%/lib/$(platform),$(IMPORT_TREES))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -out:$@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $^ $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -out:$@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $^ $$libs; \
)
endef

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


#
# CORBA stuff
#

define CompileCorbaStubRule
$(CXX) $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(CORBA_CPPFLAGS) $(IMPORT_CPPFLAGS) \
    -c $< -Fo$@
endef

# Note that the DLL version is being used, so link to omniorb2_rt.lib
OMNIORB2_IDL = /project/omni/version5.1/bin/x86_nt_3.5/omniidl2 -h .hh -s SK.cc
OMNIORB2_CPPFLAGS = -D__OMNIORB2__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS) -D "WIN32" -D "_X86_"
OMNIORB2_LIB = $(patsubst %,$(DLLSearchPattern),omniORB2) $(OMNITHREAD_LIB) wsock32.lib advapi32.lib
lib_depend := $(patsubst %,$(DLLPattern),omniORB2)
OMNIORB2_LIB_DEPEND := $(GENERATE_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)
OMNIORB2_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh
OMNIORB2_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%SK.cc
OMNIORB2_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%SK.o


ORBIX2_ROOT = /win32app/x86/ORBIX
ORBIX2_IDL = idl -c C.cc -s S.cc -B
ORBIX2_CPPFLAGS = -D__ORBIX__ -D__ORBIX_2_1 -DORBIX_MT -D_REENTRANT \
		 -I$(ORBIX2_ROOT)/include -I$(CORBA_STUB_DIR)
ORBIX2_LIB = $(ORBIX2_ROOT)/LIB/ITSRVM.LIB WSOCK32.LIB ADVAPI32.LIB OLDNAMES.LIB
ORBIX2_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh
ORBIX2_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%S.cc
ORBIX2_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%S.o
ORBIX2_EXTRA_STUB_FILES = $(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%C.cc)

CorbaImplementation = OMNIORB2
#
# OMNI thread stuff
#
# Note that the DLL version is being used, so link to omnithread_rt.lib

ThreadSystem = NT
OMNITHREAD_LIB = $(patsubst %,$(DLLSearchPattern),omnithread)
lib_depend := $(patsubst %,$(DLLPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
