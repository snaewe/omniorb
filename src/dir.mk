# Check for Python, and complain early.
ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif

ifndef EmbeddedSystem
SUBDIRS += tool
endif

SUBDIRS += lib

ifndef EmbeddedSystem
SUBDIRS += appl services
endif

all::
	@echo
	@echo 'No "all" rule here.  If you want to build and export everything'
	@echo 'use "gnumake export".  Otherwise build subdirectories separately.'
	@echo

export::
	@$(MakeSubdirs)

clean::
	rm -rf stub
