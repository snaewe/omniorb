// -*- Mode: C++; -*-
//                            Package   : omniORB
// templatedecls.h            Created on: 14/5/96
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

#ifndef __TEMPLATEDECLS_H__
#define __TEMPLATEDECLS_H__


// Empty class to prevent illegal objref _var widening
class _CORBA_ObjRef_Var_base {};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_PseudoObj_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var> class _CORBA_PseudoObj_Member;
template <class T, class T_var> class _CORBA_PseudoObj_Out;

template <class T>
class _CORBA_PseudoObj_Var : public _CORBA_ObjRef_Var_base {
public:
  typedef T* T_ptr;
  typedef _CORBA_PseudoObj_Var<T> T_var;
  typedef _CORBA_PseudoObj_Member<T,T_var> T_member;

  inline _CORBA_PseudoObj_Var();
  inline _CORBA_PseudoObj_Var(T_ptr p) : pd_data(p) {}
  inline _CORBA_PseudoObj_Var(const T_var& v);
  inline _CORBA_PseudoObj_Var(const T_member& m);

  inline ~_CORBA_PseudoObj_Var();

  inline T_var& operator=(T_ptr p);
  inline T_var& operator=(const T_var& v);
  inline T_var& operator=(const T_member& v);

  inline T_ptr operator->() const { return pd_data; }
  inline operator T_ptr () const  { return pd_data; }

  inline T*     in() const { return pd_data; }
  inline T_ptr& inout()    { return pd_data; }
  inline T_ptr& out();
  inline T_ptr _retn();

  friend class _CORBA_PseudoObj_Out<T, _CORBA_PseudoObj_Var<T> >;
  friend class _CORBA_PseudoObj_Member<T, _CORBA_PseudoObj_Var<T> >;

protected:
  T_ptr pd_data;

private:
  // Not implemented
  _CORBA_PseudoObj_Var(const _CORBA_ObjRef_Var_base&);
  T_var& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_PseudoObj_Member //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_PseudoObj_Member {
public:
  typedef T* T_ptr;
  typedef _CORBA_PseudoObj_Member<T,T_var> T_member;

  inline _CORBA_PseudoObj_Member();
  inline _CORBA_PseudoObj_Member(T_ptr p) : _ptr(p) {}
  inline _CORBA_PseudoObj_Member(const T_member& p);
  inline ~_CORBA_PseudoObj_Member();

  inline T_member& operator=(T_ptr p);
  inline T_member& operator=(const T_member& p);
  inline T_member& operator=(const T_var& p);

  inline T_ptr operator->() const { return _ptr; }
  inline operator T_ptr () const  { return _ptr; }

  T_ptr _ptr;

private:
  // Not implemented
  _CORBA_PseudoObj_Member(const _CORBA_ObjRef_Var_base&);
  T_member& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_PseudoObj_InOut ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var, class T_mbr>
class _CORBA_PseudoObj_InOut {
public:
  inline _CORBA_PseudoObj_InOut(T* p) : _data(p) {}
  inline _CORBA_PseudoObj_InOut(T_var& v) : _data(v) {}
  inline _CORBA_PseudoObj_InOut(T_mbr& m) : _data(m._ptr) {}

  T* _data;

private:
  _CORBA_PseudoObj_InOut();  // Not implemented
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_PseudoObj_Out ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_PseudoObj_Out {
public:
  inline _CORBA_PseudoObj_Out(T*& p) : _data(p) {}
  inline _CORBA_PseudoObj_Out(T_var& p);

  inline _CORBA_PseudoObj_Out<T,T_var>& operator=(T* p) {
    _data = p;
    return *this;
  }

  T*& _data;

private:
  _CORBA_PseudoObj_Out();  // Not implemented
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Pseudo_Unbounded_Sequence ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class ElemT>
class _CORBA_Pseudo_Unbounded_Sequence {
public:
  typedef _CORBA_Pseudo_Unbounded_Sequence<T,ElemT> T_seq;

  inline _CORBA_Pseudo_Unbounded_Sequence()
    : pd_max(0), pd_len(0), pd_rel(1), pd_buf(0) { }
  inline _CORBA_Pseudo_Unbounded_Sequence(_CORBA_ULong max)
    : pd_max(max), pd_len(0), pd_rel(1)
  {
    if( !(pd_buf = new ElemT[max]) )
      _CORBA_new_operator_return_null();
  }

  inline _CORBA_Pseudo_Unbounded_Sequence(_CORBA_ULong max,
					  _CORBA_ULong length_,
					  T** value,
					  _CORBA_Boolean release_ = 0)
    : pd_max(max), pd_len(length_)
  {
    if( length_ > max )  _CORBA_bound_check_error();
    pd_buf = new ElemT[length_];
    if( release_ ) {
      for( _CORBA_ULong i = 0; i < length_; i++ )  pd_buf[i] = value[i];
      delete[] value;
    }
    else {
      for( _CORBA_ULong i = 0; i < length_; i++ )
	pd_buf[i] = value[i];
    }
  }

  inline _CORBA_Pseudo_Unbounded_Sequence(const T_seq& s)
    : pd_max(s.pd_max), pd_len(s.pd_len), pd_rel(1)
  {
    if( !(pd_buf = new ElemT[s.pd_max]) )
      _CORBA_new_operator_return_null();
    for( _CORBA_ULong i=0; i < s.pd_len; i++ ) {
      pd_buf[i] = s.pd_buf[i];
    }
  }

  inline ~_CORBA_Pseudo_Unbounded_Sequence() {
    if( pd_rel && pd_buf )  delete[] pd_buf;
  }

  inline T_seq& operator= (const T_seq& s) {
    if( &s == this )  return *this;
    if( pd_max < s.pd_max ) {
      ElemT* newbuf = new ElemT[s.pd_max];
      if( !newbuf )  _CORBA_new_operator_return_null();
      pd_max = s.pd_max;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = s.pd_len;
    for( unsigned long i=0; i < pd_len; i++ )  pd_buf[i] = s.pd_buf[i];
    return *this;
  }

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong length_) {
    if (length_ > pd_max) {
      ElemT* newbuf = new ElemT[length_];
      if( !newbuf )  _CORBA_new_operator_return_null();
      for( unsigned long i = 0; i < pd_len; i++ )
	newbuf[i] = pd_buf[i];
      pd_max = length_;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = length_;
  }
  inline ElemT& operator[] (_CORBA_ULong index_) {
    if( index_ >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index_];
  }
  inline const ElemT& operator[] (_CORBA_ULong index_) const {
    if( index_ >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index_];
  }
  static inline T** allocbuf(_CORBA_ULong nelems) { 
    T** v = new T*[nelems];
    for (_CORBA_ULong i=0; i < nelems; i++) v[i] = T::_nil();
    return v;
  }
  static inline void freebuf(T** b) { if( b ) delete[] b; }

  inline ElemT* NP_data() { return pd_buf; }

protected:
  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  ElemT*          pd_buf;
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_PseudoValue_Sequence      ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_PseudoValue_Sequence {
public:
  typedef _CORBA_PseudoValue_Sequence<T> T_seq;

  inline _CORBA_PseudoValue_Sequence()
    : pd_max(0), pd_len(0), pd_rel(1), pd_buf(0) { }
  inline _CORBA_PseudoValue_Sequence(_CORBA_ULong max)
    : pd_max(max), pd_len(0), pd_rel(1)
  {
    if( !(pd_buf = allocbuf(max)) )
      _CORBA_new_operator_return_null();
  }

  inline _CORBA_PseudoValue_Sequence(_CORBA_ULong max,
				     _CORBA_ULong length_,
				     T* value, _CORBA_Boolean release_ = 0)
    : pd_max(max), pd_len(length_), pd_rel(release_), pd_buf(value)
  {
    if( length_ > max )  _CORBA_bound_check_error();
  }

  inline _CORBA_PseudoValue_Sequence(const T_seq& s)
    : pd_max(s.pd_max), pd_len(s.pd_len), pd_rel(1)
  {
    if( !(pd_buf = allocbuf(s.pd_max)) )
      _CORBA_new_operator_return_null();
    for( _CORBA_ULong i=0; i < s.pd_len; i++ ) {
      pd_buf[i] = s.pd_buf[i];
    }
  }

  inline ~_CORBA_PseudoValue_Sequence() {
    if( pd_rel && pd_buf )  delete[] pd_buf;
  }

  inline T_seq& operator= (const T_seq& s) {
    if( &s == this )  return *this;
    if( pd_max < s.pd_max ) {
      T* newbuf = allocbuf(s.pd_max);
      if( !newbuf )  _CORBA_new_operator_return_null();
      pd_max = s.pd_max;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = s.pd_len;
    for( unsigned long i=0; i < pd_len; i++ )  pd_buf[i] = s.pd_buf[i];
    return *this;
  }

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong length_) {
    if (length_ > pd_max) {
      T* newbuf = allocbuf(length_);
      if( !newbuf )  _CORBA_new_operator_return_null();
      for( unsigned long i = 0; i < pd_len; i++ )
	newbuf[i] = pd_buf[i];
      pd_max = length_;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = length_;
  }
  inline T& operator[] (_CORBA_ULong index_) {
    if( index_ >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index_];
  }
  inline const T& operator[] (_CORBA_ULong index_) const {
    if( index_ >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index_];
  }

  inline _CORBA_Boolean release() const { return pd_rel; }
  
  inline T* get_buffer(_CORBA_Boolean orphan = 0) {
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel)
	return 0;
      else {
	T* tmp = pd_buf;
	pd_buf = 0;
	pd_max = 0;
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline const T* get_buffer() const { 
    return pd_buf;
  }

  static inline T* allocbuf(_CORBA_ULong nelems) { return new T[nelems]; }
  static inline void freebuf(T* b) { if( b )  delete[] b; }

  inline T* NP_data() { return pd_buf; }

protected:
  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  T*              pd_buf;
};

//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_ObjRef_Var /////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_Member;

template <class T,class T_Helper>
class _CORBA_ObjRef_Element;

template <class T,class T_Helper>
class _CORBA_ObjRef_INOUT_arg;

template <class T,class T_Helper>
class _CORBA_ObjRef_OUT_arg;

template <class T,class T_Helper>
class _CORBA_ObjRef_tcDesc_arg;

template <class T, class T_Helper>
class _CORBA_ObjRef_Var : public _CORBA_ObjRef_Var_base {
public:
  typedef T* ptr_t;
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Member<T,T_Helper>  T_member;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Var<T,T_Helper>     T_var;

  inline _CORBA_ObjRef_Var() : pd_objref(T_Helper::_nil()) {}
  inline _CORBA_ObjRef_Var(T_ptr p) : pd_objref(p) {}
  inline _CORBA_ObjRef_Var(const T_var& p) : pd_objref(p.pd_objref) {
    T_Helper::duplicate(p.pd_objref);
  }

  inline _CORBA_ObjRef_Var(const T_member& m);
  inline _CORBA_ObjRef_Var(const T_element& m);

  inline ~_CORBA_ObjRef_Var() {
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
    T_Helper::release(pd_objref);
#else
    CORBA::release(pd_objref);
#endif
  }

  inline T_var& operator= (T_ptr p) {
    T_Helper::release(pd_objref);
    pd_objref = p;
    return *this;
  }

  inline T_var& operator= (const T_var& p) {
    if( &p != this ) {
      T_Helper::duplicate(p.pd_objref);
      T_Helper::release(pd_objref);
      pd_objref = p.pd_objref;
    }
    return *this;
  }

  inline T_var& operator= (const T_member& m);
  inline T_var& operator= (const T_element& m);
  inline T_ptr operator->() const { return pd_objref; }
  inline operator T_ptr () const  { return pd_objref; }

  inline T_ptr  in() const { return pd_objref; }
  inline T_ptr& inout()    { return pd_objref; }
  inline T_ptr& out() {
    T_Helper::release(pd_objref);
    pd_objref = T_Helper::_nil();
    return pd_objref;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_objref;
    pd_objref = T_Helper::_nil();
    return tmp;
  }

  friend class _CORBA_ObjRef_INOUT_arg<T,T_Helper>;
  friend class _CORBA_ObjRef_OUT_arg<T,T_Helper>;
  friend class _CORBA_ObjRef_tcDesc_arg<T,T_Helper>;

private:
  T_ptr pd_objref;

  // Not implemented
  _CORBA_ObjRef_Var(const _CORBA_ObjRef_Var_base&);
  T_var& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_Member ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_Member {
public:
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Member<T,T_Helper>  T_member;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Var<T,T_Helper>     T_var;

  inline _CORBA_ObjRef_Member() { 
    _ptr = T_Helper::_nil();
  }

  inline _CORBA_ObjRef_Member(const T_member& p) {
    T_Helper::duplicate(p._ptr);
    _ptr = p._ptr;
  }

inline ~_CORBA_ObjRef_Member() {
#if !defined(__DECCXX) || (__DECCXX_VER >= 50500000)
    T_Helper::release(_ptr);
#else
    CORBA::release(_ptr);
#endif
  }

  inline T_member& operator= (T_ptr p) {
    T_Helper::release(_ptr);
    _ptr = p;
    return *this;
  }

  inline T_member& operator= (const T_member& p) {
    if( &p != this ) {
      T_Helper::release(_ptr);
      T_Helper::duplicate(p._ptr);
      _ptr = p._ptr;
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    T_Helper::release(_ptr);
    T_Helper::duplicate(p);
    _ptr = (T_ptr) p;
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline T_ptr operator->() const { return _ptr; }

  inline void operator>>= (cdrStream& s) const {
    T_Helper::marshalObjRef(_ptr,s);
  }

  inline void operator<<= (cdrStream& s) {
    T_ptr _result = T_Helper::unmarshalObjRef(s);
    T_Helper::release(_ptr);
    _ptr = _result;
  }

  // The following conversion operators are needed to support the
  // implicit conversion from this type to its T* data member.
  // They are used when this type is used as the rvalue of an expression.
  inline operator T_ptr () const { return _ptr; }

  inline T_ptr  in() const { return _ptr; }
  inline T_ptr& inout()    { return _ptr; }
  inline T_ptr& out() {
    T_Helper::release(_ptr);
    _ptr = T_Helper::_nil();
    return _ptr;
  }
  inline T_ptr _retn() {
    T_ptr tmp;
    tmp = _ptr;
    _ptr = T_Helper::_nil();
    return tmp;
  }

  T_ptr          _ptr;

private:
  // Not implemented
  _CORBA_ObjRef_Member(const _CORBA_ObjRef_Var_base&);
  T_member& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_Element ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_Element {
public:
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;

  inline _CORBA_ObjRef_Element(T_ptr& p, _CORBA_Boolean rel) 
    : pd_rel(rel), pd_data(p) {}

  inline _CORBA_ObjRef_Element(const T_element& p) 
    : pd_rel(p.pd_rel), pd_data(p.pd_data) {}

  inline ~_CORBA_ObjRef_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T_ptr p) {
    if( pd_rel )  T_Helper::release(pd_data);
    pd_data = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if( pd_rel ) {
      T_Helper::release(pd_data);
      T_Helper::duplicate(p.pd_data);
      pd_data = p.pd_data;
    }
    else
      pd_data = p.pd_data;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if( pd_rel ) {
      T_Helper::release(pd_data);
      T_Helper::duplicate(p);
    }
    pd_data = (T_ptr) p;
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if( pd_rel ) {
      T_Helper::release(pd_data);
      T_Helper::duplicate(p);
    }
    pd_data = (T_ptr) p;
    return *this;
  }

  inline T_ptr operator->() const { return pd_data; }

  // The following conversion operators are needed to support the
  // implicit conversion from this type to its T* data member.
  // They are used when this type is used as the rvalue of an expression.
  inline operator T_ptr () const { return pd_data; }


  inline T_ptr in() const { return pd_data; }
  inline T_ptr& inout()         { return pd_data; }
  inline T_ptr& out() {
    if (pd_rel) {
      T_Helper::release(pd_data);
    }
    pd_data = T_Helper::_nil();
    return pd_data;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_data;
    if (!pd_rel)
      T_Helper::duplicate(pd_data);
    pd_data = T_Helper::_nil();
    return tmp;
  }

  inline T_ptr& _NP_ref() const {return pd_data;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  T_ptr&         pd_data;

private:
  // Not implemented
  _CORBA_ObjRef_Element(const _CORBA_ObjRef_Var_base&);
  T_element& operator= (const _CORBA_ObjRef_Var_base&);
};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_ObjRef_var       //////////////////////
/////////////////////// _CORBA_ObjRef_member      ////////////////////
/////////////////////// operator= and copy ctors  ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper>::
_CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>& m)
{
  T_Helper::duplicate(m._ptr);
  pd_objref = m._ptr;
}

template <class T, class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper>::
_CORBA_ObjRef_Var(const _CORBA_ObjRef_Element<T,T_Helper>& m)
{
  T_Helper::duplicate(m);
  pd_objref = (T*)m;
}


template <class T, class T_Helper>
inline _CORBA_ObjRef_Var<T,T_Helper>&
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Member<T,T_Helper>& p)
{
  T_Helper::release(pd_objref);
  T_Helper::duplicate(p._ptr);
  pd_objref = p._ptr;
  return *this;
}

template <class T, class T_Helper>
inline _CORBA_ObjRef_Var<T,T_Helper>&
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Element<T,T_Helper>& p)
{
  T_Helper::release(pd_objref);
  T_Helper::duplicate(p);
  pd_objref = (T*)p;
  return *this;
}

template <class T, class T_Helper>
inline _CORBA_ObjRef_Member<T,T_Helper>&
_CORBA_ObjRef_Member<T,T_Helper>::operator= (const _CORBA_ObjRef_Element<T,T_Helper>& p)
{
  T_Helper::release(_ptr);
  T_Helper::duplicate(p);
  _ptr = (T*)p;
  return *this;
}

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_ObjRef_INOUT_arg //////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_INOUT_arg {
public:
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;
  inline _CORBA_ObjRef_INOUT_arg(T*& p) : _data(p) {}
  inline _CORBA_ObjRef_INOUT_arg(T_var& p) : _data(p.pd_objref) {}
  inline _CORBA_ObjRef_INOUT_arg(T_member& p) : _data(p._ptr) {}
  inline _CORBA_ObjRef_INOUT_arg(T_element& p) : _data(p._NP_ref()) {
    // If the T_element has pd_rel == 0,
    // the ObjRef is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the ObjRef and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p.NP_release() )
      T_Helper::duplicate(p._NP_ref());
  }
  inline ~_CORBA_ObjRef_INOUT_arg() {}

  inline operator T*&()  { return _data; }

  T*& _data;

private:
  _CORBA_ObjRef_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_OUT_arg ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_OUT_arg {
public:
  typedef _CORBA_ObjRef_OUT_arg<T,T_Helper> T_out;
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;

  inline _CORBA_ObjRef_OUT_arg(T*& p) : _data(p) { _data = T_Helper::_nil(); }
  inline _CORBA_ObjRef_OUT_arg(T_var& p) : _data(p.pd_objref) {
    p = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_OUT_arg(T_element& p) : _data(p._NP_ref()) {
    p = T_Helper::_nil();
  }

  inline _CORBA_ObjRef_OUT_arg(const T_out& p) : _data(p._data) {}
  inline ~_CORBA_ObjRef_OUT_arg() {}

  inline T_out& operator=(const T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }

  inline operator T*&()  { return _data; }
  inline T*& ptr()       { return _data; }
  inline T* operator->() const { return _data; }

  T*& _data;
private:
  _CORBA_ObjRef_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
  // CORBA 2.3 p23-26 says that T_var assignment should be disallowed.

};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_ObjRef_tcDesc_arg /////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_tcDesc_arg {
public:
  // The sole purpose of this class is for passing as an argument
  // to the tc build descriptor function of an interface.
  // It provides the necessary conversions from T_var, T_member,
  // T_element so that only one tc build description function is needed.
  // The alternative is to have 3 overloaded build description functions
  // which makes the stub code even more bloated.
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Element<T,T_Helper> T_element;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;
  inline _CORBA_ObjRef_tcDesc_arg(T_ptr& p, _CORBA_Boolean rel) 
    : _data(p), _rel(rel) {}
  inline _CORBA_ObjRef_tcDesc_arg(T_var& p) : _data(p.pd_objref), _rel(1) {}
  inline _CORBA_ObjRef_tcDesc_arg(T_member& p) : _data(p._ptr), _rel(1) {}
  inline _CORBA_ObjRef_tcDesc_arg(T_element p) 
    : _data(p._NP_ref()), _rel(p._NP_release()) {}
  inline ~_CORBA_ObjRef_tcDesc_arg() {}

  T*& _data;
  _CORBA_Boolean _rel;

private:
  _CORBA_ObjRef_tcDesc_arg();
};


//////////////////////////////////////////////////////////////////////
////////////////////// _CORBA_ConstrType_Fix_Var /////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
class _CORBA_ConstrType_Fix_Var {
public:
  typedef _CORBA_ConstrType_Fix_Var<T> T_var;

  inline _CORBA_ConstrType_Fix_Var()  {}
  inline _CORBA_ConstrType_Fix_Var(T* p) { pd_data = *p; delete p; }
  inline _CORBA_ConstrType_Fix_Var(const T_var& p) {
    pd_data = p.pd_data;
  }
  inline ~_CORBA_ConstrType_Fix_Var()  {}
  inline T_var&operator= (T* p) {
    pd_data = *p;
    delete p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    pd_data = p.pd_data;
    return *this;
  }
  inline T_var& operator= (T p) {
    pd_data = p;
    return *this;
  }
  inline T* operator->() const { return (T*) &pd_data; }

  //#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
#if defined(__GNUG__)
  inline operator T& () const       { return (T&) pd_data; }
#else
  inline operator const T& () const { return pd_data; }
  inline operator T& ()             { return pd_data; }
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

  const T& in() const { return pd_data; }
  T& inout() { return pd_data; }
  T& out() { return pd_data; }
  T _retn() { return pd_data; }

protected:
  T pd_data;
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_ConstrType_Variable_Var ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_ConstrType_Variable_OUT_arg;

template <class T>
class _CORBA_ConstrType_Variable_Var {
public:
  typedef _CORBA_ConstrType_Variable_Var<T> T_var;

  inline _CORBA_ConstrType_Variable_Var() { pd_data = 0; }
  inline _CORBA_ConstrType_Variable_Var(T* p) { pd_data = p; }
  inline _CORBA_ConstrType_Variable_Var(const T_var& p) {
    if( !p.pd_data )  pd_data = 0;
    else {
      pd_data = new T;
      if( !pd_data )  _CORBA_new_operator_return_null();
      *pd_data = *p.pd_data;
    }
  }

  inline ~_CORBA_ConstrType_Variable_Var() {  if( pd_data )  delete pd_data; }

  inline T_var& operator= (T* p) {
    if( pd_data )  delete pd_data;
    pd_data = p;
    return *this;
  }

  inline T_var& operator= (const T_var& p) {
    if( &p == this )  return *this;
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

  inline T* operator->() const { return (T*)pd_data; }

  //#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
#if defined(__GNUG__)
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

  const T& in() const { return *pd_data; }
  T& inout() { return *pd_data; }
  T*& out() { if (pd_data) { delete pd_data; pd_data = 0; } return pd_data; }
  T* _retn() { T* tmp = pd_data; pd_data = 0; return tmp; }

  friend class _CORBA_ConstrType_Variable_OUT_arg<T, T_var>;

protected:
  T* pd_data;
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_ConstrType_Variable_OUT_arg /////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_ConstrType_Variable_OUT_arg {
public:
  typedef _CORBA_ConstrType_Variable_OUT_arg<T,T_var> T_out;
  inline _CORBA_ConstrType_Variable_OUT_arg(T*& p) : _data(p) { _data = 0; }
  inline _CORBA_ConstrType_Variable_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  inline _CORBA_ConstrType_Variable_OUT_arg(const T_out& p) : _data(p._data) {}
  inline T_out& operator=(const T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T* operator->() { return _data; }

  T*& _data;

private:
  _CORBA_ConstrType_Variable_OUT_arg();  // not implemented
  T_out& operator=(const T_var&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_Array_Variable_Var  ////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_Array_Variable_OUT_arg;

template <class T_Helper, class T>
class _CORBA_Array_Variable_Var {
public:
  typedef _CORBA_Array_Variable_Var<T_Helper,T> T_var;

  inline _CORBA_Array_Variable_Var () { pd_data = 0; }
  inline _CORBA_Array_Variable_Var (T* p) { pd_data = p; }
  inline _CORBA_Array_Variable_Var (const T_var& p) {
    if( !p.pd_data )  pd_data = 0;
    else {
      pd_data = T_Helper::dup(p.pd_data);
      if( !pd_data )  _CORBA_new_operator_return_null();
    }
  }

  inline ~_CORBA_Array_Variable_Var() { if( pd_data )  T_Helper::free(pd_data); }

  inline T_var& operator= (T* p) {
    if (pd_data) T_Helper::free(pd_data);
    pd_data = p;
    return *this;
  }

  inline T_var& operator= (const T_var& p) {
    if( &p == this )  return *this;
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

#if !defined(_MSC_VER)
  inline T& operator[] (_CORBA_ULong index_) {
    return *(pd_data + index_);
  }
  inline const T& operator[] (_CORBA_ULong index_) const {
    return *((const T*) (pd_data + index_));
  }
#endif

  inline operator T* () const { return pd_data; }
  // Define the const T* operator() causes conversion operator ambiguity with 
  // some compilers. Should be alright to leave this operator out. If not,
  // reinstate it and #ifdef it with the right compiler specific macro.
  //
  //  inline operator const T* () const { return (const T*) pd_data; }

  const T* in() const { return (const T*)pd_data; }
  T*       inout()    { return pd_data; }
  T*& out() {  // ??? Is this correct?
    if (pd_data) { 
      T_Helper::free(pd_data); 
      pd_data = 0; 
    } 
    return pd_data; 
  }
  T* _retn() { T* tmp = pd_data; pd_data = 0; return tmp; }

  friend class _CORBA_Array_Variable_OUT_arg<T, T_var>;

private:
  T* pd_data;
};


//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_Array_Fix_Var     ////////////////
//////////////////////////////////////////////////////////////////////

template <class T_Helper, class T>
class _CORBA_Array_Fix_Var {
public:
  typedef _CORBA_Array_Fix_Var<T_Helper,T> T_var;

  inline _CORBA_Array_Fix_Var () { pd_data = 0; }
  inline _CORBA_Array_Fix_Var (T* p) { pd_data = p; }
  inline _CORBA_Array_Fix_Var (const T_var& p) {
    if( !p.pd_data )  pd_data = 0;
    else {
      pd_data = T_Helper::dup(p.pd_data);
      if( !pd_data )  _CORBA_new_operator_return_null();
    }
  }

  inline ~_CORBA_Array_Fix_Var() { if( pd_data )  T_Helper::free(pd_data); }

  inline T_var& operator= (T* p) {
    if (pd_data) T_Helper::free(pd_data);
    pd_data = p;
    return *this;
  }

  inline T_var& operator= (const T_var& p) {
    if( &p == this )  return *this;
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

#if !defined(_MSC_VER)
  inline T& operator[] (_CORBA_ULong index_) {
    return *(pd_data + index_);
  }
  inline const T& operator[] (_CORBA_ULong index_) const {
    return *((const T*) (pd_data + index_));
  }
#endif

  inline operator T* () const { return pd_data; }
  // Define the const T* operator() causes conversion operator ambiguity with 
  // some compilers. Should be alright to leave this operator out. If not,
  // reinstate it and #ifdef it with the right compiler specific macro.
  //
  //  inline operator const T* () const { return (const T*) pd_data; }

  const T* in() const { return (const T*)pd_data; }
  T*       inout()    { return pd_data; }
  T* out() { return pd_data; }
  T* _retn() { T* tmp = pd_data; pd_data = 0; return tmp; }

private:
  T* pd_data;
};

//////////////////////////////////////////////////////////////////////
//////////////////// _CORBA_Array_Variable_OUT_arg ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_Array_Variable_OUT_arg {
public:
  typedef _CORBA_Array_Variable_OUT_arg<T,T_var> T_out;
  inline _CORBA_Array_Variable_OUT_arg(T*& p) : _data(p) { _data = 0; }
  inline _CORBA_Array_Variable_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  inline _CORBA_Array_Variable_OUT_arg(const T_out& p) : _data(p._data) {}
  inline T_out& operator=(const T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T& operator[] (_CORBA_ULong index_) {
    return _data[index_];
  }

  T*& _data;
private:
  _CORBA_Array_Variable_OUT_arg();
  T_out& operator=(const T_var&);
};

//////////////////////////////////////////////////////////////////////
///////////////////////// _CORBA_Array_Variable_Forany ///////////////
//////////////////////////////////////////////////////////////////////

template <class T_Helper,class T>
class _CORBA_Array_Variable_Forany {
public:
  inline _CORBA_Array_Variable_Forany () { pd_data = 0; pd_nocopy = 0; }

  // If nocopy = 0 (the default) then we just copy the pointer
  // given to us. If nocopy is set, this implies that the insertion
  // into the Any should consume the data. Thus it is Any insertion
  // operator, not the destructor here which delete's the data.
  inline _CORBA_Array_Variable_Forany (T* p,_CORBA_Boolean nocopy = 0) {
    pd_data = p;  pd_nocopy = nocopy;
  }

  inline _CORBA_Array_Variable_Forany (const _CORBA_Array_Variable_Forany<T_Helper,T>& p)
  {
    pd_data = p.pd_data;  pd_nocopy = 0;
  }

  inline ~_CORBA_Array_Variable_Forany() {
    // Does not delete the storage of the array.
  }

  inline _CORBA_Array_Variable_Forany<T_Helper,T>& operator= (T* p) {
    pd_data = p;  pd_nocopy = 0;  return *this;
  }

  inline T& operator[] (_CORBA_ULong index_) { return *(pd_data + index_); }
  inline const T& operator[] (_CORBA_ULong index_) const {
    return *( (const T*) (pd_data + index_));
  }

  inline operator T* () const             { return pd_data; }
  // inline operator const T* () const       { return (const T*)pd_data; }
  // No need for const operator, and it upsets gcc.

  const T* in() const { return (const T*)pd_data; }
  T*       inout()    { return pd_data; }
  T*& out() {  // ??? Is this correct?
    if (pd_data) { 
      T_Helper::free(pd_data); 
      pd_data = 0; 
    } 
    return pd_data; 
  }
  T* _retn() { return pd_data; }

  inline T* NP_getSlice() const           { return pd_data;   }
  inline _CORBA_Boolean NP_nocopy() const { return pd_nocopy; }

private:
  T*             pd_data;
  _CORBA_Boolean pd_nocopy;
};


//////////////////////////////////////////////////////////////////////
///////////////////////// _CORBA_Array_Fix_Forany ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T_Helper,class T>
class _CORBA_Array_Fix_Forany {
public:
  inline _CORBA_Array_Fix_Forany () { pd_data = 0; pd_nocopy = 0; }

  // If nocopy = 0 (the default) then we just copy the pointer
  // given to us. If nocopy is set, this implies that the insertion
  // into the Any should consume the data. Thus it is Any insertion
  // operator, not the destructor here which delete's the data.
  inline _CORBA_Array_Fix_Forany (T* p,_CORBA_Boolean nocopy = 0) {
    pd_data = p;  pd_nocopy = nocopy;
  }

  inline _CORBA_Array_Fix_Forany (const _CORBA_Array_Fix_Forany<T_Helper,T>& p)
  {
    pd_data = p.pd_data;  pd_nocopy = 0;
  }

  inline ~_CORBA_Array_Fix_Forany() {
    // Does not delete the storage of the array.
  }

  inline _CORBA_Array_Fix_Forany<T_Helper,T>& operator= (T* p) {
    pd_data = p;  pd_nocopy = 0;  return *this;
  }

  inline T& operator[] (_CORBA_ULong index_) { return *(pd_data + index_); }
  inline const T& operator[] (_CORBA_ULong index_) const {
    return *( (const T*) (pd_data + index_));
  }

#if defined(__GNUG__) && __GNUG__ == 3 && __GNUC_MINOR__ >= 4
  // g++ thinks the operators with ULong arguments are ambiguous when
  // used with int literals. This sorts it out.
  inline T& operator[] (int index_) { return *(pd_data + index_); }
  inline const T& operator[] (int index_) const {
    return *( (const T*) (pd_data + index_));
  }
#endif

  inline operator T* () const             { return pd_data; }
  // inline operator const T* () const       { return (const T*)pd_data; }
  // No need for const operator, and it upsets gcc.

  const T* in() const { return (const T*)pd_data; }
  T*       inout()    { return pd_data; }
  T* out() { return pd_data; }
  T* _retn() { return pd_data; }

  inline T* NP_getSlice() const           { return pd_data;   }
  inline _CORBA_Boolean NP_nocopy() const { return pd_nocopy; }

private:
  T*             pd_data;
  _CORBA_Boolean pd_nocopy;
};


#endif  // __TEMPLATEDECLS_H__
