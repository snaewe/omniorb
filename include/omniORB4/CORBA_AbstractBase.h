// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_AbstractBase.h       Created on: 2004/02/16
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2004 Apasphere Ltd.
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
//    AbstractBase definitions
//

/*
  $Log$
  Revision 1.1.2.1  2004/04/02 13:26:25  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////// AbstractBase ////////////////////////////
//////////////////////////////////////////////////////////////////////

class AbstractBase;
class AbstractBase_var;
typedef AbstractBase* AbstractBase_ptr;

class AbstractBase {
public:
  typedef AbstractBase_ptr _ptr_type;
  typedef AbstractBase_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(_ptr_type);
  static _ptr_type _nil();

  Object_ptr _to_object() { return Object::_duplicate(_NP_to_object()); }

  ValueBase* _to_value() {
    ValueBase* v = _NP_to_value();
    add_ref(v);
    return v;
  }

protected:
  AbstractBase();
  AbstractBase(const AbstractBase&);
  virtual ~AbstractBase();

  //////////////////////
  // omniORB internal //
  //////////////////////

public:
  virtual Boolean _NP_is_nil() const;

  virtual Object_ptr _NP_to_object();
  virtual ValueBase* _NP_to_value();
  // Like standard versions, but do not duplicate return value
};

_CORBA_MODULE_FN inline Boolean is_nil(AbstractBase_ptr a)
{
  if (a)
    return a->_NP_is_nil();
  else
    return 1;
}

_CORBA_MODULE_FN void release(AbstractBase_ptr);


// Object reference classes for abstract interfaces (and thus object
// references for interfaces derived from abstract interfaces) inherit
// from both Object and AbstractBase, meaning that is_nil() and
// release() are ambiguous between Object and AbstractBase
// versions. This class and the associated functions resolve the
// ambiguity.
class _omni_AbstractBaseObjref :
  public virtual Object,
  public virtual AbstractBase
{
public:
  virtual Boolean _NP_is_nil() const;
  virtual Object_ptr _NP_to_object();
};

_CORBA_MODULE_FN inline Boolean is_nil(_omni_AbstractBaseObjref* a)
{
  return is_nil(a->_NP_to_object());
}

_CORBA_MODULE_FN inline void release(_omni_AbstractBaseObjref* a)
{
  release(a->_NP_to_object());
}
