// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Any.h                Created on: 2001/08/17
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2004 Apasphere Ltd.
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
  Revision 1.1.4.5  2006/11/02 17:33:08  dgrisby
  Convert some casts to use const_cast<> where available.

  Revision 1.1.4.4  2004/10/13 17:58:18  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

  Revision 1.1.4.3  2004/07/23 10:29:56  dgrisby
  Completely new, much simpler Any implementation.

  Revision 1.1.4.2  2004/02/16 10:10:28  dgrisby
  More valuetype, including value boxes. C++ mapping updates.

  Revision 1.1.4.1  2003/03/23 21:04:25  dgrisby
  Start of omniORB 4.1.x development branch.

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

  // Marshalling operators
  void operator>>= (cdrStream& s) const;
  void operator<<= (cdrStream& s);

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
  void operator<<=(const Any& a);     // copying
  void operator<<=(Any* a);           // non-copying
  void operator<<=(TypeCode_ptr tc);  // copying
  void operator<<=(TypeCode_ptr* tc); // non-copying
  void operator<<=(Object_ptr obj);   // copying
  void operator<<=(Object_ptr* obj);  // non-copying
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
  Boolean operator>>=(Any& a) const;  // pre CORBA-2.3, obsolete; do not use.
  Boolean operator>>=(TypeCode_ptr& tc) const;
  Boolean operator>>=(Object_ptr& obj) const;
  Boolean operator>>=(const char*& s) const;
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
    to_string(const char*& s, ULong b) :
      val(OMNI_CONST_CAST(char*&,s)), bound(b) { }

    to_string(char*& s, ULong b) : val(s), bound(b) { } // deprecated

    char*& val;
    ULong bound;
  };
  struct to_wstring {
    to_wstring(const WChar*& s, ULong b) :
      val(OMNI_CONST_CAST(WChar*&,s)), bound(b) { }

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
  struct to_abstract_base {
    to_abstract_base(AbstractBase_ptr& base) : ref(base) {}
    AbstractBase_ptr& ref;
  };
  struct to_value {
    to_value(ValueBase*& base) : ref(base) {}
    ValueBase*& ref;
  };

  Boolean operator>>=(to_boolean b) const;
  Boolean operator>>=(to_char c) const;
  Boolean operator>>=(to_wchar wc) const;
  Boolean operator>>=(to_octet o) const;
  Boolean operator>>=(to_string s) const;
  Boolean operator>>=(to_wstring s) const;
  Boolean operator>>=(to_fixed f) const;
  Boolean operator>>=(to_object o) const;
  Boolean operator>>=(to_abstract_base a) const;
  Boolean operator>>=(to_value v) const;
  Boolean operator>>=(const CORBA::SystemException*& e) const;

  TypeCode_ptr type() const;
  void type(TypeCode_ptr);


  //
  // Deprecated non-typesafe functions. Do not use.
  //

  Any(TypeCode_ptr tc, void* value, Boolean release = 0);
  void replace(TypeCode_ptr TCp, void* value, Boolean release = 0);
  const void* value() const;


  //
  // omniORB data-only marshalling functions
  //

  void NP_marshalDataOnly(cdrStream& s) const;
  void NP_unmarshalDataOnly(cdrStream& s);

  //
  // omniORB internal stub support routines
  //

  // Functions provided by the stubs
  typedef void(*pr_marshal_fn)   (cdrStream&, void*);
  typedef void(*pr_unmarshal_fn) (cdrStream&, void*&);
  typedef void(*pr_destructor_fn)(void*);
  
  inline Boolean PR_equivalent(TypeCode_ptr tc);
  // True if the given TypeCode is equivalent to the Any's TypeCode.

  void PR_insert(TypeCode_ptr newtc, pr_marshal_fn marshal, void* data);
  // Insert data into the Any's buffer. Used for simple types.

  void PR_insert(TypeCode_ptr newtc, pr_marshal_fn marshal,
		 pr_destructor_fn destructor, void* data);
  // Set data pointer, to be possibly marshalled later. Deallocates
  // memory buffer and/or data pointer if necessary.

  Boolean PR_extract(TypeCode_ptr     tc,
		     pr_unmarshal_fn  unmarshal,
		     void*            data) const;
  // Extract a simple type from the Any's memory buffer. data is a
  // pointer to the place in memory to unmarshal to.

  Boolean PR_extract(TypeCode_ptr     tc,
		     pr_unmarshal_fn  unmarshal,
		     pr_marshal_fn    marshal,
		     pr_destructor_fn destructor,
		     void*&           data) const;
  // Extract data from the Any if the TypeCode matches. If the data is
  // already in unmarshalled form, just sets the data pointer to point
  // to the existing data. Otherwise, unmarshals the value from the
  // memory buffer and caches it in the Any for future extractions.
  //
  // Function is marked const to satisfy standard extraction
  // interfaces, but it can actually modify the Any.


  cdrAnyMemoryStream& PR_streamToRead() const;
  // Pack Any into a memory stream if necessary, and return it.

  cdrAnyMemoryStream& PR_streamToWrite();
  // Clear the contents and allocate a memory stream for writing into.


  void PR_clearData();
  // Clear the contents ready to insert a different value.

private:
  void operator<<=(unsigned char);
  Boolean operator>>=(unsigned char&) const;
  // Not implemented.

  TypeCode_member pd_tc;

  // The Any contents can be stored in marshalled form, or as a
  // pointer to the unmarshalled data, or both. When stored as a
  // pointer to the data, the pd_marshal and pd_destructor members are
  // set. Once one of the pointers has been set, it is never unset
  // until the Any is cleared or destroyed.
  cdrAnyMemoryStream* pd_mbuf;
  void*               pd_data;
  pr_marshal_fn       pd_marshal;
  pr_destructor_fn    pd_destructor;
};
