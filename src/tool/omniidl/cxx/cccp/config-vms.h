#ifdef __VMS

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#ifndef VAXC
/* assume decc */
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1
#if defined(__ALPHA) || defined(__VAX) && defined(__DECC) && __DECC_VER > 60000000
#include <builtins.h>
#define alloca __ALLOCA
#define HAVE_ALLOCA 1
#endif

#define HAVE_FCNTL_H 1
#define HAVE_STDLIB_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_UNISTD_H 1
#define STDC_HEADERS 1
/* but not:								    */
/*									    */
/* #define TIME_WITH_SYS_TIME						    */
/*									    */
/* (actually, most VMS compilers treat					    */
/*									    */
/*  #include <sys/time.h>						    */
/*									    */
/* the same as								    */
/*									    */
/*  #include <time.h>							    */
/*									    */
/* but why bother. */

#endif /* VAXC not defined						    */

#endif
