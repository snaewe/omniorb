//
// Wrapper class for extended Tcl DString functions.
//
// $Id$ $Author$
//

#ifndef _tclDStringClass_h
#define _tclDStringClass_h

#include <tcl.h>

#define NUMBER_BUFSIZE 512

class tclDString {
  private:
    Tcl_DString theDS;
    char numberBuf[NUMBER_BUFSIZE];

  public:
    tclDString() {this->init();}
    ~tclDString() {this->free();}

    void init();
    char *append(char *string, int length = -1);
    char *append(long lval, char *format = "%d");
    char *append(double dval, char *format = "%g");
    char *appendElement(char *string, int length = -1);
    char *appendElement(long lval, char *format = "%d");
    char *appendElement(double dval, char *format = "%g");
    void startSublist();
    void endSublist();
    char *value();
    int  length();
    void trunc(int newLength);
    void free();
    void result(Tcl_Interp *interp);
};

#endif
