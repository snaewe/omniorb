ifndef ATMosArchitecture 
SUBDIRS = omniNames utils
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
