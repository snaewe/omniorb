CXXSRCS = greeting.cc eg1.cc \
          eg2_impl.cc eg2_clt.cc \
          eg3_impl.cc eg3_clt.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

eg1        = $(patsubst %,$(BinPattern),eg1)
eg2_impl   = $(patsubst %,$(BinPattern),eg2_impl)
eg2_clt    = $(patsubst %,$(BinPattern),eg2_clt)
eg3_impl   = $(patsubst %,$(BinPattern),eg3_impl)
eg3_clt    = $(patsubst %,$(BinPattern),eg3_clt)

all:: $(eg1) $(eg2_impl) $(eg2_clt)  $(eg3_impl) $(eg3_clt)

clean::
	$(RM) $(eg1) $(eg2_impl) $(eg2_clt) $(eg3_impl) $(eg3_clt)

export:: $(eg1) $(eg2_impl) $(eg2_clt) $(eg3_impl) $(eg3_clt)
	@(module="echoexamples"; $(ExportExecutable))

$(eg1): eg1.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(eg2_impl): eg2_impl.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(eg2_clt): eg2_clt.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(eg3_impl): eg3_impl.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(eg3_clt): eg3_clt.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

