#include <omniORB3/CORBA.h>
#include <omniORB3/Messaging.h>
#include <omniORB3/userexception.h>
#include <exceptiondefs.h>
#include "PollableSet.h"


Messaging::ExceptionHolder::~ExceptionHolder()
{
  if (local_exception_object)
    delete local_exception_object;
}

void
Messaging::ExceptionHolder::_NP_marshal_exception_to_sequence(const CORBA::Exception &e)
{
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
Messaging::ExceptionHolder::operator>>= (NetBufferedStream &_n)
{
  if (local_exception_object)
    _NP_marshal_exception_to_sequence(*local_exception_object);

  pd_is_system_exception >>= _n;
  pd_byte_order >>= _n;
  pd_marshaled_exception >>= _n;
  
}

void
Messaging::ExceptionHolder::operator<<= (NetBufferedStream &_n)
{
  pd_is_system_exception <<= _n;
  pd_byte_order <<= _n;
  pd_marshaled_exception <<= _n;

  local_exception_object = NULL;
}

void
Messaging::ExceptionHolder::operator>>= (MemBufferedStream &_n)
{
  if (local_exception_object)
    _NP_marshal_exception_to_sequence(*local_exception_object);

  pd_is_system_exception >>= _n;
  pd_byte_order >>= _n;
  pd_marshaled_exception >>= _n;  
}

void
Messaging::ExceptionHolder::operator<<= (MemBufferedStream &_n)
{
  pd_is_system_exception <<= _n;
  pd_byte_order <<= _n;
  pd_marshaled_exception <<= _n;

  local_exception_object = NULL;
}

Messaging::ReplyHandler_ptr Messaging::Poller::associated_handler()
{
  Messaging::ReplyHandler_ptr handler;
  {
    omni_mutex_lock lock(pd_state_lock);
    handler = pd_associated_handler;
  }
  return handler;
}


void Messaging::Poller::associated_handler(Messaging::ReplyHandler_ptr handler)
{
  omni_mutex_lock lock(pd_state_lock);
  pd_associated_handler = handler;
}

// timeout == 0 means a non-blocking poll  
// timeout == 2^32 - 1 means block forever
CORBA::Boolean Messaging::Poller::is_ready(CORBA::ULong timeout)
{
  CORBA::Boolean reply = 0;
  {
    omni_mutex_lock lock(pd_state_lock);
    if (reply_already_read)
      throw CORBA::OBJECT_NOT_EXIST();

    // Remember that just because it is_ready, doesn't mean you can
    // grab it. Another thread can come along and get to it first.
    if (reply_received) return 1;

    // reply must not have been received yet.

    if (timeout == 0)             // immediately throw an exception
      throw CORBA::NO_RESPONSE();

    if (timeout == ULONG_MAX){    // block forever
      pd_state_cond.wait();
    }else{                        // block with a timeout
      unsigned long sec, nsec;
      omni_thread::get_time(&sec, &nsec, 0, timeout*1000);
      pd_state_cond.timedwait(sec, nsec);
    }
    
    if (!reply_received) 
      throw CORBA::TIMEOUT();

    if (!reply_already_read){
      // it's still available
      reply = 1;
    } else {
      // while we were sleeping, someone else took it
      throw CORBA::OBJECT_NOT_EXIST();
    }
  }
  return reply;
}

CORBA::Boolean Messaging::Poller::is_from_poller(){
  omni_mutex_lock lock(pd_state_lock);

  return pd_is_from_poller;
}


CORBA::PollableSet_ptr Messaging::Poller::create_pollable_set(){
  PollableSet_polling *set = new PollableSet_polling();
  return set->_this();
}
