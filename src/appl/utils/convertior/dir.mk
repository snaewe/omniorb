CXXSRCS = convertior.cc

CorbaImplementation = OMNIORB2
DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CONVERTIOR = $(patsubst %,$(BinPattern),convertior)

all:: $(CONVERTIOR)

clean::
	$(RM) $(CONVERTIOR)

$(CONVERTIOR): convertior.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

export:: $(CONVERTIOR)
	@$(ExportExecutable)
