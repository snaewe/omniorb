ifndef ATMos
SUBDIRS = omniNames utils
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ciao::
	@$(MakeSubdirs)
