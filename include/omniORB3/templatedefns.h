// -*- Mode: C++; -*-
//                            Package   : omniORB2
// templatedefns.h            Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***

#ifndef __TEMPLATEDEFNS_H__
#define __TEMPLATEDEFNS_H__


// 'inline' is commented out for constructors and destructors.
// This is to avoid a bug in g++.
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
#define __INLINE_CTOR_DEFN__
#define __INLINE_DTOR_DEFN__
#else
#define __INLINE_CTOR_DEFN__  inline
#define __INLINE_DTOR_DEFN__  inline
#endif


//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_PseudoObj_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Var<T>::_CORBA_PseudoObj_Var()
  : pd_data(T::_nil()) {}


template <class T>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Var<T>::_CORBA_PseudoObj_Var(const _CORBA_PseudoObj_Var<T>& v)
{
  pd_data = T::_duplicate(v.pd_data);
}


template <class T>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Var<T>::_CORBA_PseudoObj_Var(const _CORBA_PseudoObj_Member<T, _CORBA_PseudoObj_Var<T> >& p)
{
  pd_data = T::_duplicate(p._ptr);
}


template <class T>
__INLINE_DTOR_DEFN__
_CORBA_PseudoObj_Var<T>::~_CORBA_PseudoObj_Var()
{
  CORBA::release(pd_data);
}


template <class T>
inline _CORBA_PseudoObj_Var<T>&
_CORBA_PseudoObj_Var<T>::operator=(T* p)
{
  CORBA::release(pd_data);
  pd_data = p;
  return *this;
}


template <class T>
inline _CORBA_PseudoObj_Var<T>&
_CORBA_PseudoObj_Var<T>::operator=(const _CORBA_PseudoObj_Var<T>& v)
{
  if( pd_data != v.pd_data ) {
    CORBA::release(pd_data);
    if( v.pd_data )  pd_data = T::_duplicate(v.pd_data);
    else             pd_data = T::_nil();
  }
  return *this;
}


template <class T>
inline _CORBA_PseudoObj_Var<T>&
_CORBA_PseudoObj_Var<T>::operator=(const _CORBA_PseudoObj_Member<T, _CORBA_PseudoObj_Var<T> >& m)
{
  CORBA::release(pd_data);
  pd_data = T::_duplicate(m._ptr);
  return *this;
}


template <class T>
inline T*&
_CORBA_PseudoObj_Var<T>::out()
{
  CORBA::release(pd_data);
  pd_data = T::_nil();
  return pd_data;
}


template <class T>
inline T*
_CORBA_PseudoObj_Var<T>::_retn()
{
  T_ptr tmp = pd_data;
  pd_data = T::_nil();
  return tmp;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_PseudoObj_Out ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Out<T,T_var>::_CORBA_PseudoObj_Out(T_var& v)
  : _data(v.pd_data)
{
  // Release any storage help by <v>.
  v = T::_nil();
}

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_PseudoObj_Member //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Member<T,T_var>::_CORBA_PseudoObj_Member()
  : _ptr(T::_nil()) {}


template <class T, class T_var>
__INLINE_CTOR_DEFN__
_CORBA_PseudoObj_Member<T,T_var>::_CORBA_PseudoObj_Member(const _CORBA_PseudoObj_Member<T,T_var>& m)
{
  if( m._ptr )  _ptr = T::_duplicate(m._ptr);
  else          _ptr = T::_nil();
}


template <class T, class T_var>
__INLINE_DTOR_DEFN__
_CORBA_PseudoObj_Member<T,T_var>::~_CORBA_PseudoObj_Member()
{
  CORBA::release(_ptr);
}


template <class T, class T_var>
inline _CORBA_PseudoObj_Member<T,T_var>&
_CORBA_PseudoObj_Member<T,T_var>::operator=(T* p)
{
  CORBA::release(_ptr);
  _ptr = p;
  return *this;
}


template <class T, class T_var>
inline _CORBA_PseudoObj_Member<T,T_var>&
_CORBA_PseudoObj_Member<T,T_var>::operator=(const _CORBA_PseudoObj_Member<T,T_var>& m)
{
  if( _ptr != m._ptr ) {
    CORBA::release(_ptr);
    _ptr = T::_duplicate(m._ptr);
  }
  return *this;
}


template <class T, class T_var>
inline _CORBA_PseudoObj_Member<T,T_var>&
_CORBA_PseudoObj_Member<T,T_var>::operator=(const T_var& v)
{
  CORBA::release(_ptr);
  _ptr = T::_duplicate(v.pd_data);
  return *this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// Object_var   ////////////////////////////
//////////////////////////// Object_member////////////////////////////
//////////////////////////// operator=    ////////////////////////////
//////////////////////////// copy ctors   ////////////////////////////
//////////////////////////////////////////////////////////////////////

inline
CORBA::Object_var::Object_var(const CORBA::Object_member& p)
  : pd_ref(T::_duplicate(p._ptr))
{
}


inline CORBA::Object_var&
CORBA::Object_var::operator= (const CORBA::Object_member& p)
{
  release(pd_ref);
  pd_ref = T::_duplicate(p._ptr);
  return *this;
}


#undef __INLINE_CTOR_DEFN__
#undef __INLINE_DTOR_DEFN__


#endif  // __TEMPLATEDEFNS_H__
