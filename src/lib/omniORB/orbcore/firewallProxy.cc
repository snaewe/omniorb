// -*- Mode: C++; -*-
//                            Package   : omniORB2
// firewallProxy.cc           Created on: 28/9/99
//                            Author    : Sai Lai Lo (sll)
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

/* $Log$
/* Revision 1.1.4.1  2000/09/27 17:30:28  sll
/* *** empty log message ***
/*
/* Revision 1.1.2.3  1999/11/04 20:20:18  sll
/* GIOP engines can now do callback to the higher layer to calculate total
/* message size if necessary.
/* Where applicable, changed to use the new server side descriptor-based stub.
/*
/* Revision 1.1.2.2  1999/10/05 20:36:32  sll
/* Added option -ORBgiopTargetAddressMode <0|1|2> to control the
/* TargetAddress mode used when invoking on a remote object using GIOP 1.2
/*
/* Revision 1.1.2.1  1999/10/02 18:21:27  sll
/* Added support to decode optional tagged components in the IIOP profile.
/* Added support to negogiate with a firewall proxy- GIOPProxy to invoke
/* remote objects inside a firewall.
/* Added tagged component TAG_ORB_TYPE to identify omniORB IORs.
/* */

#include <omniORB2/CORBA.h>
#include <omniORB2/proxyCall.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <firewallProxy.h>
#include <objectManager.h>
#include <omniORB2/Firewall.hh>

//////////////////////////////////////////////////////////////////////

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " firewallProxy " ## prefix ## ": " ## message ## "\n";\
   }\
} while (0)

#define PTRACE(prefix,message) LOGMESSAGE(15,prefix,message)

//////////////////////////////////////////////////////////////////////
class firewallProxyRedirect: public virtual omniObject, 
			     public virtual CORBA::Object {

public:
  firewallProxyRedirect(GIOPObjectInfo* g,Firewall::GIOPProxy_ptr fw) 
    : objectInfo_(g), fwProxy_(fw) {
    this->PR_setobj(this);
    CORBA::BOA::getBOA()->obj_is_ready(this);
  }

  void _dispose() {
    CORBA::BOA::getBOA()->dispose(this);
  }

  omniORB::objectKey _key() {
    omniORB::objectKey result;
    getKey(result);
    return result;
  }


  class returnValueMarshaller : public giopMarshaller {
  public:
    returnValueMarshaller(giopStream& s,CORBA::Object_var& v) : 
      pd_s(s), pd_v(v) {}

    void marshalData() {
      CORBA::Object::marshalObjRef(pd_v, (cdrStream&)pd_s);
    }

    size_t dataSize(size_t initialoffset) {
      cdrCountingStream s(initialoffset);
      CORBA::Object::marshalObjRef(pd_v, s);      
      return s.total();
    }

  private:
    giopStream&     pd_s;
    CORBA::Object_var& pd_v;
  };

  CORBA::Boolean dispatch(GIOP_S &giop_s, const char *,
			  CORBA::Boolean response_expected) {

    cdrStream& s = (cdrStream&)giop_s;
    giop_s.RequestReceived(1);

    PTRACE("Redirect","Negogiate GIOP Proxy");

    if (!response_expected) return 1;

    CORBA::Object_var newDestination;

    newDestination = getTargetFromGIOPProxy();

    PTRACE("Redirect","Got target from GIOP Proxy");

    returnValueMarshaller m((giopStream&)giop_s,newDestination);

    giop_s.InitialiseReply(GIOP::LOCATION_FORWARD,m);
    giop_s.ReplyCompleted(); 

    return 1;
  }


private:

  CORBA::Object_ptr getTargetFromGIOPProxy();

  GIOPObjectInfo*         objectInfo_;
  Firewall::GIOPProxy_var fwProxy_;
};

//////////////////////////////////////////////////////////////////////
static
void firewallProxyRedirect_destructor(void* p)
{
  ((firewallProxyRedirect*)p)->_dispose();
}

//////////////////////////////////////////////////////////////////////
Rope*
firewallProxyRopeFactory::findOrCreateOutgoing(Endpoint* addr,
					       GIOPObjectInfo* g)
{
  if (omniORB::noFirewallNavigation || !g || !g->tag_components_) return 0;

  // Determine if calls must go through the GIOP proxy
  // XXX Not done yet

  // If we have an outgoing Firewall proxy, we do it here.
  // XXX Not done yet

  // Locate and decode TAG_FIREWALL_TRANS profile
  CORBA::ULong index;
  IOP::MultipleComponentProfile& p = *(g->tag_components_);
  for (index = 0; index < p.length(); index++) {
    if (p[index].tag == IOP::TAG_FIREWALL_TRANS) break;
  }

  if (index == p.length()) return 0;

  cdrEncapsulationStream s(p[index].component_data.get_buffer(),
			   p[index].component_data.length(),1);

  Firewall::MultipleFirewallMechanisms m;
  m <<= s;

  if (m.length() == 0) return 0;

  // extract the info of the incoming Firewall proxy.

  if (m[0].tag == Firewall::FW_MECH_TCP || 
      m[0].tag == Firewall::FW_MECH_SOCKSV5) {

    if (omniORB::trace(1)) {
      omniORB::logger log("omniORB: ");
      log << "firewallProxyRopeFactory: do not know how to negotiate"
          << " with a TCP or Socks Firewall proxy yet.\n";
    }
    return 0;
  }
       
  if (m[0].tag == Firewall::FW_MECH_PROXY) {

    cdrEncapsulationStream ms(m[0].profile_data.get_buffer(),
			      m[0].profile_data.length(),1);

    Firewall::GIOPProxy_var proxy = Firewall::GIOPProxy::unmarshalObjRef(ms);

    if (CORBA::is_nil(proxy)) {

      if (omniORB::trace(1)) {
	omniORB::logger log("omniORB: ");
	log << "firewallProxyRopeFactory: error GIOPProxy is nil,"
	    << " ignore firewall proxy\n";
      }
      return 0;
    }

#if 1 // For testing only
    if (omniORB::giopTargetAddressMode == GIOP::ReferenceAddr) {
      CORBA::Boolean fwd;
      GIOPObjectInfo_var pg = proxy->getInvokeInfo(fwd);
      Rope* nr = pg->rope();
      nr->incrRefCount();
      g->rope_ = nr;
      return nr;
    }
#endif

    // Create a local redirect object to do the negotiation with
    // the GIOP proxy when the application invokes on this target.
    omni::internal_init_BOA();  // Ensure that the BOA has started.
    firewallProxyRedirect* rd = new firewallProxyRedirect(g,proxy);

    (void) proxy._retn();

    if (!g->opaque_data_) {
      g->opaque_data_ = new GIOPObjectInfo::opaque_sequence();
    }
    CORBA::ULong index = g->opaque_data_->length();
    g->opaque_data_->length(index+1);
    (*(g->opaque_data_))[index].data = (void*) rd;
    (*(g->opaque_data_))[index].destructor = firewallProxyRedirect_destructor;

    // Message the GIOPObjectInfo so that it will invoke on the local
    // redirect object instead of going off to the target.
    //
    omniORB::objectKey k = rd->_key();
    g->object_key_.length(sizeof(omniORB::objectKey));
    memcpy((void*)g->object_key_.get_buffer(),(void*)&k,
	   sizeof(omniORB::objectKey));

    Rope* lb = rd->_objectManager()->defaultLoopBack();
    lb->incrRefCount();
    g->rope_ = lb;

    return lb;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////
class firewallProxyNewTargetCallDesc : public OmniProxyCallDesc {

public:
  firewallProxyNewTargetCallDesc(GIOPObjectInfo* g, Firewall::ProxyMode m)
    : OmniProxyCallDesc("new_target", 11), 
      objectInfo(g), arg_mode(m), pd_result(0) {}

  void marshalArguments(cdrStream& s) {
    // XXX Bad! we couldn't use cdrStream::marshalObjRef() here because we
    //     already have the GIOPObjectInfo
    const char* repoId = objectInfo->repositoryID();
    size_t repoIdSize = strlen(repoId)+1;
    ::operator>>= ((CORBA::ULong) repoIdSize,s);
    s.put_char_array((CORBA::Char*) repoId, repoIdSize);
    const IOP::TaggedProfileList * pl = objectInfo->iopProfiles();
    *pl >>= s;
    arg_mode >>= s;
  }
  void unmarshalReturnedValues(cdrStream& s) {
    pd_result = CORBA::Object_Helper::unmarshalObjRef(s);
  }
  inline CORBA::Object_ptr result() { return pd_result; }

private:
  GIOPObjectInfo*     objectInfo;
  Firewall::ProxyMode arg_mode;
  CORBA::Object_ptr pd_result;
};

//////////////////////////////////////////////////////////////////////
CORBA::Object_ptr
firewallProxyRedirect::getTargetFromGIOPProxy()
{
  // What decides whether to use Normal or PASSTHRU mode??
  // XXX always use Normal
  firewallProxyNewTargetCallDesc call_desc(objectInfo_,Firewall::NORMAL);

  OmniProxyCallWrapper::invoke(fwProxy_,call_desc);
  return call_desc.result();
}


//////////////////////////////////////////////////////////////////////
static firewallProxyRopeFactory* singleton_ = 0;

firewallProxyRopeFactory*
firewallProxyRopeFactory::singleton()
{
  if (!singleton_) singleton_ = new firewallProxyRopeFactory();
  return singleton_;
}

//////////////////////////////////////////////////////////////////////
class firewallProxyRopeFactory_initOnce {
public:
  firewallProxyRopeFactory_initOnce() {
    (void) firewallProxyRopeFactory::singleton();
  }
};

static firewallProxyRopeFactory_initOnce initOnce;
