# dir.mk for omnithread shared library
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
# For each source tree we want to search the parent directory to get source
# files so we put these on VPATH.  However we can't put the parent build
# directory ('..') on VPATH otherwise the .o files in there are used.  So we
# need to do that as a special case with the vpath directive:
#

override VPATH := $(patsubst %,%/..,$(VPATH))
vpath %.cc ..

ifeq ($(ThreadSystem),Solaris)
CXXSRCS = solaris.cc
OBJS = solaris.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)
endif

ifeq ($(ThreadSystem),Posix)
CXXSRCS = posix.cc
OBJS = posix.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS) $(OMNITHREAD_POSIX_CPPFLAGS)
endif

ifeq ($(ThreadSystem),NT)
CXXSRCS = nt.cc
OBJS = nt.o
DIR_CPPFLAGS = $(OMNITHREAD_CPPFLAGS)  -D"_X86_" -D "_OMNITHREAD_DLL"
DIR_CPPFLAGS +=  -D"NDEBUG"  -D"_WINDOWS"
CXXOPTIONS += -MD -W3 -GX -O2
CXXLINKOPTIONS += -DLL -IMPLIB:"omnithread_rt.lib"
endif




major_version = $(word 1,$(subst ., ,$(VERSION)))
minor_version = $(word 2,$(subst ., ,$(VERSION)))
micro_version = $(word 3,$(subst ., ,$(VERSION)))

ifeq ($(notdir $(CXX)),CC)

#CXXDEBUGFLAGS = -g

DIR_CPPFLAGS += -Kpic

libname = libomnithread.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS)
	(set -x; \
        $(RM) $@; \
        CC -G -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) -lpthread -lposix4; \
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

ifeq ($(notdir $(CXX)),cxx)

#CXXDEBUGFLAGS = 

libname = libomnithread.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS)
	(rpath="$(RPATH)"; \
         for arg in $(OMNITHREAD_LIB) /usr/lib/cmplrs/cxx; do \
         if expr "$$arg" : "-L" >/dev/null; then \
               rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
         fi; \
         done; \set -x; \
         $(RM) $@; \
         ld -shared -soname $(soname) -set_version $(soname) -o $@ $(IMPORT_LIBRARY_FLAGS) \
         $(filter-out $(LibSuffixPattern),$^) -lpthreads -lmach -lc_r  -lcxxstd -lcxx -lexc -lots -lc -rpath $$rpath; \
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

ifeq ($(notdir $(CXX)),g++)

ifdef BuildSharedLib

DIR_CPPFLAGS += -fpic

libname = libomnithread.so
soname  = $(libname).$(minor_version)
lib = $(soname).$(micro_version)

$(lib): $(OBJS)
	(set -x; \
        $(RM) $@; \
        $(CXX) -shared -o $@ -h $(soname) $(IMPORT_LIBRARY_FLAGS) \
         $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
         $(filter-out $(LibSuffixPattern),$^) -lpthread -lposix4; \
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


ifdef Win32Platform

SharedLibPattern = %_rt.dll
define SharedLibrary
( set -x; \
 $(RM) $@; \
 $(CXXLINK) -out:$@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $^ $$libs; \
)
endef

lib = $(patsubst %,$(SharedLibPattern),omnithread)

all:: $(lib)

$(lib): $(OBJS)
	@$(SharedLibrary)

clean::
	$(RM) $(lib)

# NT treats DLLs more like executables -- the .dll file needs to go in the
# bin/x86... directory so that it's on your PATH:
export:: $(lib)
	@$(ExportExecutable)

clean::
	$(RM) omnithread_rt.exp

export:: omnithread_rt.lib
	@$(ExportLibrary)

endif
