// -*- Mode: C++; -*-
//                            Package   : omniORB
// Messaging.cc               Created on: 21/08/2000
//                            Author    : David Scott (djs)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//      Implementation of Messaging::ExceptionHolder and Messaging::Poller
//
/*
 $Id$
 $Log$
 Revision 1.1.2.6  2000/10/10 15:07:12  djs
 Improved comments & tracing
 Moved code from header file to here

 Revision 1.1.2.5  2000/09/28 23:18:26  djs
 Removed a spurious closing curly bracket

 Revision 1.1.2.4  2000/09/28 18:29:39  djs
 Bugfixes in Poller (wrt timout behaviour and is_ready function)
 Removed traces of Private POA/ internal ReplyHandler servant for Poller
 strategy
 General comment tidying

*/

#include <omniORB3/CORBA.h>
#include <omniORB3/Messaging.h>
#include <omniORB3/userexception.h>
#include <exceptiondefs.h>
#include "PollableSet.h"

/////////////////////////////////////////////////////////////////////////
// Messaging::ExceptionHolder ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Messaging::ExceptionHolder::~ExceptionHolder(){
  if (local_exception_object)
    delete local_exception_object;
}

void
Messaging::ExceptionHolder::_NP_marshal_exception_to_sequence(const CORBA::Exception &e){
  MemBufferedStream stream;
  // marshal the repoId of the exception
  {
    CORBA::ULong repoIdLen;
    const char *repoId = e._NP_repoId((int*)&repoIdLen);
    repoIdLen >>= stream;
    stream.put_char_array((const CORBA::Char*)repoId, repoIdLen);
  }
  // marshal the actual exception itself
  e._NP_marshal(stream);
  
  // shunt the contents of the stream into the sequence<octet>
  // for transport
  CORBA::ULong msgsize = stream.alreadyWritten();
  CORBA::Octet *buffer = _pd_marshaled_exception_seq::allocbuf(msgsize);
  stream.get_char_array(buffer, sizeof(CORBA::Octet) * msgsize);

  _pd_marshaled_exception_seq seq(msgsize, msgsize, buffer, 1);

  pd_marshaled_exception = seq;
}

void
Messaging::ExceptionHolder::operator>>= (NetBufferedStream &_n){
  if (local_exception_object)
    _NP_marshal_exception_to_sequence(*local_exception_object);

  pd_is_system_exception >>= _n;
  pd_byte_order >>= _n;
  pd_marshaled_exception >>= _n;
  
}

void
Messaging::ExceptionHolder::operator<<= (NetBufferedStream &_n){
  pd_is_system_exception <<= _n;
  pd_byte_order <<= _n;
  pd_marshaled_exception <<= _n;

  local_exception_object = NULL;
}

void
Messaging::ExceptionHolder::operator>>= (MemBufferedStream &_n){
  if (local_exception_object)
    _NP_marshal_exception_to_sequence(*local_exception_object);

  pd_is_system_exception >>= _n;
  pd_byte_order >>= _n;
  pd_marshaled_exception >>= _n;  
}

void
Messaging::ExceptionHolder::operator<<= (MemBufferedStream &_n){
  pd_is_system_exception <<= _n;
  pd_byte_order <<= _n;
  pd_marshaled_exception <<= _n;

  local_exception_object = NULL;
}

/////////////////////////////////////////////////////////////////////////
// Messaging::Poller //////// ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


void Messaging::Poller::_NP_throw_exception_from_poller
  (const CORBA::Exception& e, CORBA::Boolean locked){
  if (!locked) pd_state_lock.lock();
  pd_is_from_poller = 1;
  if (!locked) pd_state_lock.unlock();
  e._raise();
  //throw e;
}

void Messaging::Poller::_NP_wait_and_throw_exception(CORBA::ULong timeout){
  if (!is_ready(timeout)) {
    // NO_RESPONSE is generated for the no-timeout case
    if (timeout == 0) 
      _NP_throw_exception_from_poller(CORBA::NO_RESPONSE(), 0);

    _NP_throw_exception_from_poller(CORBA::TIMEOUT(), 0);
  }
  
  // Data must have arrived within the timeout period.
  {
    omni_mutex_lock lock(pd_state_lock);
    // Has someone else got it?
    if (reply_already_read)
      _NP_throw_exception_from_poller(CORBA::OBJECT_NOT_EXIST(), 1);
    // Claim it for ourselves
    reply_already_read = 1;
    // Everyone else who tries to get this data will get an exception instead
  }
}

void Messaging::Poller::_NP_throw_wrong_transaction(){
  _NP_throw_exception_from_poller(CORBA::WRONG_TRANSACTION(), 0);
}

void Messaging::Poller::_NP_tell_poller_reply_received(){
  omni_mutex_lock lock(pd_state_lock);
  reply_received = 1;
  // The client could have lots of threads blocked on this event, better wake
  // them all up. They can fight over ownership of the data and the loser(s)
  // can return with OBJECT_NOT_EXIST
  pd_state_cond.broadcast();
}

Messaging::ReplyHandler_ptr Messaging::Poller::associated_handler(){
  Messaging::ReplyHandler_ptr handler;
  {
    omni_mutex_lock lock(pd_state_lock);
    handler = pd_associated_handler;
  }
  return handler;
}

void Messaging::Poller::associated_handler(Messaging::ReplyHandler_ptr handler){
  omni_mutex_lock lock(pd_state_lock);
  pd_associated_handler = handler;
}

// timeout is in seconds, ULONG_MAX means block forever.
// *** According to CORBA Messaging 6.6 Generic Poller Value
//   returns TRUE iff a reply is currently available for the request
//   returns FALSE iff a reply has not been received
//   throws OBJECT_NOT_EXIST if the reply has already been consumed
// Section 9.3.4.1 doesn't mention the exception at all.
CORBA::Boolean Messaging::Poller::is_ready(CORBA::ULong timeout){
  omni_mutex_lock lock(pd_state_lock);
  // It's not clear from the spec what to do if the value has already
  // been consumed. Section 6.6 suggests throw CORBA::OBJECT_NOT_EXIST
  if (reply_already_read) 
    _NP_throw_exception_from_poller(CORBA::OBJECT_NOT_EXIST(), 1);
  
  // Note that just because the data is available doesn't mean this
  // thread will be able to get it. Once the lock is gone, all bets are off.
  if (reply_received) return 1;
  
  // Reply must not have been received yet.
  if (timeout == 0) return 0;
  
  if (timeout == ULONG_MAX){
    // block indefinitely until I can return TRUE
    pd_state_cond.wait();
  }else{
    // block with a timeout (timeout seconds)
    unsigned long sec, nsec;
    omni_thread::get_time(&sec, &nsec, 0, timeout*1000);
    pd_state_cond.timedwait(sec, nsec);
  }
  // After waiting we might have lost out to someoen else
  if (reply_already_read) 
    _NP_throw_exception_from_poller(CORBA::OBJECT_NOT_EXIST(), 1);
  // Again remember that just because the data has arrived, doesn't
  // give this thread any right to it...
  return reply_received;
}

CORBA::Boolean Messaging::Poller::is_from_poller(){
  omni_mutex_lock lock(pd_state_lock);

  if (!reply_received) 
    _NP_throw_exception_from_poller(CORBA::BAD_INV_ORDER(), 1);

  return pd_is_from_poller;
}


CORBA::PollableSet_ptr Messaging::Poller::create_pollable_set(){
  PollableSet_polling *set = new PollableSet_polling();
  return set->_this();
}
