// -*- Mode: C++; -*-
//                            Package   : omniORB2
// templates.h                Created on: 14/5/96
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
//       *** PROPRIETORY INTERFACE ***

/*
  $Log$
  Revision 1.11  1998/01/27 16:11:06  ewc
  Added suppoert for type any.

  Revision 1.10  1997/12/23 19:37:57  sll
  Removed const T* conversion operator in _CORBA_Array_Var as it is causing
  conversion ambiguity.
  Demote workarounds for older DEC C++ compiler as the bug has been fixed.

  Revision 1.9  1997/12/09 20:41:02  sll
  Updated sequence array templates.

  Revision 1.8  1997/08/21 22:22:45  sll
  New templates for sequence of array.

 * Revision 1.7  1997/05/06  16:11:33  sll
 * Public release.
 *
  */

#ifndef __TEMPLATES_H__
#define __TEMPLATES_H__

class NetBufferedStream;
class MemBufferedStream;


template <class T,class T_Helper>
class _CORBA_ObjRef_Member;

template <class T,class T_var,class T_member>
class _CORBA_ObjRef_INOUT_arg;

template <class T,class T_var,class T_member,class T_Helper>
class _CORBA_ObjRef_OUT_arg;

template <class T,class T_Helper>
class _CORBA_ObjRef_Var {
public:
  typedef T* ptr_t;
  inline _CORBA_ObjRef_Var() { pd_objref = T_Helper::_nil(); }
  inline _CORBA_ObjRef_Var(T *p) { pd_objref = p; }
  inline ~_CORBA_ObjRef_Var() { 
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
    if (!T_Helper::is_nil(pd_objref)) T_Helper::release(pd_objref);
#else
    if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
#endif
  }
  inline _CORBA_ObjRef_Var(const _CORBA_ObjRef_Var<T,T_Helper> &p);
  inline _CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>&);
  inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (T * p);
  inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p);
 inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (const _CORBA_ObjRef_Member<T,T_Helper>&);
  inline T* operator->() const { return (T*) pd_objref; }

  inline operator T* () const { return pd_objref; }

  friend class _CORBA_ObjRef_Member<T,T_Helper>;
  friend class _CORBA_ObjRef_INOUT_arg<T,
                                       _CORBA_ObjRef_Var<T,T_Helper>,
                                       _CORBA_ObjRef_Member<T,T_Helper> >;
  friend class _CORBA_ObjRef_OUT_arg<T,
                                     _CORBA_ObjRef_Var<T,T_Helper>,
                                     _CORBA_ObjRef_Member<T,T_Helper>,
                                     T_Helper>;
  
private:
  T* pd_objref;
};

template <class T,class T_Helper>
class _CORBA_ObjRef_Member {
public:
  inline _CORBA_ObjRef_Member() { _ptr = T_Helper::_nil(); }
  inline _CORBA_ObjRef_Member(T *p) { _ptr = p; }
  inline _CORBA_ObjRef_Member(const _CORBA_ObjRef_Member<T,T_Helper> &p);
  inline ~_CORBA_ObjRef_Member() { 
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
    if (!T_Helper::is_nil(_ptr)) T_Helper::release(_ptr);
#else
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
#endif
  }
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (T * p);
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Member<T,T_Helper> &p);
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p);

  inline T* operator->() const { return (T*)_ptr; }

  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);

  // The following conversion operators are needed to support the
  // implicit conversion from this type to its T* data member.
  // They are used when this type is used as the rvalue of an expression.
  inline operator T* () const { return _ptr; }

  T *_ptr;
};

template <class T,class T_var,class T_member>
class _CORBA_ObjRef_INOUT_arg {
public:
  inline _CORBA_ObjRef_INOUT_arg(T*& p) : _data(p) {}
  inline _CORBA_ObjRef_INOUT_arg(T_var& p) : _data(p.pd_objref) {}
  inline _CORBA_ObjRef_INOUT_arg(T_member& p) : _data(p._ptr) {}
  T*& _data;
private:
  _CORBA_ObjRef_INOUT_arg();
};

template <class T,class T_var,class T_member,class T_Helper>
class _CORBA_ObjRef_OUT_arg {
public:
  inline _CORBA_ObjRef_OUT_arg(T*& p) : _data(p) {}
  inline _CORBA_ObjRef_OUT_arg(T_var& p) : _data(p.pd_objref) {
    p = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T_Helper::_nil();
  }
  T*& _data;
private:
  _CORBA_ObjRef_OUT_arg();
};


template <class T>
class _CORBA_ConstrType_Fix_Var {
public:
  inline _CORBA_ConstrType_Fix_Var() { }
  inline _CORBA_ConstrType_Fix_Var(T* p) { pd_data = *p; delete p; }
  inline _CORBA_ConstrType_Fix_Var(const _CORBA_ConstrType_Fix_Var<T> &p) {
    pd_data = p.pd_data;
  }
  inline ~_CORBA_ConstrType_Fix_Var() {  }
  inline _CORBA_ConstrType_Fix_Var<T> &operator= (T* p) {
    pd_data = *p;
    delete p;
    return *this;
  }
  inline _CORBA_ConstrType_Fix_Var<T> &operator= (const _CORBA_ConstrType_Fix_Var<T> &p) {
    pd_data = p.pd_data;
    return *this;
  }
  inline _CORBA_ConstrType_Fix_Var<T> &operator= (T p) {
    pd_data = p;
    return *this;
  }
  inline T* operator->() const { return (T*) &pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& () const { return (T&)pd_data; }
#else
  inline operator const T& () const { return pd_data; }
  inline operator T& () { return pd_data; }
#endif
  // This conversion operator is necessary to support the implicit conversion
  // when this var type is used as the IN or INOUT argument of an operation.

  // The following coversion operators are needed to support the casting
  // of this var type to a const T* or a T*. The CORBA spec. doesn't say
  // these castings must be supported so they are deliberately left out.
  // In fact, the operator->() can always be used to get to the T*.
  //
  // inline operator const T* () const { return pd_data; }
  // inline operator T* () { return pd_data; }

private:
  T pd_data;
};

template <class T,class T_var>
class _CORBA_ConstrType_Variable_OUT_arg;

template <class T>
class _CORBA_ConstrType_Variable_Var {
public:
  inline _CORBA_ConstrType_Variable_Var() { pd_data = 0; }
  inline _CORBA_ConstrType_Variable_Var(T* p) { pd_data = p; }
  inline _CORBA_ConstrType_Variable_Var(const _CORBA_ConstrType_Variable_Var<T> &p);
  inline ~_CORBA_ConstrType_Variable_Var() {  if (pd_data) delete pd_data; }
  inline _CORBA_ConstrType_Variable_Var<T> &operator= (T* p);
  inline _CORBA_ConstrType_Variable_Var<T> &operator= (const _CORBA_ConstrType_Variable_Var<T> &p);
  inline T* operator->() const { return (T*)pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& () const { return (T&) *pd_data; }
#else
  inline operator const T& () const { return *pd_data; }
  inline operator T& () { return *pd_data; }
#endif
  // This conversion operator is necessary to support the implicit conversion
  // when this var type is used as the IN or INOUT argument of an operation.

  // The following coversion operators are needed to support the casting
  // of this var type to a const T* or a T*. The CORBA spec. doesn't say
  // these castings must be supported so they are deliberately left out.
  // In fact, the operator->() can always be used to get to the T*.
  //
  // inline operator const T* () const { return pd_data; }
  // inline operator T* () { return pd_data; }

  friend class _CORBA_ConstrType_Variable_OUT_arg<T,_CORBA_ConstrType_Variable_Var<T> >;
private:
  T* pd_data;
};


template <class T, class T_var>
class _CORBA_ConstrType_Variable_OUT_arg {
public:
  inline _CORBA_ConstrType_Variable_OUT_arg(T*& p) : _data(p) {}
  inline _CORBA_ConstrType_Variable_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  T*& _data;
private:
  _CORBA_ConstrType_Variable_OUT_arg();
};


template <class T,class T_var>
class _CORBA_Sequence_OUT_arg;

template <class T,class ElmType>
class _CORBA_Sequence_Var {
public:
  typedef T* ptr_t;
  inline _CORBA_Sequence_Var() { pd_data = 0; }
  inline _CORBA_Sequence_Var(T* p) { pd_data = p; }
  inline _CORBA_Sequence_Var(const _CORBA_Sequence_Var<T,ElmType> &p);
  inline ~_CORBA_Sequence_Var() {  if (pd_data) delete pd_data; }
  inline _CORBA_Sequence_Var<T,ElmType> &operator= (T* p);
  inline _CORBA_Sequence_Var<T,ElmType> &operator= (const _CORBA_Sequence_Var<T,ElmType> &p);
  inline ElmType &operator[] (_CORBA_ULong index) { return (pd_data->NP_data())[index]; }
  inline const ElmType &operator[] (_CORBA_ULong index) const {
    return (pd_data->NP_data())[index];
  }
  inline T* operator->() const { return (T*)pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& () const { return *pd_data; }
#else
  inline operator const T& () const { return *pd_data; }
  inline operator T& () { return *pd_data; }
#endif
  // This conversion operator is necessary to support the implicit conversion
  // when this var type is used as the IN or INOUT argument of an operation.

  // The following coversion operators are needed to support the casting
  // of this var type to a const T* or a T*. The CORBA spec. doesn't say
  // these castings must be supported so they are deliberately left out.
  // In fact, the operator->() can always be used to get to the T*.
  //
  // inline operator const T* () const { return pd_data; }
  // inline operator T* () { return pd_data; }

  friend class _CORBA_Sequence_OUT_arg<T,_CORBA_Sequence_Var<T,ElmType> >;

private:
  T* pd_data;
};

template <class T,class T_slice>
class _CORBA_Sequence_Array_Var {
public:
  typedef T* ptr_t;
  inline _CORBA_Sequence_Array_Var() { pd_data = 0; }
  inline _CORBA_Sequence_Array_Var(T* p) { pd_data = p; }
  inline _CORBA_Sequence_Array_Var(const _CORBA_Sequence_Array_Var<T,T_slice> &p);
  inline ~_CORBA_Sequence_Array_Var() {  if (pd_data) delete pd_data; }
  inline _CORBA_Sequence_Array_Var<T,T_slice> &operator= (T* p);
  inline _CORBA_Sequence_Array_Var<T,T_slice> &operator= (const _CORBA_Sequence_Array_Var<T,T_slice> &p);
  inline T_slice* operator[] (_CORBA_ULong index) { return (T_slice*)((pd_data->NP_data())[index]); }
  inline const T_slice* operator[] (_CORBA_ULong index) const {
    return (const T_slice*)((pd_data->NP_data())[index]);
  }
  inline T* operator->() const { return (T*)pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& () const { return *pd_data; }
#else
  inline operator const T& () const { return *pd_data; }
  inline operator T& () { return *pd_data; }
#endif
  // This conversion operator is necessary to support the implicit conversion
  // when this var type is used as the IN or INOUT argument of an operation.

  // The following coversion operators are needed to support the casting
  // of this var type to a const T* or a T*. The CORBA spec. doesn't say
  // these castings must be supported so they are deliberately left out.
  // In fact, the operator->() can always be used to get to the T*.
  //
  // inline operator const T* () const { return pd_data; }
  // inline operator T* () { return pd_data; }

  friend class _CORBA_Sequence_OUT_arg<T,_CORBA_Sequence_Array_Var<T,T_slice> >;

private:
  T* pd_data;
};


template <class T, class T_var>
class _CORBA_Sequence_OUT_arg {
public:
  inline _CORBA_Sequence_OUT_arg(T*& p) : _data(p) {}
  inline _CORBA_Sequence_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  T*& _data;
private:
  _CORBA_Sequence_OUT_arg();
};

template <class T, class T_var>
class _CORBA_Array_OUT_arg;

template <class T_Helper,class T>
class _CORBA_Array_Var {
public:
  inline _CORBA_Array_Var () { pd_data = 0; }
  inline _CORBA_Array_Var (T* p) { pd_data = p; }
  inline _CORBA_Array_Var (const _CORBA_Array_Var<T_Helper,T>& p);
  inline ~_CORBA_Array_Var() { if (pd_data) T_Helper::free(pd_data); }
  inline _CORBA_Array_Var<T_Helper,T> &operator= (T* p);
  inline _CORBA_Array_Var<T_Helper,T> &operator= (const _CORBA_Array_Var<T_Helper,T>& p);
  inline T& operator[] (_CORBA_ULong index) { return *(pd_data + index); }
  inline const T& operator[] (_CORBA_ULong index) const { return *( (const T*) (pd_data + index));  }
  inline operator T* () const { return pd_data; }
  // Define the const T* operator() causes conversion operator ambiguity with 
  // some compilers. Should be alright to leave this operator out. If not,
  // reinstate it and #ifdef it with the right compiler specific macro.
  //
  //  inline operator const T* () const { return (const T*) pd_data; }

  friend class _CORBA_Array_OUT_arg<T, _CORBA_Array_Var<T_Helper,T> >;

private:
  T* pd_data;
};

template <class T, class T_var>
class _CORBA_Array_OUT_arg {
public:
  inline _CORBA_Array_OUT_arg(T*& p) : _data(p) {}
  inline _CORBA_Array_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  T*& _data;
private:
  _CORBA_Array_OUT_arg();
};

template <class T_Helper,class T>
class _CORBA_Array_Forany {
public:
  inline _CORBA_Array_Forany () { pd_data = 0; }
  inline _CORBA_Array_Forany (T* p,_CORBA_Boolean nocopy = 0);
  inline _CORBA_Array_Forany (const _CORBA_Array_Forany<T_Helper,T>& p);
  inline ~_CORBA_Array_Forany() {
    // does not delete the storage of the underlying array
  }
  inline _CORBA_Array_Forany<T_Helper,T> &operator= (T* p);
  inline T& operator[] (_CORBA_ULong index) { return *(pd_data + index); }
  inline const T& operator[] (_CORBA_ULong index) const { return *( (const T*) (pd_data + index)); }
  inline operator T* () const { return pd_data; }
  inline operator const T* () const { return (const T*) pd_data; }
private:
  T* pd_data;
};


template <class T,int max>
inline 
_CORBA_Bounded_Sequence<T,max>&
_CORBA_Bounded_Sequence<T,max>::operator= (const _CORBA_Bounded_Sequence<T,max> &s)
{
  _CORBA_Sequence<T>::operator= (s);
  return *this;
}

template <class T,int max>
inline 
void
_CORBA_Bounded_Sequence<T,max>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence<T>::length(len);
  return;
}

template <class T,class T_slice,class Telm,int dimension,int max>
inline 
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>&
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::operator= (const _CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max> &s)
{
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator= (s);
  return *this;
}

template <class T,class T_slice,class Telm,int dimension,int max>
inline 
void
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length(len);
  return;
}


template <class T,int max,int elmSize, int elmAlignment>
inline
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>&
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator=(const 
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>& s) 
{
  _CORBA_Sequence<T>::operator=(s);
  return *this;
}

template <class T,int max,int elmSize, int elmAlignment>
inline
_CORBA_ULong
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::length() const
{ 
  return _CORBA_Sequence<T>::length();
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence<T>::length(len);
  return;
}

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>&
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator=(const 
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>& s) 
{
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator=(s);
  return *this;
}

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline
_CORBA_ULong
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::length() const
{ 
  return _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length();
}

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::length(_CORBA_ULong len)
{
  if (len > max) {
    _CORBA_bound_check_error();
    // never reach here
  }
  _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length(len);
  return;
}


template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper>::_CORBA_ObjRef_Var(const _CORBA_ObjRef_Var<T,T_Helper> &p) 
{
  if (!T_Helper::is_nil(p.pd_objref)) {
    T_Helper::duplicate(p.pd_objref);
    pd_objref = p.pd_objref;
  }
  else
    pd_objref = T_Helper::_nil();
}

template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper>::_CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>& p)
{
  if (!T_Helper::is_nil(p._ptr)) {
    T_Helper::duplicate(p._ptr);
    pd_objref = p._ptr;
  }
  else
    pd_objref = T_Helper::_nil();
}

template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper> &
_CORBA_ObjRef_Var<T,T_Helper>::operator= (T * p)
{
  if (!T_Helper::is_nil(pd_objref)) T_Helper::release(pd_objref);
  pd_objref = p;
  return *this;
}

template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper> &
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p)
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

template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper> &
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

template <class T,class T_Helper>
inline 
_CORBA_ObjRef_Member<T,T_Helper>::_CORBA_ObjRef_Member(const _CORBA_ObjRef_Member<T,T_Helper> &p)
{
  if (!T_Helper::is_nil(p._ptr)) {
    T_Helper::duplicate(p._ptr);
    _ptr = p._ptr;
  }
  else
    _ptr = T_Helper::_nil();
}

template <class T,class T_Helper>
inline 
_CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (T * p)
{
  if (!T_Helper::is_nil(_ptr)) T_Helper::release(_ptr);
  _ptr = p;
  return *this;
}

template <class T,class T_Helper>
inline 
_CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (const _CORBA_ObjRef_Member<T,T_Helper> &p)
{
  if (!T_Helper::is_nil(_ptr)) T_Helper::release(_ptr);
  if (!T_Helper::is_nil(p._ptr)) {
    T_Helper::duplicate(p._ptr);
    _ptr = p._ptr;
  }
  else
    _ptr = T_Helper::_nil();
  return *this;
}

template <class T,class T_Helper>
inline 
_CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p)
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
inline 
size_t 
_CORBA_ObjRef_Member<T,T_Helper>::NP_alignedSize(size_t initialoffset) const 
{
  return T_Helper::NP_alignedSize(_ptr,initialoffset);
}

template <class T,class T_Helper>
inline 
void
_CORBA_ObjRef_Member<T,T_Helper>::operator>>= (NetBufferedStream &s) const 
{
  T_Helper::marshalObjRef(_ptr,s);
}

template <class T,class T_Helper>
inline 
void
_CORBA_ObjRef_Member<T,T_Helper>::operator<<= (NetBufferedStream &s)
{
  T* _result = T_Helper::unmarshalObjRef(s);
  T_Helper::release(_ptr);
  _ptr = _result;
}

template <class T,class T_Helper>
inline 
void
_CORBA_ObjRef_Member<T,T_Helper>::operator>>= (MemBufferedStream &s) const
{
  T_Helper::marshalObjRef(_ptr,s);
}

template <class T,class T_Helper>
inline 
void
_CORBA_ObjRef_Member<T,T_Helper>::operator<<= (MemBufferedStream &s)
{
  T* _result = T_Helper::unmarshalObjRef(s);
  T_Helper::release(_ptr);
  _ptr = _result;
}

template <class T>
inline 
_CORBA_ConstrType_Variable_Var<T>::_CORBA_ConstrType_Variable_Var(const _CORBA_ConstrType_Variable_Var<T> &p) 
{
  if (!p.pd_data) {
    pd_data = 0;
    return;
  }
  else {
    pd_data = new T;
    if (!pd_data) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    *pd_data = *p.pd_data;
  }
}

template <class T>
inline
_CORBA_ConstrType_Variable_Var<T> &
_CORBA_ConstrType_Variable_Var<T>::operator= (T* p)
{
  if (pd_data) delete pd_data;
  pd_data = p;
  return *this;
}

template <class T>
inline
_CORBA_ConstrType_Variable_Var<T> &
_CORBA_ConstrType_Variable_Var<T>::operator= (const _CORBA_ConstrType_Variable_Var<T> &p)
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

template <class T,class ElmType>
inline
_CORBA_Sequence_Var<T,ElmType> &
_CORBA_Sequence_Var<T,ElmType>::operator= (const _CORBA_Sequence_Var<T,ElmType> &p)
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

template <class T,class ElmType>
inline 
_CORBA_Sequence_Var<T,ElmType>::_CORBA_Sequence_Var(const _CORBA_Sequence_Var<T,ElmType> &p) 
{
  if (!p.pd_data) {
    pd_data = 0;
    return;
  }
  else {
    pd_data = new T;
    if (!pd_data) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    *pd_data = *p.pd_data;
  }
}

template <class T,class ElmType>
inline
_CORBA_Sequence_Var<T,ElmType> &
_CORBA_Sequence_Var<T,ElmType>::operator= (T* p)
{
  if (pd_data) delete pd_data;
  pd_data = p;
  return *this;
}

template <class T,class T_slice>
inline
_CORBA_Sequence_Array_Var<T,T_slice> &
_CORBA_Sequence_Array_Var<T,T_slice>::operator= (const _CORBA_Sequence_Array_Var<T,T_slice> &p)
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
_CORBA_Sequence_Array_Var<T,T_slice>::_CORBA_Sequence_Array_Var(const _CORBA_Sequence_Array_Var<T,T_slice> &p) 
{
  if (!p.pd_data) {
    pd_data = 0;
    return;
  }
  else {
    pd_data = new T;
    if (!pd_data) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    *pd_data = *p.pd_data;
  }
}

template <class T,class T_slice>
inline
_CORBA_Sequence_Array_Var<T,T_slice> &
_CORBA_Sequence_Array_Var<T,T_slice>::operator= (T* p)
{
  if (pd_data) delete pd_data;
  pd_data = p;
  return *this;
}


template <class T_Helper,class T>
inline
_CORBA_Array_Var<T_Helper,T>::_CORBA_Array_Var (const _CORBA_Array_Var<T_Helper,T >& p) 
{
  if (!p.pd_data) {
    pd_data = 0;
  }
  else {
    pd_data = T_Helper::dup(p.pd_data);
    if (!pd_data) {
      _CORBA_new_operator_return_null();
    }
  }
}

template <class T_Helper,class T>
inline 
_CORBA_Array_Var<T_Helper,T>&
_CORBA_Array_Var<T_Helper,T>::operator= (T* p) 
{
  if (pd_data) T_Helper::free(pd_data);
  pd_data = p;
  return *this;
}

template <class T_Helper,class T>
inline 
_CORBA_Array_Var<T_Helper,T>&
_CORBA_Array_Var<T_Helper,T>::operator= (const _CORBA_Array_Var<T_Helper,T>& p) 
{
  if (pd_data) {
    T_Helper::free(pd_data);
  }
  if (p.pd_data) {
    pd_data = T_Helper::dup(p.pd_data);
    if (!pd_data) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
  }
  else {
    pd_data = 0;
  }
  return *this;
}

template <class T_Helper,class T>
inline 
_CORBA_Array_Forany<T_Helper,T>::_CORBA_Array_Forany (T* p,_CORBA_Boolean nocopy)
{
  if (nocopy) {
    pd_data = p;
  }
  else {
    if (p) {
      pd_data = T_Helper::dup(p);
      if (!pd_data) {
	_CORBA_new_operator_return_null();
	// never reach here
      }
    }
    else {
      pd_data = 0;
    }
  }
}


template <class T_Helper,class T>
inline 
_CORBA_Array_Forany<T_Helper,T>&
_CORBA_Array_Forany<T_Helper,T>::operator= (T* p) 
{
  pd_data = p;
  return *this;
}

template <class T_Helper,class T>
inline 
_CORBA_Array_Forany<T_Helper,T>::_CORBA_Array_Forany (const _CORBA_Array_Forany<T_Helper,T>& p)
{
  if (!p.pd_data) {
    pd_data = 0;
  }
  else {
    pd_data = T_Helper::dup(p.pd_data);
    if (!pd_data) {
      _CORBA_new_operator_return_null();
    }
  }
}

#endif // __TEMPLATES_H__

