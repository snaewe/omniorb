PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be

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

export:: showast.py
	@(file="showast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))
