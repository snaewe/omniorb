all:: clwrapper.exe libwrapper.exe linkwrapper.exe \
      oidl2wrapper.exe oidlwrapper.exe omake.exe

define CompileWrapper
cat $< >tmp.c
CL.EXE tmp.c advapi32.lib
$(MV) tmp.exe $@
$(RM) tmp.obj tmp.c
endef

clwrapper.exe: clwrapper.c
	$(CompileWrapper)

libwrapper.exe: libwrapper.c
	$(CompileWrapper)

linkwrapper.exe: linkwrapper.c
	$(CompileWrapper)

oidl2wrapper.exe: oidl2wrapper.c
	$(CompileWrapper)

oidlwrapper.exe: oidlwrapper.c
	$(CompileWrapper)

omake.exe: omake.c
	gcc $< -o omake.exe

export:: clwrapper.exe libwrapper.exe linkwrapper.exe \
	 oidl2wrapper.exe oidlwrapper.exe omake.exe
	@$(ExportExecutable)
