// -*- Mode: C++; -*-
//                            Package   : omniORB2
// wstringtypes.h             Created on: 27/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library.
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
//    The CORBA wide string type helpers. Also sequence of wide string.
//

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:03:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.10  2003/01/16 12:47:08  dgrisby
  Const cast macro. Thanks Matej Kenda.

  Revision 1.1.2.9  2003/01/14 11:48:16  dgrisby
  Remove warnings from gcc -Wshadow. Thanks Pablo Mejia.

  Revision 1.1.2.8  2002/03/11 12:23:03  dpg1
  Tweaks to avoid compiler warnings.

  Revision 1.1.2.7  2001/11/06 15:41:35  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.1.2.6  2001/09/19 17:26:44  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.2.5  2001/08/17 13:44:08  dpg1
  Change freeing behaviour of string members and elements.

  Revision 1.1.2.4  2000/11/17 19:11:16  dpg1
  Rename _CORBA_Sequence__WString to _CORBA_Sequence_WString.

  Revision 1.1.2.3  2000/11/15 17:04:33  sll
  Removed marshalling functions from WString_helper.

  Revision 1.1.2.2  2000/11/09 12:27:50  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.1.2.1  2000/10/27 15:42:03  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#ifndef __OMNI_WSTRINGTYPES_H__
#define __OMNI_WSTRINGTYPES_H__


class _CORBA_WString_member;
class _CORBA_WString_element;
class _CORBA_WString_inout;
class _CORBA_WString_out;
class cdrStream;


class _CORBA_WString_helper {
public:

static _core_attr const _CORBA_WChar*const  empty_wstring;
// Used to initialise empty wide strings, since some compilers will
// allocate a separate instance for each "" in code.

static inline _CORBA_WChar* alloc(int len_)
  { return new _CORBA_WChar[len_ + 1]; }
// Allocate a wide string -- as CORBA::wstring_alloc(), except that we
// don't initialise to empty string.  <len> does not include nul
// terminator.

static inline void free(_CORBA_WChar* s) { 
  if (s && s != empty_wstring) delete[] s; 
}
// As CORBA::wstring_free().

static inline int len(const _CORBA_WChar* s) {
  int i;
  for (i=0; *s; s++, i++);
  return i;
}

static inline void cpy(_CORBA_WChar* t, const _CORBA_WChar* f) {
  while (*f) *t++ = *f++;
  *t = 0;
}

static inline _CORBA_WChar* dup(const _CORBA_WChar* s) { 
  _CORBA_WChar* r = alloc(len(s));
  if (r) {
    cpy(r, s);
    return r;
  }
  return 0;
}
// As CORBA::wstring_dup().

};

//////////////////////////////////////////////////////////////////////
///////////////////////////// WString_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_WString_var {
public:
  // Note: default constructor of WString_var initialises its managed pointer
  //       to 0. This differs from WString_member, which initialises its
  //       internal pointer to _CORBA_WString_helper::empty_wstring


  typedef _CORBA_WChar* ptr_t;

  inline _CORBA_WString_var()                { _data = 0; }
  inline _CORBA_WString_var(_CORBA_WChar* p) { _data = p; }

  inline _CORBA_WString_var(const _CORBA_WChar* p) {
    if( p ) _data = _CORBA_WString_helper::dup(p);
    else    _data = 0;
  }

  inline _CORBA_WString_var(const _CORBA_WString_var& s) {
    if( (const _CORBA_WChar*)s )  _data = _CORBA_WString_helper::dup(s);
    else                          _data = 0;
  }

  inline _CORBA_WString_var(const _CORBA_WString_member& s);

  inline _CORBA_WString_var(const _CORBA_WString_element& s);

  inline ~_CORBA_WString_var() {
    _CORBA_WString_helper::free(_data);
  }

  inline _CORBA_WString_var& operator=(_CORBA_WChar* p) {
    _CORBA_WString_helper::free(_data);
    _data = p;
    return *this;
  }

  inline _CORBA_WString_var& operator=(const _CORBA_WChar* p) {
    _CORBA_WString_helper::free(_data);
    _data = 0;
    if (p)  _data = _CORBA_WString_helper::dup(p);
    return *this;
  }

  inline _CORBA_WString_var& operator=(const _CORBA_WString_var& s) {
    if (&s != this) {
      _CORBA_WString_helper::free(_data);
      _data = 0;
      if( (const _CORBA_WChar*)s )  _data = _CORBA_WString_helper::dup(s);
    }
    return *this;
  }

  inline _CORBA_WString_var& operator=(const _CORBA_WString_member& s);

  inline _CORBA_WString_var& operator=(const _CORBA_WString_element& s);

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator _CORBA_WChar* ()             { return _data; }
  inline operator const _CORBA_WChar* () const { return _data; }
#else
  inline operator _CORBA_WChar* () const { return _data; }
#endif

  inline _CORBA_WChar& operator[] (_CORBA_ULong index_) {
    if (!_data || (_CORBA_ULong)_CORBA_WString_helper::len(_data) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return _data[index_];
  }

  inline _CORBA_WChar operator[] (_CORBA_ULong index_) const {
    if (!_data || (_CORBA_ULong)_CORBA_WString_helper::len(_data) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return _data[index_];
  }

  inline const _CORBA_WChar* in() const { return _data; }
  inline _CORBA_WChar*& inout()         { return _data; }
  inline _CORBA_WChar*& out() {
    if( _data ){
      _CORBA_WString_helper::free(_data);
      _data = 0;
    }
    return _data;
  }
  inline _CORBA_WChar* _retn() {
    _CORBA_WChar* tmp = _data;
    _data = 0;
    return tmp;
  }

  friend class _CORBA_WString_inout;
  friend class _CORBA_WString_out;

private:
  _CORBA_WChar* _data;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// WString_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_WString_member {
public:
  typedef _CORBA_WChar* ptr_t;

  inline _CORBA_WString_member()
    : _ptr(OMNI_CONST_CAST(_CORBA_WChar*, _CORBA_WString_helper::empty_wstring)) {}

  inline _CORBA_WString_member(const _CORBA_WString_member& s)  
    : _ptr(OMNI_CONST_CAST(_CORBA_WChar*, _CORBA_WString_helper::empty_wstring)) {
    if (s._ptr && s._ptr != _CORBA_WString_helper::empty_wstring)
      _ptr = _CORBA_WString_helper::dup(s._ptr);
  }

  inline _CORBA_WString_member(const _CORBA_WChar* s) {
    _ptr = _CORBA_WString_helper::dup(s);
  }

  inline ~_CORBA_WString_member() {
    _CORBA_WString_helper::free(_ptr);
  }

  inline _CORBA_WString_member& operator=(_CORBA_WChar* s) {
    _CORBA_WString_helper::free(_ptr);
    _ptr = s;
    return *this;
  }

  inline _CORBA_WString_member& operator= (const _CORBA_WChar* s) {
    _CORBA_WString_helper::free(_ptr);
    if (s)
      _ptr = _CORBA_WString_helper::dup(s);
    else
      _ptr = 0;
    return *this;
  }

  inline _CORBA_WString_member& operator=(const _CORBA_WString_member& s) {
    if (&s != this) {
      _CORBA_WString_helper::free(_ptr);
      if (s._ptr && s._ptr != _CORBA_WString_helper::empty_wstring)
	_ptr = _CORBA_WString_helper::dup(s._ptr);
      else
	_ptr = s._ptr;
    }
    return *this;
  }

  inline _CORBA_WString_member& operator=(const _CORBA_WString_var& s) {
    _CORBA_WString_helper::free(_ptr);
    if( (const _CORBA_WChar*)s ) {
      _ptr = _CORBA_WString_helper::dup((const _CORBA_WChar*)s);
    }
    else {
      _ptr = (_CORBA_WChar*) 0;
    }
    return *this;
  }

  inline _CORBA_WString_member& operator=(const _CORBA_WString_element& s);

  inline _CORBA_WChar& operator[] (_CORBA_ULong index_) {
    if (!_ptr || (_CORBA_ULong)_CORBA_WString_helper::len(_ptr) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return _ptr[index_];
  }

  inline _CORBA_WChar operator[] (_CORBA_ULong index_) const {
    if (!_ptr || (_CORBA_ULong)_CORBA_WString_helper::len(_ptr) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return _ptr[index_];
  }

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator _CORBA_WChar* ()             { return _ptr; }
  inline operator const _CORBA_WChar* () const { return _ptr; }
#else
  inline operator _CORBA_WChar* () const { return _ptr; }
#endif

  inline const _CORBA_WChar* in() const { return _ptr; }
  inline _CORBA_WChar*& inout()         { return _ptr; }
  inline _CORBA_WChar*& out() {
    _CORBA_WString_helper::free(_ptr);
    _ptr = 0;
    return _ptr;
  }

  inline _CORBA_WChar* _retn() {
    _CORBA_WChar* tmp;
    tmp = _ptr;
    _ptr = 0;
    return tmp;
  }

  void operator >>= (cdrStream& s) const;
  void operator <<= (cdrStream& s);

  inline _CORBA_WChar*& _NP_ref() {return _ptr;}

  _CORBA_WChar* _ptr;

};

//////////////////////////////////////////////////////////////////////
//////////////////////////// WString_element /////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_WString_element {

public:

  inline _CORBA_WString_element(_CORBA_WChar*& p, _CORBA_Boolean rel) 
    : pd_rel(rel), pd_data(p) {}

  inline _CORBA_WString_element(const _CORBA_WString_element& s) 
    : pd_rel(s.pd_rel), pd_data(s.pd_data) {}

  inline ~_CORBA_WString_element() {
  // intentionally does nothing.
  }

  inline _CORBA_WString_element& operator=(_CORBA_WChar* s) {
    if (pd_rel) 
      _CORBA_WString_helper::free(pd_data);
    pd_data = s;
    return *this;
  }

  inline _CORBA_WString_element& operator= (const _CORBA_WChar* s) {
    if (pd_rel)
      _CORBA_WString_helper::free(pd_data);
    if (s)
      pd_data = _CORBA_WString_helper::dup(s);
    else
      pd_data = 0;
    return *this;
  }

  inline _CORBA_WString_element& operator=(const _CORBA_WString_element& s) {
    if (&s != this) {
      if (pd_rel)
	_CORBA_WString_helper::free(pd_data);
      if (s.pd_data && s.pd_data != _CORBA_WString_helper::empty_wstring)
	pd_data = _CORBA_WString_helper::dup(s.pd_data);
      else
	pd_data = (_CORBA_WChar*)s.pd_data;
    }
    return *this;
  }

  inline _CORBA_WString_element& operator=(const _CORBA_WString_var& s) {
    if (pd_rel)
      _CORBA_WString_helper::free(pd_data);
    if( (const _CORBA_WChar*)s )
      pd_data = _CORBA_WString_helper::dup((const _CORBA_WChar*)s);
    else
      pd_data = 0;
    return *this;
  }

  inline _CORBA_WString_element& operator=(const _CORBA_WString_member& s) {
    if (pd_rel)
      _CORBA_WString_helper::free(pd_data);
    if( (const _CORBA_WChar*)s &&
	(const _CORBA_WChar*) s != _CORBA_WString_helper::empty_wstring)
      pd_data = _CORBA_WString_helper::dup((const _CORBA_WChar*)s);
    else {
      pd_data = OMNI_CONST_CAST(_CORBA_WChar*, (const _CORBA_WChar*)s);
    }
    return *this;
  }

  inline _CORBA_WChar& operator[] (_CORBA_ULong index_) {
    if (!((_CORBA_WChar*)pd_data) ||
	(_CORBA_ULong)_CORBA_WString_helper::len(pd_data) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return pd_data[index_];
  }

  inline _CORBA_WChar operator[] (_CORBA_ULong index_) const {
    if (!((_CORBA_WChar*)pd_data) ||
	(_CORBA_ULong)_CORBA_WString_helper::len(pd_data) < index_) {
      _CORBA_bound_check_error();	// never return
    }
    return pd_data[index_];
  }

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator _CORBA_WChar* ()             { return pd_data; }
  inline operator const _CORBA_WChar* () const { return pd_data; }
#else
  inline operator _CORBA_WChar* () const { return pd_data; }
#endif

  inline const _CORBA_WChar* in() const { return pd_data; }
  inline _CORBA_WChar*& inout()         { return pd_data; }
  inline _CORBA_WChar*& out() {
    if (pd_rel) {
      _CORBA_WString_helper::free(pd_data);
      pd_data = 0;
    }
    else {
      pd_data = 0;
    }
    return pd_data;
  }
  inline _CORBA_WChar* _retn() {
    _CORBA_WChar* tmp;
    if (pd_rel) {
      tmp = pd_data;
      pd_data = 0;
    }
    else {
      tmp = (((_CORBA_WChar*)pd_data) ? _CORBA_WString_helper::dup(pd_data) : 0);
      pd_data = 0;
    }
    return tmp;
  }


  inline _CORBA_WChar*& _NP_ref() const {return pd_data;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  _CORBA_WChar*&         pd_data;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// WString_var   ////////////////////////////
//////////////////////////// WString_member////////////////////////////
//////////////////////////// operator=    ////////////////////////////
//////////////////////////// copy ctors   ////////////////////////////
//////////////////////////////////////////////////////////////////////

inline _CORBA_WString_var::_CORBA_WString_var(const _CORBA_WString_member& s)
{
  if ((const _CORBA_WChar*)s) {
    _data = _CORBA_WString_helper::dup(s);
  }
  else
    _data = 0;
}

inline _CORBA_WString_var::_CORBA_WString_var(const _CORBA_WString_element& s)
{
  if ((const _CORBA_WChar*)s) {
    _data = _CORBA_WString_helper::dup(s);
  }
  else
    _data = 0;
}

inline _CORBA_WString_var&
_CORBA_WString_var::operator= (const _CORBA_WString_member& s)
{
  _CORBA_WString_helper::free(_data);
  if ((const _CORBA_WChar*)s) 
    _data = _CORBA_WString_helper::dup(s);
  else
    _data = 0;
  return *this;
}

inline _CORBA_WString_var&
_CORBA_WString_var::operator= (const _CORBA_WString_element& s)
{
  _CORBA_WString_helper::free(_data);
  if ((const _CORBA_WChar*)s)
    _data = _CORBA_WString_helper::dup(s);
  else
    _data = 0;
  return *this;
}

inline _CORBA_WString_member& 
_CORBA_WString_member::operator=(const _CORBA_WString_element& s) {
  _CORBA_WString_helper::free(_ptr);
  if( (const _CORBA_WChar*)s )
    _ptr = _CORBA_WString_helper::dup((const _CORBA_WChar*)s);
  else
    _ptr = 0;
  return *this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// WString_inout ////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_WString_inout {
public:
  inline _CORBA_WString_inout(_CORBA_WChar*& p) : _data(p) {}
  inline _CORBA_WString_inout(_CORBA_WString_var& p) : _data(p._data) {}
  inline _CORBA_WString_inout(_CORBA_WString_member& p) : _data(p._ptr) {}
  inline _CORBA_WString_inout(_CORBA_WString_element& p) : _data(p._NP_ref()) {
  // If the WString_element has pd_rel == 0,
  // the string buffer is not owned by the sequence and should not
  // be freed. Since this is an inout argument and the callee may call
  // string_free, we create a copy to pass to the callee. This will result
  // in a memory leak! This only occurs when there is a programming error
  // and cannot be trapped by the compiler.
    if (!p._NP_release() && (p._NP_ref())) {
      p._NP_ref() = ((p._NP_ref()) ?
		     _CORBA_WString_helper::dup(p._NP_ref()) : 0);
    }
  }
  inline ~_CORBA_WString_inout() {}

  inline operator _CORBA_WChar*&()  { return _data; }

  _CORBA_WChar*& _data;

private:
  _CORBA_WString_inout();
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// WString_out /////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_WString_out {
public:
  inline _CORBA_WString_out(_CORBA_WChar*& p) : _data(p) { _data = 0; }
  inline _CORBA_WString_out(_CORBA_WString_var& p) : _data(p._data) { p = (_CORBA_WChar*)0; }
  inline _CORBA_WString_out(_CORBA_WString_member& p) : _data(p._ptr) { p = (_CORBA_WChar*)0; }
  inline _CORBA_WString_out(_CORBA_WString_element& p) : _data(p._NP_ref()) { p = (_CORBA_WChar*)0; }
  inline ~_CORBA_WString_out() {}
  inline _CORBA_WString_out(const _CORBA_WString_out& p) : _data(p._data) {}
  inline _CORBA_WString_out& operator=(const _CORBA_WString_out& p) {
    _data = p._data; return *this;
  }
  inline _CORBA_WString_out& operator=(_CORBA_WChar* p) { 
    _data = p; return *this;
  }
  inline _CORBA_WString_out& operator=(const _CORBA_WChar* p) {
    _data = ((p) ? _CORBA_WString_helper::dup(p) : 0); return *this;
  }

  operator _CORBA_WChar*& () { return _data; }
  _CORBA_WChar*& ptr()       { return _data; }

  _CORBA_WChar*& _data;

private:
  _CORBA_WString_out();
  _CORBA_WString_out& operator=(const _CORBA_WString_var& );
  _CORBA_WString_out& operator=(const _CORBA_WString_member& );
  _CORBA_WString_out& operator=(const _CORBA_WString_element& );
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_WString           /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Sequence_WString
{
public:
  typedef _CORBA_WString_element ElemT;
  typedef _CORBA_Sequence_WString SeqT;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const  { return pd_len; }
  inline void length(_CORBA_ULong len) {
    if (pd_bounded && len > pd_max) {
      _CORBA_bound_check_error();
      // never reach here.
    }

    // If we've shrunk we need to clear the entries at the top.
    for( _CORBA_ULong i = len; i < pd_len; i++ ) {
      operator[](i) = OMNI_CONST_CAST(_CORBA_WChar*, _CORBA_WString_helper::empty_wstring);
    }

    if (len) {
      // Allocate buffer on-demand. Either pd_data == 0 
      //                            or pd_data = buffer for pd_max elements
      if (!pd_data || len > pd_max) {
	copybuffer(((len > pd_max) ? len : pd_max));
      }
    }
    pd_len = len;
  }

  inline ElemT operator[] (_CORBA_ULong i) {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return ElemT(pd_data[i],pd_rel);
  }
  inline const ElemT operator[] (_CORBA_ULong i) const {
    if( i >= pd_len )  _CORBA_bound_check_error();
    return ElemT(pd_data[i],pd_rel);
  }

#if SIZEOF_PTR == SIZEOF_LONG
  typedef long ptr_arith_t;
#elif SIZEOF_PTR == SIZEOF_INT
  typedef int ptr_arith_t;
#else
#error "No suitable type to do pointer arithmetic"
#endif

  static inline _CORBA_WChar** allocbuf(_CORBA_ULong nelems) {
    if (!nelems) return 0;
    _CORBA_WChar** b = new _CORBA_WChar*[nelems+2];
    ptr_arith_t l = nelems;
    b[0] = (_CORBA_WChar*) ((ptr_arith_t) 0x53515354U);
    b[1] = (_CORBA_WChar*) l;
    for (_CORBA_ULong index_ = 2; index_ < (nelems+2); index_++) {
      b[index_] = OMNI_CONST_CAST(_CORBA_WChar*,
				  _CORBA_WString_helper::empty_wstring);
    }
    return b+2;
  }

  static inline void freebuf(_CORBA_WChar** buf) {
    if (!buf) return;
    _CORBA_WChar** b = buf-2;
    if ((ptr_arith_t)b[0] != ((ptr_arith_t) 0x53515354U)) {
      _CORBA_bad_param_freebuf();
      return;
    }
    ptr_arith_t l = (ptr_arith_t) b[1];
    for (_CORBA_ULong i = 0; i < (_CORBA_ULong) l; i++) {
      _CORBA_WString_helper::free(buf[i]);
    }
    b[0] = (_CORBA_WChar*) 0;
    delete [] b;
  }


  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }

  inline _CORBA_WChar** get_buffer(_CORBA_Boolean orphan = 0) {
    if (pd_max && !pd_data) {
      copybuffer(pd_max);
    }
    if (!orphan) {
      return pd_data;
    }
    else {
      if (!pd_rel)
	return 0;
      else {
	_CORBA_WChar** tmp = pd_data;
	pd_data = 0;
	if (!pd_bounded) {
	  pd_max = 0;
	}
	pd_len = 0;
	pd_rel = 1;
	return tmp;
      }
    }
  }

  inline const _CORBA_WChar* const* get_buffer() const { 
    if (pd_max && !pd_data) {
      _CORBA_Sequence_WString* s = OMNI_CONST_CAST(_CORBA_Sequence_WString*, this);
      s->copybuffer(pd_max);
    }
#if !defined(__DECCXX) || (__DECCXX_VER > 60000000)
    return pd_data; 
#else
    return (char const* const*)pd_data; 
#endif
  }


  inline ~_CORBA_Sequence_WString() { 
    if (pd_rel && pd_data) freebuf(pd_data);
    pd_data = 0;
  }

  // omniORB2 extensions
  void operator >>= (cdrStream& s) const;
  void operator <<= (cdrStream& s);

protected:
  inline _CORBA_Sequence_WString()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_data(0) {}

  inline _CORBA_Sequence_WString(_CORBA_ULong max,
				 _CORBA_Boolean bounded=0)
    : pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_data(0) {
  }

  inline _CORBA_Sequence_WString(_CORBA_ULong   max,
				 _CORBA_ULong   len,
				 _CORBA_WChar**         value,
				 _CORBA_Boolean release_ = 0,
				 _CORBA_Boolean bounded = 0)
     : pd_max(max), pd_len(len), pd_rel(release_),
       pd_bounded(bounded), pd_data(value)  { 
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  inline _CORBA_Sequence_WString(const SeqT& s)
    : pd_max(s.pd_max), pd_len(0), pd_rel(1),
      pd_bounded(s.pd_bounded), pd_data(0) {
    length(s.pd_len);
    for( _CORBA_ULong i = 0; i < pd_len; i++ )  
      operator[](i) = s[i];
  }

  inline SeqT& operator = (const SeqT& s) {
    length(s.pd_len);
    for( _CORBA_ULong i = 0; i < pd_len; i++ )  
      operator[](i) = s[i];
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, _CORBA_WChar** data,
		      _CORBA_Boolean release_ = 0) {
    if (len > max || (len && !data)) {
      _CORBA_bound_check_error();
      // never reach here
    }
    if (pd_rel && pd_data) {
      freebuf(pd_data);
    }
    pd_max = max;
    pd_len = len;
    pd_data = data;
    pd_rel = release_;
  }


private:

  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_data with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    _CORBA_WChar** newdata = allocbuf(newmax);
    if (!newdata) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (unsigned long i=0; i < pd_len; i++) {
      if (pd_rel) {
	newdata[i] = pd_data[i];
	pd_data[i] = 0;
      }
      else {
	newdata[i] = ((pd_data[i]) ? 
		      _CORBA_WString_helper::dup(pd_data[i]) : 0);
      }
    }
    if (pd_rel) {
      if (pd_data) freebuf(pd_data);
    }
    else {
      pd_rel = 1;
    }
    pd_data = newdata;
    pd_max = newmax;
  }

  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  _CORBA_Boolean  pd_bounded;
  _CORBA_WChar**          pd_data;
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Bounded_Sequence_WString //////////////////
//////////////////////////////////////////////////////////////////////

template<int max>
class _CORBA_Bounded_Sequence_WString
  : public _CORBA_Sequence_WString
{
public:
  typedef _CORBA_Bounded_Sequence_WString<max> SeqT;

  inline _CORBA_Bounded_Sequence_WString()
    : _CORBA_Sequence_WString(max,1) {}

  inline _CORBA_Bounded_Sequence_WString(_CORBA_ULong   length_,
					 _CORBA_WChar**         value,
					 _CORBA_Boolean release_ = 0)
    : _CORBA_Sequence_WString(max, length_, value, release_, 1) {}

  inline _CORBA_Bounded_Sequence_WString(const SeqT& s)
    : _CORBA_Sequence_WString(s) {}

  inline ~_CORBA_Bounded_Sequence_WString() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence_WString::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong len, _CORBA_WChar** data,
		      _CORBA_Boolean release_ = 0) {
    _CORBA_Sequence_WString::replace(max,len,data,release_);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence_WString /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_WString
  : public _CORBA_Sequence_WString
{
public:
  typedef _CORBA_Unbounded_Sequence_WString SeqT;

  inline _CORBA_Unbounded_Sequence_WString() {}

  inline _CORBA_Unbounded_Sequence_WString(_CORBA_ULong max) :
         _CORBA_Sequence_WString(max) {}

  inline _CORBA_Unbounded_Sequence_WString(_CORBA_ULong   max,
					   _CORBA_ULong   length_,
					   _CORBA_WChar** value,
					   _CORBA_Boolean release_ = 0)
    : _CORBA_Sequence_WString(max, length_, value, release_) {}

  inline _CORBA_Unbounded_Sequence_WString(const SeqT& s)
    : _CORBA_Sequence_WString(s) {}

  inline ~_CORBA_Unbounded_Sequence_WString() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence_WString::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, _CORBA_WChar** data,
		      _CORBA_Boolean release_ = 0) {
    _CORBA_Sequence_WString::replace(max,len,data,release_);
  }

};


#endif  // __OMNI_WSTRINGTYPES_H__
