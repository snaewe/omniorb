// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_C.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2006 Apasphere Ltd
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.6.4  2009/05/06 16:16:10  dgrisby
  Update lots of copyright notices.

  Revision 1.1.6.3  2006/07/18 16:21:24  dgrisby
  New experimental connection management extension; ORB core support
  for it.

  Revision 1.1.6.2  2006/06/05 13:33:25  dgrisby
  Inline declarations; operation() access function.

  Revision 1.1.6.1  2003/03/23 21:03:56  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.4  2001/10/17 16:33:27  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.4.3  2001/09/04 14:38:08  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.2  2001/05/01 16:07:33  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.1  2001/04/18 17:19:01  sll
  Big checkin with the brand new internal APIs.

  */


#ifndef __GIOP_C_H__
#define __GIOP_C_H__

#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class GIOP_C : public IOP_C, public giopStream, public giopStreamList {
 public:

  GIOP_C(giopRope*,giopStrand*);
  ~GIOP_C();

  virtual void* ptrToClass(int* cptr);
  static inline GIOP_C* downcast(cdrStream* s) {
    return (GIOP_C*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

  void InitialiseRequest();

  GIOP::ReplyStatusType ReceiveReply();

  void RequestCompleted(CORBA::Boolean skip=0);

  GIOP::LocateStatusType IssueLocateRequest();

  void notifyCommFailure(CORBA::Boolean heldlock,
			 CORBA::ULong& minor,
			 CORBA::Boolean& retry);
  // override giopStream member

  _CORBA_ULong completion();
  // override cdrStream member.

  cdrStream& getStream() { return *this; }

  void initialise(const omniIOR*,
		  const CORBA::Octet* key,
		  int keysize,
		  omniCallDescriptor*);

  void cleanup();

  inline IOP_C::State state() const { return pd_state; }
  inline void state(IOP_C::State s) { pd_state = s; }

  inline omniCallDescriptor* calldescriptor() { return pd_calldescriptor; }
  inline void calldescriptor(omniCallDescriptor* c) { pd_calldescriptor = c; }

  inline const char* operation() { return pd_calldescriptor->op(); }

  inline const CORBA::Octet* key() const  { return pd_key;    }
  inline void key(const CORBA::Octet* k)  { pd_key = k; }

  inline CORBA::ULong keysize() const  { return pd_keysize; }
  inline void keysize(CORBA::ULong sz) { pd_keysize = sz; }

  inline const omniIOR* ior() const { return pd_ior; }
  inline void ior(const omniIOR* c) { pd_ior = c; }

  inline GIOP::ReplyStatusType replyStatus() const { return pd_replyStatus; }
  inline void replyStatus(GIOP::ReplyStatusType rc) { pd_replyStatus = rc; }

  inline GIOP::LocateStatusType locateStatus() const { return pd_locateStatus; }
  inline void locateStatus(GIOP::LocateStatusType rc) { pd_locateStatus = rc; }

  inline CORBA::ULong  replyId() const { return pd_reply_id; }
  inline void replyId(CORBA::ULong v) { pd_reply_id = v; }

  inline giopRope* rope() const { return pd_rope; }

private:
  IOP_C::State            pd_state;
  omniCallDescriptor*     pd_calldescriptor;
  const omniIOR*          pd_ior;
  const CORBA::Octet*     pd_key;
  CORBA::ULong            pd_keysize;
  giopRope*               pd_rope;
  GIOP::ReplyStatusType   pd_replyStatus;
  GIOP::LocateStatusType  pd_locateStatus;
  CORBA::ULong            pd_reply_id;

  void UnMarshallSystemException();

  GIOP_C();
  GIOP_C(const GIOP_C&);
  GIOP_C& operator=(const GIOP_C&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOP_C_H__
