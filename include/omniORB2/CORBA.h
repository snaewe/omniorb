// -*- Mode: C++; -*-
//                            Package   : omniORB2
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

#if defined(_OMNIORB2_LIBRARY) && defined(_OMNIORB2_DYNAMIC_LIBRARY)
# error "_OMNIORB2_LIBRARY and _OMNIORB2_DYNAMIC_LIBRARY are both defined."
#endif

#if    defined(_OMNIORB2_LIBRARY)
#         define _core_attr
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#elif  defined(_OMNIORB2_DYNAMIC_LIBRARY)
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr
#else
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#endif

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

#include <omniORB2/omniInternal.h>
#include <omniORB2/templatedecls.h>
#include <omniORB2/stringtypes.h>

_CORBA_MODULE CORBA

_CORBA_MODULE_BEG


  // gcc can't cope with a typedef of void, so this will have to do.
  typedef void* Status;
# define RETURN_CORBA_STATUS   return 0


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// Primitive types //////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef _CORBA_Boolean Boolean;
  typedef _CORBA_Char    Char;
  typedef _CORBA_Octet   Octet;
  typedef _CORBA_Short   Short;
  typedef _CORBA_UShort  UShort;
  typedef _CORBA_Long    Long;
  typedef _CORBA_ULong   ULong;
# ifndef NO_FLOAT
  typedef _CORBA_Float   Float;
  typedef _CORBA_Double  Double;
# endif

  typedef _CORBA_Boolean& Boolean_out;
  typedef _CORBA_Char&    Char_out;
  typedef _CORBA_Octet&   Octet_out;
  typedef _CORBA_Short&   Short_out;
  typedef _CORBA_UShort&  UShort_out;
  typedef _CORBA_Long&    Long_out;
  typedef _CORBA_ULong&   ULong_out;
# ifndef NO_FLOAT
  typedef _CORBA_Float&   Float_out;
  typedef _CORBA_Double&  Double_out;
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
  ///////////////////////////////// Any ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Object;
  typedef Object* Object_ptr;
  typedef Object_ptr ObjectRef;

  class TypeCode;
  typedef class TypeCode* TypeCode_ptr;
  typedef TypeCode_ptr TypeCodeRef;


  class Any {
  public:
    Any();

    ~Any();

    Any(const Any& a);

    Any(TypeCode_ptr tc, void* value, Boolean release = 0);	

    // Marshalling operators
    void operator>>= (NetBufferedStream& s) const;
    void operator<<= (NetBufferedStream& s);

    void operator>>= (MemBufferedStream& s) const;
    void operator<<= (MemBufferedStream& s);

    size_t NP_alignedSize(size_t initialoffset) const;

    void* NP_pd() { return pd_ptr; }
    void* NP_pd() const { return pd_ptr; }

    // omniORB2 data-only marshalling functions
    void NP_marshalDataOnly(NetBufferedStream& s) const;
    void NP_unmarshalDataOnly(NetBufferedStream& s);

    void NP_marshalDataOnly(MemBufferedStream& s) const;
    void NP_unmarshalDataOnly(MemBufferedStream& s);

    size_t NP_alignedDataOnlySize(size_t initialoffset) const;

    // omniORB2 internal stub support routines
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

#if !defined(NO_FLOAT)
    void operator<<=(Float f);

    void operator<<=(Double d);
#endif

    void operator<<=(const Any& a);   // copying

    void operator<<=(Any* a);         // non-copying

    void operator<<=(TypeCode_ptr tc);

    void operator<<=(Object_ptr obj);

    void operator<<=(const char* s);	

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

    struct from_string {
      from_string(const char* s, ULong b, Boolean nocopy = 0)
	: val((char*)s), bound(b), nc(nocopy) { }
      from_string(char* s, ULong b, Boolean nocopy = 0)
	: val(s), bound(b), nc(nocopy) { }   // deprecated

      char* val;
      ULong bound;
      Boolean nc;
    };


    void operator<<=(from_boolean f);

    void operator<<=(from_char c);	

    void operator<<=(from_octet o);

    void operator<<=(from_string s);

    // OMG Extraction operators
    Boolean operator>>=(Short& s) const;

    Boolean operator>>=(UShort& u) const;

    Boolean operator>>=(Long& l) const;

    Boolean operator>>=(ULong& u) const;

#if !defined(NO_FLOAT)
    Boolean operator>>=(Float& f) const;

    Boolean operator>>=(Double& d) const;
#endif

    Boolean operator>>=(const Any*& a) const;
    Boolean operator>>=(Any*& a) const;  // deprecated
    Boolean operator>>=(Any& a) const;  // pre CORBA-2.3, obsoluted do not use.

    Boolean operator>>=(TypeCode_ptr& tc) const;

    Boolean operator>>=(const char*& s) const;
#ifndef _NO_ANY_STRING_EXTRACTION_
    Boolean operator>>=(char*& s) const; // deprecated
#endif

    struct to_boolean {
      to_boolean(Boolean& b) : ref(b) {}
      Boolean& ref;
    };

    struct to_char {
      to_char(Char& b) : ref(b) {}
      Char& ref;
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

    struct to_object {
      to_object(Object_ptr& obj) : ref(obj) { }
      Object_ptr& ref;
    };

    Boolean operator>>=(to_boolean b) const;

    Boolean operator>>=(to_char c) const;

    Boolean operator>>=(to_octet o) const;

#ifndef _NO_ANY_STRING_EXTRACTION_
    Boolean operator>>=(to_string s) const;
#endif

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
    inline Any& inout()          { return *pd_data; }
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

#if !defined(NO_FLOAT)
    inline void operator<<=(Float f) {
      *pd_data <<= f;
    }

    inline void operator<<=(Double d) {
      *pd_data <<= d;
    }
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
	
    inline void operator<<=(Any::from_boolean f) {
      *pd_data <<= f;
    }

    inline void operator<<=(Any::from_char c) {
      *pd_data <<= c;
    }
	

    inline void operator<<=(Any::from_octet o) {
      *pd_data <<= o;
    }

    inline void operator<<=(Any::from_string s){
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

#if !defined(NO_FLOAT)
    inline Boolean operator>>=(Float& f) const {
      return (*pd_data >>= f);
      }

    inline Boolean operator>>=(Double& d) const {
      return (*pd_data >>= d);
    }

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

    inline Boolean operator>>=(Any::to_boolean b) const {
      return (*pd_data >>= b);
    }

    inline Boolean operator>>=(Any::to_char c) const {
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

    virtual void _raise();
    // 'throw' a copy of self. Must be overriden by all descendants.

    static Exception* _downcast(Exception* e);
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.

    static const Exception* _downcast(const Exception* e);
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.

    static Exception* _narrow(Exception* e);
    // An equivalent operation must be provided by each descendant,
    // returning a pointer to the descendant's type.
    // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

    static Exception* _duplicate(Exception* e);
    static Exception* _NP_is_a(const Exception* e, const char* typeId);

    static inline _CORBA_Boolean PR_is_valid(Exception* p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const _CORBA_ULong PR_magic;

    typedef void (*insertExceptionToAny)    (Any&,const Exception&);
    typedef void (*insertExceptionToAnyNCP) (Any&,const Exception*);

    insertExceptionToAny insertToAnyFn() const { return pd_insertToAnyFn; }
    insertExceptionToAnyNCP insertToAnyFnNCP() const { return pd_insertToAnyFnNCP; }

    Exception(const Exception& ex) {
         pd_magic = ex.pd_magic;
	 pd_insertToAnyFn = ex.pd_insertToAnyFn;
	 pd_insertToAnyFnNCP = ex.pd_insertToAnyFnNCP;
    }

    Exception& operator=(const Exception& ex) {
         pd_magic = ex.pd_magic;
	 pd_insertToAnyFn = ex.pd_insertToAnyFn;
	 pd_insertToAnyFnNCP = ex.pd_insertToAnyFnNCP;
	 return *this;
    }

  protected:
    Exception() { pd_magic = PR_magic; 
                  pd_insertToAnyFn = 0; 
		  pd_insertToAnyFnNCP = 0;
    }

    insertExceptionToAny     pd_insertToAnyFn;
    insertExceptionToAnyNCP  pd_insertToAnyFnNCP;

  private:
    virtual Exception* _NP_duplicate() const;
    // Must be overriden by all descendants to return a new copy of
    // themselves.

    virtual const char* _NP_mostDerivedTypeId() const;
    // Returns a type identifier in the form of a string. The format is
    // internal to omniORB2. This is used to support the _downcast()
    // operation. Must be overriden by all descendants.

    _CORBA_ULong pd_magic;
  };


  enum CompletionStatus { COMPLETED_YES, COMPLETED_NO, COMPLETED_MAYBE };
  enum exception_type { NO_EXCEPTION, USER_EXCEPTION, SYSTEM_EXCEPTION };

  class SystemException : public Exception {
  public:
    SystemException() {
      pd_minor = 0;
      pd_status = COMPLETED_NO;
    }

    SystemException(const SystemException& e) : Exception(e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
    }

    SystemException(ULong minor, CompletionStatus status) {
      pd_minor = minor;
      pd_status = status;
      return;
    }

    virtual ~SystemException();

    SystemException& operator=(const SystemException& e) {
      Exception::operator=(e);
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
      return *this;
    }

#ifdef minor
    // Digital Unix 3.2, and may be others as well, defines minor() as
    // a macro in its sys/types.h. Get rid of it!
#undef minor
#endif

    ULong minor() const { return pd_minor; }
    void minor(ULong m) { pd_minor = m;    }

    CompletionStatus completed() const { return pd_status; }
    void completed(CompletionStatus s) { pd_status = s;    }

    virtual const char* NP_RepositoryId() const;

    static SystemException* _downcast(Exception*);
    static const SystemException* _downcast(const Exception*);

    static SystemException* _narrow(Exception* e);
    // NOTE: deprecated function, use _downcast.

  protected:
    ULong             pd_minor;
    CompletionStatus  pd_status;
  };

#define  STD_EXCEPTION(name) \
  class name : public SystemException { \
  public: \
    name (ULong minor = 0, CompletionStatus completed = COMPLETED_NO \
    ) : SystemException (minor,completed) { \
           pd_insertToAnyFn    = insertToAnyFn; \
           pd_insertToAnyFnNCP = insertToAnyFnNCP; \
    } \
    name( const name& ex) : SystemException(ex) {} \
    name& operator=( const name& ex) { SystemException::operator=(ex);\
                                       return *this; } \
    virtual ~name(); \
    virtual const char* NP_RepositoryId() const; \
    virtual void _raise(); \
    static name* _downcast(Exception*); \
    static const name* _downcast(const Exception*); \
    static name* _narrow(Exception*); \
    static _core_attr Exception::insertExceptionToAny    insertToAnyFn; \
    static _core_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP; \
  private: \
    virtual Exception* _NP_duplicate() const; \
    virtual const char* _NP_mostDerivedTypeId() const; \
  }

  STD_EXCEPTION (UNKNOWN);
  STD_EXCEPTION (BAD_PARAM);
  STD_EXCEPTION (NO_MEMORY);
  STD_EXCEPTION (IMP_LIMIT);
  STD_EXCEPTION (COMM_FAILURE);
  STD_EXCEPTION (INV_OBJREF);
  STD_EXCEPTION (OBJECT_NOT_EXIST);
  STD_EXCEPTION (NO_PERMISSION);
  STD_EXCEPTION (INTERNAL);
  STD_EXCEPTION (MARSHAL);
  STD_EXCEPTION (INITIALIZE);
  STD_EXCEPTION (NO_IMPLEMENT);
  STD_EXCEPTION (BAD_TYPECODE);
  STD_EXCEPTION (BAD_OPERATION);
  STD_EXCEPTION (NO_RESOURCES);
  STD_EXCEPTION (NO_RESPONSE);
  STD_EXCEPTION (PERSIST_STORE);
  STD_EXCEPTION (BAD_INV_ORDER);
  STD_EXCEPTION (TRANSIENT);
  STD_EXCEPTION (FREE_MEM);
  STD_EXCEPTION (INV_IDENT);
  STD_EXCEPTION (INV_FLAG);
  STD_EXCEPTION (INTF_REPOS);
  STD_EXCEPTION (BAD_CONTEXT);
  STD_EXCEPTION (OBJ_ADAPTER);
  STD_EXCEPTION (DATA_CONVERSION);
  STD_EXCEPTION (TRANSACTION_REQUIRED);
  STD_EXCEPTION (TRANSACTION_ROLLEDBACK);
  STD_EXCEPTION (INVALID_TRANSACTION);
  STD_EXCEPTION (WRONG_TRANSACTION);

#undef STD_EXCEPTION

  class UserException : public Exception {
  public:
    UserException() {}
    virtual ~UserException();
    static UserException* _downcast(Exception* e);
    static const UserException* _downcast(const Exception* e);
    static UserException* _narrow(Exception* e);
    // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

    UserException(const UserException& ex) : Exception(ex) {}
    UserException& operator=(const UserException& ex) { 
      Exception::operator=(ex); return *this;
    }
  };

  class UnknownUserException : public UserException {
  public:
    UnknownUserException(Any* ex);
    // Consumes <ex> which MUST be a UserException.

    UnknownUserException(const UnknownUserException& ex) : UserException(ex) {
      pd_exception = new Any(*ex.pd_exception);
    }
    
    UnknownUserException& operator=(const UnknownUserException& ex) {
      Exception::operator=(ex);
      if (pd_exception) delete pd_exception;
      pd_exception = new Any(*ex.pd_exception);
      return *this;
    }

    virtual ~UnknownUserException();

    Any& exception();

    virtual void _raise();
    virtual Exception* _NP_duplicate() const;
    virtual const char* _NP_mostDerivedTypeId() const;
    static UnknownUserException* _downcast(Exception*);
    static const UnknownUserException* _downcast(const Exception*);
    static UnknownUserException* _narrow(Exception*);
    // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

    static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
    static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

  private:
    Any* pd_exception;
  };

  class WrongTransaction : public CORBA::UserException {
  public:
    WrongTransaction() { 
      pd_insertToAnyFn    = insertToAnyFn; 
      pd_insertToAnyFnNCP = insertToAnyFnNCP;
    }
    WrongTransaction(const WrongTransaction& ex) : UserException(ex) {}
    WrongTransaction& operator=(const WrongTransaction& ex) {
      UserException::operator=(ex); return *this;
    }
    virtual ~WrongTransaction();

    virtual void _raise();
    virtual Exception* _NP_duplicate() const;
    virtual const char* _NP_mostDerivedTypeId() const;
    static WrongTransaction* _downcast(Exception*);
    static const WrongTransaction* _downcast(const Exception*);
    static WrongTransaction* _narrow(Exception*);
    // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

    static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
    static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

  };

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

    static _core_attr const _CORBA_ULong PR_magic;

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

    static _core_attr const _CORBA_ULong PR_magic;

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

    class Bounds : public UserException {
    public:
      Bounds() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      Bounds(const Bounds& ex) : UserException(ex) {}
      Bounds& operator=(const Bounds& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~Bounds();
      virtual void _raise();
      static Bounds* _downcast(Exception* e);
      static const Bounds* _downcast(const Exception* e);
      static Bounds* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    static inline _CORBA_Boolean PR_is_valid(NVList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const _CORBA_ULong PR_magic;

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
    virtual CORBA::Status set_one_value(const char*, const CORBA::Any&) = 0;
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

    // omniORB2 specifics.
    static size_t NP_alignedSize(Context_ptr ctxt, const char*const* which,
				 int whichlen, size_t initialoffset);
    static void marshalContext(Context_ptr ctxt, const char*const* which,
			       int whichlen, NetBufferedStream& s);
    static void marshalContext(Context_ptr ctxt, const char*const* which,
			       int whichlen, MemBufferedStream& s);
    static Context_ptr unmarshalContext(NetBufferedStream& s);
    static Context_ptr unmarshalContext(MemBufferedStream& s);

    static inline _CORBA_Boolean PR_is_valid(Context_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const _CORBA_ULong PR_magic;

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

    class Bounds : public UserException {
    public:
      Bounds() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      Bounds(const Bounds& ex) : UserException(ex) {}
      Bounds& operator=(const Bounds& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~Bounds();
      virtual void _raise();
      static Bounds* _downcast(Exception* e);
      static const Bounds* _downcast(const Exception* e);
      static Bounds* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    static inline _CORBA_Boolean PR_is_valid(ContextList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const _CORBA_ULong PR_magic;

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

  class PrincipalID : public _CORBA_Unbounded_Sequence__Octet {
  public:
    inline PrincipalID() {}
    inline PrincipalID(const PrincipalID& seq)
      : _CORBA_Unbounded_Sequence__Octet(seq) {}
    inline PrincipalID(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence__Octet(max) {}
    inline PrincipalID(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence__Octet(max, len, val, rel) {}
    inline PrincipalID& operator = (const PrincipalID& seq) {
      _CORBA_Unbounded_Sequence__Octet::operator=(seq);
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

    class Bounds : public UserException {
    public:
      Bounds() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      Bounds(const Bounds& ex) : UserException(ex) {}
      Bounds& operator=(const Bounds& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~Bounds();
      virtual void _raise();
      static Bounds* _downcast(Exception* e);
      static const Bounds* _downcast(const Exception* e);
      static Bounds* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    static inline _CORBA_Boolean PR_is_valid(ExceptionList_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const _CORBA_ULong PR_magic;

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
    TypeCode_member(TypeCode_ptr p) : _ptr(p) {}
    TypeCode_member(const TypeCode_member& p);

    ~TypeCode_member();

    TypeCode_member& operator=(TypeCode_ptr p);
    TypeCode_member& operator=(const TypeCode_member& p);
    TypeCode_member& operator=(const TypeCode_var& p);

    TypeCode_ptr operator->() const { return _ptr; }

    operator TypeCode_ptr() const { return _ptr; }

    TypeCode_ptr _ptr;

    void operator>>=(NetBufferedStream& s) const;
    void operator<<=(NetBufferedStream& s);
    void operator>>=(MemBufferedStream& s) const;
    void operator<<=(MemBufferedStream& s);
    size_t NP_alignedSize(size_t initialoffset) const;
  };


#ifdef HAS_Cplusplus_Namespace
_CORBA_MODULE_END

_CORBA_MODULE CORBA
_CORBA_MODULE_BEG
#endif

#if defined(_OMNIORB2_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB2/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB2_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB2/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB2/corbaidl_defs.hh>
#endif

  class InterfaceDef;

  class ImplementationDef { // Not used. 
  };
  typedef ImplementationDef* ImplementationDef_ptr;

  class OperationDef;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Object ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Request;
  typedef Request* Request_ptr;
  typedef Request_ptr RequestRef;
  typedef _CORBA_PseudoObj_Var<Request> Request_var;
  typedef _CORBA_PseudoObj_Out<Request,Request_var> Request_out;
  typedef _CORBA_PseudoObj_Member<Request,Request_var> Request_member;

  class Object_Helper {
  public:
    // omniORB2 specifics
    static Object_ptr _nil();
    static _CORBA_Boolean is_nil(Object_ptr obj);
    static void release(Object_ptr obj);
    static void duplicate(Object_ptr obj);
    static size_t NP_alignedSize(Object_ptr obj,size_t initialoffset);
    static void marshalObjRef(Object_ptr obj,NetBufferedStream& s);
    static Object_ptr unmarshalObjRef(NetBufferedStream& s);
    static void marshalObjRef(Object_ptr obj,MemBufferedStream& s);
    static Object_ptr unmarshalObjRef(MemBufferedStream& s);
  };

  class Object {
  public:

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
    InterfaceDef*           _get_interface();
    //InterfaceDef_ptr      _get_interface();
    _CORBA_Boolean _is_a(const char *repoId);
    _CORBA_Boolean _non_existent();
    _CORBA_Boolean _is_equivalent(Object_ptr other_object);
    _CORBA_ULong   _hash(_CORBA_ULong maximum);

    static Object_ptr _duplicate(Object_ptr obj);
    static Object_ptr _nil();

    Object();
    virtual ~Object();

    // omniORB2 specifics
    void NP_release();
    _CORBA_Boolean NP_is_nil() const;
    void PR_setobj(omniObject *obj);
    omniObject *PR_getobj();
    static size_t NP_alignedSize(Object_ptr obj, size_t initialoffset);
    static void marshalObjRef(Object_ptr obj, NetBufferedStream& s);
    static Object_ptr unmarshalObjRef(NetBufferedStream& s);
    static void marshalObjRef(Object_ptr obj, MemBufferedStream& s);
    static Object_ptr unmarshalObjRef(MemBufferedStream& s);
    static Object CORBA_Object_nil;
    static const _CORBA_Char* repositoryID;

    static inline _CORBA_Boolean PR_is_valid(Object_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static _core_attr const CORBA::ULong PR_magic;

  private:
    omniObject*  pd_obj;
    _CORBA_ULong pd_magic;
  };


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// TypeCode //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // omniORB2 specific TypeCode support structures, used in stubs.
  struct PR_structMember {
    const char*  name;
    TypeCode_ptr type;
  };

  typedef ULong PR_unionDiscriminator;

  struct PR_unionMember {
    const char*           name;
    TypeCode_ptr          type;
    PR_unionDiscriminator label;
  };

  enum TCKind {
    tk_null     = 0,
    tk_void     = 1,
    tk_short    = 2,
    tk_long     = 3,
    tk_ushort	= 4,
    tk_ulong	= 5,
    tk_float	= 6,
    tk_double	= 7,
    tk_boolean	= 8,
    tk_char	= 9,
    tk_octet	= 10,
    tk_any	= 11,
    tk_TypeCode	= 12,
    tk_Principal= 13,
    tk_objref	= 14,
    tk_struct	= 15,
    tk_union	= 16,
    tk_enum	= 17,
    tk_string	= 18,
    tk_sequence	= 19,
    tk_array	= 20,
    tk_alias	= 21,
    tk_except	= 22
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

    Long param_count() const;             // obsolute
    Any* parameter(Long index) const;     // obsolute

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

    class Bounds : public UserException {
    public:
      Bounds() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      Bounds(const Bounds& ex) : UserException(ex) {}
      Bounds& operator=(const Bounds& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~Bounds();
      virtual void _raise();
      static Bounds* _downcast(Exception* e);
      static const Bounds* _downcast(const Exception* e);
      static Bounds* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    class BadKind : public UserException {
    public:
      BadKind() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      BadKind(const BadKind& ex) : UserException(ex) {}
      BadKind& operator=(const BadKind& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~BadKind();
      virtual void _raise();
      static BadKind* _downcast(Exception* e);
      static const BadKind* _downcast(const Exception* e);
      static BadKind* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    static void marshalTypeCode(TypeCode_ptr obj,NetBufferedStream& s);
    static TypeCode_ptr unmarshalTypeCode(NetBufferedStream& s);
    // omniORB2 marshalling & support routines

    static void marshalTypeCode(TypeCode_ptr obj,MemBufferedStream& s);
    static TypeCode_ptr unmarshalTypeCode(MemBufferedStream& s);

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
    static TypeCode_ptr PR_octet_tc();
    static TypeCode_ptr PR_any_tc();
    static TypeCode_ptr PR_TypeCode_tc();
    static TypeCode_ptr PR_Principal_tc();
    static TypeCode_ptr PR_Object_tc();
    static TypeCode_ptr PR_string_tc();

    // omniORB internal functions
    size_t NP_alignedSize(size_t initialoffset) const;
    virtual CORBA::Boolean NP_is_nil() const;

    static inline _CORBA_Boolean PR_is_valid(TypeCode_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }
    static _core_attr const CORBA::ULong PR_magic;

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
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_octet;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_any;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_TypeCode;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Principal;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Object;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_string;
  _CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_NamedValue;

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

    class Invalid : public UserException {
    public:
      Invalid() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      Invalid(const Invalid& ex) : UserException(ex) {}
      Invalid& operator=(const Invalid& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~Invalid();
      virtual void _raise();
      static Invalid* _downcast(Exception* e);
      static const Invalid* _downcast(const Exception* e);
      static Invalid* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    class InvalidValue : public UserException {
    public:
      InvalidValue() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InvalidValue(const InvalidValue& ex) : UserException(ex) {}
      InvalidValue& operator=(const InvalidValue& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~InvalidValue();
      virtual void _raise();
      static InvalidValue* _downcast(Exception* e);
      static const InvalidValue* _downcast(const Exception* e);
      static InvalidValue* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    class TypeMismatch : public UserException {
    public:
      TypeMismatch() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      TypeMismatch(const TypeMismatch& ex) : UserException(ex) {}
      TypeMismatch& operator=(const TypeMismatch& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~TypeMismatch();
      virtual void _raise();
      static TypeMismatch* _downcast(Exception* e);
      static const TypeMismatch* _downcast(const Exception* e);
      static TypeMismatch* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    class InvalidSeq : public UserException {
    public:
      InvalidSeq() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InvalidSeq(const InvalidSeq& ex) : UserException(ex) {}
      InvalidSeq& operator=(const InvalidSeq& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~InvalidSeq();
      virtual void _raise();
      static InvalidSeq* _downcast(Exception* e);
      static const InvalidSeq* _downcast(const Exception* e);
      static InvalidSeq* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    typedef _CORBA_Unbounded_Sequence__Octet OctetSeq;

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
    virtual void insert_short(Short value) = 0;
    virtual void insert_ushort(UShort value) = 0;
    virtual void insert_long(Long value) = 0;
    virtual void insert_ulong(ULong value) = 0;
#ifndef NO_FLOAT
    virtual void insert_float(Float value) = 0;
    virtual void insert_double(Double value) = 0;
#endif
    virtual void insert_string(const char* value) = 0;
    virtual void insert_reference(Object_ptr value) = 0;
    virtual void insert_typecode(TypeCode_ptr value) = 0;
    virtual void insert_any(const Any& value) = 0;
    virtual Boolean get_boolean() = 0;
    virtual Octet get_octet() = 0;
    virtual Char get_char() = 0;
    virtual Short get_short() = 0;
    virtual UShort get_ushort() = 0;
    virtual Long get_long() = 0;
    virtual ULong get_ulong() = 0;
#ifndef NO_FLOAT
    virtual Float get_float() = 0;
    virtual Double get_double() = 0;
#endif
    virtual char* get_string() = 0;
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

    static _core_attr const _CORBA_ULong PR_magic;
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

    static _core_attr const _CORBA_ULong PR_magic;

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

  class ServerRequest;
  typedef ServerRequest* ServerRequest_ptr;
  typedef ServerRequest_ptr ServerRequestRef;

  class ServerRequest {
  public:
    virtual ~ServerRequest();

    virtual const char* op_name() = 0;
    virtual OperationDef* op_def() = 0;
    // virtual OperationDef_ptr op_def() = 0;
    virtual Context_ptr ctx() = 0;
    virtual void params(NVList_ptr parameters) = 0;
    virtual void result(Any* value) = 0;
    virtual void exception(Any* value) = 0;

    static ServerRequest_ptr _duplicate(ServerRequest_ptr);
    static ServerRequest_ptr _nil();

  protected:
    ServerRequest() {}

  private:
    ServerRequest(const ServerRequest&);
    ServerRequest& operator=(const ServerRequest&);
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// BOA ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class BOA;
  typedef class BOA* BOA_ptr;
  typedef BOA_ptr BOARef;

  class ReferenceData : public _CORBA_Unbounded_Sequence__Octet {
  public:
    inline ReferenceData() {}
    inline ReferenceData(const ReferenceData& seq)
      : _CORBA_Unbounded_Sequence__Octet(seq) {}
    inline ReferenceData(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence__Octet(max) {}
    inline ReferenceData(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence__Octet(max, len, val, rel) {}
    inline ReferenceData& operator = (const ReferenceData& seq) {
      _CORBA_Unbounded_Sequence__Octet::operator=(seq);
      return *this;
    };
  };


  class BOA {
  public:
    Object_ptr create(const ReferenceData&, InterfaceDef*,
		      ImplementationDef_ptr);
    ReferenceData* get_id(Object_ptr);
    void change_implementation(Object_ptr,ImplementationDef_ptr);
    Principal_ptr get_principal(Object_ptr, Environment_ptr);
    void deactivate_impl(ImplementationDef_ptr);
    void deactivate_obj(Object_ptr);

    void dispose(Object_ptr);
    // see omni::disposeObject()

    void impl_is_ready(ImplementationDef_ptr p=0, Boolean NonBlocking=0);
    // The argument <NonBlocking> is omniORB2 specific.
    // Calling this function will cause the BOA to start accepting requests
    // from other address spaces.

    void impl_shutdown();
    // omniORB2 specific.
    // This is the reverse of impl_is_ready().
    // When this call returns, all the internal threads and network
    // connections will be shutdown.
    // Any thread blocking on impl_is_ready is unblocked.
    // When this call returns, requests from other address spaces will not
    // be dispatched.
    // The BOA can be reactivated by impl_is_ready(), it will continue to use
    // the existing network addresses when reactivated.
    //
    // Note: This function should not be called in the implementation of a
    //       CORBA interface. Otherwise, this call will be blocked
    //       indefinitely waiting on itself to complete the request.


    void destroy();
    // omniORB2 specific.
    // Calling this function will destroy this BOA. The function will call
    // impl_shutdown() implicitly if it has not been called. When this call
    // returns, the network addresses (endpoints) where this BOA listens on
    // will be freed.
    // Note: After this call, the BOA should not be used directly or
    //       indirectly, otherwise the behaviour is undefined. If there is
    //       any object implementation still registered with the BOA when this
    //       function is called, the object implementation should not be called
    //       afterwards. This function does not call the dispose method of
    //       the implementations.
    // Note: Initialisation of another BOA using ORB::BOA_init() is not
    //       supported. The behaviour of ORB::BOA_init() after this function
    //       is called is undefined.

    void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0);

    static BOA_ptr _duplicate(BOA_ptr);
    static BOA_ptr _nil();

    static BOA_ptr getBOA();
    // omniORB2 specific. Must be called after BOA_init is called.
    // Otherwise a CORBA::OBJ_ADAPTOR exception is raised.

    static inline _CORBA_Boolean PR_is_valid(BOA_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }

    static const _CORBA_ULong PR_magic;

    BOA();
    ~BOA();

    class DynamicImplementation;
    typedef DynamicImplementation* DynamicImplementation_ptr;

    class DynamicImplementation {
    public:
      DynamicImplementation();
      virtual ~DynamicImplementation();

      virtual void invoke(ServerRequest_ptr request,
			  Environment& env) throw() = 0;

      void* NP_pd() { return pd; }
      // omniORB internal

    protected:
      Object_ptr _this();
      // Must only be called from within invoke(). Caller must release
      // the reference returned.

      BOA_ptr _boa();
      // Must only be called from within invoke(). Caller must NOT
      // release the reference returned.

    private:
      void* pd;
    };

    Object_ptr create_dynamic_object(DynamicImplementation_ptr dir,
				     const char* intfRepoId);
    // Creates an object reference that will use the given dynamic
    // implementation for its operations. The application must provide
    // the interface repository id of the interface implemented by the
    // dynamic implementation routine (dir).
    //  The object itself takes over ownership of <dir>, and will call
    // <delete> on it when it is destroyed. To receive operation
    // invocations the object must first be registered with the BOA by
    // calling obj_is_ready().
  private:
      _CORBA_ULong pd_magic;

  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// Service ////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  typedef UShort ServiceType;
  typedef ULong  ServiceOption;
  typedef ULong  ServiceDetailType;

  _CORBA_MODULE_VARINT const ServiceType Security _init_in_decl_( = 1);

  struct ServiceDetail {
    ServiceDetailType service_detail_type;
    _CORBA_PseudoValue_Sequence<_CORBA_Octet> service_detail;

  };

  struct ServiceInformation {
    _CORBA_PseudoValue_Sequence<ServiceOption> service_options;
    _CORBA_PseudoValue_Sequence<ServiceDetail> service_details;
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// Current ////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  class Current;
  typedef class Current* Current_ptr;
  typedef Current_ptr CurrentRef;

  class Current {
  public:
    static Current_ptr _duplicate(Current_ptr p);
    static Current_ptr _nil();
  private:
    Current();
  };
  _CORBA_MODULE_FN Boolean is_nil(Current_ptr p);
  _CORBA_MODULE_FN void release(Current_ptr);

  typedef _CORBA_PseudoObj_Var<Current> Current_var;

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// Policy  ////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  typedef ULong PolicyType;

  class Policy;
  typedef class Policy* Policy_ptr;
  typedef Policy_ptr PolicyRef;

  class Policy {
  public:
    PolicyType policy_type();
    Policy_ptr copy();
    void       destroy();

    static Policy_ptr _duplicate(Policy_ptr p);
    static Policy_ptr _nil();
  private:
    Policy();
  };
  _CORBA_MODULE_FN Boolean is_nil(Policy_ptr p);
  _CORBA_MODULE_FN void release(Policy_ptr);

  typedef _CORBA_PseudoObj_Var<Policy> Policy_var;
  typedef _CORBA_PseudoObj_Member<Policy,Policy_var> Policy_member;
  typedef _CORBA_Pseudo_Unbounded_Sequence<Policy,Policy_member> PolicyList;

  _CORBA_MODULE_VARINT const PolicyType SecConstruction _init_in_decl_( = 11);

  class ConstructionPolicy;
  typedef class ConstructionPolicy* ConstructionPolicy_ptr;
  typedef ConstructionPolicy_ptr ConstructionPolicyRef;

  class ConstructionPolicy : public Policy {
  public:
    void make_domain_manager(InterfaceDef* object_type, // InterfaceDef_ptr
			     Boolean constr_policy);

    static ConstructionPolicy_ptr _duplicate(ConstructionPolicy_ptr p);
    static ConstructionPolicy_ptr _nil();
  private:
    ConstructionPolicy();
  };
  _CORBA_MODULE_FN Boolean is_nil(ConstructionPolicy_ptr p);
  _CORBA_MODULE_FN void release(ConstructionPolicy_ptr);

  typedef _CORBA_PseudoObj_Var<ConstructionPolicy> ConstructionPolicy_var;

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
  typedef _CORBA_PseudoObj_Member<DomainManager,DomainManager_var> DomainManager_member;

  typedef _CORBA_Pseudo_Unbounded_Sequence<DomainManager,DomainManager_member> DomainManagerList;


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// ORB ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ORB;
  typedef class ORB* ORB_ptr;
  typedef ORB_ptr ORBRef;

  class ORB  {
  public:

    char* object_to_string(Object_ptr);
    Object_ptr string_to_object(const char*);
    char* object_to_string(DynAny_ptr);

    typedef _CORBA_Pseudo_Unbounded_Sequence<Request,Request_member> RequestSeq;

    Status create_list(Long, NVList_out);
    Status create_operation_list(OperationDef*, // OperationDef_ptr
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

    typedef char* OAid;

    BOA_ptr BOA_init(int &argc, char** argv, const char *boa_identifier=0);

    // Initial Object Reference definitions:

    typedef char* ObjectId;
    typedef String_var ObjectId_var;

    class ObjectIdList : public _CORBA_Unbounded_Sequence__String {
    public:
      inline ObjectIdList() {}
      inline ObjectIdList(const ObjectIdList& seq)
	: _CORBA_Unbounded_Sequence__String(seq) {}
      inline ObjectIdList(CORBA::ULong max)
	: _CORBA_Unbounded_Sequence__String(max) {}
      inline ObjectIdList(CORBA::ULong max, CORBA::ULong len, char** val, CORBA::Boolean rel=0)
	: _CORBA_Unbounded_Sequence__String(max, len, val, rel) {}
      inline ObjectIdList& operator = (const ObjectIdList& seq) {
	_CORBA_Unbounded_Sequence__String::operator=(seq);
	return *this;
      };
    };

    class ObjectIdList_out;

    class ObjectIdList_var {
    public:
      typedef ObjectIdList _T;
      typedef ObjectIdList_var _T_var;

      inline ObjectIdList_var() : pd_seq(0) {}
      inline ObjectIdList_var(_T* s) : pd_seq(s) {}
      inline ObjectIdList_var(const _T_var& sv) {
	if( sv.pd_seq ) {
	  pd_seq = new _T;
	  *pd_seq = *sv.pd_seq;
	} else
	  pd_seq = 0;
      }
      inline ~ObjectIdList_var() {
	if( pd_seq )  delete pd_seq;
      }

      inline _T_var& operator = (_T* s) {
	if( pd_seq )  delete pd_seq;
	pd_seq = s;
	return *this;
      }
      inline _T_var& operator = (const _T_var& sv) {
	if( sv.pd_seq ) {
	  if( !pd_seq )  pd_seq = new _T;
	  *pd_seq = *sv.pd_seq;
	} else if( pd_seq ) {
	  delete pd_seq;
	  pd_seq = 0;
	}
	return *this;
      }

      inline _CORBA_String_member operator [] (_CORBA_ULong i) {
	return (*pd_seq)[i];
      }
      inline _T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
      inline operator _T& () const { return *pd_seq; }
#else
      inline operator const _T& () const { return *pd_seq; }
      inline operator _T& () { return *pd_seq; }
#endif

      inline const _T& in() const { return *pd_seq; }
      inline _T& inout() { return *pd_seq; }
      inline _T*& out() { return pd_seq; }
      inline _T* _retn() { _T* tmp = pd_seq; pd_seq = 0; return tmp; }
      
      friend class ObjectIdList_out;
      
    private:
      _T* pd_seq;
    };

    class ObjectIdList_out {
    public:
      typedef ObjectIdList _T;
      typedef ObjectIdList_var _T_var;

      inline ObjectIdList_out(_T*& s) : _data(s) {}
      inline ObjectIdList_out(_T_var& sv)
	: _data(sv.pd_seq) { sv = (_T*) 0; }

      _T*& _data;

    private:
      ObjectIdList_out();
    };

    class InvalidName : public UserException {
    public:
      InvalidName() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InvalidName(const InvalidName& ex) : UserException(ex) {}
      InvalidName& operator=(const InvalidName& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~InvalidName();
      virtual void _raise();
      static InvalidName* _downcast(Exception* e);
      static const InvalidName* _downcast(const Exception* e);
      static InvalidName* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _core_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _core_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

    ObjectIdList* list_initial_services();
    Object_ptr resolve_initial_references(const char* identifier);

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

    TypeCode_ptr create_sequence_tc(ULong bound, TypeCode_ptr element_type);

    TypeCode_ptr create_array_tc(ULong length, TypeCode_ptr element_type);

    TypeCode_ptr create_recursive_sequence_tc(ULong bound, ULong offset);
      // deprecated

#if 0
    // Not supported yet
    TypeCode_ptr create_recusive_tc(const char* id);  
#endif

    class InconsistentTypeCode : public UserException {
    public:
      InconsistentTypeCode() { 
	pd_insertToAnyFn    = insertToAnyFn; 
	pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InconsistentTypeCode(const InconsistentTypeCode& ex)
	: UserException(ex) {}
      InconsistentTypeCode& operator=(const InconsistentTypeCode& ex) {
	UserException::operator=(ex); return *this;
      }
      virtual ~InconsistentTypeCode();
      virtual void _raise();
      static InconsistentTypeCode* _downcast(Exception* e);
      static const InconsistentTypeCode* _downcast(const Exception* e);
      static InconsistentTypeCode* _narrow(Exception* e);
      // NOTE: deprecated function from CORBA 2.2. Same as _downcast.

      static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
      static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;
    private:
      virtual Exception* _NP_duplicate() const;
      virtual const char* _NP_mostDerivedTypeId() const;
    };

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

    Boolean get_service_information(ServiceType service_type,
				    ServiceInformation*& service_information);

    void NP_destroy();
    // omniORB2 specific
    // Calling this function would destroy the ORB runtime. After this function
    // is called, the ORB_ptr should not be used.


    static ORB_ptr _duplicate(ORB_ptr p);
    static ORB_ptr _nil();

    static inline _CORBA_Boolean PR_is_valid(ORB_ptr p ) {
      return ((p) ? (p->pd_magic == PR_magic) : 1);
    }
    static _core_attr const CORBA::ULong PR_magic;

    ORB();
    ~ORB();
  private:
    _CORBA_ULong pd_magic;
  };

  typedef char *ORBid;
  _CORBA_MODULE_FN ORB_ptr ORB_init(int& argc, char** argv,
				    const char* orb_identifier);

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
    if (!Object::PR_is_valid(o)) 
      return 0;
    else if (o)
      return o->NP_is_nil();
    else {
      // omniORB2 does not use a nil pointer to represent a nil object
      // reference. The program has passed in a pointer which has not
      // been initialised by CORBA::Object::_nil() or similar functions.
      // Some ORBs seems to be quite lax about this. We don't want to
      // break the applications that make this assumption. Just call
      // _CORBA_use_nil_ptr_as_nil_objref() to take note of this.
      return _CORBA_use_nil_ptr_as_nil_objref();
    }
  }
  _CORBA_MODULE_FN Boolean is_nil(BOA_ptr p);
  _CORBA_MODULE_FN Boolean is_nil(ORB_ptr p);
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

  _CORBA_MODULE_FN void release(Environment_ptr p);
  _CORBA_MODULE_FN void release(Context_ptr p);
  _CORBA_MODULE_FN void release(Principal_ptr);
  _CORBA_MODULE_FN inline void release(Object_ptr o) {
    // see also omni::objectRelease()
    if (CORBA::Object::PR_is_valid(o) && !CORBA::is_nil(o))
      o->NP_release();
    return;
  }
  _CORBA_MODULE_FN void release(BOA_ptr);
  _CORBA_MODULE_FN void release(ORB_ptr);
  _CORBA_MODULE_FN void release(NamedValue_ptr p);
  _CORBA_MODULE_FN void release(NVList_ptr p);
  _CORBA_MODULE_FN void release(Request_ptr p);
  _CORBA_MODULE_FN void release(ExceptionList_ptr p);
  _CORBA_MODULE_FN void release(ContextList_ptr p);
  _CORBA_MODULE_FN void release(TypeCode_ptr o);
  _CORBA_MODULE_FN void release(DynAny_ptr d);


  // omniORB2 private functions.
  _CORBA_MODULE_FN Object_ptr UnMarshalObjRef(const char *repoId,
					      NetBufferedStream& s);
  _CORBA_MODULE_FN void MarshalObjRef(Object_ptr obj, const char *repoId,
				      size_t repoIdSize, NetBufferedStream& s);
  _CORBA_MODULE_FN size_t AlignedObjRef(Object_ptr obj, const char *repoId,
					size_t repoIdSize,
					size_t initialoffset);
  _CORBA_MODULE_FN Object_ptr UnMarshalObjRef(const char *repoId,
					      MemBufferedStream& s);
  _CORBA_MODULE_FN void MarshalObjRef(Object_ptr obj, const char* repoId,
				      size_t repoIdSize, MemBufferedStream& s);


  class Object_member;
  class Object_INOUT_arg;
  class Object_OUT_arg;

  class Object_var {
  public:
    inline Object_var() { pd_objref = CORBA::Object::_nil(); }
    inline Object_var(Object_ptr p) { pd_objref = p; }
    inline ~Object_var() {
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
    }
    inline Object_var(const Object_var& p) {
      if (!CORBA::is_nil(p.pd_objref)) {
	pd_objref = CORBA::Object::_duplicate(p.pd_objref);
      }
      else
	pd_objref = CORBA::Object::_nil();
    }
    inline Object_var(const Object_member& p);
    inline Object_var& operator= (Object_ptr p) {
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
      pd_objref = p;
      return *this;
    }
    inline Object_var& operator= (const Object_var& p) {
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
      if (!CORBA::is_nil(p.pd_objref)) {
	pd_objref = CORBA::Object::_duplicate(p.pd_objref);
      }
      else
	pd_objref = CORBA::Object::_nil();
      return *this;
    }
    inline Object_var& operator= (const Object_member& p);

    inline Object_ptr operator->() const { return pd_objref; }

    inline operator Object_ptr() const { return pd_objref; }

    inline const Object_ptr in() const { return pd_objref; }
    inline Object_ptr& inout()         { return pd_objref; }
    inline Object_ptr& out() {
      if( !CORBA::is_nil(pd_objref) ) {
	CORBA::release(pd_objref);
	pd_objref = CORBA::Object::_nil();
      }
      return pd_objref;
    }
    inline Object_ptr _retn() {
      Object_ptr tmp = pd_objref;
      pd_objref = CORBA::Object::_nil();
      return tmp;
    }

    friend class Object_INOUT_arg;
    friend class Object_OUT_arg;

private:
    Object_ptr pd_objref;
  };

  // omniORB2 private class
  class Object_member {
  public:
    inline Object_member() : pd_rel(1), _ptr(pd_data) { 
      pd_data = CORBA::Object::_nil();
    }
    inline Object_member(Object_ptr& p, _CORBA_Boolean rel) 
      : pd_rel(rel), _ptr(p) {
      pd_data = CORBA::Object::_nil();
    }

    inline Object_member(Object_ptr p) : pd_data(p), pd_rel(1), _ptr(pd_data) {}
    inline Object_member(const Object_member& p) : pd_rel(1), _ptr(pd_data) {
      if (!CORBA::is_nil(p._ptr)) {
	_ptr = CORBA::Object::_duplicate(p._ptr);
      }
      else
	_ptr = CORBA::Object::_nil();
    }
    inline ~Object_member() {
      if (pd_rel && !CORBA::is_nil(pd_data)) CORBA::release(pd_data);
      // Not _ptr! Only call release when this is not a sequence member
    }
    inline Object_member& operator= (Object_ptr p) {
      if (pd_rel && !CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      _ptr = p;
      return *this;
    }
    inline Object_member& operator= (const Object_member& p) {
      if (pd_rel && !CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      if (!CORBA::is_nil(p._ptr)) {
	_ptr = CORBA::Object::_duplicate(p._ptr);
      }
      else
	_ptr = CORBA::Object::_nil();
      return *this;
    }
    inline Object_member& operator= (const Object_var& p) {
      if (pd_rel && !CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      if (!CORBA::is_nil(p)) {
	_ptr = CORBA::Object::_duplicate(p);
      }
      else
	_ptr = CORBA::Object::_nil();
      return *this;
    }

    inline size_t NP_alignedSize(size_t initialoffset) const {
      return CORBA::Object::NP_alignedSize(_ptr,initialoffset);
    }
    inline void operator>>= (NetBufferedStream& s) const {
      CORBA::Object::marshalObjRef(_ptr,s);
    }
    inline void operator<<= (NetBufferedStream& s) {
      Object_ptr _result = CORBA::Object::unmarshalObjRef(s);
      CORBA::release(_ptr);
      _ptr = _result;
    }
    inline void operator>>= (MemBufferedStream& s) const {
      CORBA::Object::marshalObjRef(_ptr,s);
    }
    inline void operator<<= (MemBufferedStream& s) {
      Object_ptr _result = CORBA::Object::unmarshalObjRef(s);
      CORBA::release(_ptr);
      _ptr = _result;
    }

    inline Object_ptr operator->() const { return (Object_ptr)_ptr; }
    inline operator Object_ptr () const { return _ptr; }

    // omniORB private
    inline Object_ptr _retn() {
      Object_ptr tmp;
      if (!pd_rel) {
	CORBA::Object::_duplicate(_ptr);
      }
      tmp = _ptr;
      _ptr = CORBA::Object::_nil();
      return tmp;
    }

  private:
    Object_ptr pd_data;

  public:
    _CORBA_Boolean pd_rel;
    Object_ptr& _ptr;
  };

  // omniORB2 private class
  class Object_INOUT_arg {
  public:
    inline Object_INOUT_arg(Object_ptr& p) : _data(p) {}
    inline Object_INOUT_arg(Object_var& p) : _data(p.pd_objref) {}
    inline Object_INOUT_arg(Object_member& p) : _data(p._ptr) {
      // If the Object_member is part of a sequence and the pd_rel == 0,
      // the ObjRef is not owned by the sequence and should not
      // be freed. Since this is an inout argument and the callee may call
      // release, we duplicate the ObjRef and pass it to the callee. This will
      // result in a memory leak! This only occurs when there is a programming
      // error and cannot be trapped by the compiler.
      if (!p.pd_rel && !CORBA::is_nil(p._ptr) ) {
	CORBA::Object::_duplicate(p);
      }
    }
    inline ~Object_INOUT_arg() {}

    Object_ptr& _data;

  private:
    Object_INOUT_arg();
  };

  // omniORB2 private class
  class Object_OUT_arg {
  public:
    inline Object_OUT_arg(Object_ptr& p) : _data(p) { _data = CORBA::Object::_nil(); }
    inline Object_OUT_arg(Object_var& p) : _data(p.pd_objref) {
      p = CORBA::Object::_nil();
    }
    inline Object_OUT_arg(Object_member& p) : _data(p._ptr) {
      p = CORBA::Object::_nil();
    }
    inline ~Object_OUT_arg() {}

    inline Object_OUT_arg& operator=(const Object_OUT_arg& p) { 
      _data = p._data; return *this;
    }
    inline Object_OUT_arg& operator=(Object_ptr p) { _data = p; return *this; }
    inline Object_OUT_arg& operator=(const Object_var& p) { 
      _data = CORBA::Object::_duplicate(p); return *this;
    }
    inline Object_OUT_arg& operator=(const Object_member& p) {
      _data = CORBA::Object::_duplicate(p); return * this;
    }
    inline operator Object_ptr&() { return _data; }
    inline Object_ptr& ptr() { return _data; }
    inline Object_ptr operator->() { return _data; }

    Object_ptr& _data;

  private:
    Object_OUT_arg();
  };

  typedef Object_OUT_arg Object_out;


  class TypeCode_INOUT_arg;
  class TypeCode_OUT_arg;

  class TypeCode_var {
  public:
    inline TypeCode_var() { pd_TC = CORBA::TypeCode::_nil(); }

    inline TypeCode_var(TypeCode_ptr p) { pd_TC = p; }

    inline TypeCode_var(const TypeCode_var& p) { 
	pd_TC = CORBA::TypeCode::_duplicate(p.pd_TC); 
    }
    TypeCode_var(const TypeCode_member& p) {
      pd_TC = CORBA::TypeCode::_duplicate(p._ptr);
    }
    ~TypeCode_var() { CORBA::release(pd_TC); }

    inline TypeCode_var& operator= (TypeCode_ptr p) {
      CORBA::release(pd_TC);
      pd_TC = p;
      return *this;
    }

    inline TypeCode_var& operator= (const TypeCode_var& p) {
      if (this != &p)
	{
	  CORBA::release(pd_TC);
	  pd_TC = CORBA::TypeCode::_duplicate(p.pd_TC);
	}
      return *this;
    }

    inline TypeCode_var& operator=(const TypeCode_member& p) {
      CORBA::release(pd_TC);
      pd_TC = CORBA::TypeCode::_duplicate(p._ptr);
      return *this;
    }

    TypeCode_ptr operator->() const { return (TypeCode_ptr) pd_TC; }
    operator TypeCode_ptr() const { return pd_TC; }

    TypeCode_ptr  in() const   { return pd_TC; }
    TypeCode_var& inout()      { return *this; }
    TypeCode_ptr& out() {
      CORBA::release(pd_TC);
      pd_TC = TypeCode::_nil();
      return pd_TC;
    }
    TypeCode_ptr _retn() {
      TypeCode_ptr tmp = pd_TC;
      pd_TC = CORBA::TypeCode::_nil();
      return tmp;
    }

    friend class TypeCode_member;
    friend class TypeCode_INOUT_arg;
    friend class TypeCode_OUT_arg;

  private:
    TypeCode_ptr pd_TC;
  };


  class TypeCode_INOUT_arg {
  public:
    inline TypeCode_INOUT_arg(TypeCode_ptr& p) : _data(p) { }
    inline TypeCode_INOUT_arg(TypeCode_var& p) : _data(p.pd_TC) { }
    inline TypeCode_INOUT_arg(TypeCode_member& p) : _data(p._ptr) { }
    inline ~TypeCode_INOUT_arg() { }

    TypeCode_ptr& _data;

  private:
    TypeCode_INOUT_arg();
  };


  class TypeCode_OUT_arg {
  public:
    TypeCode_OUT_arg(TypeCode_ptr& p) : _data(p) { }
    TypeCode_OUT_arg(TypeCode_var& p) : _data(p.pd_TC) {
      p = CORBA::TypeCode::_nil();
    }
    inline TypeCode_OUT_arg(TypeCode_member& p) : _data(p._ptr) {
      p = CORBA::TypeCode::_nil();
    }
    inline ~TypeCode_OUT_arg() { }

    TypeCode_ptr& _data;

  private:
    TypeCode_OUT_arg();
  };


  class BOA_var {
  public:
    inline BOA_var() : pd_boaref(BOA::_nil()) {}
    inline BOA_var(BOA_ptr p) { pd_boaref = p; }
    inline ~BOA_var() {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
    }
    inline BOA_var(const BOA_var& p) {
      pd_boaref = BOA::_duplicate(p.pd_boaref);
    }
    inline BOA_var& operator= (BOA_ptr p) {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
      pd_boaref = p;
      return *this;
    }
    inline BOA_var& operator= (const BOA_var& p) {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
      pd_boaref = BOA::_duplicate(p.pd_boaref);
      return *this;
    }
    inline BOA_ptr operator->() const { return (BOA_ptr)pd_boaref; }
    inline operator BOA_ptr() const { return pd_boaref; }

  private:
    BOA_ptr pd_boaref;
  };


  class ORB_var {
  public:
    inline ORB_var() : pd_orbref(ORB::_nil()) {}
    inline ORB_var(ORB_ptr p) { pd_orbref = p; }
    inline ~ORB_var() {
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
    }
    inline ORB_var(const ORB_var& p) {
      pd_orbref = ORB::_duplicate(p.pd_orbref);
    }
    inline ORB_var& operator= (ORB_ptr p) {
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
      pd_orbref = p;
      return *this;
    }
    inline ORB_var& operator= (const ORB_var& p) {
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
      pd_orbref = ORB::_duplicate(p.pd_orbref);
      return *this;
    }
    inline ORB_ptr operator->() const { return (ORB_ptr)pd_orbref; }
    inline operator ORB_ptr() const { return pd_orbref; }

  private:
    ORB_ptr pd_orbref;
  };


  // DynAny interface _var types.
  typedef _CORBA_PseudoObj_Var<DynAny>      DynAny_var;
  typedef _CORBA_PseudoObj_Var<DynEnum>     DynEnum_var;
  typedef _CORBA_PseudoObj_Var<DynStruct>   DynStruct_var;
  typedef _CORBA_PseudoObj_Var<DynUnion>    DynUnion_var;
  typedef _CORBA_PseudoObj_Var<DynSequence> DynSequence_var;
  typedef _CORBA_PseudoObj_Var<DynArray>    DynArray_var;

  // DynAny interface _member types.
  typedef _CORBA_PseudoObj_Member<DynAny,DynAny_var>       DynAny_member;
  typedef _CORBA_PseudoObj_Member<DynEnum,DynEnum_var>     DynEnum_member;
  typedef _CORBA_PseudoObj_Member<DynStruct,DynStruct_var> DynStruct_member;
  typedef _CORBA_PseudoObj_Member<DynUnion,DynUnion_var>   DynUnion_member;
  typedef _CORBA_PseudoObj_Member<DynSequence,DynSequence_var>
                                                           DynSequence_member;
  typedef _CORBA_PseudoObj_Member<DynArray,DynArray_var>   DynArray_member;

  // DynAny interface _out types.
  typedef _CORBA_PseudoObj_Out<DynAny,DynAny_var>           DynAny_out;
  typedef _CORBA_PseudoObj_Out<DynEnum,DynEnum_var>         DynEnum_out;
  typedef _CORBA_PseudoObj_Out<DynStruct,DynStruct_var>     DynStruct_out;
  typedef _CORBA_PseudoObj_Out<DynUnion,DynUnion_var>       DynUnion_out;
  typedef _CORBA_PseudoObj_Out<DynSequence,DynSequence_var> DynSequence_out;
  typedef _CORBA_PseudoObj_Out<DynArray,DynArray_var>       DynArray_out;


  // omniORB2 specific classes for creating proxy objects.
  class proxyObjectFactory_iterator;
  class proxyObjectFactory {
  public:
    proxyObjectFactory();
    virtual ~proxyObjectFactory();
  
    virtual const char *irRepoId() const = 0;
    // returns the Interface Repository ID.
       
    virtual CORBA::Object_ptr newProxyObject(Rope *r,
					     _CORBA_Octet *key,
					     size_t keysize,
					     IOP::TaggedProfileList *profiles,
					     _CORBA_Boolean release) = 0;
    // returns a new proxy object.

    virtual _CORBA_Boolean  is_a(const char *base_repoId) const = 0;
    // Return true if <base_repoId> is the interface repository ID of
    // a base interface.

    friend class proxyObjectFactory_iterator;

    static _core_attr proxyObjectFactory* proxyStubs;

  private:
    proxyObjectFactory *pd_next;
  };


  class proxyObjectFactory_iterator {
  public:
    proxyObjectFactory_iterator();
    ~proxyObjectFactory_iterator() {}
    proxyObjectFactory *operator() ();
  private:
    proxyObjectFactory *pd_f;
  };

#ifdef HAS_Cplusplus_Namespace
_CORBA_MODULE_END

_CORBA_MODULE CORBA
_CORBA_MODULE_BEG
#endif

#if defined(_OMNIORB2_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB2/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB2_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB2/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB2/ir_defs.hh>
#endif

_CORBA_MODULE_END


#include <omniORB2/omniORB.h>
#include <omniORB2/templatedefns.h>
#include <omniORB2/corba_operators.h>

// omniORB2 private functions
extern CORBA::Boolean
_omni_defaultTransientExceptionHandler(void* cookie,
				       CORBA::ULong n_retries,
				       const CORBA::TRANSIENT& ex);

extern CORBA::Boolean
_omni_defaultCommFailureExceptionHandler(void* cookie,
					 CORBA::ULong n_retries,
					 const CORBA::COMM_FAILURE& ex);

extern CORBA::Boolean
_omni_defaultSystemExceptionHandler(void* cookie,
				    CORBA::ULong n_retries,
				    const CORBA::SystemException& ex);

extern CORBA::Boolean
_omni_callTransientExceptionHandler(omniObject*,
				    CORBA::ULong,
				    const CORBA::TRANSIENT&);

extern CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObject*,
				      CORBA::ULong,
				      const CORBA::COMM_FAILURE&);

extern CORBA::Boolean
_omni_callSystemExceptionHandler(omniObject*,
				 CORBA::ULong,
				 const CORBA::SystemException&);


extern void _omni_set_NameService(CORBA::Object_ptr);

#include <omniORB2/corbaidl_operators.hh>
#include <omniORB2/ir_operators.hh>

#undef _core_attr
#undef _dyn_attr

#if !defined(_OMNIORB2_LIBRARY) && !defined(_OMNIORB2_DYNAMIC_LIBRARY)
#ifndef USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifndef USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#include <omniORB2/Naming.hh>
#ifdef  USE_core_stub_in_nt_dll_NOT_DEFINED
#undef  USE_core_stub_in_nt_dll
#undef  USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifdef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#undef  USE_dyn_stub_in_nt_dll
#undef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#endif

#endif // __CORBA_H__
