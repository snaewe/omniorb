#
# Build a shared library in this directory
# **** DO NOT forget to update the $(VERSION) number.
#

# The Version number is constructed as follows:
#    <major version no.>.<minor version no.>.<micro version no.>
#
# The <major version no.> is always 2 for omniORB2.
#
# The <minor version no.> changes when:
#   1. Public interfaces have been extended but remains backward compatible
#      with earlier minor version.
#   2. Internal interfaces have been changed.
#
# The <micro version no.> changes when the implementation has been changed
# but both the public and internal interfaces remain the same. This usually
# corresponds to a pure bug fix release.
#
# 
VERSION = 1.1.0
#
#
#
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))
vpath %.cc ../..

#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

#
# LIBRARY_OPTIONS, enable library features
# For meaning of these options, see the comments in Makefile.orig
LIBRARY_OPTIONS = \
               -DLIBC_CALLS_STRTOK \
               -DGETPEERNAME_BUG \
               -DBROKEN_FGETS \
               -DSOLARIS_24_GETHOSTBYNAME_BUG \
               -DHOSTS_DENY=\"/etc/hosts.deny\" \
               -DHOSTS_ALLOW=\"/etc/hosts.allow\" \
               -DRFC931_TIMEOUT=10

DIR_CPPFLAGS = -DHOSTS_ACCESS $(LIBRARY_OPTIONS)

SRCS = hosts_access.c options.c shell_cmd.c rfc931.c eval.c \
       hosts_ctl.c refuse.c percent_x.c clean_exit.c $(AUX_SRCS) \
       fix_options.c socket.c workarounds.c \
       update.c misc.c diag.c percent_m.c environ.c fakelog2.c

CXXSRCS = gatekeeper.cc

OBJS = hosts_access.o options.o shell_cmd.o rfc931.o eval.o \
       hosts_ctl.o refuse.o percent_x.o clean_exit.o $(AUX_OBJS) \
       fix_options.o socket.o workarounds.o \
       update.o misc.o diag.o percent_m.o environ.o fakelog2.o

CXXOBJS = gatekeeper.o

DIR_CPPFLAGS += -I. $(patsubst %,-I.,$(VPATH)) \
                -I../.. $(patsubst %,-I%/../..,$(VPATH))

major_version = $(word 1,$(subst ., ,$(VERSION)))
minor_version = $(word 2,$(subst ., ,$(VERSION)))
micro_version = $(word 3,$(subst ., ,$(VERSION)))

#############################################################################
#   Make rules for Solaris 2.x                                              #
#############################################################################

ifdef SunOS
ifeq ($(notdir $(CXX)),CC)

CXXOPTIONS += -Kpic

ifeq ($(notdir $(CC)),gcc)
COPTIONS += -fpic
else
COPTIONS += -Kpic
endif

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

all:: $(lib)

$(lib): $(OBJS) $(CXXOBJS)
	(set -x; \
        $(RM) $@; \
        CC -G -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB); \
       )

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
endif

#############################################################################
#   Make rules for  Digital Unix                                            #
#############################################################################

ifdef OSF1
ifeq ($(notdir $(CXX)),cxx)

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

all:: $(lib)

$(lib): $(OBJS) $(CXXOBJS)
	(rpath="$(RPATH)"; \
         for arg in $(OMNITHREAD_LIB) /usr/lib/cmplrs/cxx; do \
         if expr "$$arg" : "-L" >/dev/null; then \
               rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
         fi; \
         done; \set -x; \
         $(RM) $@; \
         ld -shared -soname $(soname) -set_version $(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         -expect_unresolved '*' \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB) -lcxxstd -lcxx -lexc -lots -lc -rpath $$rpath; \
        )

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
endif
