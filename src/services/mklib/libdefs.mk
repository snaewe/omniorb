# select a subset of COS interfaces to build here:

# All the interfaces:
#COS_INTERFACES = $(COS_ALL_INT)

# Just the Notification interfaces:
COS_INTERFACES = $(TIMEBASE_INT) $(COS_EVT_INT) $(COS_NFY_INT)

CORBA_INTERFACES = $(ORB_INTERFACES) $(COS_INTERFACES)

GMAKE = $(GMAKE) -s

# Include path for omniidl/2: <omnitop>/idl plus <services_top>/idl
DIR_IDL_FLAGS = $(patsubst %,-I%/idl,$(IMPORT_TREES)) $(patsubst %,-I%/$(UPTO_SERVICES_TOP)/idl,$(VPATH))

# omniidl + omniidl2 do not support long long and, hence,
# we need to pass the -DNOLONGLONG flag to generate correct stubs.
OMNIORB_IDL_ANY_FLAGS  += -DNOLONGLONG $(DIR_IDL_FLAGS)
OMNIORB2_IDL_ANY_FLAGS += -DNOLONGLONG $(DIR_IDL_FLAGS)

# Tell omniidl to splice modules together (not needed?)
# OMNIORB_IDL_ANY_FLAGS += --Wbsplice-modules

CXXDEBUGFLAGS = -g
DIR_CPPFLAGS  = $(CORBA_CPPFLAGS)
DIR_STUBS_CPPFLAGS = $(IMPORT_CPPFLAGS)

# Add <top>/include/omniORB3 to DIR_CPPFLAGS:
#    alas, some required .hh files are in this dir
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB3,$(IMPORT_TREES))

# Look for .idl files in <top>/idl plus <services_top>/idl
vpath %.idl $(IMPORT_TREES:%=%/idl) $(VPATH:%=%/$(UPTO_SERVICES_TOP)/idl)

all:: $(lib)

export:: $(lib)
	$(ExportLibrary)

debug:
	@echo ORB_INTERFACES = $(ORB_INTERFACES)
	@echo COS_INTERFACES = $(COS_INTERFACES)
	@echo DIR_IDL_FLAGS  = $(DIR_IDL_FLAGS)
	@echo IMPORT_CPPFLAGS  = $(IMPORT_CPPFLAGS)
	@echo CORBA_CPPFLAGS  = $(CORBA_CPPFLAGS)
	@echo DIR_STUBS_CPPFLAGS  = $(DIR_STUBS_CPPFLAGS)
	@echo DIR_CPPFLAGS  = $(DIR_CPPFLAGS)

$(lib): $(CORBA_STUB_OBJS)
	@$(StaticLinkLibrary)

clean::
	$(RM) $(lib) *.d

veryclean::
	$(RM) $(lib) *.d
