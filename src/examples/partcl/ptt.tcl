#
# Simple test of omniParTcl.  We just repeatedly update a couple of 
# variables on the C++ side and monitor their values in Tcl/Tk labels.
# To test communication the other way, we allow the user to change the
# output format for one of the variables.
#
# $Id$
# $Author$
#
# $Log$
# Revision 1.2.6.1  1999/09/15 20:23:19  sll
# *** empty log message ***
#
# Revision 1.2  1996/10/11  11:11:04  tjr
# *** empty log message ***
#
# Revision 1.1  1995/09/04  11:01:45  krw
# Initial revision
#
#

set tclVarOne 0
set tclVarTwo 0

CXXstartThreadOne
CXXstartThreadTwo

label .l1 -textvariable tclVarOne -bg cyan
label .l2 -textvariable tclVarTwo -bg green

button .q -text "Quit" -command exit
button .b -text "Change format" -command changeFormat

pack .q .l1 .l2 .b -fill both
update

proc changeFormat {} {
  pack forget .b  
  label .fl -text "New format string:"
  entry .fe 
  pack .fl .fe -side left
  update
  bind .fe <Return> {CXXsetformat [.fe get]; destroy .fe; destroy .fl; pack .b; update}
}
