SUBDIRS = ast driver fe narrow util omniORB2_be

OBJ_LIBS = \
           driver/$(patsubst %,$(LibPattern),drv) \
           omniORB2_be/$(patsubst %,$(LibPattern),omniORB2_be) \
           fe/$(patsubst %,$(LibPattern),fe) \
           ast/$(patsubst %,$(LibPattern),ast) \
           util/$(patsubst %,$(LibPattern),util) \
           narrow/$(patsubst %,$(LibPattern),narrow)

ifeq ($(CXX),g++)
SUBDIR_MAKEFLAGS = "CXXDEBUGFLAGS=-g -fhandle-exceptions"
else
SUBDIR_MAKEFLAGS = "CXXDEBUGFLAGS=-g"
endif

all::
	@$(MakeSubdirs)

export::
	@(target=all; $(MakeSubdirs))

prog = $(patsubst %,$(BinPattern),omniidl2)

all:: $(prog)

$(prog): $(OBJ_LIBS)
	@(libs="$(OBJ_LIBS)"; $(CXXExecutable))

export:: $(prog)
	@$(ExportExecutable)
