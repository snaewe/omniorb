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
  _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
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
  _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
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
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),5);
  _msgsize = n.NP_alignedSize(_msgsize);
  _msgsize = CORBA::Object::NP_alignedSize(obj,_msgsize);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"bind",5,_msgsize,0);
  n >>= _c;
  CORBA::Object::marshalObjRef(obj,_c);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
        CosNaming::NamingContext::AlreadyBound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

void CosNaming::_proxy_NamingContext::rebind ( const CosNaming::Name & n, CORBA::Object_ptr  obj )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),7);
  _msgsize = n.NP_alignedSize(_msgsize);
  _msgsize = CORBA::Object::NP_alignedSize(obj,_msgsize);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"rebind",7,_msgsize,0);
  n >>= _c;
  CORBA::Object::marshalObjRef(obj,_c);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

void CosNaming::_proxy_NamingContext::bind_context ( const CosNaming::Name & n, CosNaming::NamingContext_ptr  nc )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),13);
  _msgsize = n.NP_alignedSize(_msgsize);
  _msgsize = CosNaming::NamingContext::NP_alignedSize(nc,_msgsize);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"bind_context",13,_msgsize,0);
  n >>= _c;
  CosNaming::NamingContext::marshalObjRef(nc,_c);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
        CosNaming::NamingContext::AlreadyBound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

void CosNaming::_proxy_NamingContext::rebind_context ( const CosNaming::Name & n, CosNaming::NamingContext_ptr  nc )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),15);
  _msgsize = n.NP_alignedSize(_msgsize);
  _msgsize = CosNaming::NamingContext::NP_alignedSize(nc,_msgsize);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"rebind_context",15,_msgsize,0);
  n >>= _c;
  CosNaming::NamingContext::marshalObjRef(nc,_c);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

CORBA::Object_ptr  CosNaming::_proxy_NamingContext::resolve ( const CosNaming::Name & n )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),8);
  _msgsize = n.NP_alignedSize(_msgsize);
  CORBA::Object_ptr _result= 0;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"resolve",8,_msgsize,0);
  n >>= _c;
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _result = CORBA::Object::unmarshalObjRef(_c);
        _c.RequestCompleted();
        return _result;
      }
      catch (...) {
        if (_result) CORBA::release(_result);
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Object_ptr _result= 0;
    return _result;
  }
}

void CosNaming::_proxy_NamingContext::unbind ( const CosNaming::Name & n )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),7);
  _msgsize = n.NP_alignedSize(_msgsize);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"unbind",7,_msgsize,0);
  n >>= _c;
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

CosNaming::NamingContext_ptr  CosNaming::_proxy_NamingContext::new_context (  )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),12);
  CosNaming::NamingContext_ptr _result= 0;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"new_context",12,_msgsize,0);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _result = CosNaming::NamingContext::unmarshalObjRef(_c);
        _c.RequestCompleted();
        return _result;
      }
      catch (...) {
        if (_result) CORBA::release(_result);
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CosNaming::NamingContext_ptr _result= 0;
    return _result;
  }
}

CosNaming::NamingContext_ptr  CosNaming::_proxy_NamingContext::bind_new_context ( const CosNaming::Name & n )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),17);
  _msgsize = n.NP_alignedSize(_msgsize);
  CosNaming::NamingContext_ptr _result= 0;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"bind_new_context",17,_msgsize,0);
  n >>= _c;
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _result = CosNaming::NamingContext::unmarshalObjRef(_c);
        _c.RequestCompleted();
        return _result;
      }
      catch (...) {
        if (_result) CORBA::release(_result);
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[49];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 49) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotFound_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotFound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_CannotProceed_IntfRepoID) == 0) {
        CosNaming::NamingContext::CannotProceed _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_InvalidName_IntfRepoID) == 0) {
        CosNaming::NamingContext::InvalidName _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else if (strcmp((const char *)_excId,CosNaming_NamingContext_AlreadyBound_IntfRepoID) == 0) {
        CosNaming::NamingContext::AlreadyBound _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CosNaming::NamingContext_ptr _result= 0;
    return _result;
  }
}

void CosNaming::_proxy_NamingContext::destroy (  )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),8);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"destroy",8,_msgsize,0);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      CORBA::Char _excId[44];
      CORBA::ULong _len;
      _len <<= _c;
      if (_len > 44) {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      else {
        _c.get_char_array(_excId,_len);
      }
      if (strcmp((const char *)_excId,CosNaming_NamingContext_NotEmpty_IntfRepoID) == 0) {
        CosNaming::NamingContext::NotEmpty _ex;
        _ex <<= _c;
        _c.RequestCompleted();
        throw _ex;
      }
      else {
        _c.RequestCompleted(1);
        throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}

void CosNaming::_proxy_NamingContext::list ( CORBA::ULong  how_many, CosNaming::BindingList *& bl, CosNaming::BindingIterator_ptr & bi )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),5);
  _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
  _msgsize += 4;
  CosNaming::BindingList * _bl= 0;
  CosNaming::BindingIterator_ptr _bi= 0;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"list",5,_msgsize,0);
  how_many >>= _c;
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _bl = new CosNaming::BindingList;
        *_bl <<= _c;
        _bi = CosNaming::BindingIterator::unmarshalObjRef(_c);
        _c.RequestCompleted();
        bl = _bl;
        bi = _bi;
      }
      catch (...) {
        if (_bl) delete _bl;
        if (_bi) CORBA::release(_bi);
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}


CORBA::Boolean
CosNaming::_sk_NamingContext::dispatch(GIOP_S &_s,const char *_op,CORBA::Boolean _response_expected)
{
  if (strcmp(_op,"bind") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    CORBA::Object_var obj;
    obj = CORBA::Object::unmarshalObjRef(_s);
    _s.RequestReceived();
        try {
      bind ( n, obj );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 52;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 48;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,48);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"rebind") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    CORBA::Object_var obj;
    obj = CORBA::Object::unmarshalObjRef(_s);
    _s.RequestReceived();
        try {
      rebind ( n, obj );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"bind_context") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    CosNaming::NamingContext_var nc;
    nc = CosNaming::NamingContext::unmarshalObjRef(_s);
    _s.RequestReceived();
        try {
      bind_context ( n, nc );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 52;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 48;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,48);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"rebind_context") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    CosNaming::NamingContext_var nc;
    nc = CosNaming::NamingContext::unmarshalObjRef(_s);
    _s.RequestReceived();
        try {
      rebind_context ( n, nc );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"resolve") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    _s.RequestReceived();
    CORBA::Object_var _result;
    try {
      _result = resolve ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize = CORBA::Object::NP_alignedSize((_result.operator->()),_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    CORBA::Object::marshalObjRef((_result.operator->()),_s);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"unbind") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    _s.RequestReceived();
        try {
      unbind ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"new_context") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _s.RequestReceived();
    CosNaming::NamingContext_var _result;
    _result = new_context (  );
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize = CosNaming::NamingContext::NP_alignedSize((_result.operator->()),_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    CosNaming::NamingContext::marshalObjRef((_result.operator->()),_s);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"bind_new_context") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Name n;
    n <<= _s;
    _s.RequestReceived();
    CosNaming::NamingContext_var _result;
    try {
      _result = bind_new_context ( n );
    }
    catch ( CosNaming::NamingContext::NotFound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotFound_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::CannotProceed &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 53;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 49;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_CannotProceed_IntfRepoID,49);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::InvalidName &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 51;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 47;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_InvalidName_IntfRepoID,47);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    catch ( CosNaming::NamingContext::AlreadyBound &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 52;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 48;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_AlreadyBound_IntfRepoID,48);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize = CosNaming::NamingContext::NP_alignedSize((_result.operator->()),_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    CosNaming::NamingContext::marshalObjRef((_result.operator->()),_s);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"destroy") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _s.RequestReceived();
        try {
      destroy (  );
    }
    catch ( CosNaming::NamingContext::NotEmpty &ex) {
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
      _msgsize += 48;
      _msgsize = ex.NP_alignedSize(_msgsize);
      _s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);
      CORBA::ULong _len = 44;
      _len >>= _s;
      _s.put_char_array((CORBA::Char *)CosNaming_NamingContext_NotEmpty_IntfRepoID,44);
      ex >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"list") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CORBA::ULong how_many;
    how_many <<= _s;
    CosNaming::BindingList_var bl;
    CosNaming::BindingIterator_var bi;
    _s.RequestReceived();
        list ( how_many, bl, bi );
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize = (bl.operator->())->NP_alignedSize(_msgsize);
    _msgsize = CosNaming::BindingIterator::NP_alignedSize((bi.operator->()),_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    *(bl.operator->()) >>= _s;
    CosNaming::BindingIterator::marshalObjRef((bi.operator->()),_s);
    _s.ReplyCompleted();
    return 1;
  }
  else {
    return 0;
  }
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
CosNaming::NamingContext::_widenFromTheMostDerivedIntf(const char *repoId) throw()
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

CORBA::Boolean  CosNaming::_proxy_BindingIterator::next_one ( CosNaming::Binding *& b )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),9);
  CosNaming::Binding * _b= 0;
  CORBA::Boolean _result;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"next_one",9,_msgsize,0);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _b = new CosNaming::Binding;
        _result <<= _c;
        *_b <<= _c;
        _c.RequestCompleted();
        b = _b;
        return _result;
      }
      catch (...) {
        if (_b) delete _b;
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _result = 0;
    return _result;
  }
}

CORBA::Boolean  CosNaming::_proxy_BindingIterator::next_n ( CORBA::ULong  how_many, CosNaming::BindingList *& bl )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),7);
  _msgsize = omniORB::align_to(_msgsize,omniORB::ALIGN_4);
  _msgsize += 4;
  CosNaming::BindingList * _bl= 0;
  CORBA::Boolean _result;
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"next_n",7,_msgsize,0);
  how_many >>= _c;
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      try {
        _bl = new CosNaming::BindingList;
        _result <<= _c;
        *_bl <<= _c;
        _c.RequestCompleted();
        bl = _bl;
        return _result;
      }
      catch (...) {
        if (_bl) delete _bl;
        throw;
      }
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _result = 0;
    return _result;
  }
}

void CosNaming::_proxy_BindingIterator::destroy (  )
{
  GIOP_C _c(_rope());
  CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),8);
  _c.InitialiseRequest(objkey(),objkeysize(),(char *)"destroy",8,_msgsize,0);
  switch (_c.ReceiveReply())
  {
    case GIOP::NO_EXCEPTION:
    {
      _c.RequestCompleted();
      break;
    }
    case GIOP::USER_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      break;
    }
    case GIOP::SYSTEM_EXCEPTION:
    {
      _c.RequestCompleted(1);
      throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
    }
    case GIOP::LOCATION_FORWARD:
    {
      _c.RequestCompleted(1);
      throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);
    }
  }
}


CORBA::Boolean
CosNaming::_sk_BindingIterator::dispatch(GIOP_S &_s,const char *_op,CORBA::Boolean _response_expected)
{
  if (strcmp(_op,"next_one") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CosNaming::Binding_var b;
    _s.RequestReceived();
    CORBA::Boolean _result;
    _result = next_one ( b );
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize += 1;
    _msgsize = (b.operator->())->NP_alignedSize(_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _result >>= _s;
    *(b.operator->()) >>= _s;
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"next_n") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    CORBA::ULong how_many;
    how_many <<= _s;
    CosNaming::BindingList_var bl;
    _s.RequestReceived();
    CORBA::Boolean _result;
    _result = next_n ( how_many, bl );
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _msgsize += 1;
    _msgsize = (bl.operator->())->NP_alignedSize(_msgsize);
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _result >>= _s;
    *(bl.operator->()) >>= _s;
    _s.ReplyCompleted();
    return 1;
  }
  else if (strcmp(_op,"destroy") == 0)
  {
    if (!_response_expected) {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
    _s.RequestReceived();
        destroy (  );
    size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
    _s.ReplyCompleted();
    return 1;
  }
  else {
    return 0;
  }
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
CosNaming::BindingIterator::_widenFromTheMostDerivedIntf(const char *repoId) throw()
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

