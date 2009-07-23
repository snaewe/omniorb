ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

OBJS = embed.o
CXXSRCS = embed.cc

ifdef UnixPlatform
CXXDEBUGFLAGS = -g
PYPREFIX  := $(shell $(PYTHON) -c 'import sys; print sys.exec_prefix')
PYVERSION := $(shell $(PYTHON) -c 'import sys; print sys.version[:3]')
PYINCDIR  := $(PYPREFIX)/include
PYINCFILE := "<python$(PYVERSION)/Python.h>"
DIR_CPPFLAGS += -I$(PYINCDIR) -DPYTHON_INCLUDE=$(PYINCFILE)
endif


ifeq ($(platform),autoconf)

namespec := _embedmodule _ 1 0

SharedLibraryFullNameTemplate = $$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   = $$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  = $$1$$2.$(SHAREDLIB_SUFFIX)

ifdef PythonLibraryPlatformLinkFlagsTemplate
SharedLibraryPlatformLinkFlagsTemplate = $(PythonLibraryPlatformLinkFlagsTemplate)
endif

shlib := $(shell $(SharedLibraryFullName) $(namespec))

DIR_CPPFLAGS += $(SHAREDLIB_CPPFLAGS)

$(shlib): $(OBJS) $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(namespec="$(namespec)"; extralibs="$(OMNIORB_LIB_NODYN) $(extralibs)";\
          $(MakeCXXSharedLibrary))

all:: $(shlib)

export:: $(shlib)
	@(namespec="$(namespec)"; $(ExportSharedLibrary))

ifdef INSTALLTARGET
install:: $(shlib)
	@(dir="$(INSTALLPYEXECDIR)"; namespec="$(namespec)"; \
          $(ExportSharedLibraryToDir))
endif

clean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

veryclean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))


else

#############################################################################
#   Make rules for Linux                                                    #
#############################################################################

ifdef Linux

CXXOPTIONS += -fpic

lib = _embedmodule.so

all:: $(lib)

$(lib): $(OBJS) $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	(set -x; \
	$(RM) $@; \
	$(CXXLINK) $(CXXLINKOPTIONS) -shared -o $@ -Wl,-soname,$(lib) $(IMPORT_LIBRARY_FLAGS) $(CORBA_LIB_NODYN) \
	 $(filter-out $(LibSuffixPattern),$^) $(LIBS)\
	)

export:: $(lib)
	@$(ExportLibrary)

clean::
	$(RM) $(lib)

endif

#############################################################################
#   Make rules for Windows                                                  #
#############################################################################

ifdef Win32Platform

PYPREFIX1 := "$(shell $(PYTHON) -c 'import sys,string; sys.stdout.write(string.lower(sys.prefix))')"
PYPREFIX  := $(subst program files,progra~1,$(subst \,/,$(PYPREFIX1)))
PYVERSION := $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.version[:3])')
PYINCDIR  := $(PYPREFIX)/include
PYLIBDIR  := $(PYPREFIX)/libs
PYLIB     := python$(subst .,,$(PYVERSION)).lib

DIR_CPPFLAGS += -I$(PYINCDIR) -I$(PYINCDIR)/python$(PYVERSION) \
                -DPYTHON_INCLUDE="<Python.h>"

PYLIBPATH = $(patsubst %,-libpath:%,$(PYLIBDIR))

implib = _embed.lib
lib = $(patsubst %.lib,%.pyd,$(implib))

all:: $(lib)

$(lib): $(OBJS) $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	(set -x; \
	 $(RM) $@; \
	 libs="$(OMNIORB_LIB_NODYN) $(PYLIB)"; \
	 $(CXXLINK) -out:$@ -DLL $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $(PYLIBPATH) $(OBJS) $(CORBA_STATIC_STUB_OBJS) $$libs; \
	)

export:: $(lib)
	@$(ExportLibrary)


endif


endif
