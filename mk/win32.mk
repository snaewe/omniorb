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
# Standard "unix" programs.  Anything here not provided by the GNU-WIN32
# system is likely to need a wrapper around it to perform filename translation.
#
ifndef OpenNTBuildTree

# GNU-WIN32 wrappers

AR = $(TOP)/$(WRAPPER_FPATH)/libwrapper -gnuwin32

CXX = $(TOP)/$(WRAPPER_FPATH)/clwrapper -gnuwin32
CXXMAKEDEPEND = $(TOP)/$(WRAPPER_FPATH)/omkdepend -D__cplusplus -D_MSC_VER
CXXLINK	= $(TOP)/$(WRAPPER_FPATH)/linkwrapper -gnuwin32

CC = $(TOP)/$(WRAPPER_FPATH)/clwrapper -gnuwin32
CMAKEDEPEND = $(TOP)/$(WRAPPER_FPATH)/omkdepend -D_MSC_VER
CLINK = $(TOP)/$(WRAPPER_FPATH)/linkwrapper -gnuwin32

# There is a sort in %System32%/sort.exe and in GNU-WIN32. The shell of
# GNU-WIN32 may pick either one depending on the PATH setup of the user.
# To make sure that the GNU-WIN32 version is picked up, give the pathname
# of sort.

SORT = /bin/sort

else

# OpenNT wrappers

CXX = $(TOP)/$(WRAPPER_FPATH)/clwrapper.exe -opennt
CXXLINK = $(TOP)/$(WRAPPER_FPATH)/linkwrapper.exe -opennt
AR  = $(TOP)/$(WRAPPER_FPATH)/libwrapper.exe -opennt

CXXMAKEDEPEND = $(TOP)/$(WRAPPER_FPATH)/omkdepend -D__cplusplus -D_MSC_VER
CC = $(TOP)/$(WRAPPER_FPATH)/clwrapper.exe -opennt
CMAKEDEPEND = $(TOP)/$(WRAPPER_FPATH)/omkdepend -D_MSC_VER
CLINK = $(TOP)/$(WRAPPER_FPATH)/linkwrapper.exe -opennt

SORT = sort

endif


MKDIRHIER	= mkdir -p

INSTALL		= install -c
INSTLIBFLAGS	= -m 0644
INSTEXEFLAGS	= -m 0755

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
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -Fo$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<


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

IMPORT_LIBRARY_FLAGS = $(patsubst %,-libpath:%/$(LIBDIR),$(IMPORT_TREES))

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

# Note that the DLL version is being used, so link to omniorb3_rt.lib

OMNIORB_VERSION = 3.1.0
OMNIORB_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_VERSION)))

lib_depend := $(patsubst %,$(DLLPattern),omniORB300)
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(patsubst %,$(DLLPattern),omniDynamic300)
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

ifndef OpenNTBuildTree
# GNU-WIN32 wrapper
OMNIORB_IDL_ONLY = oidlwrapper.exe -gnuwin32 -bcxx -Wbh=.hh -Wbs=SK.cc
else
# OpenNT wrapper
OMNIORB_IDL_ONLY = oidlwrapper.exe -opennt -bcxx -Wbh=.hh -Wbs=SK.cc
endif

OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB3__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)

OMNIORB_LIB = $(patsubst %,$(DLLSearchPattern),omniORB300) \
		$(patsubst %,$(DLLSearchPattern),omniDynamic300) \
		$(OMNITHREAD_LIB) wsock32.lib advapi32.lib
OMNIORB_LIB_NODYN = $(patsubst %,$(DLLSearchPattern),omniORB300) \
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

ThreadSystem = NT
OMNITHREAD_LIB = $(patsubst %,$(DLLSearchPattern),omnithread2)
lib_depend := $(patsubst %,$(DLLPattern),omnithread2)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
