ifndef ATMos
SUBDIRS = omniNames utils omniMapper
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
