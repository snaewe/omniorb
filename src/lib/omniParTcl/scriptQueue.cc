//
// scriptQueue.cc
//
// Implementation of simple FIFO queue for Tcl/Tk scripts awaiting evaluation.
// Note that strings are copied on entry, and can be deleted after dequeuing.
//
// Note also that we queue a condition variable along with each entry.  For
// entries made from omniParTclAndRun(), this condition variable will be
// null, but for those from omniParTclAndWait() it will be non-null and will
// be used to signal the waiting thread to wake up after the script has been
// evaluated in the Tcl/Tk thread.
//
// $Id$ $Author$
//
 
#include <string.h>
#include "omnithread.h"

#include "scriptQueue.h"

void scriptQueue::enq(char *script, omni_condition *cond)
{
  char *newscript = new char[strlen(script)+1];
  strcpy(newscript,script);

  sqe *newsqe = new sqe;
  newsqe->scp.script = newscript;
  newsqe->scp.cond = cond;
  newsqe->next = 0;
  
  if (!head) {
    head = tail = newsqe;
  }
  else {
    tail->next = newsqe;
    tail = newsqe;
  }
}

scriptcondpair scriptQueue::deq()
{
  scriptcondpair scp;
  
  if (!head) {
    scp.script = (char *)0;
  }
  else {
    sqe *hq = head;
    scp = head->scp;
    head = head->next;
    delete hq;
  }

  return scp;
}

