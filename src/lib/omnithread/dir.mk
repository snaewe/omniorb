ifeq ($(ThreadSystem),Solaris)
CXXSRCS = solaris.cc
OBJS = solaris.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)
endif

ifeq ($(ThreadSystem),Posix)
CXXSRCS = posix.cc
OBJS = posix.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(OMNITHREAD_POSIX_CPPFLAGS)
endif

ifeq ($(ThreadSystem),NT)
CXXSRCS = nt.cc
OBJS = nt.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) -D "_WINSTATIC"
endif


lib = $(patsubst %,$(LibPattern),omnithread)

all:: $(lib)

$(lib): $(OBJS)
	@$(StaticLinkLibrary)

export:: $(lib)
	@$(ExportLibrary)


ifndef ATMosArchitecture

SUBDIRS = sharedlib

all::
	@$(MakeSubdirs)

clean::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

endif
