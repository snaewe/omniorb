#
# Standard make variables and rules for all UNIX platforms.
#

UnixPlatform = 1

#
# Any recursively-expanded variable set here can be overridden _afterwards_ by
# a platform-specific mk file which includes this one.
#

#
# Standard unix programs - note that GNU make already defines some of
# these such as AR, RM, etc (see section 10.3 of the GNU make manual).
#

RANLIB		= ranlib
MKDIRHIER	= mkdirhier
INSTALL		= installbsd -c
INSTLIBFLAGS	= -m 0644
INSTEXEFLAGS	= -m 0755
CP		= cp
MV		= mv -f
CPP		= /lib/cpp
OMKDEPEND	= $(BASE_OMNI_TREE)/$(BINDIR)/omkdepend
RMDIRHIER	= rm -rf

CXXMAKEDEPEND   = $(OMKDEPEND)
CMAKEDEPEND     = $(OMKDEPEND)

#
# General rules for cleaning.
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
LibPathPattern    = -L%
LibNoDebugPattern = lib%.a
LibDebugPattern = lib%.a
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
 $(AR) $@ $^; \
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

IMPORT_LIBRARY_FLAGS = $(patsubst %,$(LibPathPattern),$(IMPORT_LIBRARY_DIRS))

define CXXExecutable
(set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
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

# omnithread - platform libraries required by omnithread.
# Use when building omnithread.
OMNITHREAD_VERSION = 3.0
OMNITHREAD_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNITHREAD_VERSION)))
OMNITHREAD_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNITHREAD_VERSION)))

OMNITHREAD_PLATFORM_LIB = $(filter-out $(patsubst %,$(LibSearchPattern),omnithread), $(OMNITHREAD_LIB))

#
# CORBA stuff
#

OMNIORB_VERSION = 4.0.0
OMNIORB_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_VERSION)))

lib_depend := $(patsubst %,$(LibPattern),omniORB$(OMNIORB_MAJOR_VERSION))
omniORB_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(patsubst %,$(LibPattern),omniDynamic$(OMNIORB_MAJOR_VERSION))
omniDynamic_lib_depend := $(GENERATE_LIB_DEPEND)

OMNIORB_DLL_NAME = $(patsubst %,$(LibSearchPattern),omniORB$(OMNIORB_MAJOR_VERSION))
OMNIORB_DYNAMIC_DLL_NAME = $(patsubst %,$(LibSearchPattern),omniDynamic$(OMNIORB_MAJOR_VERSION))


OMNIORB_IDL_ONLY = $(BASE_OMNI_TREE)/$(BINDIR)/omniidl -bcxx
OMNIORB_IDL_ANY_FLAGS = -Wba
OMNIORB_IDL = $(OMNIORB_IDL_ONLY) $(OMNIORB_IDL_ANY_FLAGS)
OMNIORB_CPPFLAGS = -D__OMNIORB$(OMNIORB_MAJOR_VERSION)__ -I$(CORBA_STUB_DIR) $(OMNITHREAD_CPPFLAGS)
OMNIORB_IDL_OUTPUTDIR_PATTERN = -C%

OMNIORB_LIB = $(OMNIORB_DLL_NAME) $(OMNIORB_DYNAMIC_DLL_NAME)
OMNIORB_LIB_NODYN = $(OMNIORB_DLL_NAME)

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

# thread libraries required by omniORB. Make sure this is the last in
# the list of omniORB related libraries

OMNIORB_LIB += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_NODYN += $(OMNITHREAD_LIB) $(SOCKET_LIB)
OMNIORB_LIB_DEPEND += $(OMNITHREAD_LIB_DEPEND)
OMNIORB_LIB_NODYN_DEPEND += $(OMNITHREAD_LIB_DEPEND)


# omniORB SSL transport
OMNIORB_SSL_VERSION = $(OMNIORB_MAJOR_VERSION).$(OMNIORB_MINOR_VERSION)
OMNIORB_SSL_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_SSL_VERSION)))
OMNIORB_SSL_LIB = $(patsubst %,$(LibSearchPattern),omnisslTP)
lib_depend := $(patsubst %,$(LibPattern),omnisslTP)
OMNIORB_SSL_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

#
# Tcl stuff
#

define TclScriptExecutable
((set -x; $(RM) $@); \
 if [ "$$wish" = "" ]; then \
   wish="$(WISH4)"; \
 fi; \
 case "$$wish" in \
 /*) ;; \
 *) \
   if [ "$(EXPORT_TREE)" != "" ]; then \
     wish="$(EXPORT_TREE)/$(BINDIR)/$$wish"; \
   else \
     wish="./$$wish"; \
   fi ;; \
 esac; \
 echo echo "#!$$wish >$@"; \
 echo "#!$$wish" >$@; \
 echo echo "set auto_path [concat {$$tcllibpath} \$$auto_path] >>$@"; \
 echo "set auto_path [concat {$$tcllibpath} \$$auto_path]" >>$@; \
 echo "cat $< >>$@"; \
 cat $< >>$@; \
 set -x; \
 chmod a+x $@; \
)
endef


##########################################################################
#
# Shared library support stuff
#
# Default setup. Work for most platforms. For those exceptions, override
# the rules in their platform files.
#
SHAREDLIB_SUFFIX = so

SharedLibraryFullNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate = lib$$1$$2.$(SHAREDLIB_SUFFIX)

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-soname,$$soname

define SharedLibraryFullName
fn() { \
if [ $$2 = "_" ] ; then set $$1 "" $$3 $$4 ; fi ; \
echo $(SharedLibraryFullNameTemplate); \
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
#
#  e.g. namespec="COS 3 0 0" --> shared library libCOS3.so.0.0
#       extralibs="$(OMNIORB_LIB)"
#
define MakeCXXSharedLibrary
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 set -x; \
 $(RM) $@; \
 $(CXX) $(SharedLibraryPlatformLinkFlagsTemplate) -o $@ \
 $(IMPORT_LIBRARY_FLAGS) $(filter-out $(LibSuffixPattern),$^) $$extralibs;
endef

# ExportSharedLibrary- export sharedlibrary
#  Expect shell variable:
#  namespec = <library name> <major ver. no.> <minor ver. no.> <micro ver. no>
#  e.g. namespec = "COS 3 0 0" --> shared library libCOS3.so.0.0
#
define ExportSharedLibrary
 $(ExportLibrary); \
 $(ParseNameSpec); \
 soname=$(SharedLibrarySoNameTemplate); \
 libname=$(SharedLibraryLibNameTemplate); \
 set -x; \
 cd $(EXPORT_TREE)/$(LIBDIR); \
 $(RM) $$soname; \
 ln -s $(<F) $$soname; \
 $(RM) $$libname; \
 ln -s $$soname $$libname;
endef

define CleanSharedLibrary
( set -x; \
$(RM) $${dir:-.}/*.$(SHAREDLIB_SUFFIX).* )
endef


# Pattern rules to build  objects files for static and shared library.
# The convention is to build the static library in the subdirectoy "static" and
# the shared library in the subdirectory "shared".
# The pattern rules below ensured that the right compiler flags are used
# to compile the source for the library.

static/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<

shared/%.o: %.cc
	$(CXX) -c $(SHAREDLIB_CPPFLAGS) $(CXXFLAGS)  -o $@ $<

static/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

SHAREDLIB_CFLAGS = $(SHAREDLIB_CPPFLAGS)

shared/%.o: %.c
	$(CC) -c $(SHAREDLIB_CFLAGS) $(CFLAGS)  -o $@ $<

#
# Replacements for implicit rules
#

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<


