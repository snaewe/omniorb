
CXXSRCS = eg2_impl.cc eg2_clt.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

eg2_impl   = $(patsubst %,$(BinPattern),eg2_impl)
eg2_clt    = $(patsubst %,$(BinPattern),eg2_clt)

all:: $(eg2_impl) $(eg2_clt)

clean::
	$(RM) $(eg2_impl) $(eg2_clt) 

$(eg2_impl): eg2_impl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN) $(OMNIORB_SSL_LIB)"; $(CXXExecutable))

$(eg2_clt): eg2_clt.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN) $(OMNIORB_SSL_LIB)"; $(CXXExecutable))

