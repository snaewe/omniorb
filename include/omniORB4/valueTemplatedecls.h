// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueTemplatedecls.h       Created on: 2003/09/24
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003 Apasphere Ltd.
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
//    ValueType templates
//

/*
  $Log$
  Revision 1.1.2.1  2003/09/26 16:12:53  dgrisby
  Start of valuetype support.

*/

#ifndef __VALUETEMPLATEDECLS_H__
#define __VALUETEMPLATEDECLS_H__


template <class T>
class _CORBA_Value_Member;

template <class T>
class _CORBA_Value_Element;

template <class T>
class _CORBA_Value_INOUT_arg;

template <class T>
class _CORBA_Value_OUT_arg;

//////////////////////////////////////////////////////////////////////
//////////////////////////// _CORBA_Value_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Value_Var {
public:
  typedef _CORBA_Value_Var<T>     T_var;
  typedef _CORBA_Value_Member<T>  T_member;
  typedef _CORBA_Value_Element<T> T_element;

  inline _CORBA_Value_Var() : _pd_val(0) {}
  inline _CORBA_Value_Var(T* p) { _pd_val = p; }
  inline _CORBA_Value_Var(const T_var& p) : _pd_val(p._pd_val) {
    if (_pd_val) _pd_val->add_ref();
  }
  inline _CORBA_Value_Var(const T_member& p);
  inline _CORBA_Value_Var(const T_element& p);
  inline ~_CORBA_Value_Var() {
    if (_pd_val) _pd_val->_remove_ref();
  }

  inline T_var& operator= (T* p) {
    if (_pd_val) _pd_val->_remove_ref();
    _pd_val = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if( _pd_val != p._pd_val ) {
      if (_pd_val) _pd_val->_remove_ref();
      _pd_val = p._pd_val;
      if (_pd_val) _pd_val->_add_ref();
    }
    return *this;
  }
  inline T_var& operator= (const T_member& p);

  inline T_var& operator= (const T_element& p);

  inline T* operator->() const { return _pd_val; }
  inline operator T*() const   { return _pd_val; }

  inline T*  in() const { return _pd_val; }
  inline T*& inout()    { return _pd_val; }
  inline T*& out() {
    if (_pd_val) _pd_val->_remove_ref();
    _pd_val = 0;
    return _pd_val;
  }
  inline T* _retn() {
    T* tmp = _pd_val;
    _pd_val = 0;
    return tmp;
  }

  friend class ValueBase_Member;
  friend class ValueBase_Element;
  friend class ValueBase_INOUT_arg;
  friend class ValueBase_OUT_arg;
  friend class ValueBase_tcDesc_arg;

private:
  T* _pd_val;
};

#if 0
//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase_Member ////////////////////////
//////////////////////////////////////////////////////////////////////

class ValueBase_Member {
public:
  typedef ValueBase         T;
  typedef ValueBase_var     T_var;
  typedef ValueBase_Member  T_member;
  typedef ValueBase_Element T_element;

  inline ValueBase_Member() : _pd_val(0) {}
  inline ValueBase_Member(const T_member& p) {
    _pd_val = p._pd_val;
    add_ref(_pd_val);
  }
  inline ~ValueBase_Member() {
    remove_ref(_pd_val);
  }

  inline T_member& operator= (T* p) {
    remove_ref(_pd_val);
    _pd_val = p;
    return *this;
  }
  inline T_member& operator= (const T_member& p) {
    if( &p != this ) {
      remove_ref(_pd_val);
      _pd_val = p._pd_val;
      add_ref(_pd_val);
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    remove_ref(_pd_val);
    _pd_val = p._pd_val;
    add_ref(_pd_val);
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline void operator>>= (cdrStream& s) const {
    T::_NP_marshal(_pd_val, s);
  }
  inline void operator<<= (cdrStream& s) {
    ValueBase* _result = T::_NP_unmarshal(s);
    remove_ref(_pd_val);
    _pd_val = _result;
  }

  inline ValueBase* operator->() const { return _pd_val; }
  inline operator ValueBase* () const  { return _pd_val; }

  inline ValueBase*  in() const { return _pd_val; }
  inline ValueBase*& inout()    { return _pd_val; }
  inline ValueBase*& out() {
    remove_ref(_pd_val);
    _pd_val = 0;
    return _pd_val;
  }
  inline ValueBase* _retn() {
    ValueBase* tmp;
    tmp = _pd_val;
    _pd_val = 0;
    return tmp;
  }

  ValueBase* _pd_val;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase_Element ///////////////////////
//////////////////////////////////////////////////////////////////////

class ValueBase_Element {
public:
  typedef ValueBase         T;
  typedef ValueBase_var     T_var;
  typedef ValueBase_Member  T_member;
  typedef ValueBase_Element T_element;

  inline ValueBase_Element(T*& p, _CORBA_Boolean rel)
    : pd_rel(rel), _pd_val(p) {}

  inline ValueBase_Element(const T_element& p)
    : pd_rel(p.pd_rel), _pd_val(p._pd_val) {}

  inline ~ValueBase_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T* p) {
    if( pd_rel )
      if (_pd_val)
	_pd_val->_remove_ref();
    _pd_val = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if( pd_rel ) {
      if (_pd_val)
	_pd_val->_remove_ref();

      _pd_val = p._pd_val;
      _pd_val->_add_ref();
    }
    else
      _pd_val = p._pd_val;
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if( pd_rel ) {
      if (_pd_val)
	_pd_val->_remove_ref();

      p._pd_val->_add_ref();
    }
    _pd_val = (T*) p;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if( pd_rel ) {
      if (_pd_val)
	_pd_val->_remove_ref();

      p._pd_val->_add_ref();
    }
    _pd_val = (T*)p;
    return *this;
  }

  inline T* operator->() const { return _pd_val; }
  inline operator T* () const  { return _pd_val; }

  inline T* in() const { return _pd_val; }
  inline T*& inout()   { return _pd_val; }
  inline T*& out() {
    if (pd_rel) {
      if (_pd_val)
	_pd_val->_remove_ref();
    }
    _pd_val = 0;
    return _pd_val;
  }

  inline T* _retn() {
    T* tmp = _pd_val;
    if (!pd_rel) {
      if (_pd_val)
	_pd_val->_add_ref();
    }
    _pd_val = 0;
    return tmp;
  }

  inline T*& _NP_val() const {return _pd_val;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  T*& _pd_val;
};

//////////////////////////////////////////////////////////////////////
////////////////////////// ValueBase_INOUT_arg ///////////////////////
//////////////////////////////////////////////////////////////////////

class ValueBase_INOUT_arg {
public:
  inline ValueBase_INOUT_arg(ValueBase*& p) :        _pd_val(p) {}
  inline ValueBase_INOUT_arg(ValueBase_var& p) :     _pd_val(p._pd_val) {}
  inline ValueBase_INOUT_arg(ValueBase_Member& p) :  _pd_val(p._pd_val) {}
  inline ValueBase_INOUT_arg(ValueBase_Element& p) : _pd_val(p._NP_val()) {
    // If the ValueBase_Element has pd_rel == 0,
    // the ObjRef is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the value and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p._NP_release() )
      if (p._NP_val())
	p._NP_val()->_add_ref();
  }
  inline ~ValueBase_INOUT_arg() {}

  inline operator ValueBase*&()  { return _pd_val; }

  ValueBase*& _pd_val;

private:
  ValueBase_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// ValueBase_OUT_arg ////////////////////////
//////////////////////////////////////////////////////////////////////

class ValueBase_OUT_arg {
public:
  typedef ValueBase         T;
  typedef ValueBase_var     T_var;
  typedef ValueBase_Member  T_member;
  typedef ValueBase_Element T_element;
  typedef ValueBase_OUT_arg T_out;

  inline ValueBase_OUT_arg(T*& p) : _pd_val(p) { _pd_val = 0; }
  inline ValueBase_OUT_arg(T_var& p) : _pd_val(p._pd_val) {
    p = 0;
  }
  inline ValueBase_OUT_arg(T_member& p) : _pd_val(p._pd_val) {
    p = 0;
  }
  inline ValueBase_OUT_arg(T_element& p) : _pd_val(p._NP_val()) {
    p = 0;
  }
  inline ValueBase_OUT_arg(const T_out& p) : _pd_val(p._pd_val) {}
  inline ~ValueBase_OUT_arg() {}
 
  inline ValueBase_OUT_arg& operator=(const T_out& p) { 
    _pd_val = p._pd_val;
    return *this;
  }
  inline ValueBase_OUT_arg& operator=(T* p) {
    _pd_val = p;
    return *this;
  }

  inline operator T*&()  { return _pd_val; }
  inline T*& ptr()       { return _pd_val; }
  inline T* operator->() { return _pd_val; }

  T*& _pd_val;

private:
  ValueBase_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
};


typedef ValueBase_OUT_arg ValueBase_out;

#endif


#endif // __VALUETEMPLATEDECLS_H__
