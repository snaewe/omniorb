ifndef ATMosArchitecture 
SUBDIRS = genior catior convertior nameclt
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
