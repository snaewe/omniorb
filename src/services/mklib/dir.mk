include cosinterfaces.mk
include libdefs.mk

# Look for .idl files in <top>/idl plus ../idl
vpath %.idl $(IMPORT_TREES:%=%/idl) $(VPATH:%=%/../idl)

DIR_IDLFLAGS += -I. $(patsubst %,-I%/../idl,$(VPATH)) \
                   $(patsubst %,-I%/idl,$(IMPORT_TREES))

VERSION  = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))

sklib    = $(patsubst %,$(LibPattern),COS$(VERSION))
dynsklib = $(patsubst %,$(LibPattern),COSdynamic$(VERSION))

all:: $(sklib) $(dynsklib)

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)
COS_SK_SRCS = $(COS_INTERFACES:%=%SK.cc)

COS_DYNSK_OBJS = $(COS_INTERFACES:%=%DynSK.o)
COS_DYNSK_SRCS = $(COS_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(COS_DYNSK_SRCS) $(COS_SK_SRCS) 

$(sklib): $(COS_SK_OBJS)
	@$(StaticLinkLibrary)

export:: $(sklib)
	@$(ExportLibrary)

$(dynsklib): $(COS_DYNSK_OBJS)
	@$(StaticLinkLibrary)

export:: $(dynsklib)
	@$(ExportLibrary)

export:: $(COS_INTERFACES:%=%.hh)
	@(for i in $^; do \
            file="$$i"; \
            dir="$(EXPORT_TREE)/$(INCDIR)/COS"; \
		$(ExportFileToDir) \
          done )

##############################################################################
# Also build in subdirs
##############################################################################
SUBDIRS = sharedlib mkBOAlib 

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

