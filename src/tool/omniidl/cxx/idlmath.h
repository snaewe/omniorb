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
// Revision 1.7.2.1  2003/03/23 21:01:45  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.4.2.3  2002/01/15 16:38:14  dpg1
// On the road to autoconf. Dependencies refactored, configure.ac
// written. No makefiles yet.
//
// Revision 1.4.2.2  2000/10/27 16:31:09  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.4.2.1  2000/07/17 10:36:04  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.5  2000/07/13 15:25:53  dpg1
// Merge from omni3_develop for 3.0 release.
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

#ifdef HAVE_NAN_H
#  include <nan.h>
#endif

#if defined(HAVE_ISINF) && defined(HAVE_ISINFF) && (defined(HAVE_ISINFL) || !defined(HAS_LongDouble))

inline IDL_Boolean IdlFPOverflow(IDL_Double f) {
  return isinf(f) || isnan(f);
}

inline IDL_Boolean IdlFPOverflow(IDL_Float f) {
  return isinff(f) || isnanf(f);
}
#  ifdef HAS_LongDouble
inline IDL_Boolean IdlFPOverflow(IDL_LongDouble f) {
  return isinfl(f) || isnanl(f);
}
#  endif

#elif defined(HAVE_ISNANORINF)

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
