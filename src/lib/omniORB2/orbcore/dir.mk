# dir.mk for omniORB.

ORB_SRCS = \
	    taskqueue.cc \
            omniServant.cc \
            omniObjRef.cc \
	    localIdentity.cc \
            remoteIdentity.cc \
	    objectAdapter.cc \
            callDescriptor.cc \
	    poa.cc \
            portableserver.cc \
            poamanager.cc \
	    poastubs.cc \
	    proxyFactory.cc \
            omniInternal.cc \
            anonObject.cc \
            initRefs.cc \
            constants.cc \
            corbaObject.cc \
            corbaOrb.cc \
            corbaBoa.cc \
            corbaString.cc \
            exception.cc \
            giopClient.cc \
            giopServer.cc \
            initFile.cc \
            ior.cc \
            uri.cc \
            libcWrapper.cc \
            mbufferedStream.cc \
            nbufferedStream.cc \
            ropeFactory.cc \
            strand.cc \
            scavenger.cc \
            exceptn.cc \
            omniORB.cc \
            tracedthread.cc \
            policy.cc \
            dynamicLib.cc \
            relStream.cc \
            tcpSocket.cc \
            tcpSocketMTfactory.cc \
            logIOstream.cc \
            bootstrapstub.cc \
            Namingstub.cc \
            messagingstub.cc \
            pollablestub.cc \
            Messaging.cc \
            omniAMI.cc \
            PollableSet.cc \
	    value.cc


DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_LIBRARY

##########################################################################
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
  CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
  ifdef UnixPlatform
    CONFIG_DEFAULT_LOCATION = /project/omni/var/omniORB_NEW.cfg
  endif
  ifdef Win32Platform
    CONFIG_DEFAULT_LOCATION = C:\\OMNIORB.CFG
  endif
endif
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'

##########################################################################
ifdef UnixPlatform
  DIR_CPPFLAGS += -DUnixArchitecture
  ifdef AIX
    NoGateKeeper = 1
  endif
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D "NTArchitecture" 
  NoGateKeeper  = 1
  EXTRA_LIBS    = wsock32.lib advapi32.lib
  SHARED_ONLY_OBJS = msvcdllstub.o
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

ifdef NoGateKeeper
ORB_SRCS     += gatekeeper.cc
endif

LIB_NAME     := omniORB
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk

#########################################################################
ifdef Win32Platform

stublib = static/$(patsubst %,$(LibNoDebugPattern),msvcstub)

all:: $(stublib)

$(stublib): static/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stublib)
	@$(ExportLibrary)

clean::
	$(RM) $(stublib)

veryclean::
	$(RM) $(stublib)

stubdblib = debug/$(patsubst %,$(LibDebugPattern),msvcstub)

all:: $(stubdblib)

$(stubdblib): debug/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stubdblib)
	@$(ExportLibrary)

clean::
	$(RM) $(stubdblib)

veryclean::
	$(RM) $(stubdblib)

endif

#########################################################################
ifndef NoGateKeeper

SUBDIRS = gatekeepers

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

endif


