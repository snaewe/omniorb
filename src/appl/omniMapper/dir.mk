CONFIG_FILE_ENV_VAR = \"OMNIMAPPER_CONFIG\"

ifdef Win32Platform
CONFIG_FILE_DEFAULT = \"C:\\\\omniMapper.cfg\"
else
CONFIG_FILE_DEFAULT = \"/etc/omniMapper.cfg\"
endif


CXXSRCS = omniMapper.cc

#CXXDEBUGFLAGS = -g

DIR_CPPFLAGS = $(CORBA_CPPFLAGS) \
               -DCONFIG_FILE_ENV_VAR=$(CONFIG_FILE_ENV_VAR) \
               -DCONFIG_FILE_DEFAULT=$(CONFIG_FILE_DEFAULT)

omniMapper = $(patsubst %,$(BinPattern),omniMapper)

all:: $(omniMapper)

clean::
	$(RM) $(omniMapper)

export:: $(omniMapper)
	@$(ExportExecutable)

$(omniMapper): omniMapper.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB_NODYN)"; $(CXXExecutable))
