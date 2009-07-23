CXXSRCS = genior.cc

CorbaImplementation = OMNIORB
DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

GENIOR = $(patsubst %,$(BinPattern),genior)

all:: $(GENIOR)

clean::
	$(RM) $(GENIOR)

$(GENIOR): genior.o $(CORBA_LIB_NODYN_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

export:: $(GENIOR)
	@$(ExportExecutable)

ifdef INSTALLTARGET
install:: $(GENIOR)
	@$(InstallExecutable)
endif
