
CXXSRCS = bd_client.cc bd_server.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_callback


bd_client   = $(patsubst %,$(BinPattern),bd_client)
bd_server   = $(patsubst %,$(BinPattern),bd_server)
bd_shutdown = $(patsubst %,$(BinPattern),bd_shutdown)


all:: $(bd_client) $(bd_server) $(bd_shutdown)

clean::
	$(RM) $(bd_client) $(bd_server) $(bd_shutdown)

$(bd_client): bd_client.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(bd_server): bd_server.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(bd_shutdown): bd_shutdown.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
