MYSRCS = include.c main.c parse.c pr.c cppsetup.c ifparser.c

DIR_CPPFLAGS = -DDEBUG

ifdef Win32Platform
MYSRCS += gnuwin32.c
LIBS = advapi32.lib
DIR_CPPFLAGS += -DWIN32
endif

OBJS = $(MYSRCS:.c=.o)

prog = $(patsubst %,$(BinPattern),omkdepend)

all:: $(prog)

$(prog): $(OBJS)
	@(libs="$(LIBS)"; $(CExecutable))

export:: $(prog)
	@$(ExportExecutable)

ifdef INSTALLTARGET
install:: $(prog)
	@$(InstallExecutable)
endif

clean::
	$(RM) $(prog)
