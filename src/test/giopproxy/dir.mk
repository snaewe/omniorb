CXXSRCS = server.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

server = $(patsubst %,$(BinPattern),server)

all:: $(server)

clean::
	$(RM) $(server)

$(server): server.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
