
CXXSRCS       = \
		fe_declarator.cc \
		fe_private.cc \
		fe_init.cc \
		fe_extern.cc \
		fe_interface_header.cc


OBJS	      = \
                y.tab.o \
                lex.yy.o \
		fe_declarator.o \
		fe_private.o \
		fe_init.o \
		fe_extern.o \
		fe_interface_header.o

DIR_CPPFLAGS = -I../include $(patsubst %,-I%/../include,$(VPATH))

lib = $(patsubst %,$(LibPattern),fe)

all:: $(lib)

$(lib): $(OBJS)
	@$(StaticLinkLibrary)

#
# We don't seem to be able to regenerate y.tab.cc y.tab.hh and lex.yy.cc
# (at least on OSF)
#
# Create the parser and lexer
#
#y.tab.hh y.tab.cc: idl.yy
#	@-$(RM) $@
#	$(YACC) -d $<
#	mv -f y.tab.c y.tab.cc
#	mv -f y.tab.h y.tab.hh
#
#lex.yy.cc: idl.ll y.tab.hh
#	$(LEX) $(LEXFLAGS) $<
#	mv -f lex.yy.c lex.yy.cc
#
#clean::
#	$(RM) y.tab.cc y.tab.hh y.tab.c y.tab.h lex.yy.c lex.yy.cc
