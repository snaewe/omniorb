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


ifeq ($(CXX),g++)
DIR_CPPFLAGS += -fpic

SharedLibPattern = lib%.so.2
define SharedLibrary
(set -x; \
 $(RM) $@; \
  g++  -shared -Wl,-soname,$@ -o $@ $(IMPORT_LIBRARY_FLAGS) \
    $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS)) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -lpthreads; \
)
endef
endif

ifeq ($(CXX),CC)

DIR_CPPFLAGS += -Kpic

SharedLibPattern = lib%.so.2
define SharedLibrary
(set -x; \
 $(RM) $@; \
  CC -G -o $@ -h $@ $(IMPORT_LIBRARY_FLAGS) \
    $(patsubst %,-R %,$(IMPORT_LIBRARY_DIRS)) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -lpthread -lposix4; \
)
endef
endif

ifeq ($(CXX),/usr/bin/cxx)

SharedLibPattern = lib%.so

define SharedLibrary
(rpath="$(RPATH)"; \
 for arg in $$libs /usr/lib/cmplrs/cxx; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 ld -shared -set_version $@.2.0  -o $@ $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -lpthreads -lmach -lc_r -lcxxstd -lcxx -lexc -lots -lc -rpath $$rpath; \
)
endef
endif

ifdef NTArchitecture

SharedLibPattern = %_rt.dll
define SharedLibrary
( set -x; \
 $(RM) $@; \
 $(CXXLINK) -out:$@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $^ $$libs; \
)
endef
endif

lib = $(patsubst %,$(SharedLibPattern),omnithread)

ifneq ($(CXX),g++)

all:: $(lib)

$(lib): $(OBJS)
	@$(SharedLibrary)

ifdef NTArchitecture
clean::
	$(RM) $(lib) omnithread_rt.exp
else
clean::
	$(RM) $(lib)
endif

export:: $(lib)
	@$(ExportLibrary)

ifdef NTArchitecture
export:: omnithread_rt.lib
	@$(ExportLibrary)
endif

endif
