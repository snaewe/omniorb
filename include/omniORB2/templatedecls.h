// -*- Mode: C++; -*-
//                            Package   : omniORB2
// templatedecls.h            Created on: 14/5/96
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

#ifndef __TEMPLATEDECLS_H__
#define __TEMPLATEDECLS_H__


//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_PseudoObj_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var> class _CORBA_PseudoObj_Member;
template <class T, class T_var> class _CORBA_PseudoObj_Out;

template <class T>
class _CORBA_PseudoObj_Var {
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

  inline const T* in() const { return pd_data; }
  inline T_ptr&      inout()    { return pd_data; }
  inline T_ptr& out();
  inline T_ptr _retn();

  friend class _CORBA_PseudoObj_Out<T, _CORBA_PseudoObj_Var<T> >;
  friend class _CORBA_PseudoObj_Member<T, _CORBA_PseudoObj_Var<T> >;

protected:
  T_ptr pd_data;
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
					  _CORBA_ULong length,
					  T** value, _CORBA_Boolean release = 0)
    : pd_max(max), pd_len(length)
  {
    if( length > max )  _CORBA_bound_check_error();
    pd_buf = new ElemT[length];
    if( release ) {
      for( _CORBA_ULong i = 0; i < length; i++ )  pd_buf[i] = value[i];
      delete[] value;
    }
    else {
      for( _CORBA_ULong i = 0; i < length; i++ )
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
  inline void length(_CORBA_ULong length) {
    if (length > pd_max) {
      ElemT* newbuf = new ElemT[length];
      if( !newbuf )  _CORBA_new_operator_return_null();
      for( unsigned long i = 0; i < pd_len; i++ )
	newbuf[i] = pd_buf[i];
      pd_max = length;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = length;
  }
  inline ElemT& operator[] (_CORBA_ULong index) {
    if( index >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index];
  }
  inline const ElemT& operator[] (_CORBA_ULong index) const {
    if( index >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index];
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
				     _CORBA_ULong length,
				     T* value, _CORBA_Boolean release = 0)
    : pd_max(max), pd_len(length), pd_rel(release), pd_buf(value)
  {
    if( length > max )  _CORBA_bound_check_error();
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
  inline void length(_CORBA_ULong length) {
    if (length > pd_max) {
      T* newbuf = allocbuf(length);
      if( !newbuf )  _CORBA_new_operator_return_null();
      for( unsigned long i = 0; i < pd_len; i++ )
	newbuf[i] = pd_buf[i];
      pd_max = length;
      if( pd_rel && pd_buf )  delete[] pd_buf;
      else                    pd_rel = 1;
      pd_buf = newbuf;
    }
    pd_len = length;
  }
  inline T& operator[] (_CORBA_ULong index) {
    if( index >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index];
  }
  inline const T& operator[] (_CORBA_ULong index) const {
    if( index >= pd_len )  _CORBA_bound_check_error();
    return pd_buf[index];
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

template <class T,class T_var,class T_member>
class _CORBA_ObjRef_INOUT_arg;

template <class T,class T_var,class T_member,class T_Helper>
class _CORBA_ObjRef_OUT_arg;

template <class T, class T_Helper>
class _CORBA_ObjRef_Var {
public:
  typedef T* ptr_t;
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;

  inline _CORBA_ObjRef_Var()        { pd_objref = T_Helper::_nil(); }
  inline _CORBA_ObjRef_Var(T_ptr p) { pd_objref = p;                }
  inline _CORBA_ObjRef_Var(const T_var& p);
  inline _CORBA_ObjRef_Var(const T_member& m);
  inline ~_CORBA_ObjRef_Var();

  inline T_var& operator= (T_ptr p);
  inline T_var& operator= (const T_var& p);
  inline T_var& operator= (const T_member& m);

  inline T_ptr operator->() const { return pd_objref; }
  inline operator T_ptr () const  { return pd_objref; }

  inline const T* in() const { return pd_objref; }
  inline T_ptr& inout()         { return pd_objref; }
  inline T_ptr& out();
  inline T_ptr _retn();

  friend class _CORBA_ObjRef_Member<T,T_Helper>;
  friend class _CORBA_ObjRef_INOUT_arg<T, _CORBA_ObjRef_Var<T,T_Helper>,
                                       _CORBA_ObjRef_Member<T,T_Helper> >;
  friend class _CORBA_ObjRef_OUT_arg<T, _CORBA_ObjRef_Var<T,T_Helper>,
                                     _CORBA_ObjRef_Member<T,T_Helper>,
                                     T_Helper>;

private:
  T_ptr pd_objref;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_Member ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_Helper>
class _CORBA_ObjRef_Member {
public:
  typedef T* T_ptr;
  typedef _CORBA_ObjRef_Member<T,T_Helper> T_member;
  typedef _CORBA_ObjRef_Var<T,T_Helper>    T_var;

  inline _CORBA_ObjRef_Member()        { _ptr = T_Helper::_nil(); }
  inline _CORBA_ObjRef_Member(T_ptr p) { _ptr = p;                }
  inline _CORBA_ObjRef_Member(const T_member& p);
  inline ~_CORBA_ObjRef_Member();

  inline T_member& operator= (T_ptr p);
  inline T_member& operator= (const T_member& p);
  inline T_member& operator= (const T_var& p);

  inline T_ptr operator->() const { return _ptr; }

  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream& s) const;
  inline void operator<<= (NetBufferedStream& s);
  inline void operator>>= (MemBufferedStream& s) const;
  inline void operator<<= (MemBufferedStream& s);

  // The following conversion operators are needed to support the
  // implicit conversion from this type to its T* data member.
  // They are used when this type is used as the rvalue of an expression.
  inline operator T_ptr () const { return _ptr; }

  T_ptr _ptr;
};

//////////////////////////////////////////////////////////////////////
/////////////////////// _CORBA_ObjRef_INOUT_arg //////////////////////
//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_ObjRef_OUT_arg ///////////////////////
//////////////////////////////////////////////////////////////////////

template <class T,class T_var,class T_member,class T_Helper>
class _CORBA_ObjRef_OUT_arg {
public:
  typedef _CORBA_ObjRef_OUT_arg<T,T_var,T_member,T_Helper> T_out;

  inline _CORBA_ObjRef_OUT_arg(T*& p) : _data(p) { _data = T_Helper::_nil(); }
  inline _CORBA_ObjRef_OUT_arg(T_var& p) : _data(p.pd_objref) {
    p = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_OUT_arg(T_out& p) : _data(p._data) {}
  inline T_out& operator=(T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline T_out& operator=(const T_var& p) { 
    _data = T_Helper::duplicate(p); return *this;
  }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T* operator->() { return _data; }

  T*& _data;
private:
  _CORBA_ObjRef_OUT_arg();
};

//////////////////////////////////////////////////////////////////////
////////////////////// _CORBA_ConstrType_Fix_Var /////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_ConstrType_Fix_OUT_arg;

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

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
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

  friend class _CORBA_ConstrType_Fix_OUT_arg<T, T_var>;

private:
  T pd_data;
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_ConstrType_Fix_OUT_arg      /////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_ConstrType_Fix_OUT_arg {
public:
  typedef _CORBA_ConstrType_Fix_OUT_arg<T,T_var> T_out;
  inline _CORBA_ConstrType_Fix_OUT_arg(T*& p) : _data(p) { _data = 0; }
  inline _CORBA_ConstrType_Fix_OUT_arg(T_var& p) : _data(&p.pd_data) {}
  inline _CORBA_ConstrType_Fix_OUT_arg(T_out& p) : _data(p._data) {}
  inline T_out& operator=(T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T*& operator->() { return _data; }

  T*& _data;

private:
  _CORBA_ConstrType_Fix_OUT_arg();  // not implemented
  T_out& operator=(const T_var&);
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
  inline _CORBA_ConstrType_Variable_Var(const T_var& p);
  inline ~_CORBA_ConstrType_Variable_Var() {  if( pd_data )  delete pd_data; }

  inline T_var& operator= (T* p);
  inline T_var& operator= (const T_var& p);
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

  const T& in() const { return *pd_data; }
  T& inout() { return *pd_data; }
  T*& out() { if (pd_data) { delete pd_data; pd_data = 0; } return pd_data; }
  T* _retn() { T* tmp = pd_data; pd_data = 0; return tmp; }

  friend class _CORBA_ConstrType_Variable_OUT_arg<T, T_var>;

private:
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
  inline _CORBA_ConstrType_Variable_OUT_arg(T_out& p) : _data(p._data) {}
  inline T_out& operator=(T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T*& operator->() { return _data; }

  T*& _data;

private:
  _CORBA_ConstrType_Variable_OUT_arg();  // not implemented
  T_out& operator=(const T_var&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////// _CORBA_Array_Var //////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_Array_OUT_arg;

template <class T_Helper, class T>
class _CORBA_Array_Var {
public:
  typedef _CORBA_Array_Var<T_Helper,T> T_var;

  inline _CORBA_Array_Var () { pd_data = 0; }
  inline _CORBA_Array_Var (T* p) { pd_data = p; }
  inline _CORBA_Array_Var (const T_var& p);
  inline ~_CORBA_Array_Var() { if( pd_data )  T_Helper::free(pd_data); }

  inline T_var& operator= (T* p);
  inline T_var& operator= (const T_var& p);
  inline T& operator[] (_CORBA_ULong index) {
    return *(pd_data + index);
  }
  inline const T& operator[] (_CORBA_ULong index) const {
    return *((const T*) (pd_data + index));
  }
  inline operator T* () const { return pd_data; }
  // Define the const T* operator() causes conversion operator ambiguity with 
  // some compilers. Should be alright to leave this operator out. If not,
  // reinstate it and #ifdef it with the right compiler specific macro.
  //
  //  inline operator const T* () const { return (const T*) pd_data; }

  const T* in() const { return pd_data; }
  T* inout() { return *pd_data; }
  T*& out()   {  // ??? Is this correct?
    if (pd_data) { 
      T_Helper::free(pd_data); 
      pd_data = 0; 
    } 
    return pd_data; 
  }
  T* _retn() { T* tmp = pd_data; pd_data = 0; return tmp; }

  friend class _CORBA_Array_OUT_arg<T, T_var>;

private:
  T* pd_data;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// _CORBA_Array_OUT_arg ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_var>
class _CORBA_Array_OUT_arg {
public:
  typedef _CORBA_Array_OUT_arg<T,T_var> T_out;
  inline _CORBA_Array_OUT_arg(T*& p) : _data(p) { _data = 0; }
  inline _CORBA_Array_OUT_arg(T_var& p) : _data(p.pd_data) {
    p = (T*)0;
  }
  inline _CORBA_Array_OUT_arg(T_out& p) : _data(p._data) {}
  inline T_out& operator=(T_out& p) { _data = p._data; return *this; }
  inline T_out& operator=(T* p) { _data = p; return *this; }
  inline operator T*&() { return _data; }
  inline T*& ptr() { return _data; }
  inline T& operator[] (_CORBA_ULong index) {
    return *(pd_data + index);
  }

  T*& _data;
private:
  _CORBA_Array_OUT_arg();
  T_out& operator=(const T_var&);
};

//////////////////////////////////////////////////////////////////////
///////////////////////// _CORBA_Array_Forany ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T_Helper,class T>
class _CORBA_Array_Forany {
public:
  inline _CORBA_Array_Forany () { pd_data = 0; pd_nocopy = 0; }

  // If nocopy = 0 (the default) then we just copy the pointer
  // given to us. If nocopy is set, this implies that the insertion
  // into the Any should consume the data. Thus it is Any insertion
  // operator, not the destructor here which delete's the data.
  inline _CORBA_Array_Forany (T* p,_CORBA_Boolean nocopy = 0) {
    pd_data = p;  pd_nocopy = nocopy;
  }

  inline _CORBA_Array_Forany (const _CORBA_Array_Forany<T_Helper,T>& p) {
    pd_data = p.pd_data;  pd_nocopy = 0;
  }

  inline ~_CORBA_Array_Forany() {
    // Does not delete the storage of the array.
  }

  inline _CORBA_Array_Forany<T_Helper,T>& operator= (T* p) {
    pd_data = p;  pd_nocopy = 0;  return *this;
  }

  inline T& operator[] (_CORBA_ULong index) { return *(pd_data + index); }
  inline const T& operator[] (_CORBA_ULong index) const {
    return *( (const T*) (pd_data + index));
  }

  inline operator T* () const             { return pd_data;           }
  inline operator const T* () const       { return (const T*)pd_data; }

  inline T* NP_getSlice() const           { return pd_data;   }
  inline _CORBA_Boolean NP_nocopy() const { return pd_nocopy; }

private:
  T*             pd_data;
  _CORBA_Boolean pd_nocopy;
};

#endif  // __TEMPLATEDECLS_H__
