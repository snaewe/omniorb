#SUBDIRS = echo poa boa thread lifecycle anyExample dii dsi
SUBDIRS = echo poa boa thread anyExample dii dsi


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
