#
# PowerPC VxWorks 5.3.mk - make variables and rules specific to VxWorks
# and gcc on a PowerPC target.
#

PowerPCProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
PYTHON = /usr/local/bin/python


#
# Include general VxWorks things
#

include $(THIS_IMPORT_TREE)/mk/vxWorks.mk


#
# Standard programs
#
#//willi hier eingefügt
MKDIRHIER = mkdirhier

CCROOT=/net/compiler/gcc_mpt_1.02/bin
RANLIB = $(CCROOT)/powerpc-wrs-vxworks-ranlib
AR = $(CCROOT)/powerpc-wrs-vxworks-ar cq
NM = $(CCROOT)/powerpc-wrs-vxworks-nm

CXX = $(CCROOT)/powerpc-wrs-vxworks-g++
CXXMAKEDEPEND = /usr/local/bin/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
#//willi
#//original CXXFLAGS     += -fno-rtti -mno-sched-prolog -fno-builtin -Wno-unused  -ansi -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXFLAGS     += -fno-builtin -Wno-unused -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXDEBUGFLAGS = 

CXXLINK		= $(CCROOT)/powerpc-wrs-vxworks-ld
CXXLINKOPTIONS  = -r $(patsubst %,-rpath %,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused

CC           = $(CCROOT)/powerpc-wrs-vxworks-gcc
CMAKEDEPEND  = /usr/local/bin/omkdepend -D__GNUC__
CDEBUGFLAGS  =  -O2

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

IMPORT_CPPFLAGS += -D__powerpc__ -D__vxWorks__ -D__OSVERSION__=53

# define directory where to copy vxWorks tools and applications
VXWORKSBINDIR = $(BASE_OMNI_TREE)/bin/arm_vxWorks_5.3

MEAS_Make=/net/ccm_wa/mpt1/MEAS_Makefiles~gcc_mpt_1.01_21.0/MEAS_Makefiles
MUNCH=$(MEAS_Make)/munchPPC604new

# Macro for target build
define CXXExecutable
(set -x; \
 echo Building loadable file; \
 $(RM) $@; \
 $(CXXLINK) -o _tmp.o $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
 $(NM) _tmp.o | $(MUNCH) > ctdt.s; \
 $(CXX) -c ctdt.s; \
 $(CXXLINK) -r -o $@ _tmp.o ctdt.o; \
 $(RM) _tmp.o ctdt.c ctdc.o ; \
)
endef
