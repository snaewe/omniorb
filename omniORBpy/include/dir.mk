INSTALLDIR = $(EXPORT_TREE)/include

HEADERS = omniORBpy.h

export:: $(HEADERS)
	@(dir="$(INSTALLDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

all::
