// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Exception.h          Created on: 2001/08/17
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
//    CORBA::Exception, SystemException, UserException
//

/*
  $Log$
  Revision 1.1.2.4  2003/01/14 11:48:15  dgrisby
  Remove warnings from gcc -Wshadow. Thanks Pablo Mejia.

  Revision 1.1.2.3  2001/11/01 12:04:31  dpg1
  Function in SystemException to return minor code string.

  Revision 1.1.2.2  2001/10/17 16:44:00  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.1  2001/08/17 13:39:46  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
////////////////////////////// Exception /////////////////////////////
//////////////////////////////////////////////////////////////////////

class Exception {
public:
  virtual ~Exception();

  virtual void _raise() const = 0;
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

  virtual const char* _name() const;
  virtual const char* _rep_id() const;
  // Return the plain name and repository id

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
  inline SystemException(ULong minor_, CompletionStatus status) {
    pd_minor = minor_;
    pd_status = status;
  }
  inline SystemException& operator=(const SystemException& e) {
    Exception::operator=(e);
    pd_minor = e.pd_minor;
    pd_status = e.pd_status;
    return *this;
  }

  virtual const char* NP_minorString() const = 0;
  // omniORB proprietary function to get a meaningful name for the
  // minor code. The string must NOT be freed.

protected:
  ULong             pd_minor;
  CompletionStatus  pd_status;

private:
  virtual void _NP_marshal(cdrStream&) const;
};

#define OMNIORB_DECLARE_SYS_EXCEPTION(name) \
  class name : public SystemException { \
  public: \
    inline name(ULong minor_ = 0, CompletionStatus completed_ = COMPLETED_NO) \
      : SystemException (minor_, completed_) { \
          pd_insertToAnyFn    = insertToAnyFn; \
          pd_insertToAnyFnNCP = insertToAnyFnNCP; \
    } \
    inline name(const name& ex) : SystemException(ex) {} \
    inline name& operator=(const name& ex) { \
      SystemException::operator=(ex); \
      return *this; \
    } \
    virtual ~name(); \
    virtual void _raise() const; \
    static name* _downcast(Exception*); \
    static const name* _downcast(const Exception*); \
    static inline name* _narrow(Exception* e) { return _downcast(e); } \
    virtual const char* _NP_repoId(int* size) const; \
    static _core_attr Exception::insertExceptionToAny    insertToAnyFn; \
    static _core_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP; \
    virtual const char* NP_minorString() const; \
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
////////////////////////// WrongTransaction //////////////////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(WrongTransaction, _dyn_attr)


//////////////////////////////////////////////////////////////////////
////////////////////////// PolicyError      //////////////////////////
//////////////////////////////////////////////////////////////////////
typedef _CORBA_Short PolicyErrorCode;
_CORBA_MODULE_VARINT const PolicyErrorCode 
BAD_POLICY _init_in_decl_( = 0 );
_CORBA_MODULE_VARINT const PolicyErrorCode 
UNSUPPORTED_POLICY _init_in_decl_( = 1 );
_CORBA_MODULE_VARINT const PolicyErrorCode 
BAD_POLICY_TYPE _init_in_decl_( = 2 );
_CORBA_MODULE_VARINT const PolicyErrorCode
BAD_POLICY_VALUE _init_in_decl_( = 3 );
_CORBA_MODULE_VARINT const PolicyErrorCode
UNSUPPORTED_POLICY_VALUE _init_in_decl_( = 4 );

_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_PolicyErrorCode; 

class PolicyError : public CORBA::UserException {
public:
  PolicyErrorCode reason;

  inline PolicyError() {
    pd_insertToAnyFn    = insertToAnyFn;
    pd_insertToAnyFnNCP = insertToAnyFnNCP;
  }
  PolicyError(const PolicyError&);
  PolicyError(PolicyErrorCode);
  PolicyError& operator=(const PolicyError&);
  virtual ~PolicyError();
  virtual void _raise() const;
  static PolicyError* _downcast(CORBA::Exception*);
  static const PolicyError* _downcast(const CORBA::Exception*);
  static inline PolicyError* _narrow(CORBA::Exception* e) { 
    return _downcast(e);
  }

  void operator>>=(cdrStream&) const;
  void operator<<=(cdrStream&);

  static _dyn_attr insertExceptionToAny    insertToAnyFn;
  static _dyn_attr insertExceptionToAnyNCP insertToAnyFnNCP;

  static _dyn_attr const char* _PD_repoId;

private:
  virtual CORBA::Exception* _NP_duplicate() const;
  virtual const char* _NP_typeId() const;
  virtual const char* _NP_repoId(int*) const;
  virtual void _NP_marshal(cdrStream&) const;
};

_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_PolicyError;
