PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be/cxx/header

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: defs.py
	@(file="defs.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: opers.py
	@(file="opers.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: poa.py
	@(file="poa.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: tie.py
	@(file="tie.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: forward.py
	@(file="forward.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: marshal.py
	@(file="marshal.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: tcstring.py
	@(file="tcstring.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))