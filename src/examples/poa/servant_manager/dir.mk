
CXXSRCS = servant_locator.cc servant_activator.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo


servant_locator = $(patsubst %,$(BinPattern), servant_locator)
servant_activator = $(patsubst %,$(BinPattern), servant_activator)


all:: $(servant_locator) $(servant_activator)

export:: $(servant_locator) $(servant_activator)

clean::
	$(RM) $(servant_locator) $(servant_activator)


$(servant_locator): servant_locator.o $(CORBA_STATIC_STUB_OBJS) \
			$(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(servant_activator): servant_activator.o $(CORBA_STATIC_STUB_OBJS) \
			$(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
