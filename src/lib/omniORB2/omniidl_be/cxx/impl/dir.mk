PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be/cxx/impl


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: template.py
	@(file="template.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))
