#
# Standard make variables and rules for all Win32 platforms.
#

Win32Platform = 1

#
# Define macro for path of wrapper executables
#
ifndef EmbeddedSystem
WRAPPER_FPATH = $(BINDIR)
else
WRAPPER_FPATH = $(HOSTBINDIR)
endif


#
# Standard "unix" programs.  Anything here not provided by the GNU-WIN32/OpenNT/UWIN
# system is likely to need a wrapper around it to perform filename translation.
#
ifndef OpenNTBuildTree

# GNU-WIN32 wrappers
XLN = -gnuwin32

# There is a sort in %System32%/sort.exe and in GNU-WIN32. The shell of
# GNU-WIN32 may pick either one depending on the PATH setup of the user.
# To make sure that the GNU-WIN32 version is picked up, give the pathname
# of sort.

SORT = /bin/sort

else

# OpenNT or UWIN wrappers
XLN = -opennt
MKDEPOPT = -opennt
SORT = sort

endif


AR = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/libwrapper $(XLN)
CXX = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/clwrapper $(XLN)
CXXLINK	= $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/linkwrapper $(XLN)
CXXMAKEDEPEND = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/omkdepend $(MKDEPOPT) -D__cplusplus -D_MSC_VER
CC = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/clwrapper $(XLN)
CLINK = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/linkwrapper $(XLN)
CMAKEDEPEND = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/omkdepend $(MKDEPOPT) -D_MSC_VER

MKDIRHIER	= mkdir -p

INSTALL		= install -c
INSTLIBFLAGS	= 
INSTEXEFLAGS	= 

CP = cp
MV = mv -f


# Use the following set of flags to build and use multithreaded DLLs
#
MSVC_DLL_CXXNODEBUGFLAGS       = -MD -GX
MSVC_DLL_CXXLINKNODEBUGOPTIONS = 
MSVC_DLL_CNODEBUGFLAGS         = -MD
MSVC_DLL_CLINKNODEBUGOPTIONS   = 
#
MSVC_DLL_CXXDEBUGFLAGS         = -MDd -GX -Z7 -Od 
MSVC_DLL_CXXLINKDEBUGOPTIONS   = -debug -PDB:NONE
MSVC_DLL_CDEBUGFLAGS           = -MDd -Z7 -Od
MSVC_DLL_CLINKDEBUGOPTIONS     = -debug -PDB:NONE
#
# Or
#
# Use the following set of flags to build and use multithread static libraries
#
MSVC_STATICLIB_CXXNODEBUGFLAGS       = -MT -GX
MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS = 
MSVC_STATICLIB_CNODEBUGFLAGS         = -MT
MSVC_STATICLIB_CLINKNODEBUGOPTIONS   = 

MSVC_STATICLIB_CXXDEBUGFLAGS         = -MTd -GX -Z7 -Od 
MSVC_STATICLIB_CXXLINKDEBUGOPTIONS   = -debug -PDB:NONE
MSVC_STATICLIB_CDEBUGFLAGS           = -MTd -Z7 -Od
MSVC_STATICLIB_CLINKDEBUGOPTIONS     = -debug -PDB:NONE


ifdef BuildDebugBinary

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKDEBUGOPTIONS)
CXXDEBUGFLAGS  = 
CXXOPTIONS     = $(MSVC_DLL_CXXDEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKDEBUGOPTIONS)
CDEBUGFLAGS    = $(MSVC_DLL_CDEBUGFLAGS)

else

CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKNODEBUGOPTIONS)
CXXDEBUGFLAGS  = -O2
CXXOPTIONS     = $(MSVC_DLL_CXXNODEBUGFLAGS)
CLINKOPTIONS   = $(MSVC_DLL_CLINKNODEBUGOPTIONS)
CDEBUGFLAGS    = -O2
COPTIONS       = $(MSVC_DLL_CNODEBUGFLAGS)

endif

IMPORT_CPPFLAGS += -D__WIN32__

SOCKET_LIB = wsock32.lib


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

LibPathPattern = -libpath:%/$(LIBDIR)
LibNoDebugPattern = %.lib
LibDebugPattern = %d.lib
DLLNoDebugPattern = %_rt.lib
DLLDebugPattern = %_rtd.lib
LibNoDebugSearchPattern = %.lib
LibDebugSearchPattern = %d.lib
DLLNoDebugSearchPattern = %_rt.lib
DLLDebugSearchPattern = %_rtd.lib


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

IMPORT_LIBRARY_FLAGS = $(patsubst %,$(LibPathPattern),$(IMPORT_TREES))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -out:$@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
      $(filter-out $(LibPattern),$^) $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -out:$@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibPattern),$^) $$libs; \
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
BuildSharedLibrary = 1

SharedLibraryFullNameTemplate = $(SharedLibraryLibNameTemplate).lib
SharedLibraryLibNameTemplate  = $$1$$2$$3$$4_rt$${extrasuffix:-}
SharedLibraryExportSymbolFileNameTemplate = $$1$$2$${extrasuffix:-}.def
SharedLibraryVersionStringTemplate = $$3.$$4
SharedLibrarySymbolRefLibraryTemplate = $${symrefdir:-static}/$$1$$2$${extrasuffix:-}.lib

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

define ParseNameSpec
set $$namespec ; \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi
endef


# MakeCXXExportSymbolDefinitionFile
#   Internal canned command used by MakeCXXSharedLibrary
#
#  - Create a .def file containing all the functions and static class 
#    variables exported by the DLL. The symbols are extracted from the 
#    output of dumpbin.
#
#    The function symbols are extracted using the following template:
#    ... ........ SECT..  notype ()     External      | ?..................
#
#    The static class variable symbols are extracted using the following
#    template:
#    ... ........ SECT..  notype        External      | ?[^?]..............
#
#    Default destructors generated by the compiler are excluded.
#
#    It looks like class variable and function symbols start with two ??
#    and class static variable and static function symbols start with one ?.
#                                                             - SLL
#
define MakeCXXExportSymbolDefinitionFile
symrefdir=$${debug:+debug}; \
symreflib=$(SharedLibrarySymbolRefLibraryTemplate); \
if [ ! -f $$symreflib ]; then echo "Cannot find reference static library $$symreflib"; return 1; fi;  \
set -x; \
echo "LIBRARY $$libname" > $$defname; \
echo "VERSION $$version" >> $$defname; \
echo "EXPORTS" >> $$defname; \
DUMPBIN.EXE /SYMBOLS $$symreflib | \
egrep '^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +\(\) +External +\| +\?[^ ]*|^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +External +\| +\?[^?][^ ]*'|\
egrep -v 'deleting destructor[^(]+\(unsigned int\)' | \
cut -d'|' -f2 | \
cut -d' ' -f2 | $(SORT) -u >> $$defname; \
set +x;
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
$(MakeCXXExportSymbolDefinitionFile) \
set -x; \
$(RM) $@; \
$(CXXLINK) -out:$$dllname -DLL $(MSVC_DLL_CXXLINKNODEBUGOPTIONS) \
-def:$$defname -IMPLIB:$@ $(IMPORT_LIBRARY_FLAGS) \
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
$(RM) $${dir:-.}/*.dll $${dir:-.}/*.lib $${dir:-.}/*.exp $${dir:-.}/*.def )
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
	$(CXX) -c $(CXXDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

debug/%.o: %.cc
	$(CXX) -c  $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shared/%DynSK.o: %DynSK.cc
	$(CXX) -c $(CXXDEBUGFLAGS) -DUSE_core_stub_in_nt_dll $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shared/%SK.o: %SK.cc
	$(CXX) -c $(CXXDEBUGFLAGS) -DUSE_dyn_stub_in_nt_dll $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shared/%.o: %.cc
	$(CXX) -c $(CXXDEBUGFLAGS) $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<


shareddebug/%DynSK.o: %DynSK.cc
	$(CXX) -c  -DUSE_core_stub_in_nt_dll $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shareddebug/%SK.o: %SK.cc
	$(CXX) -c  -DUSE_dyn_stub_in_nt_dll $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shareddebug/%.o: %.cc
	$(CXX) -c  $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

static/%.o: %.c
	$(CC) -c $(CDEBUGFLAGS) $(MSVC_STATICLIB_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

debug/%.o: %.c
	$(CC) -c $(MSVC_STATICLIB_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<

shared/%.o: %.c
	$(CC) -c $(CDEBUGFLAGS) $(MSVC_DLL_CXXNODEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<


shareddebug/%.o: %.c
	$(CC) -c  $(MSVC_DLL_CXXDEBUGFLAGS) $(CPPFLAGS) -Fo$@ $<


#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -Fo$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<



#################################################################################
# CORBA stuff
#

# Note that the DLL version is being used, so link to omniorb3_rt.lib

OMNIORB_VERSION = 3.0.0
OMNIORB_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_VERSION)))

ifndef BuildDebugBinary

omniorb_dll_name := $(shell $(SharedLibraryFullName) $(subst ., ,omniORB.$(OMNIORB_VERSION)))
omnidynamic_dll_name := $(shell $(SharedLibraryFullName) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))

else

omniorb_dll_name := $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniORB.$(OMNIORB_VERSION)))
omnidynamic_dll_name := $(shell $(SharedLibraryDebugFullName) $(subst ., ,omniDynamic.$(OMNIORB_VERSION)))

endif

lib_depend := $(omniorb_dll_name)
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(omnidynamic_dll_name)
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

OMNIORB_IDL_ONLY = \
  $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/oidlwrapper.exe $(XLN) -bcxx -Wbh=.hh -Wbs=SK.cc
OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB3__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)
OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%

msvc_work_around_stub = $(patsubst %,$(LibPattern),msvcstub) \
                        -NODEFAULTLIB:libcmt.lib -NODEFAULTLIB:libcmtd.lib 

OMNIORB_LIB = $(omniorb_dll_name) \
		$(omnidynamic_dll_name) \
		$(OMNITHREAD_LIB) wsock32.lib advapi32.lib
OMNIORB_LIB_NODYN = $(omniorb_dll_name) $(msvc_work_around_stub) \
		$(OMNITHREAD_LIB) wsock32.lib advapi32.lib

OMNIORB_LIB_NODYN_DEPEND := $(omniORB_lib_depend) $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_DEPEND := $(omniORB_lib_depend) $(OMNITHREAD_LIB_DEPEND) \
			$(omniDynamic_lib_depend)

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

# Use pthread_nt, comment out ThreadSystem line above and uncomment the
# following 2 lines.
#ThreadSystem = NTPosix
#OMNITHREAD_CPPFLAGS= -D__POSIX_NT__



OMNITHREAD_LIB = $(patsubst %,$(DLLSearchPattern),omnithread2)
lib_depend := $(patsubst %,$(DLLPattern),omnithread2)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

