
CXXSRCS = eg1.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo


eg1 = $(patsubst %,$(BinPattern), eg1)


all:: $(eg1)

clean::
	$(RM) $(eg1)

$(eg1): eg1.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
