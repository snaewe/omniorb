OBJS = y.tab.o lex.yy.o idlerr.o idlutil.o idltype.o \
	idlrepoId.o idlscope.o idlexpr.o idlast.o idlvalidate.o \
	idldump.o idlc.o

CXXSRCS = y.tab.cc lex.yy.cc idlerr.cc idlutil.cc idltype.cc \
	idlrepoId.cc idlscope.cc idlexpr.cc idlast.cc idlvalidate.cc \
	idldump.cc idlc.cc

YYSRC = idl.yy
LLSRC = idl.ll

FLEX = flex -t
BISON = bison -d -v -o y.tab.c

idlc = $(patsubst %,$(BinPattern),idlc)

CXXDEBUGFLAGS = -g

all:: $(idlc)

$(idlc): $(OBJS)
	@($(CXXExecutable))


y.tab.h y.tab.cc: $(YYSRC)
	@-$(RM) $@
	$(BISON) $<
	mv -f y.tab.c y.tab.cc

lex.yy.cc: $(LLSRC) y.tab.h
	$(FLEX) $< > $@
