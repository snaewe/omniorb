PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be

SUBDIRS = cplusplus

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: dump.py
	@(file="dump.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: python.py
	@(file="python.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: cpp.py
	@(file="cpp.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: showast.py
	@(file="showast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

