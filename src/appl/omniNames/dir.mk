
ifdef Win32Platform
DEFAULT_LOGDIR = C:\TEMP

DIR_CPPFLAGS = -I. $(CORBA_CPPFLAGS) -DDEFAULT_LOGDIR=\\\\\"$(DEFAULT_LOGDIR)\\\\\"

else
DEFAULT_LOGDIR = \"/var/omniNames\"

DIR_CPPFLAGS = $(CORBA_CPPFLAGS) -DDEFAULT_LOGDIR=$(DEFAULT_LOGDIR)

endif



CXXSRCS = omniNames.cc NamingContext_i.cc log.cc

omniNames = $(patsubst %,$(BinPattern),omniNames)

all:: $(omniNames)

clean::
	$(RM) $(omniNames)

export:: $(omniNames)
	@$(ExportExecutable)

OBJS = $(CXXSRCS:.cc=.o)

$(omniNames): $(OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
