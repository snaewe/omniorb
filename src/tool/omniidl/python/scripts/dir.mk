ifdef UnixPlatform

export:: omniidl omniidlrun.py
	@$(ExportExecutable)

ifdef INSTALLTARGET
install:: omniidl omniidlrun.py
	@$(InstallExecutable)
endif

endif
