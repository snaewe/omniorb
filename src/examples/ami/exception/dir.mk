
CXXSRCS = exc_client.cc exc_server.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = exc

# -WbAMI tells the IDL compiler to generate AMI stubs
OMNIORB_IDL += -WbAMI

exc_client = $(patsubst %,$(BinPattern),exc_client)
exc_server = $(patsubst %,$(BinPattern),exc_server)

all:: $(exc_client) $(exc_server)

export:: $(exc_client) $(exc_server)
	@(module="amiexamples"; $(ExportExecutable))

clean::
	$(RM) $(exc_client) $(exc_server)

$(exc_client): exc_client.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(exc_server): exc_server.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))


