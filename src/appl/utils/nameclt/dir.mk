CORBA_INTERFACES = Naming

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CXXDEBUGFLAGS = -g

CXXSRCS = nameclt.cc

nameclt = $(patsubst %,$(BinPattern),nameclt)

all:: $(nameclt)

clean::
	$(RM) $(nameclt)

export:: $(nameclt)
	@$(ExportExecutable)

$(nameclt): nameclt.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
