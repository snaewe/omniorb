
OBJS  = cexp.o cccp.o config.o alloca.o
CSRCS = cexp.c cccp.c config.c alloca.c

ifdef Win32Platform
OBJS  += index.o
CSRCS += index.c
endif

omni-cpp = $(patsubst %,$(BinPattern),omni-cpp)

all:: $(omni-cpp)


ifdef Win32Platform

export:: $(omni-cpp)
	$(ExportExecutable)

else

export:: $(omni-cpp)
	@(dir="$(EXPORT_TREE)/$(LIBDIR)"; file="$(omni-cpp)"; \
	$(ExportExecutableFileToDir))

endif

$(omni-cpp): $(OBJS)
	@(libs=""; $(CExecutable))
