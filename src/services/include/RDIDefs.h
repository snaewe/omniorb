#ifndef _RDI_DEFS_H_
#define _RDI_DEFS_H_

#include <iostream.h>
#include <iomanip.h>
#include <string.h>
#include <omniORB2/CORBA.h>

#if !defined(__GNUC__) && !defined(OSF) && !defined(UWIN) && !defined(sgi)
typedef unsigned char bool;
static const bool false = 0;
static const bool true  = 1;
#endif

/** STRING MANIPULATION FUNCTIONS
  * Various utility functions for managing strings plus their memory
  * memory requirements
  */ 

#ifdef RDI_DBG_ALLOC
extern int RDI_DbgAlloc_is_string;
#endif

// NB these are used for special tracking
// of memory leakage for CORBA::string_alloc and CORBA::string_dup use

#ifndef RDI_DBG_ALLOC
#define CORBA_STRING_ALLOC(sz) CORBA::string_alloc(sz)
#define CORBA_STRING_DUP(str)  CORBA::string_dup(str)
#else
inline char* CORBA_STRING_ALLOC(CORBA::ULong len) {
  RDI_DbgAlloc_is_string = 1; char* s = CORBA::string_alloc(len); RDI_DbgAlloc_is_string = 0; return s;
}
inline char* CORBA_STRING_DUP(const char* str) {
  RDI_DbgAlloc_is_string = 1; char* s = CORBA::string_dup(str);   RDI_DbgAlloc_is_string = 0; return s;
}
#endif

#define CORBA_STRING_FREE(str) CORBA::string_free(str)

//-------------------------------------------------------------------------
// RDI_DELNULL : only invoke on reference to obj alloc'd using new
#define RDI_DELNULL(x)      if (x) {delete x; x=NULL;}
#define RDI_DELNULL2(x,y)   RDI_DELNULL(x) RDI_DELNULL(y)
#define RDI_DELNULL3(x,y,z) RDI_DELNULL(x) RDI_DELNULL(y) RDI_DELNULL(z)

#define RDI_STRDELNULL(x)      if (x) {CORBA_STRING_FREE(x); x=NULL;}
#define RDI_STRDELNULL2(x,y)   RDI_STRDELNULL(x) RDI_STRDELNULL(y)
#define RDI_STRDELNULL3(x,y,z) RDI_STRDELNULL(x) RDI_STRDELNULL(y) RDI_STRDELNULL(z)

//-------------------------------------------------------------------------
// String utils: 

#define RDI_STR_EQ(s1,s2)  (strcmp(s1,s2)==0)
#define RDI_STR_NEQ(s1,s2) (strcmp(s1,s2)!=0)
#define RDI_STR_LT(s1,s2)  (strcmp(s1,s2)<0)
#define RDI_STR_LE(s1,s2)  (strcmp(s1,s2)<=0)
#define RDI_STR_GT(s1,s2)  (strcmp(s1,s2)>0)
#define RDI_STR_GE(s1,s2)  (strcmp(s1,s2)>=0)

// compare, ignoring case
#if defined(_MSC_VER)
#  define RDI_STR_EQ_I(s1,s2) (_stricmp(s1,s2)==0)
#else
#  define RDI_STR_EQ_I(s1,s2) (strcasecmp(s1,s2)==0)
#endif

inline char*
RDI_BUILD_STRING(const char* s1) { return CORBA_STRING_DUP(s1); }

inline char*
RDI_BUILD_STRING(const char* s1, 
		 const char* s2,
		 const char* s3 = NULL,
		 const char* s4 = NULL,
		 const char* s5 = NULL,
		 const char* s6 = NULL,
		 const char* s7 = NULL,
		 const char* s8 = NULL) {
  int i = 0;
  int len = 0;
  const char* s[8] = {s1, s2, s3, s4, s5, s6, s7, s8}; 
  for (i = 0; i < 8; i++) {
    if (s[i]) len += strlen(s[i]);
  }
  char* result = CORBA_STRING_ALLOC(len);
  strcpy(result, s1);
  for (i = 1; i < 8; i++) {
    if (s[i]) strcat(result, s[i]);
  }
  return(result);
}

#endif
