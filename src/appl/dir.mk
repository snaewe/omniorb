ifndef ATMos
SUBDIRS = omniNames utils omniMapper
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ciao::
	@$(MakeSubdirs)
