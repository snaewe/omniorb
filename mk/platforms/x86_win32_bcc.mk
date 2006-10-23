#
# x86_nt_4.0.mk - make variables and rules specific to Windows NT 4.0.
#

WindowsNT = 1
x86Processor = 1

#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk

# No -D__WIN32__ as it is predefined in bcc32.
#
IMPORT_CPPFLAGS += -D__x86__ -D__NT__ -D__OSVERSION__=4


# Borland specific stuff
BCLIBDIR=/progra~1/bc5/lib
BCSTDLIBS=$(BCLIBDIR)/import32 $(BCLIBDIR)/cw32mt

LibSuffixPattern = %.lib

AR            = tlibwrapper

CPP           = cpp32

CXX           = bccwrapper
CXXMAKEDEPEND = omkdepend -D__cplusplus -D__BCPLUSPLUS__
CXXLINK       = tlinkwrapper
CXXOPTIONS    = -P -tWM
#
# -P   = ignore the file extension, treat the source file as C++
# -tWM = multi-threaded 
#
CXXDEBUGFLAGS = -w-inl -w-pia -w-ccc -w-rch -w-par -w-lvc -w-bei -w-nst

CXXLINKOPTIONS = -Tpe -ap $(BCLIBDIR)/c0x32
#
# tlink32 [@respfile][option] startup myobjs, [exe], [mapfile], [libraries], 
#         [deffile], [resfile]
#
# Link options
#      -Tpe           link for 32-bit protected-mode Windows .EXE file.
#
#      -ap         link for 32-bit console application
# or   -aa         link for 32-bit window GUI application
#
# startup = startup code
#
#       $(BCLIBDIR)/c0x32   for 32-bit console application
# or    $(BCLIBDIR)/c0w32   for 32-bit window GUI application
#
# libraries
#       $(BCLIBDIR)/import32 -  Windows import library
#
#       $(BCLIBDIR)/cw32mt   -  Multi-threaded runtime DLL
#  or   $(BCLIBDIR)/cw32mti  -  Multi-threaded runtime static lib
#
#  or   $(BCLIBDIR)/cw32     -  single-threaded runtime DLL (do not use this
#                               with -tWM
#  or   $(BCLIBDIR)/cw32i    -  singled-threaded runtime static lib
#

CC            = bccwrapper
CMAKEDEPEND   = omkdepend -D__BORLANDC__
CLINK         = bccwrapper
COPTIONS      = -tWM
CDEBUGFLAGS   =

%.o: %.c
	$(CC) -c $(CFLAGS) -o$@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o$@ $<


RPATH = $(subst $(space),;,$(strip $(IMPORT_LIBRARY_DIRS)))

define CXXExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+;}`expr $$arg : '-L\(.*\)'`"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) $(CXXLINKOPTIONS) \
            $(filter-out $(LibSuffixPattern),$^), \
            $@, \
              , \
            -L$$rpath $(patsubst $(LibPattern),%,$$libs) $(BCSTDLIBS) ; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@; \
 $(CLINK) -o$@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
)
endef

define StaticLinkLibrary
(set -x; \
 $(RM) $@; \
 $(AR) $@ /P32 $(patsubst %,+%,$^); \
)
endef


OMNIORB_LIB = $(patsubst %,$(LibSearchPattern),omniORB3) $(OMNITHREAD_LIB)
lib_depend := $(patsubst %,$(LibPattern),omniORB3)
OMNIORB_LIB_DEPEND := $(GENERATE_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread)
lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)
