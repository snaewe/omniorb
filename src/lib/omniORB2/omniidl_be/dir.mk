PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be

SUBDIRS = cxx

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: showast.py
	@(file="showast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))
