
CXXSRCS = cb_client.cc cb_server.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_callback


cb_client   = $(patsubst %,$(BinPattern),cb_client)
cb_server   = $(patsubst %,$(BinPattern),cb_server)
cb_shutdown = $(patsubst %,$(BinPattern),cb_shutdown)


all:: $(cb_client) $(cb_server) $(cb_shutdown)

clean::
	$(RM) $(cb_client) $(cb_server) $(cb_shutdown)

$(cb_client): cb_client.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(cb_server): cb_server.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(cb_shutdown): cb_shutdown.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
