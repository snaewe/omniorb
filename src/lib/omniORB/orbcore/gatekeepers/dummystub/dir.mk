DIR_CPPFLAGS = -I../../.. $(patsubst %,-I%/../../..,$(VPATH)) \
               -I../../../include $(patsubst %,-I%/../../../include,$(VPATH)) \
               -I../.. $(patsubst %,-I%/../..,$(VPATH))
DIR_CPPFLAGS += $(CORBA_CPPFLAGS)

CXXSRCS = gatekeeper.cc

CXXOBJS = gatekeeper.o

LIB_NAME     := omniGK_stub
LIB_VERSION  := $(OMNIORB_MAJOR_VERSION).$(OMNIORB_MINOR_VERSION)
LIB_OBJS     := $(CXXSRCS:.cc=.o)
LIB_IMPORTS  := $(OMNIORB_LIB) $(OMNITHREAD_LIB)

BuildSharedLibrary =

include $(BASE_OMNI_TREE)/mk/mklib.mk


