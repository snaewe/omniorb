// -*- c++ -*-
//                          Package   : omniidl
// idltype.h                Created on: 1999/10/18
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
//   Type objects

// $Id$
// $Log$
// Revision 1.8.2.1  2003/03/23 21:01:44  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.5.2.4  2001/03/13 10:32:13  dpg1
// Fixed point support.
//
// Revision 1.5.2.3  2000/11/01 12:45:57  dpg1
// Update to CORBA 2.4 specification.
//
// Revision 1.5.2.2  2000/10/27 16:31:10  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.5.2.1  2000/07/17 10:36:05  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.6  2000/07/13 15:25:52  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.3  1999/11/17 17:16:59  dpg1
// Changes to remove static initialisation of objects.
//
// Revision 1.2  1999/10/29 15:44:23  dpg1
// DeclaredType() now takes extra DeclRepoId* argument.
//
// Revision 1.1  1999/10/27 14:05:54  dpg1
// *** empty log message ***
//

#ifndef _idltype_h_
#define _idltype_h_

#include <idlutil.h>
#include <idlvisitor.h>
#include <idlscope.h>

// Note on Type object memory management
//
// All type objects which can be are statically allocated; all others
// are allocated on the heap. When Decl objects receive a pointer to a
// type object, they should call its shouldDelete() function to see if
// they are responsible for deleting the object. This only returns
// true for those heap-allocated objects which do not have an
// associated Decl object. Type objects created by Decl constructors
// must be deleted by the Decls' destructors.


class IdlType {
public:

  enum Kind {
    tk_null               = 0,
    tk_void               = 1,
    tk_short              = 2,
    tk_long               = 3,
    tk_ushort             = 4,
    tk_ulong              = 5,
    tk_float              = 6,
    tk_double             = 7,
    tk_boolean            = 8,
    tk_char	          = 9,
    tk_octet              = 10,
    tk_any	          = 11,
    tk_TypeCode           = 12,
    tk_Principal          = 13,
    tk_objref             = 14,
    tk_struct             = 15,
    tk_union              = 16,
    tk_enum	          = 17,
    tk_string             = 18,
    tk_sequence           = 19,
    tk_array              = 20,
    tk_alias              = 21,
    tk_except             = 22,
    tk_longlong           = 23,
    tk_ulonglong          = 24,
    tk_longdouble         = 25,
    tk_wchar              = 26,
    tk_wstring            = 27,
    tk_fixed              = 28,
    tk_value              = 29,
    tk_value_box          = 30,
    tk_native             = 31,
    tk_abstract_interface = 32,
    tk_local_interface    = 33,

    // omniidl-specific kinds
    ot_structforward      = 100,
    ot_unionforward       = 101
  };

  IdlType(Kind k) : kind_(k), local_(0) { }
  virtual ~IdlType() {};

  Kind        kind()         const { return kind_; }
  const char* kindAsString() const;

  IDL_Boolean local()        const { return local_; }
  void        setLocal()           { local_ = 1; }
  // True if this is a "local" type which must not be transmitted
  // across the network.

  IdlType*    unalias();
  // Return an equivalent IdlType object with aliases stripped.

  virtual IDL_Boolean shouldDelete() = 0;

  virtual void accept(TypeVisitor& visitor) = 0;

  // Find a type given a name. Marks the name used in current scope
  static IdlType* scopedNameToType(const char* file, int line,
				   const ScopedName* sn);
  static void init();

private:
  Kind        kind_;
  IDL_Boolean local_;
  static IDL_Boolean initialised_;
};


class BaseType : public IdlType {
public:
  BaseType(Kind k) : IdlType(k) {}
  virtual ~BaseType() {}

  IDL_Boolean shouldDelete() { return 0; }

  void accept(TypeVisitor& visitor) { visitor.visitBaseType(this); }

  // Static base type singletons
  static BaseType* nullType;
  static BaseType* voidType;
  static BaseType* shortType;
  static BaseType* longType;
  static BaseType* ushortType;
  static BaseType* ulongType;
  static BaseType* floatType;
  static BaseType* doubleType;
  static BaseType* booleanType;
  static BaseType* charType;
  static BaseType* octetType;
  static BaseType* anyType;
  static BaseType* TypeCodeType;
  static BaseType* PrincipalType;
  static BaseType* longlongType;
  static BaseType* ulonglongType;
  static BaseType* longdoubleType;
  static BaseType* wcharType;
};


//
// Strings can be used like base types without a declaration. eg:
//
//   void op(in string<10> s);
//
// therefore, the String type must include its bound here, rather than
// relying on looking at the corresponding declaration
//

class StringType : public IdlType {
public:

  StringType(IDL_ULong bound) : IdlType(tk_string), bound_(bound) { }
  virtual ~StringType() {}

  IDL_ULong   bound()        { return bound_; }
  IDL_Boolean shouldDelete() { return bound_ ? 1 : 0; }

  void accept(TypeVisitor& visitor) { visitor.visitStringType(this); }

  static StringType* unboundedStringType;

private:
  IDL_ULong bound_;
};


class WStringType : public IdlType {
public:
  WStringType(IDL_ULong bound) : IdlType(tk_wstring), bound_(bound) { }
  virtual ~WStringType() {}

  IDL_ULong   bound()        { return bound_; }
  IDL_Boolean shouldDelete() { return bound_ ? 1 : 0; }

  void accept(TypeVisitor& visitor) { visitor.visitWStringType(this); }

  static WStringType* unboundedWStringType;

private:
  IDL_ULong bound_;
};

//
// Sequences are never declared. They either appear as
//
//   typedef sequence <...> ...
//
// or inside a struct or union
//

class SequenceType : public IdlType {
public:
  SequenceType(IdlType* seqType, IDL_ULong bound) :
    IdlType(tk_sequence), seqType_(seqType), bound_(bound)
  {
    if (seqType && seqType->local()) setLocal();
  }

  virtual ~SequenceType() {}

  IdlType*    seqType()      { return seqType_; }
  IDL_ULong   bound()        { return bound_; }
  IDL_Boolean shouldDelete() { return 1; }

  void accept(TypeVisitor& visitor) { visitor.visitSequenceType(this); }

private:
  IdlType*     seqType_;
  IDL_ULong bound_;
};

//
// Same goes for fixed
//

class FixedType : public IdlType {
public:
  FixedType(IDL_Short digits, IDL_Short scale) :
    IdlType(tk_fixed), digits_(digits), scale_(scale) { }

  virtual ~FixedType() {}

  IDL_UShort  digits()       { return digits_; }
  IDL_UShort  scale()        { return scale_; }
  IDL_Boolean shouldDelete() { return 1; }

  void accept(TypeVisitor& visitor) { visitor.visitFixedType(this); }

private:
  IDL_UShort digits_;
  IDL_UShort scale_;
};


//
// All other types must be declared, at least implicitly, so they have
// an associated declaration object
//

class Decl;
class DeclRepoId;

class DeclaredType : public IdlType {
public:
  DeclaredType(Kind k, Decl* decl, DeclRepoId* declRepoId)
    : IdlType(k), decl_(decl), declRepoId_(declRepoId) {}

  virtual ~DeclaredType() {}

  Decl*       decl()         { return decl_; }
  DeclRepoId* declRepoId()   { return declRepoId_; }

  IDL_Boolean shouldDelete() { return 0; }

  void accept(TypeVisitor& visitor) { visitor.visitDeclaredType(this); }

  static DeclaredType* corbaObjectType;

private:
  Decl*       decl_;
  DeclRepoId* declRepoId_;
};


//
// TypeSpec class is used to distinguish simple_type_spec from
// constr_type_spec in the grammar
//

class TypeSpec {
public:
  TypeSpec(IdlType* type, IDL_Boolean constr)
    : type_(type), constr_(constr) {}
  ~TypeSpec() {}

  IdlType*    type()   const { return type_; }
  IDL_Boolean constr() const { return constr_; }

private:
  IdlType*    type_;
  IDL_Boolean constr_;
};


#endif // _idltype_h_
