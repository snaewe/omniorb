include $(MAKEFILE_INC_DIR)/../cosinterfaces.mk
include $(MAKEFILE_INC_DIR)/../libdefs.mk

vpath %.cc ..
DIR_CPPFLAGS += -I$(MAKEFILE_INC_DIR)..

COS_DYNSKLIB_NAME = COSDynamic

COS_DYNSK_OBJS = $(COS_INTERFACES:%=%DynSK.o)
COS_DYNSK_SRCS = $(COS_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(COS_DYNSK_SRCS)

all:: mkstatic mkshared

export:: mkstatic mkshared

ifdef Win32Platform
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif


##############################################################################
# Build Static library
##############################################################################

ifndef NoStaticLibrary

version  := $(word 1,$(subst ., ,$(OMNIORB_VERSION)))

dynsk = static/$(patsubst %,$(LibNoDebugPattern),$(COS_DYNSKLIB_NAME)$(version))
MDFLAGS += -p static/

mkstatic::
	@(dir=static; $(CreateDir))

mkstatic:: $(dynsk)

$(dynsk): $(patsubst %, static/%, $(COS_DYNSK_OBJS))
	@$(StaticLinkLibrary)

export:: $(dynsk)
	@$(ExportLibrary)

ifdef INSTALLTARGET
install:: $(dynsk)
	@$(InstallLibrary)
endif


clean::
	$(RM) static/*.o
	$(RM) $(dynsk)
else

mkstatic::

endif


##############################################################################
# Build Shared library
##############################################################################
ifdef BuildSharedLibrary

sharedversion = $(OMNIORB_VERSION)

dynsknamespec = $(subst ., ,$(COS_DYNSKLIB_NAME).$(sharedversion))
dynskshared   = shared/$(shell $(SharedLibraryFullName) $(dynsknamespec))

MDFLAGS += -p shared/

ifdef Win32Platform
# in case of Win32 lossage:
dynimps := $(skshared) $(patsubst $(DLLDebugSearchPattern),$(DLLNoDebugSearchPattern), $(OMNIORB_LIB))
else
dynimps := -lCOS$(OMNIORB_MAJOR_VERSION) $(OMNIORB_LIB)
endif

ifdef AIX
# AIX has special library names.
oov = $(OMNIORB_MAJOR_VERSION)$(OMNIORB_MINOR_VERSION)
oovm = $(oov)$(OMNIORB_MICRO_VERSION)
imps := -lomniORB$(oov) -lomniDynamic$(oov) $(OMNITHREAD_LIB)
dynimps := -lCOS$(oovm) $(imps)
endif

mkshared::
	@(dir=shared; $(CreateDir))

mkshared:: $(dynskshared) 

$(dynskshared): $(skshared) $(patsubst %, shared/%, $(COS_DYNSK_OBJS))
	@(namespec="$(dynsknamespec)"; extralibs="$(dynimps)"; \
         $(MakeCXXSharedLibrary))

export:: $(dynskshared)
	@(namespec="$(dynsknamespec)"; \
         $(ExportSharedLibrary))

ifdef INSTALLTARGET
install:: $(dynskshared)
	@(namespec="$(dynsknamespec)"; \
         $(InstallSharedLibrary))
endif

clean::
	$(RM) shared/*.o
	(dir=shared; $(CleanSharedLibrary))

else

mkshared::

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

dynskdbug = debug/$(patsubst %,$(LibDebugPattern),$(COS_DYNSKLIB_NAME)$(dbugversion))

MDFLAGS += -p debug/

mkstaticdbug::
	@(dir=debug; $(CreateDir))

mkstaticdbug:: $(skdbug) $(dynskdbug)

$(dynskdbug): $(patsubst %, debug/%, $(COS_DYNSK_OBJS))
	@$(StaticLinkLibrary)

export:: $(dynskdbug)
	@$(ExportLibrary)

clean::
	$(RM) debug/*.o
	$(RM) $(dynskdbug)

#####################################################
#      DLL debug libraries
#####################################################
shareddbugversion = $(OMNIORB_VERSION)

dynsknamespec   = $(subst ., ,$(COS_DYNSKLIB_NAME).$(shareddbugversion))
dynskshareddbug = shareddebug/$(shell $(SharedLibraryDebugFullName) $(dynsknamespec))

dbugimps  := $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern), \
               $(OMNIORB_LIB))

MDFLAGS += -p shareddebug/

mkshareddbug::
	@(dir=shareddebug; $(CreateDir))

mkshareddbug:: $(dynskshareddbug)

$(dynskshareddbug): $(patsubst %, shareddebug/%, $(COS_DYNSK_OBJS))
	@(namespec="$(dynsknamespec)"; debug=1; extralibs="$(skshareddbug) $(dbugimps) $(extralibs)"; \
         $(MakeCXXSharedLibrary))

export:: $(dynskshareddbug)
	@(namespec="$(dynsknamespec)" debug=1; \
         $(ExportSharedLibrary))

clean::
	$(RM) shareddebug/*.o
	@(dir=shareddebug; $(CleanSharedLibrary))

endif
