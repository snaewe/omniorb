CXXSRCS = pttwish.cc

DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(OMNIPARTCL_CPPFLAGS)

script = $(patsubst %,$(TclScriptPattern),ptt)

wish = $(patsubst %,$(BinPattern),pttwish)

all:: $(wish) $(script)

$(wish): pttwish.o $(OMNITHREAD_LIB_DEPEND) $(OMNIPARTCL_LIB_DEPEND)
	@(libs="$(OMNITHREAD_LIB) $(OMNIPARTCL_LIB)"; \
	  $(CXXExecutable))

$(script): ptt.tcl
	@(wish="$(wish)"; $(TclScriptExecutable))

export: $(wish) $(script)
	@$(ExportExecutable)

clean::
	$(RM) $(wish) $(script)
