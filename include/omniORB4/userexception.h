// -*- Mode: C++; -*-
//                            Package   : omniORB
// userexception.h            Created on: 1999
//                            Author    : David Riddoch (djr)
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
//

/*
 $Log$
 Revision 1.2.2.2  2000/09/27 16:58:07  sll
 Replaced marshalling operators for MemBufferedStream and NetBufferedStream
 with just one type for cdrStream.

 Revision 1.2.2.1  2000/07/17 10:35:38  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:04  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.7  2000/01/27 10:55:44  djr
 Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
 fqname for base class constructor for some compilers.

 Revision 1.1.2.6  1999/10/21 11:29:46  djr
 Added _core_attr to declaration of _PD_repoId in exceptions & interfaces.

 Revision 1.1.2.5  1999/10/18 17:28:19  djr
 Fixes for building MSVC dlls.

 Revision 1.1.2.4  1999/10/18 11:27:37  djr
 Centralised list of system exceptions.

 Revision 1.1.2.3  1999/10/04 17:08:30  djr
 Some more fixes/MSVC work-arounds.

 Revision 1.1.2.2  1999/10/04 15:51:51  djr
 Various fixes/MSVC work-arounds.

 Revision 1.1.2.1  1999/09/24 09:51:58  djr
 Moved from omniORB2 + some new files.

*/

#ifndef __OMNIORB_USEREXCEPTION_H__
#define __OMNIORB_USEREXCEPTION_H__


#define OMNIORB_DECLARE_USER_EXCEPTION(name, attr)  \
  \
class name : public CORBA::UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) :  \
    OMNIORB_BASE_CTOR(CORBA::)UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    * (CORBA::UserException*) this = _ex;  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise();  \
  static name* _downcast(CORBA::Exception*);  \
  static const name* _downcast(const CORBA::Exception*);  \
  static inline name* _narrow(CORBA::Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline void operator>>=(cdrStream&) const {}  \
  inline void operator<<=(cdrStream&) {}  \
  \
  static attr insertExceptionToAny    insertToAnyFn;  \
  static attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static attr const char* _PD_repoId; \
  \
private:  \
  virtual CORBA::Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(cdrStream&) const;  \
};


#define OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(name, attr)  \
  \
class name : public UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) : UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    * (UserException*) this = _ex;  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise();  \
  static name* _downcast(Exception*);  \
  static const name* _downcast(const Exception*);  \
  static inline name* _narrow(Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline void operator>>=(cdrStream&) const {}  \
  inline void operator<<=(cdrStream&) {}  \
  \
  static attr insertExceptionToAny    insertToAnyFn;  \
  static attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static attr const char* _PD_repoId; \
  \
private:  \
  virtual Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(cdrStream&) const;  \
};


// This macro applies it's argument to the name of each
// of the system exceptions.  It is expected that the
// argument <doit> will be another macro.

#define OMNIORB_FOR_EACH_SYS_EXCEPTION(doit) \
 \
doit (UNKNOWN) \
doit (BAD_PARAM) \
doit (NO_MEMORY) \
doit (IMP_LIMIT) \
doit (COMM_FAILURE) \
doit (INV_OBJREF) \
doit (OBJECT_NOT_EXIST) \
doit (NO_PERMISSION) \
doit (INTERNAL) \
doit (MARSHAL) \
doit (INITIALIZE) \
doit (NO_IMPLEMENT) \
doit (BAD_TYPECODE) \
doit (BAD_OPERATION) \
doit (NO_RESOURCES) \
doit (NO_RESPONSE) \
doit (PERSIST_STORE) \
doit (BAD_INV_ORDER) \
doit (TRANSIENT) \
doit (FREE_MEM) \
doit (INV_IDENT) \
doit (INV_FLAG) \
doit (INTF_REPOS) \
doit (BAD_CONTEXT) \
doit (OBJ_ADAPTER) \
doit (DATA_CONVERSION) \
doit (TRANSACTION_REQUIRED) \
doit (TRANSACTION_ROLLEDBACK) \
doit (INVALID_TRANSACTION) \
doit (WRONG_TRANSACTION) \


#endif // __OMNIORB_USEREXCEPTION_H__
