# dir.mk for omniORB dynamic library

ORB_SRCS = \
           any.cc \
           anyP.cc \
           constants.cc \
           context.cc \
           contextList.cc \
           deferredRequest.cc \
           dynamicImplementation.cc \
           dynamicLib.cc \
	   dynAny.cc \
           dynAnyNil.cc \
           dynException.cc \
	   environment.cc \
           exceptionList.cc \
	   namedValue.cc \
           nvList.cc \
	   orbMultiRequest.cc \
           policy.cc \
	   pseudoBase.cc \
           request.cc \
           serverRequest.cc \
           tcParser.cc \
           typecode.cc \
           unknownUserExn.cc \
           $(BUILTIN_STUB_SRCS)

BUILTIN_STUB_SRCS = \
           poastub.cc \
           bootstrapdynstub.cc \
           corbaidldynstub.cc \
           corbaidlstub.cc \
           irstub.cc \
           ir.cc \
           irdynstub.cc \
           Namingdynstub.cc


DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += -I../include $(patsubst %,-I%/../include,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_DYNAMIC_LIBRARY

##########################################################################
ifdef UnixPlatform
  DIR_CPPFLAGS += -DUnixArchitecture
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D "NTArchitecture" 
  EXTRA_LIBS    = wsock32.lib advapi32.lib
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

LIB_NAME     := omniDynamic
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(OMNIORB_DLL_NAME) \
                $(OMNIASYNCINVOKER_LIB) $(OMNITHREAD_LIB) $(EXTRA_LIBS)

include $(BASE_OMNI_TREE)/mk/mklib.mk

