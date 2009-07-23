PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl_be
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl_be

ifeq ($(PYTHON),)
PYTHON = python
endif

FILES = __init__.py python.py

export:: $(FILES)
	@(dir="$(PYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(PYLIBDIR); \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	 )

ifdef INSTALLTARGET
install:: $(FILES)
	@(dir="$(INSTALLPYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(INSTALLPYLIBDIR); \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('.')"; \
	 )
endif
