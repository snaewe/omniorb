# dir.mk for omniORB2
#
# Build a shared library in this directory
# **** DO NOT forget to update the $(VERSION) number.
#

# The Version number is constructed as follows:
#    <major version no.>.<minor version no.>.<micro version no.>
#
# The <major version no.> is always 2 for omniORB2.
#
# The <minor version no.> changes when:
#   1. Public interfaces have been extended but remains backward compatible
#      with earlier minor version.
#   2. Internal interfaces have been changed.
#
# The <micro version no.> changes when the implementation has been changed
# but both the public and internal interfaces remain the same. This usually
# corresponds to a pure bug fix release.
#
# 
VERSION = 2.8.0

major_version = $(word 1,$(subst ., ,$(VERSION)))
minor_version = $(word 2,$(subst ., ,$(VERSION)))
micro_version = $(word 3,$(subst ., ,$(VERSION)))

#
# Life cycle run time library version
LC_VERSION = 1.3.0

lc_major_version = $(word 1,$(subst ., ,$(LC_VERSION)))
lc_minor_version = $(word 2,$(subst ., ,$(LC_VERSION)))
lc_micro_version = $(word 3,$(subst ., ,$(LC_VERSION)))
#
#
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))

# Further override vpath for *.cc to make sure the stub files are found.
ifndef BuildWin32DebugLibraries

vpath %.cc .. ../..

else

vpath %.cc ../.. ../../..

endif

#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################
ifdef UnixPlatform
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
CONFIG_DEFAULT_LOCATION = \"/project/omni/var/omniORB_NEW.cfg\"
endif

#CXXDEBUGFLAGS = -g

DIR_CPPFLAGS += -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)
endif

#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

DIR_CPPFLAGS += -DNTArchitecture

ifndef BuildWin32DebugLibraries

# Temporary added micro version number to the name of the dll.
implib = $(patsubst %,$(DLLPattern),omniORB2$(minor_version)$(micro_version))
#implib = $(patsubst %,$(DLLPattern),omniORB2$(minor_version))
staticlib = ../$(patsubst %,$(LibPattern),omniORB2)

dynimplib = $(patsubst %,$(DLLPattern),omniDynamic2$(minor_version)$(micro_version))
#dynimplib = $(patsubst %,$(DLLPattern),omniDynamic2$(minor_version))
dynstaticlib = ../$(patsubst %,$(LibPattern),omniDynamic2)

lcimplib = $(patsubst %,$(DLLPattern),omniLC$(lc_minor_version)$(lc_micro_version))
#lcimplib = $(patsubst %,$(DLLPattern),omniLC$(lc_minor_version))
lcstaticlib = ../$(patsubst %,$(LibPattern),omniLC)

CXXOPTIONS  = $(MSVC_DLL_CXXNODEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKNODEBUGOPTIONS)

SUBDIRS = debug

else

# Building the debug version of the library in the debug subdirectory.
# Notice that this dir.mk is recursively used in the debug directory to build
# this library. The BuildWin32DebugLibraries make variable is set to 1 in
# the dir.mk generated in the debug directory.
#

# Temporary added micro version number to the name of the dll.
implib = $(patsubst %,$(DLLDebugPattern),omniORB2$(minor_version)$(micro_version))
#implib = $(patsubst %,$(DLLDebugPattern),omniORB2$(minor_version))
staticlib = ../../debug/$(patsubst %,$(LibDebugPattern),omniORB2)

dynimplib = $(patsubst %,$(DLLDebugPattern),omniDynamic2$(minor_version)$(micro_version))
#dynimplib = $(patsubst %,$(DLLDebugPattern),omniDynamic2$(minor_version))
dynstaticlib = ../../debug/$(patsubst %,$(LibDebugPattern),omniDynamic2)

lcimplib = $(patsubst %,$(DLLDebugPattern),omniLC$(lc_minor_version)$(lc_micro_version))
#lcimplib = $(patsubst %,$(DLLDebugPattern),omniLC$(lc_minor_version))
lcstaticlib = ../../debug/$(patsubst %,$(LibDebugPattern),omniLC)

CXXDEBUGFLAGS = 
CXXOPTIONS = $(MSVC_DLL_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKDEBUGOPTIONS)

tempvar := $(OMNITHREAD_LIB)
OMNITHREAD_LIB = $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern),$(tempvar))

DIR_CPPFLAGS += -I./../.. -I./../../..

SUBDIRS =

endif

lib = $(patsubst %.lib,%.dll,$(implib))
libname = $(patsubst %.dll,%,$(lib))
dynlib = $(patsubst %.lib,%.dll,$(dynimplib))
dynlibname = $(patsubst %.dll,%,$(dynlib))
lclib = $(patsubst %.lib,%.dll,$(lcimplib))
lclibname = $(patsubst %.dll,%,$(lclib))

# Extra libraries required by NT
NT_EXTRA_LIBS = wsock32.lib advapi32.lib
endif

#############################################################################
#   Make variables for ATMos                                                #
#############################################################################

ifdef ATMos
DIR_CPPFLAGS = -DATMosArchitecture
endif


#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

LC_SRCS = omniLifeCycle.cc reDirect.cc omniLifeCycleSK.cc omniLifeCycleDynSK.cc
LC_OBJS = omniLifeCycle.o reDirect.o omniLifeCycleSK.o omniLifeCycleDynSK.o

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I./.. -I./../..
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB2_LC_LIBRARY

CXXSRCS = $(LC_SRCS)

#############################################################################
#   Make rules for Solaris 2.x                                              #
#############################################################################

ifdef SunOS

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

dynlibname = libomniDynamic$(major_version).so
dynsoname  = $(dynlibname).$(minor_version)
dynlib = $(dynsoname).$(micro_version)

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)

ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -Kpic


all:: $(lclib)

$(lclib): $(LC_OBJS)
	(set -x; \
        $(RM) $@; \
        CC -G -o $@ -h $(lcsoname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib); \
       )


clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif

ifeq ($(notdir $(CXX)),g++)

DIR_CPPFLAGS += -fPIC

all:: $(lclib)

$(lclib): $(LC_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-h,$(lcsoname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib); \
       )


clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif

endif

#############################################################################
#   Make rules for Linux egcs
#
#############################################################################

ifdef Linux
ifdef EgcsMajorVersion

DIR_CPPFLAGS += -fPIC

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

dynlibname = libomniDynamic$(major_version).so
dynsoname  = $(dynlibname).$(minor_version)
dynlib = $(dynsoname).$(micro_version)

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)

all:: $(lclib)

$(lclib): $(LC_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-soname,$(lcsoname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib); \
       )


clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif
endif

#############################################################################
#   Make rules for Digital Unix                                             #
#############################################################################

ifdef OSF1
ifeq ($(notdir $(CXX)),cxx)

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

dynlibname = libomniDynamic$(major_version).so
dynsoname  = $(dynlibname).$(minor_version)
dynlib = $(dynsoname).$(micro_version)

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)

all:: $(lclib)

$(lclib): $(LC_OBJS)
	(rpath="$(RPATH)"; \
         for arg in $(OMNITHREAD_LIB) /usr/lib/cmplrs/cxx; do \
         if expr "$$arg" : "-L" >/dev/null; then \
               rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
         fi; \
         done; \set -x; \
         $(RM) $@; \
         ld -shared -soname $(lcsoname) -set_version $(lcsoname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib) \
         -lcxxstd -lcxx -lexc -lots -lc \
         -rpath $$rpath; \
        )


clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif
endif


#############################################################################
#   Make rules for  IBM AIX                                                 #
#############################################################################

ifdef AIX

libcorename = omniORB$(major_version)$(minor_version)
libname = lib$(libcorename).a
soname = libomniORB$(major_version).so.$(minor_version).$(micro_version)
lib = $(libname).$(micro_version)

dynlibcorename = omniDynamic$(major_version)$(minor_version)
dynlibname = lib$(dynlibcorename).a
dynsoname = libomniDynamic$(major_version).so.$(minor_version).$(micro_version)
dynlib = $(dynlibname).$(micro_version)

lclibname  = libomniLC$(lc_minor_version).a
lcsoname  = libomniLC.so.$(lc_minor_version).$(lc_micro_version)
lclib = $(lclibname).$(lc_micro_version)

# We have a bootstrapping problem with building gatekeepers and this shared 
# library. This library is built before gatekeeprs. Until we have 
# how to tell the linker it is safe to have some undefined symbols, we
# have to disable the gatekeeper feature.

all:: $(lclib)

ifeq ($(notdir $(CXX)),xlC_r)

$(lclib): $(LC_OBJS)
	(set -x; \
        $(RM) $@; \
        $(MAKECPPSHAREDLIB) \
             -o $(lcsoname) $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         -L../../orbcore/sharedlib -l$(libcorename) \
         -L../../dynamic/sharedlib -l$(dynlibcorename) \
         -p 40; \
         ar cq $(lclib) $(lcsoname); \
         $(RM) $(lcsoname); \
       )

endif

ifeq ($(notdir $(CXX)),g++)

$(lclib): $(LC_OBJS)
	(set -x; \
         $(RM) $@; \
         $(CXXLINK) -shared -mthreads \
              -o $(lcsoname) $(IMPORT_LIBRARY_FLAGS) \
          $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
          ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib); \
         ar cq $(lclib) $(lcsoname); \
         $(RM) $(lcsoname); \
       )

endif

clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lclibname); \
          ln -s $(lclib) $(lclibname); \
         )


endif

#############################################################################
#   Make rules for Win32 platforms                                          #
#############################################################################

ifdef Win32Platform

ifndef BuildWin32DebugLibraries
# Prepare a debug directory for building the debug version of the library.
# Essentially, we create a debug directory in the current directory, create
# a dir.mk and optionally a GNUmakefile in that directory and then calling
# omake (GNU make) in that directory.
# The confusing bit is that this dir.mk is recursively used in the debug 
# directory to build this library. The BuildWin32DebugLibraries make variable,
# which is set to 1 in the dir.mk generated in the debug directory,
# is used to identify this case.
#
all:: mkdebugdir

mkdebugdir:
	@(if [ ! -f debug/dir.mk ]; then \
            file=dir.mk; dirs='. $(VPATH:/..=)'; $(FindFileInDirs); \
            case "$$fullfile" in /*) ;; *) fullfile=../$$fullfile;; esac; \
            dir=debug; $(CreateDir); \
            echo 'BuildWin32DebugLibraries = 1' > debug/dir.mk; \
            echo 'override VPATH := $$(VPATH:/debug=)' >> debug/dir.mk; \
            echo include $$fullfile >> debug/dir.mk; \
            if [ -f GNUmakefile ]; then \
               echo 'TOP=../../../../../..' > debug/GNUmakefile; \
               echo 'CURRENT=src/lib/omniORB2/lifecycle/sharedlib/debug' >> debug/GNUmakefile; \
               echo 'include $$(TOP)/config/config.mk' >> debug/GNUmakefile; \
            fi \
          fi \
         )

export:: mkdebugdir

endif

all:: $(lclib)

all::
	@$(MakeSubdirs)

$(lclib): $(LC_OBJS) omniLC.def
	(libs="$(NT_EXTRA_LIBS) $(OMNITHREAD_LIB) $(implib) $(dynimplib)"; \
         $(RM) $@; \
         $(CXXLINK) -out:$@ -DLL $(CXXLINKOPTIONS) -def:omniLC.def \
           -IMPLIB:$(lcimplib) $(IMPORT_LIBRARY_FLAGS) $(LC_OBJS) $$libs; \
        )

# omniORB2.def
#  - This file contains all the functions and static class variables
#    exported by the DLL. The symbols are extracted from the output of
#    dumpbin.
#
#    The function symbols are extracted using the following template:
#    ... ........ SECT..  notype ()     External      | ?..................
#
#    The static class variable symbols are extracted using the following
#    template:
#    ... ........ SECT..  notype        External      | ?[^?]..............
#
#    Default destructors generated by the compiler are excluded.
#
#    It looks like class variable and function symbols start with two ??
#    and class static variable and static function symbols start with one ?.
#                                                             - SLL
#
omniLC.def: $(lcstaticlib)
	(set -x; \
         echo "LIBRARY $(lclibname)" > omniLC.def; \
         echo "VERSION $(minor_version).$(micro_version)" >> omniLC.def; \
         echo "EXPORTS" >> omniLC.def; \
         DUMPBIN.EXE /SYMBOLS $(lcstaticlib) | \
         egrep '^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +\(\) +External +\| +\?[^ ]*|^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +External +\| +\?[^?][^ ]*'|\
         egrep -v 'deleting destructor[^(]+\(unsigned int\)' | \
         cut -d'|' -f2 | \
         cut -d' ' -f2 | $(SORT) -u >> omniLC.def; )


clean::
	$(RM) *.lib *.def *.dll *.exp

# NT treats DLLs more like executables -- the .dll file needs to go in the
# bin/x86... directory so that it's on your PATH:

export:: $(lclib)
	@$(ExportExecutable)

export:: $(lcimplib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)


endif



#############################################################################
#   Make rules for HPUX                                                     #
#############################################################################

ifdef HPUX
ifeq ($(notdir $(CXX)),aCC)

DIR_CPPFLAGS += +Z

libname = libomniORB$(major_version).sl
soname  = $(libname).$(minor_version)
lib     = $(soname).$(micro_version)

dynlibname = libomniDynamic$(major_version).sl
dynsoname  = $(dynlibname).$(minor_version)
dynlib     = $(dynsoname).$(micro_version)

lclibname = libomniLC.sl
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)


all:: $(lclib)

$(lclib): $(LC_OBJS)
	(set -x; \
         $(RM) $@; \
         aCC -b -Wl,+h$(lcsoname) -o $@  $(IMPORT_LIBRARY_FLAGS) \
           $(patsubst %,-L %,$(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
           ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib) \
           $(HPTHREADLIBS); \
        )

clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )


endif
endif

#############################################################################
#   Make rules for to Reliant Unix                                          #
#############################################################################

ifdef SINIX
ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -Kpic

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib     = $(soname).$(micro_version)

dynlibname = libomniDynamic$(major_version).so
dynsoname  = $(dynlibname).$(minor_version)
dynlib     = $(dynsoname).$(micro_version)

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib     = $(lcsoname).$(lc_micro_version)


$(lclib): $(LC_OBJS)
	(set -x; \
         $(RM) $@; \
         CC -G -z text -Kthread -KPIC -o $@ -h $(lcsoname) \
           $(IMPORT_LIBRARY_FLAGS) $($(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^) \
           ../../orbcore/sharedlib/$(lib) ../../dynamic/sharedlib/$(dynlib); \
        )


all:: $(lclib)

clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )


endif
endif

#############################################################################
#   Make rules for SGI Irix 6.2                                             #
#############################################################################

ifdef IRIX
ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -KPIC

ifdef IRIX_n32
ADD_CPPFLAGS = -n32
endif
ifdef IRIX_64
ADD_CPPFLAGS = -64
endif

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)

all:: $(lclib)

$(lclib): $(LC_OBJS)
	($(RM) $@; \
         $(LINK.cc) -KPIC -shared -Wl,-h,$(lclibname) \
           -Wl,-set_version,$(lcsoname) -Wl,-rpath,$(LIBDIR) \
           -o $@ $(IMPORT_LIBRARY_FLAGS) $^ $(LDLIBS); \
        )


clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif
endif


#############################################################################
#   Make rules for FreeBSD 3.x egcs                                         #
#############################################################################

ifdef FreeBSD
ifdef EgcsMajorVersion

DIR_CPPFLAGS += -fPIC

lclibname = libomniLC.so
lcsoname  = $(lclibname).$(lc_minor_version)
lclib = $(lcsoname).$(lc_micro_version)

all:: $(lclib)

$(lclib): $(LC_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-soname,$(lcsoname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^); \
       )

clean::
	$(RM) $(lclib)

export:: $(lclib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(lcsoname); \
          ln -s $(lclib) $(lcsoname); \
          $(RM) $(lclibname); \
          ln -s $(lcsoname) $(lclibname); \
         )

endif
endif
