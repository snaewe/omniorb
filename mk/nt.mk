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
CXXOPTIONS = -MD
CXXMAKEDEPEND = gcc -M -undef -nostdinc -D_WIN32 -D_MT -D__cplusplus \
		-DMSC_VER=1000 -D_M_IX86=400

CXXLINK	= linkwrapper
CXXLINKOPTIONS = -libpath:/msdev/lib

CC = clwrapper
COPTIONS = -MD
CMAKEDEPEND = gcc -M -undef -nostdinc -D_WIN32 -D_MT -DMSC_VER=1000 \
		-D_M_IX86=400

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
$(SHELL) -ec "$(CMAKEDEPEND) $(CPPFLAGS) -I/msdev/include $< | sed 's/$*\\.o/& $@/g' > $@"
endef

define GenerateCXXDependencies
$(SHELL) -ec "$(CXXMAKEDEPEND) $(CPPFLAGS) -I/msdev/include $< | sed 's/$*\\.o/& $@/g' > $@"
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
LibSearchPattern = %.lib
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

CorbaImplementation = ORBIX

define CompileCorbaStubRule
$(CXX) $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(CORBA_CPPFLAGS) $(IMPORT_CPPFLAGS) \
    -c $< -Fo$@
endef

ORBIX_ROOT = /win32app/x86/ORBIX
ORBIX_IDL = idl -c C.cc -s S.cc -B
ORBIX_CPPFLAGS = -D__ORBIX__ -D__ORBIX_1_3 -DORBIX_MT -D_REENTRANT \
		 -I$(ORBIX_ROOT)/include -I$(CORBA_STUB_DIR)
ORBIX_LIB = $(ORBIX_ROOT)/LIB/ITSRVM.LIB WSOCK32.LIB ADVAPI32.LIB OLDNAMES.LIB
ORBIX_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh
ORBIX_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%S.cc
ORBIX_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%S.o
ORBIX_EXTRA_STUB_FILES = $(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%C.cc)


#
# OMNI thread stuff
#

ThreadSystem = NT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)
lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
