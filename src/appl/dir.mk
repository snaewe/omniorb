ifdef SunArchitecture
SUBDIRS += omniNames
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
