SUBDIRS = mklib omniNotify

all::
	@(subdirs="$(SUBDIRS)"; target="export"; $(MakeSubdirs))

export::
	@(subdirs="$(SUBDIRS)"; target="export"; $(MakeSubdirs))

clean::
	@(subdirs="$(SUBDIRS)"; target="clean"; $(MakeSubdirs))

veryclean::
	@(subdirs="$(SUBDIRS)"; target="veryclean"; $(MakeSubdirs))
