# dir.mk for omniORB.

GIOP_SRCS = \
            omniTransport.cc \
	    cdrStream.cc \
            cdrStreamAdapter.cc \
	    cdrMemoryStream.cc \
            giopEndpoint.cc \
            giopRope.cc \
            giopStrand.cc \
            giopStream.cc \
            giopServer.cc \
            giopWorker.cc \
            giopRendezvouser.cc \
            GIOP_C.cc \
            GIOP_S.cc \
            giopStreamImpl.cc \
            giopImpl10.cc \
            giopImpl11.cc \
            giopImpl12.cc \

TRANSPORT_SRCS = \
            tcpLib.cc 

CODESET_SRCS = \
	    codeSets.cc \
	    cs-8bit.cc \
	    cs-16bit.cc \
	    cs-8859-1.cc \
	    cs-UTF-8.cc \
	    cs-UTF-16.cc \

BUILTIN_STUB_SRCS = \
	    bootstrapstub.cc \
	    objectStub.cc \
	    poastubs.cc \
	    Namingstub.cc

ORB_SRCS =  \
	    anonObject.cc \
	    callDescriptor.cc \
	    constants.cc \
	    corbaObject.cc \
	    corbaBoa.cc \
	    corbaOrb.cc \
            corbaFixed.cc \
	    corbaString.cc \
	    corbaWString.cc \
	    current.cc \
	    dynamicLib.cc \
	    exception.cc \
	    exceptn.cc \
	    initFile.cc \
	    initRefs.cc \
	    interceptors.cc \
	    ior.cc \
	    libcWrapper.cc \
	    localIdentity.cc \
	    logIOstream.cc \
	    objectAdapter.cc \
	    omniInternal.cc \
	    omniIOR.cc \
	    omniObjRef.cc \
	    omniORB.cc \
	    omniServant.cc \
	    poa.cc \
	    poamanager.cc \
	    policy.cc \
	    portableserver.cc \
	    proxyFactory.cc \
	    remoteIdentity.cc \
	    inProcessIdentity.cc \
            callHandle.cc \
	    taskqueue.cc \
	    tracedthread.cc \
	    rmutex.cc \
	    uri.cc \
            $(GIOP_SRCS) \
            $(CODESET_SRCS) \
            $(BUILTIN_STUB_SRCS) \
            $(TRANSPORT_SRCS)

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += -I../include $(patsubst %,-I%/../include,$(VPATH))
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
#  CXXDEBUGFLAGS = -g
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
ORB_OBJS     += gatekeeper.o
vpath %.cc $(VPATH):$(VPATH:%=%/gatekeepers/dummystub)
endif

LIB_NAME     := omniORB
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(OMNIASYNCINVOKER_LIB) $(OMNITHREAD_LIB) $(EXTRA_LIBS)
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

endif

ifdef OPEN_SSL_ROOT

SUBDIRS += ssl

endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

