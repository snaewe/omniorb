# dir.mk for omniORB2.
#
# Build a static library in this directory and a shared library in ./sharedlib
#

#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

LC_SRCS = omniLifeCycle.cc reDirect.cc omniLifeCycleSK.cc omniLifeCycleDynSK.cc
LC_OBJS = omniLifeCycle.o reDirect.o omniLifeCycleSK.o omniLifeCycleDynSK.o

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I. -I./..
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB2_LC_LIBRARY

CXXSRCS = $(LC_SRCS)

# Further override vpath for *.cc to make sure that stub files are found.
ifndef BuildWin32DebugLibraries

vpath %.cc ..

else

vpath %.cc ../..

endif


#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################
ifdef UnixPlatform

# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
#
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
CONFIG_DEFAULT_LOCATION = \"/project/omni/var/omniORB_NEW.cfg\"
endif

DIR_CPPFLAGS += -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)

lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

SUBDIRS = sharedlib

endif


#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

DIR_CPPFLAGS += -D "NTArchitecture" -D "_WINSTATIC"

ifndef BuildWin32DebugLibraries

lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

CXXOPTIONS  = $(MSVC_STATICLIB_CXXNODEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_STATICLIB_CXXLINKNODEBUGOPTIONS)

SUBDIRS += debug

ifndef ETSKernel
SUBDIRS +=sharedlib
endif

else

# Building the debug version of the library in the debug subdirectory.
# Notice that this dir.mk is recursively used in the debug directory to build
# this library. The BuildWin32DebugLibraries make variable is set to 1 in
# the dir.mk generated in the debug directory.
#
lib = $(patsubst %,$(LibDebugPattern),omniORB2)
lclib = $(patsubst %,$(LibDebugPattern),omniLC)
CXXDEBUGFLAGS =
CXXOPTIONS = $(MSVC_STATICLIB_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_STATICLIB_CXXLINKDEBUGOPTIONS)

DIR_CPPFLAGS += -I./.. -I./../.. 
SUBDIRS =

endif

ifdef ETSKernel
# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
#
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
CONFIG_DEFAULT_LOCATION = C:\\OMNIORB.CFG
endif

DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'

endif

endif

#############################################################################
#   Make variables for ATMos                                                #
#############################################################################
ifdef ATMos
DIR_CPPFLAGS = -DATMosArchitecture
CONFIG_DEFAULT_LOCATION = \"//isfs/omniORB.cfg\"
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)
SUBDIRS =
lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

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
               echo 'TOP=../../../../..' > debug/GNUmakefile; \
               echo 'CURRENT=src/lib/omniORB2/lifecycle/debug' >> debug/GNUmakefile; \
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


all:: $(lclib)

all::
	@$(MakeSubdirs)

$(lclib): $(LC_OBJS)
	@$(StaticLinkLibrary)

ifndef OMNIORB2_IDL_FPATH
OMNIORB2_IDL_FPATH = $(OMNIORB2_IDL)
endif

export:: $(lclib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)

