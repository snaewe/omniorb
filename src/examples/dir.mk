#SUBDIRS = echo thread lifecycle anyExample dii dsi
SUBDIRS = echo thread anyExample dii dsi

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
