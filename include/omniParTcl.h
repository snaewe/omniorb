//
// omniParTcl.h
// 
// Header file for creating Tcl/Tk user interfaces for Orbix and other C++ programs.
// The mechanism employed is loose synchronization of a single thread running
// Tcl/Tk and one or more threads running C++ code.
//
// $Id$ $Author$
//

#ifndef _omniParTcl_h
#define _omniParTcl_h

#include "omniParTcl/tclDStringClass.h"
#include "omnithread.h"

extern "C" 
{
#include <tk.h>
}

// 
// Init function to be called from Tcl_AppInit().
//
int omniParTcl_Init(Tcl_Interp *interp);

// 
// Functions to set up Tcl/Tk invocations from C++.  The first will arrange
// to have the Tcl/Tk script evaluated as soon as possible, and will not
// return until the script has been evalulated.  The second will make the
// same arrangement, but will return immediately.  
//
void omniTclMeAndWait(char *script);
void omniTclMeAndRun(char *script);

#endif

