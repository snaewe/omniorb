# dir.mk for omniORB2.
#
#


ifdef UnixArchitecture
NETLIBSRCS = tcpSocket_UNIX.cc
NETLIBOBJS = tcpSocket_UNIX.o
DIR_CPPFLAGS = -DUnixArchitecture
endif

ifdef ATMosArchitecture
NETLIBSRCS = tcpSocket_ATMos.cc
NETLIBOBJS = tcpSocket_ATMos.o
DIR_CPPFLAGS = -DATMosArchitecture
endif

# Required to build Naming.hh and NamingSK.cc:
CorbaImplementation = OMNIORB2
vpath %.idl $(VPATH)
CORBA_STUB_HDRS = Naming.hh

ORB2_SRCS = constants.cc corbaBoa.cc corbaObject.cc corbaOrb.cc \
            corbaString.cc \
          exception.cc giopClient.cc giopServer.cc initFile.cc ior.cc \
          libcWrapper.cc mbufferedStream.cc NamingSK.cc nbufferedStream.cc \
          object.cc objectRef.cc orb.cc strand.cc $(NETLIBSRCS)

ORB2_OBJS = constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o initFile.o ior.o \
            libcWrapper.o mbufferedStream.o NamingSK.o nbufferedStream.o \
            object.o objectRef.o orb.o strand.o $(NETLIBOBJS)

DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)

ifeq ($(CXX),g++)
CXXDEBUGFLAGS = -g
DIR_CPPFLAGS += -D__OMNIORB__ -fhandle-exceptions -Wall -Wno-unused 
else
CXXDEBUGFLAGS = -g
DIR_CPPFLAGS += -D__OMNIORB__
endif

ifeq ($(platform),arm_atmos_4.0/atb)
DIR_CPPFLAGS +=  -I/project/atmos/release4.0/atb/ip \
	-D__cplusplus -fhandle-exceptions -Wall -Wno-unused
endif

CXXSRCS = $(ORB2_SRCS)


lib = $(patsubst %,$(LibPattern),omniORB2)

all:: $(lib)

$(lib): $(ORB2_OBJS)
	@$(StaticLinkLibrary)

Naming.hh:	Naming.idl
	$(OMNIORB2_IDL) $^

NamingSK.cc:

clean::
	$(RM) $(lib) NamingSK.cc

export:: $(lib)
	@$(ExportLibrary)

export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)"; $(ExportFileToDir))
