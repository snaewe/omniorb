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
 Revision 1.5.2.3  2000/02/09 14:41:37  djr
 String members now initialised to zero again by default.

 Revision 1.5.2.2  2000/01/31 11:05:58  djr
 Fixed bug in unmarshalling of String_member.

 Revision 1.5.2.1  2000/01/27 15:05:18  djr
 String_member now initialised to empty string by default.

 Revision 1.5  1999/08/30 17:14:59  sll
 Added workaround for gcc-2.95 in the conversion operators for string_var
 and string_member.

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
class _CORBA_String_inout;
class _CORBA_String_out;

//////////////////////////////////////////////////////////////////////
///////////////////////////// String_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_var {
public:
  typedef char* ptr_t;

  static inline char* string_dup(const char* p) {
    char* r = new char[strlen(p) + 1];
    strcpy(r, p);
    return r;
  }

  inline _CORBA_String_var()        { _data = 0; }
  inline _CORBA_String_var(char *p) { _data = p; }

  inline _CORBA_String_var(const char* p) {
    if( p ) _data = string_dup(p);
    else    _data = 0;
  }

  inline _CORBA_String_var(const _CORBA_String_var& s) {
    if( (const char *)s )  _data = string_dup(s);
    else                   _data = 0;
  }

  inline _CORBA_String_var(const _CORBA_String_member& s);

  inline ~_CORBA_String_var() {
    if( _data )  delete[] _data;
  }

  inline _CORBA_String_var& operator=(char* p) {
    if (_data)  delete[] _data;
    _data = p;
    return *this;
  }

  inline _CORBA_String_var& operator=(const char* p) {
    if (_data){
      delete[] _data;
      _data = 0;
    }
    if (p)  _data = string_dup(p);
    return *this;
  }

  inline _CORBA_String_var& operator=(const _CORBA_String_var& s) {
    if (_data){
      delete[] _data;
      _data = 0;
    }
    if( (const char*)s )  _data = string_dup(s);
    return *this;
  }

  inline _CORBA_String_var& operator=(const _CORBA_String_member& s);

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
      delete[] _data;
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
#ifndef OMNI_EMPTY_STRING_MEMBERS
    : pd_data(0), pd_rel(1), _ptr(pd_data) {}
#else
    : pd_data(_CORBA_String_var::string_dup("")), pd_rel(1), _ptr(pd_data) {}
#endif

  inline _CORBA_String_member(char*& p, _CORBA_Boolean rel) 
    : pd_data(0), pd_rel(rel), _ptr(p) {}

  inline _CORBA_String_member(const _CORBA_String_member& s) 
           : pd_data(0), pd_rel(1), _ptr(pd_data) {
    if (s._ptr)  _ptr = _CORBA_String_var::string_dup(s._ptr);
  }

  inline ~_CORBA_String_member() {
    if (pd_rel && pd_data)  delete[] pd_data; // Not _ptr! Only call delete
                                              // when this is not a 
                                              // sequence member.
  }

  inline _CORBA_String_member& operator=(char* s) {
    if (pd_rel && ((char*)_ptr))  delete[] _ptr;
    _ptr = s;
    pd_rel = 1;
    return *this;
  }

  inline _CORBA_String_member& operator= (const char* s) {
    if (pd_rel && ((char*)_ptr)) {
      delete[] _ptr;
      _ptr = 0;
    }
    if( s ) {
      _ptr = _CORBA_String_var::string_dup(s);
      pd_rel = 1;
    }
    return *this;
  }

  inline _CORBA_String_member& operator=(const _CORBA_String_member& s) {
    if (pd_rel && ((char*)_ptr)) {
      delete[] _ptr;
      _ptr = 0;
    }
    if( s._ptr ) {
      _ptr = _CORBA_String_var::string_dup(s._ptr);
      pd_rel = 1;
    }
    return *this;
  }

  inline _CORBA_String_member& operator=(const _CORBA_String_var& s) {
    if (pd_rel && ((char*)_ptr)) {
      delete[] _ptr;
      _ptr = 0;
    }
    if( (const char*)s ) {
      _ptr = _CORBA_String_var::string_dup((const char*)s);
      pd_rel = 1;
    }
    return *this;
  }

  inline char& operator[] (_CORBA_ULong index) {
    if (!((char*)_ptr) || (_CORBA_ULong)strlen(_ptr) < index) {
      _CORBA_bound_check_error();	// never return
    }
    return _ptr[index];
  }

  inline char operator[] (_CORBA_ULong index) const {
    if (!((char*)_ptr) || (_CORBA_ULong)strlen(_ptr) < index) {
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

  inline char* _retn() {
    char *tmp;
    if (pd_rel) {
      tmp = _ptr;
      _ptr = 0;
    }
    else {
      tmp = _CORBA_String_var::string_dup(_ptr);
      _ptr = 0;
    }
    return tmp;
  }

  void operator >>= (NetBufferedStream& s) const;
  void operator <<= (NetBufferedStream& s);
  void operator >>= (MemBufferedStream& s) const;
  void operator <<= (MemBufferedStream& s);
  size_t NP_alignedSize(size_t initialoffset) const;

private:
  char* pd_data;

public:
  _CORBA_Boolean pd_rel;
  char*&         _ptr;
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
    _data = _CORBA_String_var::string_dup(s);
  }
  else
    _data = 0;
}

inline _CORBA_String_var&
_CORBA_String_var::operator= (const _CORBA_String_member& s)
{
  if (_data) {
    delete [] _data;
    _data = 0;
  }
  if ((const char*)s) _data = _CORBA_String_var::string_dup(s);
  return *this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// String_inout ////////////////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_String_inout {
public:
  inline _CORBA_String_inout(char*& p) : _data(p) {}
  inline _CORBA_String_inout(_CORBA_String_var& p) : _data(p._data) {}
  inline _CORBA_String_inout(_CORBA_String_member& p) : _data(p._ptr) {
  // If the String_member is part of a sequence and the pd_rel == 0,
  // the string buffer is not owned by the sequence and should not
  // be freed. Since this is an inout argument and the callee may call
  // string_free, we create a copy to pass to the callee. This will result
  // in a memory leak! This only occurs when there is a programming error
  // and cannot be trapped by the compiler.
    if (!p.pd_rel && (p._ptr)) {
      p._ptr = _CORBA_String_var::string_dup(p._ptr);
    }
  }
  inline ~_CORBA_String_inout() {}

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
  inline ~_CORBA_String_out() {}
  inline _CORBA_String_out(const _CORBA_String_out& p) : _data(p._data) {}
  inline _CORBA_String_out& operator=(const _CORBA_String_out& p) {
    _data = p._data; return *this;
  }
  inline _CORBA_String_out& operator=(char* p) { 
    _data = p; return *this;
  }
  inline _CORBA_String_out& operator=(const char* p) {
    _data = _CORBA_String_var::string_dup(p); return *this;
  }
  operator char*& () { return _data; }
  char*& ptr() { return _data; }

  char*& _data;

private:
  _CORBA_String_out();
  _CORBA_String_out& operator=(const _CORBA_String_var& );
  _CORBA_String_out& operator=(const _CORBA_String_member& );
};


//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Sequence__String           /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Sequence__String
{
public:
  typedef _CORBA_String_member ElemT;
  typedef _CORBA_Sequence__String SeqT;

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const  { return pd_len; }
  inline void length(_CORBA_ULong len) {
    if (pd_bounded && len > pd_max) {
      _CORBA_bound_check_error();
      // never reach here.
    }

    // If we've shrunk we need to clear the entries at the top.
    for( _CORBA_ULong i = len; i < pd_len; i++ )  operator[](i) = (char*) 0;

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

  static inline char** allocbuf(_CORBA_ULong nelems) {
    if (!nelems) return 0;
    char** b = new char*[nelems+2];
    omni::ptr_arith_t l = nelems;
    memset(b,0,sizeof(*b)*(nelems+2));
    b[0] = (char*) ((omni::ptr_arith_t) 0x53515354U);
    b[1] = (char*) l;
    return b+2;
  }

  static inline void freebuf(char** buf) {
    if (!buf) return;
    char** b = buf-2;
    if ((omni::ptr_arith_t)b[0] != ((omni::ptr_arith_t) 0x53515354U)) {
      _CORBA_bad_param_freebuf();
      return;
    }
    omni::ptr_arith_t l = (omni::ptr_arith_t) b[1];
    for (_CORBA_ULong i = 0; i < (_CORBA_ULong) l; i++) {
      if (buf[i])
	delete [] (buf[i]);
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
      _CORBA_Sequence__String* s = const_cast<_CORBA_Sequence__String*>(this);
#else
      _CORBA_Sequence__String* s = (_CORBA_Sequence__String*)this;
#endif
      s->copybuffer(pd_max);
    }
    return pd_data; 
  }


  inline ~_CORBA_Sequence__String() { 
    if (pd_rel && pd_data) freebuf(pd_data);
    pd_data = 0;
  }

  // omniORB2 extensions
  size_t NP_alignedSize(size_t initialoffset) const;
  void operator >>= (NetBufferedStream& s) const;
  void operator <<= (NetBufferedStream& s);
  void operator >>= (MemBufferedStream& s) const;
  void operator <<= (MemBufferedStream& s);

protected:
  inline _CORBA_Sequence__String()
    : pd_max(0), pd_len(0), pd_rel(1), pd_bounded(0), pd_data(0) {}

  inline _CORBA_Sequence__String(_CORBA_ULong max,
				 _CORBA_Boolean bounded=0)
    : pd_max(max), pd_len(0), pd_rel(1), pd_bounded(bounded), pd_data(0) {
  }

  inline _CORBA_Sequence__String(_CORBA_ULong   max,
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

  inline _CORBA_Sequence__String(const SeqT& s)
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
	newdata[i] = _CORBA_String_var::string_dup(pd_data[i]);
      }
    }
    if (pd_rel && pd_data) {
      freebuf(pd_data);
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
/////////////////// _CORBA_Bounded_Sequence__String //////////////////
//////////////////////////////////////////////////////////////////////

template<int max>
class _CORBA_Bounded_Sequence__String
  : public _CORBA_Sequence__String
{
public:
  typedef _CORBA_Bounded_Sequence__String<max> SeqT;

  inline _CORBA_Bounded_Sequence__String()
    : _CORBA_Sequence__String(max,1) {}

  inline _CORBA_Bounded_Sequence__String(_CORBA_ULong   length,
					 char**         value,
					 _CORBA_Boolean release = 0)
    : _CORBA_Sequence__String(max, length, value, release, 1) {}

  inline _CORBA_Bounded_Sequence__String(const SeqT& s)
    : _CORBA_Sequence__String(s) {}

  inline ~_CORBA_Bounded_Sequence__String() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence__String::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong len, char** data,
		      _CORBA_Boolean release = 0) {
    _CORBA_Sequence__String::replace(max,len,data,release);
  }
};

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence__String /////////////////
//////////////////////////////////////////////////////////////////////

class _CORBA_Unbounded_Sequence__String
  : public _CORBA_Sequence__String
{
public:
  typedef _CORBA_Unbounded_Sequence__String SeqT;

  inline _CORBA_Unbounded_Sequence__String() {}

  inline _CORBA_Unbounded_Sequence__String(_CORBA_ULong max) :
         _CORBA_Sequence__String(max) {}

  inline _CORBA_Unbounded_Sequence__String(_CORBA_ULong   max,
					   _CORBA_ULong   length,
					   char**         value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Sequence__String(max, length, value, release) {}

  inline _CORBA_Unbounded_Sequence__String(const SeqT& s)
    : _CORBA_Sequence__String(s) {}

  inline ~_CORBA_Unbounded_Sequence__String() {}

  inline SeqT& operator = (const SeqT& s) {
    _CORBA_Sequence__String::operator = (s);
    return *this;
  }

  // CORBA 2.3 additions
  inline void replace(_CORBA_ULong max, _CORBA_ULong len, char** data,
		      _CORBA_Boolean release = 0) {
    _CORBA_Sequence__String::replace(max,len,data,release);
  }

};


#endif  // __OMNI_STRINGTYPES_H__
