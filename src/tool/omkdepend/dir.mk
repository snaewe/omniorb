MYSRCS = include.c main.c parse.c pr.c cppsetup.c ifparser.c gnuwin32.c
OBJS = $(MYSRCS:.c=.o)

DIR_CPPFLAGS = -DWIN32 -DDEBUG

all:: omkdepend.exe

omkdepend.exe: $(OBJS)
	@(libs="advapi32.lib"; $(CExecutable))

export:: omkdepend.exe
	@$(ExportExecutable)

clean::
	$(RM) omkdepend.exe
