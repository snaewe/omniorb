# dir.mk for omniORB
#
# Build a shared library in this directory
# **** DO NOT forget to update the $(VERSION) number.
# **** Remember to change <top>/mk/unix.mk and <top>/mk/win32.mk too
#

# The Version number is constructed as follows:
#    <major version no.>.<minor version no.>.<micro version no.>
#
# The <major version no.> is always 3 for omniORB3.
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
VERSION = OMNIORB_VERSION

major_version = $(word 1,$(subst ., ,$(VERSION)))
minor_version = $(word 2,$(subst ., ,$(VERSION)))
micro_version = $(word 3,$(subst ., ,$(VERSION)))

#
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))

ifndef BuildWin32DebugLibraries

vpath %.cc ..

else

vpath %.cc ../..

endif

#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################
ifdef UnixPlatform
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
CONFIG_DEFAULT_LOCATION = /project/omni/var/omniORB_NEW.cfg
endif

CXXDEBUGFLAGS = -g

NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o
DIR_CPPFLAGS += -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'
endif

#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform
NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o  gatekeeper.o
# See the extra comments on gatekeeper.o at the end of this file

DIR_CPPFLAGS += -DNTArchitecture

ifndef BuildWin32DebugLibraries

# Temporary added micro version number to the name of the dll.
implib = $(patsubst %,$(DLLPattern),omniORB3$(minor_version)$(micro_version))
#implib = $(patsubst %,$(DLLPattern),omniORB3$(minor_version))
staticlib = ../$(patsubst %,$(LibPattern),omniORB3)

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
implib = $(patsubst %,$(DLLDebugPattern),omniORB3$(minor_version)$(micro_version))
#implib = $(patsubst %,$(DLLDebugPattern),omniORB3$(minor_version))
staticlib = ../../debug/$(patsubst %,$(LibDebugPattern),omniORB3)

CXXDEBUGFLAGS = 
CXXOPTIONS = $(MSVC_DLL_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_DLL_CXXLINKDEBUGOPTIONS)

tempvar := $(OMNITHREAD_LIB)
OMNITHREAD_LIB = $(patsubst $(DLLNoDebugSearchPattern),$(DLLDebugSearchPattern),$(tempvar))

DIR_CPPFLAGS += -I./../.. -I./../../.. -I./../../../..

SUBDIRS =

endif

lib = $(patsubst %.lib,%.dll,$(implib))
libname = $(patsubst %.dll,%,$(lib))

# Extra libraries required by NT
NT_EXTRA_LIBS = wsock32.lib advapi32.lib
endif

#############################################################################
#   Make variables for ATMos                                                #
#############################################################################

ifdef ATMos
NETLIBSRCS = relStream.cc tcpATMos.cc tcpATMosMTfactory.cc
NETLIBOBJS = relStream.o tcpATMos.o tcpATMosMTfactory.o
DIR_CPPFLAGS = -DATMosArchitecture
endif


#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

ORB_SRCS = \
	    taskqueue.cc omniServant.cc omniObjRef.cc \
	    localIdentity.cc remoteIdentity.cc \
	    objectAdapter.cc callDescriptor.cc \
	    poa.cc portableserver.cc poamanager.cc \
	    poastubs.cc \
	    proxyFactory.cc omniInternal.cc anonObject.cc \
            bootstrap_i.cc \
            constants.cc corbaObject.cc corbaOrb.cc corbaBoa.cc \
            corbaString.cc \
            exception.cc giopClient.cc giopServer.cc initFile.cc ior.cc \
            libcWrapper.cc mbufferedStream.cc nbufferedStream.cc \
            ropeFactory.cc \
            strand.cc scavenger.cc exceptn.cc omniORB.cc tracedthread.cc \
            policy.cc dynamicLib.cc \
            $(NETLIBSRCS) $(LOG_SRCS) bootstrapstub.cc Namingstub.cc

ORB_OBJS =  $(ORB_SRCS:.cc=.o)

LOG_SRCS = logIOstream.cc
LOG_OBJS = logIOstream.o

DIR_CPPFLAGS += $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I./.. -I./../.. -I./../../..
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_LIBRARY

CXXSRCS = $(ORB_SRCS) $(LOG_SRCS)

#############################################################################
#   Make rules for Solaris 2.x                                              #
#############################################################################

ifdef SunOS

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -KPIC

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
        $(RM) $@; \
        CC -G -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB); \
       )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif

ifeq ($(notdir $(CXX)),g++)

DIR_CPPFLAGS += -fPIC

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-h,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB); \
       )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
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

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-soname,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB); \
       )

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
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

all:: $(lib)

$(lib): $(ORB_OBJS)
	(rpath="$(RPATH)"; \
         for arg in $(OMNITHREAD_LIB) /usr/lib/cmplrs/cxx; do \
         if expr "$$arg" : "-L" >/dev/null; then \
               rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
         fi; \
         done; \set -x; \
         $(RM) $@; \
         ld -shared -soname $(soname) -set_version $(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         -expect_unresolved denyFile__10gateKeeper \
         -expect_unresolved allowFile__10gateKeeper \
         -expect_unresolved checkConnect__10gateKeeperXP15tcpSocketStrand \
         -expect_unresolved version__10gateKeeperXv \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) -lcxxstd -lcxx -lexc -lots -lc -rpath $$rpath; \
        )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif


#############################################################################
#   Make rules for  IBM AIX                                                 #
#############################################################################

ifdef AIX

libname = libomniORB$(major_version)$(minor_version).a
soname = libomniORB$(major_version).so.$(minor_version).$(micro_version)
lib = $(libname).$(micro_version)

# We have a bootstrapping problem with building gatekeepers and this shared 
# library. This library is built before gatekeeprs. Until we have 
# how to tell the linker it is safe to have some undefined symbols, we
# have to disable the gatekeeper feature.

NETLIBOBJS += gatekeeper.o

all:: $(lib)

ifeq ($(notdir $(CXX)),xlC_r)

$(lib): $(ORB_OBJS)
	(set -x; \
        $(RM) $@; \
        $(MAKECPPSHAREDLIB) \
             -o $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
         -p 40; \
         ar cq $(lib) $(soname); \
         $(RM) $(soname); \
       )

endif

ifeq ($(notdir $(CXX)),g++)

$(lib): $(ORB_OBJS)
	(set -x; \
         $(RM) $@; \
         $(CXXLINK) -shared -mthreads \
              -o $(soname) $(IMPORT_LIBRARY_FLAGS) \
          $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) ; \
         ar cq $(lib) $(soname); \
         $(RM) $(soname); \
       )

endif

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(libname); \
          ln -s $(lib) $(libname); \
         )


gatekeeper.o: gatekeepers/dummystub/gatekeeper.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<

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
               echo 'CURRENT=src/lib/omniORB2/orbcore/sharedlib/debug' >> debug/GNUmakefile; \
               echo 'include $$(TOP)/config/config.mk' >> debug/GNUmakefile; \
            fi \
          fi \
         )

export:: mkdebugdir

endif

all:: $(lib)

all::
	@$(MakeSubdirs)

$(lib): $(ORB_OBJS) msvcdllstub.o omniORB3.def
	(libs="$(NT_EXTRA_LIBS) $(OMNITHREAD_LIB)"; \
         $(RM) $@; \
         $(CXXLINK) -out:$@ -DLL $(CXXLINKOPTIONS) -def:omniORB3.def -IMPLIB:$(implib) $(IMPORT_LIBRARY_FLAGS) $(ORB_OBJS) msvcdllstub.o $$libs; \
        )

# omniORB3.def
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
omniORB3.def: $(staticlib)
	(set -x; \
         echo "LIBRARY $(libname)" > omniORB3.def; \
         echo "VERSION $(minor_version).$(micro_version)" >> omniORB3.def; \
         echo "EXPORTS" >> omniORB3.def; \
         DUMPBIN.EXE /SYMBOLS $(staticlib) | \
         egrep '^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +\(\) +External +\| +\?[^ ]*|^[^ ]+ +[^ ]+ +SECT[^ ]+ +[^ ]+ +External +\| +\?[^?][^ ]*'|\
         egrep -v 'deleting destructor[^(]+\(unsigned int\)' | \
         cut -d'|' -f2 | \
         cut -d' ' -f2 | $(SORT) -u >> omniORB3.def; )


clean::
	$(RM) *.lib *.def *.dll *.exp

# NT treats DLLs more like executables -- the .dll file needs to go in the
# bin/x86... directory so that it's on your PATH:
export:: $(lib)
	@$(ExportExecutable)

export:: $(implib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)

# Ideally, we would like to build the dummy gatekeeper stub just like other
# platforms, i.e. as a separate static library. However, it proves to be quite
# tricky because the omniORB DLL needs the symbols provided by gatekeeper.o
# to be resolved when the DLL is build. For the moment, just workaround the
# problem by building the stub directly into the library.
#

gatekeeper.o: gatekeepers/dummystub/gatekeeper.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<


# A feature or a bug in MSVC++ causes it to generate a bunch of references
# to a list of functions even though none of the these functions are actually
# used in the DLL. This file contain stub version of these functions. The
# real function definitions are in the dynamic DLL. Notice that none
# of the stubs are exported by this DLL so they would not clash with
# the ones exported by the dynamic DLL.
msvcdllstub.o: sharedlib/msvcdllstub.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<


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

# For the moment, disable the use of gatekeeper in shared library.  Other
# platforms have a bootstrapping problem with building gatekeepers and this
# shared library. This library is built before gatekeepers so the linker
# may complain about undefined symbols when it tries to link the shared
# libaries below. Further testing is needed. 
#
NETLIBOBJS +=  gatekeeper.o

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
         $(RM) $@; \
         aCC -b -Wl,+h$(soname) -o $@  $(IMPORT_LIBRARY_FLAGS) \
           $(patsubst %,-L %,$(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) \
           $(HPTHREADLIBS) ; \
        )

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

gatekeeper.o: gatekeepers/dummystub/gatekeeper.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<

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

# For the moment, disable the use of gatekeeper in shared library.  Other
# platforms have a bootstrapping problem with building gatekeepers and this
# shared library. This library is built before gatekeepers so the linker
# may complain about undefined symbols when it tries to link the shared
# libaries below. Further testing is needed. 
#
NETLIBOBJS +=  gatekeeper.o

$(lib): $(ORB_OBJS)
	(set -x; \
         $(RM) $@; \
         CC -G -z text -Kthread -KPIC -o $@ -h $(soname) \
           $(IMPORT_LIBRARY_FLAGS) $($(IMPORT_LIBRARY_DIRS)) \
           $(filter-out $(LibSuffixPattern),$^); \
        )


all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

gatekeeper.o: gatekeepers/dummystub/gatekeeper.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<

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

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
         $(RM) $@; \
         $(LINK.cc) -KPIC -shared -Wl,-h,$(libname) \
           -Wl,-set_version,$(soname) -Wl,-rpath,$(LIBDIR) \
           -o $@ $(IMPORT_LIBRARY_FLAGS) $^ $(LDLIBS); \
        )


clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif

#############################################################################
#   Make rules for FreeBSD 3.x egcs                                         #
#############################################################################

ifdef FreeBSD
ifdef EgcsMajorVersion

DIR_CPPFLAGS += -fPIC

libname = libomniORB$(major_version).so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

all:: $(lib)

$(lib): $(ORB_OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-soname,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) \
       )

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

endif
endif
