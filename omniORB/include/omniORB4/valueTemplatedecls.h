// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueTemplatedecls.h       Created on: 2003/09/24
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2009 Apasphere Ltd.
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

#ifndef __VALUETEMPLATEDECLS_H__
#define __VALUETEMPLATEDECLS_H__


template <class T, class T_Helper>
class _CORBA_Value_Member;

template <class T, class T_Helper>
class _CORBA_Value_Element;

template <class T, class T_Helper>
class _CORBA_Value_INOUT_arg;

template <class T, class T_Helper>
class _CORBA_Value_OUT_arg;

template <class T, class ElemT, class T_Helper>
class _CORBA_Sequence_Value;

template <class T, class ElemT, class T_Helper>
class _CORBA_Unbounded_Sequence_Value;

template <class T, class ElemT, class T_Helper, int max>
class _CORBA_Bounded_Sequence_Value;


//////////////////////////////////////////////////////////////////////
//////////////////////////// _CORBA_Value_Var ////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
class _CORBA_Value_Var {
public:
  typedef _CORBA_Value_Var<T, T_Helper>     T_var;
  typedef _CORBA_Value_Member<T, T_Helper>  T_member;
  typedef _CORBA_Value_Element<T, T_Helper> T_element;

  inline _CORBA_Value_Var() : _pd_val(0) {}
  inline _CORBA_Value_Var(T* p) { _pd_val = p; }
  inline _CORBA_Value_Var(const T_var& p) : _pd_val(p._pd_val) {
    T_Helper::add_ref(p);
  }
  inline _CORBA_Value_Var(const T_member& p);
  inline _CORBA_Value_Var(const T_element& p);
  inline ~_CORBA_Value_Var() {
    T_Helper::remove_ref(_pd_val);
  }

  inline T_var& operator= (T* p) {
    T_Helper::remove_ref(_pd_val);
    _pd_val = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if (_pd_val != p._pd_val) {
      T_Helper::remove_ref(_pd_val);
      _pd_val = p._pd_val;
      T_Helper::add_ref(_pd_val);
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
    T_Helper::remove_ref(_pd_val);
    _pd_val = 0;
    return _pd_val;
  }
  inline T* _retn() {
    T* tmp = _pd_val;
    _pd_val = 0;
    return tmp;
  }

  friend class _CORBA_Value_Member   <T,T_Helper>;
  friend class _CORBA_Value_Element  <T,T_Helper>;
  friend class _CORBA_Value_INOUT_arg<T,T_Helper>;
  friend class _CORBA_Value_OUT_arg  <T,T_Helper>;

private:
  T* _pd_val;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// _CORBA_Value_Memeber ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
class _CORBA_Value_Member {
public:
  typedef _CORBA_Value_Var    <T,T_Helper> T_var;
  typedef _CORBA_Value_Member <T,T_Helper> T_member;
  typedef _CORBA_Value_Element<T,T_Helper> T_element;

  inline _CORBA_Value_Member() : _pd_val(0) {}
  inline _CORBA_Value_Member(const T_member& p) : _pd_val(p._pd_val) {
    T_Helper::add_ref(_pd_val);
  }
  inline ~_CORBA_Value_Member() {
    T_Helper::remove_ref(_pd_val);
  }

  inline T_member& operator= (T* p) {
    T_Helper::remove_ref(_pd_val);
    _pd_val = p;
    return *this;
  }
  inline T_member& operator= (const T_member& p) {
    if (_pd_val != p._pd_val) {
      T_Helper::remove_ref(_pd_val);
      _pd_val = p._pd_val;
      T_Helper::add_ref(_pd_val);
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    if (_pd_val != p._pd_val) {
      T_Helper::remove_ref(_pd_val);
      _pd_val = p._pd_val;
      T_Helper::add_ref(_pd_val);
    }
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline void operator>>= (cdrStream& s) const {
    T_Helper::marshal(_pd_val, s);
  }
  inline void operator<<= (cdrStream& s) {
    T* _result = T_Helper::unmarshal(s);
    T_Helper::remove_ref(_pd_val);
    _pd_val = _result;
  }

  inline T* operator->() const { return _pd_val; }
  inline operator T* () const  { return _pd_val; }

  inline T*  in() const { return _pd_val; }
  inline T*& inout()    { return _pd_val; }
  inline T*& out() {
    T_Helper::remove_ref(_pd_val);
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
//////////////////////////// _CORBA_Value_Element ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
class _CORBA_Value_Element {
public:
  typedef _CORBA_Value_Var    <T,T_Helper> T_var;
  typedef _CORBA_Value_Member <T,T_Helper> T_member;
  typedef _CORBA_Value_Element<T,T_Helper> T_element;

  inline _CORBA_Value_Element(T*& p, _CORBA_Boolean rel)
    : pd_rel(rel), _pd_val(p) {}

  inline _CORBA_Value_Element(const T_element& p)
    : pd_rel(p.pd_rel), _pd_val(p._pd_val) {}

  inline ~_CORBA_Value_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T* p) {
    if (pd_rel)
      T_Helper::remove_ref(_pd_val);
    _pd_val = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if (p._pd_val != _pd_val) {
      if (pd_rel) {
        T_Helper::remove_ref(_pd_val);
        _pd_val = p._pd_val;
        T_Helper::add_ref(_pd_val);
      }
      else
        _pd_val = p._pd_val;
    }
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if (pd_rel) {
      T_Helper::remove_ref(_pd_val);
      T_Helper::add_ref(p._pd_val);
    }
    _pd_val = (T*) p;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if (pd_rel) {
      T_Helper::remove_ref(_pd_val);
      T_Helper::add_ref(p._pd_val);
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
      T_Helper::remove_ref(_pd_val);
    }
    _pd_val = 0;
    return _pd_val;
  }

  inline T* _retn() {
    T* tmp = _pd_val;
    if (!pd_rel) {
      T_Helper::add_ref(_pd_val);
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

template <class T, class T_Helper>
inline
_CORBA_Value_Var<T,T_Helper>::
_CORBA_Value_Var(const _CORBA_Value_Member<T,T_Helper>& m)
{
  T_Helper::add_ref(m._pd_val);
  _pd_val = m._pd_val;
}

template <class T, class T_Helper>
inline
_CORBA_Value_Var<T,T_Helper>::
_CORBA_Value_Var(const _CORBA_Value_Element<T,T_Helper>& m)
{
  T_Helper::add_ref(m._pd_val);
  _pd_val = m._pd_val;
}


template <class T, class T_Helper>
inline _CORBA_Value_Var<T,T_Helper>&
_CORBA_Value_Var<T,T_Helper>::operator= (const _CORBA_Value_Member<T,T_Helper>& p)
{
  T_Helper::remove_ref(_pd_val);
  T_Helper::add_ref(p._pd_val);
  _pd_val = p._pd_val;
  return *this;
}

template <class T, class T_Helper>
inline _CORBA_Value_Var<T,T_Helper>&
_CORBA_Value_Var<T,T_Helper>::operator= (const _CORBA_Value_Element<T,T_Helper>& p)
{
  T_Helper::remove_ref(_pd_val);
  T_Helper::add_ref(p._pd_val);
  _pd_val = p._pd_val;
  return *this;
}

template <class T, class T_Helper>
inline _CORBA_Value_Member<T,T_Helper>&
_CORBA_Value_Member<T,T_Helper>::operator= (const _CORBA_Value_Element<T,T_Helper>& p)
{
  T_Helper::remove_ref(_pd_val);
  T_Helper::add_ref(p._pd_val);
  _pd_val = p._pd_val;
  return *this;
}


//////////////////////////////////////////////////////////////////////
////////////////////////// Value_INOUT_arg ///////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
class _CORBA_Value_INOUT_arg {
public:
  typedef _CORBA_Value_Var    <T,T_Helper> T_var;
  typedef _CORBA_Value_Member <T,T_Helper> T_member;
  typedef _CORBA_Value_Element<T,T_Helper> T_element;

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
      T_Helper::add_ref(p._NP_val());
  }
  inline ~_CORBA_Value_INOUT_arg() {}

  inline operator T*&()  { return _pd_val; }

  T*& _pd_val;

private:
  _CORBA_Value_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// Value_OUT_arg ////////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Helper>
class _CORBA_Value_OUT_arg {
public:
  typedef _CORBA_Value_Var    <T,T_Helper> T_var;
  typedef _CORBA_Value_Member <T,T_Helper> T_member;
  typedef _CORBA_Value_Element<T,T_Helper> T_element;
  typedef _CORBA_Value_OUT_arg<T,T_Helper> T_out;

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


//////////////////////////////////////////////////////////////////////
//////////////////////////// _CORBA_Sequence_Value ///////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Elem, class T_Helper>
class _CORBA_Sequence_Value {
public:
  typedef _CORBA_Sequence_Value<T,T_Elem,T_Helper> T_seq;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const  { return pd_len; }

  inline void length(_CORBA_ULong len)
  {
    if (len) {
      if (len > pd_max) {
	if (pd_bounded) {
	  _CORBA_bound_check_error();
	  // Never reach here.
	}
	// Grow exponentially
	_CORBA_ULong newmax = pd_max * 2;
	copybuffer((len > newmax) ? len : newmax);
      }
      else if (!pd_buf) {
	// Allocate initial buffer
	copybuffer(pd_max);
      }
      else {
	// We have shrunk. Clear the entries at the top
	for (_CORBA_ULong i = len; i < pd_len; i++) {
	  T_Helper::remove_ref(pd_buf[i]);
	  pd_buf[i] = 0;
	}
      }
    }
    else {
      // Zero length. Deallocate buffer if it is owned by us.
      if (pd_rel && pd_buf) {
	freebuf(pd_buf);
	pd_buf = 0;
	if (!pd_bounded)
	  pd_max = 0;
      }
    }
    pd_len = len;
  }

  inline T_Elem operator[] (_CORBA_ULong i) {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return T_Elem(pd_buf[i],pd_rel);
  }
  inline T_Elem operator[] (_CORBA_ULong i) const {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return T_Elem(pd_buf[i],pd_rel);
  }

  static inline T** allocbuf(_CORBA_ULong nelems) { 
    if (!nelems) return 0;

    T** b = new T*[nelems+1];

    for (_CORBA_ULong i_ = 1; i_ < nelems+1; i_++) {
      b[i_] = 0;
    }
    b[0] = (T*) (omni_ptr_arith_t) nelems;
    return b+1;
  }

  static inline void freebuf(T** buf) { 
    if (!buf) return;

    T** b = buf-1;

    _CORBA_ULong l = (_CORBA_ULong) (omni_ptr_arith_t) b[0];
    for (_CORBA_ULong i = 0; i < l; i++) {
      if (buf[i]) {
	T_Helper::remove_ref(buf[i]);
      }
    }
    b[0] = (T*) 0;
    delete [] b;
  }

  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }

  inline T** get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_buf) {
      copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_buf;
    }
    else {
      if (!pd_rel) {
	return 0;
      }
      else {
	T** tmp = pd_buf;
	pd_buf = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline T*const * get_buffer() const { 
    if (pd_max && !pd_buf) {
      T_seq* s = OMNI_CONST_CAST(T_seq*, this);
      s->copybuffer(pd_max);
    }
    return pd_buf; 
  }

  inline ~_CORBA_Sequence_Value() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
  }


  // omniORB extensions
  inline void operator>>= (cdrStream &s) const;
  inline void operator<<= (cdrStream &s);

  inline T** NP_data() const  { return pd_buf; }
  inline void NP_norelease()  { pd_rel = 0;     }

protected:
  inline _CORBA_Sequence_Value()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_buf(0) {}

  inline _CORBA_Sequence_Value(_CORBA_ULong max,
			       _CORBA_Boolean bounded=0)
    : pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_buf(0) {}

  inline _CORBA_Sequence_Value(_CORBA_ULong max,
			       _CORBA_ULong len,
			       T** value, 
			       _CORBA_Boolean release_ = 0,
			       _CORBA_Boolean bounded = 0)
    : pd_max(max), pd_len(len), pd_rel(release_),
      pd_bounded(bounded), pd_buf(value)
  {
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  inline _CORBA_Sequence_Value(const T_seq& s)
    : pd_max(s.pd_max), pd_len(0), pd_rel(1),
      pd_bounded(s.pd_bounded), pd_buf(0)
  {
    length(s.pd_len);
    for (_CORBA_ULong i = 0; i < pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
      if (pd_buf[i])
	T_Helper::add_ref(pd_buf[i]);
    }
  }

  inline T_seq& operator= (const T_seq& s) {
    _CORBA_ULong i;

    if (pd_rel) {
      for (i=0; i < pd_len; i++) {
	T_Helper::remove_ref(pd_buf[i]);
	pd_buf[i] = 0;
      }
      pd_len = 0;
      length(s.pd_len);
      for (i = 0; i < pd_len; i++) {
	pd_buf[i] = s.pd_buf[i];
	if (pd_buf[i])
	  T_Helper::add_ref(pd_buf[i]);
      }
    }
    else {
      pd_len = 0;
      length(s.pd_len);
      for (i = 0; i < pd_len; i++) {
	pd_buf[i] = s.pd_buf[i];
      }
    }
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_buf) {
      freebuf(pd_buf);
    }
    pd_max = max;
    pd_len = len;
    pd_buf = data;
    pd_rel = release_;
  }


protected:

  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_buf with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    T** newdata = allocbuf(newmax);
    if (!newdata) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    if (pd_rel) {
      for (unsigned long i_=0; i_ < pd_len; i_++) {
	newdata[i_] = pd_buf[i_];
	pd_buf[i_]  = 0;
      }
      if (pd_buf) {
	freebuf(pd_buf);
      }
    }
    else {
      for (unsigned long i_=0; i_ < pd_len; i_++) {
	T_Helper::add_ref(pd_buf[i_]);
	newdata[i_] = pd_buf[i_];
      }
      pd_rel = 1;
    }
    pd_buf = newdata;
    pd_max = newmax;
  }

  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  _CORBA_Boolean  pd_bounded;
  T**             pd_buf;
};


//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Unbounded_Sequence_Value ////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, class T_Elem, class T_Helper>
class _CORBA_Unbounded_Sequence_Value : public _CORBA_Sequence_Value<T,T_Elem,T_Helper> {
public:
  typedef _CORBA_Unbounded_Sequence_Value<T,T_Elem,T_Helper> T_seq;
  typedef _CORBA_Sequence_Value<T,T_Elem,T_Helper>           Base_T_seq;

  inline _CORBA_Unbounded_Sequence_Value() {}
  inline _CORBA_Unbounded_Sequence_Value(_CORBA_ULong max) : 
    Base_T_seq(max) {}

  inline _CORBA_Unbounded_Sequence_Value(_CORBA_ULong    max,
					  _CORBA_ULong   len,
					  T**            value,
					  _CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel) {}

  inline _CORBA_Unbounded_Sequence_Value(const T_seq& s) : Base_T_seq(s) {}

  inline ~_CORBA_Unbounded_Sequence_Value() {}

  inline T_seq &operator= (const T_seq& s) {
    Base_T_seq::operator= (s);
    return *this;
  };

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
///////////////// _CORBA_Bounded_Sequence_Value //////////////////////
//////////////////////////////////////////////////////////////////////

template<class T, class T_Elem, class T_Helper, int max>
class _CORBA_Bounded_Sequence_Value : public _CORBA_Sequence_Value<T,T_Elem,T_Helper> {
public:
  typedef _CORBA_Bounded_Sequence_Value<T,T_Elem,T_Helper,max> T_seq;
  typedef _CORBA_Sequence_Value<T,T_Elem,T_Helper>             Base_T_seq;

  inline _CORBA_Bounded_Sequence_Value() : Base_T_seq(max,1){}

  inline _CORBA_Bounded_Sequence_Value(_CORBA_ULong    len,
					T**            value,
					_CORBA_Boolean rel = 0) : 
    Base_T_seq(max,len,value,rel,1) {}

  inline _CORBA_Bounded_Sequence_Value(const T_seq& s) : Base_T_seq(s) {}

  inline ~_CORBA_Bounded_Sequence_Value() {}

  inline T_seq& operator= (const T_seq&s) {
    Base_T_seq::operator= (s);
    return *this;
  }

  static inline T** allocbuf(_CORBA_ULong nelems) {
    return Base_T_seq::allocbuf(max);
  }

  static inline T** allocbuf() { return allocbuf(max); }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong len, T** data,
		      _CORBA_Boolean release_ = 0) {
    Base_T_seq::replace(max,len,data,release_);
  }
};




#endif // __VALUETEMPLATEDECLS_H__
