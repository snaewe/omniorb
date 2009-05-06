// -*- c++ -*-
//                          Package   : omniidl
// idlsysdep.h              Created on: 1999/11/04
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   System dependencies

// $Id$
// $Log$
// Revision 1.8.2.3  2009/05/06 16:14:33  dgrisby
// Update lots of copyright notices.
//
// Revision 1.8.2.2  2005/01/06 23:11:14  dgrisby
// Big merge from omni4_0_develop.
//
// Revision 1.8.2.1  2003/03/23 21:01:44  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.4.2.5  2002/02/01 11:22:06  dpg1
// strcasecmp/stricmp redefinition on Windows.
//
// Revision 1.4.2.4  2002/01/15 16:38:14  dpg1
// On the road to autoconf. Dependencies refactored, configure.ac
// written. No makefiles yet.
//
// Revision 1.4.2.3  2001/06/08 17:12:24  dpg1
// Merge all the bug fixes from omni3_develop.
//
// Revision 1.4.2.2  2000/10/27 16:31:10  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.4.2.1  2000/07/17 10:36:05  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.5  2000/07/13 15:25:52  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.2  1999/11/09 12:41:40  dpg1
// strcasecmp changes for NT, AIX.
//
// Revision 1.1  1999/11/05 09:43:01  dpg1
// Changes for NT.
//

#ifndef _idlsysdep_h_
#define _idlsysdep_h_

#define USE_omniORB_logStream
#include <omniORB4/CORBA_sysdep.h>


// As usual, Windows manages to be different...
#ifdef __WIN32__
// declspec is a define in windef.h under MINGW
#ifdef __MINGW32__
#  define DLL_EXPORT __attribute__((dllexport))
#else
#  define DLL_EXPORT _declspec(dllexport)
#endif
#else
#  define DLL_EXPORT
#  include <strings.h>
#endif


#ifdef HAS_Cplusplus_Bool
typedef bool                      IDL_Boolean;
#else
typedef unsigned char             IDL_Boolean;
#endif

typedef unsigned char             IDL_Char;

typedef unsigned char             IDL_Octet;

typedef short                     IDL_Short;

typedef unsigned short            IDL_UShort;

typedef unsigned short            IDL_WChar;

#if SIZEOF_LONG == 4
typedef long                      IDL_Long;

typedef unsigned long             IDL_ULong;
#elif SIZEOF_INT == 4
typedef int                       IDL_Long;

typedef unsigned int              IDL_ULong;
#else
# error "Can't map Long (32 bits) to a native type."
#endif

#ifdef HAS_LongLong
typedef _CORBA_LONGLONG_DECL      IDL_LongLong;
typedef _CORBA_ULONGLONG_DECL     IDL_ULongLong;
#endif


#ifndef NO_FLOAT

#ifndef __VAX

// This platform uses IEEE float
typedef float                     IDL_Float;
typedef double                    IDL_Double;

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    IDL_LongDouble;
#endif

#else	// VAX float test

// VAX uses proxies for float.
#define USING_PROXY_FLOAT

class IDL_Float {
  IDL_Long pd_f;
  void cvt_(float f);
  float cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  inline IDL_Float() {cvt_(0.0f);}
  inline IDL_Float(float f) {cvt_(f);}
  inline operator float() const {return cvt_();}
};

class IDL_Double {
  IDL_Long pd_d[2];
  void cvt_(double d);
  double cvt_() const;
public:
  // using compiler generated copy constructor and copy assignment
  inline IDL_Double() {cvt_(0.0);}
  inline IDL_Double(double d) {cvt_(d);}
  inline operator double() const {return cvt_();}
};

#undef cvt_

//  Assume long double type is compatible with the CORBA standard.

#ifdef HAS_LongDouble
typedef _CORBA_LONGDOUBLE_DECL    IDL_LongDouble;
#endif

#endif   // VAX float test
#endif   // !defined(NO_FLOAT)


#endif // _idlsysdep_h_
