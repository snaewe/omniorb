# COS Stub Library
#
# Provide the following make variables to use the library
#     COS_LIB, COS_LIB_DEPEND
#   e.g.
#   foo: foo.o $(COS_LIB_DEPEND) $(CORBA_LIB_DEPEND)
#          @(libs="$(COS_LIB) $(CORBA_LIB)"; $(CXXExecutable))
#
#     COS_LIB_NODYN, COS_LIB_NODYN_DEPEND to be used when Any and Typecodes
#     of the types defined in the library are not required.
#   e.g.
#   foo: foo.o $(COS_LIB_NODYN_DEPEND) $(CORBA_LIB_DEPEND)
#          @(libs="$(COS_LIB_NODYN) $(CORBA_LIB)"; $(CXXExecutable))
#
# To use the COS idls in application IDLs
#   DIR_IDLFLAGS += $(COS_IDLFLAGS)
#
# To compile the application stubs:
#   DIR_CPPFLAGS += $(COS_CPPFLAGS)

COS_IDLFLAGS     =  $(patsubst %,-I%/idl/COS,$(IMPORT_TREES))

COS_CPPFLAGS     =  $(patsubst %,-I%/include/COS,$(IMPORT_TREES))

COS_VERSION      = $(OMNIORB_VERSION)

ifndef Win32Platform

cos_libname        = COS$(word 1,$(subst ., ,$(COS_VERSION)))
cos_dynlibname     = COSDynamic$(word 1,$(subst ., ,$(COS_VERSION)))

lib_depend        := $(patsubst %,$(LibPattern),$(cos_libname))
cos_lib_depend    := $(GENERATE_LIB_DEPEND)
lib_depend        := $(patsubst %,$(LibPattern),$(cos_dynlibname))
cos_dynlib_depend := $(GENERATE_LIB_DEPEND)


COS_LIB_NODYN        = $(patsubst %,$(LibSearchPattern),$(cos_libname))
COS_LIB_NODYN_DEPEND = $(cos_lib_depend)

COS_LIB              = $(COS_LIB_NODYN) \
                       $(patsubst %,$(LibSearchPattern),$(cos_dynlibname))
COS_LIB_DEPEND       = $(COS_LIB_NODYN_DEPEND) $(cos_dynlib_depend)

else

ifndef BuildDebugBinary

cos_dlln := $(shell $(SharedLibraryFullName) $(subst ., ,COS.$(COS_VERSION)))
cos_dyndlln := $(shell $(SharedLibraryFullName) $(subst ., ,COSDynamic.$(COS_VERSION)))

else

cos_dlln := $(shell $(SharedLibraryDebugFullName) $(subst ., ,COS.$(COS_VERSION)))
cos_dyndlln := $(shell $(SharedLibraryDebugFullName) $(subst ., ,COSDynamic.$(COS_VERSION)))

endif

lib_depend := $(cos_dlln)
cos_lib_depend := $(GENERATE_LIB_DEPEND)
lib_depend := $(cos_dyndlln)
cos_dyn_lib_depend := $(GENERATE_LIB_DEPEND)

COS_LIB_NODYN        = $(cos_dlln)
COS_LIB_NODYN_DEPEND = $(cos_lib_depend)

COS_LIB              = $(COS_LIB_NODYN) $(cos_dyndlln)
COS_LIB_DEPEND       = $(COS_LIB_NODYN_DEPEND) $(cos_dynlib_depend)

endif
