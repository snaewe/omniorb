# dir.mk for omniORB2.
#

#SUBDIRS = orbcore dynamic lifecycle
SUBDIRS = orbcore dynamic


all:: Naming.hh bootstrap.hh ir_defs.hh corbaidl_defs.hh \
      omniLifeCycle.hh Firewall.hh


export:: Naming.hh
	@(file="Naming.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: bootstrap.hh

export:: ir_defs.hh
	@(file="ir_defs.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: ir_operators.hh
	@(file="ir_operators.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: corbaidl_defs.hh
	@(file="corbaidl_defs.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: corbaidl_operators.hh
	@(file="corbaidl_operators.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: omniLifeCycle.hh
	@(file="omniLifeCycle.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

export:: Firewall.hh
	@(file="Firewall.hh"; dir="$(EXPORT_TREE)/$(INCDIR)/omniORB2"; \
		$(ExportFileToDir))

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)


######################################################################
# Make rules for stubs.                                              #
######################################################################



ifndef OMNIORB2_IDL_FPATH
OMNIORB2_IDL_FPATH = $(OMNIORB2_IDL)
endif

ifndef OMNIORB2_IDL_ONLY_FPATH
OMNIORB2_IDL_ONLY_FPATH = $(OMNIORB2_IDL_ONLY)
endif

ifndef Win32Platform

define GENERATE_IDL_STUBS
extraoptions=$(patsubst %,-I%/idl,$(IMPORT_TREES)); \
$(OMNIORB2_IDL_ONLY_FPATH) $$idlflags $$extraoptions $^
endef

else

define GENERATE_IDL_STUBS
args="$(patsubst %,-I%/idl,$(IMPORT_TREES)) $^"; \
$(OMNIORB2_IDL_ONLY_FPATH) $$idlflags `echo $$args | sed 's/\/\/\([^/]*\)\//\1:\//g'`
endef

endif

Naming.hh NamingSK.cc NamingDynSK.cc: Naming.idl
	@(idlflags=-a; $(GENERATE_IDL_STUBS);)

bootstrap.hh bootstrapSK.cc: bootstrap.idl
	@($(GENERATE_IDL_STUBS);)

ir_defs.hh ir_operators.hh irSK.cc irDynSK.cc: ir.idl
	@(idlflags='-a -m -F'; $(GENERATE_IDL_STUBS);)

corbaidl_defs.hh corbaidl_operators.hh corbaidlSK.cc corbaidlDynSK.cc: corbaidl.idl
	@(idlflags='-a -m -F'; $(GENERATE_IDL_STUBS);)

omniLifeCycle.hh omniLifeCycleSK.cc omniLifeCycleDynSK.cc: omniLifeCycle.idl
	@(idlflags='-a -m'; $(GENERATE_IDL_STUBS);)

Firewall.hh FirewallSK.cc: Firewall.idl
	@(idlflags='-m -t'; $(GENERATE_IDL_STUBS);)


ciao:: Naming.hh bootstrap.hh ir_defs.hh corbaidl_defs.hh omniLifeCycle.hh Firewall.hh
	@$(MakeSubdirs)

lastveryclean::
	$(RM)	Naming.hh NamingSK.cc NamingDynSK.cc \
		bootstrap.hh bootstrapSK.cc \
		ir_defs.hh ir_operators.hh irSK.cc irDynSK.cc \
		corbaidl_defs.hh corbaidl_operators.hh \
                corbaidlSK.cc corbaidlDynSK.cc \
		omniLifeCycle.hh omniLifeCycleSK.cc omniLifeCycleDynSK.cc \
                Firewall.hh FirewallSK.cc


