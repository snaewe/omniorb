//
// omninest.h
//

// Contains macros that allow application code to be completely
// portable with respect to the nested/non-nested issue.

// amrw (7/3/95)

#ifndef CONCAT
#define CONCAT(A,B) A##B
#endif 

#ifndef CONCAT3
#define CONCAT3(A,B,C) A##B##C
#endif

#ifndef COLONCONCAT
#define COLONCONCAT(A,B) A/**/B
#endif

#ifndef COLONCONCAT3
#define COLONCONCAT3(A,B,C) A/**/B/**/C
#endif

#ifdef NONNESTED

// MACROS FOR COMPILERS WHICH DO NOT FULLY SUPPORT NESTED CLASSES

// These macros should be integrated into CORBA.h by Iona in
// non-nested releases of Orbix

#ifndef CORBA
#define CORBA(X)		CONCAT(CORBA_,X)
#endif

#ifndef STEXCEP
#define STEXCEP(X)		CONCAT(CORBA_StExcep_,X)
#endif

// Macro to scope class names correctly

#ifndef SCOPED
#define SCOPED(X,Y)		CONCAT3(X,_,Y)
#endif

#ifndef INSCOPED
#define INSCOPED(X,Y)		Y
#endif

#else

// MACROS FOR COMPILERS WHICH FULLY SUPPORT NESTED CLASSES

// These macros will be intergrated into CORBA.h by Iona in 
// future nested releases of Orbix.

#ifndef CORBA
#define CORBA(X)		CORBA::X
#endif

#ifndef STEXCEP
#define STEXCEP(X)		CORBA::StExcep::X
#endif

// Macro to scope class names correctly

#ifndef SCOPED
#define SCOPED(X,Y)		COLONCONCAT3(X,::,Y)
#endif

#ifndef INSCOPED
#define INSCOPED(X,Y)		COLONCONCAT3(X,::,Y)
#endif

#endif







