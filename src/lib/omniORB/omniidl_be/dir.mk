PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl_be

ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


SUBDIRS = cxx

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)

install::
	@(file="__init__.py"; dir="$(INSTALLPYLIBDIR)"; $(ExportFileToDir))
endif
