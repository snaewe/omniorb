ifdef UnixPlatform

export:: omniidlrun.py
	@$(ExportExecutable)

export:: omniidl
	@$(ExportExecutable)
endif
