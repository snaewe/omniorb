// -*- c++ -*-
//                          Package   : omniidl
// idlexpr.h                Created on: 1999/10/18
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
//   Expression tree and evaluator

// $Id$
// $Log$
// Revision 1.4.2.3  2001/03/13 10:32:12  dpg1
// Fixed point support.
//
// Revision 1.4.2.2  2000/10/27 16:31:09  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.4.2.1  2000/07/17 10:36:03  sll
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

#ifndef _idlexpr_h_
#define _idlexpr_h_

#include <idlutil.h>
#include <idlscope.h>
#include <idlfixed.h>

class Enumerator;
class Enum;
class Const;


class IdlExpr {
public:
  IdlExpr(const char* file, int line) : file_(idl_strdup(file)), line_(line) {}
  virtual ~IdlExpr() { delete [] file_; }

  virtual IDL_Short        evalAsShort();
  virtual IDL_Long         evalAsLong();
  virtual IDL_UShort       evalAsUShort();
  virtual IDL_ULong        evalAsULong();
  virtual IDL_Float        evalAsFloat();
  virtual IDL_Double       evalAsDouble();
  virtual IDL_Boolean      evalAsBoolean();
  virtual IDL_Char         evalAsChar();
  virtual IDL_Octet        evalAsOctet();
  virtual const char*      evalAsString();
  virtual Enumerator*      evalAsEnumerator(const Enum* target);
#ifdef HAS_LongLong
  virtual IDL_LongLong     evalAsLongLong();
  virtual IDL_ULongLong    evalAsULongLong();
#endif
#ifdef HAS_LongDouble
  virtual IDL_LongDouble   evalAsLongDouble();
#endif
  virtual IDL_WChar        evalAsWChar();
  virtual const IDL_WChar* evalAsWString();
  virtual IDL_Fixed*       evalAsFixed();

  inline const char* file() { return file_; }
  inline int         line() { return line_; }

  virtual const char* errText() = 0;

  static IdlExpr* scopedNameToExpr(const char* file, int line, ScopedName* sn);

private:
  char* file_;
  int   line_;
};


// Dummy expression class used as a place-holder after an error
class DummyExpr : public IdlExpr {
public:
  DummyExpr(const char* file, int line) : IdlExpr(file, line) {}
  virtual ~DummyExpr() {}

  IDL_Short        evalAsShort()                        { return 1; }
  IDL_Long         evalAsLong()                         { return 1; }
  IDL_UShort       evalAsUShort()                       { return 1; }
  IDL_ULong        evalAsULong()                        { return 1; }
  IDL_Float        evalAsFloat()                        { return 1.0; }
  IDL_Double       evalAsDouble()                       { return 1.0; }
  IDL_Boolean      evalAsBoolean()                      { return 0; }
  IDL_Char         evalAsChar()                         { return '!'; }
  IDL_Octet        evalAsOctet()                        { return 1; }
  const char*      evalAsString()                       { return "!"; }
  Enumerator*      evalAsEnumerator(const Enum* target) { return 0; }
#ifdef HAS_LongLong
  IDL_LongLong     evalAsLongLong()                     { return 1; }
  IDL_ULongLong    evalAsULongLong()                    { return 1; }
#endif
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble()                   { return 1.0; }
#endif
  IDL_WChar        evalAsWChar()                        { return '!'; }
  const IDL_WChar* evalAsWString();
  IDL_Fixed*       evalAsFixed()                 { return new IDL_Fixed("1"); }

  const char*      errText() { return "dummy"; }
};


// Literals

class IntegerExpr : public IdlExpr {
public:
  IntegerExpr(const char* file, int line, IdlIntLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~IntegerExpr() {}

  IDL_Short        evalAsShort();
  IDL_Long         evalAsLong();
  IDL_UShort       evalAsUShort();
  IDL_ULong        evalAsULong();
  IDL_Octet        evalAsOctet();
#ifdef HAS_LongLong
  IDL_LongLong     evalAsLongLong();
  IDL_ULongLong    evalAsULongLong();
#endif
  const char*      errText() { return "integer literal"; }
private:
  IdlIntLiteral    value_;
};

class StringExpr : public IdlExpr {
public:
  StringExpr(const char* file, int line, const char* v)
    : IdlExpr(file, line), value_(idl_strdup(v)) { }
  ~StringExpr() { delete [] value_; }

  const char*      evalAsString();
  const char*      errText() { return "string literal"; }
private:
  char* value_;
};

class WStringExpr : public IdlExpr {
public:
  WStringExpr(const char* file, int line, const IDL_WChar* v)
    : IdlExpr(file, line), value_(idl_wstrdup(v)) {}
  ~WStringExpr() { delete [] value_; }

  const IDL_WChar* evalAsWString();
  const char*      errText() { return "wide string literal"; }
private:
  IDL_WChar*       value_;
};

class CharExpr : public IdlExpr {
public:
  CharExpr(const char* file, int line, IDL_Char v)
    : IdlExpr(file, line), value_(v) { }
  ~CharExpr() {}

  IDL_Char         evalAsChar();
  const char*      errText() { return "character literal"; }
private:
  IDL_Char         value_;
};

class WCharExpr : public IdlExpr {
public:
  WCharExpr(const char* file, int line, IDL_WChar v)
    : IdlExpr(file, line), value_(v) {}
  ~WCharExpr() {}

  IDL_WChar        evalAsWChar();
  const char*      errText() { return "wide character literal"; }
private:
  IDL_WChar        value_;
};

class FixedExpr : public IdlExpr {
public:
  FixedExpr(const char* file, int line, IDL_Fixed* v)
    : IdlExpr(file, line), value_(v) {}
  ~FixedExpr() {}

  IDL_Fixed*       evalAsFixed();
  const char*      errText() { return "fixed point literal"; }
private:
  IDL_Fixed*       value_;
};

class FloatExpr : public IdlExpr {
public:
  FloatExpr(const char* file, int line, IdlFloatLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~FloatExpr() {}

  IDL_Float        evalAsFloat();
  IDL_Double       evalAsDouble();
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble();
#endif
  const char*      errText() { return "floating point literal"; }
private:
  IdlFloatLiteral value_;
};

class BooleanExpr : public IdlExpr {
public:
  BooleanExpr(const char* file, int line, IDL_Boolean v)
    : IdlExpr(file, line), value_(v) { }
  ~BooleanExpr() {}

  IDL_Boolean      evalAsBoolean();
  const char*      errText() { return "boolean literal"; }
private:
  IDL_Boolean      value_;
};

// Enumerator referred to by scoped name
class EnumExpr : public IdlExpr {
public:
  EnumExpr(const char* file, int line, Enumerator* e, ScopedName* sn)
    : IdlExpr(file, line), value_(e), scopedName_(sn) {}
  ~EnumExpr() {}

  Enumerator*      evalAsEnumerator(const Enum* target);
  const char*      errText() { return "enumerator"; }
private:
  Enumerator* 	   value_;
  ScopedName* 	   scopedName_;
};

// Constant referred to by scoped name
class ConstExpr : public IdlExpr {
public:
  ConstExpr(const char* file, int line, Const* c, ScopedName* sn)
    : IdlExpr(file, line), c_(c), scopedName_(sn) {}
  ~ConstExpr() {}

  IDL_Short        evalAsShort();
  IDL_Long         evalAsLong();
  IDL_UShort       evalAsUShort();
  IDL_ULong        evalAsULong();
  IDL_Float        evalAsFloat();
  IDL_Double       evalAsDouble();
  IDL_Boolean      evalAsBoolean();
  IDL_Char         evalAsChar();
  IDL_Octet        evalAsOctet();
  const char*      evalAsString();
  Enumerator*      evalAsEnumerator(const Enum* target);
#ifdef HAS_LongLong
  IDL_LongLong     evalAsLongLong();
  IDL_ULongLong    evalAsULongLong();
#endif
#ifdef HAS_LongDouble
  IDL_LongDouble   evalAsLongDouble();
#endif
  IDL_WChar        evalAsWChar();
  const IDL_WChar* evalAsWString();
  IDL_Fixed*       evalAsFixed();

  const char* errText() { return "constant"; }
private:
  Const*      c_;
  ScopedName* scopedName_;
};



// Expressions

#ifdef HAS_LongLong

#define EXPR_S_INT_CONVERSION_FUNCTIONS \
  IDL_Short        evalAsShort();    \
  IDL_Long         evalAsLong();     \
  IDL_LongLong     evalAsLongLong();

#define EXPR_U_INT_CONVERSION_FUNCTIONS \
  IDL_UShort       evalAsUShort();   \
  IDL_ULong        evalAsULong();    \
  IDL_Octet        evalAsOctet();    \
  IDL_ULongLong    evalAsULongLong();

#else
#define EXPR_S_INT_CONVERSION_FUNCTIONS \
  IDL_Short        evalAsShort();    \
  IDL_Long         evalAsLong();

#define EXPR_U_INT_CONVERSION_FUNCTIONS \
  IDL_UShort       evalAsUShort();   \
  IDL_ULong        evalAsULong();    \
  IDL_Octet        evalAsOctet();

#endif

#define EXPR_INT_CONVERSION_FUNCTIONS \
  EXPR_S_INT_CONVERSION_FUNCTIONS     \
  EXPR_U_INT_CONVERSION_FUNCTIONS

#ifdef HAS_LongDouble
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  IDL_Float        evalAsFloat();    \
  IDL_Double       evalAsDouble();   \
  IDL_LongDouble   evalAsLongDouble();
#else
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  IDL_Float        evalAsFloat();    \
  IDL_Double       evalAsDouble();
#endif

#define EXPR_FIXED_CONVERSION_FUNCTIONS \
  IDL_Fixed*       evalAsFixed();

#define EXPR_CONVERSION_FUNCTIONS    \
  EXPR_INT_CONVERSION_FUNCTIONS      \
  EXPR_FLOAT_CONVERSION_FUNCTIONS    \
  EXPR_FIXED_CONVERSION_FUNCTIONS


#define EXPR_INT_BINARY_CLASS(cls, str) \
class cls : public IdlExpr { \
public: \
  cls(const char* file, int line, IdlExpr* a, IdlExpr* b) \
    : IdlExpr(file, line), a_(a), b_(b) { } \
  ~cls() { delete a_; delete b_; } \
  EXPR_INT_CONVERSION_FUNCTIONS \
  const char* errText() { return "result of " str " operation"; } \
private: \
  IdlExpr* a_; \
  IdlExpr* b_; \
};

#define EXPR_BINARY_CLASS(cls, str) \
class cls : public IdlExpr { \
public: \
  cls(const char* file, int line, IdlExpr* a, IdlExpr* b) \
    : IdlExpr(file, line), a_(a), b_(b) { } \
  ~cls() { delete a_; delete b_; } \
  EXPR_CONVERSION_FUNCTIONS \
  const char* errText() { return "result of " str " operation"; } \
private: \
  IdlExpr* a_; \
  IdlExpr* b_; \
};

EXPR_INT_BINARY_CLASS(OrExpr,     "or")
EXPR_INT_BINARY_CLASS(XorExpr,    "exclusive or")
EXPR_INT_BINARY_CLASS(AndExpr,    "and")
EXPR_INT_BINARY_CLASS(RShiftExpr, "right shift")
EXPR_INT_BINARY_CLASS(LShiftExpr, "left shift")
EXPR_INT_BINARY_CLASS(ModExpr,    "remainder")

EXPR_BINARY_CLASS(AddExpr,  "add")
EXPR_BINARY_CLASS(SubExpr,  "subtract")
EXPR_BINARY_CLASS(MultExpr, "multiply")
EXPR_BINARY_CLASS(DivExpr,  "divide")


class InvertExpr : public IdlExpr {
public:
  InvertExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~InvertExpr() { delete e_; }
  EXPR_INT_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary invert operator"; }
private:
  IdlExpr* e_;
};

class MinusExpr : public IdlExpr {
public:
  MinusExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~MinusExpr() { delete e_; }
  EXPR_S_INT_CONVERSION_FUNCTIONS
  EXPR_FLOAT_CONVERSION_FUNCTIONS
  EXPR_FIXED_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary negate operator"; }
private:
  IdlExpr* e_;
};

class PlusExpr : public IdlExpr {
public:
  PlusExpr(const char* file, int line, IdlExpr* e)
    : IdlExpr(file, line), e_(e) { }
  ~PlusExpr() { delete e_; }
  EXPR_CONVERSION_FUNCTIONS
  const char* errText() { return "result of unary plus operator"; }
private:
  IdlExpr* e_;
};


#endif // _idlexpr_h_
