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
  Revision 1.1.2.2  2003/11/06 11:56:55  dgrisby
  Yet more valuetype. Plain valuetype and abstract valuetype are now working.

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
    if (_pd_val != p._pd_val) {
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

  friend class _CORBA_Value_Member<T>;
  friend class _CORBA_Value_Element<T>;
  friend class _CORBA_Value_INOUT_arg<T>;
  friend class _CORBA_Value_OUT_arg<T>;

private:
  T* _pd_val;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase_Member ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Value_Member {
public:
  typedef _CORBA_Value_Var<T>     T_var;
  typedef _CORBA_Value_Member<T>  T_member;
  typedef _CORBA_Value_Element<T> T_element;

  inline _CORBA_Value_Member() : _pd_val(0) {}
  inline _CORBA_Value_Member(const T_member& p) : _pd_val(p._pd_val) {
    if (_pd_val) _pd_val->_add_ref();
  }
  inline ~_CORBA_Value_Member() {
    if (_pd_val) _pd_val->_remove_ref();
  }

  inline T_member& operator= (T* p) {
    if (_pd_val) _pd_val->_remove_ref();
    _pd_val = p;
    return *this;
  }
  inline T_member& operator= (const T_member& p) {
    if (_pd_val != p._pd_val) {
      if (_pd_val) _pd_val->_remove_ref();
      _pd_val = p._pd_val;
      if (_pd_val) _pd_val->_add_ref();
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    if (_pd_val != p._pd_val) {
      if (_pd_val) _pd_val->_remove_ref();
      _pd_val = p._pd_val;
      if (_pd_val) _pd_val->_add_ref();
    }
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline void operator>>= (cdrStream& s) const {
    T::_NP_marshal(_pd_val, s);
  }
  inline void operator<<= (cdrStream& s) {
    T* _result = T::_NP_unmarshal(s);
    if (_pd_val) _pd_val->_remove_ref();
    _pd_val = _result;
  }

  inline T* operator->() const { return _pd_val; }
  inline operator T* () const  { return _pd_val; }

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

  T* _pd_val;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase_Element ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Value_Element {
public:
  typedef _CORBA_Value_Var<T>     T_var;
  typedef _CORBA_Value_Member<T>  T_member;
  typedef _CORBA_Value_Element<T> T_element;

  inline _CORBA_Value_Element(T*& p, _CORBA_Boolean rel)
    : pd_rel(rel), _pd_val(p) {}

  inline _CORBA_Value_Element(const T_element& p)
    : pd_rel(p.pd_rel), _pd_val(p._pd_val) {}

  inline ~_CORBA_Value_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T* p) {
    if (pd_rel)
      if (_pd_val)
	_pd_val->_remove_ref();
    _pd_val = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if (pd_rel) {
      if (_pd_val)
	_pd_val->_remove_ref();

      _pd_val = p._pd_val;
      if (_pd_val)
	_pd_val->_add_ref();
    }
    else
      _pd_val = p._pd_val;
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if (pd_rel) {
      if (_pd_val)
	_pd_val->_remove_ref();

      if (p._pd_val) p._pd_val->_add_ref();
    }
    _pd_val = (T*) p;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if (pd_rel) {
      if (_pd_val)
	_pd_val->_remove_ref();

      if (p._pd_val) p._pd_val->_add_ref();
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
/////////////////////// _CORBA_Value_var          ////////////////////
/////////////////////// _CORBA_Value_member       ////////////////////
/////////////////////// operator= and copy ctors  ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
inline
_CORBA_Value_Var<T>::
_CORBA_Value_Var(const _CORBA_Value_Member<T>& m)
{
  if (m._pd_val) m._pd_val->_add_ref();
  _pd_val = m._pd_val;
}

template <class T>
inline
_CORBA_Value_Var<T>::
_CORBA_Value_Var(const _CORBA_Value_Element<T>& m)
{
  if (m._pd_val) m._pd_val->_add_ref();
  _pd_val = m._pd_val;
}


template <class T>
inline _CORBA_Value_Var<T>&
_CORBA_Value_Var<T>::operator= (const _CORBA_Value_Member<T>& p)
{
  if (_pd_val) _pd_val->_remove_ref();
  if (p._pd_val) p._pd_val->_add_ref();
  _pd_val = p._pd_val;
  return *this;
}

template <class T>
inline _CORBA_Value_Var<T>&
_CORBA_Value_Var<T>::operator= (const _CORBA_Value_Element<T>& p)
{
  if (_pd_val) _pd_val->_remove_ref();
  if (p._pd_val) p._pd_val->_add_ref();
  _pd_val = p._pd_val;
  return *this;
}

template <class T>
inline _CORBA_Value_Member<T>&
_CORBA_Value_Member<T>::operator= (const _CORBA_Value_Element<T>& p)
{
  if (_pd_val) _pd_val->_remove_ref();
  if (p._pd_val) p._pd_val->_add_ref();
  _pd_val = p._pd_val;
  return *this;
}


//////////////////////////////////////////////////////////////////////
////////////////////////// ValueBase_INOUT_arg ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Value_INOUT_arg {
public:
  typedef _CORBA_Value_Var<T>     T_var;
  typedef _CORBA_Value_Member<T>  T_member;
  typedef _CORBA_Value_Element<T> T_element;

  inline _CORBA_Value_INOUT_arg(T*& p) :        _pd_val(p) {}
  inline _CORBA_Value_INOUT_arg(T_var& p) :     _pd_val(p._pd_val) {}
  inline _CORBA_Value_INOUT_arg(T_member& p) :  _pd_val(p._pd_val) {}
  inline _CORBA_Value_INOUT_arg(T_element& p) : _pd_val(p._NP_val()) {
    // If the _CORBA_Value_Element has pd_rel == 0,
    // the value is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the value and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p._NP_release() )
      if (p._NP_val())
	p._NP_val()->_add_ref();
  }
  inline ~_CORBA_Value_INOUT_arg() {}

  inline operator T*&()  { return _pd_val; }

  T*& _pd_val;

private:
  _CORBA_Value_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// ValueBase_OUT_arg ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_Value_OUT_arg {
public:
  typedef _CORBA_Value_Var<T>     T_var;
  typedef _CORBA_Value_Member<T>  T_member;
  typedef _CORBA_Value_Element<T> T_element;
  typedef _CORBA_Value_OUT_arg<T> T_out;

  inline _CORBA_Value_OUT_arg(T*& p) : _pd_val(p) { _pd_val = 0; }
  inline _CORBA_Value_OUT_arg(T_var& p) : _pd_val(p._pd_val) {
    p = 0;
  }
  inline _CORBA_Value_OUT_arg(T_member& p) : _pd_val(p._pd_val) {
    p = 0;
  }
  inline _CORBA_Value_OUT_arg(T_element& p) : _pd_val(p._NP_val()) {
    p = 0;
  }
  inline _CORBA_Value_OUT_arg(const T_out& p) : _pd_val(p._pd_val) {}
  inline ~_CORBA_Value_OUT_arg() {}
 
  inline T_out& operator=(const T_out& p) { 
    _pd_val = p._pd_val;
    return *this;
  }
  inline T_out& operator=(T* p) {
    _pd_val = p;
    return *this;
  }

  inline operator T*&()  { return _pd_val; }
  inline T*& ptr()       { return _pd_val; }
  inline T* operator->() { return _pd_val; }

  T*& _pd_val;

private:
  _CORBA_Value_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
};



#endif // __VALUETEMPLATEDECLS_H__
