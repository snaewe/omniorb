UPTO_SERVICES_TOP = ..

include cosinterfaces.mk
include orbinterfaces.mk

lib    = $(patsubst %,$(LibPattern),cos)

include libdefs.mk

##############################################################################
# Also build in subdirs
##############################################################################
SUBDIRS = mkBOAlib # mkdynamiclib mkBOAdynamiclib

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

clean::
	@$(MakeSubdirs)

veryclean::
	@$(MakeSubdirs)

