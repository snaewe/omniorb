SUBDIRS = omnithread omniORB

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif

ciao::
	@$(MakeSubdirs)

ifndef EmbeddedSystem
ifdef Win32Platform
export::
	(cd $(EXPORT_TREE)/$(BINDIR); rebase.exe -b 0x68000000 -d *_rt.dll; )
	(cd $(EXPORT_TREE)/$(BINDIR); rebase.exe -b 0x68000000 -d *_rtd.dll; )
endif
endif
