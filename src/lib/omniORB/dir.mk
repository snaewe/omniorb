# dir.mk for omniORB.
#

#ifndef EmbeddedSystem
#SUBDIRS = omniidl_be
#endif

SUBDIRS += orbcore

#ifndef OrbCoreOnly
#SUBDIRS += dynamic
#endif
#
#EXPORTHEADERS = omniORB4/Naming.hh \
#                omniORB4/corbaidl_defs.hh \
#                omniORB4/corbaidl_operators.hh \
#                omniORB4/corbaidl_poa.hh \
#                omniORB4/ir_defs.hh \
#                omniORB4/ir_operators.hh \
#                omniORB4/ir_poa.hh
#
#
#STUBHEADERS = $(EXPORTHEADERS) \
#              omniORB4/bootstrap.hh
#
#all:: $(STUBHEADERS)
#
#export:: omniORB4/bootstrap.hh
#
#export:: $(EXPORTHEADERS)
#	@(for i in $^; do \
#            file="$$i"; \
#            dir="$(EXPORT_TREE)/$(INCDIR)/omniORB4"; \
#		$(ExportFileToDir) \
#          done )
#
#veryclean::
#	$(RM) $(STUBHEADERS) omniORB4/*SK.cc

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)


######################################################################
# Make rules for stubs.                                              #
######################################################################

OMNIORB_IDL += -p$(BASE_OMNI_TREE)/src/lib/omniORB2 

omniORB4/Naming.hh : Naming.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -ComniORB4 $<

omniORB4/bootstrap.hh : bootstrap.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -ComniORB4 $<

omniORB4/ir_defs.hh omniORB4/ir_operators.hh omniORB4/ir_poa.hh: ir.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -WbF $(IMPORT_IDLFLAGS) -ComniORB4 $<

omniORB4/corbaidl_defs.hh corbaidl_operators.hh corbaidl_poa.hh: corbaidl.idl
	@(dir=omniORB4; $(CreateDir))
	$(OMNIORB_IDL) -WbF -ComniORB4 $<

ciao:: $(STUBHEADERS)
	@$(MakeSubdirs)



