FT_INTERFACES = 

# Look for .idl files in <top>/idl/FT
vpath %.idl $(IMPORT_TREES:%=%/idl/FT)

DIR_IDLFLAGS += -Wbuse_quotes
DIR_IDLFLAGS +=  -I. $(patsubst %,-I%/idl/FT,$(IMPORT_TREES))

DIR_CPPFLAGS = $(CORBA_CPPFLAGS) -D_FT_LIBRARY

FT_SKLIB_NAME    = dummyFTClient
FT_DYNSKLIB_NAME = dummyFTClientDynamic

FT_SK_OBJS = $(FT_INTERFACES:%=%SK.o)
FT_SK_SRCS = $(FT_INTERFACES:%=%SK.cc)

FT_DYNSK_OBJS = $(FT_INTERFACES:%=%DynSK.o)
FT_DYNSK_SRCS = $(FT_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(FT_DYNSK_SRCS) $(FT_SK_SRCS) remoteGroupIdentity.cc

all:: mkstatic mkshared

export:: mkstatic mkshared

veryclean::
	$(RM) $(FT_INTERFACES:%=%SK.cc) $(FT_INTERFACES:%=%DynSK.cc) \
              $(FT_INTERFACES:%=%.hh)

##############################################################################

%.DynSK.cc %SK.cc: %.idl
	$(CORBA_IDL) $(DIR_IDLFLAGS) $^	

.PRECIOUS: %.DynSK.cc %SK.cc


##############################################################################
# Build Static library
##############################################################################

version  := $(word 1,$(subst ., ,$(OMNIORB_VERSION)))

sk = static/$(patsubst %,$(LibNoDebugPattern),$(FT_SKLIB_NAME)$(version))
dynsk = static/$(patsubst %,$(LibNoDebugPattern),$(FT_DYNSKLIB_NAME)$(version))
MDFLAGS += -p static/

mkstatic::
	@(dir=static; $(CreateDir))

mkstatic:: $(sk) $(dynsk) 

$(sk): $(patsubst %, static/%, $(FT_SK_OBJS))
	@$(StaticLinkLibrary)

$(dynsk): $(patsubst %, static/%, $(FT_DYNSK_OBJS))
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

sknamespec    = $(subst ., ,$(FT_SKLIB_NAME).$(sharedversion))
skshared      = shared/$(shell $(SharedLibraryFullName) $(sknamespec))

dynsknamespec = $(subst ., ,$(FT_DYNSKLIB_NAME).$(sharedversion))
dynskshared   = shared/$(shell $(SharedLibraryFullName) $(dynsknamespec))

MDFLAGS += -p shared/

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

$(skshared): $(patsubst %, shared/%, $(FT_SK_OBJS))
	@(namespec="$(sknamespec)" extralibs="$(imps)"; \
         $(MakeCXXSharedLibrary))

$(dynskshared): $(patsubst %, shared/%, $(FT_DYNSK_OBJS))
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

skdbug = debug/$(patsubst %,$(LibDebugPattern),$(FT_SKLIB_NAME)$(dbugversion))
dynskdbug = debug/$(patsubst %,$(LibDebugPattern),$(FT_DYNSKLIB_NAME)$(dbugversion))

MDFLAGS += -p debug/

mkstaticdbug::
	@(dir=debug; $(CreateDir))

mkstaticdbug:: $(skdbug) $(dynskdbug)

$(skdbug): $(patsubst %, debug/%, $(FT_SK_OBJS))
	@$(StaticLinkLibrary)

$(dynskdbug): $(patsubst %, debug/%, $(FT_DYNSK_OBJS))
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

sknamespec      = $(subst ., ,$(FT_SKLIB_NAME).$(shareddbugversion))
skshareddbug    = shareddebug/$(shell $(SharedLibraryDebugFullName) $(sknamespec))

dynsknamespec   = $(subst ., ,$(FT_DYNSKLIB_NAME).$(shareddbugversion))
dynskshareddbug = shareddebug/$(shell $(SharedLibraryDebugFullName) $(dynsknamespec))

dbugimps  := $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern), \
               $(OMNIORB_LIB))

MDFLAGS += -p shareddebug/

mkshareddbug::
	@(dir=shareddebug; $(CreateDir))

mkshareddbug:: $(skshareddbug) $(dynskshareddbug) 

$(skshareddbug): $(patsubst %, shareddebug/%, $(FT_SK_OBJS))
	(namespec="$(sknamespec)" debug=1 extralibs="$(dbugimps)"; \
         $(MakeCXXSharedLibrary))

$(dynskshareddbug): $(patsubst %, shareddebug/%, $(FT_DYNSK_OBJS))
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

