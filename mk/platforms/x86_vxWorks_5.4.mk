#
# x86 VxWorks 5.4.mk - make variables and rules specific to VxWorks with Tornado II
#                           and egcs compiler on a x86 target.
#

x86Processor = 1

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

RANLIB = ranlib386
AR = ar386 cq
NM = nm386

CXX = cc386
CXXMAKEDEPEND = $(TOP)/$(HOSTBINDIR)/omkdepend -D__cplusplus -D__GNUG__ -D__GNUC__
CXXFLAGS     += -Wno-unused -DRW_MULTI_THREAD -mpentium -nostdinc -nostdlib -fno-builtin \
           -fno-defer-pop -fno-gnu-linker
CXXDEBUGFLAGS = -g -O0

CXXLINK		= ld386
CXXLINKOPTIONS  = -r $(patsubst %,-rpath %,$(IMPORT_LIBRARY_DIRS))

CXXOPTIONS      = -Wall -Wno-unused  -I/tornado/target/h -DCPU=I80386 -DUSING_OLD_SOCKADDR_IN_STYLE

CC           = cc386
CMAKEDEPEND  = $(TOP)/$(HOSTBINDIR)/omkdepend -D__GNUC__
CDEBUGFLAGS  = -O

CLINK        = $(CC)
CLINKOPTIONS = $(CDEBUGFLAGS) $(COPTIONS) \
	       $(patsubst %,-Wl$(comma)-rpath$(comma)%,$(IMPORT_LIBRARY_DIRS))

IMPORT_CPPFLAGS += -D__x86__ -D__vxWorks__ -D__OSVERSION__=54

# use of a floting point unit of the processor (default: yes)
ifndef USE_FPU
USE_FPU=1
endif

# define directory where to copy vxWorks tools and applications
VXWORKSBINDIR = $(BASE_OMNI_TREE)/bin/x86_vxWorks_5.4

# Macro for target build
define CXXExecutable
(set -x; \
 echo Building loadable file; \
 $(RM) $@; \
 $(CXXLINK) -o _tmp.o $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
 $(NM) _tmp.o | $(WTXTCL) $(MUNCH_TCL_SCRIPT) -asm I80386 > ctdt.c; \
 $(CXX) -c ctdt.c; \
 $(CXXLINK) -r -o $@ _tmp.o ctdt.o; \
 $(RM) _tmp.o ctdt.c ctdc.o ; \
)
endef
