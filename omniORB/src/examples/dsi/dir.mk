CXXSRCS = echo_dsiimpl.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)


echo_dsiimpl = $(patsubst %,$(BinPattern),echo_dsiimpl)


all:: $(echo_dsiimpl)

clean::
	$(RM) $(echo_dsiimpl)

export:: $(echo_dsiimpl)
	@$(ExportExecutable)


$(echo_dsiimpl): echo_dsiimpl.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
