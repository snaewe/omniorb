#
# Standard make variables and rules for all Win32 platforms.
#

Win32Platform = 1
MinGW32Build = 1

SORT = sort


AR = ar cq
CXX = g++
CXXLINK	= g++
CC = gcc
CLINK = gcc
OMKDEPEND	= $(BASE_OMNI_TREE)/$(BINDIR)/omkdepend
CMAKEDEPEND = $(OMKDEPEND) -D__GNUC__
CXXMAKEDEPEND = $(OMKDEPEND) -D__cplusplus -D__GNUG__ -D__GNUC__

MKDIRHIER	= mkdir -p

INSTALL		= install -c
INSTLIBFLAGS	= 
INSTEXEFLAGS	= 

CP = cp
MV = mv -f

CDLLFLAGS=
CXXDLLFLAGS=
MSVC_STATICLIB_CXXNODEBUGFLAGS=
MSVC_STATICLIB_CXXDEBUGFLAGS=

CXXOPTIONS     = -mthreads
COPTIONS       = -mthreads

ifdef BuildDebugBinary

CXXLINKOPTIONS = -g -mthreads -Wl,--enable-runtime-pseudo-reloc
CXXDEBUGFLAGS    = -g
CLINKOPTIONS   = -g -mthreads -Wl,--enable-runtime-pseudo-reloc
CDEBUGFLAGS      = -g

else

CXXLINKOPTIONS = -mthreads -Wl,--enable-runtime-pseudo-reloc
CXXDEBUGFLAGS    = -O2
CLINKOPTIONS   = -mthreads -Wl,--enable-runtime-pseudo-reloc
CDEBUGFLAGS      = -O2

endif

ifndef WINVER
WINVER = 0x0400
endif

IMPORT_CPPFLAGS += -D__WIN32__ -D_WIN32_WINNT=$(WINVER)

SOCKET_LIB = -lws2_32 -lmswsock


#
# General rule for cleaning.
#

define CleanRule
$(RM) *.o *.a
endef

define VeryCleanRule
$(RM) *.d
$(RM) *.pyc
$(RM) $(CORBA_STUB_FILES)
endef


#
# Patterns for various file types
#

LibPathPattern = -L%
LibNoDebugPattern = lib%.a
LibDebugPattern = lib%d.a
DLLNoDebugPattern = lib%_rt.a
DLLDebugPattern = lib%_rtd.a
LibNoDebugSearchPattern = -l%
LibDebugSearchPattern = -l%d
DLLNoDebugSearchPattern = -l%_rt
DLLDebugSearchPattern = -l%_rtd


ifndef BuildDebugBinary

LibPattern = $(LibNoDebugPattern)
DLLPattern = $(DLLNoDebugPattern)
LibSearchPattern = $(LibNoDebugSearchPattern)
DLLSearchPattern = $(DLLNoDebugSearchPattern)

else

LibPattern = $(LibDebugPattern)
DLLPattern = $(DLLDebugPattern)
LibSearchPattern = $(LibDebugSearchPattern)
DLLSearchPattern = $(DLLDebugSearchPattern)

endif

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

IMPORT_LIBRARY_FLAGS = $(patsubst %,$(LibPathPattern),$(IMPORT_LIBRARY_DIRS))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -o$@ $(CXXLINKOPTIONS)  $(IMPORT_LIBRARY_FLAGS) \
      $(filter-out $(LibPattern),$^) $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -o$@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibPattern),$^) $$libs; \
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


###########################################################################
#
# Shared library support stuff
#
ifndef EmbeddedSystem
BuildSharedLibrary = 1
endif

SharedLibraryFullNameTemplate = lib$(SharedLibraryLibNameTemplate).a
SharedLibrarySearchTemplate = -l$(SharedLibraryLibNameTemplate)
SharedLibraryLibNameTemplate  = $$1$$2$$3$$4_rt$${extrasuffix:-}
SharedLibraryExportSymbolFileNameTemplate = $$1$$2$${extrasuffix:-}.def
SharedLibraryVersionStringTemplate = $$3.$$4
SharedLibrarySymbolRefLibraryTemplate = lib$${symrefdir:-static}/$$1$$2$${extrasuffix:-}.a

define SharedLibraryFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibraryFullNameTemplate); \
}; fn
endef

define SharedLibraryDebugFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
extrasuffix="d"; \
echo $(SharedLibraryFullNameTemplate); \
}; fn
endef

define SharedLibrarySearch
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibrarySearchTemplate); \
}; fn
endef

define SharedLibraryDebugSearch
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
extrasuffix="d"; \
echo $(SharedLibrarySearchTemplate); \
}; fn
endef

define ParseNameSpec
set $$namespec ; \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi
endef


# MakeCXXSharedLibrary- Build shared library
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  extralibs = <libraries to add to the link line>
#  debug = 1 (build debug version).
#
#  e.g. namespec="COS 3 0 0" --> COS300_rt.dll
#       extralibs="$(OMNIORB_LIB)"
#
define MakeCXXSharedLibrary
$(ParseNameSpec); \
extrasuffix=$${debug:+d}; \
targetdir=$(@D); \
libname=$(SharedLibraryLibNameTemplate); \
dllname=$$targetdir/$$libname.dll; \
defname=$$targetdir/$(SharedLibraryExportSymbolFileNameTemplate); \
version=$(SharedLibraryVersionStringTemplate); \
set -x; \
$(RM) $@; \
$(CXXLINK) -shared -mthreads -Wl,-export-all -Wl,--dll -o$$dllname \
-Wl,--out-implib,$@ -Wl,--output-def,$$defname \
-Wl,--enable-runtime-pseudo-reloc \
$$extralinkoption $(IMPORT_LIBRARY_FLAGS) \
$^ $$extralibs;
endef

# Export SharedLibrary
#   Expected shell variable:
#   namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#    e.g. namespec = "COS 3 0 0"
#
# NT treats DLLs more like executables -- the .dll file needs to go in the
# bin/x86... directory so that it's on your PATH:
#
define ExportSharedLibrary
$(ParseNameSpec); \
extrasuffix=$${debug:+d}; \
targetdir=$(<D); \
libname=$(SharedLibraryLibNameTemplate); \
dllname=$$targetdir/$$libname.dll; \
(dir="$(EXPORT_TREE)/$(LIBDIR)"; \
 file="$^"; \
 $(ExportFileToDir); \
(dir="$(EXPORT_TREE)/$(BINDIR)"; \
 file="$$dllname"; \
 $(ExportExecutableFileToDir); ); \
);
endef

# CleanSharedLibrary
#   Expected shell variable:
#      dir = directory name to clean. Default to . (current directory)
#
define CleanSharedLibrary
( set -x; \
$(RM) $${dir:-.}/*.dll $${dir:-.}/*.a $${dir:-.}/*.exp $${dir:-.}/*.def )
endef

# Pattern rules to build objects files for static and shared library and the
# debug versions for both.
# The convention is to build object files and libraries in different
# subdirectoryies.
#    static - the static library
#    debug  - the static debug library
#    shared - the DLL
#    shareddebug - the DLL debug library
#
# The pattern rules below ensured that the right compiler flags are used
# to compile the source for the library.

static/%.o: %.cc
	$(CXX) -c $(CXXDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(CXXOPTIONS) $(CPPFLAGS) -o$@ $<

debug/%.o: %.cc
	$(CXX) -c -g $(CXXOPTIONS) $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(CPPFLAGS) -o$@ $<

shared/%DynSK.o: %DynSK.cc
	$(CXX) -c $(CXXDLLFLAGS) $(CXXDEBUGFLAGS) $(CXXOPTIONS) -DUSE_core_stub_in_nt_dll $(CPPFLAGS) -o$@ $<

shared/%SK.o: %SK.cc
	$(CXX) -c $(CXXDLLFLAGS) $(CXXDEBUGFLAGS) $(CXXOPTIONS) -DUSE_dyn_stub_in_nt_dll $(CPPFLAGS) -o$@ $<

shared/%.o: %.cc
	$(CXX) -c $(CXXDLLFLAGS) $(CXXDEBUGFLAGS) $(CXXOPTIONS) $(CPPFLAGS) -o$@ $<


shareddebug/%DynSK.o: %DynSK.cc
	$(CXX) -c  $(CXXDLLFLAGS) -g $(CXXOPTIONS) -DUSE_core_stub_in_nt_dll $(CPPFLAGS) -o$@ $<

shareddebug/%SK.o: %SK.cc
	$(CXX) -c  $(CXXDLLFLAGS) -g $(CXXOPTIONS) -DUSE_dyn_stub_in_nt_dll $(CPPFLAGS) -o$@ $<

shareddebug/%.o: %.cc
	$(CXX) -c  $(CXXDLLFLAGS) -g $(CXXOPTIONS) $(CPPFLAGS) -o$@ $<

static/%.o: %.c
	$(CC) -c $(CDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(COPTIONS) $(CPPFLAGS) -o$@ $<

debug/%.o: %.c
	$(CC) -c $(CPPFLAGS) $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(COPTIONS) -g -o$@ $<

shared/%.o: %.c
	$(CC) -c $(CXXDLLFLAGS) $(CDEBUGFLAGS) $(COPTIONS) $(CPPFLAGS) -o$@ $<


shareddebug/%.o: %.c
	$(CC) -c $(CXXDLLFLAGS) -g $(COPTIONS) $(CPPFLAGS) -o$@ $<


#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -o$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<



#################################################################################
# CORBA stuff
#

# Note that the DLL version is being used, so link to omniorb3_rt.lib

include $(BASE_OMNI_TREE)/mk/version.mk

OMNIORB_DLL_NAME = $(shell $(SharedLibrarySearch) $(subst ., ,omniORB.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_DLL_NAME = $(shell $(SharedLibraryDebugSearch) $(subst ., ,omniORB.$(OMNIORB_VERSION)))

OMNIORB_DYNAMIC_DLL_NAME = $(shell $(SharedLibrarySearch) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_DYNAMIC_DLL_NAME = $(shell $(SharedLibraryDebugSearch) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))

OMNIORB_CODESETS_DLL_NAME = $(shell $(SharedLibrarySearch) $(subst ., ,omniCodeSets.$(OMNIORB_VERSION)))
OMNIORB_DEBUG_CODESETS_DLL_NAME = $(shell $(SharedLibraryDebugSearch) $(subst ., ,omniCodeSets.$(OMNIORB_VERSION)))


ifndef BuildDebugBinary

omniorb_dll_name := $(OMNIORB_DLL_NAME)
omnidynamic_dll_name := $(OMNIORB_DYNAMIC_DLL_NAME)
omnicodesets_dll_name := $(OMNIORB_CODESETS_DLL_NAME)

else

omniorb_dll_name := $(OMNIORB_DEBUG_DLL_NAME)
omnidynamic_dll_name := $(OMNIORB_DEBUG_DYNAMIC_DLL_NAME)
omnicodesets_dll_name := $(OMNIORB_DEBUG_CODESETS_DLL_NAME)
endif

lib_depend := $(omniorb_dll_name)
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnidynamic_dll_name)
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnicodesets_dll_name)
omniCodeSets_lib_depend := $(GENERATE_LIB_DEPEND)

#OMNIIDL = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/oidlwrapper.exe $(XLN)
OMNIIDL = $(BASE_OMNI_TREE)/$(BINDIR)/omniidl
OMNIORB_IDL_ONLY = $(OMNIIDL) -T -bcxx -Wbh=.hh -Wbs=SK.cc
OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB4__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)
OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%


OMNIORB_LIB = $(omniorb_dll_name) \
		$(omnidynamic_dll_name) \
		$(OMNITHREAD_LIB) $(SOCKET_LIB) -ladvapi32
OMNIORB_LIB_NODYN = $(omniorb_dll_name) \
		$(OMNITHREAD_LIB) $(SOCKET_LIB) -ladvapi32

OMNIORB_LIB_NODYN_DEPEND := $(omniORB_lib_depend) \
                            $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_DEPEND := $(omniORB_lib_depend) \
                      $(OMNITHREAD_LIB_DEPEND) \
		      $(omniDynamic_lib_depend)

# CodeSets library
OMNIORB_CODESETS_LIB = $(omnicodesets_dll_name)
OMNIORB_CODESETS_LIB_DEPEND := $(omniCodeSets_lib_depend)



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


CorbaImplementation = OMNIORB

#
# OMNI thread stuff
#
# Note that the DLL version is being used, so link to omnithread_rt.lib

# Use native win32 threads
ThreadSystem = NT

OMNITHREAD_LIB = $(patsubst %,$(DLLSearchPattern),omnithread$(OMNITHREAD_MAJOR_VERSION)$(OMNITHREAD_MINOR_VERSION))
lib_depend := $(patsubst %,$(DLLPattern),omnithread$(OMNITHREAD_MAJOR_VERSION)$(OMNITHREAD_MINOR_VERSION))
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

OMNITHREAD_PLATFORM_LIB =


# omniORB SSL transport
OMNIORB_SSL_VERSION = $(OMNIORB_MAJOR_VERSION).$(OMNIORB_MINOR_VERSION)
OMNIORB_SSL_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_LIB = $(patsubst %,$(DLLSearchPattern),omnisslTP$(OMNIORB_SSL_MAJOR_VERSION)$(OMNIORB_SSL_MINOR_VERSION))
lib_depend := $(patsubst %,$(DLLPattern),omnisslTP$(OMNIORB_SSL_MAJOR_VERSION)$(OMNIORB_SSL_MINOR_VERSION))
OMNIORB_SSL_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
