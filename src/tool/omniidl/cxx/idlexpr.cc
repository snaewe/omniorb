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
// Revision 1.10.2.1  2003/03/23 21:01:45  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.6.2.6  2001/10/17 16:48:33  dpg1
// Minor error message tweaks
//
// Revision 1.6.2.5  2001/08/29 11:54:20  dpg1
// Clean up const handling in IDL compiler.
//
// Revision 1.6.2.4  2001/03/13 10:32:11  dpg1
// Fixed point support.
//
// Revision 1.6.2.3  2000/10/27 16:31:09  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.6.2.2  2000/10/10 10:18:50  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.4.2.2  2000/08/07 15:34:36  dpg1
// Partial back-port of long long from omni3_1_develop.
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

const IDL_WChar EMPTY_WSTRING[] = {'!', 0};

#define EXPR_ERR(rt, fn, str, rv) \
rt IdlExpr::fn() { \
  IdlError(file(), line(), "Cannot interpret %s as " str, errText()); \
  return rv; \
}

// Error functions

EXPR_ERR(IdlLongVal, evalAsLongV, "an integer", IdlLongVal((IDL_ULong)1))
#ifdef HAS_LongLong
EXPR_ERR(IdlLongLongVal, evalAsLongLongV, "an integer",
         IdlLongLongVal((IDL_ULongLong)1))
#endif

EXPR_ERR(IDL_Float,        evalAsFloat,      "a float",               1.0)
EXPR_ERR(IDL_Double,       evalAsDouble,     "a double",              1.0)
EXPR_ERR(IDL_Boolean,      evalAsBoolean,    "a boolean",             0)
EXPR_ERR(IDL_Char,         evalAsChar,       "a character",           '!')
EXPR_ERR(const char*,      evalAsString,     "a string",              "!")
#ifdef HAS_LongDouble
EXPR_ERR(IDL_LongDouble,   evalAsLongDouble, "a long double",         1.0)
#endif
EXPR_ERR(IDL_WChar,        evalAsWChar,      "a wide character",      '!')
EXPR_ERR(const IDL_WChar*, evalAsWString,    "a wide string", EMPTY_WSTRING)
EXPR_ERR(IDL_Fixed*,       evalAsFixed,      "fixed point", new IDL_Fixed("1"))

Enumerator*
IdlExpr::
evalAsEnumerator(const Enum* target)
{
  IdlError(file(), line(), "Cannot interpret %s as enumerator", errText());
  return 0;
}

//
// Conversions to IDL integer types

IDL_Short IdlExpr::evalAsShort()
{
  IdlLongVal v = evalAsLongV();
  if (v.negative) {
    if (v.s < -0x8000)
      IdlError(file(), line(), "Value too small for short");
    return v.s;
  }
  else {
    if (v.u > 0x7fff)
      IdlError(file(), line(), "Value too large for short");
    return v.u;
  }
}
IDL_Long IdlExpr::evalAsLong()
{
  IdlLongVal v = evalAsLongV();
  if (v.negative) {
    return v.s;
  }
  else {
    if (v.u > 0x7fffffff)
      IdlError(file(), line(), "Value too large for long");
    return v.u;
  }
}
IDL_UShort IdlExpr::evalAsUShort()
{
  IdlLongVal v = evalAsLongV();

  if (v.negative) {
    IdlError(file(), line(), "Value too small for unsigned short");
  }
  else if (v.u > 0xffff) {
    IdlError(file(), line(), "Value too large for unsigned short");
  }
  return v.u;
}
IDL_ULong IdlExpr::evalAsULong()
{
  IdlLongVal v = evalAsLongV();

  if (v.negative) {
    IdlError(file(), line(), "Value too small for unsigned long");
  }
  return v.u;
}
IDL_Octet IdlExpr::evalAsOctet()
{
  IdlLongVal v = evalAsLongV();

  if (v.negative) {
    IdlError(file(), line(), "Value too small for octet");
  }
  else if (v.u > 0xff) {
    IdlError(file(), line(), "Value too large for octet");
  }
  return v.u;
}
#ifdef HAS_LongLong
IDL_LongLong IdlExpr::evalAsLongLong()
{
  IdlLongLongVal v = evalAsLongLongV();
  if (v.negative) {
    return v.s;
  }
  else {
    if (v.u > _CORBA_LONGLONG_CONST(0x7fffffffffffffff))
      IdlError(file(), line(), "Value too large for long long");
    return v.u;
  }
}
IDL_ULongLong IdlExpr::evalAsULongLong()
{
  IdlLongLongVal v = evalAsLongLongV();

  if (v.negative) {
    IdlError(file(), line(), "Value too small for unsigned long long");
  }
  return v.u;
}
#endif

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
      IdlError(file, line, "'%s' is not valid in an expression", ssn);
      IdlErrorCont(se->file(), se->line(), "('%s' declared here)", ssn);
      delete [] ssn;
    }
  }
  // If entry was not found, findScopedName() will have reported the error
  return new DummyExpr(file, line);
}

const IDL_WChar* DummyExpr::evalAsWString() { return EMPTY_WSTRING; }


// Literals

IdlLongVal IntegerExpr::evalAsLongV() {
#ifdef HAS_LongLong
  if (value_ > 0xffffffff) {
    IdlError(file(), line(), "Integer literal is too large for unsigned long");
    return IdlLongVal((IDL_ULong)1);
  }
#endif
  return IdlLongVal((IDL_ULong)value_);
}
#ifdef HAS_LongLong
IdlLongLongVal IntegerExpr::evalAsLongLongV() {
  return IdlLongLongVal((IDL_ULongLong)value_);
}
#endif

const char* StringExpr::evalAsString() {
  return value_;
}
const IDL_WChar* WStringExpr::evalAsWString() {
  return value_;
}
IDL_Char CharExpr::evalAsChar() {
  return value_;
}
IDL_WChar WCharExpr::evalAsWChar() {
  return value_;
}
IDL_Fixed* FixedExpr::evalAsFixed() {
  return value_;
}

// Float
IDL_Float FloatExpr::evalAsFloat() {

#ifndef _MSC_VER
  // Use direct initialisation except for MS Visual C++, which allegedly
  // does not work properly with types involving built-in types. To
  // play it safe, use copy initialisation instead.
  IDL_Float    f(value_);
  IdlFloatLiteral g(f);
#else
  IDL_Float    f = value_;
  IdlFloatLiteral g = f;
#endif

  if (f != g)
    IdlWarning(file(), line(), "Loss of precision converting literal "
	       "floating point value to float");
  return f;
}
IDL_Double FloatExpr::evalAsDouble() {
  IDL_Double   f = value_;
#ifdef HAS_LongDouble
  IdlFloatLiteral g = f;
  if (f != g)
    IdlWarning(file(), line(), "Loss of precision converting literal "
	       "floating point value to double");
#endif
  return f;
}
#ifdef HAS_LongDouble
IDL_LongDouble FloatExpr::evalAsLongDouble() {
  return value_;
}
#endif

// Boolean
IDL_Boolean BooleanExpr::evalAsBoolean() {
  return value_;
}

// Enumerator
Enumerator* EnumExpr::evalAsEnumerator(const Enum* target) {

  if (value_->container() != target) {
    char* vssn = value_->scopedName()->toString();
    char* essn  = target->scopedName()->toString();
    IdlError(file(), line(), "Enumerator '%s' does not belong to enum '%s'",
	     vssn, essn);
    delete [] essn;
    essn = value_->container()->scopedName()->toString();
    IdlErrorCont(value_->file(), value_->line(),
		 "(Enumerator '%s' declared in '%s' here)",
		 vssn, essn);
    delete [] essn;
    delete [] vssn;
  }
  return value_;
}

// Constant

IdlLongVal ConstExpr::evalAsLongV() {
  switch (c_->constKind()) {

  case IdlType::tk_short:  return IdlLongVal(IDL_Long (c_->constAsShort()));
  case IdlType::tk_long:   return IdlLongVal(IDL_Long (c_->constAsLong()));
  case IdlType::tk_ushort: return IdlLongVal(IDL_ULong(c_->constAsUShort()));
  case IdlType::tk_ulong:  return IdlLongVal(IDL_ULong(c_->constAsULong()));
  case IdlType::tk_octet:  return IdlLongVal(IDL_ULong(c_->constAsOctet()));

#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    {
      IDL_LongLong v = c_->constAsLongLong();
      if (v < -0x80000000 || v > 0xffffffff) goto precision_error;
      if (v >= 0)
	return IdlLongVal(IDL_ULong(v));
      else
	return IdlLongVal(IDL_Long(v));
    }
  case IdlType::tk_ulonglong:
    {
      IDL_ULongLong v = c_->constAsULongLong();
      if (v > 0xffffffff) goto precision_error;
      return IdlLongVal(IDL_ULong(v));
    }
#endif
  default:
    {
      char* ssn = scopedName_->toString();
      IdlError(file(), line(),
	       "Cannot interpret constant '%s' as an integer", ssn);
      IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
      delete [] ssn;
      return IdlLongVal((IDL_ULong)1);
    }
  }
 precision_error:
  char* ssn = scopedName_->toString();
  IdlError(file(), line(),
	   "Value of constant '%s' exceeds precision of target", ssn);
  IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
  delete [] ssn;
  return IdlLongVal(IDL_ULong(1));
}

#ifdef HAS_LongLong

IdlLongLongVal ConstExpr::evalAsLongLongV() {
  switch (c_->constKind()) {

  case IdlType::tk_short:
    return IdlLongLongVal(IDL_LongLong(c_->constAsShort()));

  case IdlType::tk_long:
    return IdlLongLongVal(IDL_LongLong(c_->constAsLong()));

  case IdlType::tk_ushort:
    return IdlLongLongVal(IDL_ULongLong(c_->constAsUShort()));

  case IdlType::tk_ulong:
    return IdlLongLongVal(IDL_ULongLong(c_->constAsULong()));

  case IdlType::tk_octet:
    return IdlLongLongVal(IDL_ULongLong(c_->constAsOctet()));

  case IdlType::tk_longlong:
    return IdlLongLongVal(c_->constAsLongLong());

  case IdlType::tk_ulonglong:
    return IdlLongLongVal(c_->constAsULongLong());

  default:
    {
      char* ssn = scopedName_->toString();
      IdlError(file(), line(),
	       "Cannot interpret constant '%s' as an integer", ssn);
      IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
      delete [] ssn;
      return IdlLongLongVal((IDL_ULongLong)1);
    }
  }
}
#endif // HAS_LongLong


IDL_Float ConstExpr::evalAsFloat() {
  IDL_Float r;

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
    IdlError(file(), line(), "Cannot interpret constant '%s' as float", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Value of constant '%s' overflows float", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

IDL_Double ConstExpr::evalAsDouble() {
  IDL_Double r;

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
    IdlError(file(), line(), "Cannot interpret constant '%s' as double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(), "Value of constant '%s' overflows double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return r;
}

#ifdef HAS_LongDouble
IDL_LongDouble ConstExpr::evalAsLongDouble() {
  IDL_LongDouble r;

  switch (c_->constKind()) {
  case IdlType::tk_float:      r = c_->constAsFloat();      break;
  case IdlType::tk_double:     r = c_->constAsDouble();     break;
  case IdlType::tk_longdouble: r = c_->constAsLongDouble(); break;
  default:
    r = 1.0;
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant '%s' as long double", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  if (IdlFPOverflow(r)) { // Don't see how this could happen...
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Value of constant '%s' overflows long double", ssn);
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
	     "Cannot interpret constant '%s' as " str, ssn); \
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn); \
    delete [] ssn; \
  } \
  return rv; \
}

CONST_EXPR_EVAL(IDL_Boolean, evalAsBoolean, tk_boolean,
		constAsBoolean, "boolean", 0)
CONST_EXPR_EVAL(IDL_Char, evalAsChar, tk_char,
		constAsChar, "character", '!')
CONST_EXPR_EVAL(const char*, evalAsString, tk_string,
		constAsString, "string", "!")
CONST_EXPR_EVAL(IDL_WChar, evalAsWChar, tk_wchar,
		constAsWChar, "wide character", '!')
CONST_EXPR_EVAL(const IDL_WChar*, evalAsWString, tk_wstring,
		constAsWString, "wide string", EMPTY_WSTRING)
CONST_EXPR_EVAL(IDL_Fixed*, evalAsFixed, tk_fixed,
		constAsFixed, "fixed", new IDL_Fixed("1"))

Enumerator* ConstExpr::evalAsEnumerator(const Enum* target) {
  if (c_->constKind() == IdlType::tk_enum) {

    Enumerator* e = c_->constAsEnumerator();
    if (e->container() != target) {
      char* ssn = target->scopedName()->toString();
      IdlError(file(), line(), "Enumerator '%s' does not belong to enum '%s'",
	       e->identifier(), ssn);
      delete [] ssn;
      ssn = e->container()->scopedName()->toString();
      IdlErrorCont(e->file(), e->line(),
		   "(Enumerator '%s' declared in '%s' here)",
		   e->identifier(), ssn);
      delete [] ssn;
    }
    return c_->constAsEnumerator();
  }
  else {
    char* ssn = scopedName_->toString();
    IdlError(file(), line(),
	     "Cannot interpret constant '%s' as enumerator", ssn);
    IdlErrorCont(c_->file(), c_->line(), "(%s declared here)", ssn);
    delete [] ssn;
  }
  return 0;
}


// Binary expressions

// Or
IdlLongVal OrExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (a.negative)
    return IdlLongVal(IDL_Long(a.s | b.s));
  else
    return IdlLongVal(IDL_ULong(a.u | b.u));
}
#ifdef HAS_LongLong
IdlLongLongVal OrExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (a.negative)
    return IdlLongLongVal(IDL_LongLong(a.s | b.s));
  else
    return IdlLongLongVal(IDL_ULongLong(a.u | b.u));
}
#endif

// Xor
IdlLongVal XorExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (a.negative)
    return IdlLongVal(IDL_Long(a.s ^ b.s));
  else
    return IdlLongVal(IDL_ULong(a.u ^ b.u));
}
#ifdef HAS_LongLong
IdlLongLongVal XorExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (a.negative)
    return IdlLongLongVal(IDL_LongLong(a.s ^ b.s));
  else
    return IdlLongLongVal(IDL_ULongLong(a.u ^ b.u));
}
#endif

// And
IdlLongVal AndExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (a.negative)
    return IdlLongVal(IDL_Long(a.s & b.s));
  else
    return IdlLongVal(IDL_ULong(a.u & b.u));
}
#ifdef HAS_LongLong
IdlLongLongVal AndExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (a.negative)
    return IdlLongLongVal(IDL_LongLong(a.s & b.s));
  else
    return IdlLongLongVal(IDL_ULongLong(a.u & b.u));
}
#endif

// Right shift
IdlLongVal RShiftExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  // Assume two's complement, and treat b as unsigned. If it's
  // actually signed and negative, its unsigned value will be much >
  // 64.
  if (b.u >= 64) {
    IdlError(file(), line(),
	     "Right operand of shift operation must be >= 0 and < 64");
    return a;
  }
  if (a.negative)
    return IdlLongVal(IDL_Long(a.s >> b.u));
  else
    return IdlLongVal(IDL_ULong(a.u >> b.u));
}
#ifdef HAS_LongLong
IdlLongLongVal RShiftExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (b.u >= 64) {
    IdlError(file(), line(),
	     "Right operand of shift operation must be >= 0 and < 64");
    return a;
  }
  if (a.negative)
    return IdlLongLongVal(IDL_LongLong(a.s >> b.u));
  else
    return IdlLongLongVal(IDL_ULongLong(a.u >> b.u));
}
#endif

// Left shift
IdlLongVal LShiftExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (b.u >= 64) {
    IdlError(file(), line(),
	     "Right operand of shift operation must be >= 0 and < 64");
    return a;
  }
  if (a.negative)
    return IdlLongVal(IDL_Long(a.s << b.u));
  else
    return IdlLongVal(IDL_ULong(a.u << b.u));
}
#ifdef HAS_LongLong
IdlLongLongVal LShiftExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (b.u >= 64) {
    IdlError(file(), line(),
	     "Right operand of shift operation must be >= 0 and < 64");
    return a;
  }
  if (a.negative)
    return IdlLongLongVal(IDL_LongLong(a.s << b.u));
  else
    return IdlLongLongVal(IDL_ULongLong(a.u << b.u));
}
#endif


// %
IdlLongVal ModExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (b.u == 0) {
    IdlError(file(), line(), "Remainder of division by 0 is undefined");
    return a;
  }
  if (a.negative || b.negative)
    IdlWarning(file(), line(), "Result of %% operator involving negative "
	       "operands is implementation dependent");

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0: return IdlLongVal(IDL_ULong(a.u % b.u));
  case 1: return IdlLongVal(-IDL_Long((-a.s) % b.u));
  case 2: return IdlLongVal(IDL_ULong(a.u % IDL_ULong(-b.s)));
  case 3: return IdlLongVal(-IDL_Long((-a.s) % (-b.s)));
  }
  return IdlLongVal(IDL_ULong(0)); // Never reach here
}
#ifdef HAS_LongLong
IdlLongLongVal ModExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (b.u == 0) {
    IdlError(file(), line(), "Remainder of division by 0 is undefined");
    return a;
  }
  if (a.negative || b.negative)
    IdlWarning(file(), line(), "Result of %% operator involving negative "
	       "operands is platform dependent");

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0: return IdlLongLongVal(IDL_ULongLong(a.u % b.u));
  case 1: return IdlLongLongVal(IDL_LongLong (a.s % b.u));
  case 2: return IdlLongLongVal(IDL_LongLong (a.u % b.s));
  case 3: return IdlLongLongVal(IDL_LongLong (a.s % b.s));
  }
  return IdlLongLongVal(IDL_ULongLong(0)); // Never reach here
}
#endif


// Add
IdlLongVal AddExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      IDL_ULong r = a.u + b.u;
      if (r < a.u) goto overflow;
      return IdlLongVal(r);
    }
  case 1:
    {
      if (IDL_ULong(-a.s) < b.u)
	return IdlLongVal(b.u - IDL_ULong(-a.s));
      else
	return IdlLongVal(a.s + IDL_Long(b.u));
    }
  case 2:
    {
      if (IDL_ULong(-b.s) < a.u)
	return IdlLongVal(a.u - IDL_ULong(-b.s));
      else
	return IdlLongVal(IDL_Long(a.u) + b.s);
    }
  case 3:
    {
      IDL_Long r = a.s + b.s;
      if (r > a.s) goto overflow;
      return IdlLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of addition overflows");
  return a;
}

#ifdef HAS_LongLong
IdlLongLongVal AddExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      IDL_ULongLong r = a.u + b.u;
      if (r < a.u) goto overflow;
      return IdlLongLongVal(r);
    }
  case 1:
    {
      if (IDL_ULongLong(-a.s) < b.u)
	return IdlLongLongVal(b.u - IDL_ULongLong(-a.s));
      else
	return IdlLongLongVal(a.s + IDL_LongLong(b.u));
    }
  case 2:
    {
      if (IDL_ULongLong(-b.s) < a.u)
	return IdlLongLongVal(a.u - IDL_ULongLong(-b.s));
      else
	return IdlLongLongVal(IDL_LongLong(a.u) + b.s);
    }
  case 3:
    {
      IDL_LongLong r = a.s + b.s;
      if (r > a.s) goto overflow;
      return IdlLongLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of addition overflows");
  return a;
}
#endif

#define ADD_EXPR_EVAL_F(ret, op, str) \
ret AddExpr::op() { \
  ret a, b, r; \
  a = a_->op(); b = b_->op(); \
  r = a+b; \
  if (IdlFPOverflow(r)) { \
    IdlError(file(), line(), "Result of addition overflows " str); \
    r = 1.0; \
  } \
  return r; \
}
ADD_EXPR_EVAL_F(IDL_Float,      evalAsFloat,      "float")
ADD_EXPR_EVAL_F(IDL_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
ADD_EXPR_EVAL_F(IDL_LongDouble, evalAsLongDouble, "long double")
#endif

IDL_Fixed* AddExpr::evalAsFixed() {
  IDL_Fixed *a, *b, *r;
  a = a_->evalAsFixed(); b = b_->evalAsFixed();
  try {
    r = new IDL_Fixed(*a + *b);
  }
  catch (IDL_Fixed::Overflow&) {
    IdlError(file(), line(), "Result of addition overflows fixed digits");
    r = new IDL_Fixed("1");
  }
  delete a; delete b;
  return r;
}

// Sub
IdlLongVal SubExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      if (a.u >= b.u) return IdlLongVal(a.u - b.u);
      IDL_ULong mr = b.u - a.u;
      if (mr > 0x80000000) goto overflow;
      return IdlLongVal(IDL_Long(-mr));
    }
  case 1:
    {
      IDL_ULong mr = IDL_ULong(-a.s) + b.s;
      if (mr > 0x80000000) goto overflow;
      return IdlLongVal(IDL_Long(-mr));
    }
  case 2:
    {
      IDL_ULong r = a.u + IDL_ULong(-b.s);
      if (r < a.u) goto overflow;
      return IdlLongVal(r);
    }
  case 3:
    {
      IDL_Long r = a.s - b.s;
      if (r > a.s) goto overflow;
      return IdlLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of subtraction overflows");
  return a;
}

#ifdef HAS_LongLong
IdlLongLongVal SubExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      if (a.u >= b.u) return IdlLongLongVal(a.u - b.u);
      IDL_ULongLong mr = b.u - a.u;
      if (mr > _CORBA_LONGLONG_CONST(0x8000000000000000)) goto overflow;
      return IdlLongLongVal(IDL_LongLong(-mr));
    }
  case 1:
    {
      IDL_ULongLong mr = IDL_ULongLong(-a.s) + b.s;
      if (mr > _CORBA_LONGLONG_CONST(0x8000000000000000)) goto overflow;
      return IdlLongLongVal(IDL_LongLong(-mr));
    }
  case 2:
    {
      IDL_ULongLong r = a.u + IDL_ULongLong(-b.s);
      if (r < a.u) goto overflow;
      return IdlLongLongVal(r);
    }
  case 3:
    {
      IDL_LongLong r = a.s - b.s;
      if (r > a.s) goto overflow;
      return IdlLongLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of subtraction overflows");
  return a;
}
#endif


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
SUB_EXPR_EVAL_F(IDL_Float,      evalAsFloat,      "float")
SUB_EXPR_EVAL_F(IDL_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
SUB_EXPR_EVAL_F(IDL_LongDouble, evalAsLongDouble, "long double")
#endif

IDL_Fixed* SubExpr::evalAsFixed() {
  IDL_Fixed *a, *b, *r;
  a = a_->evalAsFixed(); b = b_->evalAsFixed();
  try {
    r = new IDL_Fixed(*a - *b);
  }
  catch (IDL_Fixed::Overflow&) {
    IdlError(file(), line(), "Sub-expression overflows fixed digits");
    r = new IDL_Fixed("1");
  }
  delete a; delete b;
  return r;
}



// Mult
IdlLongVal MultExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      IDL_ULong r = a.u * b.u;
      if (b.u != 0 && (r / b.u) != a.u) goto overflow;
      return IdlLongVal(r);
    }
  case 1:
    {
      IDL_Long r = a.s * IDL_Long(b.u);
      if (b.u != 0 && (r / IDL_Long(b.u)) != a.s) goto overflow;
      return IdlLongVal(r);
    }
  case 2:
    {
      IDL_Long r = IDL_Long(a.u) * b.s;
      if (b.s != 0 && (r / b.s) != IDL_Long(a.u)) goto overflow;
      return IdlLongVal(r);
    }
  case 3:
    {
      IDL_ULong r = IDL_ULong(-a.s) * IDL_ULong(-b.s);
      if (b.s != 0 && (r / IDL_ULong(-b.s)) != IDL_ULong(-a.s)) goto overflow;
      return IdlLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of multiplication overflows");
  return a;
}

#ifdef HAS_LongLong
IdlLongLongVal MultExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      IDL_ULongLong r = a.u * b.u;
      if (b.u != 0 && (r / b.u) != a.u) goto overflow;
      return IdlLongLongVal(r);
    }
  case 1:
    {
      IDL_LongLong r = a.s * IDL_LongLong(b.u);
      if (b.u != 0 && (r / IDL_LongLong(b.u)) != a.s) goto overflow;
      return IdlLongLongVal(r);
    }
  case 2:
    {
      IDL_LongLong r = IDL_LongLong(a.u) * b.s;
      if (b.s != 0 && (r / b.s) != IDL_LongLong(a.u)) goto overflow;
      return IdlLongLongVal(r);
    }
  case 3:
    {
      IDL_ULongLong r = IDL_ULongLong(-a.s) * IDL_ULongLong(-b.s);
      if (b.s != 0 &&
	  (r / IDL_ULongLong(-b.s)) != IDL_ULongLong(-a.s)) goto overflow;
      return IdlLongLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of multiplication overflows");
  return a;
}
#endif


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
MULT_EXPR_EVAL_F(IDL_Float,      evalAsFloat,      "float")
MULT_EXPR_EVAL_F(IDL_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
MULT_EXPR_EVAL_F(IDL_LongDouble, evalAsLongDouble, "long double")
#endif

IDL_Fixed* MultExpr::evalAsFixed() {
  IDL_Fixed *a, *b, *r;
  a = a_->evalAsFixed(); b = b_->evalAsFixed();
  try {
    r = new IDL_Fixed(*a * *b);
  }
  catch (IDL_Fixed::Overflow&) {
    IdlError(file(), line(), "Sub-expression overflows fixed digits");
    r = new IDL_Fixed("1");
  }
  delete a; delete b;
  return r;
}

// Div
IdlLongVal DivExpr::evalAsLongV() {
  IdlLongVal a = a_->evalAsLongV();
  IdlLongVal b = b_->evalAsLongV();

  if (b.u == 0) {
    IdlError(file(), line(), "Divide by zero");
    return a;
  }

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      return IdlLongVal(a.u / b.u);
    }
  case 1:
    {
      IDL_ULong mr = IDL_ULong(-a.s) / b.u;
      return IdlLongVal(IDL_Long(-mr));
    }
  case 2:
    {
      IDL_ULong mr = a.u / IDL_ULong(-b.s);
      if (mr > 0x80000000) goto overflow;
      return IdlLongVal(IDL_Long(-mr));
    }
  case 3:
    {
      IDL_ULong r = IDL_ULong(-a.s) / IDL_ULong(-b.s);
      return IdlLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of division overflows");
  return a;
}

#ifdef HAS_LongLong
IdlLongLongVal DivExpr::evalAsLongLongV() {
  IdlLongLongVal a = a_->evalAsLongLongV();
  IdlLongLongVal b = b_->evalAsLongLongV();

  if (b.u == 0) {
    IdlError(file(), line(), "Divide by zero");
    return a;
  }

  switch ((a.negative ? 1:0) + (b.negative ? 2:0)) {
  case 0:
    {
      return IdlLongLongVal(a.u / b.u);
    }
  case 1:
    {
      IDL_ULongLong mr = IDL_ULongLong(-a.s) / b.u;
      return IdlLongLongVal(IDL_LongLong(-mr));
    }
  case 2:
    {
      IDL_ULongLong mr = a.u / IDL_ULongLong(-b.s);
      if (mr > _CORBA_LONGLONG_CONST(0x8000000000000000)) goto overflow;
      return IdlLongLongVal(IDL_LongLong(-mr));
    }
  case 3:
    {
      IDL_ULongLong r = IDL_ULongLong(-a.s) / IDL_ULongLong(-b.s);
      return IdlLongLongVal(r);
    }
  }
 overflow:
  IdlError(file(), line(), "Result of division overflows");
  return a;
}
#endif


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
DIV_EXPR_EVAL_F(IDL_Float,      evalAsFloat,      "float")
DIV_EXPR_EVAL_F(IDL_Double,     evalAsDouble,     "double")
#ifdef HAS_LongDouble
DIV_EXPR_EVAL_F(IDL_LongDouble, evalAsLongDouble, "long double")
#endif

IDL_Fixed* DivExpr::evalAsFixed() {
  IDL_Fixed *a, *b, *r;
  a = a_->evalAsFixed(); b = b_->evalAsFixed();
  try {
    r = new IDL_Fixed(*a / *b);
  }
  catch (IDL_Fixed::Overflow&) {
    IdlError(file(), line(), "Sub-expression overflows fixed digits");
    r = new IDL_Fixed("1");
  }
  catch (IDL_Fixed::DivideByZero&) {
    IdlError(file(), line(), "Divide by zero");
    r = new IDL_Fixed("1");
  }
  delete a; delete b;
  return r;
}


// Invert
IdlLongVal InvertExpr::evalAsLongV() {
  IdlLongVal e = e_->evalAsLongV();
  return IdlLongVal(~e.u);
}
#ifdef HAS_LongLong
IdlLongLongVal InvertExpr::evalAsLongLongV() {
  IdlLongLongVal e = e_->evalAsLongLongV();
  return IdlLongLongVal(~e.u);
}
#endif


// Minus
IdlLongVal MinusExpr::evalAsLongV() {
  IdlLongVal e = e_->evalAsLongV();

  if (e.negative)
    return IdlLongVal(IDL_ULong(-e.s));
  else {
    if (e.u > 0x80000000)
      IdlError(file(), line(), "Result of unary minus overflows");
    return IdlLongVal(IDL_Long(-e.u));
  }
}

#ifdef HAS_LongLong
IdlLongLongVal MinusExpr::evalAsLongLongV() {
  IdlLongLongVal e = e_->evalAsLongLongV();

  if (e.negative)
    return IdlLongLongVal(IDL_ULongLong(-e.s));
  else {
    if (e.u > _CORBA_LONGLONG_CONST(0x8000000000000000))
      IdlError(file(), line(), "Result of unary minus overflows");
    return IdlLongLongVal(IDL_LongLong(-e.u));
  }
}
#endif

#define MINUS_EXPR_EVAL(ret, op) \
ret MinusExpr::op() { \
  return -(e_->op()); \
}
MINUS_EXPR_EVAL(IDL_Float,      evalAsFloat)
MINUS_EXPR_EVAL(IDL_Double,     evalAsDouble)
#ifdef HAS_LongDouble
MINUS_EXPR_EVAL(IDL_LongDouble, evalAsLongDouble)
#endif

IDL_Fixed* MinusExpr::evalAsFixed() {
  IDL_Fixed* r = new IDL_Fixed(- *(e_->evalAsFixed()));
  delete e_->evalAsFixed();
  return r;
}


// Plus
#define PLUS_EXPR_EVAL(ret, op) \
ret PlusExpr::op() { \
  return e_->op(); \
}
PLUS_EXPR_EVAL(IdlLongVal,     evalAsLongV)
#ifdef HAS_LongLong
PLUS_EXPR_EVAL(IdlLongLongVal, evalAsLongLongV)
#endif
PLUS_EXPR_EVAL(IDL_Float,      evalAsFloat)
PLUS_EXPR_EVAL(IDL_Double,     evalAsDouble)
#ifdef HAS_LongDouble
PLUS_EXPR_EVAL(IDL_LongDouble, evalAsLongDouble)
#endif
PLUS_EXPR_EVAL(IDL_Fixed*,     evalAsFixed)
