PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be/cxx/skel

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: poa.py
	@(file="poa.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: mangler.py
	@(file="mangler.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: dispatch.py
	@(file="dispatch.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: proxy.py
	@(file="proxy.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: template.py
	@(file="template.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

