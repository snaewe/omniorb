// -*- Mode: C++; -*-
//                            Package   : omniORB2
// stringtypes.h              Created on: 16/4/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    The CORBA string type helpers. Also sequence of string.
//

/*
 $Log$
 Revision 1.2.2.5  2000/11/22 14:37:59  dpg1
 Code set marshalling functions now take a string length argument.

 Revision 1.2.2.4  2000/11/09 12:27:50  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.2.2.3  2000/11/03 19:04:34  sll
 Renamed _CORBA_Unbounded_Sequence__String to _CORBA_Unbounded_Sequence_String
 and _CORBA_Bounded_Sequence__String to _CORBA_Bounded_Sequence_String
 Removed marshal() and unmarshal() from String_helper.

 Revision 1.2.2.2  2000/09/27 17:02:24  sll
 Consolidate all string allocation functions into the _CORBA_String_helper
 class. Updated to use the new cdrStream abstraction.

 Revision 1.2.2.1  2000/07/17 10:35:37  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:04  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.4  2000/06/27 16:15:08  sll
 New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
 _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
 sequence of string and a sequence of object reference.

 Revision 1.1.2.3  2000/02/09 15:01:28  djr
 Fixed _CORBA_String_member bug.

 Revision 1.1.2.2  2000/01/27 16:31:32  djr
 String_member now initialised to empty string by default.

 Revision 1.1.2.1  1999/09/24 09:51:54  djr
 Moved from omniORB2 + some new files.

 Revision 1.4  1999/06/18 20:28:30  sll
 New Sequence string implementation. New string_member.

 Revision 1.3  1999/06/03 17:10:50  sll
 Updated String_out and String_var to CORBA 2.2

 Revision 1.2  1999/05/25 13:24:06  sll
 Removed unnecessary const qualifier in operator char*().

 Revision 1.1  1999/04/21 11:18:31  djr
 Initial revision

*/

#ifndef __OMNI_STRINGTYPES_H__
#define __OMNI_STRINGTYPES_H__


class _CORBA_String_member;
class _CORBA_String_element;
class _CORBA_String_inout;
class _CORBA_String_out;
class cdrStream;


class _CORBA_String_helper {
public:

static _core_attr const char*const  empty_string;
// Used to initialise empty strings, since some compilers will allocate
// a separate instance for each "" in code.

static inline char* alloc(int len) { return new char[len + 1]; }
// Allocate a string -- as CORBA::string_alloc(), except that
// we don't initialise to empty string.
//  <len> does not include nul terminator.

static inline void free(char* s) { 
  if (s && s != empty_string) delete[] s; 
}
// As CORBA::string_free().

static inline char* dup(const char* s) { 
  char* r = alloc(strlen(s));
  strcpy(r, s);
  return r;
}
// As CORBA::string_dup().

static void unmarshal_zero_length_string();
// Marshalling code calls this if we receive a zero length string. All
// strings should be at least length 1 because they contain a
// terminating null. Logs an error message, and throws MARSHAL if
// strict IIOP, otherwise the marshalling code treats it as an empty
// string.

};

//////////////////////////////////////////////////////////////////////
///////////////////////////// String_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_var {
public:
  // Note: default constructor of String_var initialises its managed pointer
  //       to 0. This differs from String_member, which initialises its
  //       internal pointer to _CORBA_String_helper::empty_string


  typedef char* ptr_t;

  inline _CORBA_String_var()        { _data = 0; }
  inline _CORBA_String_var(char *p) { _data = p; }

  inline _CORBA_String_var(const char* p) {
    if( p ) _data = _CORBA_String_helper::dup(p);
    else    _data = 0;
  }

  inline _CORBA_String_var(const _CORBA_String_var& s) {
    if( (const char *)s )  _data = _CORBA_String_helper::dup(s);
    else                   _data = 0;
  }

  inline _CORBA_String_var(const _CORBA_String_member& s);

  inline _CORBA_String_var(const _CORBA_String_element& s);

  inline ~_CORBA_String_var() {
    _CORBA_String_helper::free(_data);
  }

  inline _CORBA_String_var& operator=(char* p) {
    _CORBA_String_helper::free(_data);
    _data = p;
    return *this;
  }

  inline _CORBA_String_var& operator=(const char* p) {
    _CORBA_String_helper::free(_data);
    _data = 0;
    if (p)  _data = _CORBA_String_helper::dup(p);
    return *this;
  }

  inline _CORBA_String_var& operator=(const _CORBA_String_var& s) {
    if (&s != this) {
      _CORBA_String_helper::free(_data);
      _data = 0;
      if( (const char*)s )  _data = _CORBA_String_helper::dup(s);
    }
    return *this;
  }

  inline _CORBA_String_var& operator=(const _CORBA_String_member& s);

  inline _CORBA_String_var& operator=(const _CORBA_String_element& s);

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator char* ()             { return _data; }
  inline operator const char* () const { return _data; }
#else
  inline operator char* () const { return _data; }
#endif

  inline char& operator[] (_CORBA_ULong index) {
    if (!_data || (_CORBA_ULong)strlen(_data) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return _data[index];
  }

  inline char operator[] (_CORBA_ULong index) const {
    if (!_data || (_CORBA_ULong)strlen(_data) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return _data[index];
  }

  inline const char* in() const { return _data; }
  inline char*& inout()         { return _data; }
  inline char*& out() {
    if( _data ){
      _CORBA_String_helper::free(_data);
      _data = 0;
    }
    return _data;
  }
  inline char* _retn() {
    char* tmp = _data;
    _data = 0;
    return tmp;
  }

  friend class _CORBA_String_inout;
  friend class _CORBA_String_out;

private:
  char* _data;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// String_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_member {
public:
  typedef char* ptr_t;

  inline _CORBA_String_member()
    : _ptr((char*) _CORBA_String_helper::empty_string) {}

  inline _CORBA_String_member(const _CORBA_String_member& s) 
           : _ptr((char*)_CORBA_String_helper::empty_string) {
    if (s._ptr && s._ptr != _CORBA_String_helper::empty_string)
      _ptr = _CORBA_String_helper::dup(s._ptr);
  }

  inline ~_CORBA_String_member() {
    _CORBA_String_helper::free(_ptr);
  }

  inline _CORBA_String_member& operator=(char* s) {
    _CORBA_String_helper::free(_ptr);
    _ptr = s;
    return *this;
  }

  inline _CORBA_String_member& operator= (const char* s) {
    _CORBA_String_helper::free(_ptr);
    if (s)
      _ptr = _CORBA_String_helper::dup(s);
    else
      _ptr = 0;
    return *this;
  }

  inline _CORBA_String_member& operator=(const _CORBA_String_member& s) {
    if (&s != this) {
      _CORBA_String_helper::free(_ptr);
      if (s._ptr && s._ptr != _CORBA_String_helper::empty_string)
	_ptr = _CORBA_String_helper::dup(s._ptr);
      else
	_ptr = s._ptr;
    }
    return *this;
  }

  inline _CORBA_String_member& operator=(const _CORBA_String_var& s) {
    _CORBA_String_helper::free(_ptr);
    if( (const char*)s ) {
      _ptr = _CORBA_String_helper::dup((const char*)s);
    }
    else {
      _ptr = (char*) 0;
    }
    return *this;
  }

  inline _CORBA_String_member& operator=(const _CORBA_String_element& s);

  inline char& operator[] (_CORBA_ULong index) {
    if (!_ptr || (_CORBA_ULong)strlen(_ptr) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return _ptr[index];
  }

  inline char operator[] (_CORBA_ULong index) const {
    if (!_ptr || (_CORBA_ULong)strlen(_ptr) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return _ptr[index];
  }

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator char* ()             { return _ptr; }
  inline operator const char* () const { return _ptr; }
#else
  inline operator char* () const { return _ptr; }
#endif

  inline const char* in() const { return _ptr; }
  inline char*& inout()         { return _ptr; }
  inline char*& out() {
    _CORBA_String_helper::free(_ptr);
    _ptr = 0;
    return _ptr;
  }

  inline char* _retn() {
    char *tmp;
    tmp = _ptr;
    _ptr = 0;
    return tmp;
  }

  void operator >>= (cdrStream& s) const;
  void operator <<= (cdrStream& s);

  inline char*& _NP_ref() {return _ptr;}

  char* _ptr;

};

//////////////////////////////////////////////////////////////////////
//////////////////////////// String_element ///////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_element {

public:

  inline _CORBA_String_element(char*& p, _CORBA_Boolean rel) 
    : pd_rel(rel), pd_data(p) {}

  inline _CORBA_String_element(const _CORBA_String_element& s) 
  : pd_rel(s.pd_rel), pd_data(s.pd_data) {}

  inline ~_CORBA_String_element() {
  // intentionally does nothing.
  }

  inline _CORBA_String_element& operator=(char* s) {
    if (pd_rel) 
      _CORBA_String_helper::free(pd_data);
    pd_data = s;
    return *this;
  }

  inline _CORBA_String_element& operator= (const char* s) {
    if (pd_rel) {
      _CORBA_String_helper::free(pd_data);
      if (s)
	pd_data = _CORBA_String_helper::dup(s);
      else
	pd_data = 0;
    } else {
      pd_data = (char*)s;
    }
    return *this;
  }

  inline _CORBA_String_element& operator=(const _CORBA_String_element& s) {
    if (&s != this) {
      if (pd_rel) {
	_CORBA_String_helper::free(pd_data);
	if (s.pd_data && s.pd_data != _CORBA_String_helper::empty_string)
	  pd_data = _CORBA_String_helper::dup(s.pd_data);
	else
	  pd_data = (char*)s.pd_data;
      } else
	pd_data = (char*)s.pd_data;
    }
    return *this;
  }

  inline _CORBA_String_element& operator=(const _CORBA_String_var& s) {
    if (pd_rel) {
      _CORBA_String_helper::free(pd_data);
      if( (const char*)s )
	pd_data = _CORBA_String_helper::dup((const char*)s);
      else
	pd_data = 0;
    } else
      pd_data = (char*)(const char*)s;
    return *this;
  }

  inline _CORBA_String_element& operator=(const _CORBA_String_member& s) {
    if (pd_rel) {
      _CORBA_String_helper::free(pd_data);
      if( (const char*)s && (const char*) s != _CORBA_String_helper::empty_string)
	pd_data = _CORBA_String_helper::dup((const char*)s);
      else
	pd_data = (char*)(const char*)s;
    } else
      pd_data = (char*)(const char*)s;
    return *this;
  }

  inline char& operator[] (_CORBA_ULong index) {
    if (!((char*)pd_data) || (_CORBA_ULong)strlen(pd_data) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return pd_data[index];
  }

  inline char operator[] (_CORBA_ULong index) const {
    if (!((char*)pd_data) || (_CORBA_ULong)strlen(pd_data) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return pd_data[index];
  }

#if ! (defined(__GNUG__) && __GNUC_MINOR__ == 95)
  inline operator char* ()             { return pd_data; }
  inline operator const char* () const { return pd_data; }
#else
  inline operator char* () const { return pd_data; }
#endif

  inline const char* in() const { return pd_data; }
  inline char*& inout()         { return pd_data; }
  inline char*& out() {
    if (pd_rel) {
      _CORBA_String_helper::free(pd_data);
      pd_data = 0;
    }
    else {
      pd_data = 0;
    }
    return pd_data;
  }
  inline char* _retn() {
    char *tmp;
    if (pd_rel) {
      tmp = pd_data;
      pd_data = 0;
    }
    else {
      tmp = (((char*)pd_data) ? _CORBA_String_helper::dup(pd_data) : 0);
      pd_data = 0;
    }
    return tmp;
  }


  inline char*& _NP_ref() const {return pd_data;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  char*&         pd_data;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// String_var   ////////////////////////////
//////////////////////////// String_member////////////////////////////
//////////////////////////// operator=    ////////////////////////////
//////////////////////////// copy ctors   ////////////////////////////
//////////////////////////////////////////////////////////////////////

inline _CORBA_String_var::_CORBA_String_var(const _CORBA_String_member& s)
{
  if ((const char*)s) {
    _data = _CORBA_String_helper::dup(s);
  }
  else
    _data = 0;
}

inline _CORBA_String_var::_CORBA_String_var(const _CORBA_String_element& s)
{
  if ((const char*)s) {
    _data = _CORBA_String_helper::dup(s);
  }
  else
    _data = 0;
}

inline _CORBA_String_var&
_CORBA_String_var::operator= (const _CORBA_String_member& s)
{
  _CORBA_String_helper::free(_data);
  if ((const char*)s) 
    _data = _CORBA_String_helper::dup(s);
  else
    _data = 0;
  return *this;
}

inline _CORBA_String_var&
_CORBA_String_var::operator= (const _CORBA_String_element& s)
{
  _CORBA_String_helper::free(_data);
  if ((const char*)s)
    _data = _CORBA_String_helper::dup(s);
  else
    _data = 0;
  return *this;
}

inline _CORBA_String_member& 
_CORBA_String_member::operator=(const _CORBA_String_element& s) {
  _CORBA_String_helper::free(_ptr);
  if( (const char*)s )
    _ptr = _CORBA_String_helper::dup((const char*)s);
  else
    _ptr = 0;
  return *this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// String_inout ////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_inout {
public:
  inline _CORBA_String_inout(char*& p) : _data(p) {}
  inline _CORBA_String_inout(_CORBA_String_var& p) : _data(p._data) {}
  inline _CORBA_String_inout(_CORBA_String_member& p) : _data(p._ptr) {}
  inline _CORBA_String_inout(_CORBA_String_element& p) : _data(p._NP_ref()) {
  // If the String_element has pd_rel == 0,
  // the string buffer is not owned by the sequence and should not
  // be freed. Since this is an inout argument and the callee may call
  // string_free, we create a copy to pass to the callee. This will result
  // in a memory leak! This only occurs when there is a programming error
  // and cannot be trapped by the compiler.
    if (!p._NP_release() && (p._NP_ref())) {
      p._NP_ref() = ((p._NP_ref()) ?
		     _CORBA_String_helper::dup(p._NP_ref()) : 0);
    }
  }
  inline ~_CORBA_String_inout() {}

  inline operator char*&()  { return _data; }

  char*& _data;

private:
  _CORBA_String_inout();
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// String_out /////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_out {
public:
  inline _CORBA_String_out(char*& p) : _data(p) { _data = 0; }
  inline _CORBA_String_out(_CORBA_String_var& p) : _data(p._data) { p = (char*)0; }
  inline _CORBA_String_out(_CORBA_String_member& p) : _data(p._ptr) { p = (char*)0; }
  inline _CORBA_String_out(_CORBA_String_element& p) : _data(p._NP_ref()) { p = (char*)0; }
  inline ~_CORBA_String_out() {}
  inline _CORBA_String_out(const _CORBA_String_out& p) : _data(p._data) {}
  inline _CORBA_String_out& operator=(const _CORBA_String_out& p) {
    _data = p._data; return *this;
  }
  inline _CORBA_String_out& operator=(char* p) { 
    _data = p; return *this;
  }
  inline _CORBA_String_out& operator=(const char* p) {
    _data = ((p) ? _CORBA_String_helper::dup(p) : 0); return *this;
  }

  operator char*& () { return _data; }
  char*& ptr()       { return _data; }

  char*& _data;

private:
  _CORBA_String_out();
  _CORBA_String_out& operator=(const _CORBA_String_var& );
  _CORBA_String_out& operator=(const _CORBA_String_member& );
  _CORBA_String_out& operator=(const _CORBA_String_element& );
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence_String           //////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Sequence_String
{
public:
  typedef _CORBA_String_element ElemT;
  typedef _CORBA_Sequence_String SeqT;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const  { return pd_len; }
  inline void length(_CORBA_ULong len) {
    if (pd_bounded && len > pd_max) {
      _CORBA_bound_check_error();
      // never reach here.
    }

    // If we've shrunk we need to clear the entries at the top.
    for( _CORBA_ULong i = len; i < pd_len; i++ ) 
      operator[](i) = (char*) _CORBA_String_helper::empty_string;

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

  static inline char** allocbuf(_CORBA_ULong nelems) {
    if (!nelems) return 0;
    char** b = new char*[nelems+2];
    ptr_arith_t l = nelems;
    b[0] = (char*) ((ptr_arith_t) 0x53515354U);
    b[1] = (char*) l;
    for (_CORBA_ULong index = 2; index < (nelems+2); index++)
      b[index] = (char*)_CORBA_String_helper::empty_string;
    return b+2;
  }

  static inline void freebuf(char** buf) {
    if (!buf) return;
    char** b = buf-2;
    if ((ptr_arith_t)b[0] != ((ptr_arith_t) 0x53515354U)) {
      _CORBA_bad_param_freebuf();
      return;
    }
    ptr_arith_t l = (ptr_arith_t) b[1];
    for (_CORBA_ULong i = 0; i < (_CORBA_ULong) l; i++) {
      _CORBA_String_helper::free(buf[i]);
    }
    b[0] = (char*) 0;
    delete [] b;
  }


  // CORBA 2.3 additions
  inline _CORBA_Boolean release() const { return pd_rel; }

  inline char** get_buffer(_CORBA_Boolean orphan = 0) {
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
	char** tmp = pd_data;
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

  inline const char* const* get_buffer() const { 
    if (pd_max && !pd_data) {
#ifdef HAS_Cplusplus_const_cast
      _CORBA_Sequence_String* s = const_cast<_CORBA_Sequence_String*>(this);
#else
      _CORBA_Sequence_String* s = (_CORBA_Sequence_String*)this;
#endif
      s->copybuffer(pd_max);
    }
#if !defined(__DECCXX) || (__DECCXX_VER > 60000000)
    return pd_data; 
#else
    return (char const* const*)pd_data; 
#endif
  }


  inline ~_CORBA_Sequence_String() { 
    if (pd_rel && pd_data) freebuf(pd_data);
    pd_data = 0;
  }

  // omniORB2 extensions
  void operator >>= (cdrStream& s) const;
  void operator <<= (cdrStream& s);

protected:
  inline _CORBA_Sequence_String()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_data(0) {}

  inline _CORBA_Sequence_String(_CORBA_ULong max,
				 _CORBA_Boolean bounded=0)
    : pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_data(0) {
  }

  inline _CORBA_Sequence_String(_CORBA_ULong   max,
				 _CORBA_ULong   len,
				 char**         value,
				 _CORBA_Boolean release = 0,
				 _CORBA_Boolean bounded = 0)
     : pd_max(max), pd_len(len), pd_rel(release),
       pd_bounded(bounded), pd_data(value)  { 
    if (len > max || (len && !value)) {
      _CORBA_bound_check_error();
      // never reach here
    }
  }

  inline _CORBA_Sequence_String(const SeqT& s)
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
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, char** data,
		      _CORBA_Boolean release = 0) {
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
    pd_rel = release;
  }


private:

  void copybuffer(_CORBA_ULong newmax) {
    // replace pd_data with a new buffer of size newmax.
    // Invariant:  pd_len <= newmax
    //
    char** newdata = allocbuf(newmax);
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
		      _CORBA_String_helper::dup(pd_data[i]) : 0);
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
  char**          pd_data;
};

//////////////////////////////////////////////////////////////////////
/////////////////// _CORBA_Bounded_Sequence_String //////////////////
//////////////////////////////////////////////////////////////////////

template<int max>
class _CORBA_Bounded_Sequence_String
  : public _CORBA_Sequence_String
{
public:
  typedef _CORBA_Bounded_Sequence_String<max> SeqT;

  inline _CORBA_Bounded_Sequence_String()
    : _CORBA_Sequence_String(max,1) {}

  inline _CORBA_Bounded_Sequence_String(_CORBA_ULong   length,
					 char**         value,
					 _CORBA_Boolean release = 0)
    : _CORBA_Sequence_String(max, length, value, release, 1) {}

  inline _CORBA_Bounded_Sequence_String(const SeqT& s)
    : _CORBA_Sequence_String(s) {}

  inline ~_CORBA_Bounded_Sequence_String() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence_String::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong len, char** data,
		      _CORBA_Boolean release = 0) {
    _CORBA_Sequence_String::replace(max,len,data,release);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence_String /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence_String
  : public _CORBA_Sequence_String
{
public:
  typedef _CORBA_Unbounded_Sequence_String SeqT;

  inline _CORBA_Unbounded_Sequence_String() {}

  inline _CORBA_Unbounded_Sequence_String(_CORBA_ULong max) :
         _CORBA_Sequence_String(max) {}

  inline _CORBA_Unbounded_Sequence_String(_CORBA_ULong   max,
					   _CORBA_ULong   length,
					   char**         value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Sequence_String(max, length, value, release) {}

  inline _CORBA_Unbounded_Sequence_String(const SeqT& s)
    : _CORBA_Sequence_String(s) {}

  inline ~_CORBA_Unbounded_Sequence_String() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence_String::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, char** data,
		      _CORBA_Boolean release = 0) {
    _CORBA_Sequence_String::replace(max,len,data,release);
  }

};


#endif  // __OMNI_STRINGTYPES_H__
