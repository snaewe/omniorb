
CXXSRCS = anyExample_impl.cc anyExample_clt.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = anyExample


anyExample_impl = $(patsubst %,$(BinPattern),anyExample_impl)
anyExample_clt  = $(patsubst %,$(BinPattern),anyExample_clt)


all:: $(anyExample_impl) $(anyExample_clt)

clean::
	$(RM) $(anyExample_impl) $(anyExample_clt)

export::$(anyExample_impl) 
	@$(ExportExecutable)
	@(packages="anyExample"; $(ExportATMosPackages))

export:: $(anyExample_clt)
	@$(ExportExecutable)

$(anyExample_impl): anyExample_impl.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(anyExample_clt): anyExample_clt.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
