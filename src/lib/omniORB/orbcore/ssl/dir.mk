SSL_SRCS = \
           sslAddress.cc \
           sslConnection.cc \
           sslEndpoint.cc \
           sslTransportImpl.cc \
           sslContext.cc

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += -I../../include $(patsubst %,-I%/../../include,$(VPATH))
DIR_CPPFLAGS += $(OMNIORB_CPPFLAGS)
DIR_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)

#########################################################################

SSL_OBJS      = $(SSL_SRCS:.cc=.o)
CXXSRCS       = $(SSL_SRCS)

ifdef Win32Platform
EXTRA_LIBS    = wsock32.lib advapi32.lib
SHARED_ONLY_OBJS = msvcdllstub.o
endif

LIB_NAME     := omnisslTP
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(SSL_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../shared) \
                $(patsubst %,$(LibSearchPattern),omniORB$(OMNIORB_MAJOR_VERSION)) \
                $(OMNIASYNCINVOKER_LIB) \
                $(OMNITHREAD_LIB) # $(OPEN_SSL_LIB)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)


include $(BASE_OMNI_TREE)/mk/mklib.mk
