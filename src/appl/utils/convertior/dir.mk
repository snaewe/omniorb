CXXSRCS = convertior.cc

CorbaImplementation = OMNIORB
DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CONVERTIOR = $(patsubst %,$(BinPattern),convertior)

all:: $(CONVERTIOR)

clean::
	$(RM) $(CONVERTIOR)

$(CONVERTIOR): convertior.o $(CORBA_LIB_NODYN_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

export:: $(CONVERTIOR)
	@$(ExportExecutable)

ifdef INSTALLTARGET
install:: $(CONVERTIOR)
	@$(InstallExecutable)
endif
