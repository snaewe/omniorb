PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl_be/cxx
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl_be/cxx

SUBDIRS = header skel dynskel impl

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

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif

FILES = __init__.py util.py skutil.py id.py types.py config.py output.py \
        ast.py iface.py call.py cxx.py support.py descriptor.py

export:: $(FILES)
	@(dir="$(PYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(PYLIBDIR); \
          cd ..; \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('cxx')"; \
	 )

ifdef INSTALLTARGET
install:: $(FILES)
	@(dir="$(INSTALLPYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
          cd $(INSTALLPYLIBDIR); \
          cd ..; \
	  $(PYTHON) -c "import compileall; compileall.compile_dir('cxx')"; \
	 )
endif
