CXXSRCS = echo_diiclt.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)


echo_diiclt = $(patsubst %,$(BinPattern),echo_diiclt)


all:: $(echo_diiclt)

clean::
	$(RM) $(echo_diiclt)

export:: $(echo_diiclt)
	@$(ExportExecutable)


$(echo_diiclt): echo_diiclt.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
