ifndef ATMos
SUBDIRS = genior catior convertior nameclt
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ciao::
	@$(MakeSubdirs)
