# dir.mk for omniORB2.
#
# Build a static library in this directory and a shared library in ./sharedlib
#

#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################
ifdef UnixPlatform

# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
#
CONFIG_DEFAULT_LOCATION = \"/project/omni/var/omniORB.cfg\"
NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o
DIR_CPPFLAGS = -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)

lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

ifndef Linux
SUBDIRS = sharedlib
endif

endif


#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o

DIR_CPPFLAGS += -D "NTArchitecture" -D "_WINSTATIC"

ifndef BuildWin32DebugLibraries

lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

CXXOPTIONS  = $(MSVC_CXXNODEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_CXXLINKNODEBUGOPTIONS)

SUBDIRS = debug sharedlib

else

# Building the debug version of the library in the debug subdirectory.
# Notice that this dir.mk is recursively used in the debug directory to build
# this library. The BuildWin32DebugLibraries make variable is set to 1 in
# the dir.mk generated in the debug directory.
#
lib = $(patsubst %,$(LibDebugPattern),omniORB2)
lclib = $(patsubst %,$(LibDebugPattern),omniLC)
CXXDEBUGFLAGS =
CXXOPTIONS = $(MSVC_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_CXXLINKDEBUGOPTIONS)

DIR_CPPFLAGS += -I./.. -I./../.. 
SUBDIRS =

vpath %.cc ..

endif


endif


#############################################################################
#   Make variables for ATMos                                                #
#############################################################################
ifdef ATMos
NETLIBSRCS = relStream.cc tcpATMos.cc tcpATMosMTfactory.cc
NETLIBOBJS = relStream.o tcpATMos.o tcpATMosMTfactory.o
DIR_CPPFLAGS = -DATMosArchitecture
CONFIG_DEFAULT_LOCATION = \"//isfs/omniORB.cfg\"
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)

lib = $(patsubst %,$(LibPattern),omniORB2)
lclib = $(patsubst %,$(LibPattern),omniLC)

endif

#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

ORB2_SRCS = constants.cc corbaBoa.cc corbaObject.cc corbaOrb.cc \
            corbaString.cc \
          exception.cc giopClient.cc giopServer.cc initFile.cc ior.cc \
          libcWrapper.cc mbufferedStream.cc nbufferedStream.cc NamingSK.cc \
          object.cc objectKey.cc objectRef.cc ropeFactory.cc \
          strand.cc scavenger.cc \
          $(NETLIBSRCS)

ORB2_OBJS = constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o initFile.o ior.o \
            libcWrapper.o mbufferedStream.o nbufferedStream.o NamingSK.o \
            object.o objectRef.o objectKey.o ropeFactory.o \
	    strand.o scavenger.o \
            $(NETLIBOBJS)

LC_SRCS = omniLifeCycle.cc reDirect.cc omniLifeCycleSK.cc
LC_OBJS = omniLifeCycle.o reDirect.o omniLifeCycleSK.o

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS) -I. -I./.. 

CXXSRCS = $(ORB2_SRCS) $(LC_SRCS)


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

all:: $(lclib)

all::
	@$(MakeSubdirs)

$(lib): $(ORB2_OBJS)
	@$(StaticLinkLibrary)

$(lclib): $(LC_OBJS)
	@$(StaticLinkLibrary)

Naming.hh NamingSK.cc:	Naming.idl
	-$(CP) $^ .
	$(OMNIORB2_IDL) $(notdir $^)

omniLifeCycle.hh omniLifeCycleSK.cc: omniLifeCycle.idl
	-$(CP) $^ .
	$(OMNIORB2_IDL) $(notdir $^)

clean::
	$(RM) $(lib) NamingSK.cc omniLifeCycleSK.cc

export:: $(lib)
	@$(ExportLibrary)

export:: $(lclib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)

export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; $(ExportFileToDir))

export:: omniLifeCycle.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; $(ExportFileToDir))




