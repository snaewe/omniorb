CXXSRCS = genior.cc

CorbaImplementation = OMNIORB2
DIR_CPPFLAGS = $(CORBA_CPPFLAGS) $(OMNIORB2_CPPFLAGS) $(OMNITHREAD_CPPFLAGS)

GENIOR = $(patsubst %,$(BinPattern),genior)

all:: $(GENIOR)

clean::
	$(RM) $(GENIOR)

$(GENIOR): genior.o $(CORBA_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(CORBA_LIB) $(OMNITHREAD_LIB)"; $(CXXExecutable))

export:: $(GENIOR)
	@$(ExportExecutable)
