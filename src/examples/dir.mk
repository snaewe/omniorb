SUBDIRS = echo poa boa thread lifecycle anyExample dii dsi


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
