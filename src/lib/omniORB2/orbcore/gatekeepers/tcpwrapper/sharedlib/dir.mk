#
# Build a shared library in this directory
# **** DO NOT forget to update the $(VERSION) number.
#

# The Version number is constructed as follows:
#    <major version no.>.<minor version no.>.<micro version no.>
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
VERSION = 1.2.0
#
#
#
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))
vpath %.cc ..
vpath %.c ..

#############################################################################
#   Make variables common to all platforms                                  #
#############################################################################

#
# LIBRARY_OPTIONS, enable library features
# For meaning of these options, see the comments in Makefile.orig
LIBRARY_OPTIONS = \
               -DLIBC_CALLS_STRTOK \
               -DBROKEN_FGETS \
               -DHOSTS_DENY=\"/etc/hosts.deny\" \
               -DHOSTS_ALLOW=\"/etc/hosts.allow\" \
               -DRFC931_TIMEOUT=10

ifndef OSR5
ifndef IRIX
LIBRARY_OPTIONS += -DSOLARIS_24_GETHOSTBYNAME_BUG
endif
endif

ifndef AIX
LIBRARY_OPTIONS += -DGETPEERNAME_BUG
endif

DIR_CPPFLAGS = -DHOSTS_ACCESS $(LIBRARY_OPTIONS) -D_REENTRANT

SRCS = hosts_access.c options.c shell_cmd.c rfc931.c eval.c \
       hosts_ctl.c refuse.c percent_x.c clean_exit.c $(AUX_SRCS) \
       fix_options.c socket.c workarounds.c \
       update.c misc.c diag.c percent_m.c setenv.c fakelog2.c
#      update.c misc.c diag.c percent_m.c environ.c fakelog2.c

CXXSRCS = gatekeeper.cc

OBJS = hosts_access.o options.o shell_cmd.o rfc931.o eval.o \
       hosts_ctl.o refuse.o percent_x.o clean_exit.o $(AUX_OBJS) \
       fix_options.o socket.o workarounds.o \
       update.o misc.o diag.o percent_m.o setenv.o fakelog2.o
#      update.o misc.o diag.o percent_m.o environ.o fakelog2.o

CXXOBJS = gatekeeper.o

DIR_CPPFLAGS += -I.. $(patsubst %,-I%,$(VPATH)) \
                -I../../.. $(patsubst %,-I%/../..,$(VPATH)) \
                -I../../../.. $(patsubst %,-I%/../../..,$(VPATH))

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

ifeq ($(notdir $(CXX)),g++)

CXXOPTIONS += -fpic
COPTIONS   += -fpic

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

all:: $(lib)

$(lib): $(OBJS) $(CXXOBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -Wl,-h,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
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

#############################################################################
#   Make rules for  Linux egcs                                              #
#############################################################################

ifdef Linux
ifdef EgcsMajorVersion

DIR_CPPFLAGS += -fPIC

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS) $(CXXOBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) $(CXXOPTIONS) -shared -Wl,-soname,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) $(OMNITHREAD_LIB); \
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
endif


#############################################################################
#   Make rules for SGI Irix 6.2                                             #
#############################################################################

ifdef IRIX
ifeq ($(notdir $(CXX)),CC)

DIR_CPPFLAGS += -KPIC

ifdef IRIX_n32
ADD_CPPFLAGS = -n32
endif
ifdef IRIX_64
ADD_CPPFLAGS = -64
endif

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS) $(CXXOBJS)
	(set -x; \
        $(RM) $@; \
        $(LINK.cc) -KPIC -shared -Wl,-h,$(libname) -Wl,-set_version,$(soname) \
         -Wl,-rpath,$(LIBDIR) -o $@ $(IMPORT_LIBRARY_FLAGS) $^ $(LDLIBS); \
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
endif

#############################################################################
#   Make rules for FreeBSD 3.x egcs                                         #
#############################################################################

ifdef FreeBSD
ifdef EgcsMajorVersion

DIR_CPPFLAGS += -fPIC

libname = libtcpwrapGK.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS) $(CXXOBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) $(CXXOPTIONS) -shared -Wl,-soname,$(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^);  \
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
endif

