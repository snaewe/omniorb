UPTO_SERVICES_TOP = ../..

include ../cosinterfaces.mk
include ../orbinterfaces.mk

# Tell omniidl to generate the old BOA interfaces
OMNIORB_IDL_ANY_FLAGS += -WbBOA
lib    = $(patsubst %,$(LibPattern),cosBOA)

# Use a different stub dir for BOA stubs
CORBA_STUB_DIR = $(TOP)/stubBOA

include ../libdefs.mk

