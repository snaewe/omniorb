
CXXSRCS = catior.cc

CorbaImplementation = OMNIORB

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CATIOR = $(patsubst %,$(BinPattern),catior)


all:: $(CATIOR)

clean::
	$(RM) $(CATIOR)

$(CATIOR): catior.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

export:: $(CATIOR)
	@$(ExportExecutable)
