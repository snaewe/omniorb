// This file is hand-modified from omniORB 3.0 stubs. Do not delete.

#include "Naming.hh"
#include <omniORB4/callDescriptor.h>

static const char* _0RL_library_version = omniORB_4_0;

void
CosNaming::NameComponent::operator>>= (cdrStream &_n) const
{
  id >>= _n;
  kind >>= _n;
  
}

void
CosNaming::NameComponent::operator<<= (cdrStream &_n)
{
  id <<= _n;
  kind <<= _n;
  
}

void
CosNaming::Binding::operator>>= (cdrStream &_n) const
{
  binding_name >>= _n;
  binding_type >>= _n;
  
}

void
CosNaming::Binding::operator<<= (cdrStream &_n)
{
  binding_name <<= _n;
  binding_type <<= _n;
  
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContext::NotFound::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContext::NotFound::insertToAnyFnNCP = 0;

CosNaming::NamingContext::NotFound::NotFound(const CosNaming::NamingContext::NotFound& _s) : CORBA::UserException(_s)
{
  why = _s.why;
  rest_of_name = _s.rest_of_name;
  
}

CosNaming::NamingContext::NotFound::NotFound(NotFoundReason _why, const Name _rest_of_name)
{
  pd_insertToAnyFn    = CosNaming::NamingContext::NotFound::insertToAnyFn;
  pd_insertToAnyFnNCP = CosNaming::NamingContext::NotFound::insertToAnyFnNCP;
  why = _why;
  rest_of_name = _rest_of_name;
  
}

CosNaming::NamingContext::NotFound& CosNaming::NamingContext::NotFound::operator=(const CosNaming::NamingContext::NotFound& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  why = _s.why;
  rest_of_name = _s.rest_of_name;
  
  return *this;
}

CosNaming::NamingContext::NotFound::~NotFound() {}

void CosNaming::NamingContext::NotFound::_raise() { throw *this; }

CosNaming::NamingContext::NotFound* CosNaming::NamingContext::NotFound::_downcast(CORBA::Exception* _e) {
  return (NotFound*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::NotFound");
}

const CosNaming::NamingContext::NotFound* CosNaming::NamingContext::NotFound::_downcast(const CORBA::Exception* _e) {
  return (const NotFound*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::NotFound");
}

const char* CosNaming::NamingContext::NotFound::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0";

CORBA::Exception* CosNaming::NamingContext::NotFound::_NP_duplicate() const {
  return new NotFound(*this);
}

const char* CosNaming::NamingContext::NotFound::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContext::NotFound";
}

const char* CosNaming::NamingContext::NotFound::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContext/NotFound:1.0");
  return "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0";
}

void CosNaming::NamingContext::NotFound::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

void
CosNaming::NamingContext::NotFound::operator>>= (cdrStream& _n) const
{
  why >>= _n;
  rest_of_name >>= _n;
  
}

void
CosNaming::NamingContext::NotFound::operator<<= (cdrStream& _n)
{
  why <<= _n;
  rest_of_name <<= _n;
  
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContext::CannotProceed::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContext::CannotProceed::insertToAnyFnNCP = 0;

CosNaming::NamingContext::CannotProceed::CannotProceed(const CosNaming::NamingContext::CannotProceed& _s) : CORBA::UserException(_s)
{
  cxt = _s.cxt;
  rest_of_name = _s.rest_of_name;
  
}

CosNaming::NamingContext::CannotProceed::CannotProceed(NamingContext_ptr _cxt, const Name _rest_of_name)
{
  pd_insertToAnyFn    = CosNaming::NamingContext::CannotProceed::insertToAnyFn;
  pd_insertToAnyFnNCP = CosNaming::NamingContext::CannotProceed::insertToAnyFnNCP;
  CosNaming::NamingContext_Helper::duplicate(_cxt);
  cxt = _cxt;
  rest_of_name = _rest_of_name;
  
}

CosNaming::NamingContext::CannotProceed& CosNaming::NamingContext::CannotProceed::operator=(const CosNaming::NamingContext::CannotProceed& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  cxt = _s.cxt;
  rest_of_name = _s.rest_of_name;
  
  return *this;
}

CosNaming::NamingContext::CannotProceed::~CannotProceed() {}

void CosNaming::NamingContext::CannotProceed::_raise() { throw *this; }

CosNaming::NamingContext::CannotProceed* CosNaming::NamingContext::CannotProceed::_downcast(CORBA::Exception* _e) {
  return (CannotProceed*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::CannotProceed");
}

const CosNaming::NamingContext::CannotProceed* CosNaming::NamingContext::CannotProceed::_downcast(const CORBA::Exception* _e) {
  return (const CannotProceed*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::CannotProceed");
}

const char* CosNaming::NamingContext::CannotProceed::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0";

CORBA::Exception* CosNaming::NamingContext::CannotProceed::_NP_duplicate() const {
  return new CannotProceed(*this);
}

const char* CosNaming::NamingContext::CannotProceed::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContext::CannotProceed";
}

const char* CosNaming::NamingContext::CannotProceed::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0");
  return "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0";
}

void CosNaming::NamingContext::CannotProceed::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

void
CosNaming::NamingContext::CannotProceed::operator>>= (cdrStream& _n) const
{
  NamingContext_Helper::marshalObjRef(cxt,_n);
  rest_of_name >>= _n;
  
}

void
CosNaming::NamingContext::CannotProceed::operator<<= (cdrStream& _n)
{
  cxt = NamingContext_Helper::unmarshalObjRef(_n);
  rest_of_name <<= _n;
  
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContext::InvalidName::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContext::InvalidName::insertToAnyFnNCP = 0;

CosNaming::NamingContext::InvalidName::InvalidName(const CosNaming::NamingContext::InvalidName& _s) : CORBA::UserException(_s)
{
  
}

CosNaming::NamingContext::InvalidName& CosNaming::NamingContext::InvalidName::operator=(const CosNaming::NamingContext::InvalidName& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  
  return *this;
}

CosNaming::NamingContext::InvalidName::~InvalidName() {}

void CosNaming::NamingContext::InvalidName::_raise() { throw *this; }

CosNaming::NamingContext::InvalidName* CosNaming::NamingContext::InvalidName::_downcast(CORBA::Exception* _e) {
  return (InvalidName*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::InvalidName");
}

const CosNaming::NamingContext::InvalidName* CosNaming::NamingContext::InvalidName::_downcast(const CORBA::Exception* _e) {
  return (const InvalidName*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::InvalidName");
}

const char* CosNaming::NamingContext::InvalidName::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0";

CORBA::Exception* CosNaming::NamingContext::InvalidName::_NP_duplicate() const {
  return new InvalidName(*this);
}

const char* CosNaming::NamingContext::InvalidName::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContext::InvalidName";
}

const char* CosNaming::NamingContext::InvalidName::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0");
  return "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0";
}

void CosNaming::NamingContext::InvalidName::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContext::AlreadyBound::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContext::AlreadyBound::insertToAnyFnNCP = 0;

CosNaming::NamingContext::AlreadyBound::AlreadyBound(const CosNaming::NamingContext::AlreadyBound& _s) : CORBA::UserException(_s)
{
}

CosNaming::NamingContext::AlreadyBound& CosNaming::NamingContext::AlreadyBound::operator=(const CosNaming::NamingContext::AlreadyBound& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  
  return *this;
}

CosNaming::NamingContext::AlreadyBound::~AlreadyBound() {}

void CosNaming::NamingContext::AlreadyBound::_raise() { throw *this; }

CosNaming::NamingContext::AlreadyBound* CosNaming::NamingContext::AlreadyBound::_downcast(CORBA::Exception* _e) {
  return (AlreadyBound*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::AlreadyBound");
}

const CosNaming::NamingContext::AlreadyBound* CosNaming::NamingContext::AlreadyBound::_downcast(const CORBA::Exception* _e) {
  return (const AlreadyBound*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::AlreadyBound");
}

const char* CosNaming::NamingContext::AlreadyBound::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0";

CORBA::Exception* CosNaming::NamingContext::AlreadyBound::_NP_duplicate() const {
  return new AlreadyBound(*this);
}

const char* CosNaming::NamingContext::AlreadyBound::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContext::AlreadyBound";
}

const char* CosNaming::NamingContext::AlreadyBound::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0");
  return "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0";
}

void CosNaming::NamingContext::AlreadyBound::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContext::NotEmpty::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContext::NotEmpty::insertToAnyFnNCP = 0;

CosNaming::NamingContext::NotEmpty::NotEmpty(const CosNaming::NamingContext::NotEmpty& _s) : CORBA::UserException(_s)
{
  
}

CosNaming::NamingContext::NotEmpty& CosNaming::NamingContext::NotEmpty::operator=(const CosNaming::NamingContext::NotEmpty& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  
  return *this;
}

CosNaming::NamingContext::NotEmpty::~NotEmpty() {}

void CosNaming::NamingContext::NotEmpty::_raise() { throw *this; }

CosNaming::NamingContext::NotEmpty* CosNaming::NamingContext::NotEmpty::_downcast(CORBA::Exception* _e) {
  return (NotEmpty*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::NotEmpty");
}

const CosNaming::NamingContext::NotEmpty* CosNaming::NamingContext::NotEmpty::_downcast(const CORBA::Exception* _e) {
  return (const NotEmpty*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContext::NotEmpty");
}

const char* CosNaming::NamingContext::NotEmpty::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext/NotEmpty:1.0";

CORBA::Exception* CosNaming::NamingContext::NotEmpty::_NP_duplicate() const {
  return new NotEmpty(*this);
}

const char* CosNaming::NamingContext::NotEmpty::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContext::NotEmpty";
}

const char* CosNaming::NamingContext::NotEmpty::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContext/NotEmpty:1.0");
  return "IDL:omg.org/CosNaming/NamingContext/NotEmpty:1.0";
}

void CosNaming::NamingContext::NotEmpty::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

CosNaming::NamingContext_ptr CosNaming::NamingContext_Helper::_nil() {
  return CosNaming::NamingContext::_nil();
}

CORBA::Boolean CosNaming::NamingContext_Helper::is_nil(CosNaming::NamingContext_ptr p) {
  return CORBA::is_nil(p);

}

void CosNaming::NamingContext_Helper::release(CosNaming::NamingContext_ptr p) {
  CORBA::release(p);
}

void CosNaming::NamingContext_Helper::duplicate(CosNaming::NamingContext_ptr p) {
  if( p && !p->_NP_is_nil() )  omni::duplicateObjRef(p);
}

void CosNaming::NamingContext_Helper::marshalObjRef(CosNaming::NamingContext_ptr obj, cdrStream& s) {
  CosNaming::NamingContext::_marshalObjRef(obj, s);
}

CosNaming::NamingContext_ptr CosNaming::NamingContext_Helper::unmarshalObjRef(cdrStream& s) {
  return CosNaming::NamingContext::_unmarshalObjRef(s);
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_duplicate(CosNaming::NamingContext_ptr obj)
{
  if( obj && !obj->_NP_is_nil() )  omni::duplicateObjRef(obj);

  return obj;
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_narrow(CORBA::Object_ptr obj)
{
  if( !obj || obj->_NP_is_nil() || obj->_NP_is_pseudo() ) return _nil();
  _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
  return e ? e : _nil();
}

CosNaming::NamingContext_ptr
CosNaming::NamingContext::_nil()
{
  static _objref_NamingContext* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
  if( !_the_nil_ptr )  _the_nil_ptr = new _objref_NamingContext;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* CosNaming::NamingContext::_PD_repoId = "IDL:omg.org/CosNaming/NamingContext:1.0";

CosNaming::_objref_NamingContext::~_objref_NamingContext() {}

CosNaming::_objref_NamingContext::_objref_NamingContext(omniIOR* ior,
   omniIdentity* id, omniLocalIdentity* lid) :
   
   omniObjRef(CosNaming::NamingContext::_PD_repoId, ior, id, lid)
{
  _PR_setobj(this);
}

void*
CosNaming::_objref_NamingContext::_ptrToObjRef(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(id, CosNaming::NamingContext::_PD_repoId) )
    return (CosNaming::NamingContext_ptr) this;
  
  return 0;
}

// Proxy call descriptor class. Mangled signature:
//  void_i_cCosNaming_mName_i_cCORBA_mObject_e_cCosNaming_mNamingContext_mAlreadyBound_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_00000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_00000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Object_ptr a_1, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 4, upcall),
     arg_0(a_0),
     arg_1(a_1) {}

  inline ~_0RL_cd_69ceca6a39f685b5_00000000() {
    if (is_upcall()) {
      CORBA::release(arg_1);
    }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);

  void userException(GIOP_C&, const char*);
  
  CosNaming::Name& arg_0;
  CORBA::Object_ptr arg_1;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_00000000::_user_exns[] = {
    "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0",
    "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
    "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
    "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
  };

void _0RL_cd_69ceca6a39f685b5_00000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  CORBA::Object::_marshalObjRef(arg_1,s);
}

void _0RL_cd_69ceca6a39f685b5_00000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  arg_1 = CORBA::Object::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_00000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::AlreadyBound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::AlreadyBound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_10000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_00000000* tcd = (_0RL_cd_69ceca6a39f685b5_00000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  impl->bind(tcd->arg_0, tcd->arg_1);
#else
  if (!is_upcall())
    impl->bind(tcd->arg_0, tcd->arg_1);
  else {
    try {
      impl->bind(tcd->arg_0, tcd->arg_1);
    }

    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::bind(const Name& n, CORBA::Object_ptr obj)
{
  _0RL_cd_69ceca6a39f685b5_00000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_10000000, "bind", 5, (Name&)n, obj);
  
  _invoke(_call_desc);
  
}

// Proxy call descriptor class. Mangled signature:
//  void_i_cCosNaming_mName_i_cCORBA_mObject_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_20000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_20000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Object_ptr a_1, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 4, upcall),
     arg_0(a_0),
     arg_1(a_1) {}

  inline ~_0RL_cd_69ceca6a39f685b5_20000000() {
    if (is_upcall()) {
      CORBA::release(arg_1);
    }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);

  void userException(GIOP_C&, const char*);
  
  CosNaming::Name& arg_0;
  CORBA::Object_ptr arg_1;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_20000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
  "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_20000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  CORBA::Object::_marshalObjRef(arg_1,s);
}

void _0RL_cd_69ceca6a39f685b5_20000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  arg_1 = CORBA::Object::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_20000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;

  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_30000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_20000000* tcd = (_0RL_cd_69ceca6a39f685b5_20000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  impl->rebind(tcd->arg_0, tcd->arg_1);
#else
  if (!is_upcall())
    impl->rebind(tcd->arg_0, tcd->arg_1);
  else {
    try {
      impl->rebind(tcd->arg_0, tcd->arg_1);
    }

    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::rebind(const Name& n, CORBA::Object_ptr obj)
{
  _0RL_cd_69ceca6a39f685b5_20000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_30000000, "rebind", 7, (Name&) n, obj);
  
  _invoke(_call_desc);
  
}

// Proxy call descriptor class. Mangled signature:
//  void_i_cCosNaming_mName_i_cCosNaming_mNamingContext_e_cCosNaming_mNamingContext_mAlreadyBound_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_40000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_40000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CosNaming::NamingContext_ptr a_1, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 4, upcall),
     arg_0(a_0),
     arg_1(a_1) {}

  inline ~_0RL_cd_69ceca6a39f685b5_40000000() {
    if (is_upcall()) {
      CORBA::release(arg_1);
    }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void userException(GIOP_C&, const char*);
  
  CosNaming::Name& arg_0;
  CosNaming::NamingContext_ptr arg_1;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_40000000::_user_exns[] = {
    "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0",
    "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
    "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
    "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_40000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  CosNaming::NamingContext::_marshalObjRef(arg_1,s);
  
}

void _0RL_cd_69ceca6a39f685b5_40000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  arg_1 = CosNaming::NamingContext::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_40000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
 if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::AlreadyBound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::AlreadyBound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_50000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_40000000* tcd = (_0RL_cd_69ceca6a39f685b5_40000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  impl->bind_context(tcd->arg_0, tcd->arg_1);
#else
  if (!is_upcall())
    impl->bind_context(tcd->arg_0, tcd->arg_1);
  else {
    try {
      impl->bind_context(tcd->arg_0, tcd->arg_1);
    }

    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::bind_context(const Name& n, NamingContext_ptr nc)
{
  _0RL_cd_69ceca6a39f685b5_40000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_50000000, "bind_context", 13, (Name&)n, nc);
  
  _invoke(_call_desc);

}

// Proxy call descriptor class. Mangled signature:
//  void_i_cCosNaming_mName_i_cCosNaming_mNamingContext_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_60000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_60000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CosNaming::NamingContext_ptr a_1, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 3, upcall),
     arg_0(a_0),
     arg_1(a_1) {}

  inline ~_0RL_cd_69ceca6a39f685b5_60000000() {
    if (is_upcall()) {
      CORBA::release(arg_1);
    }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void userException(GIOP_C&, const char*);
  
  CosNaming::Name& arg_0;
  CosNaming::NamingContext_ptr arg_1;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_60000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
  "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_60000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  CosNaming::NamingContext::_marshalObjRef(arg_1,s);
  
}

void _0RL_cd_69ceca6a39f685b5_60000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  arg_1 = CosNaming::NamingContext::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_60000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_70000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_60000000* tcd = (_0RL_cd_69ceca6a39f685b5_60000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base

  impl->rebind_context(tcd->arg_0, tcd->arg_1);

#else
  if (!is_upcall())
    impl->rebind_context(tcd->arg_0, tcd->arg_1);
  else {
    try {
      impl->rebind_context(tcd->arg_0, tcd->arg_1);
    }
    
    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::rebind_context(const Name& n, NamingContext_ptr nc)
{
  _0RL_cd_69ceca6a39f685b5_60000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_70000000, "rebind_context", 15, (Name&)n, nc);
  
  _invoke(_call_desc);
  
}

// Proxy call descriptor class. Mangled signature:
//  _cCORBA_mObject_i_cCosNaming_mName_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_80000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_80000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 3, upcall),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  void userException(GIOP_C&, const char*);
  
  inline CORBA::Object_ptr result() { return pd_result._retn(); }
  CosNaming::Name& arg_0;
  CORBA::Object_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_80000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
  "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_80000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
}

void _0RL_cd_69ceca6a39f685b5_80000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
}

void _0RL_cd_69ceca6a39f685b5_80000000::unmarshalReturnedValues(cdrStream& s)
{
  
  pd_result = CORBA::Object::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_80000000::marshalReturnedValues(cdrStream& s)
{
  CORBA::Object::_marshalObjRef(pd_result,s);
}

void _0RL_cd_69ceca6a39f685b5_80000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_90000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_80000000* tcd = (_0RL_cd_69ceca6a39f685b5_80000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->resolve(tcd->arg_0);
#else
  if (!is_upcall())
    tcd->pd_result = impl->resolve(tcd->arg_0);
  else {
    try {
      tcd->pd_result = impl->resolve(tcd->arg_0);
    }
    
    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

CORBA::Object_ptr CosNaming::_objref_NamingContext::resolve(const Name& n)
{
  _0RL_cd_69ceca6a39f685b5_80000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_90000000, "resolve", 8, (Name&)n);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  void_i_cCosNaming_mName_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_a0000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_a0000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 3, upcall),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  
  void userException(GIOP_C&, const char*);
  
  CosNaming::Name& arg_0;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_a0000000::_user_exns[] = {
    "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
    "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
    "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_a0000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  
}

void _0RL_cd_69ceca6a39f685b5_a0000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_b0000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_a0000000* tcd = (_0RL_cd_69ceca6a39f685b5_a0000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  impl->unbind(tcd->arg_0);
#else
  if (is_upcall())
    impl->unbind(tcd->arg_0);
  else {
    try {
      impl->unbind(tcd->arg_0);
    }
    
    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::unbind(const Name& n)
{
  _0RL_cd_69ceca6a39f685b5_a0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_b0000000, "unbind", 7, (Name&)n);
  
  _invoke(_call_desc);
  
}

// Proxy call descriptor class. Mangled signature:
//  _cCosNaming_mNamingContext
class _0RL_cd_69ceca6a39f685b5_c0000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_c0000000(LocalCallFn lcfn, const char* op, size_t oplen, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall) {}

  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  inline CosNaming::NamingContext_ptr result() { return pd_result._retn(); }
  
  CosNaming::NamingContext_var pd_result;
};

void _0RL_cd_69ceca6a39f685b5_c0000000::unmarshalReturnedValues(cdrStream& s)
{
  
  pd_result = CosNaming::NamingContext::_unmarshalObjRef(s);
  
}

void _0RL_cd_69ceca6a39f685b5_c0000000::marshalReturnedValues(cdrStream& s)
{
  
  CosNaming::NamingContext::_marshalObjRef(pd_result,s);
  
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_d0000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_c0000000* tcd = (_0RL_cd_69ceca6a39f685b5_c0000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);
  tcd->pd_result = impl->new_context();
}

CosNaming::NamingContext_ptr CosNaming::_objref_NamingContext::new_context()
{
  _0RL_cd_69ceca6a39f685b5_c0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_d0000000, "new_context", 12);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  _cCosNaming_mNamingContext_i_cCosNaming_mName_e_cCosNaming_mNamingContext_mAlreadyBound_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_e0000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_e0000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 4, upcall),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  virtual void userException(GIOP_C&, const char*);
  
  inline CosNaming::NamingContext_ptr result() { return pd_result._retn(); }
  CosNaming::Name& arg_0;
  CosNaming::NamingContext_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_e0000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0",
  "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
  "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
};

void _0RL_cd_69ceca6a39f685b5_e0000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  
}

void _0RL_cd_69ceca6a39f685b5_e0000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  
}

void _0RL_cd_69ceca6a39f685b5_e0000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CosNaming::NamingContext::_unmarshalObjRef(s);
  
}

void _0RL_cd_69ceca6a39f685b5_e0000000::marshalReturnedValues(cdrStream& s)
{
  CosNaming::NamingContext::_marshalObjRef(pd_result,s);
  
}

void _0RL_cd_69ceca6a39f685b5_e0000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::AlreadyBound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::AlreadyBound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_f0000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_e0000000* tcd = (_0RL_cd_69ceca6a39f685b5_e0000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->bind_new_context(tcd->arg_0);
#else
  if (!is_upcall())
    tcd->pd_result = impl->bind_new_context(tcd->arg_0);
  else {
    try {
      tcd->pd_result = impl->bind_new_context(tcd->arg_0);
    }
    
    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

CosNaming::NamingContext_ptr CosNaming::_objref_NamingContext::bind_new_context(const Name& n)
{
  _0RL_cd_69ceca6a39f685b5_e0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_f0000000, "bind_new_context", 17, (Name&)n);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  void_e_cCosNaming_mNamingContext_mNotEmpty
class _0RL_cd_69ceca6a39f685b5_01000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_01000000(LocalCallFn lcfn, const char* op, size_t oplen, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 1, upcall) {}

  void userException(GIOP_C&, const char*);

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_01000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/NotEmpty:1.0"
};

void _0RL_cd_69ceca6a39f685b5_01000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotEmpty::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotEmpty _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_11000000(omniCallDescriptor* cd, omniServant* svnt)
{
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  impl->destroy();
#else
  if (!is_upcall())
    impl->destroy();
  else {
    try {
      impl->destroy();
    }

    catch(CosNaming::NamingContext::NotEmpty& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

void CosNaming::_objref_NamingContext::destroy()
{
  _0RL_cd_69ceca6a39f685b5_01000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_11000000, "destroy", 8);
  
  _invoke(_call_desc);
  
}

// Proxy call descriptor class. Mangled signature:
//  void_i_cunsigned_plong_o_cCosNaming_mBindingList_o_cCosNaming_mBindingIterator
class _0RL_cd_69ceca6a39f685b5_21000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_21000000(LocalCallFn lcfn, const char* op, size_t oplen, CORBA::ULong a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  CORBA::ULong arg_0;
  CosNaming::BindingList_var arg_1;
  CosNaming::BindingIterator_var arg_2;
  
};

void _0RL_cd_69ceca6a39f685b5_21000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  
}

void _0RL_cd_69ceca6a39f685b5_21000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  
}

void _0RL_cd_69ceca6a39f685b5_21000000::unmarshalReturnedValues(cdrStream& s)
{
  arg_1 =  new CosNaming::BindingList;
  
  (CosNaming::BindingList&)arg_1 <<= s;

  arg_2 = CosNaming::BindingIterator::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_21000000::marshalReturnedValues(cdrStream& s)
{
  (CosNaming::BindingList&)arg_1 >>= s;
  CosNaming::BindingIterator::_marshalObjRef(arg_2,s);
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_31000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_21000000* tcd = (_0RL_cd_69ceca6a39f685b5_21000000*) cd;
  CosNaming::_impl_NamingContext* impl = (CosNaming::_impl_NamingContext*) svnt->_ptrToInterface(CosNaming::NamingContext::_PD_repoId);

  impl->list(tcd->arg_0, tcd->arg_1.out(), tcd->arg_2.out());
}

void CosNaming::_objref_NamingContext::list(CORBA::ULong how_many, BindingList_out bl, _CORBA_ObjRef_OUT_arg< CosNaming::_objref_BindingIterator, BindingIterator_Helper > bi)
{
  _0RL_cd_69ceca6a39f685b5_21000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_31000000, "list", 5, how_many);
  
  _invoke(_call_desc);
  bl.ptr() = _call_desc.arg_1._retn();
  bi.ptr() = _call_desc.arg_2._retn();
}

CosNaming::_pof_NamingContext::~_pof_NamingContext() {}

omniObjRef*
CosNaming::_pof_NamingContext::newObjRef(omniIOR* ior,
               omniIdentity* id, omniLocalIdentity* lid)
{
  return new CosNaming::_objref_NamingContext(ior, id, lid);
}

CORBA::Boolean
CosNaming::_pof_NamingContext::is_a(const char* id) const
{
  if( !strcmp(id, CosNaming::NamingContext::_PD_repoId) )
    return 1;
  
  return 0;
}

const CosNaming::_pof_NamingContext _the_pof_CosNaming_mNamingContext;

CosNaming::_impl_NamingContext::~_impl_NamingContext() {}

CORBA::Boolean
CosNaming::_impl_NamingContext::_dispatch(GIOP_S& giop_s)
{
  const char* op = giop_s.invokeInfo().operation();

  if( !strcmp(op, "bind") ) {

    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_00000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_10000000, "bind", 5, arg_n, CORBA::Object::_nil(), 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "rebind") ) {

    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_20000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_30000000, "rebind", 7, arg_n, CORBA::Object::_nil(), 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "bind_context") ) {
    
    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_40000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_50000000, "bind_context", 13, arg_n, CosNaming::NamingContext::_nil(), 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "rebind_context") ) {

    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_60000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_70000000, "rebind_context", 15, arg_n, CosNaming::NamingContext::_nil(), 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "resolve") ) {
    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_80000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_90000000, "resolve", 8, arg_n, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "unbind") ) {

    Name arg_n;

    _0RL_cd_69ceca6a39f685b5_a0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_b0000000, "unbind", 7, arg_n, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "new_context") ) {
    
    _0RL_cd_69ceca6a39f685b5_c0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_d0000000, "new_context", 12, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "bind_new_context") ) {
    
    Name arg_n;
    
    _0RL_cd_69ceca6a39f685b5_e0000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_f0000000, "bind_new_context", 17, arg_n, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "destroy") ) {

    _0RL_cd_69ceca6a39f685b5_01000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_11000000, "destroy", 8, 1);
    _upcall(giop_s,_call_desc);
    return 1;

  }

  if( !strcmp(op, "list") ) {
    
    _0RL_cd_69ceca6a39f685b5_21000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_31000000, "list", 5, 0, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  return 0;
}

void*
CosNaming::_impl_NamingContext::_ptrToInterface(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (void*) 1;
  if( !strcmp(id, CosNaming::NamingContext::_PD_repoId) )
    return (_impl_NamingContext*) this;
  
  return 0;
}

const char*
CosNaming::_impl_NamingContext::_mostDerivedRepoId()
{
  return CosNaming::NamingContext::_PD_repoId;
}

CosNaming::BindingIterator_ptr CosNaming::BindingIterator_Helper::_nil() {
  return CosNaming::BindingIterator::_nil();
}

CORBA::Boolean CosNaming::BindingIterator_Helper::is_nil(CosNaming::BindingIterator_ptr p) {
  return CORBA::is_nil(p);

}

void CosNaming::BindingIterator_Helper::release(CosNaming::BindingIterator_ptr p) {
  CORBA::release(p);
}

void CosNaming::BindingIterator_Helper::duplicate(CosNaming::BindingIterator_ptr p) {
  if( p && !p->_NP_is_nil() )  omni::duplicateObjRef(p);
}

void CosNaming::BindingIterator_Helper::marshalObjRef(CosNaming::BindingIterator_ptr obj, cdrStream& s) {
  CosNaming::BindingIterator::_marshalObjRef(obj, s);
}

CosNaming::BindingIterator_ptr CosNaming::BindingIterator_Helper::unmarshalObjRef(cdrStream& s) {
  return CosNaming::BindingIterator::_unmarshalObjRef(s);
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_duplicate(CosNaming::BindingIterator_ptr obj)
{
  if( obj && !obj->_NP_is_nil() )  omni::duplicateObjRef(obj);

  return obj;
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_narrow(CORBA::Object_ptr obj)
{
  if( !obj || obj->_NP_is_nil() || obj->_NP_is_pseudo() ) return _nil();
  _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
  return e ? e : _nil();
}

CosNaming::BindingIterator_ptr
CosNaming::BindingIterator::_nil()
{
  static _objref_BindingIterator* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
  if( !_the_nil_ptr )  _the_nil_ptr = new _objref_BindingIterator;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* CosNaming::BindingIterator::_PD_repoId = "IDL:omg.org/CosNaming/BindingIterator:1.0";

CosNaming::_objref_BindingIterator::~_objref_BindingIterator() {}

CosNaming::_objref_BindingIterator::_objref_BindingIterator(omniIOR* ior, omniIdentity* id, omniLocalIdentity* lid) :
   
   omniObjRef(CosNaming::BindingIterator::_PD_repoId, ior, id, lid)
{
  _PR_setobj(this);
}

void*
CosNaming::_objref_BindingIterator::_ptrToObjRef(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(id, CosNaming::BindingIterator::_PD_repoId) )
    return (CosNaming::BindingIterator_ptr) this;
  
  return 0;
}

// Proxy call descriptor class. Mangled signature:
//  _cboolean_o_cCosNaming_mBinding
class _0RL_cd_69ceca6a39f685b5_41000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_41000000(LocalCallFn lcfn, const char* op, size_t oplen, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall) {}

  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);

  inline CORBA::Boolean result() { return pd_result; }
  CosNaming::Binding_var arg_0;
  CORBA::Boolean pd_result;
};

void _0RL_cd_69ceca6a39f685b5_41000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result <<= s;

  arg_0 = new CosNaming::Binding;

  (CosNaming::Binding&)arg_0 <<= s;
}

void _0RL_cd_69ceca6a39f685b5_41000000::marshalReturnedValues(cdrStream& s)
{
  pd_result >>= s;

  (CosNaming::Binding&)arg_0 >>= s;
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_51000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_41000000* tcd = (_0RL_cd_69ceca6a39f685b5_41000000*) cd;
  CosNaming::_impl_BindingIterator* impl = (CosNaming::_impl_BindingIterator*) svnt->_ptrToInterface(CosNaming::BindingIterator::_PD_repoId);
  tcd->pd_result = impl->next_one(tcd->arg_0.out());
}

CORBA::Boolean CosNaming::_objref_BindingIterator::next_one(Binding_out b)
{
  _0RL_cd_69ceca6a39f685b5_41000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_51000000, "next_one", 9);
  
  _invoke(_call_desc);
  b.ptr() = _call_desc.arg_0._retn();
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  _cboolean_i_cunsigned_plong_o_cCosNaming_mBindingList
class _0RL_cd_69ceca6a39f685b5_61000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_61000000(LocalCallFn lcfn, const char* op, size_t oplen, CORBA::ULong a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  inline CORBA::Boolean result() { return pd_result; }
  CORBA::ULong arg_0;
  CosNaming::BindingList_var arg_1;
  CORBA::Boolean pd_result;
};

void _0RL_cd_69ceca6a39f685b5_61000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  
}

void _0RL_cd_69ceca6a39f685b5_61000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  
}

void _0RL_cd_69ceca6a39f685b5_61000000::unmarshalReturnedValues(cdrStream& s)
{
  
  pd_result <<= s;
  arg_1 =  new CosNaming::BindingList;
  (CosNaming::BindingList&)arg_1 <<= s;
  
}

void _0RL_cd_69ceca6a39f685b5_61000000::marshalReturnedValues(cdrStream& s)
{
  
  pd_result >>= s;
  (CosNaming::BindingList&)arg_1 >>= s;
  
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_71000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_61000000* tcd = (_0RL_cd_69ceca6a39f685b5_61000000*) cd;
  CosNaming::_impl_BindingIterator* impl = (CosNaming::_impl_BindingIterator*) svnt->_ptrToInterface(CosNaming::BindingIterator::_PD_repoId);
  tcd->pd_result = impl->next_n(tcd->arg_0, tcd->arg_1.out());
}

CORBA::Boolean CosNaming::_objref_BindingIterator::next_n(CORBA::ULong how_many, BindingList_out bl)
{
  _0RL_cd_69ceca6a39f685b5_61000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_71000000, "next_n", 7, how_many);
  
  _invoke(_call_desc);
  bl.ptr() = _call_desc.arg_1._retn();
  return _call_desc.result();
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_81000000(omniCallDescriptor* cd, omniServant* svnt)
{
  CosNaming::_impl_BindingIterator* impl = (CosNaming::_impl_BindingIterator*) svnt->_ptrToInterface(CosNaming::BindingIterator::_PD_repoId);
  impl->destroy();
}

void CosNaming::_objref_BindingIterator::destroy()
{
  omniStdCallDesc::void_call _call_desc(_0RL_lcfn_69ceca6a39f685b5_81000000, "destroy", 8, 0, 0, 0, 0);
  
  _invoke(_call_desc);
  
}

CosNaming::_pof_BindingIterator::~_pof_BindingIterator() {}

omniObjRef*
CosNaming::_pof_BindingIterator::newObjRef(omniIOR* ior,
               omniIdentity* id, omniLocalIdentity* lid)
{
  return new CosNaming::_objref_BindingIterator(ior, id, lid);
}

CORBA::Boolean
CosNaming::_pof_BindingIterator::is_a(const char* id) const
{
  if( !strcmp(id, CosNaming::BindingIterator::_PD_repoId) )
    return 1;
  
  return 0;
}

const CosNaming::_pof_BindingIterator _the_pof_CosNaming_mBindingIterator;

CosNaming::_impl_BindingIterator::~_impl_BindingIterator() {}

CORBA::Boolean
CosNaming::_impl_BindingIterator::_dispatch(GIOP_S& giop_s)
{
  const char* op = giop_s.invokeInfo().operation();

  if( !strcmp(op, "next_one") ) {
    
    _0RL_cd_69ceca6a39f685b5_41000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_51000000, "next_one", 9, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "next_n") ) {

    _0RL_cd_69ceca6a39f685b5_61000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_71000000, "next_n", 7, 0, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "destroy") ) {
    
    omniStdCallDesc::void_call _call_desc(_0RL_lcfn_69ceca6a39f685b5_81000000, "destroy", 8, 0, 0, 0, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  return 0;
}

void*
CosNaming::_impl_BindingIterator::_ptrToInterface(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (void*) 1;
  if( !strcmp(id, CosNaming::BindingIterator::_PD_repoId) )
    return (_impl_BindingIterator*) this;
  
  return 0;
}

const char*
CosNaming::_impl_BindingIterator::_mostDerivedRepoId()
{
  return CosNaming::BindingIterator::_PD_repoId;
}

CORBA::Exception::insertExceptionToAny CosNaming::NamingContextExt::InvalidAddress::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP CosNaming::NamingContextExt::InvalidAddress::insertToAnyFnNCP = 0;

CosNaming::NamingContextExt::InvalidAddress::InvalidAddress(const CosNaming::NamingContextExt::InvalidAddress& _s) : CORBA::UserException(_s)
{
  
}

CosNaming::NamingContextExt::InvalidAddress& CosNaming::NamingContextExt::InvalidAddress::operator=(const CosNaming::NamingContextExt::InvalidAddress& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  
  return *this;
}

CosNaming::NamingContextExt::InvalidAddress::~InvalidAddress() {}

void CosNaming::NamingContextExt::InvalidAddress::_raise() { throw *this; }

CosNaming::NamingContextExt::InvalidAddress* CosNaming::NamingContextExt::InvalidAddress::_downcast(CORBA::Exception* _e) {
  return (InvalidAddress*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContextExt::InvalidAddress");
}

const CosNaming::NamingContextExt::InvalidAddress* CosNaming::NamingContextExt::InvalidAddress::_downcast(const CORBA::Exception* _e) {
  return (const InvalidAddress*) _NP_is_a(_e, "Exception/UserException/CosNaming::NamingContextExt::InvalidAddress");
}

const char* CosNaming::NamingContextExt::InvalidAddress::_PD_repoId = "IDL:omg.org/CosNaming/NamingContextExt/InvalidAddress:1.0";

CORBA::Exception* CosNaming::NamingContextExt::InvalidAddress::_NP_duplicate() const {
  return new InvalidAddress(*this);
}

const char* CosNaming::NamingContextExt::InvalidAddress::_NP_typeId() const {
  return "Exception/UserException/CosNaming::NamingContextExt::InvalidAddress";
}

const char* CosNaming::NamingContextExt::InvalidAddress::_NP_repoId(int* _size) const {
  *_size = sizeof("IDL:omg.org/CosNaming/NamingContextExt/InvalidAddress:1.0");
  return "IDL:omg.org/CosNaming/NamingContextExt/InvalidAddress:1.0";
}

void CosNaming::NamingContextExt::InvalidAddress::_NP_marshal(cdrStream& _s) const {
  *this >>= _s;
}

CosNaming::NamingContextExt_ptr CosNaming::NamingContextExt_Helper::_nil() {
  return CosNaming::NamingContextExt::_nil();
}

CORBA::Boolean CosNaming::NamingContextExt_Helper::is_nil(CosNaming::NamingContextExt_ptr p) {
  return CORBA::is_nil(p);

}

void CosNaming::NamingContextExt_Helper::release(CosNaming::NamingContextExt_ptr p) {
  CORBA::release(p);
}

void CosNaming::NamingContextExt_Helper::duplicate(CosNaming::NamingContextExt_ptr p) {
  if( p && !p->_NP_is_nil() )  omni::duplicateObjRef(p);
}

void CosNaming::NamingContextExt_Helper::marshalObjRef(CosNaming::NamingContextExt_ptr obj, cdrStream& s) {
  CosNaming::NamingContextExt::_marshalObjRef(obj, s);
}

CosNaming::NamingContextExt_ptr CosNaming::NamingContextExt_Helper::unmarshalObjRef(cdrStream& s) {
  return CosNaming::NamingContextExt::_unmarshalObjRef(s);
}

CosNaming::NamingContextExt_ptr
CosNaming::NamingContextExt::_duplicate(CosNaming::NamingContextExt_ptr obj)
{
  if( obj && !obj->_NP_is_nil() )  omni::duplicateObjRef(obj);

  return obj;
}

CosNaming::NamingContextExt_ptr
CosNaming::NamingContextExt::_narrow(CORBA::Object_ptr obj)
{
  if( !obj || obj->_NP_is_nil() || obj->_NP_is_pseudo() ) return _nil();
  _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
  return e ? e : _nil();
}

CosNaming::NamingContextExt_ptr
CosNaming::NamingContextExt::_nil()
{
  static _objref_NamingContextExt* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
  if( !_the_nil_ptr )  _the_nil_ptr = new _objref_NamingContextExt;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char* CosNaming::NamingContextExt::_PD_repoId = "IDL:omg.org/CosNaming/NamingContextExt:1.0";

CosNaming::_objref_NamingContextExt::~_objref_NamingContextExt() {}

CosNaming::_objref_NamingContextExt::_objref_NamingContextExt(omniIOR* ior, 
       omniIdentity* id, omniLocalIdentity* lid) :
   OMNIORB_BASE_CTOR(CosNaming::)_objref_NamingContext(ior, id, lid),
   
   omniObjRef(CosNaming::NamingContextExt::_PD_repoId, ior, id, lid)
{
  _PR_setobj(this);
}

void*
CosNaming::_objref_NamingContextExt::_ptrToObjRef(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(id, CosNaming::NamingContextExt::_PD_repoId) )
    return (CosNaming::NamingContextExt_ptr) this;
  if( !strcmp(id, CosNaming::NamingContext::_PD_repoId) )
    return (CosNaming::NamingContext_ptr) this;
  
  return 0;
}

// Proxy call descriptor class. Mangled signature:
//  _cstring_i_cCosNaming_mName_e_cCosNaming_mNamingContext_mInvalidName
class _0RL_cd_69ceca6a39f685b5_91000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_91000000(LocalCallFn lcfn, const char* op, size_t oplen, CosNaming::Name& a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 1, upcall ),
     arg_0(a_0) {}

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  void userException(GIOP_C&, const char*);
  
  inline char * result() { return pd_result._retn(); }
  CosNaming::Name& arg_0;
  CORBA::String_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_91000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0"
};

void _0RL_cd_69ceca6a39f685b5_91000000::marshalArguments(cdrStream& s)
{
  arg_0 >>= s;
  
}

void _0RL_cd_69ceca6a39f685b5_91000000::unmarshalArguments(cdrStream& s)
{
  arg_0 <<= s;
  
}

void _0RL_cd_69ceca6a39f685b5_91000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = _CORBA_String_helper::unmarshal(s);
}

void _0RL_cd_69ceca6a39f685b5_91000000::marshalReturnedValues(cdrStream& s)
{
  _CORBA_String_helper::marshal(pd_result,s);
}

void _0RL_cd_69ceca6a39f685b5_91000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_a1000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_91000000* tcd = (_0RL_cd_69ceca6a39f685b5_91000000*) cd;
  CosNaming::_impl_NamingContextExt* impl = (CosNaming::_impl_NamingContextExt*) svnt->_ptrToInterface(CosNaming::NamingContextExt::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->to_string(tcd->arg_0);
#else
  if (!is_upcall())
    tcd->pd_result = impl->to_string(tcd->arg_0);
  else {
    try {
      tcd->pd_result = impl->to_string(tcd->arg_0);
    }

    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif

}

char* CosNaming::_objref_NamingContextExt::to_string(const Name& n)
{
  _0RL_cd_69ceca6a39f685b5_91000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_a1000000, "to_string", 10, (Name&)n);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  _cCosNaming_mName_i_cstring_e_cCosNaming_mNamingContext_mInvalidName
class _0RL_cd_69ceca6a39f685b5_b1000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_b1000000(LocalCallFn lcfn, const char* op, size_t oplen, char* a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 1, upcall),
     arg_0(a_0) {}

  inline ~_0RL_cd_69ceca6a39f685b5_b1000000() {
    if (is_upcall()) { _CORBA_String_helper::free(arg_0); }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  void userException(GIOP_C&, const char*);
  
  inline CosNaming::Name* result() { return pd_result._retn(); }
  char* arg_0;
  CosNaming::Name_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_b1000000::_user_exns[] = {
  "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0"
};

void _0RL_cd_69ceca6a39f685b5_b1000000::marshalArguments(cdrStream& s)
{
  _CORBA_String_helper::marshal(arg_0,s);
}

void _0RL_cd_69ceca6a39f685b5_b1000000::unmarshalArguments(cdrStream& s)
{
  arg_0 = _CORBA_String_helper::unmarshal(s);
}

void _0RL_cd_69ceca6a39f685b5_b1000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = new CosNaming::Name;
  
  (CosNaming::Name&)pd_result <<= s;
}

void _0RL_cd_69ceca6a39f685b5_b1000000::marshalReturnedValues(cdrStream& s)
{
  (CosNaming::Name&)pd_result >>= s;
}

void _0RL_cd_69ceca6a39f685b5_b1000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_c1000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_b1000000* tcd = (_0RL_cd_69ceca6a39f685b5_b1000000*) cd;
  CosNaming::_impl_NamingContextExt* impl = (CosNaming::_impl_NamingContextExt*) svnt->_ptrToInterface(CosNaming::NamingContextExt::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->to_name(tcd->arg_0);
#else
  if (!is_upcall())
    tcd->pd_result = impl->to_name(tcd->arg_0);
  else {
    try {
      tcd->pd_result = impl->to_name(tcd->arg_0);
    }

    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

CosNaming::Name* CosNaming::_objref_NamingContextExt::to_name(const char* sn)
{
  _0RL_cd_69ceca6a39f685b5_b1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_c1000000, "to_name", 8, (char*)sn);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  _cstring_i_cstring_i_cstring_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContextExt_mInvalidAddress
class _0RL_cd_69ceca6a39f685b5_d1000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_d1000000(LocalCallFn lcfn, const char* op, size_t oplen, char* a_0, char* a_1, CORBA::Boolean upcall=0):
    omniCallDescriptor(lcfn, op, oplen, 0,  _user_exns, 2, upcall),
     arg_0(a_0),
     arg_1(a_1) {}

  inline ~_0RL_cd_69ceca6a39f685b5_d1000000() {
    if (is_upcall()) {
      _CORBA_String_helper::free(arg_0);
      _CORBA_String_helper::free(arg_1);
    }
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  virtual void userException(GIOP_C&, const char*);
  
  inline char * result() { return pd_result._retn(); }
  char* arg_0;
  char* arg_1;
  CORBA::String_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_d1000000::_user_exns[] = {
    "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
    "IDL:omg.org/CosNaming/NamingContextExt/InvalidAddress:1.0"
  };

void _0RL_cd_69ceca6a39f685b5_d1000000::marshalArguments(cdrStream& s)
{
  _CORBA_String_helper::marshal(arg_0,s);
  _CORBA_String_helper::marshal(arg_1,s);
}

void _0RL_cd_69ceca6a39f685b5_d1000000::unmarshalArguments(cdrStream& s)
{
  arg_0 = _CORBA_String_helper::unmarshal(s);
  arg_1 = _CORBA_String_helper::unmarshal(s);
}

void _0RL_cd_69ceca6a39f685b5_d1000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = _CORBA_String_helper::unmarshal(s);
}

void _0RL_cd_69ceca6a39f685b5_d1000000::marshalReturnedValues(cdrStream& s)
{
  _CORBA_String_helper::marshal(pd_result,s);
}

void _0RL_cd_69ceca6a39f685b5_d1000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContextExt::InvalidAddress::_PD_repoId) == 0 ) {
    CosNaming::NamingContextExt::InvalidAddress _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_e1000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_d1000000* tcd = (_0RL_cd_69ceca6a39f685b5_d1000000*) cd;
  CosNaming::_impl_NamingContextExt* impl = (CosNaming::_impl_NamingContextExt*) svnt->_ptrToInterface(CosNaming::NamingContextExt::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->to_url(tcd->arg_0, tcd->arg_1);
#else
  if (!is_upcall())
    tcd->pd_result = impl->to_url(tcd->arg_0, tcd->arg_1);
  else {
    try {
      tcd->pd_result = impl->to_url(tcd->arg_0, tcd->arg_1);
    }
    
    catch(CosNaming::NamingContextExt::InvalidAddress& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

char* CosNaming::_objref_NamingContextExt::to_url(const char* addr, const char* sn)
{
  _0RL_cd_69ceca6a39f685b5_d1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_e1000000, "to_url", 7, (char*)addr, (char*)sn);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

// Proxy call descriptor class. Mangled signature:
//  _cCORBA_mObject_i_cstring_e_cCosNaming_mNamingContext_mAlreadyBound_e_cCosNaming_mNamingContext_mCannotProceed_e_cCosNaming_mNamingContext_mInvalidName_e_cCosNaming_mNamingContext_mNotFound
class _0RL_cd_69ceca6a39f685b5_f1000000
  : public omniCallDescriptor
{
public:
  inline _0RL_cd_69ceca6a39f685b5_f1000000(LocalCallFn lcfn, const char* op, size_t oplen, char* a_0, CORBA::Boolean upcall=0):
     omniCallDescriptor(lcfn, op, oplen, 0, _user_exns, 4, upcall),
     arg_0(a_0) {}

  inline ~_0RL_cd_69ceca6a39f685b5_f1000000() {
    if (is_upcall()) _CORBA_String_helper::free(arg_0);
  }

  void marshalArguments(cdrStream&);
  void unmarshalArguments(cdrStream&);
  
  void unmarshalReturnedValues(cdrStream&);
  void marshalReturnedValues(cdrStream&);
    
  void userException(GIOP_C&, const char*);
  
  inline CORBA::Object_ptr result() { return pd_result._retn(); }
  char* arg_0;
  CORBA::Object_var pd_result;

  static const char* const _user_exns[];
};

const char* const _0RL_cd_69ceca6a39f685b5_f1000000::_user_exns[] = {
    "IDL:omg.org/CosNaming/NamingContext/AlreadyBound:1.0",
    "IDL:omg.org/CosNaming/NamingContext/CannotProceed:1.0",
    "IDL:omg.org/CosNaming/NamingContext/InvalidName:1.0",
    "IDL:omg.org/CosNaming/NamingContext/NotFound:1.0"
  };

void _0RL_cd_69ceca6a39f685b5_f1000000::marshalArguments(cdrStream& s)
{
  _CORBA_String_helper::marshal(arg_0,s);
}

void _0RL_cd_69ceca6a39f685b5_f1000000::unmarshalArguments(cdrStream& s)
{
  arg_0 = _CORBA_String_helper::unmarshal(s);
}

void _0RL_cd_69ceca6a39f685b5_f1000000::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CORBA::Object::_unmarshalObjRef(s);
}

void _0RL_cd_69ceca6a39f685b5_f1000000::marshalReturnedValues(cdrStream& s)
{
  CORBA::Object::_marshalObjRef(pd_result,s);
}

void _0RL_cd_69ceca6a39f685b5_f1000000::userException(GIOP_C& giop_client, const char* repoId)
{
  cdrStream& s = (cdrStream&) giop_client;
  if( strcmp(repoId, CosNaming::NamingContext::NotFound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::NotFound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::CannotProceed::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::CannotProceed _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::InvalidName::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::InvalidName _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else if( strcmp(repoId, CosNaming::NamingContext::AlreadyBound::_PD_repoId) == 0 ) {
    CosNaming::NamingContext::AlreadyBound _ex;
    _ex <<= s;
    giop_client.RequestCompleted();
    throw _ex;
  }
  
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}

// Local call call-back function.
static void
_0RL_lcfn_69ceca6a39f685b5_02000000(omniCallDescriptor* cd, omniServant* svnt)
{
  _0RL_cd_69ceca6a39f685b5_f1000000* tcd = (_0RL_cd_69ceca6a39f685b5_f1000000*) cd;
  CosNaming::_impl_NamingContextExt* impl = (CosNaming::_impl_NamingContextExt*) svnt->_ptrToInterface(CosNaming::NamingContextExt::_PD_repoId);

#ifdef HAS_Cplusplus_catch_exception_by_base
  tcd->pd_result = impl->resolve_str(tcd->arg_0);
#else
  if (!is_upcall())
    tcd->pd_result = impl->resolve_str(tcd->arg_0);
  else {
    try {
      tcd->pd_result = impl->resolve_str(tcd->arg_0);
    }

    catch(CosNaming::NamingContext::NotFound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::CannotProceed& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::InvalidName& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
    
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      throw omniORB::StubUserException(ex._NP_duplicate());
    }
  }
#endif
}

CORBA::Object_ptr CosNaming::_objref_NamingContextExt::resolve_str(const char* n)
{
  _0RL_cd_69ceca6a39f685b5_f1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_02000000, "resolve_str", 12, (char*)n);
  
  _invoke(_call_desc);
  return _call_desc.result();
}

CosNaming::_pof_NamingContextExt::~_pof_NamingContextExt() {}

omniObjRef*
CosNaming::_pof_NamingContextExt::newObjRef(omniIOR* ior,
               omniIdentity* id, omniLocalIdentity* lid)
{
  return new CosNaming::_objref_NamingContextExt(ior, id, lid);
}

CORBA::Boolean
CosNaming::_pof_NamingContextExt::is_a(const char* id) const
{
  if( !strcmp(id, CosNaming::NamingContextExt::_PD_repoId) )
    return 1;
  if( !strcmp(id, CosNaming::NamingContext::_PD_repoId) )
    return 1;
  
  return 0;
}

const CosNaming::_pof_NamingContextExt _the_pof_CosNaming_mNamingContextExt;

CosNaming::_impl_NamingContextExt::~_impl_NamingContextExt() {}

CORBA::Boolean
CosNaming::_impl_NamingContextExt::_dispatch(GIOP_S& giop_s)
{
  const char* op = giop_s.invokeInfo().operation();

  if( !strcmp(op, "to_string") ) {

    Name arg_n;
    _0RL_cd_69ceca6a39f685b5_91000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_a1000000, "to_string", 10, arg_n, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "to_name") ) {
    _0RL_cd_69ceca6a39f685b5_b1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_c1000000, "to_name", 8, 0, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "to_url") ) {
    _0RL_cd_69ceca6a39f685b5_d1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_e1000000, "to_url", 7, 0, 0);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( !strcmp(op, "resolve_str") ) {
    _0RL_cd_69ceca6a39f685b5_f1000000 _call_desc(_0RL_lcfn_69ceca6a39f685b5_02000000, "resolve_str", 12, 0, 1);
    _upcall(giop_s,_call_desc);
    return 1;
  }

  if( _impl_NamingContext::_dispatch(giop_s) ) {
    return 1;
  }

  return 0;
}

void*
CosNaming::_impl_NamingContextExt::_ptrToInterface(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (void*) 1;
  if( !strcmp(id, CosNaming::NamingContextExt::_PD_repoId) )
    return (_impl_NamingContextExt*) this;
  if( !strcmp(id, NamingContext::_PD_repoId) )
    return (_impl_NamingContext*) this;

  return 0;
}

const char*
CosNaming::_impl_NamingContextExt::_mostDerivedRepoId()
{
  return CosNaming::NamingContextExt::_PD_repoId;
}

POA_CosNaming::NamingContext::~NamingContext() {}

POA_CosNaming::BindingIterator::~BindingIterator() {}

POA_CosNaming::NamingContextExt::~NamingContextExt() {}

