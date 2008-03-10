#
# powerpc_LynxOS_4.0.mk - make variables and rules specific to LynxOS 4.0 on PPC. Cross-compilation on Linux machine.
# 25.08.2006 - Wojciech Gajewski
#

LynxOS = 1
PowerPCProcessor = 1


#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter.

PYTHON = /usr/bin/python
LEX = flex


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Standard programs
#

# Declarations from /ps/src/dsc/co/Make.common
ENV_PREFIX=/acc/sys/ppc4
TOOLS=$(ENV_PREFIX)/cdk/linux-elf-ppc/usr/bin/



STDFLAGS=-Ui386 -U__i386 -U__i386__ -Ulinux -U__linux -U__linux__ -U__ELF__ -I$(ENV_PREFIX)/usr/include $(CPPFLAGS)

CPPFLAGS+=-D__NO_INCLUDE_WARN__
LOADLIBES+=-L$(ENV_PREFIX)/usr/lib -L$(ENV_PREFIX)/lib/ces  -L$(ENV_PREFIX)/usr/X430/lib
XTRALIBS=-lvme -lbma -luio -lbus
STDFLAGS+=-D__Lynx__ -D__powerpc__ 

AR = ar cq

#CPP = /usr/bin/cpp
CPP=$(ENV_PREFIX)/cdk/linux-elf-ppc/usr/lib/gcc-lib/ppc-elf-lynxos/2.95.3/cpp

#CXX = g++
CXX=$(TOOLS)g++ -pipe

CXXMAKEDEPEND += -D__cplusplus -D__GNUG__ -D__GNUC__

CXXDEBUGFLAGS = 
CXXOPTIONS    =  -fpermissive -fexceptions -fcommon -Wall -Wno-unused -mthreads $(STDFLAGS)


CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) \
		$(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

#CC           = gcc
CC	= $(TOOLS)gcc -pipe
CMAKEDEPEND  += -D__GNUC__
CDEBUGFLAGS  = -O
COPTIONS     = -mthreads

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

INSTALL = /usr/bin/install -c
#INSTALL = dsc_install

IMPORT_CPPFLAGS += -D__Lynx__ -D__lynxos__ -D__powerpc__ -D__OSVERSION__=40

#
# Socket library
#

SOCKET_LIB = -lbsd -lnsl -lstdc++ -lrpc

#
# CORBA stuff
#

CorbaImplementation = OMNIORB
omniORBGatekeeperImplementation = OMNIORB_TCPWRAPGK

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DPthreadSupportThreadPriority
OMNITHREAD_CPPFLAGS = -D_REENTRANT -DUsePthread
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)
OMNITHREAD_LIB += -mthreads

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)

# Default location of the omniORB configuration file [falls back to this if
# the environment variable OMNIORB_CONFIG is not set] :

OMNIORB_CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg

# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = /var/omninames

#
# Shared library support
#

#ifeq ($(notdir $(CXX)),CC)

#BuildSharedLibrary = 0

#SHAREDLIB_CPPFLAGS = -KPIC

#SharedLibraryPlatformLinkFlagsTemplate = -G -h $$soname

#endif

#ifeq ($(notdir $(CC)),gcc)

#SHAREDLIB_CFLAGS = fPIC

#endif

#ifeq ($(notdir $(CXX)),g++ )

#BuildSharedLibrary = 0

#SHAREDLIB_CPPFLAGS = -fPIC

#SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-h,$$soname

#endif
