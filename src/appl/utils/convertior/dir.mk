CXXSRCS = convertior.cc

CorbaImplementation = OMNIORB2
DIR_CPPFLAGS = $(CORBA_CPPFLAGS) $(OMNIORB2_CPPFLAGS) $(OMNITHREAD_CPPFLAGS)

CONVERTIOR = $(patsubst %,$(BinPattern),convertior)

all:: $(CONVERTIOR)

clean::
	$(RM) $(CONVERTIOR)

$(CONVERTIOR): convertior.o $(CORBA_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(CORBA_LIB) $(OMNITHREAD_LIB)"; $(CXXExecutable))

export:: $(CONVERTIOR)
	@$(ExportExecutable)
