CXXSRCS = diner.cc prio.cc thrspecdata.cc prodcons.cc

DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)

diner       = $(patsubst %,$(BinPattern),diner)
prio        = $(patsubst %,$(BinPattern),prio)
thrspecdata = $(patsubst %,$(BinPattern),thrspecdata)
prodcons    = $(patsubst %,$(BinPattern),prodcons)

all:: $(diner) $(prio) $(thrspecdata) $(prodcons)

clean::
	$(RM) $(diner) $(prio) $(thrspecdata) $(prodcons)

export: $(diner) $(prio) $(thrspecdata) $(prodcons)
	@(module="threadtests"; $(ExportExecutable))


$(diner): diner.o $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(OMNITHREAD_LIB)"; $(CXXExecutable))

$(prio): prio.o $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(OMNITHREAD_LIB)"; $(CXXExecutable))

$(thrspecdata): thrspecdata.o $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(OMNITHREAD_LIB)"; $(CXXExecutable))

$(prodcons): prodcons.o $(OMNITHREAD_LIB_DEPEND)
	@(libs="$(OMNITHREAD_LIB)"; $(CXXExecutable))
