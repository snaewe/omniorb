
OBJS  = cexp.o cccp.o config.o alloca.o
CSRCS = cexp.c cccp.c config.c alloca.c

ifdef Win32Platform
OBJS  += index.o
CSRCS += index.c
endif

DIR_CPPFLAGS += -DHAVE_CONFIG_H -I.

omni-cpp = $(patsubst %,$(BinPattern),omni-cpp)

all:: $(omni-cpp)


#############################################################################
#   Platform specific things                                                #
#############################################################################

ifdef AIX
DIR_CPPFLAGS += -ma
endif

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
