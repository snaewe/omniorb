SUBDIRS = dummystub #alone

ifdef UnixPlatform
SUBDIRS += tcpwrapper
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
