DIR_CPPFLAGS = -I../.. $(patsubst %,-I%/../..,$(VPATH))

CXXSRCS = gatekeeper.cc

CXXOBJS = gatekeeper.o

lib = $(patsubst %,$(LibPattern),omniGK_stub)

all:: $(lib)

$(lib): $(CXXOBJS)
	@$(StaticLinkLibrary)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)


