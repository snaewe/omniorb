# Usage:
#     nmake /f dir.mak [<build options>]
#        where <build options> are:
#           all     - build all subdirectories
#           clean   - clean all subdirectories
#           veryclean - do a veryclean in all subdirectories
#
#
SUBDIRS = implicit_activation

TOP = ..\..

XVARUSE0 = $(SUBDIRS)
XVARUSE1 = $$

all::
	@$(MAKE) $(MFLAGS) /c -f <<
.SUFFIXES:

TOP=..\$(TOP:/=\)

all : $(XVARUSE0:/=\)

$(XVARUSE0:/=\) ::
 @cd $(XVARUSE1)@
 @echo "making" all in $(XVARUSE1)@...
 @$(MAKE) /C /f dir.mak $(MFLAGS) all
 @cd ..
<<


XVARUSE0 = $(SUBDIRS)
XVARUSE1 = $$

clean::
	@$(MAKE) $(MFLAGS) /c -f <<
.SUFFIXES:

TOP=..\$(TOP:/=\)

clean : $(XVARUSE0:/=\)

$(XVARUSE0:/=\) ::
 @cd $(XVARUSE1)@
 @echo "making" clean in $(XVARUSE1)@...
 @$(MAKE) /C /f dir.mak $(MFLAGS) clean
 @cd ..
<<


XVARUSE0 = $(SUBDIRS)
XVARUSE1 = $$

veryclean::
	@$(MAKE) $(MFLAGS) /c -f <<
.SUFFIXES:

TOP=..\$(TOP:/=\)

veryclean : $(XVARUSE0:/=\)

$(XVARUSE0:/=\) ::
 @cd $(XVARUSE1)@
 @echo "making" veryclean in $(XVARUSE1)@...
 @$(MAKE) /C /f dir.mak $(MFLAGS) veryclean
 @cd ..
<<

