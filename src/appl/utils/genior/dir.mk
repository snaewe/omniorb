CXXSRCS = genior.cc

CorbaImplementation = OMNIORB2
DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

GENIOR = $(patsubst %,$(BinPattern),genior)

all:: $(GENIOR)

clean::
	$(RM) $(GENIOR)

$(GENIOR): genior.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

export:: $(GENIOR)
	@$(ExportExecutable)
