PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl/be/cxx

SUBDIRS = header

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: tyutil.py
	@(file="tyutil.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: util.py
	@(file="util.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: name.py
	@(file="name.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: config.py
	@(file="config.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))


