ifdef UnixPlatform
SUBDIRS = omkdepend omniidl2
endif

ifdef Win32Platform
SUBDIRS = omniidl2
endif


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
