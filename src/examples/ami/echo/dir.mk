
CXXSRCS = echo.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_ami

# -WbAMI tells the IDL compiler to generate AMI stubs
OMNIORB_IDL += -WbAMI

echo_ami = $(patsubst %,$(BinPattern),echo_ami)

all:: $(echo_ami)

export:: $(echo_ami)
	@(module="amiexamples"; $(ExportExecutable))

clean::
	$(RM) $(echo_ami)

$(echo_ami): echo.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))

