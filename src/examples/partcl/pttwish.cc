//--------------------------------------------------------------------------------
// ptt.cc - Simple test of omniParTcl.  We just repeatedly update a couple of 
//          variables on the C++ side and monitor their values in Tcl/Tk labels.
//          To test communication the other way, we allow the user to change the
//          output format for one of the variables.
//
// $Id$
// $Author$
//--------------------------------------------------------------------------------
/*
$Log$
Revision 1.2  1996/10/11 11:13:02  tjr
*** empty log message ***

*/

#include <string.h>
#include "omniParTcl.h"

static omni_thread *threadOne;
static omni_thread *threadTwo;

static char *fmt;
static omni_mutex fmtMutex;

//--------------------------------------------------------------------------------

void threadOneFunc(void *arg)
{
  tclDString ds;
  static long i = 0;
  
  while (1) { // Repeatedly do some Tcl.
      
    ds.init();
    ds.append("set tclVarOne ");

    //
    // By using appendElement() rather than append() we ensure that
    // any format string (e.g. one containing whitespace) will work.
    //
    fmtMutex.acquire();
    ds.appendElement(++i,fmt);      
    fmtMutex.release();    
    
    omniTclMeAndRun(ds.value());
    ds.free();
      
    omniTclMeAndWait("update");
    
    omni_thread::sleep(0,100000000); // Sleep a bit
  }
}

//--------------------------------------------------------------------------------

void threadTwoFunc(void *arg)
{
  tclDString ds;
  static double d = 0.999;
  
  while (1) { // Repeatedly do some Tcl.

    ds.init();
    ds.append("set tclVarTwo ");
    ds.append(d);
    
    omniTclMeAndRun(ds.value());
    ds.free();

    d -= 0.001;
    
    omni_thread::sleep(1,0); // Sleep a second
  }
}

//--------------------------------------------------------------------------------

int startThreadOne(ClientData clientData, Tcl_Interp *interp,
                   int argc, char *argv[])
{
  if ((threadOne = omni_thread::create(threadOneFunc,NULL)) == NULL) {
    Tcl_SetResult(interp, "ptt: cannot create threadOne", TCL_VOLATILE);
    return TCL_ERROR;
  }
  return TCL_OK;
}

//--------------------------------------------------------------------------------

int startThreadTwo(ClientData clientData, Tcl_Interp *interp,
                   int argc, char *argv[])
{
  if ((threadTwo = omni_thread::create(threadTwoFunc,NULL)) == NULL) {
    Tcl_SetResult(interp, "ptt: cannot create threadTwo", TCL_VOLATILE);
    return TCL_ERROR;
  }
  return TCL_OK;
}

//--------------------------------------------------------------------------------

int setFormat(ClientData clientData, Tcl_Interp *interp,
              int argc, char *argv[])
{
  //
  // Make sure we've been passed a new format string.  
  // If so, use it to update the format.  Otherwise,
  // leave it as-is.
  //
  
  if (argc >= 2) {
    delete fmt;
    fmtMutex.acquire();
    fmt = new char[strlen(argv[1])+1];
    strcpy(fmt,argv[1]);
    fmtMutex.release();
  }
  
  return TCL_OK;
}

//--------------------------------------------------------------------------------

int Tcl_AppInit(Tcl_Interp *interp)
{
  //
  // For omniParTcl applications, Tcl_Appinit must call omniParTcl_Init()
  // as well as the Tcl and Tk init routines.   Any C++ functions which
  // are to be callable from Tcl must be specified in a call to
  // Tcl_CreateCommand().
  //

  Tk_Window main;
  main = Tk_MainWindow(interp);

  if (Tcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }
  if (Tk_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }
  if (omniParTcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }

  // tcl_RcFileName = ".mytclrc"; // No RC file for this app.

  //
  // Make our C++ functions available in Tck/Tk.
  //
  Tcl_CreateCommand(interp, "CXXstartThreadOne", startThreadOne,
                    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "CXXstartThreadTwo", startThreadTwo,
                    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "CXXsetformat", setFormat,
                    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  //
  // Application specific initialization.  Default format for
  // the first variable is standard decimal.
  //
  fmt = new char[3];
  strcpy(fmt,"%d");
  
  return TCL_OK;
}

//--------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  //
  // omniParTcl is designed for use with Tcl7.4/Tk4.0 which means that
  // the user must write his or her own main().  This provides extra
  // flexibility.  For example, a command-line switch could determine
  // whether the application used a graphical interface (in which case
  // it would call Tk_Main) or an ascii interface (in which case it
  // would call Tcl_Main.)
  //
  // For typical Tk-based graphical interfaces, main() simply calls
  // Tk_Main() and returns 0 as below.  In this case, the program
  // behaves like a standard wish and would typically be invoked
  // using the "# <program> -f" construct at the top of a Tcl/Tk
  // source file.
  //
  
  Tk_Main(argc, argv, Tcl_AppInit);  // Run the Tk scheduler.
  return 0;	                     // Make the compiler happy.
}

