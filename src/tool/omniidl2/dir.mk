SUBDIRS = ast driver fe narrow util omniORB2_be

ifdef NTArchitecture
OBJ_LIBS = \
           omniORB2_be/$(patsubst %,$(LibPattern),omniORB2_be) \
           fe/$(patsubst %,$(LibPattern),fe) \
           ast/$(patsubst %,$(LibPattern),ast) \
           util/$(patsubst %,$(LibPattern),util) \
           narrow/$(patsubst %,$(LibPattern),narrow)
else
OBJ_LIBS = \
           driver/$(patsubst %,$(LibPattern),drv) \
           omniORB2_be/$(patsubst %,$(LibPattern),omniORB2_be) \
           fe/$(patsubst %,$(LibPattern),fe) \
           ast/$(patsubst %,$(LibPattern),ast) \
           util/$(patsubst %,$(LibPattern),util) \
           narrow/$(patsubst %,$(LibPattern),narrow)
endif

all::
	@$(MakeSubdirs)

export::
	@(target=all; $(MakeSubdirs))

prog = $(patsubst %,$(BinPattern),omniidl2)

all:: $(prog)

ifdef NTArchitecture
DRV_OBJS = \
                driver/drv_init.o \
                driver/drv_private.o \
                driver/drv_main.o \
                driver/drv_args.o \
                driver/drv_fork.o \
                driver/drv_link.o \
                driver/drv_preproc.o

$(prog): $(DRV_OBJS) $(OBJ_LIBS) 
	@(libs="advapi32.lib $(OBJ_LIBS)"; $(CXXExecutable))
else
$(prog): $(OBJ_LIBS)
	@(libs="$(OBJ_LIBS)"; $(CXXExecutable))
endif

export:: $(prog)
	@$(ExportExecutable)
