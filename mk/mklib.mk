#
# Boilerplate for building and installing a library in its various forms.
#
# Requires the following variables defined on entry:
#
#   LIB_NAME		:= foo
#   LIB_VERSION		:= 3.0.2
#   LIB_OBJS		:= foomain.o bar.o baz.o
#   LIB_IMPORTS		:= $(OMNIORB_LIB)
#
# On many Unixes, this produces a static library called libfoo3.a and a
# shared library called libfoo3.so.0.2; other platforms get corresponding
# results.
#
# If LIB_VERSION only has two components, for instance "1.3", the results
# are called libfoo.a and libfoo.so.1.3 etc.
#

all:: mkstatic mkshared

export:: mkstatic mkshared

vers := $(subst ., ,$(LIB_VERSION))
ifeq ($(words $(vers)), 2)
  vers  := _ $(vers)
  major := ""
else
  major := $(word 1, $(vers))
endif

namespec := $(LIB_NAME) $(vers)

##############################################################################
# Build Static library
##############################################################################

staticlib := static/$(patsubst %,$(LibNoDebugPattern),$(LIB_NAME)$(major))

mkstatic::
	@(dir=static; $(CreateDir))

mkstatic:: $(staticlib)

$(staticlib): $(patsubst %, static/%, $(LIB_OBJS))
	@$(StaticLinkLibrary)

export:: $(staticlib)
	@$(ExportLibrary)

clean::
	$(RM) static/*.o
	$(RM) $(staticlib)

veryclean::
	$(RM) static/*.o
	$(RM) $(staticlib)


##############################################################################
# Build Shared library
##############################################################################
ifdef BuildSharedLibrary

shlib := shared/$(shell $(SharedLibraryFullName) $(namespec))

ifdef Win32Platform
# in case of Win32 lossage:
  imps := $(patsubst $(DLLDebugSearchPattern),$(DLLNoDebugSearchPattern), \
          $(LIB_IMPORTS))
else
  imps := $(LIB_IMPORTS)
endif

mkshared::
	@(dir=shared; $(CreateDir))

mkshared:: $(shlib)

$(shlib): $(patsubst %, shared/%, $(LIB_OBJS))
	@(namespec="$(namespec)" extralibs="$(imps)"; \
         $(MakeCXXSharedLibrary))

export:: $(shlib)
	@(namespec="$(namespec)"; \
          $(ExportSharedLibrary))

clean::
	$(RM) shared/*.o
	(dir=shared; $(CleanSharedLibrary))

veryclean::
	$(RM) shared/*.o
	@(dir=shared; $(CleanSharedLibrary))

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

dbuglib := debug/$(patsubst %,$(LibDebugPattern),$(LIB_NAME)$(major))

mkstaticdbug::
	@(dir=debug; $(CreateDir))

mkstaticdbug:: $(dbuglib)

$(dbuglib): $(patsubst %, debug/%, $(LIB_OBJS))
	@$(StaticLinkLibrary)

export:: $(dbuglib)
	@$(ExportLibrary)

clean::
	$(RM) debug/*.o
	$(RM) $(dbuglib)

veryclean::
	$(RM) debug/*.o
	$(RM) $(dbuglib)

#####################################################
#      DLL debug libraries
#####################################################

dbugshlib := shareddebug/$(shell $(SharedLibraryDebugFullName) $(namespec))

dbugimps  := $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern), \
               $(LIB_IMPORTS))

mkshareddbug::
	@(dir=shareddebug; $(CreateDir))

mkshareddbug:: $(dbugshlib)

$(dbugshlib): $(patsubst %, shareddebug/%, $(LIB_OBJS))
	(namespec="$(namespec)" debug=1 extralibs="$(dbugimps)"; \
         $(MakeCXXSharedLibrary))

export:: $(dbugshlib)
	@(namespec="$(namespec)" debug=1; \
          $(ExportSharedLibrary))

clean::
	$(RM) shareddebug/*.o
	@(dir=shareddebug; $(CleanSharedLibrary))

veryclean::
	$(RM) shareddebug/*.o
	@(dir=shareddebug; $(CleanSharedLibrary))

endif
