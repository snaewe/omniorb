#
# LIBRARY_OPTIONS, enable library features
# For meaning of these options, see the comments in Makefile.orig
LIBRARY_OPTIONS = \
               -DLIBC_CALLS_STRTOK \
               -DBROKEN_FGETS \
               -DHOSTS_DENY=\"/etc/hosts.deny\" \
               -DHOSTS_ALLOW=\"/etc/hosts.allow\" \
               -DRFC931_TIMEOUT=10

ifndef OSR5
ifndef IRIX
LIBRARY_OPTIONS += -DSOLARIS_24_GETHOSTBYNAME_BUG
endif
endif

ifndef AIX
LIBRARY_OPTIONS += -DGETPEERNAME_BUG
endif

DIR_CPPFLAGS = -DHOSTS_ACCESS $(LIBRARY_OPTIONS) -D_REENTRANT

CXXOPTIONS += $(CORBA_CPPFLAGS)

SRCS = hosts_access.c options.c shell_cmd.c rfc931.c eval.c \
       hosts_ctl.c refuse.c percent_x.c clean_exit.c $(AUX_SRCS) \
       fix_options.c socket.c workarounds.c \
       update.c misc.c diag.c percent_m.c setenv.c fakelog2.c

CXXSRCS = gatekeeper.cc

DIR_CPPFLAGS += -I. $(patsubst %,-I%,$(VPATH)) \
                -I../.. $(patsubst %,-I%/../..,$(VPATH)) \
                -I../../.. $(patsubst %,-I%/../../..,$(VPATH))

LIB_NAME     := tcpwrapGK
LIB_VERSION  := $(OMNIORB_MAJOR_VERSION).$(OMNIORB_MINOR_VERSION)
LIB_OBJS     := $(SRCS:.c=.o) $(CXXSRCS:.cc=.o)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../../shared) \
                $(patsubst %,$(LibSearchPattern),omniORB$(OMNIORB_MAJOR_VERSION)) \
                 $(OMNITHREAD_LIB)


include $(BASE_OMNI_TREE)/mk/mklib.mk
