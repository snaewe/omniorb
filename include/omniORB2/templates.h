// -*- Mode: C++; -*-
//                            Package   : omniORB2
// templates.h                Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//       *** PROPRIETORY INTERFACE ***

/*
  $Log$
  Revision 1.4  1997/03/10 11:34:24  sll
  - T_var types can now be passed directly as arguments to operations that
    have variable length data types as INOUT and OUT parameters.
  - New templates: _CORBA_ObjRef_INOUT_arg, _CORBA_ObjRef_OUT_arg,
                   _CORBA_ConstrType_Variable_OUT_arg,
                   _CORBA_Sequence_OUT_arg, _CORBA_Array_OUT_arg
                   _CORBA_Sequence_Var
  - template _CORBA_ObjRef_Member now has the missing operator->
  - other minor cleanups.

  Revision 1.3  1997/02/04 13:56:07  sll
  Backup the previous change when DEC C++ compiler is used.

  Revision 1.2  1997/01/30 20:23:45  sll
  Remove reference to CORBA::is_nil() etc in _CORBA_ObjRef_Var and
  _CORBA_ObjRef_Member.

  Revision 1.1  1997/01/23 15:20:03  sll
  Initial revision

  */

#ifndef __TEMPLATES_H__
#define __TEMPLATES_H__

class NetBufferedStream;
class MemBufferedStream;

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
  inline _CORBA_Bounded_Sequence<T,max> &operator= (const _CORBA_Bounded_Sequence<T,max> &s);
  inline _CORBA_ULong length() const { return _CORBA_Sequence<T>::length(); }
  inline void length(_CORBA_ULong len);
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
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
            s);
  inline _CORBA_ULong length() const;
  inline void length(_CORBA_ULong len);
  // omniORB2 extensions
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

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
#ifndef __DECCXX
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
  inline T* operator->() { return pd_objref; }

  inline operator T* () { return pd_objref; }

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
#ifndef __DECCXX
    if (!T_Helper::is_nil(_ptr)) T_Helper::release(_ptr);
#else
    if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
#endif
  }
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (T * p);
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Member<T,T_Helper> &p);
  inline _CORBA_ObjRef_Member<T,T_Helper> &operator= (const _CORBA_ObjRef_Var<T,T_Helper> &p);

  inline T* operator->() { return _ptr; }

  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);

  // The following conversion operators are needed to support the
  // implicit conversion from this type to its T* data member.
  // They are used when this type is used as the rvalue of an expression.
  inline operator T* () { return _ptr; }

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
  inline T* operator->() { return &pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& ()  { return pd_data; }
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
  inline T* operator->() { return pd_data; }

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
  inline T* operator->() { return pd_data; }

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
  inline const T& operator[] (_CORBA_ULong index) const { return *(pd_data + index);  }
  inline operator T* () { return pd_data; }
  inline operator const T* () const { return pd_data; }

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
  inline T& operator[] (_CORBA_ULong index) { return *(pd_data + index); }
  inline const T& operator[] (_CORBA_ULong index) const { return *(pd_data + index); }
  inline operator T* () { return pd_data; }
  inline operator const T* () const { return pd_data; }
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

template <class T,int max>
inline 
size_t
_CORBA_Bounded_Sequence<T,max>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  for (unsigned long i=0; i < length(); i++) {
    alignedsize = NP_data()[i].NP_alignedSize(alignedsize);
  }
  return alignedsize;
}

template <class T,int max>
inline 
void
_CORBA_Bounded_Sequence<T,max>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.RdMessageUnRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
  return;
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator>>= (MemBufferedStream &s) const 
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.unRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
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

template <class T,int max,int elmSize, int elmAlignment>
inline
size_t
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  alignedsize = ((alignedsize+(elmAlignment-1)) & ~(elmAlignment-1));
  alignedsize += length() * elmSize;
  return alignedsize;
}


template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.RdMessageUnRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
  return;
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (MemBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.unRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
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

