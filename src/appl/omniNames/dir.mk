
#CXXDEBUGFLAGS = -g

ifdef Win32Platform

ifdef OMNINAMES_LOG_DEFAULT_LOCATION
DEFAULT_LOGDIR = $(OMNINAMES_LOG_DEFAULT_LOCATION)
else
DEFAULT_LOGDIR = C:\\TEMP
endif

DIR_CPPFLAGS = -I. $(CORBA_CPPFLAGS) -DDEFAULT_LOGDIR='"$(DEFAULT_LOGDIR)"'

else

ifdef OMNINAMES_LOG_DEFAULT_LOCATION
DEFAULT_LOGDIR = $(OMNINAMES_LOG_DEFAULT_LOCATION)
else
DEFAULT_LOGDIR = /var/omniNames
endif

DIR_CPPFLAGS = -I. $(CORBA_CPPFLAGS) -DDEFAULT_LOGDIR='"$(DEFAULT_LOGDIR)"'

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
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
