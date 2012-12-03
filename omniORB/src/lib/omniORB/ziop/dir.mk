# dir.mk for omniORB ZIOP library

ORB_SRCS = omniZIOP.cc ziopStubs.cc messagingStubs.cc zlibCompressor.cc

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -D_OMNIORB_ZIOP_LIBRARY

##########################################################################
ifdef UnixPlatform
  #CXXDEBUGFLAGS = -g
  DIR_CPPFLAGS += -DUnixArchitecture
  EXTRA_LIBS = -lz
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture" $(ZLIB_CPPFLAGS)
  EXTRA_LIBS = $(patsubst %,$(LibNoDebugSearchPattern),advapi32) \
               $(ZLIB_LIB)
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
  vpath %.cc $(VPATH):$(VPATH:%=%/../orbcore)
  SHARED_ONLY_OBJS = msvcdllstub.o
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

LIB_NAME     := omniZIOP
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(OMNIORB_DLL_NAME) \
                $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk
