SUBDIRS = dummystub alone

ifndef NoGateKeeper
ifdef UnixPlatform
SUBDIRS += tcpwrapper
endif
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
