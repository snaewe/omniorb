include ../cosinterfaces.mk
include ../libdefs.mk

# Generate BOA skeleton
DIR_IDLFLAGS += -WbBOA

# Look for .idl files in <top>/idl plus ../../idl
vpath %.idl ../../idl $(IMPORT_TREES:%=%/idl) $(VPATH:%=%/../../idl)

DIR_IDLFLAGS += -I. -I../../idl $(patsubst %,-I%/../../idl,$(VPATH)) \
                   $(patsubst %,-I%/idl,$(IMPORT_TREES))


COS_SKLIB_NAME    = COS_BOA

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)
COS_SK_SRCS = $(COS_INTERFACES:%=%SK.cc)
CXXSRCS = $(COS_SK_SRCS) 

all:: mkstatic mkshared

export:: mkstatic mkshared

export:: $(COS_INTERFACES:%=%.hh) COS_sysdep.h
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/COS/BOA"; \
		$(ExportFileToDir) \
          done )

veryclean::
	$(RM) $(COS_INTERFACES:%=%SK.cc) $(COS_INTERFACES:%=%DynSK.cc) \
              $(COS_INTERFACES:%=%.hh)

##############################################################################
# Build Static library
##############################################################################

version  := $(word 1,$(subst ., ,$(OMNIORB_VERSION)))

sk = static/$(patsubst %,$(LibNoDebugPattern),$(COS_SKLIB_NAME)$(version))

mkstatic::
	@(dir=static; $(CreateDir))

mkstatic:: $(sk)

$(sk): $(patsubst %, static/%, $(COS_SK_OBJS))
	@$(StaticLinkLibrary)

export:: $(sk)
	@$(ExportLibrary)

clean::
	$(RM) static/*.o
	$(RM) $(sk)

##############################################################################
# Build Shared library
##############################################################################
ifdef BuildSharedLibrary

sharedversion = $(OMNIORB_VERSION)

sknamespec    = $(subst ., ,$(COS_SKLIB_NAME).$(sharedversion))
skshared      = shared/$(shell $(SharedLibraryFullName) $(sknamespec))

ifdef Win32Platform
# in case of Win32 lossage:
imps := $(patsubst $(DLLDebugSearchPattern),$(DLLNoDebugSearchPattern), \
         $(OMNIORB_LIB))
else
imps := $(OMNIORB_LIB)
endif

mkshared::
	@(dir=shared; $(CreateDir))

mkshared:: $(skshared)

$(skshared): $(patsubst %, shared/%, $(COS_SK_OBJS))
	@(namespec="$(sknamespec)" extralibs="$(imps)"; \
         $(MakeCXXSharedLibrary))

export:: $(skshared)
	@(namespec="$(sknamespec)"; \
         $(ExportSharedLibrary))

clean::
	$(RM) shared/*.o
	(dir=shared; $(CleanSharedLibrary))

endif

##############################################################################
# Build debug libraries for Win32
##############################################################################
ifdef Win32Platform

all:: mkstaticdbug mkshareddbug

export:: mkstaticdbug mkshareddbug

#####################################################
#      Static debug libraries
#####################################################
dbugversion = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))

skdbug = debug/$(patsubst %,$(LibDebugPattern),$(COS_SKLIB_NAME)$(dbugversion))

mkstaticdbug::
	@(dir=debug; $(CreateDir))

mkstaticdbug:: $(skdbug)

$(skdbug): $(patsubst %, debug/%, $(COS_SK_OBJS))
	@$(StaticLinkLibrary)

export:: $(skdbug)
	@$(ExportLibrary)

clean::
	$(RM) debug/*.o
	$(RM) $(skdbug)

#####################################################
#      DLL debug libraries
#####################################################
shareddbugversion = $(OMNIORB_VERSION)

sknamespec      = $(subst ., ,$(COS_SKLIB_NAME).$(shareddbugversion))
skshareddbug    = shareddebug/$(shell $(SharedLibraryDebugFullName) $(sknamespec))

dbugimps  := $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern), \
               $(OMNIORB_LIB))

mkshareddbug::
	@(dir=shareddebug; $(CreateDir))

mkshareddbug:: $(skshareddbug)

$(skshareddbug): $(patsubst %, shareddebug/%, $(COS_SK_OBJS))
	(namespec="$(sknamespec)" debug=1 extralibs="$(dbugimps)"; \
         $(MakeCXXSharedLibrary))

export:: $(skshareddbug)
	@(namespec="$(sknamespec)" debug=1; \
         $(ExportSharedLibrary))

clean::
	$(RM) shareddebug/*.o
	@(dir=shareddebug; $(CleanSharedLibrary))

endif


