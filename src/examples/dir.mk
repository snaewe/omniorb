SUBDIRS = echo thread lifecycle anyExample dii dsi

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
