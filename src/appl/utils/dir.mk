ifndef ATMosArchitecture 
SUBDIRS = genior catior nameclt
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
