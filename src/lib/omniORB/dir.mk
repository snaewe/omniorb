# dir.mk for omniORB.
#

ifndef EmbeddedSystem
SUBDIRS = omniidl_be
endif

SUBDIRS += orbcore

ifndef OrbCoreOnly
SUBDIRS += dynamic codesets
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
		omniORB4/omniTypedefs.hh \
                omniORB4/bootstrap.hh


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

omniORB4/corbaidl_defs.hh corbaidl_operators.hh corbaidl_poa.hh: corbaidl.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -nf -P -WbF -ComniORB4 $<

omniORB4/boxes_defs.hh boxes_operators.hh boxes_poa.hh: boxes.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -v -nf -P -WbF -ComniORB4 $<

omniORB4/omniTypedefs.hh: omniTypedefs.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL_ONLY) -v -ComniORB4 $<

ciao:: $(STUBHEADERS)
	@$(MakeSubdirs)



