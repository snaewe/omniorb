#include <omniORB2/Firewall.hh>
#include <omniORB2/proxyCall.h>
#include <iostream.h>

// TO DO:
//    The Relay_i objects recreated by new_target are never cleaned up!
//    To do this properly, these objects should be deleted once the
//    target is known to have been deleted.

CORBA::ORB_ptr orb;
CORBA::BOA_ptr boa;

/////////////////////////////////////////////////////////////////////////
class Relay_i : public virtual omniObject,
                public virtual CORBA::Object {
public:
  Relay_i(CORBA::Object_ptr p) : target(CORBA::Object::_duplicate(p)) {
    const char* repoid = p->PR_getobj()->NP_IRRepositoryId();
    {
      omniORB::logger log("Relay_i: ");
      log << " create proxy for target object type: "
	  << repoid << "\n";
    }
    PR_setRepositoryID(repoid);
    this->PR_setobj(this);
    boa->obj_is_ready(this);
  }

  CORBA::Object_ptr _this() {
    return CORBA::Object::_duplicate(this);
  }

  CORBA::Boolean dispatch(GIOP_S &giop_s, const char *op,
			  CORBA::Boolean response_expected);

private:
  CORBA::Object_var target;
};

//////////////////////////////////////////////////////////////////////////
class relayCallDesc : public OmniProxyCallDesc
{
public:
  inline relayCallDesc(const char* _op, 
		       size_t _op_len, 
		       GIOP_S& giop_s) :
    OmniProxyCallDesc(_op, _op_len, 1), giop_s_(giop_s) {}

  void initialise(cdrStream&);
  void marshalArguments(cdrStream&);
  void unmarshalReturnedValues(cdrStream&);
  void userException(GIOP_C& giop_client, const char* repoId);

private:
  GIOP_S& giop_s_;
};


//////////////////////////////////////////////////////////////////////////
CORBA::Boolean
Relay_i::dispatch(GIOP_S &giop_s, const char *op,
		  CORBA::Boolean response_expected) 
{
  giopStream& s = giop_s;
  if (!s.startSavingInputMessageBody()) {
    {
      omniORB::logger log("Relay_i: ");
      log << "GIOP version does not support message relay. Cannot forward this message\n";
    }
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_NO);
  }

  relayCallDesc call_desc(op,strlen(op)+1,giop_s);
  OmniProxyCallWrapper::invoke(target->PR_getobj(),call_desc);
  return 1;
}

//////////////////////////////////////////////////////////////////////////
class Proxy_i {
public:
  CORBA::Object_ptr new_target(CORBA::Object_ptr target, Firewall::ProxyMode) {
    // Only do NORMAL, no PASSTHRU
    {
      omniORB::logger log("Proxy_i: ");
      log << "create new Relay_i\n";
    }
    Relay_i* result = new Relay_i(target);
    {
      omniORB::logger log("Proxy_i: ");
      log << "return Relay_i\n";
    }
    return result->_this();
  }
  CORBA::Object_ptr new_callback(CORBA::Object_ptr callback) {
    // XXX not implemented yet.
    return CORBA::Object::_nil();
  }
};


//////////////////////////////////////////////////////////////////////////
int
main(int argc, char** argv)
{
  omniORB::noFirewallNavigation = 1;

  orb = CORBA::ORB_init(argc,argv,"omniORB2");
  
  char* boa_option[] = { "giopProxy", "-BOAiiop_port", "9000" };
  int   boa_argc     = 3;
  boa = orb->BOA_init(boa_argc,boa_option,"omniORB2_BOA");

  omniObjectKey key;
  key.hi = key.med = key.lo = 0x00545441;
  Proxy_i* myimpl = new Proxy_i();
  _tie_Firewall_GIOPProxy<Proxy_i,1> *myobj = new _tie_Firewall_GIOPProxy<Proxy_i,1>(myimpl,key);
  myobj->_obj_is_ready(boa);
  
  {
    Firewall::GIOPProxy_var myobjRef = myobj->_this();
    CORBA::String_var p = orb->object_to_string(myobjRef);
    cerr << "'" << (char*)p << "'" << endl;
  }

  boa->impl_is_ready();
  // Never reach here so the following is really redundent.
  boa->destroy();
  orb->NP_destroy();
  return 0;
}

//////////////////////////////////////////////////////////////////////////
void
relayCallDesc::initialise(cdrStream& giop_client)
{
  giopStream& s = (giopStream&)giop_client;
  giopStream& t = giop_s_;
  if (!s.prepareCopyMessageBodyFrom(t)) {
    {
      omniORB::logger log("relayCallDesc: ");
      log << "GIOP version does not support message relay. Cannot forward this message\n";
    }
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_NO);
  }
}

//////////////////////////////////////////////////////////////////////////
void
relayCallDesc::marshalArguments(cdrStream& giop_client)
{
  giopStream& s1 = (giopStream&)giop_client;
  giopStream& s2 = giop_s_;

  s1.copyMessageBodyFrom(s2);
  giop_s_.RequestReceived();
}

//////////////////////////////////////////////////////////////////////////
void
relayCallDesc::unmarshalReturnedValues(cdrStream& giop_client)
{
  giopStream& s1 = (giopStream&) giop_client;
  giopStream& s2 = giop_s_;

  s1.startSavingInputMessageBody();
  s2.prepareCopyMessageBodyFrom(s1);
  giop_s_.InitialiseReply(GIOP::NO_EXCEPTION);
  s2.copyMessageBodyFrom(s1);
  giop_s_.ReplyCompleted();
}

//////////////////////////////////////////////////////////////////////////
void
relayCallDesc::userException(GIOP_C& giop_client, const char* repoId)
{
  giopStream& s1 = (giopStream&) giop_client;
  giopStream& s2 = giop_s_;

  s1.startSavingInputMessageBody();
  s2.prepareCopyMessageBodyFrom(s1);
  giop_s_.InitialiseReply(GIOP::USER_EXCEPTION);
  CORBA::ULong repoIdLen = strlen(repoId) + 1;
  repoIdLen <<= s2;
  s2.put_char_array((CORBA::Char*)repoId,repoIdLen);
  s2.copyMessageBodyFrom(s1);
  giop_s_.ReplyCompleted();
}


