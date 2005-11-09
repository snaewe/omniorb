// -*- Mode: C++; -*-
//                              Package   : omniORB
// CORBA_LocalObject_vartypes.h Created on: 2005/09/21
//                              Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2005 Apasphere Ltd.
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

/*
  $Log$
  Revision 1.1.2.1  2005/11/09 12:22:18  dgrisby
  Local interfaces support.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// LocalObject_var ////////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject_Member;
class LocalObject_Element;
class LocalObject_INOUT_arg;
class LocalObject_OUT_arg;

class LocalObject_var {
public:
  typedef LocalObject         T;
  typedef LocalObject_ptr     T_ptr;
  typedef LocalObject_var     T_var;
  typedef LocalObject_Member  T_member;
  typedef LocalObject_Element T_element;

  inline LocalObject_var() : pd_ref(T::_nil()) {}
  inline LocalObject_var(T_ptr p) { pd_ref = p; }
  inline LocalObject_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline LocalObject_var(const T_member& p);
  inline LocalObject_var(const T_element& p);
  inline ~LocalObject_var() { release(pd_ref); }

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
  inline T_var& operator= (const T_member& p);

  inline T_var& operator= (const T_element& p);

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

  friend class LocalObject_Member;
  friend class LocalObject_INOUT_arg;
  friend class LocalObject_OUT_arg;

private:
  T_ptr pd_ref;

  // Not implemented
  LocalObject_var(const _CORBA_ObjRef_Var_base&);
  T_var& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// LocalObject_Member //////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject_Member {
public:
  typedef LocalObject         T;
  typedef LocalObject_ptr     T_ptr;
  typedef LocalObject_var     T_var;
  typedef LocalObject_Member  T_member;
  typedef LocalObject_Element T_element;

  inline LocalObject_Member() : _ptr(T::_nil()) {}
  inline LocalObject_Member(const T_member& p) {
    _ptr = T::_duplicate(p._ptr);
  }
  inline ~LocalObject_Member() {
    CORBA::release(_ptr);
  }

  inline T_member& operator= (T_ptr p) {
    CORBA::release(_ptr);
    _ptr = p;
    return *this;
  }
  inline T_member& operator= (const T_member& p) {
    if( &p != this ) {
      CORBA::release(_ptr);
      _ptr = T::_duplicate(p._ptr);
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    CORBA::release(_ptr);
    _ptr = T::_duplicate(p);
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline void operator>>= (cdrStream& s) const {
    CORBA::LocalObject::_marshalObjRef(_ptr,s);
  }
  inline void operator<<= (cdrStream& s) {
    LocalObject_ptr _result = CORBA::LocalObject::_unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }

  inline LocalObject_ptr operator->() const { return _ptr; }
  inline operator LocalObject_ptr () const  { return _ptr; }

  inline LocalObject_ptr  in() const { return _ptr; }
  inline LocalObject_ptr& inout()    { return _ptr; }
  inline LocalObject_ptr& out() {
    CORBA::release(_ptr);
    _ptr = CORBA::LocalObject::_nil();
    return _ptr;
  }
  inline LocalObject_ptr _retn() {
    LocalObject_ptr tmp;
    tmp = _ptr;
    _ptr = CORBA::LocalObject::_nil();
    return tmp;
  }

  LocalObject_ptr _ptr;

private:
  // Not implemented
  LocalObject_Member(const _CORBA_ObjRef_Var_base&);
  T_member& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// LocalObject_Element /////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject_Element {
public:
  typedef LocalObject         T;
  typedef LocalObject_ptr     T_ptr;
  typedef LocalObject_var     T_var;
  typedef LocalObject_Member  T_member;
  typedef LocalObject_Element T_element;

  inline LocalObject_Element(T_ptr& p, _CORBA_Boolean rel)
    : pd_rel(rel), pd_data(p) {}

  inline LocalObject_Element(const T_element& p)
    : pd_rel(p.pd_rel), pd_data(p.pd_data) {}

  inline ~LocalObject_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T_ptr p) {
    if( pd_rel )  CORBA::release(pd_data);
    pd_data = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p.pd_data);
      pd_data = p.pd_data;
    }
    else
      pd_data = p.pd_data;
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p);
    }
    pd_data = (T_ptr) p;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p);
    }
    pd_data = (T_ptr)p;
    return *this;
  }

  inline T_ptr operator->() const { return pd_data; }
  inline operator T_ptr () const  { return pd_data; }

  inline T_ptr in() const { return pd_data; }
  inline T_ptr& inout()         { return pd_data; }
  inline T_ptr& out() {
    if (pd_rel) {
      CORBA::release(pd_data);
    }
    pd_data = T::_nil();
    return pd_data;
  }

  inline T_ptr _retn() {
    T_ptr tmp = pd_data;
    if (!pd_rel) { T::_duplicate(pd_data); }
    pd_data = T::_nil();
    return tmp;
  }

  inline T_ptr& _NP_ref() const {return pd_data;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  T_ptr& pd_data;

private:
  // Not implemented
  LocalObject_Element(const _CORBA_ObjRef_Var_base&);
  T_element& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////// LocalObject_INOUT_arg /////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject_INOUT_arg {
public:
  inline LocalObject_INOUT_arg(LocalObject_ptr& p) : _data(p) {}
  inline LocalObject_INOUT_arg(LocalObject_var& p) : _data(p.pd_ref) {}
  inline LocalObject_INOUT_arg(LocalObject_Member& p) : _data(p._ptr) {}
  inline LocalObject_INOUT_arg(LocalObject_Element& p) : _data(p._NP_ref()) {
    // If the LocalObject_Element has pd_rel == 0,
    // the ObjRef is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the ObjRef and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p._NP_release() )
      LocalObject::_duplicate(p._NP_ref());
  }
  inline ~LocalObject_INOUT_arg() {}

  inline operator LocalObject_ptr&()  { return _data; }

  LocalObject_ptr& _data;

private:
  LocalObject_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// LocalObject_OUT_arg //////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject_OUT_arg {
public:
  typedef LocalObject        T;
  typedef LocalObject_ptr    T_ptr;
  typedef LocalObject_var    T_var;
  typedef LocalObject_Member T_member;
  typedef LocalObject_Element T_element;
  typedef LocalObject_OUT_arg T_out;

  inline LocalObject_OUT_arg(T_ptr& p) : _data(p) { _data = T::_nil(); }
  inline LocalObject_OUT_arg(T_var& p) : _data(p.pd_ref) {
    p = T::_nil();
  }
  inline LocalObject_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T::_nil();
  }
  inline LocalObject_OUT_arg(T_element& p) : _data(p._NP_ref()) {
    p = T::_nil();
  }
  inline LocalObject_OUT_arg(const T_out& p) : _data(p._data) {}
  inline ~LocalObject_OUT_arg() {}
 
  inline LocalObject_OUT_arg& operator=(const T_out& p) { 
    _data = p._data; return *this;  }
  inline LocalObject_OUT_arg& operator=(T_ptr p) { _data = p; return *this; }

  inline operator T_ptr&()  { return _data; }
  inline T_ptr& ptr()       { return _data; }
  inline T_ptr operator->() { return _data; }

  T_ptr& _data;

private:
  LocalObject_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
  // CORBA 2.3 p23-26 says that T_var assignment should be disallowed.
};

typedef LocalObject_OUT_arg LocalObject_out;
