ifndef ATMos
#SUBDIRS = omniNames utils
SUBDIRS = 
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ciao::
	@$(MakeSubdirs)
