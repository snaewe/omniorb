SUBDIRS = sharedlib

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
       update.c misc.c diag.c percent_m.c fakelog2.c

CXXSRCS = gatekeeper.cc

OBJS = hosts_access.o options.o shell_cmd.o rfc931.o eval.o \
       hosts_ctl.o refuse.o percent_x.o clean_exit.o $(AUX_OBJS) \
       fix_options.o socket.o tli.o workarounds.o \
       update.o misc.o diag.o percent_m.o fakelog2.o

CXXOBJS = gatekeeper.o

DIR_CPPFLAGS += -I. $(patsubst %,-I%,$(VPATH)) \
                -I../.. $(patsubst %,-I%/../..,$(VPATH))

lib = $(patsubst %,$(LibPattern),tcpwrapGK)

all:: $(lib)

all::
	@$(MakeSubdirs)

$(lib): $(OBJS) $(CXXOBJS)
	@$(StaticLinkLibrary)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)

export::
	@$(MakeSubdirs)


