PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl

ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif

FILES = __init__.py main.py idlast.py idltype.py idlutil.py idlvisitor.py \
        idlstring.py output.py

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
