include ../cosinterfaces.mk
include ../libdefs.mk

# Generate BOA skeleton
DIR_IDLFLAGS += -WbBOA

# Look for .idl files in <top>/idl plus ../../idl
vpath %.idl $(IMPORT_TREES:%=%/idl) $(VPATH:%=%/../../idl)

DIR_IDLFLAGS += -I. $(patsubst %,-I%/../../idl,$(VPATH)) \
                   $(patsubst %,-I%/idl,$(IMPORT_TREES))

sklib    = $(patsubst %,$(LibPattern),COS_BOA)

all:: $(sklib)

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)

CXXSRCS = $(COS_SK_SRCS) 

$(sklib): $(COS_SK_OBJS)
	@$(StaticLinkLibrary)

export:: $(sklib)
	@$(ExportLibrary)

export:: $(COS_INTERFACES:%=%.hh)
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/COS/BOA"; \
		$(ExportFileToDir) \
          done )

##############################################################################
# Also build in subdirs
##############################################################################
#SUBDIRS = sharedlib

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

clean::
	@$(MakeSubdirs)

veryclean::
	@$(MakeSubdirs)


##############################################################################
clean::
	$(RM) $(lib)

veryclean::
	$(RM) $(lib) \
               $(COS_INTERFACES:%=%SK.cc) \
               $(COS_INTERFACES:%=%DynSK.cc) \
               $(COS_INTERFACES:%=%.hh)


