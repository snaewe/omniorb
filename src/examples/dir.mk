#SUBDIRS = echo poa boa thread lifecycle anyExample dii dsi
SUBDIRS = echo poa boa thread anyExample dii dsi call_back


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
