ifndef ATMos
SUBDIRS = genior catior convertior nameclt
endif

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
