
CXXSRCS = echo.cc echo_poller.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_ami

# -WbAMI tells the IDL compiler to generate AMI stubs
OMNIORB_IDL += -WbAMI

echo_ami = $(patsubst %,$(BinPattern),echo_ami)
echo_poller = $(patsubst %,$(BinPattern),echo_poller)

all:: $(echo_ami) $(echo_poller)

export:: $(echo_ami) $(echo_poller)
	@(module="amiexamples"; $(ExportExecutable))

clean::
	$(RM) $(echo_ami) $(echo_poller)

$(echo_ami): echo.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

$(echo_poller): echo_poller.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

