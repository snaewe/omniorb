
CXXSRCS = eg1.cc eg2_impl.cc eg2_clt.cc eg3_impl.cc eg3_clt.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo

ifdef OSF1
ifeq ($(notdir $(CXX)),cxx)
NoTieExample = 1
endif
endif

ifndef NoTieExample
# -Wbtp tells the compiler to generate tie implementation template
OMNIORB_IDL += -Wbtp
eg3_tieimpl = $(patsubst %,$(BinPattern),eg3_tieimpl)
endif


eg1        = $(patsubst %,$(BinPattern),eg1)
eg2_impl   = $(patsubst %,$(BinPattern),eg2_impl)
eg2_clt    = $(patsubst %,$(BinPattern),eg2_clt)
eg3_impl   = $(patsubst %,$(BinPattern),eg3_impl)
eg3_clt    = $(patsubst %,$(BinPattern),eg3_clt)


all:: $(eg1) $(eg2_impl) $(eg2_clt)  $(eg3_impl) $(eg3_clt) $(eg3_tieimpl)

clean::
	$(RM) $(eg1) $(eg2_impl) $(eg2_clt) $(eg3_impl) $(eg3_clt) \
		$(eg3_tieimpl)

export:: $(eg1) $(eg2_impl) $(eg2_clt) $(eg3_impl) $(eg3_clt) $(eg3_tieimpl)
	@(module="echoexamples"; $(ExportExecutable))

$(eg1): eg1.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(eg2_impl): eg2_impl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(eg2_clt): eg2_clt.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(eg3_impl): eg3_impl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(eg3_clt): eg3_clt.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

ifndef NoTieExample
$(eg3_tieimpl): eg3_tieimpl.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
endif
