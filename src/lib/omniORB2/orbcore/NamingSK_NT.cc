/* NamingSK_NT.cc                                       */
/*                                                      */
/* Manually Edited NamingSK for Windows NT/ MSVC++ 4.2  */
/* Includes work-arounds for MSVC++ 4.2 bugs            */

#include "omniORB2/Naming_NT.hh"


size_t
CosNaming::NameComponent::NP_alignedSize(size_t _initialoffset) const
{
  CORBA::ULong _msgsize = _initialoffset;
  _msgsize = id.NP_alignedSize(_msgsize);
  _msgsize = kind.NP_alignedSize(_msgsize);
  return _msgsize;
}

void
CosNaming::NameComponent::operator>>= (NetBufferedStream &_n) const
{
  id >>= _n;
  kind >>= _n;
}

void
CosNaming::NameComponent::operator<<= (NetBufferedStream &_n)
{
  id <<= _n;
  kind <<= _n;
}

void
CosNaming::NameComponent::operator>>= (MemBufferedStream &_n) const
{
  id >>= _n;
  kind >>= _n;
}

void
CosNaming::NameComponent::operator<<= (MemBufferedStream &_n)
{
  id <<= _n;
  kind <<= _n;
}

size_t
CosNaming::Binding::NP_alignedSize(size_t _initialoffset) const
{
  CORBA::ULong _msgsize = _initialoffset;
  _msgsize = binding_name.NP_alignedSize(_msgsize);
  _msgsize = omni::align_to(_msgsize,omni::ALIGN_4);
  _msgsize += 4;
  return _msgsize;
}

void
CosNaming::Binding::operator>>= (NetBufferedStream &_n) const
{
  binding_name >>= _n;
  binding_type >>= _n;
}

void
CosNaming::Binding::operator<<= (NetBufferedStream &_n)
{
  binding_name <<= _n;
  binding_type <<= _n;
}

void
CosNaming::Binding::operator>>= (MemBufferedStream &_n) const
{
  binding_name >>= _n;
  binding_type >>= _n;
}

void
CosNaming::Binding::operator<<= (MemBufferedStream &_n)
{
  binding_name <<= _n;
  binding_type <<= _n;
}

CosNaming::NamingContext::NotFound::NotFound(const CosNaming::NamingContext::NotFound &_s)
{
  why = _s.why;
  rest_of_name = _s.rest_of_name;
}

CosNaming::NamingContext::NotFound::NotFound(CosNaming::NamingContext::NotFoundReason  _why, const CosNaming::Name & _rest_of_name)
{
  why = _why;
  rest_of_name = _rest_of_name;
}

CosNaming::NamingContext::NotFound & CosNaming::NamingContext::NotFound::operator=(const CosNaming::NamingContext::NotFound &_s)
{
  why = _s.why;
  rest_of_name = _s.rest_of_name;
  return *this;
}

size_t
CosNaming::NamingContext::NotFound::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  _msgsize = omni::align_to(_msgsize,omni::ALIGN_4);
  _msgsize += 4;
  _msgsize = rest_of_name.NP_alignedSize(_msgsize);
  return _msgsize;
}

void
CosNaming::NamingContext::NotFound::operator>>= (NetBufferedStream &_n)
{
  why >>= _n;
  rest_of_name >>= _n;
}

void
CosNaming::NamingContext::NotFound::operator<<= (NetBufferedStream &_n)
{
  why <<= _n;
  rest_of_name <<= _n;
}

void
CosNaming::NamingContext::NotFound::operator>>= (MemBufferedStream &_n)
{
  why >>= _n;
  rest_of_name >>= _n;
}

void
CosNaming::NamingContext::NotFound::operator<<= (MemBufferedStream &_n)
{
  why <<= _n;
  rest_of_name <<= _n;
}

CosNaming::NamingContext::CannotProceed::CannotProceed(const CosNaming::NamingContext::CannotProceed &_s)
{
  cxt = _s.cxt;
  rest_of_name = _s.rest_of_name;
}

CosNaming::NamingContext::CannotProceed::CannotProceed(CosNaming::NamingContext_ptr _cxt, const CosNaming::Name & _rest_of_name)
{
  cxt = _cxt;
  rest_of_name = _rest_of_name;
}

CosNaming::NamingContext::CannotProceed & CosNaming::NamingContext::CannotProceed::operator=(const CosNaming::NamingContext::CannotProceed &_s)
{
  cxt = _s.cxt;
  rest_of_name = _s.rest_of_name;
  return *this;
}

size_t
CosNaming::NamingContext::CannotProceed::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  _msgsize = CosNaming::NamingContext::NP_alignedSize(cxt,_msgsize);
  _msgsize = rest_of_name.NP_alignedSize(_msgsize);
  return _msgsize;
}

void
CosNaming::NamingContext::CannotProceed::operator>>= (NetBufferedStream &_n)
{
  CosNaming::NamingContext::marshalObjRef(cxt,_n);
  rest_of_name >>= _n;
}

void
CosNaming::NamingContext::CannotProceed::operator<<= (NetBufferedStream &_n)
{
  cxt = CosNaming::NamingContext::unmarshalObjRef(_n);
  rest_of_name <<= _n;
}

void
CosNaming::NamingContext::CannotProceed::operator>>= (MemBufferedStream &_n)
{
  CosNaming::NamingContext::marshalObjRef(cxt,_n);
  rest_of_name >>= _n;
}

void
CosNaming::NamingContext::CannotProceed::operator<<= (MemBufferedStream &_n)
{
  cxt = CosNaming::NamingContext::unmarshalObjRef(_n);
  rest_of_name <<= _n;
}

CosNaming::NamingContext::InvalidName::InvalidName(const CosNaming::NamingContext::InvalidName &_s)
{
}

CosNaming::NamingContext::InvalidName & CosNaming::NamingContext::InvalidName::operator=(const CosNaming::NamingContext::InvalidName &_s)
{
  return *this;
}

size_t
CosNaming::NamingContext::InvalidName::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  return _msgsize;
}

void
CosNaming::NamingContext::InvalidName::operator>>= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::InvalidName::operator<<= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::InvalidName::operator>>= (MemBufferedStream &_n)
{
}

void
CosNaming::NamingContext::InvalidName::operator<<= (MemBufferedStream &_n)
{
}

CosNaming::NamingContext::AlreadyBound::AlreadyBound(const CosNaming::NamingContext::AlreadyBound &_s)
{
}

CosNaming::NamingContext::AlreadyBound & CosNaming::NamingContext::AlreadyBound::operator=(const CosNaming::NamingContext::AlreadyBound &_s)
{
  return *this;
}

size_t
CosNaming::NamingContext::AlreadyBound::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  return _msgsize;
}

void
CosNaming::NamingContext::AlreadyBound::operator>>= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::AlreadyBound::operator<<= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::AlreadyBound::operator>>= (MemBufferedStream &_n)
{
}

void
CosNaming::NamingContext::AlreadyBound::operator<<= (MemBufferedStream &_n)
{
}

CosNaming::NamingContext::NotEmpty::NotEmpty(const CosNaming::NamingContext::NotEmpty &_s)
{
}

CosNaming::NamingContext::NotEmpty & CosNaming::NamingContext::NotEmpty::operator=(const CosNaming::NamingContext::NotEmpty &_s)
{
  return *this;
}

size_t
CosNaming::NamingContext::NotEmpty::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  return _msgsize;
}

void
CosNaming::NamingContext::NotEmpty::operator>>= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::NotEmpty::operator<<= (NetBufferedStream &_n)
{
}

void
CosNaming::NamingContext::NotEmpty::operator>>= (MemBufferedStream &_n)
{
}

void
CosNaming::NamingContext::NotEmpty::operator<<= (MemBufferedStream &_n)
{
}

void CosNaming::_proxy_NamingContext::bind ( const CosNaming::Name & n, CORBA::Object_ptr  obj )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),5);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_msgsize = CORBA::Object::NP_alignedSize(obj,_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"bind",5,_0RL_msgsize,0);
    n >>= _0RL_c;
    CORBA::Object::marshalObjRef(obj,_0RL_c);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
          CosNaming::NamingContext::AlreadyBound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        bind ( n, obj );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

void CosNaming::_proxy_NamingContext::rebind ( const CosNaming::Name & n, CORBA::Object_ptr  obj )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),7);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_msgsize = CORBA::Object::NP_alignedSize(obj,_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"rebind",7,_0RL_msgsize,0);
    n >>= _0RL_c;
    CORBA::Object::marshalObjRef(obj,_0RL_c);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        rebind ( n, obj );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

void CosNaming::_proxy_NamingContext::bind_context ( const CosNaming::Name & n, CosNaming::NamingContext_ptr  nc )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),13);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_msgsize = CosNaming::NamingContext::NP_alignedSize(nc,_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"bind_context",13,_0RL_msgsize,0);
    n >>= _0RL_c;
    CosNaming::NamingContext::marshalObjRef(nc,_0RL_c);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
          CosNaming::NamingContext::AlreadyBound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        bind_context ( n, nc );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

void CosNaming::_proxy_NamingContext::rebind_context ( const CosNaming::Name & n, CosNaming::NamingContext_ptr  nc )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),15);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_msgsize = CosNaming::NamingContext::NP_alignedSize(nc,_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"rebind_context",15,_0RL_msgsize,0);
    n >>= _0RL_c;
    CosNaming::NamingContext::marshalObjRef(nc,_0RL_c);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        rebind_context ( n, nc );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

CORBA::Object_ptr  CosNaming::_proxy_NamingContext::resolve ( const CosNaming::Name & n )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CORBA::Object_ptr _0RL_result= 0;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),8);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"resolve",8,_0RL_msgsize,0);
    n >>= _0RL_c;
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_result = CORBA::Object::unmarshalObjRef(_0RL_c);
        _0RL_c.RequestCompleted();
        return _0RL_result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        return resolve ( n );
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_result) CORBA::release(_0RL_result);
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_0RL_result) CORBA::release(_0RL_result);
    throw;
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Object_ptr _0RL_result= 0;
    return _0RL_result;
  }
}

void CosNaming::_proxy_NamingContext::unbind ( const CosNaming::Name & n )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),7);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"unbind",7,_0RL_msgsize,0);
    n >>= _0RL_c;
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        unbind ( n );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

CosNaming::NamingContext_ptr  CosNaming::_proxy_NamingContext::new_context (  )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CosNaming::NamingContext_ptr _0RL_result= 0;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),12);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"new_context",12,_0RL_msgsize,0);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_result = CosNaming::NamingContext::unmarshalObjRef(_0RL_c);
        _0RL_c.RequestCompleted();
        return _0RL_result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        return new_context (  );
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_result) CORBA::release(_0RL_result);
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_0RL_result) CORBA::release(_0RL_result);
    throw;
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CosNaming::NamingContext_ptr _0RL_result= 0;
    return _0RL_result;
  }
}

CosNaming::NamingContext_ptr  CosNaming::_proxy_NamingContext::bind_new_context ( const CosNaming::Name & n )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CosNaming::NamingContext_ptr _0RL_result= 0;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),17);
    _0RL_msgsize = n.NP_alignedSize(_0RL_msgsize);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"bind_new_context",17,_0RL_msgsize,0);
    n >>= _0RL_c;
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_result = CosNaming::NamingContext::unmarshalObjRef(_0RL_c);
        _0RL_c.RequestCompleted();
        return _0RL_result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[46];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 46) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotFound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
          CosNaming::NamingContext::CannotProceed _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
          CosNaming::NamingContext::InvalidName _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
          CosNaming::NamingContext::AlreadyBound _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        return bind_new_context ( n );
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_result) CORBA::release(_0RL_result);
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_0RL_result) CORBA::release(_0RL_result);
    throw;
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CosNaming::NamingContext_ptr _0RL_result= 0;
    return _0RL_result;
  }
}

void CosNaming::_proxy_NamingContext::destroy (  )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),8);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"destroy",8,_0RL_msgsize,0);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        CORBA::Char _0RL_excId[41];
        CORBA::ULong _0RL_len;
        _0RL_len <<= _0RL_c;
        if (_0RL_len > 41) {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        else {
          _0RL_c.get_char_array(_0RL_excId,_0RL_len);
        }
        if (strcmp((const char *)_0RL_excId,CosNaming_NamingContext_NotEmpty_IntfRepoID) == 0) {
          CosNaming::NamingContext::NotEmpty _0RL_ex;
          _0RL_ex <<= _0RL_c;
          _0RL_c.RequestCompleted();
          throw _0RL_ex;
        }
        else {
          _0RL_c.RequestCompleted(1);
          throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
        }
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        destroy (  );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}

void CosNaming::_proxy_NamingContext::_0RL__list ( CORBA::ULong  how_many, CosNaming::BindingList *& bl, CosNaming::BindingIterator_ptr & bi )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CosNaming::BindingList * _bl= 0;
  CosNaming::BindingIterator_ptr _bi= 0;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),5);
    _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
    _0RL_msgsize += 4;
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"list",5,_0RL_msgsize,0);
    how_many >>= _0RL_c;
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _bl = new CosNaming::BindingList;
        *_bl <<= _0RL_c;
        _bi = CosNaming::BindingIterator::unmarshalObjRef(_0RL_c);
        _0RL_c.RequestCompleted();
        bl = _bl;
        bi = _bi;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        list ( how_many, bl, bi );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_bl) delete _bl;
    if (_bi) CORBA::release(_bi);
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_bl) delete _bl;
    if (_bi) CORBA::release(_bi);
    throw;
  }
}


CORBA::Boolean
CosNaming::_sk_NamingContext::dispatch(GIOP_S &_0RL_s,const char *_0RL_op,CORBA::Boolean _0RL_response_expected)
{
  if (strcmp(_0RL_op,"bind") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    CORBA::Object_var obj;
    obj = CORBA::Object::unmarshalObjRef(_0RL_s);
    _0RL_s.RequestReceived();
        try {
      bind ( n, obj );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 49;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 45;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,45);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"rebind") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    CORBA::Object_var obj;
    obj = CORBA::Object::unmarshalObjRef(_0RL_s);
    _0RL_s.RequestReceived();
        try {
      rebind ( n, obj );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"bind_context") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    CosNaming::NamingContext_var nc;
    nc = CosNaming::NamingContext::unmarshalObjRef(_0RL_s);
    _0RL_s.RequestReceived();
        try {
      bind_context ( n, nc );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 49;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 45;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,45);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"rebind_context") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    CosNaming::NamingContext_var nc;
    nc = CosNaming::NamingContext::unmarshalObjRef(_0RL_s);
    _0RL_s.RequestReceived();
        try {
      rebind_context ( n, nc );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"resolve") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    _0RL_s.RequestReceived();
    CORBA::Object_var _0RL_result;
    try {
      _0RL_result = resolve ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize = CORBA::Object::NP_alignedSize((_0RL_result.operator->()),_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    CORBA::Object::marshalObjRef((_0RL_result.operator->()),_0RL_s);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"unbind") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    _0RL_s.RequestReceived();
        try {
      unbind ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"new_context") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _0RL_s.RequestReceived();
    CosNaming::NamingContext_var _0RL_result;
    _0RL_result = new_context (  );
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize = CosNaming::NamingContext::NP_alignedSize((_0RL_result.operator->()),_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    CosNaming::NamingContext::marshalObjRef((_0RL_result.operator->()),_0RL_s);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"bind_new_context") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _0RL_s;
    _0RL_s.RequestReceived();
    CosNaming::NamingContext_var _0RL_result;
    try {
      _0RL_result = bind_new_context ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 50;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 46;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,46);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 48;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,44);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 49;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 45;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,45);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize = CosNaming::NamingContext::NP_alignedSize((_0RL_result.operator->()),_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    CosNaming::NamingContext::marshalObjRef((_0RL_result.operator->()),_0RL_s);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"destroy") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _0RL_s.RequestReceived();
        try {
      destroy (  );
    }
    catch ( CosNaming::NamingContext::NotEmpty &_0RL_ex) {
      size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
      _0RL_msgsize += 45;
      _0RL_msgsize = _0RL_ex.NP_alignedSize(_0RL_msgsize);
      _0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
      CORBA::ULong _len = 41;
      _len >>= _0RL_s;
      _0RL_s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotEmpty_IntfRepoID,41);
      _0RL_ex >>= _0RL_s;
      _0RL_s.ReplyCompleted();
      return 1;
    }
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"list") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CORBA::ULong how_many;
    how_many <<= _0RL_s;
    CosNaming::BindingList_var bl;
    CosNaming::BindingIterator_var bi;
    _0RL_s.RequestReceived();
        CosNaming_NamingContext::list ( how_many, bl, bi );
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize = (bl.operator->())->NP_alignedSize(_0RL_msgsize);
    _0RL_msgsize = CosNaming::BindingIterator::NP_alignedSize((bi.operator->()),_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    *(bl.operator->()) >>= _0RL_s;
    CosNaming::BindingIterator::marshalObjRef((bi.operator->()),_0RL_s);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else {
    return 0;
  }
}

CosNaming::_sk_NamingContext::_sk_NamingContext (const omniORB::objectKey& k)
{
  omniRopeAndKey l(0,(CORBA::Octet*)&k,(CORBA::ULong)sizeof(k));
  setRopeAndKey(l,0);
}

omniORB::objectKey
CosNaming::_sk_NamingContext::_key()
{
  omniRopeAndKey l;
  getRopeAndKey(l);
  return (*((omniORB::objectKey*)l.key()));
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_duplicate(CosNaming::NamingContext_ptr obj)
{
  if (CORBA::is_nil(obj))
    return CosNaming::NamingContext::_nil();
  CORBA::Object::_duplicate(obj);
  return obj;
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_narrow(CORBA::Object_ptr obj)
{
  if (CORBA::is_nil(obj))
    return CosNaming::NamingContext::_nil();
  CosNaming::NamingContext_ptr e = (CosNaming::NamingContext_ptr) ((obj->PR_getobj())->_widenFromTheMostDerivedIntf(CosNaming_NamingContext_IntfRepoID));
  if (e) {
    CORBA::Object::_duplicate(e);
    return e;
  }
  else
    return CosNaming::NamingContext::_nil();
}

void *
CosNaming::NamingContext::_widenFromTheMostDerivedIntf(const char *repoId)
{
  if (!repoId)
    return (void *)((CORBA::Object_ptr)this);
  if (strcmp(CosNaming_NamingContext_IntfRepoID,repoId) == 0)
    return (void *)this;
  else {
    return 0;
  }
}

CORBA::Boolean
CosNaming::NamingContext::_is_a(const char *base_repoId) {
  if (strcmp(base_repoId,(const char *)CosNaming_NamingContext_IntfRepoID)==0)
    return 1;
  else
    return 0;
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext_Helper::_nil() {
  return CosNaming::NamingContext::_nil();
}

CORBA::Boolean
CosNaming::NamingContext_Helper::is_nil(CosNaming::NamingContext_ptr p) {
  return CORBA::is_nil(p);
}

void
CosNaming::NamingContext_Helper::release(CosNaming::NamingContext_ptr p) {
  CORBA::release(p);
}

void
CosNaming::NamingContext_Helper::duplicate(CosNaming::NamingContext_ptr p) {
  CORBA::Object::_duplicate(p);
}

size_t
CosNaming::NamingContext_Helper::NP_alignedSize(CosNaming::NamingContext_ptr obj,size_t initialoffset) {
  return CosNaming::NamingContext::NP_alignedSize(obj,initialoffset);
}

void
CosNaming::NamingContext_Helper::marshalObjRef(CosNaming::NamingContext_ptr obj,NetBufferedStream &s) {
  CosNaming::NamingContext::marshalObjRef(obj,s);
}

CosNaming::NamingContext_ptr CosNaming::NamingContext_Helper::unmarshalObjRef(NetBufferedStream &s) {
  return CosNaming::NamingContext::unmarshalObjRef(s);
}

void
CosNaming::NamingContext_Helper::marshalObjRef(CosNaming::NamingContext_ptr obj,MemBufferedStream &s) {
  CosNaming::NamingContext::marshalObjRef(obj,s);
}

CosNaming::NamingContext_ptr CosNaming::NamingContext_Helper::unmarshalObjRef(MemBufferedStream &s) {
  return CosNaming::NamingContext::unmarshalObjRef(s);
}

const char *
CosNaming::NamingContext_proxyObjectFactory::irRepoId() const
{
  return (const char *)CosNaming_NamingContext_IntfRepoID;
}

CORBA::Object_ptr
CosNaming::NamingContext_proxyObjectFactory::newProxyObject(Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release)
{
  CosNaming::_proxy_NamingContext *p = new CosNaming::_proxy_NamingContext(r,key,keysize,profiles,release);
  if (!p) {
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  }
  return (CORBA::Object_ptr) p;
}

CORBA::Boolean
CosNaming::NamingContext_proxyObjectFactory::is_a(const char *base_repoId) const
{
  return CosNaming::NamingContext::_is_a(base_repoId);

}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_nil() {
  return CosNaming::NamingContext_proxyObjectFactory::_nil();
}

static const CosNaming::NamingContext_proxyObjectFactory CosNaming_NamingContext_proxyObjectFactory;
CosNaming::NamingContext_ptr CosNaming::NamingContext_proxyObjectFactory::__nil_NamingContext = 0;

CORBA::Boolean  CosNaming::_proxy_BindingIterator::_0RL__next_one ( CosNaming::Binding *& b )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CosNaming::Binding * _b= 0;
  CORBA::Boolean _0RL_result;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),9);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"next_one",9,_0RL_msgsize,0);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _b = new CosNaming::Binding;
        _0RL_result <<= _0RL_c;
        *_b <<= _0RL_c;
        _0RL_c.RequestCompleted();
        b = _b;
        return _0RL_result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        return next_one ( b );
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_b) delete _b;
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_b) delete _b;
    throw;
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _0RL_result = 0;
    return _0RL_result;
  }
}

CORBA::Boolean  CosNaming::_proxy_BindingIterator::_0RL__next_n ( CORBA::ULong  how_many, CosNaming::BindingList *& bl )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  CosNaming::BindingList * _bl= 0;
  CORBA::Boolean _0RL_result;
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),7);
    _0RL_msgsize = omni::align_to(_0RL_msgsize,omni::ALIGN_4);
    _0RL_msgsize += 4;
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"next_n",7,_0RL_msgsize,0);
    how_many >>= _0RL_c;
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _bl = new CosNaming::BindingList;
        _0RL_result <<= _0RL_c;
        *_bl <<= _0RL_c;
        _0RL_c.RequestCompleted();
        bl = _bl;
        return _0RL_result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        return next_n ( how_many, bl );
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_bl) delete _bl;
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    if (_bl) delete _bl;
    throw;
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _0RL_result = 0;
    return _0RL_result;
  }
}

void CosNaming::_proxy_BindingIterator::destroy (  )
{
  assertObjectExistent();
  omniRopeAndKey _0RL_r;
  CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);
  try {
    GIOP_C _0RL_c(_0RL_r.rope());
    CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),8);
    _0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)"destroy",8,_0RL_msgsize,0);
    switch (_0RL_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _0RL_c.RequestCompleted();
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _0RL_c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);
          _0RL_c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              cerr << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference." << endl;
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL__r;
          obj->PR_getobj()->getRopeAndKey(_0RL__r);
          setRopeAndKey(_0RL__r);
          _0RL_c.~GIOP_C();
        }
        if (omniORB::traceLevel > 10) {
          cerr << "GIOP::LOCATION_FORWARD: retry request." << endl;
        }
        destroy (  );
        return;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_0RL_fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
  catch (...) {
    throw;
  }
}


CORBA::Boolean
CosNaming::_sk_BindingIterator::dispatch(GIOP_S &_0RL_s,const char *_0RL_op,CORBA::Boolean _0RL_response_expected)
{
  if (strcmp(_0RL_op,"next_one") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Binding_var b;
    _0RL_s.RequestReceived();
    CORBA::Boolean _0RL_result;
    _0RL_result = CosNaming_BindingIterator::next_one ( b );
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize += 1;
    _0RL_msgsize = (b.operator->())->NP_alignedSize(_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_result >>= _0RL_s;
    *(b.operator->()) >>= _0RL_s;
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"next_n") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CORBA::ULong how_many;
    how_many <<= _0RL_s;
    CosNaming::BindingList_var bl;
    _0RL_s.RequestReceived();
    CORBA::Boolean _0RL_result;
    _0RL_result = CosNaming_BindingIterator::next_n ( how_many, bl );
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_msgsize += 1;
    _0RL_msgsize = (bl.operator->())->NP_alignedSize(_0RL_msgsize);
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_result >>= _0RL_s;
    *(bl.operator->()) >>= _0RL_s;
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_0RL_op,"destroy") == 0)
  {
    if (!_0RL_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _0RL_s.RequestReceived();
        destroy (  );
    size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);
    _0RL_s.ReplyCompleted();
    return 1;
  }
  else {
    return 0;
  }
}

CosNaming::_sk_BindingIterator::_sk_BindingIterator (const omniORB::objectKey& k)
{
  omniRopeAndKey l(0,(CORBA::Octet*)&k,(CORBA::ULong)sizeof(k));
  setRopeAndKey(l,0);
}

omniORB::objectKey
CosNaming::_sk_BindingIterator::_key()
{
  omniRopeAndKey l;
  getRopeAndKey(l);
  return (*((omniORB::objectKey*)l.key()));
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_duplicate(CosNaming::BindingIterator_ptr obj)
{
  if (CORBA::is_nil(obj))
    return CosNaming::BindingIterator::_nil();
  CORBA::Object::_duplicate(obj);
  return obj;
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_narrow(CORBA::Object_ptr obj)
{
  if (CORBA::is_nil(obj))
    return CosNaming::BindingIterator::_nil();
  CosNaming::BindingIterator_ptr e = (CosNaming::BindingIterator_ptr) ((obj->PR_getobj())->_widenFromTheMostDerivedIntf(CosNaming_BindingIterator_IntfRepoID));
  if (e) {
    CORBA::Object::_duplicate(e);
    return e;
  }
  else
    return CosNaming::BindingIterator::_nil();
}

void *
CosNaming::BindingIterator::_widenFromTheMostDerivedIntf(const char *repoId)
{
  if (!repoId)
    return (void *)((CORBA::Object_ptr)this);
  if (strcmp(CosNaming_BindingIterator_IntfRepoID,repoId) == 0)
    return (void *)this;
  else {
    return 0;
  }
}

CORBA::Boolean
CosNaming::BindingIterator::_is_a(const char *base_repoId) {
  if (strcmp(base_repoId,(const char *)CosNaming_BindingIterator_IntfRepoID)==0)
    return 1;
  else
    return 0;
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator_Helper::_nil() {
  return CosNaming::BindingIterator::_nil();
}

CORBA::Boolean
CosNaming::BindingIterator_Helper::is_nil(CosNaming::BindingIterator_ptr p) {
  return CORBA::is_nil(p);
}

void
CosNaming::BindingIterator_Helper::release(CosNaming::BindingIterator_ptr p) {
  CORBA::release(p);
}

void
CosNaming::BindingIterator_Helper::duplicate(CosNaming::BindingIterator_ptr p) {
  CORBA::Object::_duplicate(p);
}

size_t
CosNaming::BindingIterator_Helper::NP_alignedSize(CosNaming::BindingIterator_ptr obj,size_t initialoffset) {
  return CosNaming::BindingIterator::NP_alignedSize(obj,initialoffset);
}

void
CosNaming::BindingIterator_Helper::marshalObjRef(CosNaming::BindingIterator_ptr obj,NetBufferedStream &s) {
  CosNaming::BindingIterator::marshalObjRef(obj,s);
}

CosNaming::BindingIterator_ptr CosNaming::BindingIterator_Helper::unmarshalObjRef(NetBufferedStream &s) {
  return CosNaming::BindingIterator::unmarshalObjRef(s);
}

void
CosNaming::BindingIterator_Helper::marshalObjRef(CosNaming::BindingIterator_ptr obj,MemBufferedStream &s) {
  CosNaming::BindingIterator::marshalObjRef(obj,s);
}

CosNaming::BindingIterator_ptr CosNaming::BindingIterator_Helper::unmarshalObjRef(MemBufferedStream &s) {
  return CosNaming::BindingIterator::unmarshalObjRef(s);
}

const char *
CosNaming::BindingIterator_proxyObjectFactory::irRepoId() const
{
  return (const char *)CosNaming_BindingIterator_IntfRepoID;
}

CORBA::Object_ptr
CosNaming::BindingIterator_proxyObjectFactory::newProxyObject(Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release)
{
  CosNaming::_proxy_BindingIterator *p = new CosNaming::_proxy_BindingIterator(r,key,keysize,profiles,release);
  if (!p) {
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  }
  return (CORBA::Object_ptr) p;
}

CORBA::Boolean
CosNaming::BindingIterator_proxyObjectFactory::is_a(const char *base_repoId) const
{
  return CosNaming::BindingIterator::_is_a(base_repoId);

}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_nil() {
  return CosNaming::BindingIterator_proxyObjectFactory::_nil();
}

static const CosNaming::BindingIterator_proxyObjectFactory CosNaming_BindingIterator_proxyObjectFactory;
CosNaming::BindingIterator_ptr CosNaming::BindingIterator_proxyObjectFactory::__nil_BindingIterator = 0;

