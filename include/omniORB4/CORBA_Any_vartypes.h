// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Any_vartypes.h       Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::Any_var, etc.
//

/*
  $Log$
  Revision 1.1.4.3  2004/10/13 17:58:18  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

  Revision 1.1.4.2  2004/07/23 10:29:56  dgrisby
  Completely new, much simpler Any implementation.

  Revision 1.1.4.1  2003/03/23 21:04:25  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.2  2001/10/17 16:43:59  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.1  2001/08/17 13:39:44  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

class Any_OUT_arg;

class Any_var {
public:
  inline Any_var() { pd_data = 0; }
  inline Any_var(Any* p) { pd_data = p; }
  inline Any_var(const Any_var& p) {
    if (!p.pd_data) {
      pd_data = 0;
      return;
    }
    else {
      pd_data = new Any;
      if (!pd_data) {
	_CORBA_new_operator_return_null();
	// never reach here
      }
      *pd_data = *p.pd_data;
    }
  }

  inline ~Any_var() {  if (pd_data) delete pd_data; }
  inline Any_var& operator= (Any* p) {
    if (pd_data) delete pd_data;
    pd_data = p;
    return *this;
  }

  inline Any_var& operator= (const Any_var& p) {
    if (p.pd_data) {
      if (!pd_data) {
	pd_data = new Any;
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

  inline Any* operator->() const { return (Any*)pd_data; }

  //#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
#if defined(__GNUG__)
  inline operator Any& () const { return (Any&) *pd_data; }
#else
  inline operator const Any& () const { return *pd_data; }
  inline operator Any& () { return *pd_data; }
#endif

  inline const Any& in() const { return *pd_data; }
  inline Any&       inout()    { return *pd_data; }
  inline Any*& out() {
    if( pd_data ){
      delete pd_data;
      pd_data = 0;
    }
    return pd_data;
  }
  inline Any* _retn() {
    Any* tmp = pd_data;
    pd_data = 0;
    return tmp;
  }

  // Any member-function insertion operators:

  inline void operator<<=(Short s) {
    *pd_data <<= s;
  }
  inline void operator<<=(UShort u) {
    *pd_data <<= u;
  }	
  inline void operator<<=(Long l) {
    *pd_data <<= l;
  }
  inline void operator<<=(ULong u) {
    *pd_data <<= u;
  }
#ifdef HAS_LongLong
  inline void operator<<=(LongLong l) {
    *pd_data <<= l;
  }
  inline void operator<<=(ULongLong u) {
    *pd_data <<= u;
  }
#endif
#if !defined(NO_FLOAT)
  inline void operator<<=(Float f) {
    *pd_data <<= f;
  }
  inline void operator<<=(Double d) {
    *pd_data <<= d;
  }
#ifdef HAS_LongDouble
  inline void operator<<=(LongDouble d) {
    *pd_data <<= d;
  }
#endif
#endif
  inline void operator<<=(const Any& a) {
    *pd_data <<= a;
  }	
  inline void operator<<=(Any* a) {
    *pd_data <<= a;
  }
  inline void operator<<=(TypeCode_ptr tc) {
    *pd_data <<= tc;
  }
  inline void operator<<=(Object_ptr obj) {
    *pd_data <<= obj;
  }
  inline void operator<<=(const char* s) {
    *pd_data <<= s;
  }
  inline void operator<<=(const WChar* s) {
    *pd_data <<= s;
  }
  inline void operator<<=(Any::from_boolean f) {
    *pd_data <<= f;
  }
  inline void operator<<=(Any::from_char c) {
    *pd_data <<= c;
  }
  inline void operator<<=(Any::from_wchar c) {
    *pd_data <<= c;
  }
  inline void operator<<=(Any::from_octet o) {
    *pd_data <<= o;
  }
  inline void operator<<=(Any::from_string s) {
    *pd_data <<= s;
  }
  inline void operator<<=(Any::from_wstring s) {
    *pd_data <<= s;
  }
  inline void operator<<=(Any::from_fixed f) {
    *pd_data <<= f;
  }

  // Any member-function extraction operators:
  inline Boolean operator>>=(Short& s) const {
    return (*pd_data >>= s);
  }
  inline Boolean operator>>=(UShort& u) const {
    return (*pd_data >>= u);
  }
  inline Boolean operator>>=(Long& l) const {
    return (*pd_data >>= l);
  }
  inline Boolean operator>>=(ULong& u) const {
    return (*pd_data >>= u);
  }
#ifdef HAS_LongLong
  inline Boolean operator>>=(LongLong& l) const {
    return (*pd_data >>= l);
  }
  inline Boolean operator>>=(ULongLong& u) const {
    return (*pd_data >>= u);
  }
#endif
#if !defined(NO_FLOAT)
  inline Boolean operator>>=(Float& f) const {
    return (*pd_data >>= f);
  }
  inline Boolean operator>>=(Double& d) const {
    return (*pd_data >>= d);
  }
#ifdef HAS_LongDouble
  inline Boolean operator>>=(LongDouble& d) const {
    return (*pd_data >>= d);
  }
#endif
#endif
  inline Boolean operator>>=(const Any*& a) const {
    return (*pd_data >>= a);
  }
  inline Boolean operator>>=(Any*& a) const {
    return (*pd_data >>= a);
  }
  Boolean operator>>=(Any& a) const {  // pre CORBA-2.3, obsoluted do not use
    return (*pd_data >>= a);
  }
  inline Boolean operator>>=(TypeCode_ptr& tc) const {
    return (*pd_data >>= tc);
  }
  inline Boolean operator>>=(const char*& s) const {
    return (*pd_data >>= s);
  }
  inline Boolean operator>>=(Object_ptr& obj) const {
    return (*pd_data >>= obj);
  }
  inline Boolean operator>>=(const WChar*& s) const {
    return (*pd_data >>= s);
  }
  inline Boolean operator>>=(Any::to_boolean b) const {
    return (*pd_data >>= b);
  }
  inline Boolean operator>>=(Any::to_char c) const {
    return (*pd_data >>= c);
  }
  inline Boolean operator>>=(Any::to_wchar c) const {
    return (*pd_data >>= c);
  }
  inline Boolean operator>>=(Any::to_octet o) const {
    return (*pd_data >>= o);
  }
  inline Boolean operator>>=(Any::to_string s) const {
    return (*pd_data >>= s);
  }
  inline Boolean operator>>=(Any::to_wstring s) const {
    return (*pd_data >>= s);
  }
  inline Boolean operator>>=(Any::to_fixed f) const {
    return (*pd_data >>= f);
  }
  inline Boolean operator>>=(Any::to_object o) const {
    return (*pd_data >>= o);
  }
  inline Boolean operator>>=(Any::to_abstract_base a) const {
    return (*pd_data >>= a);
  }
  inline Boolean operator>>=(Any::to_value v) const {
    return (*pd_data >>= v);
  }
  inline Boolean operator>>=(const CORBA::SystemException*& e) const {
    return (*pd_data >>= e);
  }

  friend class Any_OUT_arg;

private:
  Any* pd_data;
};


class Any_OUT_arg {
public:
  inline Any_OUT_arg(Any*& p) : _data(p) { _data = (Any*) 0; }
  inline Any_OUT_arg(Any_var& p) : _data(p.pd_data) {
    p = (Any*)0;
  }
  inline Any_OUT_arg(const Any_OUT_arg& p) : _data(p._data) {}
  inline Any_OUT_arg& operator=(const Any_OUT_arg& p) {
    _data = p._data; return *this;
  }
  inline Any_OUT_arg& operator=(Any* p) { _data = p; return *this; }

  operator Any*& () { return _data; }
  Any*& ptr() { return _data; }

  Any*& _data;
private:
  Any_OUT_arg();
  Any_OUT_arg& operator=(const Any_var&);
};

typedef Any_OUT_arg Any_out;
