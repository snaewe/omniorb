// -*- Mode: C++; -*-
//                            Package   : omniORB2
// proxyCall.h                Created on: 12/98
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 Olivetti & Oracle Research Laboratory
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
//	*** PROPRIETORY INTERFACE ***

#ifndef __PROXYCALL_H__
#define __PROXYCALL_H__


class OmniProxyCallDesc {
public:
  inline OmniProxyCallDesc(const char* op, size_t op_len,
			   CORBA::Boolean has_exceptions = 0)
    : pd_has_user_exceptions(has_exceptions),
      pd_operation(op), pd_operation_len(op_len) {}

  virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
  // Defaults to no arguments.

  virtual void marshalArguments(GIOP_C&);
  // Defaults to no arguments.

  virtual void unmarshalReturnedValues(GIOP_C&);
  // Defaults to no arguments and returns void.

  virtual void userException(GIOP_C& giop_client, const char* repoId);
  // Defaults to no user exceptions, and thus throws
  // CORBA::UNKNOWN. Any version of this should in all
  // cases either throw a user exception or CORBA::UNKNOWN.
  // Must call giop_client.RequestCompleted().

  inline CORBA::Boolean has_user_exceptions() const {
    return pd_has_user_exceptions;
  }
  inline const char* operation() const { return pd_operation; }
  inline size_t operation_len() const  { return pd_operation_len; }

private:
  CORBA::Boolean pd_has_user_exceptions;
  const char* pd_operation;
  size_t pd_operation_len;
};


class OmniOWProxyCallDesc {
public:
  inline OmniOWProxyCallDesc(const char* op, size_t op_len)
    : pd_operation(op), pd_operation_len(op_len) {}

  virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
  // Defaults to no arguments.

  virtual void marshalArguments(GIOP_C&);
  // Defaults to no arguments.

  inline const char* operation() const { return pd_operation; }
  inline size_t operation_len() const  { return pd_operation_len; }

protected:
  const char* pd_operation;
  size_t pd_operation_len;
};


_CORBA_MODULE OmniProxyCallWrapper
_CORBA_MODULE_BEG
  _CORBA_MODULE_FN void invoke(omniObject*, OmniProxyCallDesc&);
  _CORBA_MODULE_FN void one_way(omniObject*, OmniOWProxyCallDesc&);

  typedef OmniProxyCallDesc void_call;
  typedef OmniOWProxyCallDesc ow_void_call;

_CORBA_MODULE_END


#endif  // __PROXYCALL_H__
