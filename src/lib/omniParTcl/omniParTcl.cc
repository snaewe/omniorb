//
// omniParTcl.cc
// 
// Implementation of loosely synchronized parallel C++ and Tcl/Tk threads, 
// for use in particular to create GUIs for Orbix applications.  This is
// version II which is more lightweight, allows the developer to write
// his or her own main() if desired, and provides a C++ class for Tcl Dstring
// manipulation.
//
// $Id$ $Author$
//
 
extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
}

#include <stream.h>
#include <strstream.h>

#include "omniParTcl.h"
#include "scriptQueue.h"

//
// We use a pipe to allow the C++ threads to signal asynchronously
// to Tcl/Tk that there is something to do.  We mutex-protect access
// to the pipe.
//
static int        tclWakeupPipe[2];
static omni_mutex twpMutex;

//
// We maintain a FIFO queue of Tcl/Tk scripts waiting for evaluation.
// Note that the script queue copies the script names, so the two
// omniParTcl functions can be called with constant strings.  We
// mutex-protect access to the script queue.
//
static scriptQueue sQ;
static omni_mutex  sqMutex;

void omniTclMeAndWait(char *script)
{
  //
  // Arrange to have the given script evaluated by Tcl/Tk, and don't
  // return until the script has been evaulated.
  //
  char dummybyte; // We can write any old byte to the pipe
  
  omni_condition sqeCond(&sqMutex);
    
  sqMutex.acquire();  

  sQ.enq(script,&sqeCond);                 // Put script on queue of those
                                           // waiting for eval.

  twpMutex.acquire();
  write(tclWakeupPipe[1],&dummybyte,1);    // Wake up Tcl/Tk
  twpMutex.release();
  
  sqeCond.wait();                          // Wait for Tcl/Tk to finish with
                                           // this script.

  sqMutex.release();                       // Release the lock on the queue
}  

void omniTclMeAndRun(char *script)
{
  //
  // Arrange to have the given script evaluated by Tcl/Tk, and return
  // immediately.
  //
  char dummybyte; // We can write any old byte to the pipe
  
  sqMutex.acquire(); //
  sQ.enq(script,0);   // Put script on queue of those waiting for eval.
  sqMutex.release(); //

  twpMutex.acquire();
  write(tclWakeupPipe[1],&dummybyte,1);    // Wake up Tcl/Tk
  twpMutex.release();
}  

void cxxTclScriptHandler(ClientData clientData, int mask)
{
  //
  // This is invoked by Tcl/Tk whenever there is something to read on
  // tclWakeupPipe[0].  This will only be the case when at least one C++ 
  // thread has called one of the above omniParTcl functions.  There may 
  // be more than one script awaiting evaluation, and we evaluate all of 
  // them.
  //
  Tcl_Interp *interp = (Tcl_Interp *) clientData;
  scriptcondpair scp;
  char sink;
  
  if (mask != TK_READABLE) {
    cerr << "omniParTcl: unexpected state of tclWakeupPipe." << endl;
    ::exit(1);
  }

  int done = 0;
  while (!done) {
    
    sqMutex.acquire();  //
    scp = sQ.deq();      // Get next script/condition pair.
    sqMutex.release();  //

    if (scp.script == (char *)0) {
      done = 1;
    }
    else {
      //
      // Consume the trigger input:
      //
      read(tclWakeupPipe[0],&sink,1); 

      //
      // Evaluate the script:
      //
      int rc = Tcl_GlobalEval(interp, scp.script);
      if (rc != TCL_OK) {
        cerr << "omniParTcl: error evaluating Tcl/Tk script \""
             << scp.script << "\"" << endl;
        cerr << interp->result << endl;
        ::exit(1);
      }

      //
      // Reclaim the string storage:
      //
      delete scp.script;

      //
      // Wake up the C++ thread if it's waiting.
      //
      if (scp.cond != 0) {
        sqMutex.acquire();
        scp.cond->signal();
        sqMutex.release(); 
      }
    }
  }
}

int threadSafeExec(ClientData clientData, Tcl_Interp *interp,
                   int argc, char *argv[])
{
  //
  // Tcl's exec command uses vfork which is not thread-safe and therefore
  // not omniParTcl-safe either.  Use threadSafeExec instead.
  //
  int i;
  
  if (argc < 2) {
    Tcl_SetResult(interp, "omniParTcl: threadSafeExec requires an argument",
                  TCL_VOLATILE);
    return TCL_ERROR;
  }

  int cmdlen = 0;
  for (i = 1; i < argc; i++) {
    cmdlen += strlen(argv[i]) + 1;
  }  
  cmdlen += 1;

  char *cmd = new char[cmdlen];
  ostrstream cmdos(cmd,cmdlen);

  for (i = 1; i < argc; i++) {
    cmdos << argv[i] << " ";
  }
  cmdos << ends;

  system(cmd);
  delete cmd;

  return TCL_OK;
}
  
int omniParTcl_Init(Tcl_Interp *interp)
{
  //
  // omniParTcl initialization.  To be called from Tcl_AppInit().
  //

  //
  // We use Tcl/Tk's file event callbacks to allow C++ to invoke
  // Tcl/Tk commands asynchronously.
  //
  
  if (pipe(tclWakeupPipe) != 0) {
    Tcl_SetResult(interp, "omniParTcl: cannot create tclWakeupPipe", TCL_VOLATILE);
    return TCL_ERROR;
  }
  
  Tk_CreateFileHandler(tclWakeupPipe[0], TK_READABLE, 
                       (Tk_FileProc *) cxxTclScriptHandler,
                       (ClientData) interp);

  //
  // Provide a thread-safe exec since Tcl doesn't.
  //
  Tcl_CreateCommand(interp, "threadSafeExec", threadSafeExec,
                    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return TCL_OK;
}


