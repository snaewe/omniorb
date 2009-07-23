PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniORB/COS
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniORB/COS

# Where to look for IDL files.
# This list could be configurable.

IDLDIRS = $(OMNIORB_ROOT)/share/idl/omniORB/COS \
          $(OMNIORB_ROOT)/share/idl/omniORB \
          $(OMNIORB_ROOT)/idl/omniORB \
          $(OMNIORB_ROOT)/idl/omniORB/COS \
          $(BASE_OMNI_TREE)/idl \
          $(BASE_OMNI_TREE)/idl/COS \
          $(DATADIR)/idl/omniORB \
          $(DATADIR)/idl/omniORB/COS

# We list IDL files below and use the vpath to find them.
# Another possibility is to use $(wildcard $(IDLPATH)/*.idl)
# and then filter out the bad IDL files.

vpath %.idl $(IDLDIRS)

INCDIRS = $(foreach d, $(IDLDIRS), -I$(d))

# These IDL files do not (yet) cleanly compile.

BADIDLFILES = CosConcurrencyControl.idl \
CosLicensingManager.idl \
CosTransactions.idl CosTSPortability.idl \
DCE_CIOPSecurity.idl \
NRService.idl \
SECIOP.idl \
SecurityAdmin.idl Security.idl \
SecurityLevel1.idl SecurityLevel2.idl SecurityReplaceable.idl \
SSLIOP.idl TimeBase.idl

# Files with dashes are converted to files with underscores :(
# Should be able to fix with a string substitution...
# Lname-library.idl

IDLFILES = CosCollection.idl \
           CosCompoundLifeCycle.idl \
           CosContainment.idl CosEventChannelAdmin.idl \
           CosEventComm.idl CosExternalizationContainment.idl \
           CosExternalization.idl CosExternalizationReference.idl \
           CosGraphs.idl \
           CosLifeCycleContainment.idl CosLifeCycle.idl \
           CosLifeCycleReference.idl CosNaming.idl \
           CosNotification.idl CosNotifyChannelAdmin.idl \
           CosNotifyComm.idl CosNotifyFilter.idl \
           CosObjectIdentity.idl CosPersistenceDDO.idl \
           CosPersistenceDS_CLI.idl CosPersistencePDS_DA.idl \
           CosPersistencePDS.idl CosPersistencePID.idl \
           CosPersistencePO.idl CosPersistencePOM.idl \
           CosPropertyService.idl CosQueryCollection.idl \
           CosQuery.idl CosReference.idl \
           CosRelationships.idl CosStream.idl \
           CosTime.idl CosTimerEvent.idl \
           CosTradingDynamic.idl CosTrading.idl CosTradingRepos.idl \
           CosTypedEventChannelAdmin.idl CosTypedEventComm.idl \
           CosTypedNotifyChannelAdmin.idl CosTypedNotifyComm.idl \
           LifeCycleService.idl \
           RDITestTypes.idl \
           TimeBase.idl

FILES =
INSTALLEDFILES =

# All of the IDL
FILES += $(IDLFILES:.idl=_idl.py)
FILES += $(IDLFILES:.idl=/__init__.py)
FILES += $(IDLFILES:.idl=__POA/__init__.py)
INSTALLEDFILES += $(foreach f, $(FILES), $(INSTALLPYLIBDIR)/$(f))
# Put CosNaming at the top of the installation area too
INSTALLEDFILES += $(foreach f, $(filter CosNaming%, $(FILES)), $(INSTALLPYTHONDIR)/$(f))

EXPORTEDFILES += $(foreach f, $(FILES), $(PYLIBDIR)/$(f))
EXPORTEDFILES += $(foreach f, $(filter CosNaming%, $(FILES)), $(PYLIBROOT)/$(f))

# __init__.py to make it a package
FILES += __init__.py
INSTALLEDFILES += $(INSTALLPYTHONDIR)/__init__.py

# A .pth file to expose omniORB/COS to the global namespace
FILES += omniORB.pth
INSTALLEDFILES += $(INSTALLPYTHONDIR)/omniORB.pth


all:: $(FILES)

clean::
	$(RM) $(FILES)
	$(RM) -r $(IDLFILES:.idl=) $(IDLFILES:.idl=__POA)

export:: $(EXPORTEDFILES)
	cd $(PYLIBDIR); \
	$(PYTHON) -c "import compileall; compileall.compile_dir('.')";

ifdef INSTALLTARGET
install:: $(INSTALLEDFILES)
	cd $(INSTALLPYLIBDIR); \
	$(PYTHON) -c "import compileall; compileall.compile_dir('.')";
endif

# Specific rule for installing CosNaming at the top level
# Per Duncan's point regarding the CORBA mapping standard
# and backward compatibility with previous omniORB distros.
$(INSTALLPYTHONDIR)/CosNaming%: CosNaming%
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))

# Specific rule for installing a path file to expose all
# of the CORBA stubs to the top level name space without
# requiring that they be installed there. Note that the
# .pth file is only respected in python's standard area.
$(INSTALLPYTHONDIR)/omniORB.pth: omniORB.pth
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))

$(INSTALLPYTHONDIR)/__init__.py: __init__.py
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))

# Generic rule to install files.
$(INSTALLPYLIBDIR)/%: %
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))

# Specific rule for building the path file.
omniORB.pth: ;
	@echo $(subst $(INSTALLPYTHONDIR)/,,$(INSTALLPYLIBDIR)) > $@


# Rules for make export
$(PYLIBROOT)/CosNaming%: CosNaming%
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))

$(PYLIBDIR)/%: %
	@(dir="$(dir $@)"; file="$^"; $(ExportFileToDir))


# Generic rule for building all stubs and skeletons.
%_idl.py %/__init__.py %__POA/__init__.py: %.idl
	$(OMNIIDL) -p$(BASE_OMNI_TREE)/omniidl_be \
	$(INCDIRS) -bpython $^
