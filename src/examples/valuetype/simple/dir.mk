CXXSRCS = vclient.cc vserver.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

DIR_IDLFLAGS = -Wbdebug

CXXDEBUGFLAGS = -g

CORBA_INTERFACES = value

vclient = $(patsubst %,$(BinPattern),vclient)
vserver = $(patsubst %,$(BinPattern),vserver)

all:: $(vclient) $(vserver)

clean::
	$(RM) $(vclient) $(vserver)

$(vclient): vclient.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(vserver): vserver.o $(CORBA_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
