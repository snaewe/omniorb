PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: dump.py
	@(file="dump.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: python.py
	@(file="python.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

