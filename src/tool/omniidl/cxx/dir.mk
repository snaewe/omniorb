IDLMODULE_MAJOR = 0
IDLMODULE_MINOR = 1

OBJS = y.tab.o lex.yy.o idlerr.o idlutil.o idltype.o \
	idlrepoId.o idlscope.o idlexpr.o idlast.o idlvalidate.o \
	idldump.o

PYOBJS = idlpython.o

CXXSRCS = y.tab.cc lex.yy.cc idlerr.cc idlutil.cc idltype.cc \
	idlrepoId.cc idlscope.cc idlexpr.cc idlast.cc idlvalidate.cc \
	idldump.cc idlpython.cc idlc.cc

YYSRC = idl.yy
LLSRC = idl.ll

FLEX = flex -t
BISON = bison -d -v -o y.tab.c

idlc = $(patsubst %,$(BinPattern),idlc)

CXXDEBUGFLAGS = -g

all:: $(idlc)

$(idlc): $(OBJS) idlc.o
	@($(CXXExecutable))

y.tab.h y.tab.cc: $(YYSRC)
	@-$(RM) $@
	$(BISON) $<
	mv -f y.tab.c y.tab.cc

lex.yy.cc: $(LLSRC) y.tab.h
	$(FLEX) $< > $@


#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################

ifdef UnixPlatform
CXXDEBUGFLAGS = -g
endif


#############################################################################
#   Make rules for Linux                                                    #
#############################################################################

ifdef Linux

CXXOPTIONS += -fpic

DIR_CPPFLAGS += -DCPP_LOCATION="\"/lib/cpp\"" \
                -DCPP_FLAGS="\"--lang-c++ -undef\""

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

all:: $(lib)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
	$(RM) $@; \
	$(CXXLINK) $(CXXLINKOPTIONS) -shared -o $@ -Wl,-soname,$(soname) $(IMPORT_LIBRARY_FLAGS) \
	 $(filter-out $(LibSuffixPattern),$^) $(LIBS)\
	)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )

clean::
	$(RM) $(lib)

endif


#############################################################################
#   Make rules for Solaris 2.x                                              #
#############################################################################

ifdef SunOS

CXXOPTIONS += -Kpic -I/usr/local/include

DIR_CPPFLAGS += -DCPP_LOCATION="\"$(CPP)\"" -DCPP_FLAGS="\"-B -undef\""

libname = _omniidlmodule.so
soname = $(libname).$(IDLMODULE_MAJOR)
lib = $(soname).$(IDLMODULE_MINOR)

$(lib): $(OBJS) $(PYOBJS)
	(set -x; \
	$(RM) $@; \
        $(CXX) -ptv -G -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) -lposix4 -lnsl -lC \
	)

all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)
	@(set -x; \
          cd $(EXPORT_TREE)/$(LIBDIR); \
          $(RM) $(soname); \
          ln -s $(lib) $(soname); \
          $(RM) $(libname); \
          ln -s $(soname) $(libname); \
         )
endif
