PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be/cxx/dynskel

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

