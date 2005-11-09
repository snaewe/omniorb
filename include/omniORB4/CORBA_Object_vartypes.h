// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Object_vartypes.h    Created on: 2001/08/15
//                            Author    : Duncan Grisby <dpg1>
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//

/*
  $Log$
  Revision 1.1.4.3  2005/11/09 12:16:46  dgrisby
  Remove obsolete tcDesc helper classes.

  Revision 1.1.4.2  2004/02/16 10:10:28  dgrisby
  More valuetype, including value boxes. C++ mapping updates.

  Revision 1.1.4.1  2003/03/23 21:04:23  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/17 13:39:47  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// Object_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Member;
class Object_Element;
class Object_INOUT_arg;
class Object_OUT_arg;

class Object_var {
public:
  typedef Object         T;
  typedef Object_ptr     T_ptr;
  typedef Object_var     T_var;
  typedef Object_Member  T_member;
  typedef Object_Element T_element;

  inline Object_var() : pd_ref(T::_nil()) {}
  inline Object_var(T_ptr p) { pd_ref = p; }
  inline Object_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline Object_var(const T_member& p);
  inline Object_var(const T_element& p);
  inline ~Object_var() { release(pd_ref); }

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

  friend class Object_Member;
  friend class Object_INOUT_arg;
  friend class Object_OUT_arg;

private:
  T_ptr pd_ref;

  // Not implemented
  Object_var(const _CORBA_ObjRef_Var_base&);
  T_var& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// Object_Member ///////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Member {
public:
  typedef Object         T;
  typedef Object_ptr     T_ptr;
  typedef Object_var     T_var;
  typedef Object_Member  T_member;
  typedef Object_Element T_element;

  inline Object_Member() : _ptr(T::_nil()) {}
  inline Object_Member(const T_member& p) {
    _ptr = T::_duplicate(p._ptr);
  }
  inline ~Object_Member() {
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
    CORBA::Object::_marshalObjRef(_ptr,s);
  }
  inline void operator<<= (cdrStream& s) {
    Object_ptr _result = CORBA::Object::_unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }

  inline Object_ptr operator->() const { return _ptr; }
  inline operator Object_ptr () const  { return _ptr; }

  inline Object_ptr  in() const { return _ptr; }
  inline Object_ptr& inout()    { return _ptr; }
  inline Object_ptr& out() {
    CORBA::release(_ptr);
    _ptr = CORBA::Object::_nil();
    return _ptr;
  }
  inline Object_ptr _retn() {
    Object_ptr tmp;
    tmp = _ptr;
    _ptr = CORBA::Object::_nil();
    return tmp;
  }

  Object_ptr _ptr;

private:
  // Not implemented
  Object_Member(const _CORBA_ObjRef_Var_base&);
  T_member& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// Object_Element //////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Element {
public:
  typedef Object         T;
  typedef Object_ptr     T_ptr;
  typedef Object_var     T_var;
  typedef Object_Member  T_member;
  typedef Object_Element T_element;

  inline Object_Element(T_ptr& p, _CORBA_Boolean rel)
    : pd_rel(rel), pd_data(p) {}

  inline Object_Element(const T_element& p)
    : pd_rel(p.pd_rel), pd_data(p.pd_data) {}

  inline ~Object_Element() {
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
  Object_Element(const _CORBA_ObjRef_Var_base&);
  T_element& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////// Object_INOUT_arg //////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_INOUT_arg {
public:
  inline Object_INOUT_arg(Object_ptr& p) : _data(p) {}
  inline Object_INOUT_arg(Object_var& p) : _data(p.pd_ref) {}
  inline Object_INOUT_arg(Object_Member& p) : _data(p._ptr) {}
  inline Object_INOUT_arg(Object_Element& p) : _data(p._NP_ref()) {
    // If the Object_Element has pd_rel == 0,
    // the ObjRef is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the ObjRef and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p._NP_release() )
      Object::_duplicate(p._NP_ref());
  }
  inline ~Object_INOUT_arg() {}

  inline operator Object_ptr&()  { return _data; }

  Object_ptr& _data;

private:
  Object_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// Object_OUT_arg ///////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_OUT_arg {
public:
  typedef Object        T;
  typedef Object_ptr    T_ptr;
  typedef Object_var    T_var;
  typedef Object_Member T_member;
  typedef Object_Element T_element;
  typedef Object_OUT_arg T_out;

  inline Object_OUT_arg(T_ptr& p) : _data(p) { _data = T::_nil(); }
  inline Object_OUT_arg(T_var& p) : _data(p.pd_ref) {
    p = T::_nil();
  }
  inline Object_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T::_nil();
  }
  inline Object_OUT_arg(T_element& p) : _data(p._NP_ref()) {
    p = T::_nil();
  }
  inline Object_OUT_arg(const T_out& p) : _data(p._data) {}
  inline ~Object_OUT_arg() {}
 
  inline Object_OUT_arg& operator=(const T_out& p) { 
    _data = p._data; return *this;  }
  inline Object_OUT_arg& operator=(T_ptr p) { _data = p; return *this; }

  inline operator T_ptr&()  { return _data; }
  inline T_ptr& ptr()       { return _data; }
  inline T_ptr operator->() { return _data; }

  T_ptr& _data;

private:
  Object_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
  // CORBA 2.3 p23-26 says that T_var assignment should be disallowed.
};

typedef Object_OUT_arg Object_out;
