#ifndef _RDI_UTILS_H_
#define _RDI_UTILS_H_


#include "RDIDefs.h"

//-------------------------------------------------------------------------
// XXX always use these for string alloc/free
// XXX #include <omniORB2/CORBA.h>
// XXX #define RDI_STRALLOC(len) CORBA::string_alloc(len)
// XXX #define RDI_STRFREE(sptr) CORBA::string_free(sptr)
// XXX
// XXX thimios is not using the CORBA string class /
// XXX   flex and yacc are not using it either

#define RDI_STRALLOC(len) RDI_StringAlloc(len)
// #define RDI_STRFREE(sptr) cout << "XXX RDI_STRFREE called on " << (void*)sptr << endl; RDI_StringFree(sptr)
#define RDI_STRFREE(sptr) RDI_StringFree(sptr)

//-------------------------------------------------------------------------
// RDI_DELNULL : only invoke on reference to obj alloc'd using new
#define RDI_DELNULL(x)      if (x) {delete x; x=NULL;}
#define RDI_DELNULL2(x,y)   RDI_DELNULL(x) RDI_DELNULL(y)
#define RDI_DELNULL3(x,y,z) RDI_DELNULL(x) RDI_DELNULL(y) RDI_DELNULL(z)

#define RDI_STRDELNULL(x)      if (x) {RDI_STRFREE(x); x=NULL;}
#define RDI_STRDELNULL2(x,y)   RDI_STRDELNULL(x) RDI_STRDELNULL(y)
#define RDI_STRDELNULL3(x,y,z) RDI_STRDELNULL(x) RDI_STRDELNULL(y) RDI_STRDELNULL(z)

//-------------------------------------------------------------------------
// String utils: 

#define STR_EQ(s1,s2)  (strcmp(s1,s2)==0)
#define STR_NEQ(s1,s2) (strcmp(s1,s2)!=0)
#define STR_LT(s1,s2)  (strcmp(s1,s2)<0)
#define STR_LE(s1,s2)  (strcmp(s1,s2)<=0)
#define STR_GT(s1,s2)  (strcmp(s1,s2)>0)
#define STR_GE(s1,s2)  (strcmp(s1,s2)>=0)

// compare, ignoring case
#if defined(_MSC_VER)
#  define STR_EQ_I(s1,s2) (_stricmp(s1,s2)==0)
#else
#  define STR_EQ_I(s1,s2) (strcasecmp(s1,s2)==0)
#endif

inline char*
BUILD_STRING(const char* s1, 
	     const char* s2 = NULL,
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
  char* result = RDI_STRALLOC(len);
  strcpy(result, s1);
  for (i = 1; i < 8; i++) {
    if (s[i]) strcat(result, s[i]);
  }
  return(result);
}

#endif
