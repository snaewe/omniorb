# dir.mk for omniORB.
#

ifndef EmbeddedSystem
SUBDIRS = omniidl_be
endif

SUBDIRS += orbcore

ifndef OrbCoreOnly
SUBDIRS += dynamic codesets connections

ifdef EnableZIOP
SUBDIRS += ziop
endif

endif

EXPORTHEADERS = omniORB4/distdate.hh \
                omniORB4/Naming.hh \
                omniORB4/corbaidl_defs.hh \
                omniORB4/corbaidl_operators.hh \
                omniORB4/corbaidl_poa.hh \
                omniORB4/ir_defs.hh \
                omniORB4/ir_operators.hh \
                omniORB4/ir_poa.hh \
                omniORB4/boxes_defs.hh \
                omniORB4/boxes_operators.hh \
                omniORB4/boxes_poa.hh \
                omniORB4/poa_enums_defs.hh \
                omniORB4/poa_enums_operators.hh \
                omniORB4/poa_enums_poa.hh \
		omniORB4/omniTypedefs.hh \
                omniORB4/bootstrap.hh \
		omniORB4/omniConnectionData.hh

ifdef EnableZIOP
EXPORTHEADERS += omniORB4/compression.hh \
		 omniORB4/messaging.hh \
		 omniORB4/ziop_defs.hh
endif


all:: $(EXPORTHEADERS)

export:: $(EXPORTHEADERS)
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/omniORB4"; \
		$(ExportFileToDir) \
          done )

ifdef INSTALLTARGET
install:: $(EXPORTHEADERS)
	@(for i in $^; do \
            file="$$i"; \
            dir="$(INSTALLINCDIR)/omniORB4"; \
		$(ExportFileToDir) \
          done )
endif


veryclean::
	$(RM) $(EXPORTHEADERS) omniORB4/*SK.cc

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif


######################################################################
# Make rules for stubs.                                              #
######################################################################

ifdef DisableLongDouble
UNDEFINES = -UHAS_LongDouble
endif

OMNIORB_IDL += -p$(BASE_OMNI_TREE)/src/lib/omniORB -Wbdebug
OMNIORB_IDL_ONLY += -p$(BASE_OMNI_TREE)/src/lib/omniORB -Wbdebug

omniORB4/distdate.hh : $(BASE_OMNI_TREE)/update.log
	@(dir=omniORB4; $(CreateDir))
	$(PYTHON) $(BASE_OMNI_TREE)/bin/scripts/distdate.py <$(BASE_OMNI_TREE)/update.log >omniORB4/distdate.hh

omniORB4/Naming.hh : Naming.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -ComniORB4 $<

omniORB4/bootstrap.hh : bootstrap.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -ComniORB4 $<

omniORB4/ir_defs.hh omniORB4/ir_operators.hh omniORB4/ir_poa.hh: ir.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -WbF $(IMPORT_IDLFLAGS) -ComniORB4 $<

omniORB4/corbaidl_defs.hh omniORB4/corbaidl_operators.hh omniORB4/corbaidl_poa.hh: corbaidl.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -nf -P $(UNDEFINES) -WbF -ComniORB4 $<

omniORB4/boxes_defs.hh omniORB4/boxes_operators.hh omniORB4/boxes_poa.hh: boxes.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -nf -P $(UNDEFINES) -WbF -ComniORB4 $<

omniORB4/poa_enums_defs.hh omniORB4/poa_enums_operators.hh omniORB4/poa_enums_poa.hh: poa_enums.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -nf -P $(UNDEFINES) -WbF -ComniORB4 $<

omniORB4/omniTypedefs.hh: omniTypedefs.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v -ComniORB4 $<

omniORB4/omniConnectionData.hh: omniConnectionData.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v -ComniORB4 $<

ifdef EnableZIOP

omniORB4/compression.hh : compression.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v $(IMPORT_IDLFLAGS) -ComniORB4 $<

omniORB4/messaging.hh : messaging.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v $(IMPORT_IDLFLAGS) -ComniORB4 $<

omniORB4/ziop_defs.hh : ziop.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v $(IMPORT_IDLFLAGS) -WbF -ComniORB4 $<

endif


ciao:: $(STUBHEADERS)
	@$(MakeSubdirs)



