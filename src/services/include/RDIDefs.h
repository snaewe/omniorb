#ifndef _RDI_DEFS_H_
#define _RDI_DEFS_H_

#include <iostream.h>
#include <iomanip.h>
#include <string.h>

#if !defined(__GNUC__) && !defined(OSF) && !defined(UWIN) && !defined(sgi)
typedef unsigned char bool;
static const bool false = 0;
static const bool true  = 1;
#endif

/** STRING MANIPULATION FUNCTIONS
  * Various utility functions for managing strings plus their memory
  * memory requirements
  */ 

inline char* RDI_StringDupl(const char* string) 
{ char* ptr = 0;
  if ( string && (ptr = new char [ strlen(string) + 1 ]) != (char *) 0 )
        strcpy(ptr, string);
  return ptr;
}

inline char* RDI_StringAlloc(unsigned int size)
{ char* ptr = new char [ size + 1]; return ptr; }

inline void RDI_StringFree(char* string)
{ if ( string ) delete [] string; }

#endif
 
