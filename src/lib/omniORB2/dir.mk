# dir.mk for omniORB.
#

ifndef EmbeddedSystem
SUBDIRS = omniidl_be
endif

SUBDIRS += orbcore

ifndef OrbCoreOnly
SUBDIRS += dynamic
endif

EXPORTHEADERS = omniORB3/Naming.hh \
                omniORB3/corbaidl_defs.hh \
                omniORB3/corbaidl_operators.hh \
                omniORB3/corbaidl_poa.hh \
                omniORB3/ir_defs.hh \
                omniORB3/ir_operators.hh \
                omniORB3/ir_poa.hh \
                omniORB3/omniLifeCycle.hh

STUBHEADERS = $(EXPORTHEADERS) \
              omniORB3/bootstrap.hh

all:: $(STUBHEADERS)

export:: omniORB3/bootstrap.hh

export:: $(EXPORTHEADERS)
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir) \
          done )

veryclean::
	$(RM) $(STUBHEADERS) omniORB3/*SK.cc

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)


######################################################################
# Make rules for stubs.                                              #
######################################################################

OMNIORB_IDL += -p$(BASE_OMNI_TREE)/src/lib/omniORB2 

omniORB3/Naming.hh : Naming.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL) -ComniORB3 $<

omniORB3/bootstrap.hh : bootstrap.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL) -ComniORB3 $<

omniORB3/ir_defs.hh omniORB3/ir_oprators.hh omniORB3/ir_poa.hh: ir.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL) -WbF $(IMPORT_IDLFLAGS) -ComniORB3 $<

omniORB3/corbaidl_defs.hh corbaidl_operators.hh corbaidl_poa.hh: corbaidl.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL) -WbF -ComniORB3 $<

omniORB3/omniLifeCycle.hh : omniLifeCycle.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL) -ComniORB3 $<

ciao:: $(STUBHEADERS)
	@$(MakeSubdirs)



