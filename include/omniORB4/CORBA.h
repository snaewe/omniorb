// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA.h                    Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    A complete set of C++ definitions for the CORBA module.
//

/*
 $Log$
 Revision 1.2.2.12  2001/04/09 15:18:46  dpg1
 Tweak fixed point to make life easier for omniORBpy.

 Revision 1.2.2.11  2001/03/13 10:32:04  dpg1
 Fixed point support.

 Revision 1.2.2.10  2001/01/08 12:35:41  dpg1
 _duplicate in CORBA::Object::_narrow

 Revision 1.2.2.9  2000/11/20 14:40:03  sll
 Added TypeCode::PR_wstring_tc(CORBA::ULong bound).

 Revision 1.2.2.8  2000/11/17 19:09:36  dpg1
 Support codeset conversion in any.

 Revision 1.2.2.7  2000/11/15 17:03:14  sll
 Moved include codeSets.h to omniInternal.h

 Revision 1.2.2.6  2000/11/09 12:27:48  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.2.2.5  2000/11/07 18:18:47  sll
 Added external guard in CORBA.h to ensure that any idl that include
 CosNaming.idl will compile as expected.

 Revision 1.2.2.4  2000/11/03 18:58:46  sll
 Unbounded sequence of octet got a new type name.

 Revision 1.2.2.3  2000/10/27 15:42:02  dpg1
 Initial code set conversion support. Not yet enabled or fully tested.

 Revision 1.2.2.2  2000/09/27 17:19:12  sll
 Updated to use the new cdrStream abstraction.
 Replace include/omniORB3 with include/omniORB4.

 Revision 1.2.2.1  2000/07/17 10:35:32  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:06  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.9  2000/07/10 13:06:44  dpg1
 Initialisation of Any insertion functions was missing from system
 exceptions.

 Revision 1.1.2.8  2000/06/27 16:15:07  sll
 New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
 _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
 sequence of string and a sequence of object reference.

 Revision 1.1.2.7  2000/06/02 14:15:17  dpg1
 SystemException constructors now public so exceptions caught by base
 class can be rethrown

 Revision 1.1.2.6  2000/03/03 14:29:15  djr
 Improvement to BOA skeletons (less generated code).

 Revision 1.1.2.5  1999/10/21 11:04:59  djr
 Added _core_attr to declarations of _PD_repoId in interfaces.

 Revision 1.1.2.4  1999/10/18 17:28:17  djr
 Fixes for building MSVC dlls.

 Revision 1.1.2.3  1999/10/18 11:27:36  djr
 Centralised list of system exceptions.

 Revision 1.1.2.2  1999/10/16 13:22:51  djr
 Changes to support compiling on MSVC.

 Revision 1.1.2.1  1999/09/24 09:51:36  djr
 Moved from omniORB2 + some new files.

 Revision 1.46  1999/08/30 18:45:58  sll
 Made #include ir stubs conditional on ENABLE_CLIENT_IR_SUPPORT.
 Application code has to define ENABLE_CLIENT_IR_SUPPORT in order to act
 as a client for an interface repository.

 Revision 1.45  1999/08/30 17:40:22  sll
 Removed use of _T and _T_var.

 Revision 1.44  1999/08/24 12:36:44  djr
 PR_structMember now uses 'const char*'.

 Revision 1.43  1999/08/16 19:33:24  sll
 New method NP_destroy() in class CORBA::ORB.

 Revision 1.42  1999/08/15 13:51:33  sll
 Define ImplmentationDef as an empty class to keep some compilers happy.

 Revision 1.41  1999/06/27 16:44:42  sll
 enclose Any extraction operator for string with test for macro
 _NO_ANY_STRING_EXTRACTION_. Define this macro as compiler option would
 remove the operators from the declaration. This make it easier to
 find the code in the source where the operators are used. Hence make it
 easier to fix old code which uses the pre-2.8.0 semantics.

 Revision 1.40  1999/06/25 13:54:17  sll
 Removed Any::operator>>=(Object_ptr&) const.

 Revision 1.39  1999/06/22 15:04:29  sll
 Put back pre CORBA-2.3 operator>>=(Any&a) in Any_var.

 Revision 1.38  1999/06/18 20:32:59  sll
 Updated to CORBA 2.3 mapping.

 Revision 1.37  1999/06/03 17:10:32  sll
 Added T_out types and updated T_var types to CORBA 2.2

 Revision 1.36  1999/06/02 16:07:44  sll
 Enabled IR client support for all platforms. Previously, IR client is
 not available with compilers that do not support namespace.

 Revision 1.35  1999/05/26 12:21:35  sll
 Use ENABLE_CLIENT_IR_SUPPORT alone to enable IR support.

 Revision 1.34  1999/05/25 13:37:44  sll
 Added missing CORBA 2.1 definitions.
 Added pd_magic field, PR_is_valid() static member functions to pseudo
 objects so that at runtime static member functions of these objects
 can check if the argument pointer is valid.

 Revision 1.33  1999/04/21 15:23:51  djr
 CORBA::ORB::ObjectIdList corrected to use new string sequence class.

 Revision 1.32  1999/04/21 13:41:10  djr
 Added marshalling methods to CORBA::Context.
 String types are now defined in stringtypes.h, and typedef inside the
 CORBA module.

 Revision 1.31  1999/02/18 15:23:29  djr
 New type CORBA::Request_member. Corrected CORBA::ORB::RequestSeq to use
 this. CORBA::ORB::get_next_response now has Request_out parameter.

 Revision 1.30  1999/01/11 16:39:51  djr
 Added guard to prevent attempt to include corbaidl.hh from ir.hh. This
 fails as it is in omniORB2/corbaidl.hh. It doesn't need to be included
 from ir.hh, since it has already been included into CORBA.h.

 Revision 1.29  1999/01/07 18:14:11  djr
 Changes to support
  - New implementation of TypeCode and Any
  - New implementation of DynAny
  - DII and DSI
  - Other minor changes.

 Revision 1.28  1998/08/25 18:55:33  sll
 Added dummy marshalling functions in DynAny_member to keep gcc-2.7.2 happy.

 Revision 1.27  1998/08/21 19:26:48  sll
 New private function _omni_set_NameService.

 Revision 1.26  1998/08/19 15:59:00  sll
 All <<= and >>= operators are now defined in the global namespace.
 In particular, the operator>>= and <<= for DefinitionKind have been
 moved out of the namespace CORBA. This change should have no effect
 on platforms which do not support C++ namespace. On platforms with
 C++ namespace, the new code is expected to work whether or not the
 compiler supports koenig lookup rule.

 Revision 1.25  1998/08/15 15:07:08  sll
 *** empty log message ***

 Revision 1.24  1998/08/15 14:22:04  sll
 Remove inline member implementations in _nil_TypeCode and IRObject.

 Revision 1.23  1998/08/05 18:10:39  sll
 Added DynAny.

 Revision 1.22  1998/04/18 10:07:28  sll
 Renamed __e with _0RL_e in operator<<=() of DefinitionKind.

 Revision 1.21  1998/04/08 13:56:47  sll
 Minor change to the ctor of _nil_TypeCode to help some compiler to find
 the right TypeCode ctor.

 Revision 1.20  1998/04/07 19:55:53  sll
 Updated to use namespace if available.
 Moved inline functions of Any and TypeCode out of this header file.

 * Revision 1.19  1998/02/20  14:44:44  ewc
 * Changed to compile with aCC on HPUX
 *
 * Revision 1.18  1998/02/03  16:47:09  ewc
 * Updated some interfaces.
 *
 * Revision 1.17  1998/01/27  16:02:34  ewc
 * Added TypeCode and type Any
 *
 Revision 1.16  1997/12/18 17:37:20  sll
 Added (const char*) type casting to help strcpy().

 Revision 1.15  1997/12/09 20:35:16  sll
 New members BOA::impl_shutdown, BOA::destroy.

 Revision 1.14  1997/08/21 22:20:17  sll
 - String_member copy ctor bug fix.
 - New system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
   INVALID_TRANSACTION and WRONG_TRANSACTION.
 - Correct ORB_init() signature.
 - CORBA::is_nil(Object_ptr) is now more sympathetic to applications
   treating a nil pointer as a nil object reference.

 * Revision 1.13  1997/05/21  15:01:40  sll
 * Added typedef <type>_ptr <type>Ref;
 *
 * Revision 1.12  1997/05/06  16:04:43  sll
 * Public release.
 *
*/

#ifndef __CORBA_H__
#define __CORBA_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
# define __CORBA_H_EXTERNAL_GUARD__
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY) && defined(_OMNIORB_DYNAMIC_LIBRARY)
# error "_OMNIORB_LIBRARY and _OMNIORB_DYNAMIC_LIBRARY are both defined."
#endif

#if    defined(_OMNIORB_LIBRARY)
#         define _core_attr
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#elif  defined(_OMNIORB_DYNAMIC_LIBRARY)
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr
#else
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#endif

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

#include <omniORB4/omniInternal.h>


// Forward declarations.
class omniOrbBoaServant;


_CORBA_MODULE CORBA

_CORBA_MODULE_BEG


  // gcc can't cope with a typedef of void, so this will have to do.
  typedef void* Status;
# define RETURN_CORBA_STATUS   return 0


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// Primitive types //////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef _CORBA_Boolean    Boolean;
  typedef _CORBA_Char       Char;
  typedef _CORBA_Octet      Octet;
  typedef _CORBA_Short      Short;
  typedef _CORBA_UShort     UShort;
  typedef _CORBA_Long       Long;
  typedef _CORBA_ULong      ULong;
# ifdef HAS_LongLong
  typedef _CORBA_LongLong   LongLong;
  typedef _CORBA_ULongLong  ULongLong;
# endif
  typedef _CORBA_WChar      WChar;
# ifndef NO_FLOAT
  typedef _CORBA_Float      Float;
  typedef _CORBA_Double     Double;
# ifdef HAS_LongDouble
  typedef _CORBA_LongDouble LongDouble;
# endif
# endif

  typedef _CORBA_Boolean&    Boolean_out;
  typedef _CORBA_Char&       Char_out;
  typedef _CORBA_Octet&      Octet_out;
  typedef _CORBA_Short&      Short_out;
  typedef _CORBA_UShort&     UShort_out;
  typedef _CORBA_Long&       Long_out;
  typedef _CORBA_ULong&      ULong_out;
# ifdef HAS_LongLong
  typedef _CORBA_LongLong&   LongLong_out;
  typedef _CORBA_ULongLong&  ULongLong_out;
# endif
  typedef _CORBA_WChar&      WChar_out;
# ifndef NO_FLOAT
  typedef _CORBA_Float&      Float_out;
  typedef _CORBA_Double&     Double_out;
# ifdef HAS_LongDouble
  typedef _CORBA_LongDouble& LongDouble_out;
# endif
# endif


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// String ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN char* string_alloc(ULong len);
  _CORBA_MODULE_FN void string_free(char*);
  _CORBA_MODULE_FN char* string_dup(const char*);

  typedef _CORBA_String_var String_var;
  typedef _CORBA_String_member String_member;
  typedef _CORBA_String_inout String_INOUT_arg;
  typedef _CORBA_String_out String_OUT_arg;
  typedef String_OUT_arg String_out;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// WString //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN _CORBA_WChar* wstring_alloc(ULong len);
  _CORBA_MODULE_FN void          wstring_free(_CORBA_WChar*);
  _CORBA_MODULE_FN _CORBA_WChar* wstring_dup(const _CORBA_WChar*);

  typedef _CORBA_WString_var    WString_var;
  typedef _CORBA_WString_member WString_member;
  typedef _CORBA_WString_inout  WString_INOUT_arg;
  typedef _CORBA_WString_out    WString_OUT_arg;
  typedef WString_OUT_arg       WString_out;


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// Any ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Object;
  class Object_var;
  typedef Object* Object_ptr;
  typedef Object_ptr ObjectRef;

  class TypeCode;
  typedef class TypeCode* TypeCode_ptr;
  typedef TypeCode_ptr TypeCodeRef;

  class Fixed;

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
	: val((char*)s), bound(b), nc(nocopy) { }
      from_string(char* s, ULong b, Boolean nocopy = 0)
	: val(s), bound(b), nc(nocopy) { }   // deprecated

      char* val;
      ULong bound;
      Boolean nc;
    };

    struct from_wstring {
      from_wstring(const WChar* s, ULong b, Boolean nocopy = 0)
	: val((WChar*)s), bound(b), nc(nocopy) { }
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
      to_object(Object_ptr& obj) : ref(obj) { }
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

    Boolean operator>>=(to_fixed s) const;

    Boolean operator>>=(to_object o) const;

    void replace(TypeCode_ptr TCp, void* value, Boolean release = 0);

    TypeCode_ptr type() const;
    void type(TypeCode_ptr);

    const void* value() const;

  private:
    void *pd_ptr;
  };


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

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
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

    inline void operator<<=(Any::from_string s){
      *pd_data <<= s;
    }

    inline void operator<<=(Any::from_wstring s){
      *pd_data <<= s;
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

#ifndef _NO_ANY_STRING_EXTRACTION_
    inline Boolean operator>>=(char*& s) const {
      return (*pd_data >>= s);
    }
#endif

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

#ifndef _NO_ANY_STRING_EXTRACTION_
    inline Boolean operator>>=(Any::to_string s) const {
      return (*pd_data >>= s);
    }
#endif

    inline Boolean operator>>=(Any::to_wstring s) const {
      return (*pd_data >>= s);
    }

    inline Boolean operator>>=(Any::to_object o) const {
      return (*pd_data >>= o);
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


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// Exception /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Exception {
  public:
    virtual ~Exception();

    virtual void _raise() = 0;
    // 'throw' a copy of self. Must be overriden by all descendants.

    static inline Exception* _downcast(Exception* e) { return e; }
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.

    static inline const Exception* _downcast(const Exception* e) { return e; }
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.

    static inline Exception* _narrow(Exception* e) { return e; }
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.
    // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

    static Exception* _duplicate(Exception* e);

    // omniORB internal.
    virtual const char* _NP_repoId(int* size) const = 0;
    virtual void _NP_marshal(cdrStream&) const = 0;

    static inline _CORBA_Boolean PR_is_valid(Exception* p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }
    static _core_attr const _CORBA_ULong PR_magic;

    typedef void (*insertExceptionToAny)    (Any&, const Exception&);
    typedef void (*insertExceptionToAnyNCP) (Any&, const Exception*);

    inline insertExceptionToAny insertToAnyFn() const {
      return pd_insertToAnyFn;
    }
    inline insertExceptionToAnyNCP insertToAnyFnNCP() const {
      return pd_insertToAnyFnNCP;
    }

  protected:
    inline Exception() :
      pd_insertToAnyFn(0),
      pd_insertToAnyFnNCP(0),
      pd_magic(PR_magic)
      {}
    inline Exception& operator = (const Exception& ex) {
      pd_magic = ex.pd_magic;
      pd_insertToAnyFn = ex.pd_insertToAnyFn;
      pd_insertToAnyFnNCP = ex.pd_insertToAnyFnNCP;
      return *this;
    }
    static Exception* _NP_is_a(const Exception* e, const char* typeId);

    insertExceptionToAny     pd_insertToAnyFn;
    insertExceptionToAnyNCP  pd_insertToAnyFnNCP;

#if _MSC_VER      // Copy ctor has to be public for catch on this
                  // base class to work when a derived class is thrown.
  public:
#endif
    inline Exception(const Exception& ex) :
      pd_insertToAnyFn(ex.pd_insertToAnyFn),
      pd_insertToAnyFnNCP(ex.pd_insertToAnyFnNCP),
      pd_magic(ex.pd_magic)
      {}

  private:
    virtual Exception* _NP_duplicate() const = 0;
    // Must be overriden by all descendants to return a new copy of
    // themselves.

    virtual const char* _NP_typeId() const = 0;
    // Returns a type identifier in the form of a string.  The format is
    // internal to omniORB. This is used to support the _downcast()
    // operation.  Must be overriden by all descendants.

    ULong pd_magic;
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// SystemException //////////////////////////
  //////////////////////////////////////////////////////////////////////

  enum CompletionStatus { COMPLETED_YES, COMPLETED_NO, COMPLETED_MAYBE };
  enum exception_type { NO_EXCEPTION, USER_EXCEPTION, SYSTEM_EXCEPTION };

  _CORBA_MODULE_VARINT const ULong
       OMGVMCID _init_in_decl_(  = 1330446336 );


  class SystemException : public Exception {
  public:
    virtual ~SystemException();

#ifdef minor
    // Digital Unix 3.2, and may be others as well, defines minor() as
    // a macro in its sys/types.h. Get rid of it!
#undef minor
#endif

    inline ULong minor() const { return pd_minor; }
    inline void minor(ULong m) { pd_minor = m;    }

    inline CompletionStatus completed() const { return pd_status; }
    inline void completed(CompletionStatus s) { pd_status = s;    }

    static SystemException* _downcast(Exception*);
    static const SystemException* _downcast(const Exception*);
    static inline SystemException* _narrow(Exception* e) {
      return _downcast(e);
    }

    inline SystemException() {
      pd_minor = 0;
      pd_status = COMPLETED_NO;
    }
    inline SystemException(const SystemException& e) : Exception(e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
    }
    inline SystemException(ULong minor, CompletionStatus status) {
      pd_minor = minor;
      pd_status = status;
    }
    inline SystemException& operator=(const SystemException& e) {
      Exception::operator=(e);
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
      return *this;
    }

  protected:
    ULong             pd_minor;
    CompletionStatus  pd_status;

  private:
    virtual void _NP_marshal(cdrStream&) const;
  };

#define OMNIORB_DECLARE_SYS_EXCEPTION(name) \
  class name : public SystemException { \
  public: \
    inline name(ULong minor = 0, CompletionStatus completed = COMPLETED_NO) \
      : SystemException (minor, completed) { \
          pd_insertToAnyFn    = insertToAnyFn; \
          pd_insertToAnyFnNCP = insertToAnyFnNCP; \
    } \
    inline name(const name& ex) : SystemException(ex) {} \
    inline name& operator=(const name& ex) { \
      SystemException::operator=(ex); \
      return *this; \
    } \
    virtual ~name(); \
    virtual void _raise(); \
    static name* _downcast(Exception*); \
    static const name* _downcast(const Exception*); \
    static inline name* _narrow(Exception* e) { return _downcast(e); } \
    virtual const char* _NP_repoId(int* size) const; \
    static _core_attr Exception::insertExceptionToAny    insertToAnyFn; \
    static _core_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP; \
  private: \
    virtual Exception* _NP_duplicate() const; \
    virtual const char* _NP_typeId() const; \
  };

  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIORB_DECLARE_SYS_EXCEPTION)

#undef OMNIORB_DECLARE_SYS_EXCEPTION


  //////////////////////////////////////////////////////////////////////
  //////////////////////////// UserException ///////////////////////////
  //////////////////////////////////////////////////////////////////////

  class UserException : public Exception {
  public:
    virtual ~UserException();
    static UserException* _downcast(Exception* e);
    static const UserException* _downcast(const Exception* e);
    static inline UserException* _narrow(Exception* e) { return _downcast(e); }

  protected:
    inline UserException() {}

#if _MSC_VER      // Copy ctor has to be public for catch on this
                  // base class to work when a derived class is thrown.
  public:
#endif
    inline UserException(const UserException& ex) : Exception(ex) {}

  public: // Don't know why this needs to be public ...
    UserException& operator=(const UserException& ex) {
      Exception::operator=(ex);
      return *this;
    }
  };


  //////////////////////////////////////////////////////////////////////
  //////////////////////// UnknownUserException ////////////////////////
  //////////////////////////////////////////////////////////////////////

  class UnknownUserException : public UserException {
  public:
    virtual ~UnknownUserException();

    UnknownUserException(Any* ex);
    // Consumes <ex> which MUST be a UserException.

    inline UnknownUserException(const UnknownUserException& ex)
      : UserException(ex) {
      pd_exception = new Any(*ex.pd_exception);
    }
    UnknownUserException& operator=(const UnknownUserException& ex) {
      UserException::operator=(ex);
      if (pd_exception) delete pd_exception;
      pd_exception = new Any(*ex.pd_exception);
      return *this;
    }

    Any& exception();

    virtual void _raise();
    static UnknownUserException* _downcast(Exception*);
    static const UnknownUserException* _downcast(const Exception*);
    static inline UnknownUserException* _narrow(Exception* e) {
      return _downcast(e);
    }

    static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
    static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

  private:
    virtual Exception* _NP_duplicate() const;
    virtual const char* _NP_typeId() const;
    virtual const char* _NP_repoId(int* size) const;
    virtual void _NP_marshal(cdrStream&) const;

    Any* pd_exception;
  };


  //////////////////////////////////////////////////////////////////////
  ////////////////////////// WrongTransaction //////////////////////////
  //////////////////////////////////////////////////////////////////////

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(WrongTransaction, _dyn_attr)


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// Environment ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Environment;
  typedef Environment* Environment_ptr;
  typedef Environment_ptr EnvironmentRef;
  typedef _CORBA_PseudoObj_Var<Environment> Environment_var;
  typedef _CORBA_PseudoObj_Out<Environment,Environment_var> Environment_out;

  class Environment {
  public:
    Environment();     // Allows Environment instance to be created on
                       // stack or by x = new Environment().
    virtual ~Environment();

    virtual void exception(Exception*);
    virtual Exception* exception() const;
    virtual void clear();

    virtual Boolean NP_is_nil() const;
    virtual Environment_ptr NP_duplicate();

    static Environment_ptr _duplicate(Environment_ptr);
    static Environment_ptr _nil();

    static inline _CORBA_Boolean PR_is_valid(Environment_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  private:
    CORBA::Exception* pd_exception;
    _CORBA_ULong      pd_magic;

    Environment(const Environment&);
    Environment& operator=(const Environment&);

  public:
    CORBA::Boolean pd_is_pseudo;
  };


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// NamedValue /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  enum _Flags {
    ARG_IN              = 0x1,
    ARG_OUT             = 0x2,
    ARG_INOUT           = 0x4,
    CTX_RESTRICT_SCOPE  = 0x8,
    OUT_LIST_MEMORY     = 0x10,
    IN_COPY_VALUE       = 0x20
  };

  typedef ULong Flags;

  class NamedValue;
  typedef NamedValue* NamedValue_ptr;
  typedef NamedValue_ptr NamedValueRef;
  typedef _CORBA_PseudoObj_Var<NamedValue> NamedValue_var;
  typedef _CORBA_PseudoObj_Out<NamedValue,NamedValue_var> NamedValue_out;

  class NamedValue {
  public:
    virtual ~NamedValue();

    virtual const char* name() const = 0;
    // Retains ownership of return value.

    virtual Any* value() const = 0;
    // Retains ownership of return value.

    virtual Flags flags() const = 0;

    virtual Boolean NP_is_nil() const = 0;
    virtual NamedValue_ptr NP_duplicate() = 0;

    static NamedValue_ptr _duplicate(NamedValue_ptr);
    static NamedValue_ptr _nil();

    static inline _CORBA_Boolean PR_is_valid(NamedValue_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    NamedValue() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    NamedValue(const NamedValue&);
    NamedValue& operator=(const NamedValue&);
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// NVList ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class NVList;
  typedef NVList* NVList_ptr;
  typedef NVList_ptr NVListRef;
  typedef _CORBA_PseudoObj_Var<NVList> NVList_var;
  typedef _CORBA_PseudoObj_Out<NVList,NVList_var> NVList_out;

  class NVList {
  public:
    virtual ~NVList();

    virtual ULong count() const = 0;
    virtual NamedValue_ptr add(Flags) = 0;
    virtual NamedValue_ptr add_item(const char*, Flags) = 0;
    virtual NamedValue_ptr add_value(const char*, const Any&, Flags) = 0;
    virtual NamedValue_ptr add_item_consume(char*,Flags) = 0;
    virtual NamedValue_ptr add_value_consume(char*, Any*, Flags) = 0;
    virtual NamedValue_ptr item(ULong index) = 0;
    virtual Status remove (ULong) = 0;

    virtual Boolean NP_is_nil() const = 0;
    virtual NVList_ptr NP_duplicate() = 0;

    static NVList_ptr _duplicate(NVList_ptr);
    static NVList_ptr _nil();

    // OMG Interface:

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

    static inline _CORBA_Boolean PR_is_valid(NVList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    NVList() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    NVList(const NVList& nvl);
    NVList& operator=(const NVList&);
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Context //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Context;
  typedef Context* Context_ptr;
  typedef Context_ptr ContextRef;
  typedef _CORBA_PseudoObj_Var<Context> Context_var;
  typedef _CORBA_PseudoObj_Out<Context,Context_var> Context_out;

  class Context {
  public:
    virtual ~Context();

    virtual const char* context_name() const = 0;
    virtual CORBA::Context_ptr parent() const = 0;
    virtual CORBA::Status create_child(const char*, Context_out) = 0;
    virtual CORBA::Status set_one_value(const char*, const Any&) = 0;
    virtual CORBA::Status set_values(CORBA::NVList_ptr) = 0;
    virtual CORBA::Status delete_values(const char*) = 0;
    virtual CORBA::Status get_values(const char* start_scope,
				     CORBA::Flags op_flags,
				     const char* pattern,
				     CORBA::NVList_out values) = 0;
    // Throws BAD_CONTEXT if <start_scope> is not found.
    // Returns a nil NVList in <values> if no matches are found.

    virtual Boolean NP_is_nil() const = 0;
    virtual CORBA::Context_ptr NP_duplicate() = 0;

    static Context_ptr _duplicate(Context_ptr);
    static Context_ptr _nil();

    // omniORB specifics.
    static void marshalContext(Context_ptr ctxt, const char*const* which,
			       int whichlen, cdrStream& s);
    static Context_ptr unmarshalContext(cdrStream& s);

    static inline _CORBA_Boolean PR_is_valid(Context_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    Context() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    Context(const Context&);
    Context& operator=(const Context&);
  };


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// ContextList ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ContextList;
  typedef ContextList* ContextList_ptr;
  typedef ContextList_ptr ContextListRef;
  typedef _CORBA_PseudoObj_Var<ContextList> ContextList_var;
  typedef _CORBA_PseudoObj_Out<ContextList,ContextList_var> ContextList_out;

  class ContextList {
  public:
    virtual ~ContextList();

    virtual ULong count() const = 0;
    virtual void add(const char* ctxt) = 0;
    virtual void add_consume(char* ctxt) = 0;
    // consumes ctxt

    virtual const char* item(ULong index) = 0;
    // retains ownership of return value

    virtual Status remove(ULong index) = 0;

    virtual Boolean NP_is_nil() const = 0;
    virtual ContextList_ptr NP_duplicate() = 0;

    static ContextList_ptr _duplicate(ContextList_ptr);
    static ContextList_ptr _nil();

    // OMG Interface:

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

    static inline _CORBA_Boolean PR_is_valid(ContextList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    ContextList() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    ContextList(const ContextList&);
    ContextList& operator=(const ContextList&);
  };


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// Principal /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Principal;
  typedef Principal* Principal_ptr;
  typedef Principal_ptr PrincipalRef;
  //typedef _CORBA_PseudoObj_Var<Principal> Principal_var;
  //typedef _CORBA_PseudoObj_Out<Principal,Principal_var> Principal_out;

  class PrincipalID : public _CORBA_Unbounded_Sequence_Octet {
  public:
    inline PrincipalID() {}
    inline PrincipalID(const PrincipalID& seq)
      : _CORBA_Unbounded_Sequence_Octet(seq) {}
    inline PrincipalID(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence_Octet(max) {}
    inline PrincipalID(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence_Octet(max, len, val, rel) {}
    inline PrincipalID& operator = (const PrincipalID& seq) {
      _CORBA_Unbounded_Sequence_Octet::operator=(seq);
      return *this;
    };
  };

  class Principal {
  public:
    static Principal_ptr _duplicate(Principal_ptr);
    static Principal_ptr _nil();
  private:
    Principal(); // Not implemented yet
  };


  //////////////////////////////////////////////////////////////////////
  //////////////////////////// ExceptionList ///////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ExceptionList;
  typedef ExceptionList* ExceptionList_ptr;
  typedef ExceptionList_ptr ExceptionListRef;
  typedef _CORBA_PseudoObj_Var<ExceptionList> ExceptionList_var;
  typedef _CORBA_PseudoObj_Out<ExceptionList,ExceptionList_var> ExceptionList_out;

  class ExceptionList {
  public:
    virtual ~ExceptionList();

    virtual ULong count() const = 0;
    virtual void add(TypeCode_ptr tc) = 0;
    virtual void add_consume(TypeCode_ptr tc) = 0;
    // Consumes <tc>.

    virtual TypeCode_ptr item(ULong index) = 0;
    // Retains ownership of return value.

    virtual Status remove(ULong index) = 0;

    virtual Boolean NP_is_nil() const = 0;
    virtual ExceptionList_ptr NP_duplicate() = 0;

    static ExceptionList_ptr _duplicate(ExceptionList_ptr);
    static ExceptionList_ptr _nil();

    // OMG Interface:

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

    static inline _CORBA_Boolean PR_is_valid(ExceptionList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    ExceptionList() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    ExceptionList(const ExceptionList& el);
    ExceptionList& operator=(const ExceptionList&);
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// TypeCode_member //////////////////////////
  //////////////////////////////////////////////////////////////////////

  class TypeCode_var;

  class TypeCode_member {
  public:
    TypeCode_member();
    inline TypeCode_member(TypeCode_ptr p) : _ptr(p) {}
    TypeCode_member(const TypeCode_member& p);
    ~TypeCode_member();

    TypeCode_member& operator=(TypeCode_ptr p);
    TypeCode_member& operator=(const TypeCode_member& p);
    TypeCode_member& operator=(const TypeCode_var& p);

    inline TypeCode_ptr operator->() const { return _ptr; }
    inline operator TypeCode_ptr() const   { return _ptr; }

    TypeCode_ptr _ptr;

    void operator>>=(cdrStream&) const;
    void operator<<=(cdrStream&);
  };


#ifdef HAS_Cplusplus_Namespace
_CORBA_MODULE_END

_CORBA_MODULE CORBA
_CORBA_MODULE_BEG
#endif

#if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB4/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB4/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB4/corbaidl_defs.hh>
#endif

  class InterfaceDef;
  class _objref_InterfaceDef;

  class ImplementationDef {}; // Not used.
  typedef ImplementationDef* ImplementationDef_ptr;
  typedef ImplementationDef_ptr ImplementationDefRef;

  class OperationDef;
  class _objref_OperationDef;

  class                     ServerRequest;
  typedef ServerRequest*    ServerRequest_ptr;
  typedef ServerRequest_ptr ServerRequestRef;

  class                                                Request;
  typedef Request*                                     Request_ptr;
  typedef Request_ptr                                  RequestRef;
  typedef _CORBA_PseudoObj_Var<Request>                Request_var;
  typedef _CORBA_PseudoObj_Out<Request,Request_var>    Request_out;
  typedef _CORBA_PseudoObj_Member<Request,Request_var> Request_member;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Object ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Object_Helper {
  public:
    typedef Object_ptr _ptr_type;

    static _ptr_type _nil();
    static _CORBA_Boolean is_nil(_ptr_type);
    static void release(_ptr_type);
    static void duplicate(_ptr_type);
    static void marshalObjRef(_ptr_type, cdrStream&);
    static _ptr_type unmarshalObjRef(cdrStream&);
  };

  class Object {
  public:
    typedef Object_ptr _ptr_type;
    typedef Object_var _var_type;

    virtual ~Object();

    Status _create_request(Context_ptr ctx,
			   const char *operation,
			   NVList_ptr arg_list,
			   NamedValue_ptr result,
			   Request_out request,
			   Flags req_flags);

    Status _create_request(Context_ptr ctx,
			   const char *operation,
			   NVList_ptr arg_list,
			   NamedValue_ptr result,
			   ExceptionList_ptr exceptions,
			   ContextList_ptr ctxlist,
			   Request_out request,
			   Flags req_flags);

    Request_ptr _request(const char* operation);

    ImplementationDef_ptr _get_implementation();
    _objref_InterfaceDef* _get_interface();
    // = InterfaceDef_ptr    _get_interface();

    Boolean         _is_a(const char* repoId);
    virtual Boolean _non_existent();
    Boolean         _is_equivalent(_ptr_type other_object);
    ULong           _hash(ULong maximum);

    static _ptr_type        _duplicate(_ptr_type);
    static inline _ptr_type _narrow(Object_ptr o) { return _duplicate(o); }
    static _ptr_type        _nil();

    //////////////////////
    // omniORB internal //
    //////////////////////

    inline Object() : pd_obj(0), pd_magic(_PR_magic) {}

    inline Boolean _NP_is_nil() const { return pd_obj == 0; }
    inline Boolean _NP_is_pseudo() const { return pd_obj == (omniObjRef*) 1; }
    inline omniObjRef* _PR_getobj() { return pd_obj; }
    inline void _PR_setobj(omniObjRef* o) { pd_obj = o; }
    static inline _CORBA_Boolean _PR_is_valid(Object_ptr p) {
      return p ? (p->pd_magic == _PR_magic) : 1;
    }

    virtual void _NP_incrRefCount();
    virtual void _NP_decrRefCount();
    virtual void* _ptrToObjRef(const char* repoId);

    static void _marshalObjRef(Object_ptr, cdrStream&);
    static Object_ptr _unmarshalObjRef(cdrStream&);

    static _core_attr const char* _PD_repoId;
    static _core_attr const ULong _PR_magic;

  private:
    omniObjRef* pd_obj;
    // <obj> is 0 for a nil reference, 1 for a pseudo reference.

    ULong       pd_magic;
  };


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// TypeCode //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // omniORB specific TypeCode support structures, used in stubs.
  struct PR_structMember {
    const char*  name;
    TypeCode_ptr type;
  };

#ifndef HAS_LongLong
  typedef ULong PR_unionDiscriminator;
  typedef Long  PR_unionDiscriminatorSigned;
#else
  typedef ULongLong PR_unionDiscriminator;
  typedef LongLong  PR_unionDiscriminatorSigned;
#endif

  struct PR_unionMember {
    const char*           name;
    TypeCode_ptr          type;
    PR_unionDiscriminator label;
  };

  enum TCKind {
    tk_null		  = 0,
    tk_void		  = 1,
    tk_short		  = 2,
    tk_long		  = 3,
    tk_ushort		  = 4,
    tk_ulong		  = 5,
    tk_float		  = 6,
    tk_double		  = 7,
    tk_boolean		  = 8,
    tk_char		  = 9,
    tk_octet		  = 10,
    tk_any		  = 11,
    tk_TypeCode		  = 12,
    tk_Principal	  = 13,
    tk_objref		  = 14,
    tk_struct		  = 15,
    tk_union		  = 16,
    tk_enum		  = 17,
    tk_string		  = 18,
    tk_sequence		  = 19,
    tk_array		  = 20,
    tk_alias		  = 21,
    tk_except		  = 22,
    tk_longlong		  = 23,
    tk_ulonglong	  = 24,
    tk_longdouble	  = 25,
    tk_wchar		  = 26,
    tk_wstring		  = 27,
    tk_fixed		  = 28,
    tk_value		  = 29,
    tk_value_box	  = 30,
    tk_native		  = 31,
    tk_abstract_interface = 32,
    tk_local_interface	  = 33
  };


  class TypeCode {
  public:
    virtual ~TypeCode();

    TCKind kind() const;

    Boolean equal(TypeCode_ptr TCp) const;

    Boolean equivalent(TypeCode_ptr TCp) const;
    // CORBA 2.3 addition

    TypeCode_ptr get_compact_typecode() const;
    // CORBA 2.3 addition

    const char* id() const;
    const char* name() const;

    ULong member_count() const;
    const char* member_name(ULong index) const;

    TypeCode_ptr member_type(ULong i) const;

    Any* member_label(ULong i) const;
    TypeCode_ptr discriminator_type() const;
    Long default_index() const;

    ULong length() const;

    TypeCode_ptr content_type() const;

    UShort fixed_digits() const;
    Short fixed_scale() const;
    
    Long param_count() const;             // obsolete
    Any* parameter(Long index) const;     // obsolete

#if 0
    // CORBA 2.3 additions
    // Not supported yet
    Visibility member_visibility(ULong index) const;
    ValuetypeModifier type_modifier() const;
    TypeCode_ptr concrete_base_type() const;
#endif    

    static TypeCode_ptr _duplicate(TypeCode_ptr t);
    static TypeCode_ptr _nil();

    // OMG Interface:

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(BadKind, _dyn_attr)

    static void marshalTypeCode(TypeCode_ptr obj,cdrStream& s);
    static TypeCode_ptr unmarshalTypeCode(cdrStream& s);

    // omniORB only static constructors
    // 1) These constructors are used by omniORB stubs & libraries to produce
    //    typecodes for complex types.  They should not be used in CORBA
    //    application code.
    // 2) Any typecode pointers passed into these functions are _duplicated
    //    before being saved into the relevant structures.
    // 3) The returned typecode pointers have reference counts of 1.

#if 0
    static TypeCode_ptr NP_struct_tc(const char* id, const char* name,
				     const StructMemberSeq& members);
    static TypeCode_ptr NP_exception_tc(const char* id, const char* name,
					const StructMemberSeq& members);
#endif
    static TypeCode_ptr NP_union_tc(const char* id, const char* name,
				    TypeCode_ptr discriminator_type,
				    const UnionMemberSeq& members);
    static TypeCode_ptr NP_enum_tc(const char* id, const char* name,
				   const EnumMemberSeq& members);
    static TypeCode_ptr NP_alias_tc(const char* id, const char* name,
				    TypeCode_ptr original_type);
    static TypeCode_ptr NP_interface_tc(const char* id, const char* name);
    static TypeCode_ptr NP_string_tc(ULong bound);
    static TypeCode_ptr NP_wstring_tc(ULong bound);
    static TypeCode_ptr NP_fixed_tc(UShort digits, Short scale);
    static TypeCode_ptr NP_sequence_tc(ULong bound, TypeCode_ptr element_type);
    static TypeCode_ptr NP_array_tc(ULong length, TypeCode_ptr element_type);
    static TypeCode_ptr NP_recursive_sequence_tc(ULong bound, ULong offset);

    // omniORB only static constructors for stubs
    static TypeCode_ptr PR_struct_tc(const char* id, const char* name,
				     const PR_structMember* members,
				     ULong memberCount);
    // Duplicates <id> and <name>. Duplicates the names in <members>,
    // but consumes the types.
    static TypeCode_ptr PR_union_tc(const char* id, const char* name,
				    TypeCode_ptr discriminator_type,
				    const PR_unionMember* members,
				    ULong memberCount, Long deflt = -1);
    static TypeCode_ptr PR_enum_tc(const char* id, const char* name,
				   const char** members, ULong memberCount);
    static TypeCode_ptr PR_alias_tc(const char* id, const char* name,
				    TypeCode_ptr original_type);
    static TypeCode_ptr PR_exception_tc(const char* id, const char* name,
					const PR_structMember* members,
					ULong memberCount);
    // Duplicates <id> and <name>. Duplicates the names in <members>,
    // but consumes the types.
    static TypeCode_ptr PR_interface_tc(const char* id, const char* name);
    static TypeCode_ptr PR_string_tc(ULong bound);
    static TypeCode_ptr PR_wstring_tc(ULong bound);
    static TypeCode_ptr PR_sequence_tc(ULong bound, TypeCode_ptr element_type);
    static TypeCode_ptr PR_array_tc(ULong length, TypeCode_ptr element_type);
    static TypeCode_ptr PR_recursive_sequence_tc(ULong bound, ULong offset);
    static TypeCode_ptr PR_null_tc();
    static TypeCode_ptr PR_void_tc();
    static TypeCode_ptr PR_short_tc();
    static TypeCode_ptr PR_long_tc();
    static TypeCode_ptr PR_ushort_tc();
    static TypeCode_ptr PR_ulong_tc();
    static TypeCode_ptr PR_float_tc();
    static TypeCode_ptr PR_double_tc();
    static TypeCode_ptr PR_boolean_tc();
    static TypeCode_ptr PR_char_tc();
    static TypeCode_ptr PR_wchar_tc();
    static TypeCode_ptr PR_octet_tc();
    static TypeCode_ptr PR_any_tc();
    static TypeCode_ptr PR_TypeCode_tc();
    static TypeCode_ptr PR_Principal_tc();
    static TypeCode_ptr PR_Object_tc();
    static TypeCode_ptr PR_string_tc();
    static TypeCode_ptr PR_wstring_tc();
    static TypeCode_ptr PR_fixed_tc(UShort digits, UShort scale);
#ifdef HAS_LongLong
    static TypeCode_ptr PR_longlong_tc();
    static TypeCode_ptr PR_ulonglong_tc();
#endif
#ifdef HAS_LongDouble
    static TypeCode_ptr PR_longdouble_tc();
#endif

    // omniORB internal functions
    virtual CORBA::Boolean NP_is_nil() const;

    static inline _CORBA_Boolean PR_is_valid(TypeCode_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }
    static _dyn_attr const CORBA::ULong PR_magic;

  protected:
    // These operators are placed here to avoid them being used externally
    TypeCode(const TypeCode& tc);
    TypeCode& operator=(const TypeCode& tc);
    TypeCode() { pd_magic = PR_magic; };

    _CORBA_ULong pd_magic;
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////// TypeCodes of Primitives //////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_null;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_void;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_short;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_long;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ushort;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ulong;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_float;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_double;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_boolean;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_char;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_wchar;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_octet;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_any;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_TypeCode;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Principal;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Object;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_string;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_wstring;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_NamedValue;
#ifdef HAS_LongLong
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_longlong;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ulonglong;
#endif
#ifdef HAS_LongDouble
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_longdouble;
#endif


  //////////////////////////////////////////////////////////////////////
  ////////////////////////// DynAny Interface //////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynAny;
  typedef DynAny* DynAny_ptr;
  typedef DynAny_ptr DynAnyRef;

  class DynEnum;
  typedef DynEnum* DynEnum_ptr;
  typedef DynEnum_ptr DynEnumRef;

  class DynStruct;
  typedef DynStruct* DynStruct_ptr;
  typedef DynStruct_ptr DynStructRef;

  class DynUnion;
  typedef DynUnion* DynUnion_ptr;
  typedef DynUnion_ptr DynUnionRef;

  class DynSequence;
  typedef DynSequence* DynSequence_ptr;
  typedef DynSequence_ptr DynSequenceRef;

  class DynArray;
  typedef DynArray* DynArray_ptr;
  typedef DynArray_ptr DynArrayRef;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynAny ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynAny {
  public:

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Invalid, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidValue, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(TypeMismatch, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidSeq, _dyn_attr)

    typedef _CORBA_Unbounded_Sequence_Octet OctetSeq;

    virtual ~DynAny();

    virtual TypeCode_ptr type() const = 0;
    virtual void assign(DynAny_ptr dyn_any) = 0;
    virtual void from_any(const Any& value) = 0;
    virtual Any* to_any() = 0;
    virtual void destroy() = 0;
    virtual DynAny_ptr copy() = 0;
    virtual void insert_boolean(Boolean value) = 0;
    virtual void insert_octet(Octet value) = 0;
    virtual void insert_char(Char value) = 0;
    virtual void insert_wchar(WChar value) = 0;
    virtual void insert_short(Short value) = 0;
    virtual void insert_ushort(UShort value) = 0;
    virtual void insert_long(Long value) = 0;
    virtual void insert_ulong(ULong value) = 0;
#ifdef HAS_LongLong
    virtual void insert_longlong(LongLong value) = 0;
    virtual void insert_ulonglong(ULongLong value) = 0;
#endif
#ifndef NO_FLOAT
    virtual void insert_float(Float value) = 0;
    virtual void insert_double(Double value) = 0;
#ifdef HAS_LongDouble
    virtual void insert_longdouble(LongDouble value) = 0;
#endif
#endif
    virtual void insert_string(const char* value) = 0;
    virtual void insert_wstring(const WChar* value) = 0;
    virtual void insert_reference(Object_ptr value) = 0;
    virtual void insert_typecode(TypeCode_ptr value) = 0;
    virtual void insert_any(const Any& value) = 0;
    virtual Boolean get_boolean() = 0;
    virtual Octet get_octet() = 0;
    virtual Char get_char() = 0;
    virtual WChar get_wchar() = 0;
    virtual Short get_short() = 0;
    virtual UShort get_ushort() = 0;
    virtual Long get_long() = 0;
    virtual ULong get_ulong() = 0;
#ifdef HAS_LongLong
    virtual LongLong get_longlong() = 0;
    virtual ULongLong get_ulonglong() = 0;
#endif
#ifndef NO_FLOAT
    virtual Float get_float() = 0;
    virtual Double get_double() = 0;
#ifdef HAS_LongDouble
    virtual LongDouble get_longdouble() = 0;
#endif
#endif
    virtual char* get_string() = 0;
    virtual WChar* get_wstring() = 0;
    virtual Object_ptr get_reference() = 0;
    virtual TypeCode_ptr get_typecode() = 0;
    virtual Any* get_any() = 0;
    virtual DynAny_ptr current_component() = 0;
    virtual Boolean next() = 0;
    virtual Boolean seek(Long index) = 0;
    virtual void rewind() = 0;

    static DynAny_ptr _duplicate(DynAny_ptr);
    static DynAny_ptr _narrow(DynAny_ptr);
    static DynAny_ptr _nil();

    virtual Boolean NP_is_nil() const = 0;
    virtual int NP_nodetype() const = 0;
    virtual void* NP_narrow() = 0;

    static inline _CORBA_Boolean PR_is_valid(DynAny_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;
  protected:
    DynAny();

  private:
    _CORBA_ULong pd_magic;

    DynAny(const DynAny&);
    DynAny& operator=(const DynAny&);
  };

#if 0
  // DynFixed - not yet implemented.
  class DynFixed :  public virtual DynAny {
  public:

    virtual OctetSeq* get_value() = 0;
    virtual void set_value(const OctetSeq& val) = 0;

    static DynFixed_ptr _duplicate(DynFixed_ptr);
    static DynFixed_ptr _narrow(DynAny_ptr);
    static DynFixed_ptr _nil();

    virtual ~DynFixed();

  protected:
    DynFixed() {}

  private:
    DynFixed(const DynFixed&);
    DynFixed& operator=(const DynFixed&);
  };
#endif

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynEnum //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynEnum :  public virtual DynAny {
  public:

    virtual char* value_as_string() = 0;
    virtual void value_as_string(const char* value) = 0;
    virtual ULong value_as_ulong() = 0;
    virtual void value_as_ulong(ULong value) = 0;

    static DynEnum_ptr _duplicate(DynEnum_ptr);
    static DynEnum_ptr _narrow(DynAny_ptr);
    static DynEnum_ptr _nil();

    virtual ~DynEnum();

  protected:
    DynEnum() {}

  private:
    DynEnum(const DynEnum&);
    DynEnum& operator=(const DynEnum&);
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynStruct /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef char* FieldName;
  typedef String_var FieldName_var;

  struct NameValuePair {
    String_member id;
    Any value;
  };

  typedef _CORBA_ConstrType_Variable_Var<NameValuePair> NameValuePair_var;
  typedef _CORBA_PseudoValue_Sequence<NameValuePair> NameValuePairSeq;

  class DynStruct :  public virtual DynAny {
  public:

    virtual char*  current_member_name() = 0;
    virtual TCKind current_member_kind() = 0;
    virtual NameValuePairSeq* get_members() = 0;
    virtual void set_members(const NameValuePairSeq& NVSeqVal) = 0;

    static DynStruct_ptr _duplicate(DynStruct_ptr);
    static DynStruct_ptr _narrow(DynAny_ptr);
    static DynStruct_ptr _nil();

    virtual ~DynStruct();

  protected:
    DynStruct() {}

  private:
    DynStruct(const DynStruct&);
    DynStruct& operator=(const DynStruct&);
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynUnion //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynUnion :  public virtual DynAny {
  public:
    virtual Boolean set_as_default() = 0;
    virtual void set_as_default(Boolean value) = 0;
    virtual DynAny_ptr discriminator() = 0;
    virtual TCKind discriminator_kind() = 0;
    virtual DynAny_ptr member() = 0;
    virtual char*  member_name() = 0;
    virtual void member_name(const char* value) = 0;
    virtual TCKind member_kind() = 0;

    static DynUnion_ptr _duplicate(DynUnion_ptr);
    static DynUnion_ptr _narrow(DynAny_ptr);
    static DynUnion_ptr _nil();

    virtual ~DynUnion();

  protected:
    DynUnion() {}

  private:
    DynUnion(const DynUnion&);
    DynUnion& operator=(const DynUnion&);
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// DynSequence ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef _CORBA_PseudoValue_Sequence<Any> AnySeq;

  class DynSequence :  public virtual DynAny {
  public:

    virtual ULong length() = 0;
    virtual void length (ULong value) = 0;
    virtual AnySeq* get_elements() = 0;
    virtual void set_elements(const AnySeq& value) = 0;

    static DynSequence_ptr _duplicate(DynSequence_ptr);
    static DynSequence_ptr _narrow(DynAny_ptr);
    static DynSequence_ptr _nil();

    virtual ~DynSequence();

  protected:
    DynSequence() {}

  private:
    DynSequence(const DynSequence&);
    DynSequence& operator=(const DynSequence&);
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynArray //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynArray : public virtual DynAny {
  public:

    virtual AnySeq* get_elements() = 0;
    virtual void set_elements(const AnySeq& value) = 0;

    static DynArray_ptr _duplicate(DynArray_ptr);
    static DynArray_ptr _narrow(DynAny_ptr);
    static DynArray_ptr _nil();

    virtual ~DynArray();

  protected:
    DynArray() {}

  private:
    DynArray(const DynArray&);
    DynArray& operator=(const DynArray&);
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Request //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Request {
  public:
    virtual ~Request();

    virtual Object_ptr        target() const = 0;
    virtual const char*       operation() const = 0;
    virtual NVList_ptr        arguments() = 0;
    virtual NamedValue_ptr    result() = 0;
    virtual Environment_ptr   env() = 0;
    virtual ExceptionList_ptr exceptions() = 0;
    virtual ContextList_ptr   contexts() = 0;
    virtual Context_ptr       ctx() const = 0;
    virtual void              ctx(Context_ptr) = 0;

    virtual Any& add_in_arg() = 0;
    virtual Any& add_in_arg(const char* name) = 0;
    virtual Any& add_inout_arg() = 0;
    virtual Any& add_inout_arg(const char* name) = 0;
    virtual Any& add_out_arg() = 0;
    virtual Any& add_out_arg(const char* name) = 0;

    virtual void set_return_type(TypeCode_ptr tc) = 0;
    virtual Any& return_value() = 0;

    virtual Status  invoke() = 0;
    virtual Status  send_oneway() = 0;
    virtual Status  send_deferred() = 0;
    virtual Status  get_response() = 0;
    virtual Boolean poll_response() = 0;

    virtual Boolean NP_is_nil() const = 0;
    virtual Request_ptr NP_duplicate() = 0;

    static Request_ptr _duplicate(Request_ptr);
    static Request_ptr _nil();

    static inline _CORBA_Boolean PR_is_valid(Request_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _dyn_attr const _CORBA_ULong PR_magic;

  protected:
    Request() { pd_magic = PR_magic; }

  private:
    _CORBA_ULong pd_magic;

    Request(const Request&);
    Request& operator=(const Request&);
  };


  //////////////////////////////////////////////////////////////////////
  //////////////////////////// ServerRequest ///////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ServerRequest {
  public:
    virtual const char* operation() = 0;
    virtual void arguments(NVList_ptr& parameters) = 0;
    virtual Context_ptr ctx() = 0;
    virtual void set_result(const Any& value) = 0;
    virtual void set_exception(const Any& value) = 0;

  protected:
    inline ServerRequest() {}
    virtual ~ServerRequest();

  private:
    ServerRequest(const ServerRequest&);
    ServerRequest& operator=(const ServerRequest&);
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Policy ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef ULong PolicyType;

  class Policy;
  typedef Policy* Policy_ptr;
  typedef Policy_ptr PolicyRef;

  class Policy : public virtual Object {
  public:
    virtual ~Policy();

    virtual PolicyType policy_type();
    virtual Policy_ptr copy();
    // This at least needs to be overridden in subclasses.
    virtual void destroy();
    // May throw NO_PERMISSION.

    static Policy_ptr _duplicate(Policy_ptr);
    static Policy_ptr _narrow(Object_ptr);
    static Policy_ptr _nil();

    // omniORB internal.

    Policy(PolicyType type);
    Policy(); // nil policy

    virtual void* _ptrToObjRef(const char* repoId);
    virtual void _NP_incrRefCount();
    virtual void _NP_decrRefCount();
    // Override Object.

    static _core_attr const char* _PD_repoId;

  private:
    Policy(const Policy&);
    Policy& operator = (const Policy&);

    int pd_refCount;
    PolicyType pd_type;
  };

  typedef _CORBA_PseudoObj_Var<Policy> Policy_var;
  typedef _CORBA_PseudoObj_Member<Policy, Policy_var> Policy_member;

  typedef _CORBA_Pseudo_Unbounded_Sequence<Policy, Policy_member> PolicyList;
  typedef PolicyList* PolicyList_var;  //??


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// Domain Manager  ////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DomainManager;
  typedef class DomainManager* DomainManager_ptr;
  typedef DomainManager_ptr DomainManagerRef;

  class DomainManager {
  public:
    Policy_ptr get_domain_policy(PolicyType policy_type);

    static DomainManager_ptr _duplicate(DomainManager_ptr p);
    static DomainManager_ptr _nil();

  private:
    DomainManager();
  };

  _CORBA_MODULE_FN Boolean is_nil(DomainManager_ptr p);
  _CORBA_MODULE_FN void release(DomainManager_ptr);

  typedef _CORBA_PseudoObj_Var<DomainManager> DomainManager_var;
  typedef _CORBA_PseudoObj_Member<DomainManager,DomainManager_var>
    DomainManager_member;
  typedef _CORBA_Pseudo_Unbounded_Sequence<DomainManager,DomainManager_member>
    DomainManagerList;


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// BOA ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class BOA;
  class BOA_var;
  typedef BOA* BOA_ptr;
  typedef BOA_ptr BOARef;


  class ReferenceData : public _CORBA_Unbounded_Sequence_Octet {
  public://??
    inline ReferenceData() {}
    inline ReferenceData(const ReferenceData& seq)
      : _CORBA_Unbounded_Sequence_Octet(seq) {}
    inline ReferenceData(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence_Octet(max) {}
    inline ReferenceData(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence_Octet(max, len, val, rel) {}
    inline ReferenceData& operator = (const ReferenceData& seq) {
      _CORBA_Unbounded_Sequence_Octet::operator=(seq);
      return *this;
    };
  };


  class BOA : public Object {
  public:
    virtual void impl_is_ready(ImplementationDef_ptr p=0, Boolean NoBlk=0) = 0;
    virtual void impl_shutdown() = 0;
    virtual void destroy() = 0;

    virtual void obj_is_ready(omniOrbBoaServant*, ImplementationDef_ptr p=0)=0;
    virtual void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0)=0;
    virtual void dispose(Object_ptr) = 0;

    virtual Object_ptr create(const ReferenceData&, _objref_InterfaceDef*,
			      ImplementationDef_ptr) = 0;
    virtual ReferenceData* get_id(Object_ptr) = 0;
    virtual void change_implementation(Object_ptr,ImplementationDef_ptr) = 0;
    virtual Principal_ptr get_principal(Object_ptr, Environment_ptr) = 0;
    virtual void deactivate_impl(ImplementationDef_ptr) = 0;
    virtual void deactivate_obj(Object_ptr) = 0;

    static BOA_ptr _duplicate(BOA_ptr);
    static BOA_ptr _narrow(Object_ptr);
    static BOA_ptr _nil();
    static BOA_ptr getBOA();

    static _core_attr const char* _PD_repoId;

  protected:
    inline BOA(int nil) { _PR_setobj((omniObjRef*) (nil ? 0:1)); }
    virtual ~BOA();

  private:
    BOA(const BOA&);
    BOA& operator=(const BOA&);
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// ORB ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ORB;
  class ORB_var;
  typedef ORB* ORB_ptr;
  typedef ORB_ptr ORBRef;

  class ORB : public Object {
  public:
    typedef ORB_ptr _ptr_type;
    typedef ORB_var _var_type;

    typedef char* OAid;

    typedef char* ObjectId;
    typedef String_var ObjectId_var;

    typedef _CORBA_Pseudo_Unbounded_Sequence<Request,Request_member>
      RequestSeq;

    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidName, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InconsistentTypeCode, _dyn_attr)

    ///////////////////////
    // ORB::ObjectIdList //
    ///////////////////////

    class ObjectIdList_var;

    class ObjectIdList : public _CORBA_Unbounded_Sequence_String {
    public:
      typedef ObjectIdList_var _var_type;
      inline ObjectIdList() {}
      inline ObjectIdList(const ObjectIdList& seq)
        : _CORBA_Unbounded_Sequence_String(seq) {}
      inline ObjectIdList(ULong max)
        : _CORBA_Unbounded_Sequence_String(max) {}
      inline ObjectIdList(ULong max, ULong len, char** val,
			  Boolean rel=0)
        : _CORBA_Unbounded_Sequence_String(max, len, val, rel) {}
      inline ObjectIdList& operator = (const ObjectIdList& seq) {
        _CORBA_Unbounded_Sequence_String::operator=(seq);
        return *this;
      }
    };

    class ObjectIdList_out;

    class ObjectIdList_var {
    public:
      typedef ObjectIdList T;
      typedef ObjectIdList_var T_var;

      inline ObjectIdList_var() : pd_seq(0) {}
      inline ObjectIdList_var(T* s) : pd_seq(s) {}
      inline ObjectIdList_var(const T_var& sv) {
        if( sv.pd_seq ) {
          pd_seq = new T;
          *pd_seq = *sv.pd_seq;
        } else
          pd_seq = 0;
      }
      inline ~ObjectIdList_var() { if( pd_seq ) delete pd_seq; }

      inline T_var& operator = (T* s) {
        if( pd_seq )  delete pd_seq;
        pd_seq = s;
        return *this;
      }
      inline T_var& operator = (const T_var& sv) {
        if( sv.pd_seq ) {
          if( !pd_seq )  pd_seq = new T;
          *pd_seq = *sv.pd_seq;
        } else if( pd_seq ) {
          delete pd_seq;
          pd_seq = 0;
        }
        return *this;
      }

      inline _CORBA_String_element operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
      inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
      inline operator T& () const { return *pd_seq; }
#else
      inline operator const T& () const { return *pd_seq; }
      inline operator T& () { return *pd_seq; }
#endif

      inline const T& in() const { return *pd_seq; }
      inline T&       inout()    { return *pd_seq; }
      inline T*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
      inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }

      friend class ObjectIdList_out;

    private:
      T* pd_seq;
    };

    class ObjectIdList_out {
    public:
      typedef ObjectIdList T;
      typedef ObjectIdList_var T_var;

      inline ObjectIdList_out(T*& s) : _data(s) { _data = 0; }
      inline ObjectIdList_out(T_var& sv)
        : _data(sv.pd_seq) { sv = (T*) 0; }
      inline ObjectIdList_out(const ObjectIdList_out& s) : _data(s._data) {}
      inline ObjectIdList_out& operator=(const ObjectIdList_out& s) {
	_data = s._data; return *this;
      }
      inline ObjectIdList_out& operator=(T* s) { _data = s; return *this; }
      inline operator T*&() { return _data; }
      inline T*& ptr() { return _data; }
      inline T* operator->() { return _data; }
      inline _CORBA_String_element operator [] (_CORBA_ULong i) {
	return (*_data)[i];
      }
      T*& _data;

    private:
      ObjectIdList_out();
      ObjectIdList_out operator=(const T_var&);
    };

    /////////////////
    // ORB Methods //
    /////////////////

    static _ptr_type _duplicate(_ptr_type);
    static _ptr_type _narrow(Object_ptr);
    static _ptr_type _nil();

    virtual char* object_to_string(Object_ptr) = 0;
    virtual Object_ptr string_to_object(const char*) = 0;

    virtual ObjectIdList* list_initial_services() = 0;
    virtual Object_ptr resolve_initial_references(const char* identifier) = 0;

    virtual Boolean work_pending() = 0;
    virtual void perform_work() = 0;
    virtual void run() = 0;
    virtual void shutdown(Boolean wait_for_completion) = 0;
    virtual void destroy() = 0;

    inline void NP_destroy() { destroy(); }
    // Deprecated omniORB specific method.  Use 'destroy()'.

    BOA_ptr BOA_init(int& argc, char** argv, const char* boa_identifier=0);

    Status create_list(Long, NVList_out);
    Status create_operation_list(_objref_OperationDef*, // OperationDef_ptr
				 NVList_out);
    Status create_named_value(NamedValue_out);
    Status create_exception_list(ExceptionList_out);
    Status create_context_list(ContextList_out);

    Status get_default_context(Context_out context_out);
    // Returns a reference to the default context, which should be
    // released when finished with.

    Status create_environment(Environment_out);

    Status send_multiple_requests_oneway(const RequestSeq&);
    Status send_multiple_requests_deferred(const RequestSeq&);
    Boolean poll_next_response();
    Status get_next_response(Request_out);

    TypeCode_ptr create_struct_tc(const char* id, const char* name,
				  const StructMemberSeq& members);
    TypeCode_ptr create_union_tc(const char* id, const char* name,
				 TypeCode_ptr discriminator_type,
				 const UnionMemberSeq& members);
    TypeCode_ptr create_enum_tc(const char* id, const char* name,
				const EnumMemberSeq& members);
    TypeCode_ptr create_alias_tc(const char* id, const char* name,
				 TypeCode_ptr original_type);
    TypeCode_ptr create_exception_tc(const char* id, const char* name,
				     const StructMemberSeq& members);
    TypeCode_ptr create_interface_tc(const char* id, const char* name);
    TypeCode_ptr create_string_tc(ULong bound);
    TypeCode_ptr create_wstring_tc(ULong bound);
    TypeCode_ptr create_fixed_tc(UShort digits, Short scale);
    TypeCode_ptr create_sequence_tc(ULong bound,
				    TypeCode_ptr element_type);
    TypeCode_ptr create_array_tc(ULong length, TypeCode_ptr etype);
    TypeCode_ptr create_recursive_sequence_tc(ULong bound, ULong offset);
    // deprecated
#if 0
    // Not supported yet
    TypeCode_ptr create_recursive_tc(const char* id);
#endif
    DynAny_ptr create_dyn_any(const Any& value);
    DynAny_ptr create_basic_dyn_any(TypeCode_ptr tc);
    DynStruct_ptr create_dyn_struct(TypeCode_ptr tc);
    DynSequence_ptr create_dyn_sequence(TypeCode_ptr tc);
    DynArray_ptr create_dyn_array(TypeCode_ptr tc);
    DynUnion_ptr create_dyn_union(TypeCode_ptr tc);
    DynEnum_ptr create_dyn_enum(TypeCode_ptr tc);
#if 0
    DynFixed_ptr create_dyn_fixed(TypeCode_ptr tc);
#endif

    // omniORB internal.
    static _core_attr const char* _PD_repoId;
    virtual ~ORB();

  protected:
    inline ORB(int nil) { _PR_setobj((omniObjRef*) (nil ? 0:1)); }

  private:
    ORB(const ORB&);
    ORB& operator = (const ORB&);
  };

  typedef char* ORBid;
  typedef String_var ORBid_var;

  _CORBA_MODULE_FN ORB_ptr ORB_init(int& argc, char** argv,
				    const char* orb_identifier="");


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Fixed ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Fixed {

#define OMNI_FIXED_DIGITS 31

  public:
    // Constructors
    Fixed(int val = 0);
    Fixed(unsigned val);

#ifndef OMNI_LONG_IS_INT
    Fixed(Long val);
    Fixed(ULong val);
#endif
#ifdef HAS_LongLong
    Fixed(LongLong val);
    Fixed(ULongLong val);
#endif
#ifndef NO_FLOAT
    Fixed(Double val);
#endif
#ifdef HAS_LongDouble
    Fixed(LongDouble val);
#endif
    Fixed(const Fixed& val);
    Fixed(const char* val);

    Fixed(const Octet* val, UShort digits, UShort scale, Boolean negative);
    // omniORB specific constructor

    ~Fixed();

    // Conversions
#ifdef HAS_LongLong
    operator LongLong() const;
#else
    operator Long() const;
#endif
#ifndef NO_FLOAT
#  ifdef HAS_LongDouble
    operator LongDouble() const;
#  else
    operator Double() const;
#  endif
#endif
    Fixed round   (UShort scale) const;
    Fixed truncate(UShort scale) const;

    // Operators
    Fixed&  operator= (const Fixed& val);
    Fixed&  operator+=(const Fixed& val);
    Fixed&  operator-=(const Fixed& val);
    Fixed&  operator*=(const Fixed& val);
    Fixed&  operator/=(const Fixed& val);

    Fixed&  operator++();
    Fixed   operator++(int);
    Fixed&  operator--();
    Fixed   operator--(int);
    Fixed   operator+ () const;
    Fixed   operator- () const;
    Boolean operator! () const;

    // Other member functions
    UShort fixed_digits() const { return pd_digits; }
    UShort fixed_scale()  const { return pd_scale;  }

    // omniORB specific functions

    char* NP_asString() const;
    // Return a string containing the fixed. Caller frees with
    // CORBA::string_free().

    void NP_fromString(const char* val, Boolean ignore_end = 0);
    // Set the value from the given string.
    // If ignore_end is true, do not complain about trailing garbage.

    static int NP_cmp(const Fixed& a, const Fixed& b);
    // Compare a and b. Returns -1 if a < b, 1 if a > b, 0 if a == b.


    // "Private" functions only to be called by omniORB internals.
    // Application code which uses these is asking for trouble.

    const Octet* PR_val() const { return pd_val; }
    // Return the internal value buffer. Used by arithmetic functions.

    Boolean PR_negative() const { return pd_negative; }
    // True if the value is negative, false if positive or zero.

    void PR_checkLimits();
    // Function to check that this fixed point value fits in the
    // digits/scale limits declared in the IDL. Truncates the value,
    // or throws DATA_CONVERSION if the value is too big. Does nothing
    // for base CORBA::Fixed, where there are no limits.

    void PR_setLimits(UShort idl_digits, UShort idl_scale);
    // Set and check the digits/scale limits.

    void PR_changeScale(UShort new_scale);
    // Modify the scale. Updates the number of digits if necessary.

    // Marshalling operators
    void operator>>= (cdrStream& s) const;
    void operator<<= (cdrStream& s);

  private:
    Octet   pd_val[OMNI_FIXED_DIGITS]; // Value stored least sig. digit first

    UShort  pd_digits;     // Digits and scale the number has
    UShort  pd_scale;
    Boolean pd_negative;   // True if value is negative
    UShort  pd_idl_digits; // Digits and scale the IDL says it should have
    UShort  pd_idl_scale;  //  (zero for base CORBA::Fixed).
  };


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// is_nil ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN inline Boolean is_nil(Environment_ptr p) {
    if (!Environment::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("Environment"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(Context_ptr p) {
    if (!Context::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("Context"));
  }
  _CORBA_MODULE_FN Boolean is_nil(Principal_ptr);
  _CORBA_MODULE_FN inline Boolean is_nil(Object_ptr o) {
    if( !Object::_PR_is_valid(o) )  return 0;
    if( o )                         return o->_NP_is_nil();
    else {
      // omniORB does not use a nil pointer to represent a nil object
      // reference. The program has passed in a pointer which has not
      // been initialised by CORBA::Object::_nil() or similar functions.
      // Some ORBs seems to be quite lax about this. We don't want to
      // break the applications that make this assumption. Just call
      // _CORBA_use_nil_ptr_as_nil_objref() to take note of this.
      return _CORBA_use_nil_ptr_as_nil_objref();
    }
  }
  _CORBA_MODULE_FN inline Boolean is_nil(NamedValue_ptr p) {
    if (!NamedValue::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("NamedValue"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(NVList_ptr p) {
    if (!NVList::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("NVList"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(Request_ptr p) {
    if (!Request::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("Request"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(ExceptionList_ptr p) {
    if (!ExceptionList::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("ExceptionList"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(ContextList_ptr p) {
    if (!ContextList::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("ContextList"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(TypeCode_ptr p) {
    if (!TypeCode::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("TypeCode"));
  }
  _CORBA_MODULE_FN inline Boolean is_nil(DynAny_ptr p) {
    if (!DynAny::PR_is_valid(p)) 
      return 0;
    else
      return (p ? p->NP_is_nil() :
	      _CORBA_use_nil_ptr_as_nil_pseudo_objref("DynAny"));
  }


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// release ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN inline void release(Object_ptr o) {
    if( o && !o->_NP_is_nil() ) {
      if( o->_NP_is_pseudo() )  o->_NP_decrRefCount();
      else  omni::releaseObjRef(o->_PR_getobj());
    }
  }
  _CORBA_MODULE_FN void release(Environment_ptr p);
  _CORBA_MODULE_FN void release(Context_ptr p);
  _CORBA_MODULE_FN void release(Principal_ptr);
  _CORBA_MODULE_FN void release(NamedValue_ptr p);
  _CORBA_MODULE_FN void release(NVList_ptr p);
  _CORBA_MODULE_FN void release(Request_ptr p);
  _CORBA_MODULE_FN void release(ExceptionList_ptr p);
  _CORBA_MODULE_FN void release(ContextList_ptr p);
  _CORBA_MODULE_FN void release(TypeCode_ptr o);
  _CORBA_MODULE_FN void release(DynAny_ptr d);

#include <omniORB4/CORBA_vartypes.h>

#ifdef HAS_Cplusplus_Namespace
_CORBA_MODULE_END

_CORBA_MODULE CORBA
_CORBA_MODULE_BEG
#endif

#if defined(ENABLE_CLIENT_IR_SUPPORT)
#if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB4/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB4/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB4/ir_defs.hh>
#endif
#endif

_CORBA_MODULE_END


#include <omniORB4/omniORB.h>
#include <omniORB4/proxyFactory.h>
#include <omniORB4/templatedefns.h>
#include <omniORB4/corba_operators.h>
#include <omniORB4/poa.h>
#include <omniORB4/fixed.h>


//?? These really want to be renamed and put elsewhere.
extern CORBA::Boolean
_omni_callTransientExceptionHandler(omniObjRef* obj, CORBA::ULong retries,
				    const CORBA::TRANSIENT& ex);
extern CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObjRef* obj, CORBA::ULong retries,
				      const CORBA::COMM_FAILURE& ex);
extern CORBA::Boolean
_omni_callSystemExceptionHandler(omniObjRef* obj, CORBA::ULong retries,
				 const CORBA::SystemException& ex);


extern void _omni_set_NameService(CORBA::Object_ptr);

#include <omniORB4/corbaidl_operators.hh>

#if defined(ENABLE_CLIENT_IR_SUPPORT)
#include <omniORB4/ir_operators.hh>
#endif


_CORBA_MODULE POA_CORBA
_CORBA_MODULE_BEG

#include <omniORB4/corbaidl_poa.hh>
#if defined(ENABLE_CLIENT_IR_SUPPORT)
#include <omniORB4/ir_poa.hh>
#endif

_CORBA_MODULE_END

#include <omniORB4/boa.h>

#undef _core_attr
#undef _dyn_attr


#if !defined(_OMNIORB_LIBRARY) && !defined(_OMNIORB_DYNAMIC_LIBRARY)
#ifndef USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifndef USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#include <omniORB4/Naming.hh>
#ifdef  USE_core_stub_in_nt_dll_NOT_DEFINED
#undef  USE_core_stub_in_nt_dll
#undef  USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifdef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#undef  USE_dyn_stub_in_nt_dll
#undef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#endif

// OMG COS IDLs refer to CosNaming IDL as "CosNaming.idl".
// omniORB uses the file name "Naming.idl". Any IDLs that include
// CosNaming.idl will have in their stubs #include "CosNaming.hh".
// Define the external guard for CosNaming to stop include to have
// any effect. This works because the stub generated by omniidl
// put external guards around the include.
#ifndef __CosNaming_hh_EXTERNAL_GUARD__
#define __CosNaming_hh_EXTERNAL_GUARD__
#endif

#endif // __CORBA_H__
