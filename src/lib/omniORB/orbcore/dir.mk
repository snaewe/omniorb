# dir.mk for omniORB2.
#
# Build a static library in this directory and a shared library in ./sharedlib
#

#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

ORB2_SRCS = bootstrap_i.cc \
            constants.cc corbaBoa.cc corbaObject.cc corbaOrb.cc \
            corbaString.cc \
            exception.cc giopClient.cc giopServer.cc initFile.cc ior.cc \
            libcWrapper.cc mbufferedStream.cc nbufferedStream.cc \
            object.cc objectKey.cc objectRef.cc ropeFactory.cc \
            strand.cc scavenger.cc exceptn.cc proxyCall.cc \
            current.cc policy.cc orbservice.cc domainManager.cc \
            $(NETLIBSRCS) $(LOG_SRCS) bootstrapstub.cc Namingstub.cc

ORB2_OBJS = bootstrap_i.o \
            constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o initFile.o ior.o \
            libcWrapper.o mbufferedStream.o nbufferedStream.o \
            object.o objectRef.o objectKey.o ropeFactory.o \
            strand.o scavenger.o exceptn.o proxyCall.o \
            current.o policy.o orbservice.o domainManager.o \
            $(NETLIBOBJS) $(LOG_OBJS) bootstrapstub.o Namingstub.o

LOG_SRCS = logIOstream.cc
LOG_OBJS = logIOstream.o

DIR_CPPFLAGS += $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I. -I./.. -I./../..
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB2_LIBRARY

CXXSRCS = $(ORB2_SRCS)


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
CONFIG_DEFAULT_LOCATION = /project/omni/var/omniORB_NEW.cfg
endif

NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o
DIR_CPPFLAGS += -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'

lib = $(patsubst %,$(LibPattern),omniORB2)

SUBDIRS = sharedlib gatekeepers

endif


#############################################################################
#   Make variables for Win32 platforms                                      #
#############################################################################

ifdef Win32Platform

NETLIBSRCS = relStream.cc tcpSocket.cc tcpSocketMTfactory.cc
NETLIBOBJS = relStream.o tcpSocket.o tcpSocketMTfactory.o gatekeeper.o
# See extra comments on gatekeeper.o at the end of this file

DIR_CPPFLAGS += -D "NTArchitecture" -D "_WINSTATIC"

ifndef BuildWin32DebugLibraries

lib = $(patsubst %,$(LibPattern),omniORB2)

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
CXXDEBUGFLAGS =
CXXOPTIONS = $(MSVC_STATICLIB_CXXDEBUGFLAGS)
CXXLINKOPTIONS = $(MSVC_STATICLIB_CXXLINKDEBUGOPTIONS)

DIR_CPPFLAGS += -I./.. -I./../.. -I./../../..
SUBDIRS =

vpath %.cc ..

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
NETLIBSRCS = relStream.cc tcpATMos.cc tcpATMosMTfactory.cc
NETLIBOBJS = relStream.o tcpATMos.o tcpATMosMTfactory.o
DIR_CPPFLAGS = -DATMosArchitecture
CONFIG_DEFAULT_LOCATION = \"//isfs/omniORB.cfg\"
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)
SUBDIRS = gatekeepers
lib = $(patsubst %,$(LibPattern),omniORB2)

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
               echo 'CURRENT=src/lib/omniORB2/orbcore/debug' >> debug/GNUmakefile; \
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

all::
	@$(MakeSubdirs)

$(lib): $(ORB2_OBJS)
	@$(StaticLinkLibrary)

ifndef OMNIORB2_IDL_FPATH
OMNIORB2_IDL_FPATH = $(OMNIORB2_IDL)
endif

ifndef BuildWin32DebugLibraries

#bootstrapSK.cc: ../bootstrapSK.cc
#	$(CP) $< $@
#
#NamingSK.cc: ../NamingSK.cc
#	$(CP) $< $@

endif

clean::
	$(RM) $(lib) $(dynlib) bootstrapSK.cc NamingSK.cc

export:: $(lib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)

ifdef Win32Platform

# Ideally, we would like to build the dummy gatekeeper stub just like other
# platforms, i.e. as a separate static library. However, it proves to be quite
# tricky because the omniORB2 DLL needs the symbols provided by gatekeeper.o
# to be resolved when the DLL is build. For the moment, just workaround the
# problem by building the stub directly into the library.
#
gatekeeper.o: gatekeepers/dummystub/gatekeeper.cc
	$(CXX) -c $(CXXFLAGS) -Fo$@ $<

endif
