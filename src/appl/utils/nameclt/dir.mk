DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CXXSRCS = nameclt.cc

nameclt = $(patsubst %,$(BinPattern),nameclt)

all:: $(nameclt)

clean::
	$(RM) $(nameclt)

export:: $(nameclt)
	@$(ExportExecutable)

$(nameclt): nameclt.o $(CORBA_LIB_NODYN_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
