// -*- Mode: C++; -*-
//                            Package   : omniORB2
// reDirect.cc                Created on: 19/3/97
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1997
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
//

// $Log$
// Revision 1.2  1997/12/10 11:39:44  sll
// Updated life cycle runtime.
//
// Revision 1.1  1997/09/20  17:06:52  dpg1
// Initial revision
//
// Revision 1.1  1997/09/20  17:06:52  dpg1
// Initial revision
//


#include <omniORB2/omniLC.h>

omniLC::reDirect::reDirect(CORBA::Object_ptr fwdref) 
  : pd_fwdref(fwdref) 
{
  if (CORBA::is_nil(fwdref)) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  omniObject::PR_IRRepositoryId(fwdref->PR_getobj()->NP_IRRepositoryId());
  this->PR_setobj(this);
}

omniLC::reDirect::reDirect(CORBA::Object_ptr fwdref,
			   const omniORB::objectKey& mykey)
  : pd_fwdref(fwdref)
{
  if (CORBA::is_nil(fwdref)) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  omniObject::PR_IRRepositoryId(fwdref->PR_getobj()->NP_IRRepositoryId());
  this->PR_setobj(this);
  omniRopeAndKey l(0,(CORBA::Octet*)&mykey,(CORBA::ULong)sizeof(mykey));
  setRopeAndKey(l,0);
}

CORBA::Object_ptr
omniLC::reDirect::forwardReference() const
{
  return CORBA::Object::_duplicate(pd_fwdref);
}

CORBA::Object_ptr
omniLC::reDirect::_this()
{ 
  return CORBA::Object::_duplicate(this); 
}

void 
omniLC::reDirect::_obj_is_ready(CORBA::BOA_ptr boa)
{ 
  boa->obj_is_ready(this);
}

CORBA::BOA_ptr
omniLC::reDirect::_boa()
{ 
  return CORBA::BOA::getBOA();
}

void
omniLC::reDirect::_dispose()
{ 
  _boa()->dispose(this);
}
  
omniORB::objectKey
omniLC::reDirect::_key()
{
  omniRopeAndKey l;
  getRopeAndKey(l);
  return (*((omniORB::objectKey*)l.key()));
}

CORBA::Boolean
omniLC::reDirect::dispatch(GIOP_S &s, const char *,CORBA::Boolean response)
{
  s.RequestReceived(1);

  if (!response) {
    // a one-way call, not allowed to give a reply
    // silently dump the request.
    s.ReplyCompleted();
    return 1;
  }
  size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
  msgsize = CORBA::Object::NP_alignedSize(pd_fwdref,msgsize);
  s.InitialiseReply(GIOP::LOCATION_FORWARD,(CORBA::ULong)msgsize);
  CORBA::Object::marshalObjRef(pd_fwdref,s);
  s.ReplyCompleted();
  return 1;
}

