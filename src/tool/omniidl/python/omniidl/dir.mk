PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlast.py
	@(file="idlast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idltype.py
	@(file="idltype.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlutil.py
	@(file="idlutil.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlvisitor.py
	@(file="idlvisitor.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: output.py
	@(file="output.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export::
	@(set -x; \
	cd $(PYLIBDIR); \
	python -c 'import compileall; compileall.compile_dir(".")'; \
	)
