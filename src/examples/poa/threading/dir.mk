CXXSRCS = mainthread.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

CXXDEBUGFLAGS = -g

mainthread = $(patsubst %,$(BinPattern),mainthread)

all:: $(mainthread)

clean::
	$(RM) $(mainthread)

export:: $(mainthread)
	$(ExportExecutable)

$(mainthread): mainthread.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
