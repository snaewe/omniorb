all:: clwrapper.exe libwrapper.exe linkwrapper.exe omake.exe

define CompileWrapper
cat $< >tmp.c
cl tmp.c advapi32.lib
$(MV) tmp.exe $@
$(RM) tmp.obj tmp.c
endef

clwrapper.exe: clwrapper.c
	$(CompileWrapper)

libwrapper.exe: libwrapper.c
	$(CompileWrapper)

linkwrapper.exe: linkwrapper.c
	$(CompileWrapper)

omake.exe: omake.c
	gcc $< -o omake.exe

export: clwrapper.exe libwrapper.exe linkwrapper.exe omake.exe
	@$(ExportExecutable)
