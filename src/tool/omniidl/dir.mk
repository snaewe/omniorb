SUBDIRS = cxx python

ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)
