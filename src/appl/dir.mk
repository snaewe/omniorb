ifdef UnixArchitecture
SUBDIRS += omniNames
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
