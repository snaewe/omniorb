#
# x86_win32_dmc.mk - make variables and rules specific to Digital Mars C/C++.
#

WindowsNT = 1
x86Processor = 1

WINVER = 0x0400

BINDIR = bin/x86_win32
LIBDIR = lib/x86_win32

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
#PYTHON = /cygdrive/c/progra~1/Python/python
#PYTHON = c:/Python22/python


#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk



IMPORT_CPPFLAGS += -D__x86__ -D__NT__ -D__OSVERSION__=4


# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = C:\\temp


AR            = lib

CXX           = dmc
CXXMAKEDEPEND = omkdepend -D__cplusplus -D__DMC__
CXXLINK       = link
#CXXOPTIONS    = -cpp -Ae -g -D_MT -D_STLP_NO_OWN_IOSTREAMS -D_handle=_omni_handle
CXXOPTIONS    = -cpp -Ae -ND -D_STLP_NO_OWN_IOSTREAMS -D_handle=_omni_handle
CXXDEBUGFLAGS = 

CXXLINKOPTIONS = -delexecutable

DMC_DLL_CXXLINKDEBUGOPTIONS = -co
DMC_DLL_CXXLINKNODEBUGOPTIONS =

CC            = dmc
CMAKEDEPEND   = omkdepend -D__DMC__
CLINK         = link
COPTIONS      = -Jm -o+all
CDEBUGFLAGS   =

CLINKOPTIONS  = -delexecutable

OMNIIDL = $(BASE_OMNI_TREE)/$(WRAPPER_FPATH)/omniidl.exe


static/%.o: %.cc
	$(CXX) -c -o+all -WA $(CXXFLAGS) -o$@ $<

debug/%.o: %.cc
	$(CXX) -c -g -S -WA $(CXXFLAGS) -o$@ $<

shared/%DynSK.o: %DynSK.cc
	$(CXX) -c -o+all -WD -DUSE_core_stub_in_nt_dll $(CXXFLAGS) -o$@ $<

shared/%SK.o: %SK.cc
	$(CXX) -c -o+all -WD -DUSE_dyn_stub_in_nt_dll $(CXXFLAGS) -o$@ $<

shared/%.o: %.cc
	$(CXX) -c -o+all -WD $(CXXFLAGS) -o$@ $<

shareddebug/%DynSK.o: %DynSK.cc
	$(CXX) -c -g -S -WD -DUSE_core_stub_in_nt_dll $(CXXFLAGS) -o$@ $<

shareddebug/%SK.o: %SK.cc
	$(CXX) -c -g -S -WD -DUSE_dyn_stub_in_nt_dll $(CXXFLAGS) -o$@ $<

shareddebug/%.o: %.cc
	$(CXX) -c -g -S -WD $(CXXFLAGS) -o$@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<


RPATH = $(subst $(space),;,$(strip $(IMPORT_LIBRARY_DIRS)))

define CleanRule
$(RM) *.o *.lib
endef


define CXXExecutable
(set -x; \
 $(RM) $@; \
 cmd //c $(CXXLINK) $(filter-out $(LibPathPattern),$(CXXLINKOPTIONS)) \
    $(foreach file,$(filter-out $(LibPattern),$^),$(subst /,\\,$(file))), \
    $@, NUL, \
    user32 kernel32 $(patsubst $(LibPattern),%,$$libs) \
)
endef

define CExecutable
( set -x; \
 cmd //c $(CLINK) $(filter-out $(LibPathPattern),$(CLINKOPTIONS)) \
    $(filter-out $(LibPattern),$^), \
    $@, NUL, \
    user32 kernel32 $(patsubst $(LibPattern),%,$$libs) \
)
endef

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 cmd //c $(AR) -c -p256 -n $@ $^; \
)
endef

# MakeCXXExportSymbolDefinitionFile
#   Internal canned command used by MakeCXXSharedLibrary
#
#  - Create a .def file containing all the functions and static class 
#    variables exported by the DLL. The symbols are extracted from the 
#    output of libunres.
#
define MakeCXXExportSymbolDefinitionFile
symrefdir=$${debug:+debug}; \
symreflib=$(SharedLibrarySymbolRefLibraryTemplate); \
if [ ! -f $$symreflib ]; then echo "Cannot find reference static library $$symreflib"; return 1; fi;  \
set -x; \
echo "LIBRARY $$libname" > $$defname; \
echo "VERSION $$version" >> $$defname; \
echo "EXETYPE NT" >> $$defname; \
echo "SUBSYSTEM WINDOWS" >> $$defname; \
echo "CODE SHARED EXECUTE" >> $$defname; \
echo "DATA WRITE" >> $$defname; \
echo "EXPORTS" >> $$defname; \
libunres -d $$symreflib | \
egrep '^\?[^ ]*|^\?[^?][^ ]*' | fgrep -v "%" >> $$defname; \
set +x;
endef

define MakeCXXSharedLibrary
$(ParseNameSpec); \
extrasuffix=$${debug:+d}; \
targetdir=$(@D); \
libname=$(SharedLibraryLibNameTemplate); \
dllname=$$targetdir\\$$libname.dll; \
defname=$$targetdir\\$(SharedLibraryExportSymbolFileNameTemplate); \
version=$(SharedLibraryVersionStringTemplate); \
if [ -n "$$debug" ]; then \
extralinkoption="$(DMC_DLL_CXXLINKDEBUGOPTIONS)"; \
else \
extralinkoption="$(DMC_DLL_CXXLINKNODEBUGOPTIONS)"; \
fi; \
$(MakeCXXExportSymbolDefinitionFile) \
set -x; \
$(RM) $@; \
cmd //c $(CXXLINK) $(filter-out $(LibPathPattern),$(CXXLINKOPTIONS)) $$extralinkoption \
 $(subst /,\\,$^) , $$dllname ,, \
 $${extralibs/\//\\} user32 kernel32 , $$defname; \
implib $$targetdir\\$$libname $$dllname;
endef
