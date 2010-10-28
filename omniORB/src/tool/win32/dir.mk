all:: clwrapper.exe libwrapper.exe linkwrapper.exe oidlwrapper.exe

define CompileWrapper
cat $< >tmp.c
CL.EXE tmp.c advapi32.lib
mv tmp.exe $@
rm tmp.obj tmp.c
endef

clwrapper.exe: clwrapper.c
	$(CompileWrapper)

libwrapper.exe: libwrapper.c
	$(CompileWrapper)

linkwrapper.exe: linkwrapper.c
	$(CompileWrapper)

oidlwrapper.exe: oidlwrapper.c
	$(CompileWrapper)

export:: clwrapper.exe libwrapper.exe linkwrapper.exe oidlwrapper.exe
	@$(ExportExecutable)
