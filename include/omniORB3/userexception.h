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
 Revision 1.1.2.2  1999/10/04 15:51:51  djr
 Various fixes/MSVC work-arounds.

 Revision 1.1.2.1  1999/09/24 09:51:58  djr
 Moved from omniORB2 + some new files.

*/

#ifndef __OMNIORB_USEREXCEPTION_H__
#define __OMNIORB_USEREXCEPTION_H__


#define OMNIORB_DECLARE_USER_EXCEPTION(name)  \
  \
class name : public CORBA::UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) : UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    CORBA::UserException::operator=(_ex);  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise();  \
  static name* _downcast(CORBA::Exception*);  \
  static const name* _downcast(const CORBA::Exception*);  \
  static inline name* _narrow(CORBA::Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline size_t _NP_alignedSize(size_t os) const { return os; }  \
  inline void operator>>=(NetBufferedStream&) const {}  \
  inline void operator>>=(MemBufferedStream&) const {}  \
  inline void operator<<=(NetBufferedStream&) {}  \
  inline void operator<<=(MemBufferedStream&) {}  \
  \
  static _core_attr insertExceptionToAny    insertToAnyFn;  \
  static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static const char* _PD_repoId; \
  \
private:  \
  virtual CORBA::Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(NetBufferedStream&) const;  \
  virtual void _NP_marshal(MemBufferedStream&) const;  \
};


#define OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(name)  \
  \
class name : public UserException {  \
public:  \
  inline name() {  \
    pd_insertToAnyFn    = insertToAnyFn;  \
    pd_insertToAnyFnNCP = insertToAnyFnNCP;  \
  }  \
  inline name(const name& _ex) : UserException(_ex) {}  \
  inline name& operator=(const name& _ex) {  \
    UserException::operator=(_ex);  return *this;  \
  }  \
  virtual ~name();  \
  virtual void _raise();  \
  static name* _downcast(Exception*);  \
  static const name* _downcast(const Exception*);  \
  static inline name* _narrow(Exception* _ex) {  \
    return _downcast(_ex);  \
  }  \
  \
  inline size_t _NP_alignedSize(size_t os) const { return os; }  \
  inline void operator>>=(NetBufferedStream&) const {}  \
  inline void operator>>=(MemBufferedStream&) const {}  \
  inline void operator<<=(NetBufferedStream&) {}  \
  inline void operator<<=(MemBufferedStream&) {}  \
  \
  static _core_attr insertExceptionToAny    insertToAnyFn;  \
  static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;  \
  \
  static const char* _PD_repoId; \
  \
private:  \
  virtual Exception* _NP_duplicate() const;  \
  virtual const char* _NP_typeId() const;  \
  virtual const char* _NP_repoId(int* size) const;  \
  virtual void _NP_marshal(NetBufferedStream&) const;  \
  virtual void _NP_marshal(MemBufferedStream&) const;  \
};


#endif // __OMNIORB_USEREXCEPTION_H__
