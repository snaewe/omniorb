include ../cosinterfaces.mk
include ../libdefs.mk

#
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))

ifndef BuildWin32DebugLibraries

vpath %.cc ..
DIR_CPPFLAGS += -I..

else

vpath %.cc ../..
DIR_CPPFLAGS += -I../..

endif

# Build a shared library in this directory
# **** DO NOT forget to update the version number.
#

# The Version number is constructed as follows:
#    <major version no.>.<minor version no.>.<micro version no.>
#
# The <major version no.> and <minor version no.> always match the
# corresponding numbers of the omniORB runtime libraries.
#
#
# The <micro version no.> changes when the implementation has been changed
# but both the public and internal interfaces remain the same. This usually
# corresponds to a pure bug fix release.
#
# 
VERSION = $(OMNIORB_VERSION)

COS_SKLIB_NAME    = COS
COS_DYNSKLIB_NAME = COSDynamic

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)
COS_SK_SRCS = $(COS_INTERFACES:%=%SK.cc)

COS_DYNSK_OBJS = $(COS_INTERFACES:%=%DynSK.o)
COS_DYNSK_SRCS = $(COS_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(COS_DYNSK_SRCS) $(COS_SK_SRCS) 

###########################################################################
#     Rules to build shared libraries for Elf-based platforms             #
###########################################################################
ifdef ELF_SHARED_LIBRARY

DIR_CPPFLAGS += $(SHAREDLIB_CPPFLAGS)

SharedLibraryNameSpec := $(COS_SKLIB_NAME).$(VERSION)
skfullname := $(SharedLibraryFullName)
sksoname    := $(SharedLibrarySoName)
skname     := $(SharedLibraryName)

SharedLibraryNameSpec := $(COS_DYNSKLIB_NAME).$(VERSION)
dynskfullname := $(SharedLibraryFullName)
dynsksoname    := $(SharedLibrarySoName)
dynskname     := $(SharedLibraryName)

all:: $(skfullname) $(dynskfullname)

$(skfullname): $(COS_SK_OBJS)
	(soname=$(sksoname) libname=$(skname) \
         extralibs="$(OMNIORB_LIB_NODYN)"; \
         $(MakeCXXSharedLibrary))

$(dynskfullname): $(COS_DYNSK_OBJS)
	(soname=$(dynsksoname) libname=$(dynskname) \
         extralibs="$(OMNIORB_LIB)"; \
         $(MakeCXXSharedLibrary))

export:: $(skfullname)
	@(soname=$(sksoname) libname=$(skname); $(ExportSharedLibrary))

export:: $(dynskfullname)
	@(soname=$(dynsksoname) libname=$(dynskname);$(ExportSharedLibrary))

clean::
	$(RM) $(sklib) $(dynsklib)

endif

