# dir.mk for omniORB.

DYN_SRCS = \
           any.cc \
           typecode.cc \
           anyP.cc \
           tcParser.cc \
	   dynAny.cc \
           dynAnyNil.cc \
	   namedValue.cc \
           nvList.cc \
           exceptionList.cc \
           contextList.cc \
	   environment.cc \
           context.cc \
           deferredRequest.cc \
           unknownUserExn.cc \
	   request.cc \
           serverRequest.cc \
           dynamicImplementation.cc \
	   pseudoBase.cc \
           dynException.cc \
           ir.cc \
	   irstub.cc \
           irdynstub.cc \
           corbaidlstub.cc \
           corbaidldynstub.cc \
           bootstrapdynstub.cc \
           Namingdynstub.cc \
	   orbMultiRequest.cc \
           constants.cc \
           dynamicLib.cc \
	   messagingdynstub.cc

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_DYNAMIC_LIBRARY

##########################################################################
ifdef UnixPlatform
  DIR_CPPFLAGS += -DUnixArchitecture
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D "NTArchitecture" 

  ORB_SRCS += gatekeeper.cc
  # See extra comments on gatekeeper.o at the end of this file
endif

#########################################################################

CXXSRCS       = $(DYN_SRCS)

LIB_NAME     := omniDynamic
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(DYN_SRCS:.cc=.o)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(patsubst %,$(LibPathPattern),../orbcore/shareddebug) \
                $(OMNIORB_LIB_NODYN)

include $(BASE_OMNI_TREE)/mk/mklib.mk

CXXSRCS = $(DYN_SRCS)

