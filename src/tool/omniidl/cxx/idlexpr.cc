// -*- c++ -*-
//                          Package   : omniidl
// idlexpr.cc               Created on: 1999/10/18
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
//   Expression evaluation functions

// $Id$
// $Log$
// Revision 1.6  2000/07/04 15:22:44  dpg1
// Merge from omni3_develop.
//
// Revision 1.4.2.1  2000/06/27 16:00:17  sll
// Fixes to WIN 32 related build and compiler issues.
//
// Revision 1.4  2000/02/04 12:17:09  dpg1
// Support for VMS.
//
// Revision 1.3  1999/11/02 17:07:26  dpg1
// Changes to compile on Solaris.
//
// Revision 1.2  1999/10/29 10:01:31  dpg1
// Nicer error reporting.
//
// Revision 1.1  1999/10/27 14:05:57  dpg1
// *** empty log message ***
//

#include <idlexpr.h>
#include <idlerr.h>
#include <idlmath.h>
#include <idlast.h>

const _CORBA_WChar EMPTY_WSTRING[] = {'!', 0};

#define EXPR_ERR(rt, fn, str, rv) \
rt IdlExpr::fn() { \
  IdlError(file(), line(), "Cannot interpret %s as " str, errText()); \
  return rv; \
}

// Error functions

EXPR_ERR(_CORBA_Short,        evalAsShort,      "a short",               1)
EXPR_ERR(_CORBA_Long,         evalAsLong,       "a long",                1)
EXPR_ERR(_CORBA_UShort,       evalAsUShort,     "an unsigned short",     1)
EXPR_ERR(_CORBA_ULong,        evalAsULong,      "an unsigned long",      1)
EXPR_ERR(_CORBA_Float,        evalAsFloat,      "a float",               1.0)
EXPR_ERR(_CORBA_Double,       evalAsDouble,     "a double",              1.0)
EXPR_ERR(_CORBA_Boolean,      evalAsBoolean,    "a boolean",             0)
EXPR_ERR(_CORBA_Char,         evalAsChar,       "a character",           '!')
EXPR_ERR(_CORBA_Octet,        evalAsOctet,      "an octet",              1)
EXPR_ERR(const char*,         evalAsString,     "a string",              "!")
#ifdef HAS_LongLong
EXPR_ERR(_CORBA_LongLong,     evalAsLongLong,   "a long long",           1)
EXPR_ERR(_CORBA_ULongLong,    evalAsULongLong,  "an unsigned long long", 1)
#endif
#ifdef HAS_LongDouble
EXPR_ERR(_CORBA_LongDouble,   evalAsLongDouble, "a long double",         1.0)
#endif
EXPR_ERR(_CORBA_WChar,        evalAsWChar,      "a wide character",      '!')
EXPR_ERR(const _CORBA_WChar*, evalAsWString,    "a wide string", EMPTY_WSTRING)

_CORBA_Fixed IdlExpr::evalAsFixed() {
  IdlError(file(), line(), "Fixed is not yet supported");
  return 1;
}

Enumerator*
IdlExpr::
evalAsEnumerator(const Enum* target)
{
  IdlError(file(), line(), "Cannot interpret %s as enumerator", errText());
  return 0;
}


// ScopedName handling
IdlExpr*
IdlExpr::
scopedNameToExpr(const char* file, int line, ScopedName* sn)
{
  const Scope::Entry* se = Scope::current()->findForUse(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL &&
	se->decl()->kind() == Decl::D_ENUMERATOR) {
      return new EnumExpr(file, line, (Enumerator*)se->decl(), sn);
    }
    else if (se->kind() == Scope::Entry::E_DECL &&
	se->decl()->kind() == Decl::D_CONST) {
      return new ConstExpr(file, line, (Const*)se->decl(), sn);
    }
    else {
      char* ssn = sn->toString();
      IdlError(file, line, "`%s' is not valid in an expression", ssn);
      IdlErrorCont(se->file(), se->line(), "(`%s' declared here)", ssn);
      delete [] ssn;
    }
  }
  // If entry was not found, findScopedName() will have reported the error
  return new DummyExpr(file, line);
}

const _CORBA_WChar* DummyExpr::evalAsWString() { return EMPTY_WSTRING; }


// Literals

// Integer literal
_CORBA_Short IntegerExpr::evalAsShort() {
  if (value_ > ((1 << 15) - 1)) {
    IdlError(file(), line(), "Integer literal is too large for short");
    return 1;
  }
  return value_;
}
_CORBA_Long IntegerExpr::evalAsLong() {
  if (value_ > ((1ul << 31) - 1)) {
    IdlError(file(), line(), "Integer literal is too large for long");
    return 1;
  }
  return value_;
}
_CORBA_UShort IntegerExpr::evalAsUShort() {
  if (value_ > ((1 << 16) - 1)) {
    IdlError(file(), line(),
	     "Integer literal is too large for unsigned short");
    return 1;
  }
  return value_;
}
_CORBA_ULong IntegerExpr::evalAsULong() {
#ifdef HAS_LongLong
  if (value_ > 0xffffffff) {
    IdlError(file(), line(), "Integer literal is too large for unsigned long");
    return 1;
  }
#endif
  return value_;
}
_CORBA_Octet IntegerExpr::evalAsOctet() {
  if (value_ > 255) {
    IdlError(file(), line(), "Integer literal is too large for octet");
    return 1;
  }
  return value_;
}
#ifdef HAS_LongLong
_CORBA_LongLong IntegerExpr::evalAsLongLong() {
  return value_;
}
_CORBA_ULongLong IntegerExpr::evalAsULongLong() {
  return value_;
}
#endif

const char* StringExpr::evalAsString() {
  return value_;
}
const _CORBA_WChar* WStringExpr::evalAsWString() {
  return value_;
}
_CORBA_Char CharExpr::evalAsChar() {
  return value_;
}
_CORBA_WChar WCharExpr::evalAsWChar() {
  return value_;
}
_CORBA_Fixed FixedExpr::evalAsFixed() {
  return value_;
}

// Float
_CORBA_Float FloatExpr::evalAsFloat() {

#ifndef _MSC_VER
  // Use direct initialisation except for MS Visual C++, which allegedly
  // does not work properly with types involving built-in types. To
  // play it safe, use copy initialisation instead.
  _CORBA_Float    f(value_);
  IdlFloatLiteral g(f);
#else
  _CORBA_Float    f = value_;
  IdlFloatLiteral g = f;
#endif

  if (f != g)
    IdlWarning(file(), line(), "Loss of precision converting literal "
	       "floating point value to float");
  return f;
}
_CORBA_Double FloatExpr::evalAsDouble() {
  _CORBA_Double   f = value_;
#ifdef HAS_LongDouble
  IdlFloatLiteral g = f;
  if (f != g)
    IdlWarning(file(), line(), "Loss of precision converting literal "
	       "floating point value to double");
#endif
  return f;
}
#ifdef HAS_LongDouble
_CORBA_LongDouble FloatExpr::evalAsLongDouble() {
  return value_;
}
#endif

// Boolean
_CORBA_Boolean BooleanExpr::evalAsBoolean() {
  return value_;
}

// Enumerator
Enumerator* EnumExpr::evalAsEnumerator(const Enum* target) {

  if (value_->container() != target) {
    char* vssn = value_->scopedName()->toString();
    char* essn  = target->scopedName()->toString();
    IdlError(file(), line(), "Enumerator `%s' does not belong to enum `%s'",
	     vssn, essn);
    delete [] essn;
    essn = value_->container()->scopedName()->toString();
    IdlErrorCont(value_->file(), value_->line(),
		 "(Enumerator `%s' declared in `%s' here)",
		 vssn, essn);
    delete [] essn;
    delete [] vssn;
  }
  return value_;
}

// Constant

_CORBA_Short ConstExpr::evalAsShort() {
  _CORBA_Short   r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short: r = c_->constAsShort(); break;
  case IdlType::tk_long: {
    _CORBA_Long z = c_->constAsLong();
    r = z; p = ((z >= -0x8000) && (z <= 0x7fff));
    break;
  }
  case IdlType::tk_ushort: {
    _CORBA_UShort z = c_->constAsUShort();
    r = z; p = (z <= 0x7fff);
    break;
  }
  case IdlType::tk_ulong: {
    _CORBA_ULong z = c_->constAsULong();
    r = z; p = (z <= 0x7fff);
    break;
  }
  case IdlType::tk_octet: r = c_->constAsOctet(); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = ((z >= -0x8000) && (z <= 0x7fff));
    break;
  }
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0x7fff);
    break;
  }
#endif
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Cannot interpret constant `%s' as short", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of short", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_Long ConstExpr::evalAsLong() {
  _CORBA_Long    r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short:  r = c_->constAsShort(); break;
  case IdlType::tk_long:   r = c_->constAsLong();  break;
  case IdlType::tk_ushort: r = c_->constAsUShort(); break;
  case IdlType::tk_ulong: {
    _CORBA_ULong z = c_->constAsULong();
    r = z; p = (z <= 0x7fffffff);
    break;
  }
  case IdlType::tk_octet: r = c_->constAsOctet(); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = ((z >= -0x80000000) && (z <= 0x7fffffff));
    break;
  }
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0x7fffffff);
    break;
  }
#endif
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Cannot interpret constant `%s' as long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_UShort ConstExpr::evalAsUShort() {
  _CORBA_UShort  r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short: {
    _CORBA_Short z = c_->constAsShort();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_long: {
    _CORBA_Long z = c_->constAsLong();
    r = z; p = ((z >= 0) && (z <= 0xffff));
    break;
  }
  case IdlType::tk_ushort: r = c_->constAsUShort(); break;
  case IdlType::tk_ulong: {
    _CORBA_ULong z = c_->constAsULong();
    r = z; p = (z <= 0xffff);
    break;
  }
  case IdlType::tk_octet: r = c_->constAsOctet(); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = ((z >= 0) && (z <= 0xffff));
    break;
  }
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0xffff);
    break;
  }
#endif
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as unsigned short", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of unsigned short",
	     ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_ULong ConstExpr::evalAsULong() {
  _CORBA_ULong   r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short: {
    _CORBA_Short z = c_->constAsShort();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_long: {
    _CORBA_Long z = c_->constAsLong();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_ushort: r = c_->constAsUShort(); break;
  case IdlType::tk_ulong:  r = c_->constAsULong();  break;
  case IdlType::tk_octet:  r = c_->constAsOctet(); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = ((z >= 0) && (z <= 0xffffffff));
    break;
  }
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0xffffffff);
    break;
  }
#endif
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as unsigned long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of unsigned long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_Octet ConstExpr::evalAsOctet() {
  _CORBA_Octet   r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short: {
    _CORBA_Short z = c_->constAsShort();
    r = z; p = ((z >= 0) && (z < 0xff));
    break;
  }
  case IdlType::tk_long: {
    _CORBA_Long z = c_->constAsLong();
    r = z; p = ((z >= 0) && (z <= 0xff));
    break;
  }
  case IdlType::tk_ushort: {
    _CORBA_UShort z = c_->constAsUShort();
    r = z; p = (z <= 0xff);
    break;
  }
  case IdlType::tk_ulong: {
    _CORBA_ULong z = c_->constAsULong();
    r = z; p = (z <= 0xff);
    break;
  }
  case IdlType::tk_octet: r = c_->constAsOctet(); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = ((z >= 0) && (z <= 0xff));
    break;
  }
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0xff);
    break;
  }
#endif
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Cannot interpret constant `%s' as octet", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of octet", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

#ifdef HAS_LongLong

_CORBA_LongLong ConstExpr::evalAsLongLong() {
  _CORBA_LongLong r;
  _CORBA_Boolean  p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short:  r = c_->constAsShort(); break;
  case IdlType::tk_long:   r = c_->constAsLong();  break;
  case IdlType::tk_ushort: r = c_->constAsUShort(); break;
  case IdlType::tk_ulong:  r = c_->constAsULong(); break;
  case IdlType::tk_octet:  r = c_->constAsOctet(); break;
  case IdlType::tk_longlong: r = c_->constAsLongLong(); break;
  case IdlType::tk_ulonglong: {
    _CORBA_ULongLong z = c_->constAsULongLong();
    r = z; p = (z <= 0x7fffffffffffffffLL);
    break;
  }
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as long long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of long long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_ULongLong ConstExpr::evalAsULongLong() {
  _CORBA_Short   r;
  _CORBA_Boolean p = 1;

  switch (c_->constKind()) {
  case IdlType::tk_short: {
    _CORBA_Short z = c_->constAsShort();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_long: {
    _CORBA_Long z = c_->constAsLong();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_ushort: r = c_->constAsUShort(); break;
  case IdlType::tk_ulong:  r = c_->constAsULong();  break;
  case IdlType::tk_octet:  r = c_->constAsOctet(); break;
  case IdlType::tk_longlong: {
    _CORBA_LongLong z = c_->constAsLongLong();
    r = z; p = (z >= 0);
    break;
  }
  case IdlType::tk_ulonglong: r = c_->constAsULongLong(); break;
  default:
    r = 1; p = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as unsigned long long", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (!p) {
    r = 1;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' exceeds precision of unsigned long long",
	     ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}
#endif // HAS_LongLong

_CORBA_Float ConstExpr::evalAsFloat() {
  _CORBA_Float r;

  switch (c_->constKind()) {
  case IdlType::tk_float:      r = c_->constAsFloat();      break;
#ifndef __VMS
  case IdlType::tk_double:     r = c_->constAsDouble();     break;
#else
  case IdlType::tk_double:     r = (float)(double)c_->constAsDouble(); break;
#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble: r = c_->constAsLongDouble(); break;
#endif
  default:
    r = 1.0;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Cannot interpret constant `%s' as float", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Value of constant `%s' overflows float", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

_CORBA_Double ConstExpr::evalAsDouble() {
  _CORBA_Double r;

  switch (c_->constKind()) {
#ifndef __VMS
  case IdlType::tk_float:      r = c_->constAsFloat();      break;
#else
  case IdlType::tk_float:      r = (double)(float)c_->constAsFloat(); break;
#endif
  case IdlType::tk_double:     r = c_->constAsDouble();     break;
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble: r = c_->constAsLongDouble(); break;
#endif
  default:
    r = 1.0;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Cannot interpret constant `%s' as double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Value of constant `%s' overflows double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

#ifdef HAS_LongDouble
_CORBA_LongDouble ConstExpr::evalAsLongDouble() {
  _CORBA_LongDouble r;

  switch (c_->constKind()) {
  case IdlType::tk_float:      r = c_->constAsFloat();      break;
  case IdlType::tk_double:     r = c_->constAsDouble();     break;
  case IdlType::tk_longdouble: r = c_->constAsLongDouble(); break;
  default:
    r = 1.0;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as long double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) { // Don't see how this could happen...
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant `%s' overflows long double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}
#endif // HAS_LongDouble


#define CONST_EXPR_EVAL(rt, eop, tk, cop, str, rv) \
rt ConstExpr::eop() { \
  if (c_->constKind() == IdlType::tk) \
    return c_->cop(); \
  else { \
    char* ssn = scopedName_->toString(); \
    IdlError(file(), line(), \
	     "Cannot interpret constant `%s' as " str, ssn); \
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn); \
    delete [] ssn; \
  } \
  return rv; \
}

CONST_EXPR_EVAL(_CORBA_Boolean, evalAsBoolean, tk_boolean,
		constAsBoolean, "boolean", 0)
CONST_EXPR_EVAL(_CORBA_Char, evalAsChar, tk_char,
		constAsChar, "character", '!')
CONST_EXPR_EVAL(const char*, evalAsString, tk_string,
		constAsString, "string", "!")
CONST_EXPR_EVAL(_CORBA_WChar, evalAsWChar, tk_wchar,
		constAsWChar, "wide character", '!')
CONST_EXPR_EVAL(const _CORBA_WChar*, evalAsWString, tk_wstring,
		constAsWString, "wide string", EMPTY_WSTRING)
CONST_EXPR_EVAL(_CORBA_Fixed, evalAsFixed, tk_fixed,
		constAsFixed, "fixed", 0)

Enumerator* ConstExpr::evalAsEnumerator(const Enum* target) {
  if (c_->constKind() == IdlType::tk_enum) {

    Enumerator* e = c_->constAsEnumerator();
    if (e->container() != target) {
      char* ssn = target->scopedName()->toString();
      IdlError(file(), line(), "Enumerator `%s' does not belong to enum `%s'",
	       e->identifier(), ssn);
      delete [] ssn;
      ssn = e->container()->scopedName()->toString();
      IdlErrorCont(e->file(), e->line(),
		   "(Enumerator `%s' declared in `%s' here)",
		   e->identifier(), ssn);
      delete [] ssn;
    }
    return c_->constAsEnumerator();
  }
  else {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant `%s' as enumerator", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return 0;
}


// Binary expressions

// Or
#define OR_EXPR_EVAL(ret, op) \
ret OrExpr::op() { \
  return a_->op() | b_->op(); \
}
OR_EXPR_EVAL(_CORBA_Short,     evalAsShort)
OR_EXPR_EVAL(_CORBA_Long,      evalAsLong)
OR_EXPR_EVAL(_CORBA_UShort,    evalAsUShort)
OR_EXPR_EVAL(_CORBA_ULong,     evalAsULong)
OR_EXPR_EVAL(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
OR_EXPR_EVAL(_CORBA_LongLong,  evalAsLongLong)
OR_EXPR_EVAL(_CORBA_ULongLong, evalAsULongLong)
#endif

// Xor
#define XOR_EXPR_EVAL(ret, op) \
ret XorExpr::op() { \
  return a_->op() ^ b_->op(); \
}
XOR_EXPR_EVAL(_CORBA_Short,     evalAsShort)
XOR_EXPR_EVAL(_CORBA_Long,      evalAsLong)
XOR_EXPR_EVAL(_CORBA_UShort,    evalAsUShort)
XOR_EXPR_EVAL(_CORBA_ULong,     evalAsULong)
XOR_EXPR_EVAL(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
XOR_EXPR_EVAL(_CORBA_LongLong,  evalAsLongLong)
XOR_EXPR_EVAL(_CORBA_ULongLong, evalAsULongLong)
#endif

// And
#define AND_EXPR_EVAL(ret, op) \
ret AndExpr::op() { \
  return a_->op() & b_->op(); \
}
AND_EXPR_EVAL(_CORBA_Short,     evalAsShort)
AND_EXPR_EVAL(_CORBA_Long,      evalAsLong)
AND_EXPR_EVAL(_CORBA_UShort,    evalAsUShort)
AND_EXPR_EVAL(_CORBA_ULong,     evalAsULong)
AND_EXPR_EVAL(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
AND_EXPR_EVAL(_CORBA_LongLong,  evalAsLongLong)
AND_EXPR_EVAL(_CORBA_ULongLong, evalAsULongLong)
#endif

// Right shift
#define RSHIFT_EXPR_EVAL(ret, op) \
ret RShiftExpr::op() { \
  _CORBA_Long shift = b_->evalAsLong(); \
  if (shift < 0 || shift >= 64) { \
    IdlError(file(), line(), \
	     "Right operand of shift operation must be >= 0 and < 64"); \
    shift = 1; \
  } \
  return a_->op() >> shift; \
}
RSHIFT_EXPR_EVAL(_CORBA_Short,     evalAsShort)
RSHIFT_EXPR_EVAL(_CORBA_Long,      evalAsLong)
RSHIFT_EXPR_EVAL(_CORBA_UShort,    evalAsUShort)
RSHIFT_EXPR_EVAL(_CORBA_ULong,     evalAsULong)
RSHIFT_EXPR_EVAL(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
RSHIFT_EXPR_EVAL(_CORBA_LongLong,  evalAsLongLong)
RSHIFT_EXPR_EVAL(_CORBA_ULongLong, evalAsULongLong)
#endif

// Left shift
#define LSHIFT_EXPR_EVAL(ret, op) \
ret LShiftExpr::op() { \
  _CORBA_Long shift = b_->evalAsLong(); \
  if (shift < 0 || shift >= 64) { \
    IdlError(file(), line(), \
	     "Right operand of shift operation must be >= 0 and < 64"); \
    shift = 1; \
  } \
  return a_->op() << shift; \
}
LSHIFT_EXPR_EVAL(_CORBA_Short,     evalAsShort)
LSHIFT_EXPR_EVAL(_CORBA_Long,      evalAsLong)
LSHIFT_EXPR_EVAL(_CORBA_UShort,    evalAsUShort)
LSHIFT_EXPR_EVAL(_CORBA_ULong,     evalAsULong)
LSHIFT_EXPR_EVAL(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
LSHIFT_EXPR_EVAL(_CORBA_LongLong,  evalAsLongLong)
LSHIFT_EXPR_EVAL(_CORBA_ULongLong, evalAsULongLong)
#endif


// %
#define MOD_EXPR_EVAL_S(ret, op) \
ret ModExpr::op() { \
  ret a, b; \
  a = a_->op(); b = b_->op(); \
  if (b == 0) { \
    IdlError(file(), line(), "Remainder of division by 0 is undefined"); \
    return 1; \
  } \
  if (a < 0 || b < 0) { \
    IdlWarning(file(), line(), "Result of %% operator involving negative " \
	       "operands is platform dependent"); \
  } \
  return a % b; \
}
#define MOD_EXPR_EVAL_U(ret, op) \
ret ModExpr::op() { \
  ret a, b; \
  a = a_->op(); b = b_->op(); \
  if (b == 0) { \
    IdlError(file(), line(), "Remainder of division by 0 is undefined"); \
    return 1; \
  } \
  return a % b; \
}
MOD_EXPR_EVAL_S(_CORBA_Short,     evalAsShort)
MOD_EXPR_EVAL_S(_CORBA_Long,      evalAsLong)
MOD_EXPR_EVAL_U(_CORBA_UShort,    evalAsUShort)
MOD_EXPR_EVAL_U(_CORBA_ULong,     evalAsULong)
MOD_EXPR_EVAL_U(_CORBA_Octet,     evalAsOctet)
#ifdef HAS_LongLong
MOD_EXPR_EVAL_S(_CORBA_LongLong,  evalAsLongLong)
MOD_EXPR_EVAL_U(_CORBA_ULongLong, evalAsULongLong)
#endif


// Add
#define ADD_EXPR_EVAL_S(ret, op, str) \
ret AddExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a+b; \
  if ((r^a) < 0 && (r^b) < 0) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define ADD_EXPR_EVAL_U(ret, op, str) \
ret AddExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a+b; \
  if (r < a) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define ADD_EXPR_EVAL_F(ret, op, str) \
ret AddExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a+b; \
  if (IdlFPOverflow(r)) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1.0; \
  } \
  return r; \
}
ADD_EXPR_EVAL_S(_CORBA_Short,      evalAsShort,      "short")
ADD_EXPR_EVAL_S(_CORBA_Long,       evalAsLong,       "long")
ADD_EXPR_EVAL_U(_CORBA_UShort,     evalAsUShort,     "unsigned short")
ADD_EXPR_EVAL_U(_CORBA_ULong,      evalAsULong,      "unsigned long")
ADD_EXPR_EVAL_U(_CORBA_Octet,      evalAsOctet,      "octet")
#ifdef HAS_LongLong
ADD_EXPR_EVAL_S(_CORBA_LongLong,   evalAsLongLong,   "long long")
ADD_EXPR_EVAL_U(_CORBA_ULongLong,  evalAsULongLong,  "unsigned long long")
#endif
ADD_EXPR_EVAL_F(_CORBA_Float,      evalAsFloat,      "float")
ADD_EXPR_EVAL_F(_CORBA_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
ADD_EXPR_EVAL_F(_CORBA_LongDouble, evalAsLongDouble, "long double")
#endif


// Sub
#define SUB_EXPR_EVAL_S(ret, op, str) \
ret SubExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a-b; \
  if ((r^a) < 0 && (r^-b) < 0) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define SUB_EXPR_EVAL_U(ret, op, str) \
ret SubExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a-b; \
  if (r > a) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define SUB_EXPR_EVAL_F(ret, op, str) \
ret SubExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a-b; \
  if (IdlFPOverflow(r)) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1.0; \
  } \
  return r; \
}
SUB_EXPR_EVAL_S(_CORBA_Short,      evalAsShort,      "short")
SUB_EXPR_EVAL_S(_CORBA_Long,       evalAsLong,       "long")
SUB_EXPR_EVAL_U(_CORBA_UShort,     evalAsUShort,     "unsigned short")
SUB_EXPR_EVAL_U(_CORBA_ULong,      evalAsULong,      "unsigned long")
SUB_EXPR_EVAL_U(_CORBA_Octet,      evalAsOctet,      "octet")
#ifdef HAS_LongLong
SUB_EXPR_EVAL_S(_CORBA_LongLong,   evalAsLongLong,   "long long")
SUB_EXPR_EVAL_U(_CORBA_ULongLong,  evalAsULongLong,  "unsigned long long")
#endif
SUB_EXPR_EVAL_F(_CORBA_Float,      evalAsFloat,      "float")
SUB_EXPR_EVAL_F(_CORBA_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
SUB_EXPR_EVAL_F(_CORBA_LongDouble, evalAsLongDouble, "long double")
#endif


// Mult
#define MULT_EXPR_EVAL_S(ret, op, str) \
ret MultExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a*b; \
  if (b != 0 && (r/b) != a) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define MULT_EXPR_EVAL_U(ret, op, str) \
ret MultExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a*b; \
  if (b != 0 && (r/b) != a) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1; \
  } \
  return r; \
}
#define MULT_EXPR_EVAL_F(ret, op, str) \
ret MultExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a*b; \
  if (IdlFPOverflow(r)) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1.0; \
  } \
  return r; \
}
MULT_EXPR_EVAL_S(_CORBA_Short,      evalAsShort,      "short")
MULT_EXPR_EVAL_S(_CORBA_Long,       evalAsLong,       "long")
MULT_EXPR_EVAL_U(_CORBA_UShort,     evalAsUShort,     "unsigned short")
MULT_EXPR_EVAL_U(_CORBA_ULong,      evalAsULong,      "unsigned long")
MULT_EXPR_EVAL_U(_CORBA_Octet,      evalAsOctet,      "octet")
#ifdef HAS_LongLong
MULT_EXPR_EVAL_S(_CORBA_LongLong,   evalAsLongLong,   "long long")
MULT_EXPR_EVAL_U(_CORBA_ULongLong,  evalAsULongLong,  "unsigned long long")
#endif
MULT_EXPR_EVAL_F(_CORBA_Float,      evalAsFloat,      "float")
MULT_EXPR_EVAL_F(_CORBA_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
MULT_EXPR_EVAL_F(_CORBA_LongDouble, evalAsLongDouble, "long double")
#endif


// Div
#define DIV_EXPR_EVAL_S(ret, op) \
ret DivExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  if (b == 0) { \
    IdlError(file(), line(), "Divide by zero"); \
    b = 1; \
  } \
  r = a/b; \
  return r; \
}
#define DIV_EXPR_EVAL_U(ret, op) \
ret DivExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  if (b == 0) { \
    IdlError(file(), line(), "Divide by zero"); \
    b = 1; \
  } \
  r = a/b; \
  return r; \
}
#define DIV_EXPR_EVAL_F(ret, op, str) \
ret DivExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  if (b == 0.0) { \
    IdlError(file(), line(), "Divide by zero"); \
    b = 1.0; \
  } \
  r = a/b; \
  if (IdlFPOverflow(r)) { \
    IdlError(file(), line(), "Sub-expression overflows " str); \
    r = 1.0; \
  } \
  return r; \
}
DIV_EXPR_EVAL_S(_CORBA_Short,      evalAsShort)
DIV_EXPR_EVAL_S(_CORBA_Long,       evalAsLong)
DIV_EXPR_EVAL_U(_CORBA_UShort,     evalAsUShort)
DIV_EXPR_EVAL_U(_CORBA_ULong,      evalAsULong)
DIV_EXPR_EVAL_U(_CORBA_Octet,      evalAsOctet)
#ifdef HAS_LongLong
DIV_EXPR_EVAL_S(_CORBA_LongLong,   evalAsLongLong)
DIV_EXPR_EVAL_U(_CORBA_ULongLong,  evalAsULongLong)
#endif
DIV_EXPR_EVAL_F(_CORBA_Float,      evalAsFloat,      "float")
DIV_EXPR_EVAL_F(_CORBA_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
DIV_EXPR_EVAL_F(_CORBA_LongDouble, evalAsLongDouble, "long double")
#endif

// Invert
#define INVERT_EXPR_EVAL(ret, op) \
ret InvertExpr::op() { \
  return ~(e_->op()); \
}
INVERT_EXPR_EVAL(_CORBA_Short,      evalAsShort)
INVERT_EXPR_EVAL(_CORBA_Long,       evalAsLong)
INVERT_EXPR_EVAL(_CORBA_UShort,     evalAsUShort)
INVERT_EXPR_EVAL(_CORBA_ULong,      evalAsULong)
INVERT_EXPR_EVAL(_CORBA_Octet,      evalAsOctet)
#ifdef HAS_LongLong
INVERT_EXPR_EVAL(_CORBA_LongLong,   evalAsLongLong)
INVERT_EXPR_EVAL(_CORBA_ULongLong,  evalAsULongLong)
#endif

// Minus
#define MINUS_EXPR_EVAL(ret, op) \
ret MinusExpr::op() { \
  return -(e_->op()); \
}
MINUS_EXPR_EVAL(_CORBA_Short,      evalAsShort)
MINUS_EXPR_EVAL(_CORBA_Long,       evalAsLong)
#ifdef HAS_LongLong
MINUS_EXPR_EVAL(_CORBA_LongLong,   evalAsLongLong)
#endif
MINUS_EXPR_EVAL(_CORBA_Float,      evalAsFloat)
MINUS_EXPR_EVAL(_CORBA_Double,     evalAsDouble)
#ifdef HAS_LongDouble
MINUS_EXPR_EVAL(_CORBA_LongDouble, evalAsLongDouble)
#endif

// Plus
#define PLUS_EXPR_EVAL(ret, op) \
ret PlusExpr::op() { \
  return +(e_->op()); \
}
PLUS_EXPR_EVAL(_CORBA_Short,      evalAsShort)
PLUS_EXPR_EVAL(_CORBA_Long,       evalAsLong)
PLUS_EXPR_EVAL(_CORBA_UShort,     evalAsUShort)
PLUS_EXPR_EVAL(_CORBA_ULong,      evalAsULong)
PLUS_EXPR_EVAL(_CORBA_Octet,      evalAsOctet)
#ifdef HAS_LongLong
PLUS_EXPR_EVAL(_CORBA_LongLong,   evalAsLongLong)
PLUS_EXPR_EVAL(_CORBA_ULongLong,  evalAsULongLong)
#endif
PLUS_EXPR_EVAL(_CORBA_Float,      evalAsFloat)
PLUS_EXPR_EVAL(_CORBA_Double,     evalAsDouble)
#ifdef HAS_LongDouble
PLUS_EXPR_EVAL(_CORBA_LongDouble, evalAsLongDouble)
#endif
