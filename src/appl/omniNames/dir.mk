DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

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
