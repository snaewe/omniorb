PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl_be/cxx/dynskel
INSTALLPYLIBDIR = $(INSTALLPYTHONDIR)/omniidl_be/cxx/dynskel

FILES = __init__.py main.py typecode.py tcstring.py template.py

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
