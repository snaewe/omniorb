all::
	@echo
	@echo 'No "all" rule here.  Do "omake export" to export ATMos interface files.'
	@echo

export::
	@(packages="omniORB diner calc"; \
	  $(ExportATMosInterfaceFiles))
