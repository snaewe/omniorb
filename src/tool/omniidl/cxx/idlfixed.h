// -*- c++ -*-
//                          Package   : omniidl
// idlfixed.h               Created on: 2001/01/30
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//   Implementation of fixed point type

// $Log$
// Revision 1.1.2.1  2001/03/13 10:32:12  dpg1
// Fixed point support.
//

#ifndef _idlfixed_h_
#define _idlfixed_h_

#include <idlsysdep.h>


#ifndef OMNI_FIXED_DIGITS
#define OMNI_FIXED_DIGITS 31
#endif


class IDL_Fixed {
public:

  // Subset of functions from CORBA::Fixed

  IDL_Fixed();
  IDL_Fixed(const IDL_Fixed& f);
  IDL_Fixed(const char* s, const char* file = 0, int line = 0);

  IDL_Fixed(const IDL_Octet* val, IDL_UShort digits,
	    IDL_UShort scale, IDL_Boolean negative);

  ~IDL_Fixed();

  IDL_Fixed truncate(IDL_UShort scale);

  IDL_Fixed& operator=(const IDL_Fixed& f);

  IDL_Fixed operator-() const;

  IDL_UShort fixed_digits() const { return digits_; }
  IDL_UShort fixed_scale()  const { return scale_;  }

  char* asString() const;
  // Return the value of the fixed as a string. Caller frees.

  const IDL_Octet* val()      const { return val_;      }
  IDL_Boolean      negative() const { return negative_; }

  class Overflow     {};
  class DivideByZero {};
  // Exceptions thrown when fixed digits overflow or divide by zero

private:
  IDL_Octet   val_[OMNI_FIXED_DIGITS];
  IDL_UShort  digits_;
  IDL_UShort  scale_;
  IDL_Boolean negative_; // true if negative; false if positive
};

IDL_Fixed operator+(const IDL_Fixed& a, const IDL_Fixed& b);
IDL_Fixed operator-(const IDL_Fixed& a, const IDL_Fixed& b);
IDL_Fixed operator*(const IDL_Fixed& a, const IDL_Fixed& b);
IDL_Fixed operator/(const IDL_Fixed& a, const IDL_Fixed& b);


#endif // _idlfixed_h_
