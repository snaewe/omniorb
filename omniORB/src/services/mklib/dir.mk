include $(MAKEFILE_INC_DIR)cosinterfaces.mk
include $(MAKEFILE_INC_DIR)libdefs.mk

# Uncomment the next line to build BOA versions of the COS libraries
# BUILD_BOA_COS_LIB = 1


# Look for .idl files in <top>/idl/COS
vpath %.idl $(IMPORT_TREES:%=%/idl/COS)

DIR_IDLFLAGS +=  -I. $(patsubst %,-I%/idl/COS,$(IMPORT_TREES))

COS_SKLIB_NAME    = COS
COS_DYNSKLIB_NAME = COSDynamic

COS_HEADERS = $(COS_INTERFACES:%=%.hh)

COS_SK_OBJS = $(COS_INTERFACES:%=%SK.o)
COS_SK_SRCS = $(COS_INTERFACES:%=%SK.cc)

COS_DYNSK_OBJS = $(COS_INTERFACES:%=%DynSK.o)
COS_DYNSK_SRCS = $(COS_INTERFACES:%=%DynSK.cc)

CXXSRCS = $(COS_DYNSK_SRCS) $(COS_SK_SRCS) 

all:: $(COS_HEADERS)

export:: $(COS_HEADERS) COS_sysdep.h
	@(dir="$(EXPORT_TREE)/$(INCDIR)/COS"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

ifdef INSTALLTARGET
install:: $(COS_HEADERS) COS_sysdep.h
	@(dir="$(INSTALLINCDIR)/COS"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )
endif

veryclean::
	$(RM) $(COS_INTERFACES:%=%SK.cc) $(COS_INTERFACES:%=%DynSK.cc) \
              $(COS_INTERFACES:%=%.hh)


##############################################################################
# Build Subdirectories
##############################################################################

SUBDIRS = stublib dynstublib

ifdef BUILD_BOA_COS_LIB
SUBDIRS += mkBOAlib
endif

all::
	@$(MakeSubdirs)

export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif
