// -*- Mode: C++; -*-
//                            Package   : omniORB
// streamOperators.h          Created on: 2006/05/23
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2006 Apasphere Ltd.
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
//    Stream operators
//
// $Id$

/*
  $Log$
  Revision 1.1.2.1  2006/05/25 18:16:50  dgrisby
  Standard stream operators for String_var, Fixed, Exception, etc.

*/

#ifndef __OMNI_STREAM_OPERATORS_H__
#define __OMNI_STREAM_OPERATORS_H__

#include <string>
#include <iostream>
#include <cctype>

#ifdef minor
// Can you believe some compilers #define this...
#  undef minor
#endif

inline std::istream& operator>>(std::istream& is,
				CORBA::String_var& val)
{
  std::string s;
  if (is >> s)
    val = (const char*)s.c_str();
  return is;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::String_var& val)
{
  os << (const char*)val;
  return os;
}

inline std::istream& operator>>(std::istream& is,
				CORBA::String_out& val)
{
  std::string s;
  if (is >> s)
    val = (const char*)s.c_str();
  return is;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::String_out& val)
{
  os << (const char*)val._data;
  return os;
}

inline std::istream& operator>>(std::istream& is,
				CORBA::String_member& val)
{
  std::string s;
  if (is >> s)
    val = (const char*)s.c_str();
  return is;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::String_member& val)
{
  os << (const char*)val;
  return os;
}

inline std::istream& operator>>(std::istream& is,
				_CORBA_String_element& val)
{
  std::string s;
  if (is >> s)
    val = (const char*)s.c_str();
  return is;
}

inline std::ostream& operator<<(std::ostream& os,
				const _CORBA_String_element& val)
{
  os << (const char*)val;
  return os;
}

inline std::istream& operator>>(std::istream& is,
				CORBA::Fixed& val)
{
  if (!is.good())
    return is;

  char buf[34]; // plus/minus, up to 31 digits and a decimal point,
		// plus terminating null
  char* c   = buf;
  char* end = buf + sizeof(buf) - 3;
  int decimal = 0;

  is >> *c; // Get first char, skipping white space

  if (*c == '+' || *c == '-') {
    ++c; ++end; *c = is.get();
  }
  if (*c == '.') {
    decimal = 1;
    ++c; *c = is.get();
  }

  if (!isdigit(*c)) {
    is.putback(*c);
    is.setstate(std::ios_base::failbit);
    return is;
  }

  while (++c != end + decimal) {
    *c = is.get();
    if (*c == '.') {
      if (decimal) {
	is.putback(*c);
	break;
      }
      else {
	decimal = 1;
      }
    }
    else if (!isdigit(*c)) {
      is.putback(*c);
      break;
    }
  }
  *c = '\0';
  val.NP_fromString(buf);
  return is;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::Fixed& val)
{
  // The C++ mapping says that this should always obey the stream's
  // precision, but never use scientific notation. Since the default
  // precision is 6, that would mean that by default any fixed value
  // larger than six digits would be shown as an integer, which would
  // not be what most people expect. What we do here is leave the
  // fixed value as-is unless the format flags are set to fixed, in
  // which case we round to the precision.

  CORBA::String_var s;

  if (os.flags() & std::ios_base::fixed) {
    CORBA::Fixed rounded = val.round(os.precision());
    rounded.PR_setLimits(31-os.precision(), os.precision());
    s = rounded.to_string();
  }
  else {
    s = val.NP_asString();
  }
  os << s;
  return os;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::Exception* ex)
{
  const CORBA::SystemException* sex = CORBA::SystemException::_downcast(ex);
  if (sex) {
    os << "CORBA::" << sex->_name() << "(";
    const char* minor = sex->NP_minorString();
    if (minor)
      os << minor;
    else {
      std::ios_base::fmtflags flags = os.flags();
      os << "0x" << std::hex << sex->minor();
      os.flags(flags);
    }
    os << ", CORBA::";
    switch (sex->completed()) {
    case CORBA::COMPLETED_YES:
      os << "COMPLETED_YES";
      break;
    case CORBA::COMPLETED_NO:
      os << "COMPLETED_NO";
      break;
    default:
      os << "COMPLETED_MAYBE";
    }
    os << ")";
  }
  else {
    os << "User exception: " << ex->_rep_id();
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os,
				const CORBA::Exception& ex)
{
  return os << &ex;
}


#ifdef OMNI_WIDE_STREAM_OPERATORS

inline std::wistream& operator>>(std::wistream& is,
				 CORBA::WString_var& val)
{
  std::wstring s;
  if (is >> s)
    val = (const CORBA::WChar*)s.c_str();
  return is;
}

inline std::wostream& operator<<(std::wostream& os,
				 const CORBA::WString_var& val)
{
  os << (const CORBA::WChar*)val;
  return os;
}

inline std::wistream& operator>>(std::wistream& is,
				 CORBA::WString_out& val)
{
  std::wstring s;
  if (is >> s)
    val = (const CORBA::WChar*)s.c_str();
  return is;
}

inline std::wostream& operator<<(std::wostream& os,
				 const CORBA::WString_out& val)
{
  os << (const CORBA::WChar*)val._data;
  return os;
}

inline std::wistream& operator>>(std::wistream& is,
				 CORBA::WString_member& val)
{
  std::wstring s;
  if (is >> s)
    val = (const CORBA::WChar*)s.c_str();
  return is;
}

inline std::wostream& operator<<(std::wostream& os,
				 const CORBA::WString_member& val)
{
  os << (const CORBA::WChar*)val;
  return os;
}

inline std::wistream& operator>>(std::wistream& is,
				 _CORBA_WString_element& val)
{
  std::wstring s;
  if (is >> s)
    val = (const CORBA::WChar*)s.c_str();
  return is;
}

inline std::wostream& operator<<(std::wostream& os,
				 const _CORBA_WString_element& val)
{
  os << (const CORBA::WChar*)val;
  return os;
}

#endif // OMNI_WIDE_STREAM_OPERATORS

#endif // __OMNI_STREAM_OPERATORS_H__
