#
# ARM VxWorks 5.4.mk - make variables and rules specific to VxWorks with Tornado II
#                           and egcs compiler on a ARM target.
#

ARMProcessor = 1

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
PYTHON = /usr/gnu/bin/python


#
# Include general VxWorks things
#

include $(THIS_IMPORT_TREE)/mk/vxWorks.mk


#
# Standard programs
#
#//willi hier eingefügt
MKDIRHIER = mkdirhier

RANLIB = ranlibarm
AR = ararm cq
NM = nmarm

CXX = ccarm
CXXMAKEDEPEND = $(TOP)/$(HOSTBINDIR)/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
#//willi
#//original CXXFLAGS     += -fno-rtti -mno-sched-prolog -fno-builtin -Wno-unused  -ansi -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXFLAGS     += -mno-sched-prolog -fno-builtin -Wno-unused -nostdinc -DRW_MULTI_THREAD -fvolatile
CXXDEBUGFLAGS = -g

CXXLINK		= ldarm
CXXLINKOPTIONS  = -r $(patsubst %,-rpath %,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused -g -DCPU=ARMSA110 -mcpu=strongarm110 -mapcs-32

CC           = ccarm
CMAKEDEPEND  = $(TOP)/$(HOSTBINDIR)/omkdepend -D__GNUC__
CDEBUGFLAGS  = -g -O2

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

IMPORT_CPPFLAGS += -D__arm__ -D__vxWorks__ -D__OSVERSION__=54

# define directory where to copy vxWorks tools and applications
VXWORKSBINDIR = $(BASE_OMNI_TREE)/bin/arm_vxWorks_5.4

# Macro for target build
define CXXExecutable
(set -x; \
 echo Building loadable file; \
 $(RM) $@; \
 $(CXXLINK) -o _tmp.o $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
 $(NM) _tmp.o | $(WTXTCL) $(MUNCH_TCL_SCRIPT) -asm IARMSA110 > ctdt.c; \
 $(CXX) -c ctdt.c; \
 $(CXXLINK) -r -o $@ _tmp.o ctdt.o; \
 $(RM) _tmp.o ctdt.c ctdc.o ; \
)
endef
