// -*- Mode: C++; -*-
//                            Package   : omniORB2
// templatedefns.h            Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
_CORBA_PseudoObj_Var<T>::operator=(const _CORBA_PseudoObj_Var<T>& v) {
  if( &v != this ) {
    CORBA::release(pd_data);
    if( v.pd_data )  pd_data = T::_duplicate(v.pd_data);
    else             pd_data = T::_nil();
  }
  return *this;
}


template <class T>
inline _CORBA_PseudoObj_Var<T>&
_CORBA_PseudoObj_Var<T>::operator=(const _CORBA_PseudoObj_Member<T, _CORBA_PseudoObj_Var<T> >& m) {
  CORBA::release(pd_data);
  if( m._ptr )  pd_data = T::_duplicate(m._ptr);
  else          pd_data = T::_nil();
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
  if( &m != this ) {
    CORBA::release(_ptr);
    if( m._ptr )  _ptr = T::_duplicate(m._ptr);
    else          _ptr = T::_nil();
  }
  return *this;
}


template <class T, class T_var>
inline _CORBA_PseudoObj_Member<T,T_var>&
_CORBA_PseudoObj_Member<T,T_var>::operator=(const T_var& v)
{
  CORBA::release(_ptr);
  if( v.pd_data )  _ptr = T::_duplicate(v.pd_data);
  else             _ptr = T::_nil();
  return *this;
}

//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_ObjRef_Var /////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
__INLINE_CTOR_DEFN__
_CORBA_ObjRef_Var<T,T_Helper>::
_CORBA_ObjRef_Var(const _CORBA_ObjRef_Var<T,T_Helper>& p)
{
  if( !T_Helper::is_nil(p.pd_objref) ) {
    T_Helper::duplicate(p.pd_objref);
    pd_objref = p.pd_objref;
  }
  else
    pd_objref = T_Helper::_nil();
}


template <class T, class T_Helper>
__INLINE_CTOR_DEFN__
_CORBA_ObjRef_Var<T,T_Helper>::
_CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>& m)
{
  if (!T_Helper::is_nil(m._ptr)) {
    T_Helper::duplicate(m._ptr);
    pd_objref = m._ptr;
  }
  else
    pd_objref = T_Helper::_nil();
}


template <class T, class T_Helper>
__INLINE_DTOR_DEFN__
_CORBA_ObjRef_Var<T,T_Helper>::~_CORBA_ObjRef_Var()
{
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
  if (!T_Helper::is_nil(pd_objref)) T_Helper::release(pd_objref);
#else
  if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
#endif
}


template <class T, class T_Helper>
inline _CORBA_ObjRef_Var<T,T_Helper>&
_CORBA_ObjRef_Var<T,T_Helper>::operator= (T* p)
{
  if( !T_Helper::is_nil(pd_objref) )  T_Helper::release(pd_objref);
  pd_objref = p;
  return *this;
}


template <class T, class T_Helper>
inline _CORBA_ObjRef_Var<T,T_Helper>&
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Var<T,T_Helper>& p)
{
  if (!T_Helper::is_nil(pd_objref)) T_Helper::release(pd_objref);
  if (!T_Helper::is_nil(p.pd_objref)) {
    T_Helper::duplicate(p.pd_objref);
    pd_objref = p.pd_objref;
  }
  else
    pd_objref = T_Helper::_nil();
  return *this;
}


template <class T, class T_Helper>
inline _CORBA_ObjRef_Var<T,T_Helper>&
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Member<T,T_Helper>& p)
{
  if (!T_Helper::is_nil(pd_objref)) T_Helper::release(pd_objref);
  if (!T_Helper::is_nil(p._ptr)) {
    T_Helper::duplicate(p._ptr);
    pd_objref = p._ptr;
  }
  else
    pd_objref = T_Helper::_nil();
  return *this;
}


template <class T, class T_Helper>
inline T*&
_CORBA_ObjRef_Var<T,T_Helper>::out()
{
  if( !T_Helper::is_nil(pd_objref) ) {
    T_Helper::release(pd_objref);
    pd_objref = T_Helper::_nil();
  }
  return pd_objref;
}


template <class T, class T_Helper>
inline T*
_CORBA_ObjRef_Var<T,T_Helper>::_retn()
{
  T_ptr tmp = pd_objref;
  pd_objref = T_Helper::_nil();
  return tmp;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_Member ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
__INLINE_CTOR_DEFN__
_CORBA_ObjRef_Member<T,T_Helper>::
_CORBA_ObjRef_Member(const _CORBA_ObjRef_Member<T,T_Helper>& p)
{
  if (!T_Helper::is_nil(p._ptr)) {
    T_Helper::duplicate(p._ptr);
    _ptr = p._ptr;
  }
  else
    _ptr = T_Helper::_nil();
}


template <class T, class T_Helper>
__INLINE_DTOR_DEFN__
_CORBA_ObjRef_Member<T,T_Helper>::~_CORBA_ObjRef_Member()
{
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
  if( !T_Helper::is_nil(_ptr) )  T_Helper::release(_ptr);
#else
  if( !CORBA::is_nil(_ptr) )  CORBA::release(_ptr);
#endif
}


template <class T,class T_Helper>
inline _CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (T* p)
{
  if( !T_Helper::is_nil(_ptr) )  T_Helper::release(_ptr);
  _ptr = p;
  return *this;
}

template <class T,class T_Helper>
inline _CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (const _CORBA_ObjRef_Member<T,T_Helper>& p)
{
  if( !T_Helper::is_nil(_ptr) )  T_Helper::release(_ptr);
  if( !T_Helper::is_nil(p._ptr) ) {
    T_Helper::duplicate(p._ptr);
    _ptr = p._ptr;
  }
  else  _ptr = T_Helper::_nil();

  return *this;
}


template <class T,class T_Helper>
inline _CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (const _CORBA_ObjRef_Var<T,T_Helper>& p)
{
  if (!T_Helper::is_nil(_ptr)) T_Helper::release(_ptr);
  if (!T_Helper::is_nil(p.pd_objref)) {
    T_Helper::duplicate(p.pd_objref);
    _ptr = p.pd_objref;
  }
  else
    _ptr = T_Helper::_nil();
  return *this;
}


template <class T,class T_Helper>
inline size_t
_CORBA_ObjRef_Member<T,T_Helper>::NP_alignedSize(size_t initialoffset) const
{
  return T_Helper::NP_alignedSize(_ptr,initialoffset);
}


template <class T,class T_Helper>
inline void
_CORBA_ObjRef_Member<T,T_Helper>::operator>>= (NetBufferedStream& s) const
{
  T_Helper::marshalObjRef(_ptr,s);
}


template <class T,class T_Helper>
inline void
_CORBA_ObjRef_Member<T,T_Helper>::operator<<= (NetBufferedStream& s)
{
  T_ptr _result = T_Helper::unmarshalObjRef(s);
  T_Helper::release(_ptr);
  _ptr = _result;
}


template <class T,class T_Helper>
inline void
_CORBA_ObjRef_Member<T,T_Helper>::operator>>= (MemBufferedStream& s) const
{
  T_Helper::marshalObjRef(_ptr,s);
}


template <class T,class T_Helper>
inline void
_CORBA_ObjRef_Member<T,T_Helper>::operator<<= (MemBufferedStream& s)
{
  T_ptr _result = T_Helper::unmarshalObjRef(s);
  T_Helper::release(_ptr);
  _ptr = _result;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// _CORBA_Sequence_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class ElmType>
__INLINE_CTOR_DEFN__
_CORBA_Sequence_Var<T,ElmType>::
_CORBA_Sequence_Var(const _CORBA_Sequence_Var<T,ElmType>& p)
{
  if( !p.pd_data ) {
    pd_data = 0;
  }
  else {
    pd_data = new T;
    if( !pd_data )  _CORBA_new_operator_return_null();
    *pd_data = *p.pd_data;
  }
}


template <class T,class ElmType>
inline _CORBA_Sequence_Var<T,ElmType>&
_CORBA_Sequence_Var<T,ElmType>::operator= (const _CORBA_Sequence_Var<T,ElmType>& p)
{
  if( p.pd_data ) {
    if( !pd_data ) {
      pd_data = new T;
      if (!pd_data)  _CORBA_new_operator_return_null();
    }
    *pd_data = *p.pd_data;
  }
  else {
    if( pd_data )  delete pd_data;
    pd_data = 0;
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_ConstrType_Variable_Var ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
__INLINE_CTOR_DEFN__
_CORBA_ConstrType_Variable_Var<T>::
_CORBA_ConstrType_Variable_Var(const _CORBA_ConstrType_Variable_Var<T>& p)
{
  if( !p.pd_data )  pd_data = 0;
  else {
    pd_data = new T;
    if( !pd_data )  _CORBA_new_operator_return_null();
    *pd_data = *p.pd_data;
  }
}


template <class T>
inline _CORBA_ConstrType_Variable_Var<T>&
_CORBA_ConstrType_Variable_Var<T>::operator= (T* p)
{
  if( pd_data )  delete pd_data;
  pd_data = p;
  return *this;
}


template <class T>
inline _CORBA_ConstrType_Variable_Var<T>&
_CORBA_ConstrType_Variable_Var<T>::operator= (const _CORBA_ConstrType_Variable_Var<T>& p)
{
  if( p.pd_data ) {
    if( !pd_data ) {
      pd_data = new T;
      if( !pd_data )  _CORBA_new_operator_return_null();
    }
    *pd_data = *p.pd_data;
  }
  else {
    if( pd_data )  delete pd_data;
    pd_data = 0;
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_Bounded_Sequence //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,int max>
inline _CORBA_Bounded_Sequence<T,max>&
_CORBA_Bounded_Sequence<T,max>::operator= (const _CORBA_Bounded_Sequence<T,max>& s)
{
  _CORBA_Sequence<T>::operator= (s);
  return *this;
}


template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence<T>::length(len);
}

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Unbounded_Sequence_Array //////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class Telm,int dimension,int max>
inline
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>&
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::operator= (const _CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>& s)
{
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator= (s);
  return *this;
}


template <class T,class T_slice,class Telm,int dimension,int max>
inline void
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::length(_CORBA_ULong len)
{
  if( len > max )  _CORBA_bound_check_error();

  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length(len);
}

//////////////////////////////////////////////////////////////////////
////////////// _CORBA_Bounded_Sequence_w_FixSizeElement //////////////
//////////////////////////////////////////////////////////////////////

template <class T, int max, int elmSize, int elmAlignment>
inline _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>&
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator=(const _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>& s)
{
  _CORBA_Sequence<T>::operator=(s);
  return *this;
}


template <class T, int max, int elmSize, int elmAlignment>
inline _CORBA_ULong
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::length() const
{
  return _CORBA_Sequence<T>::length();
}


template <class T,int max,int elmSize, int elmAlignment>
inline void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::length(_CORBA_ULong len)
{
  if( len > max )  _CORBA_bound_check_error();
  _CORBA_Sequence<T>::length(len);
}

//////////////////////////////////////////////////////////////////////
/////////// _CORBA_Bounded_Sequence_Array_w_FixSizeElement ///////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>&
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator=(const
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>& s)
{
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator=(s);
  return *this;
}


template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline _CORBA_ULong
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::length() const
{
  return _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length();
}


template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::length(_CORBA_ULong len)
{
  if( len > max )  _CORBA_bound_check_error();
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length(len);
}

//////////////////////////////////////////////////////////////////////
////////////////////// _CORBA_Sequence_Array_Var /////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_slice>
inline _CORBA_Sequence_Array_Var<T,T_slice>&
_CORBA_Sequence_Array_Var<T,T_slice>::operator= (const _CORBA_Sequence_Array_Var<T,T_slice>& p)
{
  if (p.pd_data) {
    if (!pd_data) {
      pd_data = new T;
      if (!pd_data) {
	_CORBA_new_operator_return_null();
	// never reach here
      }
    }
    *pd_data = *p.pd_data;
  }
  else {
    if (pd_data) delete pd_data;
    pd_data = 0;
  }
  return *this;
}


template <class T,class T_slice>
inline
_CORBA_Sequence_Array_Var<T,T_slice>::
_CORBA_Sequence_Array_Var(const _CORBA_Sequence_Array_Var<T,T_slice>& p)
{
  if( !p.pd_data )  pd_data = 0;
  else {
    pd_data = new T;
    if( !pd_data )  _CORBA_new_operator_return_null();
    *pd_data = *p.pd_data;
  }
}


template <class T,class T_slice>
inline _CORBA_Sequence_Array_Var<T,T_slice>&
_CORBA_Sequence_Array_Var<T,T_slice>::operator= (T* p)
{
  if (pd_data) delete pd_data;
  pd_data = p;
  return *this;
}

//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_Array_Var //////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T_Helper,class T>
__INLINE_CTOR_DEFN__
_CORBA_Array_Var<T_Helper,T>::
_CORBA_Array_Var(const _CORBA_Array_Var<T_Helper,T >& p)
{
  if( !p.pd_data )  pd_data = 0;
  else {
    pd_data = T_Helper::dup(p.pd_data);
    if( !pd_data )  _CORBA_new_operator_return_null();
  }
}


template <class T_Helper,class T>
inline _CORBA_Array_Var<T_Helper,T>&
_CORBA_Array_Var<T_Helper,T>::operator= (T* p)
{
  if (pd_data) T_Helper::free(pd_data);
  pd_data = p;
  return *this;
}


template <class T_Helper,class T>
inline _CORBA_Array_Var<T_Helper,T>&
_CORBA_Array_Var<T_Helper,T>::operator= (const _CORBA_Array_Var<T_Helper,T>& p)
{
  if( pd_data )  T_Helper::free(pd_data);

  if( p.pd_data ) {
    pd_data = T_Helper::dup(p.pd_data);
    if( !pd_data )  _CORBA_new_operator_return_null();
  }
  else {
    pd_data = 0;
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////
/////////////    _CORBA_Bounded_Sequence_ObjRef //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class ElemT, class T_Helper, int max>
inline _CORBA_Bounded_Sequence_ObjRef<T,ElemT,T_Helper,max>&
_CORBA_Bounded_Sequence_ObjRef<T,ElemT,T_Helper,max>::operator= (const _CORBA_Bounded_Sequence_ObjRef<T,ElemT,T_Helper,max>& s)
{
  _CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::operator= (s);
  return *this;
}


template <class T,class ElemT, class T_Helper, int max>
inline void
_CORBA_Bounded_Sequence_ObjRef<T,ElemT,T_Helper,max>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::length(len);
}

#undef __INLINE_CTOR_DEFN__
#undef __INLINE_DTOR_DEFN__


#endif  // __TEMPLATEDEFNS_H__
