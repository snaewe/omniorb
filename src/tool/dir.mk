# Check for Python and complain early
ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif

ifdef UnixPlatform
SUBDIRS = omkdepend omniidl
endif

ifdef Win32Platform
SUBDIRS = omniidl
endif


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif
