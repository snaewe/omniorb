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
// Revision 1.1  1999/10/27 14:05:56  dpg1
// *** empty log message ***
//

#ifndef _idlexpr_h_
#define _idlexpr_h_

#include <idlutil.h>
#include <idlscope.h>

typedef int _CORBA_Fixed; // ***

class Enumerator;
class Enum;
class Const;


class IdlExpr {
public:
  IdlExpr(const char* file, int line) : file_(idl_strdup(file)), line_(line) {}
  virtual ~IdlExpr() { delete [] file_; }

  virtual _CORBA_Short        evalAsShort();
  virtual _CORBA_Long         evalAsLong();
  virtual _CORBA_UShort       evalAsUShort();
  virtual _CORBA_ULong        evalAsULong();
  virtual _CORBA_Float        evalAsFloat();
  virtual _CORBA_Double       evalAsDouble();
  virtual _CORBA_Boolean      evalAsBoolean();
  virtual _CORBA_Char         evalAsChar();
  virtual _CORBA_Octet        evalAsOctet();
  virtual const char*         evalAsString();
  virtual Enumerator*         evalAsEnumerator(const Enum* target);
#ifdef HAS_LongLong
  virtual _CORBA_LongLong     evalAsLongLong();
  virtual _CORBA_ULongLong    evalAsULongLong();
#endif
#ifdef HAS_LongDouble
  virtual _CORBA_LongDouble   evalAsLongDouble();
#endif
  virtual _CORBA_WChar        evalAsWChar();
  virtual const _CORBA_WChar* evalAsWString();
  virtual _CORBA_Fixed        evalAsFixed();

  inline const char* file() { return file_; }
  inline int         line() { return line_; }

  virtual const char* errText() = 0;

  static IdlExpr* scopedNameToExpr(const char* file, int line, ScopedName* sn);

private:
  const char* file_;
  int         line_;
};


// Dummy expression class used as a place-holder after an error
class DummyExpr : public IdlExpr {
public:
  DummyExpr(const char* file, int line) : IdlExpr(file, line) {}
  virtual ~DummyExpr() {}

  _CORBA_Short        evalAsShort()                        { return 1; }
  _CORBA_Long         evalAsLong()                         { return 1; }
  _CORBA_UShort       evalAsUShort()                       { return 1; }
  _CORBA_ULong        evalAsULong()                        { return 1; }
  _CORBA_Float        evalAsFloat()                        { return 1.0; }
  _CORBA_Double       evalAsDouble()                       { return 1.0; }
  _CORBA_Boolean      evalAsBoolean()                      { return 0; }
  _CORBA_Char         evalAsChar()                         { return '!'; }
  _CORBA_Octet        evalAsOctet()                        { return 1; }
  const char*         evalAsString()                       { return "!"; }
  Enumerator*         evalAsEnumerator(const Enum* target) { return 0; }
#ifdef HAS_LongLong
  _CORBA_LongLong     evalAsLongLong()                     { return 1; }
  _CORBA_ULongLong    evalAsULongLong()                    { return 1; }
#endif
#ifdef HAS_LongDouble
  _CORBA_LongDouble   evalAsLongDouble()                   { return 1.0; }
#endif
  _CORBA_WChar        evalAsWChar()                        { return '!'; }
  const _CORBA_WChar* evalAsWString();
  _CORBA_Fixed        evalAsFixed()                        { return 1; }

  const char* errText() { return "dummy"; }
};


// Literals

class IntegerExpr : public IdlExpr {
public:
  IntegerExpr(const char* file, int line, IdlIntLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~IntegerExpr() {}

  _CORBA_Short        evalAsShort();
  _CORBA_Long         evalAsLong();
  _CORBA_UShort       evalAsUShort();
  _CORBA_ULong        evalAsULong();
  _CORBA_Octet        evalAsOctet();
#ifdef HAS_LongLong
  _CORBA_LongLong     evalAsLongLong();
  _CORBA_ULongLong    evalAsULongLong();
#endif
  const char*         errText() { return "integer literal"; }
private:
  IdlIntLiteral value_;
};

class StringExpr : public IdlExpr {
public:
  StringExpr(const char* file, int line, const char* v)
    : IdlExpr(file, line), value_(idl_strdup(v)) { }
  ~StringExpr() { delete [] value_; }

  const char*         evalAsString();
  const char*         errText() { return "string literal"; }
private:
  const char* value_;
};

class WStringExpr : public IdlExpr {
public:
  WStringExpr(const char* file, int line, const _CORBA_WChar* v)
    : IdlExpr(file, line), value_(idl_wstrdup(v)) {}
  ~WStringExpr() { delete [] value_; }

  const _CORBA_WChar* evalAsWString();
  const char*         errText() { return "wide string literal"; }
private:
  const _CORBA_WChar* value_;
};

class CharExpr : public IdlExpr {
public:
  CharExpr(const char* file, int line, _CORBA_Char v)
    : IdlExpr(file, line), value_(v) { }
  ~CharExpr() {}

  _CORBA_Char         evalAsChar();
  const char*         errText() { return "character literal"; }
private:
  _CORBA_Char value_;
};

class WCharExpr : public IdlExpr {
public:
  WCharExpr(const char* file, int line, _CORBA_WChar v)
    : IdlExpr(file, line), value_(v) {}
  ~WCharExpr() {}

  _CORBA_WChar        evalAsWChar();
  const char*         errText() { return "wide character literal"; }
private:
  _CORBA_WChar value_;
};

class FixedExpr : public IdlExpr {
public:
  FixedExpr(const char* file, int line, _CORBA_Fixed v)
    : IdlExpr(file, line), value_(v) {}
  ~FixedExpr() {}

  _CORBA_Fixed        evalAsFixed();
  const char*         errText() { return "fixed point literal"; }
private:
  _CORBA_Fixed value_;
};

class FloatExpr : public IdlExpr {
public:
  FloatExpr(const char* file, int line, IdlFloatLiteral v)
    : IdlExpr(file, line), value_(v) { }
  ~FloatExpr() {}

  _CORBA_Float        evalAsFloat();
  _CORBA_Double       evalAsDouble();
#ifdef HAS_LongDouble
  _CORBA_LongDouble   evalAsLongDouble();
#endif
  const char*         errText() { return "floating point literal"; }
private:
  IdlFloatLiteral value_;
};

class BooleanExpr : public IdlExpr {
public:
  BooleanExpr(const char* file, int line, _CORBA_Boolean v)
    : IdlExpr(file, line), value_(v) { }
  ~BooleanExpr() {}

  _CORBA_Boolean      evalAsBoolean();
  const char*         errText() { return "boolean literal"; }
private:
  _CORBA_Boolean value_;
};

// Enumerator referred to by scoped name
class EnumExpr : public IdlExpr {
public:
  EnumExpr(const char* file, int line, Enumerator* e, ScopedName* sn)
    : IdlExpr(file, line), value_(e), scopedName_(sn) {}
  ~EnumExpr() {}

  Enumerator* evalAsEnumerator(const Enum* target);
  const char* errText() { return "enumerator"; }
private:
  Enumerator* value_;
  ScopedName* scopedName_;
};

// Constant referred to by scoped name
class ConstExpr : public IdlExpr {
public:
  ConstExpr(const char* file, int line, Const* c, ScopedName* sn)
    : IdlExpr(file, line), c_(c), scopedName_(sn) {}
  ~ConstExpr() {}

  _CORBA_Short        evalAsShort();
  _CORBA_Long         evalAsLong();
  _CORBA_UShort       evalAsUShort();
  _CORBA_ULong        evalAsULong();
  _CORBA_Float        evalAsFloat();
  _CORBA_Double       evalAsDouble();
  _CORBA_Boolean      evalAsBoolean();
  _CORBA_Char         evalAsChar();
  _CORBA_Octet        evalAsOctet();
  const char*         evalAsString();
  Enumerator*         evalAsEnumerator(const Enum* target);
#ifdef HAS_LongLong
  _CORBA_LongLong     evalAsLongLong();
  _CORBA_ULongLong    evalAsULongLong();
#endif
#ifdef HAS_LongDouble
  _CORBA_LongDouble   evalAsLongDouble();
#endif
  _CORBA_WChar        evalAsWChar();
  const _CORBA_WChar* evalAsWString();
  _CORBA_Fixed        evalAsFixed();

  const char* errText() { return "constant"; }
private:
  Const*      c_;
  ScopedName* scopedName_;
};



// Expressions

#ifdef HAS_LongLong

#define EXPR_S_INT_CONVERSION_FUNCTIONS \
  _CORBA_Short        evalAsShort();    \
  _CORBA_Long         evalAsLong();     \
  _CORBA_LongLong     evalAsLongLong();

#define EXPR_U_INT_CONVERSION_FUNCTIONS \
  _CORBA_UShort       evalAsUShort();   \
  _CORBA_ULong        evalAsULong();    \
  _CORBA_Octet        evalAsOctet();    \
  _CORBA_ULongLong    evalAsULongLong();

#else
#define EXPR_S_INT_CONVERSION_FUNCTIONS \
  _CORBA_Short        evalAsShort();    \
  _CORBA_Long         evalAsLong();

#define EXPR_U_INT_CONVERSION_FUNCTIONS \
  _CORBA_UShort       evalAsUShort();   \
  _CORBA_ULong        evalAsULong();    \
  _CORBA_Octet        evalAsOctet();

#endif

#define EXPR_INT_CONVERSION_FUNCTIONS \
  EXPR_S_INT_CONVERSION_FUNCTIONS     \
  EXPR_U_INT_CONVERSION_FUNCTIONS

#ifdef HAS_LongDouble
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  _CORBA_Float        evalAsFloat();    \
  _CORBA_Double       evalAsDouble();   \
  _CORBA_LongDouble   evalAsLongDouble();
#else
#define EXPR_FLOAT_CONVERSION_FUNCTIONS \
  _CORBA_Float        evalAsFloat();    \
  _CORBA_Double       evalAsDouble();
#endif

#define EXPR_CONVERSION_FUNCTIONS    \
  EXPR_INT_CONVERSION_FUNCTIONS      \
  EXPR_FLOAT_CONVERSION_FUNCTIONS


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

#define EXPR_INT_UNARY_CLASS(cls, str) \
class cls : public IdlExpr { \
public: \
  cls(const char* file, int line, IdlExpr* e) \
    : IdlExpr(file, line), e_(e) { } \
  ~cls() { delete e_; } \
  EXPR_INT_CONVERSION_FUNCTIONS \
  const char* errText() { return "result of " str " operator"; } \
private: \
  IdlExpr* e_; \
};

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
