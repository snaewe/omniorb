# dir.mk for omniORB2.
#

SUBDIRS = orbcore dynamic lifecycle


all:: Naming.hh bootstrap.hh ir.hh corbaidl.hh omniLifeCycle.hh


export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: bootstrap.hh
	@(file="bootstrap.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: ir.hh
	@(file="ir.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: corbaidl.hh
	@(file="corbaidl.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: omniLifeCycle.hh
	@(file="omniLifeCycle.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))


veryclean::
	$(RM)	Naming.idl Naming.hh NamingSK.cc NamingDynSK.cc \
		bootstrap.idl bootstrap.hh bootstrapSK.cc bootstrapDynSK.cc \
		ir.idl ir.hh irSK.cc irDynSK.cc \
		corbaidl.idl corbaidl.hh corbaidlSK.cc corbaidlDynSK.cc \
		omniLifeCycle.idl omniLifeCycle.hh omniLifeCycleSK.cc \
			omniLifeCycleDynSK.cc


all::
	@$(MakeSubdirs)

clean::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)


######################################################################
# Make rules for stubs.                                              #
######################################################################

ifndef OMNIORB2_IDL_FPATH
OMNIORB2_IDL_FPATH = $(OMNIORB2_IDL)
endif

Naming.hh NamingSK.cc NamingDynSK.cc: Naming.idl
	-if [ "$<" != Naming.idl ]; then $(CP) $< . ; fi
	$(OMNIORB2_IDL_FPATH) Naming.idl

bootstrap.hh bootstrapSK.cc bootstrapDynSK.cc: bootstrap.idl
	-if [ "$<" != bootstrap.idl ]; then $(CP) $< . ; fi
	$(OMNIORB2_IDL_FPATH) bootstrap.idl

ir.hh irSK.cc irDynSK.cc: ir.idl corbaidl.hh
	-if [ "$<" != ir.idl ]; then $(CP) $< . ; fi
	$(OMNIORB2_IDL_FPATH) -m ir.idl

corbaidl.hh corbaidlSK.cc corbaidlDynSK.cc: corbaidl.idl
	-if [ "$<" != corbaidl.idl ]; then $(CP) $< . ; fi
	$(OMNIORB2_IDL_FPATH) -m corbaidl.idl

omniLifeCycle.hh omniLifeCycleSK.cc omniLifeCycleDynSK.cc: omniLifeCycle.idl
	-if [ "$<" != omniLifeCycle.idl ]; then $(CP) $< . ; fi
	$(OMNIORB2_IDL_FPATH) -m omniLifeCycle.idl
