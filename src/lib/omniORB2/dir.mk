# dir.mk for omniORB.
#

#SUBDIRS = orbcore dynamic lifecycle
SUBDIRS = omniidl_be orbcore dynamic

STUBHEADERS = omniORB3/Naming.hh \
              omniORB3/bootstrap.hh \
              omniORB3/corbaidl_defs.hh \
              omniORB3/corbaidl_operators.hh \
              omniORB3/corbaidl_poa.hh \
              omniORB3/ir_defs.hh \
              omniORB3/ir_operators.hh \
              omniORB3/ir_poa.hh \
              omniORB3/omniLifeCycle.hh


all:: $(STUBHEADERS)


export:: omniORB3/Naming.hh
	@(file="omniORB3/Naming.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/bootstrap.hh

export:: omniORB3/ir_defs.hh
	@(file="omniORB3/ir_defs.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3";\
		$(ExportFileToDir))

export:: omniORB3/ir_operators.hh
	@(file="omniORB3/ir_operators.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/ir_poa.hh
	@(file="omniORB3/ir_poa.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/corbaidl_defs.hh
	@(file="omniORB3/corbaidl_defs.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/corbaidl_operators.hh
	@(file="omniORB3/corbaidl_operators.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/corbaidl_poa.hh
	@(file="omniORB3/corbaidl_poa.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))

export:: omniORB3/omniLifeCycle.hh
	@(file="omniORB3/omniLifeCycle.hh"; \
          dir="$(EXPORT_TREE)/$(INCDIR)/omniORB3"; \
		$(ExportFileToDir))


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)


######################################################################
# Make rules for stubs.                                              #
######################################################################

ifndef OMNIORB_IDL_FPATH
OMNIORB_IDL_FPATH = $(OMNIORB_IDL)
endif

OMNIORB_IDL_FPATH += -p$(BASE_OMNI_TREE)/src/lib/omniORB2 

omniORB3/Naming.hh : Naming.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL_FPATH) -ComniORB3 $<

omniORB3/bootstrap.hh : bootstrap.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL_FPATH) -ComniORB3 $<

omniORB3/ir_defs.hh omniORB3/ir_oprators.hh omniORB3/ir_poa.hh: ir.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL_FPATH) -WbF $(IMPORT_IDLFLAGS) -ComniORB3 $<

omniORB3/corbaidl_defs.hh corbaidl_operators.hh corbaidl_poa.hh: corbaidl.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL_FPATH) -WbF -ComniORB3 $<

omniORB3/omniLifeCycle.hh : omniLifeCycle.idl
	@(dir=omniORB3; $(CreateDir))
	$(OMNIORB_IDL_FPATH) -ComniORB3 $<

ciao:: $(STUBHEADERS)
	@$(MakeSubdirs)

veryclean::
	$(RM) $(STUBHEADERS) omniORB3/*SK.cc


