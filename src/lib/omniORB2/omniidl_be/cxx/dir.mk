PYLIBDIR = $(EXPORT_TREE)/lib/python/omniidl_be/cxx

SUBDIRS = header skel dynskel impl ami

ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: util.py
	@(file="util.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: skutil.py
	@(file="skutil.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: id.py
	@(file="id.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: types.py
	@(file="types.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: config.py
	@(file="config.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: output.py
	@(file="output.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: ast.py
	@(file="ast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: iface.py
	@(file="iface.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: call.py
	@(file="call.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: cxx.py
	@(file="cxx.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: support.py
	@(file="support.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export::
	@(set -x; \
	cd $(PYLIBDIR); \
	$(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	)
