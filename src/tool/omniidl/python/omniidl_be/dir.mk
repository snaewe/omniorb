PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: dump.py
	@(file="dump.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: example.py
	@(file="example.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export::
	@(set -x; \
	cd $(PYLIBDIR); \
	python -c 'import compileall; compileall.compile_dir(".")'; \
	)
