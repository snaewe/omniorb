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
  Revision 1.1.2.2  2004/10/13 17:58:18  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

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

class AbstractBase_var;

class AbstractBase {
public:
  typedef AbstractBase_ptr _ptr_type;
  typedef AbstractBase_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(_ptr_type);
  static _ptr_type _nil();

  inline Object_ptr _to_object() {
    return Object::_duplicate(_NP_to_object());
  }

  inline ValueBase* _to_value() {
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

  static _dyn_attr const char* _PD_repoId;

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
// release() are ambiguous between Object and AbstractBase versions.
// This class and the associated functions resolve the ambiguity.

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


//////////////////////////////////////////////////////////////////////
//////////////////////////// AbstractBase_var ////////////////////////
//////////////////////////////////////////////////////////////////////

class AbstractBase_var {
public:
  typedef AbstractBase         T;
  typedef AbstractBase_ptr     T_ptr;
  typedef AbstractBase_var     T_var;

  inline AbstractBase_var() : pd_ref(T::_nil()) {}
  inline AbstractBase_var(T_ptr p) { pd_ref = p; }
  inline AbstractBase_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline ~AbstractBase_var() { release(pd_ref); }

  inline T_var& operator= (T_ptr p) {
    release(pd_ref);
    pd_ref = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if( pd_ref != p.pd_ref ) {
      release(pd_ref);
      pd_ref = T::_duplicate(p.pd_ref);
    }
    return *this;
  }
  inline T_ptr operator->() const { return pd_ref; }
  inline operator T_ptr() const   { return pd_ref; }

  inline T_ptr in() const { return pd_ref; }
  inline T_ptr& inout()   { return pd_ref; }
  inline T_ptr& out() {
    if( !is_nil(pd_ref) ) {
      release(pd_ref);
      pd_ref = T::_nil();
    }
    return pd_ref;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_ref;
    pd_ref = T::_nil();
    return tmp;
  }

private:
  T_ptr pd_ref;
};
