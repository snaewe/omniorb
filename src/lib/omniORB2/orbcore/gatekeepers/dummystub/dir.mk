#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

DIR_CPPFLAGS = -I../.. $(patsubst %,-I%/../..,$(VPATH))

CXXSRCS = gatekeeper.cc

CXXOBJS = gatekeeper.o


#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

ifndef BuildWin32DebugLibraries

lib = $(patsubst %,$(LibPattern),omniGK_stub)

CXXOPTIONS  = $(MSVC_CXXNODEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_CXXLINKNODEBUGOPTIONS)

SUBDIRS += debug

else

vpath %.cc ..

# Building the debug version of the library in the debug subdirectory.
# Notice that this dir.mk is recursively used in the debug directory to build
# this library. The BuildWin32DebugLibraries make variable is set to 1 in
# the dir.mk generated in the debug directory.
#
lib = $(patsubst %,$(LibDebugPattern),omniGK_stub)
CXXDEBUGFLAGS =
CXXOPTIONS = $(MSVC_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_CXXLINKDEBUGOPTIONS)

DIR_CPPFLAGS += -I./.. -I./../.. 
SUBDIRS =

vpath %.cc ..

endif

else

#############################################################################
#   Make variables for non-Win32 platforms                                  #
#############################################################################

lib = $(patsubst %,$(LibPattern),omniGK_stub)

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
               echo 'CURRENT=src/lib/omniORB2/debug' >> debug/GNUmakefile; \
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

$(lib): $(CXXOBJS)
	@$(StaticLinkLibrary)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)


