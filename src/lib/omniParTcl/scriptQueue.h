//
// Simple FIFO queue for Tcl/Tk scripts awaiting evaluation.
//
// $Id$ $Author$
//

#ifndef _scriptQueue_h
#define _scriptQueue_h

struct scriptcondpair {
  char *script;
  omni_condition *cond;
};

class scriptQueue {
  private:

    struct sqe {
      scriptcondpair scp;
      sqe *next;
    };

    sqe  *head;
    sqe  *tail;

  public:
    scriptQueue() {head = tail = 0;}

    void enq(char *script, omni_condition *cond);
    scriptcondpair deq();
};

#endif
