PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be/cxx/ami


ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


all::


export::


export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: exholder.py
	@(file="exholder.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: poller.py
	@(file="poller.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: rhandler.py
	@(file="rhandler.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: objref.py
	@(file="objref.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: ami.py
	@(file="ami.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: calldesc.py
	@(file="calldesc.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: hooks.py
	@(file="hooks.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export::
	@(set -x; \
	cd $(PYLIBDIR); \
	$(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	)
