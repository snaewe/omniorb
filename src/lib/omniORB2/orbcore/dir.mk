# dir.mk for omniORB2.
#
# Build a static library in this directory and a shared library in ./sharedlib


#Shared library only available on Unix at the moment:
ifdef UnixArchitecture
SUBDIRS = sharedlib
endif

ifdef UnixArchitecture
# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
CONFIG_DEFAULT_LOCATION = \"/project/omni/var/omniORB.cfg\"
NETLIBSRCS = tcpSocket_UNIX.cc
NETLIBOBJS = tcpSocket_UNIX.o
DIR_CPPFLAGS = -DUnixArchitecture 
CXXDEBUGFLAGS = -fast
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)
endif

ifdef NTArchitecture
# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
CONFIG_DEFAULT_LOCATION = \"E:\\corba2\\test\"
NETLIBSRCS = tcpSocket_NT.cc
NETLIBOBJS = tcpSocket_NT.o
DIR_CPPFLAGS = -MD -W3 -GX -O2 -D "NDEBUG" -D "WIN32" 
DIR_CPPFLAGS += -D "_WINDOWS" -D "__NT__" -D "_X86_" -D "NTArchitecture"
DIR_CPPFLAGS += -I"C:\MSDEV\INCLUDE" 
endif

ifdef ATMosArchitecture
NETLIBSRCS = tcpSocket_ATMos.cc
NETLIBOBJS = tcpSocket_ATMos.o
DIR_CPPFLAGS = -DATMosArchitecture
endif

# Required to build Naming.hh and NamingSK.cc:
ifndef NTArchitecture
CorbaImplementation = OMNIORB2
vpath %.idl $(VPATH)
CORBA_STUB_HDRS = Naming.hh
NAMINGSRC = NamingSK.cc
NAMINGOBJ = NamingSK.o
else 
NAMINGSRC = NamingSK_NT.cc
NAMINGOBJ = NamingSK_NT.o
endif

UNSHARED_SRCS = unshared.cc
UNSHARED_OBJS = unshared.o

ORB2_SRCS = constants.cc corbaBoa.cc corbaObject.cc corbaOrb.cc \
            corbaString.cc \
          exception.cc giopClient.cc giopServer.cc initFile.cc ior.cc \
          libcWrapper.cc mbufferedStream.cc nbufferedStream.cc $(NAMINGSRC) \
          object.cc objectRef.cc orb.cc strand.cc $(NETLIBSRCS)

ORB2_OBJS = constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o initFile.o ior.o \
            libcWrapper.o mbufferedStream.o nbufferedStream.o $(NAMINGOBJ) \
            object.o objectRef.o orb.o strand.o $(NETLIBOBJS)

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I./..

ifeq ($(CXX),g++)
CXXDEBUGFLAGS = -g
DIR_CPPFLAGS += -D__OMNIORB__ -fhandle-exceptions -Wall -Wno-unused 
else 
DIR_CPPFLAGS += -D__OMNIORB__
endif

ifeq ($(platform),arm_atmos_4.0/atb)
CXXDEBUGFLAGS = -g
DIR_CPPFLAGS +=  -I/project/atmos/release4.0/atb/ip \
	-D__cplusplus -fhandle-exceptions -Wall -Wno-unused
endif

CXXSRCS = $(ORB2_SRCS) $(UNSHARED_SRCS)


lib = $(patsubst %,$(LibPattern),omniORB2)

all:: $(lib)

all::
	@$(MakeSubdirs)

$(lib): $(ORB2_OBJS) $(UNSHARED_OBJS)
	@$(StaticLinkLibrary)

Naming.hh NamingSK.cc:	Naming.idl
	$(OMNIORB2_IDL) $^

clean::
	$(RM) $(lib) Naming.hh NamingSK.cc

export:: $(lib)
	@$(ExportLibrary)

ifndef NTArchitecture
export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; $(ExportFileToDir))
endif

export::
	@$(MakeSubdirs)

