ifdef UnixPlatform

export:: omniidl.py
	@$(ExportExecutable)

export:: omniidl
	@$(ExportExecutable)
endif
