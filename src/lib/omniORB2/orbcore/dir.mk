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

ORB2_SRCS = constants.cc corbaBoa.cc corbaObject.cc corbaOrb.cc \
            corbaString.cc \
          exception.cc giopClient.cc giopServer.cc ior.cc \
          libcWrapper.cc mbufferedStream.cc nbufferedStream.cc \
          object.cc objectRef.cc orb.cc strand.cc $(NETLIBSRCS)

ORB2_OBJS = constants.o corbaBoa.o corbaObject.o corbaOrb.o \
            corbaString.o \
            exception.o giopClient.o giopServer.o ior.o \
            libcWrapper.o mbufferedStream.o nbufferedStream.o \
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

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
