#
# i586_linux_2.0_glibc_gnupro.mk - 
#      make variables and rules specific to Linux 2.0 and glibc-2.0 
#      and GNUpro compiler, such as the compilers distributed with
#      Cygnus Code Fusion.
#

Linux = 1
x86Processor = 1


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# Standard programs
#

AR = ar cq

CPP = /usr/bin/cpp

EgcsMajorVersion = 1
EgcsMinorVersion = 1

CXX = /opt/cygnus/codefusion-990706/H-i686-pc-linux-gnulibc2.0/bin/g++

CXXMAKEDEPEND = $(OMKDEPEND) -D__cplusplus -D__GNUG__ -D__GNUC__
CXXDEBUGFLAGS = -O2 

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) \
                -Wl,-rpath /opt/cygnus/codefusion-990706/H-i686-pc-linux-gnulibc2.0/lib -L/opt/cygnus/codefusion-990706/H-i686-pc-linux-gnulibc2.0/lib \
		$(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

#CXXOPTIONS      = -fmemoize-lookups -fsave-memoized

CC           = /opt/cygnus/codefusion-990706/H-i686-pc-linux-gnulibc2.0/bin/gcc
CMAKEDEPEND  = $(OMKDEPEND) -D__GNUC__
CDEBUGFLAGS  = -O2

COPTIONS     = 

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

INSTALL = install -c

IMPORT_CPPFLAGS += -D__x86__ -D__linux__ -D__OSVERSION__=2


#
# CORBA stuff
#

omniORB2GatekeeperImplementation = OMNIORB2_TCPWRAPGK
CorbaImplementation = OMNIORB2

#
# OMNI thread stuff
#

ThreadSystem = Posix
OMNITHREAD_POSIX_CPPFLAGS = -DNoNanoSleep
OMNITHREAD_CPPFLAGS = -D_REENTRANT
OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)

ifndef UseMITthreads
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=10
OMNITHREAD_LIB += -lpthread
else
OMNITHREAD_POSIX_CPPFLAGS += -DPthreadDraftVersion=8 
OMNITHREAD_CPPFLAGS += -D_MIT_POSIX_THREADS
OMNITHREAD_LIB += -lpthreads
endif

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


#
# OMNI ParTcl stuff
#

TCLTK_LIB = -ltk -ltcl -lm -ldl
X11_CPPFLAGS = -I/usr/X11R6/include
X11_LIB = -L/usr/X11R6/lib -lX11
WISH4 = /usr/bin/wish

OMNIPARTCL_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(X11_CPPFLAGS)
OMNIPARTCL_LIB = $(patsubst %,$(LibSearchPattern),omniParTcl) $(TCLTK_LIB) \
		 $(X11_LIB) $(OMNITHREAD_LIB)
lib_depend := $(patsubst %,$(LibPattern),omniParTcl)
OMNIPARTCL_LIB_DEPEND := $(GENERATE_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)


#
# Java stuff
#

JAVAC = jikes
JAR = /usr/local/java/bin/jar
JAVAC_FLAGS =
CORBA_STUB_JAVAC_FLAGS = +$$

JAVA_SYSTEM_CLASSES = /usr/local/java/lib/classes.zip

CORBA_JAVA_IDL_ONLY = /usr/local/bin/vbj-idl2java

CORBA_JAVA_IDL = $(CORBA_JAVA_IDL_ONLY) -portable -strict -no_comments	 \
		 -no_examples -no_tie -idl2package ::CORBA org.omg.CORBA \
		 -idl2package ::CosNaming org.omg.CosNaming		 \
		 $(CORBA_INTERFACES_JAVA_PACKAGE:%=-package %)
