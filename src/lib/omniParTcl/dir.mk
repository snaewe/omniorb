
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(OMNIPARTCL_CPPFLAGS)

CXXSRCS = omniParTcl.cc scriptQueue.cc tclDStringClass.cc

lib = $(patsubst %,$(LibPattern),omniParTcl)

all:: $(lib)

$(lib): omniParTcl.o scriptQueue.o tclDStringClass.o
	@$(StaticLinkLibrary)

export: $(lib)
	@$(ExportLibrary)
