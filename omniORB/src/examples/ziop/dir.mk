CXXSRCS = ziop_clt.cc ziop_impl.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

ziop_clt  = $(patsubst %,$(BinPattern),ziop_clt)
ziop_impl = $(patsubst %,$(BinPattern),ziop_impl)

all:: $(ziop_clt) $(ziop_impl)

clean::
	$(RM) $(ziop_clt) $(ziop_impl)

$(ziop_clt): ziop_clt.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND) $(OMNIORB_ZIOP_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN) $(OMNIORB_ZIOP_LIB)"; $(CXXExecutable))

$(ziop_impl): ziop_impl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND) $(OMNIORB_ZIOP_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN) $(OMNIORB_ZIOP_LIB)"; $(CXXExecutable))
