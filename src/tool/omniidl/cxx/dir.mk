IDLMODULE_MAJOR   = 0
IDLMODULE_MINOR   = 1
IDLMODULE_VERSION = 0x2301# => CORBA 2.3, front-end 0.1

DIR_CPPFLAGS += -DIDLMODULE_VERSION="\"$(IDLMODULE_VERSION)\""

SUBDIRS = cccp
all::
	@$(MakeSubdirs)
export::
	@$(MakeSubdirs)

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
BISON = bison -d -o y.tab.c

idlc = $(patsubst %,$(BinPattern),idlc)

# y.tab.h y.tab.cc: $(YYSRC)
# 	@-$(RM) $@
# 	$(BISON) $<
# 	mv -f y.tab.c y.tab.cc

# lex.yy.cc: $(LLSRC) y.tab.h
# 	$(FLEX) $< | sed -e 's/^#include <unistd.h>//' > $@
# 	echo '#ifdef __VMS' >> $@
# 	echo '// Some versions of DEC C++ for OpenVMS set the module name used by the' >> $@
# 	echo '// librarian based on the last #line encountered.' >> $@
# 	echo '#line' `cat $@ | wc -l` '"lex_yy.cc"' >> $@
# 	echo '#endif' >> $@


#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################

ifdef UnixPlatform
CXXDEBUGFLAGS = -g
DIR_CPPFLAGS += -I/usr/local/include/python1.5 -I/usr/include/python1.5
endif


#############################################################################
#   Make rules for Linux                                                    #
#############################################################################

ifdef Linux

CXXOPTIONS += -fpic

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



#############################################################################
#   Make rules for Windows                                                  #
#############################################################################

ifdef Win32Platform

DIR_CPPFLAGS += -DMSDOS

DIR_CPPFLAGS += -I"c:\progra~1/Python/include"
CXXLINKOPTIONS += -libpath:"c:\progra~1\Python\libs"

omniidl = $(patsubst %,$(BinPattern),omniidl)

all:: $(omniidl)

export:: $(omniidl)
	@$(ExportExecutable)

clean::
	$(RM) $(omniidl)

$(omniidl): $(OBJS) $(PYOBJS)
	@(libs="python15.lib"; $(CXXExecutable))

endif



#############################################################################
#   Make rules for AIX                                                      #
#############################################################################

ifdef AIX

CXXOPTIONS += -I. -I/usr/local/include

lib = _omniidlmodule.so
libinit = init_omniidl
py_exp = /usr/local/lib/python1.5/config/python.exp

ifeq ($(notdir $(CXX)),xlC_r)

$(lib): $(OBJS) $(PYOBJS)
	@(set -x; \
	$(RM) $@; \
	$(MAKECPPSHAREDLIB) \
	     -o $(lib) \
	     -bI:$(py_exp) \
	     -n $(libinit) \
	     $(IMPORT_LIBRARY_FLAGS) \
	     -bhalt:4 -T512 -H512 \
	     $(filter-out $(LibSuffixPattern),$^) \
	     -p 40 \
	 ; \
       )

endif

all:: $(lib)

clean::
	$(RM) $(lib)

export:: $(lib)
	@$(ExportLibrary)

endif



#############################################################################
#   Test executable                                                         #
#############################################################################

all:: $(idlc)

$(idlc): $(OBJS) idlc.o
	@(libs=""; $(CXXExecutable))
