PYBINDIR = $(EXPORT_TREE)/bin/scripts
PYLIBROOT= $(EXPORT_TREE)/lib/python
PYLIBDIR = $(PYLIBROOT)/omniidl

PAR = $(PYBINDIR)/par

PYSTDLIBS = \\.os \\.dospath \\.macpath \\.pcpath \\.ntpath      \
            \\.posixpath \\.stat \\.UserDict \\.getopt \\.string \
            \\.re \\.types \\.keyword

PYLIBS = $(PYSTDLIBS) \\.omniidl.*

SUBDIRS = be arch

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef UnixPlatform
export:: omniidl
	@(file="omniidl"; dir="$(PYBINDIR)"; $(ExportExecutableFileToDir))
endif

export:: __init__.py
	@(file="__init__.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: main.py
	@(file="main.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlast.py
	@(file="idlast.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idltype.py
	@(file="idltype.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlutil.py
	@(file="idlutil.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: idlvisitor.py
	@(file="idlvisitor.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

export:: output.py
	@(file="output.py"; dir="$(PYLIBDIR)"; $(ExportFileToDir))

# export::
# 	$(PAR) -r $(PYLIBROOT)/omni.par $(PYLIBS)
