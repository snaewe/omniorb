# dir.mk for omniORB.

ORB_SRCS = \
            anonObject.cc \
            bootstrapstub.cc \
            callDescriptor.cc \
            cdrMemoryStream.cc \
            constants.cc \
            corbaBoa.cc \
            corbaObject.cc \
            corbaOrb.cc \
            corbaString.cc \
            dynamicLib.cc \
            exception.cc \
            exceptn.cc \
            giopClient.cc \
            giopServer.cc \
            giopStream.cc \
            giopStream10.cc \
            giopStream11.cc \
            giopStream12.cc \
            initFile.cc \
            initRefs.cc \
            interceptors.cc \
            ior.cc \
            libcWrapper.cc \
	    localIdentity.cc \
            logIOstream.cc \
            Namingstub.cc \
	    objectAdapter.cc \
            objectStub.cc \
            omniInternal.cc \
            omniIOR.cc \
            omniObjRef.cc \
            omniORB.cc \
            omniServant.cc \
	    poa.cc \
            poamanager.cc \
	    poastubs.cc \
            policy.cc \
            portableserver.cc \
	    proxyFactory.cc \
            relStream.cc \
            remoteIdentity.cc \
            ropeFactory.cc \
            scavenger.cc \
            strand.cc \
	    taskqueue.cc \
            tcpSocket.cc \
            tcpSocketAux.cc \
            tcpSocketMTfactory.cc \
            tracedthread.cc \
            uri.cc

NOT_DONE =  \
            firewallProxy.cc \
            openssl.cc \
            sslMTFactory.cc \
            sslMTFactory.h

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -DUSE_omniORB_logStream
DIR_CPPFLAGS += -D_OMNIORB_LIBRARY

DIR_CPPFLAGS += -DSuppress_Spurious_gcc_Warnings

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


