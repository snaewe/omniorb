# dir.mk for omniORB connections library.

ORB_SRCS = omniConnectionMgmt.cc dataStub.cc


DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)

##########################################################################
ifdef UnixPlatform
#  CXXDEBUGFLAGS = -g
  DIR_CPPFLAGS += -DUnixArchitecture
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture"
  vpath %.cc $(VPATH):$(VPATH:%=%/../orbcore)
  SHARED_ONLY_OBJS = msvcdllstub.o
  EXTRA_LIBS    = advapi32.lib
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

LIB_NAME     := omniConnectionMgmt
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(OMNIORB_DLL_NAME) \
                $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk
