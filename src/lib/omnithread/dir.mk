ifeq ($(ThreadSystem),Solaris)
CXXSRCS = solaris.cc
OBJS = solaris.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)
endif

ifeq ($(ThreadSystem),Posix)
CXXSRCS = posix.cc
OBJS = posix.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(OMNITHREAD_POSIX_CPPFLAGS)
endif

ifeq ($(ThreadSystem),NT)
CXXSRCS = nt.cc
OBJS = nt.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) -D "_WINSTATIC"
endif

ifeq ($(ThreadSystem),Mach)
CXXSRCS = mach.cc
OBJS = mach.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)
endif


#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

lib     = $(patsubst %,$(LibPattern),omnithread)
SUBDIRS = sharedlib

#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

ifndef BuildWin32DebugLibraries

lib = $(patsubst %,$(LibPattern),omnithread)
CXXOPTIONS     = $(MSVC_STATICLIB_CXXNODEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS)

SUBDIRS = debug

ifndef EmbeddedSystem
SUBDIRS +=sharedlib
endif

else

# Building the debug version of the library in the debug subdirectory.
# Notice that this dir.mk is recursively used in the debug directory to build
# this library. The BuildWin32DebugLibraries make variable is set to 1 in
# the dir.mk generated in the debug directory.
#
lib = $(patsubst %,$(LibDebugPattern),omnithread)
CXXDEBUGFLAGS = 
CXXOPTIONS = $(MSVC_STATICLIB_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_STATICLIB_CXXLINKDEBUGOPTIONS)

SUBDIRS =

vpath %.cc ..

endif


endif

#############################################################################
#   Make rules for to Win32 platforms                                       #
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
            file=dir.mk; $(FindFileInVpath); \
            case "$$fullfile" in /*) ;; *) fullfile=../$$fullfile;; esac; \
            dir=debug; $(CreateDir); \
            echo 'BuildWin32DebugLibraries = 1' > debug/dir.mk; \
            echo 'override VPATH := $$(VPATH:/debug=)' >> debug/dir.mk; \
            echo include $$fullfile >> debug/dir.mk; \
            if [ -f GNUmakefile ]; then \
               echo 'TOP=../../../..' > debug/GNUmakefile; \
               echo 'CURRENT=src/lib/omnithread/debug' >> debug/GNUmakefile; \
               echo 'include $$(TOP)/config/config.mk' >> debug/GNUmakefile; \
            fi \
          fi \
         )

export:: mkdebugdir

endif

endif


#############################################################################
#   Make rules common to all platforms                                      #
#############################################################################

all:: $(lib)

$(lib): $(OBJS)
	@$(StaticLinkLibrary)

export:: $(lib)
	@$(ExportLibrary)

clean::
	$(RM) $(lib)

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
