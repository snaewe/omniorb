PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be/cxx/header

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: defs.py
	@(file="defs.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

