SUBDIRS = omnithread omniORB2

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ciao::
	@$(MakeSubdirs)

ifndef EmbeddedSystem
ifdef Win32Platform
export::
	(cd $(EXPORT_TREE)/$(BINDIR); rebase.exe -b 0x68000000 -d *_rt.dll; )
	(cd $(EXPORT_TREE)/$(BINDIR); rebase.exe -b 0x68000000 -d *_rtd.dll; )
endif
endif
