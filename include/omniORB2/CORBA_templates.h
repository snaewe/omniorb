// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_templates.h          Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:


/*
 $Log$
 Revision 1.2  1997/01/13 15:08:05  sll
 All the member functions in the templates are now inline.
 Added template _CORBA_Array_Var and _CORBA_Array_Forany.

 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

 */

#ifndef __CORBA_TEMPLATES_H__
#define __CORBA_TEMPLATES_H__

class NetBufferedStream;
class MemBufferedStream;

template <class T>
class _CORBA_Sequence {
public:
  inline _CORBA_Sequence() : pd_max(0), pd_len(0), pd_rel(1), pd_buf(0) { }

  inline _CORBA_Sequence(_CORBA_ULong max) : pd_max(max), pd_len(0), pd_rel(1)
  {
    if (!(pd_buf = new T[(int)max])) {
       _CORBA_new_operator_return_null();
      // never reach here
    }
    return;
  }

  inline _CORBA_Sequence(_CORBA_ULong max,
			 _CORBA_ULong length,
			 T           *value,
			 _CORBA_Boolean release = 0) 
      : pd_max(max), 
	pd_len(length), 
	pd_rel(release),
	pd_buf(value)
  {
    if (length > max) {
      _CORBA_bound_check_error();
      // never reach here
    }
    return;
  }

  inline _CORBA_Sequence(const _CORBA_Sequence<T>& s)
       : pd_max(s.pd_max), 
	 pd_len(s.pd_len),
	 pd_rel(1)
  {
    if (!(pd_buf = new T[(int)(s.pd_len)])) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (_CORBA_ULong i=0; i < s.pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
    }
  }

  inline ~_CORBA_Sequence() {
    if (pd_rel && pd_buf) delete [] pd_buf;
    pd_buf = 0;
    return;
  }

  inline _CORBA_Sequence<T> &operator= (const _CORBA_Sequence<T> &s) {
    if (pd_max < s.pd_max)
      {
	T *newbuf = new T[(int)(s.pd_max)];
	if (!newbuf) {
	  _CORBA_new_operator_return_null();
	  // never reach here
	}
	pd_max = s.pd_max;
	if (pd_rel && pd_buf) {
	  delete [] pd_buf;
	}
	else {
	  pd_rel = 1;
	}
	pd_buf = newbuf;
      }
    pd_len = s.pd_len;
    for (unsigned long i=0; i < pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
    }
    return *this;
  }

  inline _CORBA_ULong maximum() const { return pd_max; }

  inline _CORBA_ULong length() const { return pd_len; }

  inline void length(_CORBA_ULong length) {
    if (length > pd_max)
      {
	T *newbuf = new T[(int)length];
	if (!newbuf) {
	  _CORBA_new_operator_return_null();
	  // never reach here
	}
	for (unsigned long i=0; i < pd_len; i++) {
	  newbuf[i] = pd_buf[i];
	}
	pd_max = length;
	if (pd_rel && pd_buf) {
	  delete [] pd_buf;
	}
	else {
	  pd_rel = 1;
	}
	pd_buf = newbuf;
      }
    pd_len = length;
    return;
  }

  inline T &operator[] (_CORBA_ULong index) {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return pd_buf[index];
  }

  inline const T &operator[] (_CORBA_ULong index) const {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return pd_buf[index];
  }

  static inline T *allocbuf(_CORBA_ULong nelems) { return new T[nelems]; }

  static inline void freebuf(T * b) { if (b) delete [] b; return; }

  // omniORB2 extensions
  inline T *NP_data() const { return pd_buf; }

  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);

  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);

private:
  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  T              *pd_buf;
};

template <class T>
class _CORBA_Unbounded_Sequence : public _CORBA_Sequence<T> {
public:
  inline _CORBA_Unbounded_Sequence() {}

  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max) : _CORBA_Sequence<T>(max) {}

  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max,
				   _CORBA_ULong length,
				   T           *value,
				   _CORBA_Boolean release = 0)
     : _CORBA_Sequence<T>(max,length,value,release) {}

  inline _CORBA_Unbounded_Sequence(const _CORBA_Unbounded_Sequence<T>& s) 
     : _CORBA_Sequence<T>(s) {}

  inline ~_CORBA_Unbounded_Sequence() {}

  inline _CORBA_Unbounded_Sequence<T> &operator= (const _CORBA_Unbounded_Sequence<T> &s) {
    _CORBA_Sequence<T>::operator= (s);
    return *this;
  }

  inline size_t NP_alignedSize(size_t initialoffset) const {
    size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
    for (unsigned long i=0; i < length(); i++) {
      alignedsize = NP_data()[i].NP_alignedSize(alignedsize);
    }
    return alignedsize;
  }

  inline void operator>>= (NetBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (NetBufferedStream &s) {
    _CORBA_Sequence<T>::operator<<=(s);
  }

  inline void operator>>= (MemBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (MemBufferedStream &s) {
    _CORBA_Sequence<T>::operator<<=(s);
  }
};


template <class T,int max>
class _CORBA_Bounded_Sequence : public _CORBA_Sequence<T> {
public:
  inline _CORBA_Bounded_Sequence() : _CORBA_Sequence<T>(max) {}

  inline _CORBA_Bounded_Sequence(_CORBA_ULong length,
				 T           *value,
				 _CORBA_Boolean release = 0)
            : _CORBA_Sequence<T>(max,length,value,release) {}

  inline _CORBA_Bounded_Sequence(const _CORBA_Bounded_Sequence<T,max>& s)
            : _CORBA_Sequence<T>(s) {}

  inline ~_CORBA_Bounded_Sequence() {}

  inline _CORBA_Bounded_Sequence<T,max> &operator= (const _CORBA_Bounded_Sequence<T,max> &s) {
    _CORBA_Sequence<T>::operator= (s);
    return *this;
  }

  inline _CORBA_ULong length() const { return _CORBA_Sequence<T>::length(); }
  inline void length(_CORBA_ULong len) {
    if (len > max) {
      _CORBA_bound_check_error();
      // never reach here
    }
    _CORBA_Sequence<T>::length(len);
    return;
  }

  inline size_t NP_alignedSize(size_t initialoffset) const {
    size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
    for (unsigned long i=0; i < length(); i++) {
      alignedsize = NP_data()[i].NP_alignedSize(alignedsize);
    }
    return alignedsize;
  }

  inline void operator>>= (NetBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (NetBufferedStream &s);

  inline void operator>>= (MemBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (MemBufferedStream &s);
};

template <class T,int elmSize,int elmAlignment>
class _CORBA_Unbounded_Sequence_w_FixSizeElement 
   : public _CORBA_Sequence<T> 
{
public:
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement() {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max)
    : _CORBA_Sequence<T>(max) {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max,
						    _CORBA_ULong length,
						    T           *value,
						    _CORBA_Boolean release = 0)
    : _CORBA_Sequence<T>(max,length,value,release) {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement (const 
       _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>& s)
    : _CORBA_Sequence<T>(s) {}

  inline ~_CORBA_Unbounded_Sequence_w_FixSizeElement() {}

  inline _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment> &
      operator= 
        (const 
	  _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment> &
            s) 
  {
    _CORBA_Sequence<T>::operator= (s);
    return *this;
  }

  inline size_t NP_alignedSize(size_t initialoffset) const {
    size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
    alignedsize = ((alignedsize+(elmAlignment-1)) & ~(elmAlignment-1));
    alignedsize += length() * elmSize;
    return alignedsize;
  }

  inline void operator>>= (NetBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (NetBufferedStream &s) {
    _CORBA_Sequence<T>::operator<<=(s);
  }

  inline void operator>>= (MemBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (MemBufferedStream &s) {
    _CORBA_Sequence<T>::operator<<=(s);
  }
};


template <class T,int max,int elmSize, int elmAlignment>
class _CORBA_Bounded_Sequence_w_FixSizeElement 
  : public _CORBA_Sequence<T> 
{
public:
  inline _CORBA_Bounded_Sequence_w_FixSizeElement() {}

  inline _CORBA_Bounded_Sequence_w_FixSizeElement(_CORBA_ULong length,
					   T           *value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Sequence<T>(max,length,value,release) {}

  inline _CORBA_Bounded_Sequence_w_FixSizeElement(const 
      _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>& s) 
    : _CORBA_Sequence<T>(s) {}

  inline ~_CORBA_Bounded_Sequence_w_FixSizeElement() {}

  inline _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment> &
      operator= 
        (const 
          _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>&
            s) 
  {
    _CORBA_Sequence<T>::operator=(s);
    return *this;
  }

  inline _CORBA_ULong length() const { return _CORBA_Sequence<T>::length(); }
  inline void length(_CORBA_ULong len) {
    if (len > max) {
      _CORBA_bound_check_error();
      // never reach here
    }
    _CORBA_Sequence<T>::length(len);
    return;
  }

  // omniORB2 extensions
  inline size_t NP_alignedSize(size_t initialoffset) const {
    size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
    alignedsize = ((alignedsize+(elmAlignment-1)) & ~(elmAlignment-1));
    alignedsize += length() * elmSize;
    return alignedsize;
  }

  inline void operator>>= (NetBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (NetBufferedStream &s);

  inline void operator>>= (MemBufferedStream &s) const {
    _CORBA_Sequence<T>::operator>>=(s);
  }

  inline void operator<<= (MemBufferedStream &s);
};

typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Boolean,1,1> _CORBA_Unbounded_Sequence_Boolean;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Char,1,1>  _CORBA_Unbounded_Sequence_Char;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1> _CORBA_Unbounded_Sequence_Octet;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Short,2,2> _CORBA_Unbounded_Sequence_Short;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_UShort,2,2> _CORBA_Unbounded_Sequence_UShort;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Long,4,4>   _CORBA_Unbounded_Sequence_Long;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_ULong,4,4>  _CORBA_Unbounded_Sequence_ULong;
#ifndef NO_FLOAT
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Float,4,4>  _CORBA_Unbounded_Sequence_Float;
typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Double,8,8> _CORBA_Unbounded_Sequence_Double;
#endif

template <class T,class T_Helper>
class _CORBA_ObjRef_Member;

template <class T,class T_Helper>
class _CORBA_ObjRef_Var {
public:

  typedef T* ptr_t;

  inline _CORBA_ObjRef_Var() {
    pd_objref = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_Var(T *p) {
    pd_objref = p;
  }
  inline _CORBA_ObjRef_Var(const _CORBA_ObjRef_Var<T,T_Helper> &p) {
    if (!CORBA::is_nil(p.pd_objref)) {
      CORBA::Object::_duplicate(p.pd_objref);
      pd_objref = p.pd_objref;
    }
    else
      pd_objref = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>&);
  ~_CORBA_ObjRef_Var() {
    if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
  }
  inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (T * p) {
    if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
    pd_objref = p;
    return *this;
  }
  inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p) {
    if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
    if (!CORBA::is_nil(p.pd_objref)) {
      CORBA::Object::_duplicate(p.pd_objref);
      pd_objref = p.pd_objref;
    }
    else
      pd_objref = T_Helper::_nil();
    return *this;
  }
 inline _CORBA_ObjRef_Var<T,T_Helper> &operator= (const _CORBA_ObjRef_Member<T,T_Helper>&);

  inline T* operator->() { return pd_objref; }
  inline operator ptr_t& () { return pd_objref; }
#if !defined(__GNUG__) || __GNUG__ != 2 || __GNUC_MINOR__ > 7
  inline operator const ptr_t () const { return pd_objref; }
#endif

  friend _CORBA_ObjRef_Member<T,T_Helper>;

private:
  T* pd_objref;
};

template <class T,class T_Helper>
class _CORBA_ObjRef_Member {
public:
  typedef T* ptr_t;

  inline _CORBA_ObjRef_Member() {
    _ptr = T_Helper::_nil();
  }
  inline _CORBA_ObjRef_Member(T *p) {
    _ptr = p;
  }
  inline _CORBA_ObjRef_Member(const _CORBA_ObjRef_Member<T,T_Helper> &p) {
    if (!CORBA::is_nil(p._ptr)) {
      CORBA::Object::_duplicate(p._ptr);
      _ptr = p._ptr;
    }
    else
      _ptr = T_Helper::_nil();
  }
  inline ~_CORBA_ObjRef_Member() {
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
  }
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (T * p) {
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
    _ptr = p;
    return *this;
  }
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Member<T,T_Helper> &p) {
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
    if (!CORBA::is_nil(p._ptr)) {
      CORBA::Object::_duplicate(p._ptr);
      _ptr = p._ptr;
    }
    else
      _ptr = T_Helper::_nil();
    return *this;
  }
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p) {
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
    if (!CORBA::is_nil(p.pd_objref)) {
      CORBA::Object::_duplicate(p.pd_objref);
      _ptr = p.pd_objref;
    }
    else
      _ptr = T_Helper::_nil();
    return *this;
  }
#if !defined(__GNUG__) || __GNUG__ != 2 || __GNUC_MINOR__ > 7
  inline operator const ptr_t () const { return _ptr; }
#endif
  inline operator ptr_t &() { return _ptr; }

  T *_ptr;

  inline size_t NP_alignedSize(size_t initialoffset) const {
    return T_Helper::NP_alignedSize(_ptr,initialoffset);
  }
  inline void operator>>= (NetBufferedStream &s) const {
    T_Helper::marshalObjRef(_ptr,s);
  }
  inline void operator<<= (NetBufferedStream &s) {
    T* _result = T_Helper::unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }
  inline void operator>>= (MemBufferedStream &s) const {
    T_Helper::marshalObjRef(_ptr,s);
  }
  inline void operator<<= (MemBufferedStream &s) {
    T* _result = T_Helper::unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }
};


template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper> &
_CORBA_ObjRef_Var<T,T_Helper>::operator= (const _CORBA_ObjRef_Member<T,T_Helper>&p)
{
  if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
  if (!CORBA::is_nil(p._ptr)) {
      CORBA::Object::_duplicate(p._ptr);
      pd_objref = p._ptr;
    }
    else
      pd_objref = T_Helper::_nil();
    return *this;
}

template <class T,class T_Helper>
inline
_CORBA_ObjRef_Var<T,T_Helper>::_CORBA_ObjRef_Var(const _CORBA_ObjRef_Member<T,T_Helper>&p)
{
  if (!CORBA::is_nil(p._ptr)) {
    CORBA::Object::_duplicate(p._ptr);
    pd_objref = p._ptr;
  }
  else
    pd_objref = T_Helper::_nil();
}

template <class T>
class _CORBA_ConstrType_Fix_Var {
public:
  typedef T* ptr_t;

  inline _CORBA_ConstrType_Fix_Var() {
    pd_data = 0;
  }
  inline _CORBA_ConstrType_Fix_Var(T* p) {
    pd_data = p;
  }
  inline _CORBA_ConstrType_Fix_Var(const _CORBA_ConstrType_Fix_Var<T> &p) {
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
  inline ~_CORBA_ConstrType_Fix_Var() {
    if (pd_data) delete pd_data;
  }
  inline _CORBA_ConstrType_Fix_Var<T> &operator= (T* p) {
    if (pd_data) delete pd_data;
    pd_data = p;
    return *this;
  }
  inline _CORBA_ConstrType_Fix_Var<T> &operator= (const _CORBA_ConstrType_Fix_Var<T> &p) {
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

  inline T* operator->() {
    return pd_data;
    
  }

  inline operator T &() { return *pd_data; }
#if !defined(__GNUG__) || __GNUG__ != 2 || __GNUC_MINOR__ > 7
  inline operator const ptr_t () const { return pd_data; }
#endif
  inline operator ptr_t& () { return pd_data; }
  
private:
  T* pd_data;
};

template <class T>
class _CORBA_ConstrType_Variable_Var {
public:
  typedef T* ptr_t;

  inline _CORBA_ConstrType_Variable_Var() {
    pd_data = 0;
  }
  inline _CORBA_ConstrType_Variable_Var(T* p) {
    pd_data = p;
  }
  inline _CORBA_ConstrType_Variable_Var(const _CORBA_ConstrType_Variable_Var<T> &p) {
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
  inline ~_CORBA_ConstrType_Variable_Var() {
    if (pd_data) delete pd_data;
  }
  inline _CORBA_ConstrType_Variable_Var<T> &operator= (T* p) {
    if (pd_data) delete pd_data;
    pd_data = p;
    return *this;
  }
  inline _CORBA_ConstrType_Variable_Var<T> &operator= (const _CORBA_ConstrType_Variable_Var<T> &p) {
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

  inline T* operator->() {
    return pd_data;
    
  }

  inline operator T& () { return *pd_data; }
#if !defined(__GNUG__) || __GNUG__ != 2 || __GNUC_MINOR__ > 7
  inline operator const ptr_t () const { return pd_data; }
#endif
  inline operator ptr_t& () { return pd_data; }
  
private:
  T* pd_data;
};


template <class T_Helper,class T>
class _CORBA_Array_Var {
public:
  typedef T* ptr_t;
  inline _CORBA_Array_Var () {
    pd_data = 0;
  }
  inline _CORBA_Array_Var (T* p) {
    pd_data = p;
  }
  inline _CORBA_Array_Var (const _CORBA_Array_Var<T_Helper,T>& p) {
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
  inline ~_CORBA_Array_Var() {
    if (pd_data) T_Helper::free(pd_data);
  }
  inline _CORBA_Array_Var<T_Helper,T> &operator= (T* p) {
    if (pd_data) T_Helper::free(pd_data);
    pd_data = p;
    return *this;
  }
  inline _CORBA_Array_Var<T_Helper,T> &operator= (const _CORBA_Array_Var<T_Helper,T>& p) {
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
  inline T& operator[] (CORBA::ULong index) {
    return *(pd_data + index);
  }
  inline const T& operator[] (CORBA::ULong index) const {
    return *(pd_data + index);
  }
  inline operator ptr_t& () {
    return pd_data;
  }
private:
  T* pd_data;
};

template <class T_Helper,class T>
class _CORBA_Array_Forany {
public:
  typedef T* ptr_t;
  inline _CORBA_Array_Forany () {
    pd_data = 0;
  }
  inline _CORBA_Array_Forany (T* p,_CORBA_Boolean nocopy = 0) {
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
  inline _CORBA_Array_Forany (const _CORBA_Array_Forany<T_Helper,T>& p) {
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
  inline ~_CORBA_Array_Forany() { 
    // does not delete the storage of the underlying array
  }
  inline T& operator[] (CORBA::ULong index) {
    return *(pd_data + index);
  }
  inline const T& operator[] (CORBA::ULong index) const {
    return *(pd_data + index);
  }
  inline operator ptr_t& () {
    return pd_data;
  }
private:
  T* pd_data;
};



#endif // __CORBA_TEMPLATES_H__

