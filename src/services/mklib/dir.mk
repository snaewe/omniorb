include cosinterfaces.mk
include libdefs.mk

# Look for .idl files in <top>/idl
vpath %.idl $(IMPORT_TREES:%=%/idl/COS)

DIR_IDLFLAGS +=  -I. $(patsubst %,-I%/idl/COS,$(IMPORT_TREES))

COS_SKLIB_NAME    = COS
COS_DYNSKLIB_NAME = COSDynamic

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)
COS_SK_SRCS = $(COS_INTERFACES:%=%SK.cc)

COS_DYNSK_OBJS = $(COS_INTERFACES:%=%DynSK.o)
COS_DYNSK_SRCS = $(COS_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(COS_DYNSK_SRCS) $(COS_SK_SRCS) 

all:: mkstatic mkshared

export:: mkstatic mkshared

export:: $(COS_INTERFACES:%=%.hh) COS_sysdep.h
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/COS"; \
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
dynsk = static/$(patsubst %,$(LibNoDebugPattern),$(COS_DYNSKLIB_NAME)$(version))

mkstatic::
	@(dir=static; $(CreateDir))

mkstatic:: $(sk) $(dynsk) 

$(sk): $(patsubst %, static/%, $(COS_SK_OBJS))
	@$(StaticLinkLibrary)

$(dynsk): $(patsubst %, static/%, $(COS_DYNSK_OBJS))
	@$(StaticLinkLibrary)

export:: $(sk)
	@$(ExportLibrary)

export:: $(dynsk)
	@$(ExportLibrary)

clean::
	$(RM) static/*.o
	$(RM) $(sk) $(dynsk)

##############################################################################
# Build Shared library
##############################################################################
ifdef BuildSharedLibrary

sharedversion = $(OMNIORB_VERSION)

sknamespec    = $(subst ., ,$(COS_SKLIB_NAME).$(sharedversion))
skshared      = shared/$(shell $(SharedLibraryFullName) $(sknamespec))

dynsknamespec = $(subst ., ,$(COS_DYNSKLIB_NAME).$(sharedversion))
dynskshared   = shared/$(shell $(SharedLibraryFullName) $(dynsknamespec))

ifdef Win32Platform
# in case of Win32 lossage:
imps := $(patsubst $(DLLDebugSearchPattern),$(DLLNoDebugSearchPattern), \
         $(OMNIORB_LIB))
else
imps := $(OMNIORB_LIB)
endif


mkshared::
	@(dir=shared; $(CreateDir))

mkshared:: $(skshared) $(dynskshared) 

$(skshared): $(patsubst %, shared/%, $(COS_SK_OBJS))
	@(namespec="$(sknamespec)" extralibs="$(imps)"; \
         $(MakeCXXSharedLibrary))

$(dynskshared): $(patsubst %, shared/%, $(COS_DYNSK_OBJS))
	@(namespec="$(dynsknamespec)" extralibs="$(skshared) $(imps)"; \
         $(MakeCXXSharedLibrary))

export:: $(skshared)
	@(namespec="$(sknamespec)"; \
         $(ExportSharedLibrary))

export:: $(dynskshared)
	@(namespec="$(dynsknamespec)"; \
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
dynskdbug = debug/$(patsubst %,$(LibDebugPattern),$(COS_DYNSKLIB_NAME)$(dbugversion))

mkstaticdbug::
	@(dir=debug; $(CreateDir))

mkstaticdbug:: $(skdbug) $(dynskdbug)

$(skdbug): $(patsubst %, debug/%, $(COS_SK_OBJS))
	@$(StaticLinkLibrary)

$(dynskdbug): $(patsubst %, debug/%, $(COS_DYNSK_OBJS))
	@$(StaticLinkLibrary)

export:: $(skdbug)
	@$(ExportLibrary)

export:: $(dynskdbug)
	@$(ExportLibrary)

clean::
	$(RM) debug/*.o
	$(RM) $(skdbug) $(dynskdbug)

#####################################################
#      DLL debug libraries
#####################################################
shareddbugversion = $(OMNIORB_VERSION)

sknamespec      = $(subst ., ,$(COS_SKLIB_NAME).$(shareddbugversion))
skshareddbug    = shareddebug/$(shell $(SharedLibraryDebugFullName) $(sknamespec))

dynsknamespec   = $(subst ., ,$(COS_DYNSKLIB_NAME).$(shareddbugversion))
dynskshareddbug = shareddebug/$(shell $(SharedLibraryDebugFullName) $(dynsknamespec))

dbugimps  := $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern), \
               $(OMNIORB_LIB))

mkshareddbug::
	@(dir=shareddebug; $(CreateDir))

mkshareddbug:: $(skshareddbug) $(dynskshareddbug) 

$(skshareddbug): $(patsubst %, shareddebug/%, $(COS_SK_OBJS))
	(namespec="$(sknamespec)" debug=1 extralibs="$(dbugimps)"; \
         $(MakeCXXSharedLibrary))

$(dynskshareddbug): $(patsubst %, shareddebug/%, $(COS_DYNSK_OBJS))
	@(namespec="$(dynsknamespec)" debug=1 extralibs="$(skshareddbug) $(dbugimps)"; \
         $(MakeCXXSharedLibrary))

export:: $(skshareddbug)
	@(namespec="$(sknamespec)" debug=1; \
         $(ExportSharedLibrary))

export:: $(dynskshareddbug)
	@(namespec="$(dynsknamespec)" debug=1; \
         $(ExportSharedLibrary))

clean::
	$(RM) shareddebug/*.o
	@(dir=shareddebug; $(CleanSharedLibrary))

endif

##############################################################################
# Build Subdirectories
##############################################################################
SUBDIRS = mkBOAlib

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

