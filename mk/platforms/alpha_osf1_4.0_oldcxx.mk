#
# alpha_osf1_4.0.mk - make variables and rules specific to Digital Unix
# (i.e. OSF1) 4.0.
#

OSF1 = 1
AlphaProcessor = 1


#
# Include general unix things
#

include $(THIS_IMPORT_TREE)/mk/unix.mk


#
# C preprocessor macro definitions for this architecture
#

IMPORT_CPPFLAGS += -D__alpha__ -D__osf1__ -D__OSVERSION__=4


#
# Standard programs
#

AR = ar clq

CXX = /usr/bin/cxx
# Use oldcxx (v 5.6) that comes with DEC C++ 6.0
CXXOPTIONS = -oldcxx
# For DEC C++ 5.6
#
DecCxxMajorVersion = 5
DecCxxMinorVersion = 6
#
# For DEC C++ 6.0
# Uncommment the following line to speed up the compilation, but may require
# manually deleted some .pch and cxx_respository/TIMESTAMP files to pick
# up changes in templates or the order of -I flags.
#
# CXXOPTIONS += -ttimestamp -pch

CXXMAKEDEPEND = omkdepend -D__DECCXX -D__cplusplus
CXXDEBUGFLAGS = -O

CXXLINK		= $(CXX)
CXXLINKOPTIONS  = $(CXXDEBUGFLAGS) $(CXXOPTIONS) -call_shared

CC = cc
CMAKEDEPEND = omkdepend
CDEBUGFLAGS = -O

CLINK = $(CC)


#
# When specifying the "rpath" (directories which the run-time linker should
# search for shared libraries) we unfortunately need to do it in a single
# argument.  For this reason we override the default unix CXXExecutable and
# CExecutable rules.  Any -L flags given in $$libs results in another element
# being added to the rpath and we then give the whole rpath at the end of the
# link command line.
#

RPATH = $(subst $(space),:,$(strip $(IMPORT_LIBRARY_DIRS)))

define CXXExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CXXLINK) -o $@ $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef

define CExecutable
(rpath="$(RPATH)"; \
 for arg in $$libs; do \
   if expr "$$arg" : "-L" >/dev/null; then \
     rpath="$$rpath$${rpath+:}`expr $$arg : '-L\(.*\)'"; \
   fi; \
 done; \
 set -x; \
 $(RM) $@; \
 $(CLINK) -o $@ $(CLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs -rpath $$rpath; \
)
endef


#
# CORBA stuff
#

ORBIX2_ROOT = /usr/local/Orbix2
ORBIX2_IDL = idl -c C.cc -s S.cc -B -minterOp
ORBIX2_CPPFLAGS = -D__ORBIX__ -D__ORBIX_2.0 -DORBIX_MT -D_REENTRANT \
		  -DUSE_IIOP -I$(ORBIX2_ROOT)/include -I$(CORBA_STUB_DIR)
ORBIX2_LIB = -L$(ORBIX2_ROOT)/lib -liiop -lITsrvmt -lIRcltmt -lITinimt -lpthread -lmach -lc_r -lrt
ORBIX2_STUB_HDR_PATTERN = $(CORBA_STUB_DIR)/%.hh
ORBIX2_STUB_SRC_PATTERN = $(CORBA_STUB_DIR)/%S.cc
ORBIX2_STUB_OBJ_PATTERN = $(CORBA_STUB_DIR)/%S.o
ORBIX2_EXTRA_STUB_FILES = $(CORBA_INTERFACES:%=$(CORBA_STUB_DIR)/%C.cc)

omniORB2GatekeeperImplementation = OMNIORB2_TCPWRAPGK
CorbaImplementation = OMNIORB2

#
# OMNI thread stuff
#

ThreadSystem = Posix

OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=10 -DNoNanoSleep
OMNITHREAD_CPPFLAGS = -D_REENTRANT -pthread

# The pthread package before 4.0 was POSIX 1003.4a draft 4. If for some
# reason it is necessary to run the same binaries on 4.0 and older systems
# (e.g. 3.2), use the following make variables instead.
#
# OMNITHREAD_POSIX_CPPFLAGS = -DPthreadDraftVersion=4 -DNoNanoSleep 
# OMNITHREAD_CPPFLAGS = -D_PTHREAD_USE_D4 -D_REENTRANT

OMNITHREAD_LIB = $(patsubst %,$(LibSearchPattern),omnithread) \
		 -lpthread -lmach -lc_r

lib_depend := $(patsubst %,$(LibPattern),omnithread)
OMNITHREAD_LIB_DEPEND := $(GENERATE_LIB_DEPEND)


#
# OMNI ParTcl stuff
#

TCLTK_CPPFLAGS = -I/usr/local/include/Tcl7.4Tk4.0
TCLTK_LIB = -L/usr/local/lib -ltk4.0 -ltcl7.4 -lm
X11_CPPFLAGS = -I/usr/include/X11
X11_LIB = -lX11
WISH4 = /usr/local/bin/wish4.0

OMNIPARTCL_CPPFLAGS = $(TCLTK_CPPFLAGS) $(X11_CPPFLAGS) $(OMNITHREAD_CPPFLAGS)
OMNIPARTCL_LIB = $(patsubst %,$(LibSearchPattern),omniParTcl) $(TCLTK_LIB) \
		 $(X11_LIB) $(OMNITHREAD_LIB)
lib_depend := $(patsubst %,$(LibPattern),omniParTcl)
OMNIPARTCL_LIB_DEPEND := $(GENERATE_LIB_DEPEND) $(OMNITHREAD_LIB_DEPEND)
