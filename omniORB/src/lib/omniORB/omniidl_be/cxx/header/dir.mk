PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl_be/cxx/header
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl_be/cxx/header

FILES = __init__.py defs.py opers.py poa.py tie.py forward.py marshal.py \
        tcstring.py obv.py template.py

export:: $(FILES)
	@(dir="$(PYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
	 )

ifdef INSTALLTARGET
install:: $(FILES)
	@(dir="$(INSTALLPYLIBDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done; \
	 )
endif
