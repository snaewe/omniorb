SUBDIRS += tool

SUBDIRS += lib appl

all::
	@echo
	@echo 'No "all" rule here.  If you want to build and export everything'
	@echo 'use "gnumake export".  Otherwise build subdirectories separately.'
	@echo

export::
	@$(MakeSubdirs)

clean::
	rm -rf stub
