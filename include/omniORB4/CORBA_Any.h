// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Any.h                Created on: 2001/08/17
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
//    CORBA::Any
//

/*
  $Log$
  Revision 1.1.2.5  2003/01/16 12:47:08  dgrisby
  Const cast macro. Thanks Matej Kenda.

  Revision 1.1.2.4  2002/03/11 12:23:03  dpg1
  Tweaks to avoid compiler warnings.

  Revision 1.1.2.3  2002/01/09 11:37:46  dpg1
  Platform, constness fixes.

  Revision 1.1.2.2  2001/10/17 16:43:59  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.1  2001/08/17 13:39:44  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////////// Any ////////////////////////////////
//////////////////////////////////////////////////////////////////////

class Any {
public:
  Any();

  ~Any();

  Any(const Any& a);

  Any(TypeCode_ptr tc, void* value, Boolean release = 0);	

  // Marshalling operators
  void operator>>= (cdrStream& s) const;
  void operator<<= (cdrStream& s);

  void* NP_pd() { return pd_ptr; }
  void* NP_pd() const { return pd_ptr; }

  // omniORB data-only marshalling functions
  void NP_marshalDataOnly(cdrStream& s) const;
  void NP_unmarshalDataOnly(cdrStream& s);


  // omniORB internal stub support routines
  void PR_packFrom(TypeCode_ptr newtc, void* tcdesc);
  Boolean PR_unpackTo(TypeCode_ptr tc, void* tcdesc) const;

  void *PR_getCachedData() const;
  void PR_setCachedData(void *data, void(*destructor)(void *));

  // OMG Insertion operators
  Any& operator=(const Any& a);

  void operator<<=(Short s);
  void operator<<=(UShort u);
  void operator<<=(Long l);
  void operator<<=(ULong u);
#ifdef HAS_LongLong
  void operator<<=(LongLong  l);
  void operator<<=(ULongLong u);
#endif
#if !defined(NO_FLOAT)
  void operator<<=(Float f);
  void operator<<=(Double d);
#ifdef HAS_LongDouble
  void operator<<=(LongDouble l);
#endif
#endif
  void operator<<=(const Any& a);   // copying
  void operator<<=(Any* a);         // non-copying
  void operator<<=(TypeCode_ptr tc);
  void operator<<=(Object_ptr obj);
  void operator<<=(const char* s);	
  void operator<<=(const WChar* s);

  struct from_boolean {
    from_boolean(Boolean b) : val(b) {}
    Boolean val;
  };
  struct from_octet {
    from_octet(Octet b) : val(b) {}
    Octet val;
  };
  struct from_char {
    from_char(Char b) : val(b) {}
    Char val;
  };
  struct from_wchar {
    from_wchar(WChar b) : val(b) {}
    WChar val;
  };
  struct from_string {
    from_string(const char* s, ULong b, Boolean nocopy = 0)
      : val(OMNI_CONST_CAST(char*, s)), bound(b), nc(nocopy) { }
    from_string(char* s, ULong b, Boolean nocopy = 0)
      : val(s), bound(b), nc(nocopy) { }   // deprecated

    char* val;
    ULong bound;
    Boolean nc;
  };
  struct from_wstring {
    from_wstring(const WChar* s, ULong b, Boolean nocopy = 0)
      : val(OMNI_CONST_CAST(WChar*, s)), bound(b), nc(nocopy) { }
    from_wstring(WChar* s, ULong b, Boolean nocopy = 0)
      : val(s), bound(b), nc(nocopy) { }   // deprecated

    WChar* val;
    ULong bound;
    Boolean nc;
  };
  struct from_fixed {
    from_fixed(const Fixed& f, UShort d, UShort s)
      : val(f), digits(d), scale(s) {}

    const Fixed& val;
    UShort       digits;
    UShort       scale;
  };

  void operator<<=(from_boolean f);
  void operator<<=(from_char c);
  void operator<<=(from_wchar wc);
  void operator<<=(from_octet o);
  void operator<<=(from_string s);
  void operator<<=(from_wstring s);
  void operator<<=(from_fixed f);

  // OMG Extraction operators
  Boolean operator>>=(Short& s) const;
  Boolean operator>>=(UShort& u) const;
  Boolean operator>>=(Long& l) const;
  Boolean operator>>=(ULong& u) const;
#ifdef HAS_LongLong
  Boolean operator>>=(LongLong&  l) const;
  Boolean operator>>=(ULongLong& u) const;
#endif
#if !defined(NO_FLOAT)
  Boolean operator>>=(Float& f) const;
  Boolean operator>>=(Double& d) const;
#ifdef HAS_LongDouble
  Boolean operator>>=(LongDouble& l) const;
#endif
#endif
  Boolean operator>>=(const Any*& a) const;
  Boolean operator>>=(Any*& a) const;  // deprecated
  Boolean operator>>=(Any& a) const;  // pre CORBA-2.3, obsoluted do not use.
  Boolean operator>>=(TypeCode_ptr& tc) const;
  Boolean operator>>=(Object_ptr& obj) const;
  Boolean operator>>=(const char*& s) const;
#ifndef _NO_ANY_STRING_EXTRACTION_
  Boolean operator>>=(char*& s) const; // deprecated
#endif
  Boolean operator>>=(const WChar*& s) const;

  struct to_boolean {
    to_boolean(Boolean& b) : ref(b) {}
    Boolean& ref;
  };
  struct to_char {
    to_char(Char& b) : ref(b) {}
    Char& ref;
  };
  struct to_wchar {
    to_wchar(WChar& b) : ref(b) {}
    WChar& ref;
  };
  struct to_octet {
    to_octet(Octet& b) : ref(b) {}
    Octet& ref;
  };
  struct to_string {
    to_string(const char*& s, ULong b) : val((char*&)s), bound(b) { }
    to_string(char*& s, ULong b) : val(s), bound(b) { } // deprecated

    char*& val;
    ULong bound;
  };
  struct to_wstring {
    to_wstring(const WChar*& s, ULong b) : val((WChar*&)s), bound(b) { }
    to_wstring(WChar*& s, ULong b) : val(s), bound(b) { } // deprecated

    WChar*& val;
    ULong bound;
  };
  struct to_fixed {
    to_fixed(Fixed& f, UShort d, UShort s)
      : val(f), digits(d), scale(s) {}

    Fixed& val;
    UShort digits;
    UShort scale;
  };
  struct to_object {
    to_object(Object_out obj) : ref(obj._data) { }
    Object_ptr& ref;
  };

  Boolean operator>>=(to_boolean b) const;
  Boolean operator>>=(to_char c) const;
  Boolean operator>>=(to_wchar wc) const;
  Boolean operator>>=(to_octet o) const;
#ifndef _NO_ANY_STRING_EXTRACTION_
  Boolean operator>>=(to_string s) const;
#endif
  Boolean operator>>=(to_wstring s) const;
  Boolean operator>>=(to_fixed f) const;
  Boolean operator>>=(to_object o) const;
  Boolean operator>>=(const CORBA::SystemException*& e) const;

  void replace(TypeCode_ptr TCp, void* value, Boolean release = 0);

  TypeCode_ptr type() const;
  void type(TypeCode_ptr);

  const void* value() const;

private:
  void operator<<=(unsigned char);
  Boolean operator>>=(unsigned char&) const;
  // Not implemented

  void *pd_ptr;
};
