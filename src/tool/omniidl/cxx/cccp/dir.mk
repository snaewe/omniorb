
OBJS  = cexp.o cccp.o config.o alloca.o
CSRCS = cexp.c cccp.c config.c alloca.c

ifdef Win32Platform
OBJS  += index.o
CSRCS += index.c
endif

DIR_CPPFLAGS += -DHAVE_CONFIG_H -I.

omnicpp = $(patsubst %,$(BinPattern),omnicpp)

all:: $(omnicpp)


#############################################################################
#   Platform specific things                                                #
#############################################################################

ifdef AIX
ifeq ($(notdir $(CXX)),xlC_r)
DIR_CPPFLAGS += -ma
endif
endif

ifdef Win32Platform

export:: $(omnicpp)
	$(ExportExecutable)

else

export:: $(omnicpp)
	@(dir="$(EXPORT_TREE)/$(LIBDIR)"; file="$(omnicpp)"; \
	$(ExportExecutableFileToDir))

ifdef INSTALLTARGET
install:: $(omnicpp)
	@$(InstallExecutable)
endif

endif


$(omnicpp): $(OBJS)
	@(libs=""; $(CExecutable))


veryclean::
	$(RM) $(omnicpp)
