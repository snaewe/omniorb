//
// Implementation of wrapper class for extended Tcl DString functions.
//
// $Id$ $Author$
//

#include "omniParTcl/tclDStringClass.h"

#include <stdio.h>
#include <string.h>

//
//---------- Overloaded extensions to Tcl_DStringAppend() ----------
//

char *tclDString::append(char *string, int length)
{
  return Tcl_DStringAppend(&theDS,string,length);
}

char *tclDString::append(long lval, char *format)
{
  sprintf(numberBuf,format,lval);

  return Tcl_DStringAppend(&theDS,numberBuf,-1);
}

char *tclDString::append(double dval, char *format)
{
  sprintf(numberBuf,format,dval);

  return Tcl_DStringAppend(&theDS,numberBuf,-1);
}

//
//---------- Overloaded extensions to Tcl_DStringAppendElement() ----------
//

char *tclDString::appendElement(char *string, int length)
{
  //
  // We have extended the appendElement function to mirror append,
  // i.e. you can optionally specify a length to which the (original)
  // string will be truncated before being converted to a list element
  // and appended.
  //

  if (length < 0) {
    return Tcl_DStringAppendElement(&theDS,string);
  }
  else {
    int slen = strlen(string);
    int nlen = (slen < length) ? slen : length;
    char *cbuf = new char[nlen+1];
    memcpy((void *) cbuf, (void *) string, (size_t) nlen);
    cbuf[nlen] = '\0';
    char *result = Tcl_DStringAppendElement(&theDS,cbuf);
    delete cbuf;
    return result;
  }
}

char *tclDString::appendElement(long lval, char *format)
{
  sprintf(numberBuf,format,lval);  

  return Tcl_DStringAppendElement(&theDS,numberBuf);
}

char *tclDString::appendElement(double dval, char *format)
{
  sprintf(numberBuf,format,dval);

  return Tcl_DStringAppendElement(&theDS,numberBuf);
}

//
//---------- Wrappers for the rest ----------
//

void tclDString::init()
{
  Tcl_DStringInit(&theDS);  
}

void tclDString::startSublist()
{
  Tcl_DStringStartSublist(&theDS);  
}

void tclDString::endSublist()
{
  Tcl_DStringEndSublist(&theDS);  
}

char *tclDString::value()
{
  return Tcl_DStringValue(&theDS);  
}

int tclDString::length()
{
  return Tcl_DStringLength(&theDS);  
}

void tclDString::trunc(int newLength)
{
  Tcl_DStringTrunc(&theDS,newLength);  
}

void tclDString::free()
{
  Tcl_DStringFree(&theDS);  
}

void tclDString::result(Tcl_Interp *interp)
{
  Tcl_DStringResult(interp,&theDS);  
}

