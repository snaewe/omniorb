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

major_version = $(word 1,$(subst ., ,$(VERSION)))
minor_version = $(word 2,$(subst ., ,$(VERSION)))
micro_version = 0

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

sklibname = lib$(COS_SKLIB_NAME)$(major_version)$(SHAREDLIB_SUFFIX)
sksoname  = $(sklibname).$(minor_version)
sklib = $(sksoname).$(micro_version)

dynsklibname = lib$(COS_DYNSKLIB_NAME)$(major_version)$(SHAREDLIB_SUFFIX)
dynsksoname  = $(dynsklibname).$(minor_version)
dynsklib = $(dynsksoname).$(micro_version)


ifdef AIX

sklibname = lib$(COS_SKLIB_NAME)$(major_version)$(minor_version)$(SHAREDLIB_SUFFIX)
sksoname  = lib$(COS_SKLIB_NAME)$(major_version).so.$(minor_version).$(micro_version)
sklib = $(sklibname).$(micro_version)

dynsklibname = lib$(COS_DYNSKLIB_NAME)$(major_version)$(minor_version)$(SHAREDLIB_SUFFIX)
dynsksoname  = lib$(COS_DYNSKLIB_NAME)$(major_version).so.$(minor_version).$(micro_version)
dynsklib = $(dynsklibname).$(micro_version)

endif

all:: $(sklib) $(dynsklib)

$(sklib): $(COS_SK_OBJS)
	(soname=$(sksoname) libname=$(sklibname) \
         extralibs="$(OMNIORB_LIB_NODYN)"; \
         $(MakeCXXSharedLibrary))

$(dynsklib): $(COS_DYNSK_OBJS)
	(soname=$(dynsksoname) libname=$(sklibname) \
         extralibs="$(OMNIORB_LIB)"; \
         $(MakeCXXSharedLibrary))

export:: $(sklib)
	@(soname=$(sksoname) libname=$(sklibname); $(ExportSharedLibrary))

export:: $(dynsklib)
	@(soname=$(dynsksoname) libname=$(dynsklibname);$(ExportSharedLibrary))

clean::
	$(RM) $(sklib) $(dynsklib)

endif

