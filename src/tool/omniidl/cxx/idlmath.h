// -*- c++ -*-
//                          Package   : omniidl
// idlmath.cc               Created on: 1999/10/19
//			    Author    : Duncan Grisby (dpg1)
//
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
//   Floating point maths functions

// $Id$
// $Log$
// Revision 1.2.2.1  2000/10/24 09:53:29  dpg1
// Clean up omniidl system dependencies. Replace use of _CORBA_ types
// with IDL_ types.
//
// Revision 1.2  1999/11/02 17:07:26  dpg1
// Changes to compile on Solaris.
//
// Revision 1.1  1999/10/27 14:05:56  dpg1
// *** empty log message ***
//

#ifndef _idlmath_h_
#define _idlmath_h_

#include <math.h>
#include <idlutil.h>

#if defined(__linux__)

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  return isinff(f) || isnanf(f);
}
inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return isinf(f) || isnan(f);
}
#ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return isinfl(f) || isnanl(f);
}
#endif

#elif defined(__sunos__)

#include <nan.h>

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  double d = f;
  return IsNANorINF(d);
}
inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return IsNANorINF(f);
}
#ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return 0;
}
#endif

#else // No FP overflow detection

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  return 0;
}
inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return 0;
}
#ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return 0;
}
#endif

#endif

#endif // _idlmath_h_
