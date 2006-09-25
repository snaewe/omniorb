# dir.mk for omniORB codesets.

ORB_SRCS = \
            cs-8859-2.cc  \
            cs-8859-3.cc  \
            cs-8859-4.cc  \
            cs-8859-5.cc  \
            cs-8859-6.cc  \
            cs-8859-7.cc  \
            cs-8859-8.cc  \
            cs-8859-9.cc  \
            cs-8859-10.cc \
            cs-UCS-4.cc   \
            cs-cp1251.cc  \
            cs-037.cc     \
            cs-500.cc     \
            cs-EDF-4.cc   \
            cs-GBK.cc     \
            omniCodeSets.cc


DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream

##########################################################################
ifdef UnixPlatform
#  CXXDEBUGFLAGS = -g
  DIR_CPPFLAGS += -DUnixArchitecture
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture" 
  vpath %.cc $(VPATH):$(VPATH:%=%/../orbcore)
  SHARED_ONLY_OBJS = msvcdllstub.o
  EXTRA_LIBS    = $(patsubst %,$(LibNoDebugSearchPattern),wsock32) $(patsubst %,$(LibNoDebugSearchPattern),advapi32)
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

LIB_NAME     := omniCodeSets
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(OMNIORB_DLL_NAME) \
                $(OMNIASYNCINVOKER_LIB) $(OMNITHREAD_LIB) \
                $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk
