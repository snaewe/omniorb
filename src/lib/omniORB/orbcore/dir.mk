# dir.mk for omniORB2.
#
# Build a static library in this directory and a shared library in ./sharedlib


#Shared library only available on Unix at the moment:
ifdef UnixArchitecture
ifndef LinuxArchitecture
SUBDIRS = sharedlib
endif
endif

ifdef NTArchitecture
SUBDIRS = sharedlib
endif

ifdef UnixArchitecture
# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
CONFIG_DEFAULT_LOCATION = \"/project/omni/var/omniORB.cfg\"
NETLIBSRCS = tcpSocket_UNIX.cc
NETLIBOBJS = tcpSocket_UNIX.o
DIR_CPPFLAGS = -DUnixArchitecture
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION=$(CONFIG_DEFAULT_LOCATION)
endif

ifdef NTArchitecture
# Default location of the omniORB2 configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :
NETLIBSRCS = tcpSocket_NT.cc
NETLIBOBJS = tcpSocket_NT.o
DIR_CPPFLAGS = -D "NDEBUG" -D "_WINDOWS" 
DIR_CPPFLAGS += -D "_X86_" -D "NTArchitecture" -D "_WINSTATIC"
CXXOPTIONS += -MD -W3 -GX -O2 
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
          object.cc objectKey.cc objectRef.cc orb.cc strand.cc $(NETLIBSRCS)

ORB2_OBJS = constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o initFile.o ior.o \
            libcWrapper.o mbufferedStream.o nbufferedStream.o $(NAMINGOBJ) \
            object.o objectRef.o objectKey.o orb.o strand.o $(NETLIBOBJS)

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -I./..
DIR_CPPFLAGS += -D__OMNIORB__

CXXSRCS = $(ORB2_SRCS) $(UNSHARED_SRCS)


lib = $(patsubst %,$(LibPattern),omniORB2)

all:: $(lib)

all::
	@$(MakeSubdirs)

$(lib): $(ORB2_OBJS) $(UNSHARED_OBJS)
	@$(StaticLinkLibrary)

Naming.hh NamingSK.cc:	Naming.idl
	$(OMNIORB2_IDL) $^

ifdef NTArchitecture
clean::
	$(RM) $(lib)
else
clean::
	$(RM) $(lib) Naming.hh NamingSK.cc
endif

export:: $(lib)
	@$(ExportLibrary)

ifndef NTArchitecture
export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; $(ExportFileToDir))
endif

export::
	@$(MakeSubdirs)

