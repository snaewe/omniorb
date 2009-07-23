// -*- Mode: C++; -*-
//                            Package   : omniORB
// fixed.h                    Created on: 06/02/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//
//    Global definitions and operators for the Fixed type

// $Log$
// Revision 1.1.2.1  2001/03/13 10:32:05  dpg1
// Fixed point support.
//

#ifndef __fixed_h__
#define __fixed_h__


//
// Arithmetic operators
//

CORBA::Fixed operator+(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Fixed operator-(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Fixed operator*(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Fixed operator/(const CORBA::Fixed& a, const CORBA::Fixed& b);

//
// Comparison operators
//

CORBA::Boolean operator> (const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Boolean operator< (const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Boolean operator>=(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Boolean operator<=(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Boolean operator==(const CORBA::Fixed& a, const CORBA::Fixed& b);
CORBA::Boolean operator!=(const CORBA::Fixed& a, const CORBA::Fixed& b);

//
// Stream insertion/extraction operators
//

//?? Not here yet ***


//
// Template class for IDL like typedef fixed<4,2> foo;
//

template<CORBA::UShort idl_digits, CORBA::UShort idl_scale>
class _omni_Fixed : public CORBA::Fixed {

public:
  // Constructors
  _omni_Fixed(int val = 0) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

  _omni_Fixed(unsigned val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

#ifndef OMNI_LONG_IS_INT
  _omni_Fixed(CORBA::Long val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

  _omni_Fixed(CORBA::ULong val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

#endif
#ifdef HAS_LongLong
  _omni_Fixed(CORBA::LongLong val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

  _omni_Fixed(CORBA::ULongLong val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

#endif
#ifndef NO_FLOAT
  _omni_Fixed(CORBA::Double val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

#endif
#ifdef HAS_LongDouble
  _omni_Fixed(CORBA::LongDouble val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

#endif
  _omni_Fixed(const CORBA::Fixed& val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }

  _omni_Fixed(const char* val) :
    CORBA::Fixed(val) { PR_setLimits(idl_digits, idl_scale); }
};

#endif // __fixed_h__
