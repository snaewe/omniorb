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
// Revision 1.1  1999/10/27 14:05:56  dpg1
// *** empty log message ***
//

#ifndef _idlmath_h_
#define _idlmath_h_

#include <math.h>
#include <idlutil.h>

inline _CORBA_Boolean IdlFPOverflow(_CORBA_Float f) {
  return isinff(f) || isnanf(f);
}
inline _CORBA_Boolean IdlFPOverflow(_CORBA_Double f) {
  return isinf(f) || isnan(f);
}
#ifdef HAS_LongDouble
inline _CORBA_Boolean IdlFPOverflow(_CORBA_LongDouble f) {
  return isinfl(f) || isnanl(f);
}
#endif

#endif // _idlmath_h_
