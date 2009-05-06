// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopBiDir.cc               Created on: 17/7/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2009 Apasphere Ltd
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
  Revision 1.1.4.11  2009/05/06 16:15:19  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.10  2009/02/25 11:56:16  dgrisby
  Correct log message.

  Revision 1.1.4.9  2006/06/22 13:53:49  dgrisby
  Add flags to strand.

  Revision 1.1.4.8  2006/06/05 11:28:04  dgrisby
  Change clientSendRequest interceptor members to a single GIOP_C.

  Revision 1.1.4.7  2006/06/02 12:48:32  dgrisby
  Small code cleanups.

  Revision 1.1.4.6  2006/04/09 19:52:31  dgrisby
  More IPv6, endPointPublish parameter.

  Revision 1.1.4.5  2006/01/10 13:59:37  dgrisby
  New clientConnectTimeOutPeriod configuration parameter.

  Revision 1.1.4.4  2005/03/02 13:33:42  dgrisby
  Variable name clash.

  Revision 1.1.4.3  2005/03/02 12:39:18  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.2  2005/01/06 23:10:15  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:02:17  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.15  2003/02/03 16:53:14  dgrisby
  Force type in constructor argument to help confused compilers.

  Revision 1.1.2.14  2002/11/26 14:51:50  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.13  2002/08/23 14:15:02  dgrisby
  Avoid exception with bidir when no POA.

  Revision 1.1.2.12  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.11  2002/01/16 11:31:59  dpg1
  Race condition in use of registerNilCorbaObject/registerTrackedObject.
  (Reported by Teemu Torma).

  Revision 1.1.2.10  2001/11/08 16:31:19  dpg1
  Minor tweaks.

  Revision 1.1.2.9  2001/09/19 17:26:49  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.2.8  2001/09/10 17:45:22  sll
  Call stopIdleCounter when a strand is switched to bidirectional in
  getBiDirServiceContext.

  Revision 1.1.2.7  2001/09/03 17:32:05  sll
  Make sure that acquireClient honours the deadline set in the calldescriptor.

  Revision 1.1.2.6  2001/08/29 17:52:03  sll
  Consult serverTransportRule to decide whether to accept the switch to
  bidirectional on the server side.

  Revision 1.1.2.5  2001/08/23 10:11:53  sll
  Initialise BiDirPolicy constants properly for compilers with no namespace
  support.

  Revision 1.1.2.4  2001/08/21 11:02:14  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.3  2001/08/17 17:12:36  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.2  2001/07/31 17:31:40  sll
  strchr returns const char*.

  Revision 1.1.2.1  2001/07/31 16:10:38  sll
  Added GIOP BiDir support.

  */

#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/objTracker.h>
#include <omniORB4/callDescriptor.h>
#include <exceptiondefs.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <giopBiDir.h>
#include <giopStreamImpl.h>
#include <giopStream.h>
#include <GIOP_C.h>
#include <GIOP_S.h>
#include <giopServer.h>
#include <initialiser.h>
#include <objectAdapter.h>
#include <tcp/tcpTransportImpl.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <transportRules.h>

OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::acceptBiDirectionalGIOP = 0;
//   Applies to the server side. Set to 1 to indicates that the
//   ORB may choose to accept a clients offer to use bidirectional
//   GIOP calls on a connection. Set to 0 means the ORB should
//   never accept any bidirectional offer and should stick to normal
//   GIOP.
//
//   Valid values = 0 or 1

CORBA::Boolean orbParameters::offerBiDirectionalGIOP = 0;
//   Applies to the client side. Set to 1 to indicates that the
//   ORB may choose to use a connection to do bidirectional GIOP
//   calls. Set to 0 means the ORB should never do bidirectional.
//
//   Valid values = 0 or 1

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace BiDirPolicy {

_init_in_def_( const CORBA::PolicyType
	       BIDIRECTIONAL_POLICY_TYPE = 37; )

_init_in_def_( const BidirectionalPolicyValue 
               NORMAL = 0; )

_init_in_def_( const BidirectionalPolicyValue 
               BOTH = 1; )
}
#else
_init_in_def_( const CORBA::PolicyType
	       BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE = 37; )

_init_in_def_( const BiDirPolicy::BidirectionalPolicyValue 
               BiDirPolicy::NORMAL = 0; )

_init_in_def_( const BiDirPolicy::BidirectionalPolicyValue 
               BiDirPolicy::BOTH = 1; )
#endif

BiDirPolicy::BidirectionalPolicy::~BidirectionalPolicy() {}

CORBA::Policy_ptr
BiDirPolicy::BidirectionalPolicy::copy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();
  return new BidirectionalPolicy(pd_value);
}

void*
BiDirPolicy::BidirectionalPolicy::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId );

  if( omni::ptrStrMatch(repoId, BiDirPolicy::BidirectionalPolicy::_PD_repoId) )
    return (BiDirPolicy::BidirectionalPolicy_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId) )
    return (CORBA::Policy_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_duplicate(BiDirPolicy::BidirectionalPolicy_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) )  return _nil();

  BidirectionalPolicy_ptr p = (BidirectionalPolicy_ptr) obj->_ptrToObjRef(BidirectionalPolicy::_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_nil()
{
  static BidirectionalPolicy* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new BidirectionalPolicy;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char*
BiDirPolicy::BidirectionalPolicy::_PD_repoId = "IDL:omg.org/BiDirPolicy/BidirectionalPolicy:1.0";

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
typedef _CORBA_Unbounded_Sequence<IIOP::Address> ListenPointList;

////////////////////////////////////////////////////////////////////////
RopeLink BiDirServerRope::ropes;

////////////////////////////////////////////////////////////////////////
BiDirServerRope::BiDirServerRope(giopStrand* strand, giopAddress* addr) : 
  giopRope(addr,0), 
  pd_sendfrom((const char*)strand->connection->peeraddress()) 
{
  pd_maxStrands = 1;
  pd_oneCallPerConnection = 0;
  strand->RopeLink::insert(pd_strands);
}

////////////////////////////////////////////////////////////////////////
BiDirServerRope::~BiDirServerRope()
{
  giopAddressList::const_iterator i, last;
  i    = pd_redirect_addresses.begin();
  last = pd_redirect_addresses.end();
  for (; i != last; i++) {
    delete (*i);
  }
}


////////////////////////////////////////////////////////////////////////
BiDirServerRope*
BiDirServerRope::addRope(giopStrand* strand, const giopAddressList& addrlist) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!strand->isClient() && strand->biDir == 1);

  const char* sendfrom = strand->connection->peeraddress();

  BiDirServerRope* br = 0;

  // Check if there already exists a rope for the strand.
  RopeLink* p = BiDirServerRope::ropes.next;
  while ( p != &BiDirServerRope::ropes ) {
    br = (BiDirServerRope*)p;
    if (strcmp(sendfrom,br->pd_sendfrom) == 0) {
      break;
    }
    else if (br->pd_refcount == 0 &&
	     RopeLink::is_empty(br->pd_strands) &&
	     !br->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      br->RopeLink::remove();
      delete br;
    }
    else {
      p = p->next;
    }
    br = 0;
  }

  if (!br) {
    giopAddress* addr = giopAddress::str2Address(strand->connection->peeraddress());
    br = new BiDirServerRope(strand,addr);
    br->RopeLink::insert(BiDirServerRope::ropes);
  }

  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    CORBA::Boolean matched = 0;
    giopAddressList::const_iterator j, k;
    j = br->pd_redirect_addresses.begin();
    k = br->pd_redirect_addresses.end();
    for (; j != k; j++) {
      if (omni::ptrStrMatch((*i)->address(),(*j)->address())) {
	matched = 1;
	break;
      }
    }
    if (!matched) {
      giopAddress* a = (*i)->duplicate();
      br->pd_redirect_addresses.push_back(a);
    }
  }

  return br;
}

////////////////////////////////////////////////////////////////////////
int
BiDirServerRope::selectRope(const giopAddressList& addrlist,
			    omniIOR::IORInfo* info,
			    Rope*& r) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  const char* sendfrom = 0;

  omniIOR::IORExtraInfoList& cinfo = info->extraInfo();
  for (CORBA::ULong index = 0; index < cinfo.length(); index++) {
    if (cinfo[index]->compid == IOP::TAG_OMNIORB_BIDIR) {
      sendfrom = ((BiDirInfo*)cinfo[index])->sendfrom;
      break;
    }
  }

  if (!sendfrom) return 0;

  GIOP::Version v = info->version();
  // Only GIOP 1.2 IORs can do bidirectional calls.
  if (v.major != 1 || v.minor < 2) return 0;


  BiDirServerRope* br;

  RopeLink* p = BiDirServerRope::ropes.next;
  while ( p != &BiDirServerRope::ropes ) {
    br = (BiDirServerRope*)p;
    if (br->match(sendfrom,addrlist)) {
      br->realIncrRefCount();
      r = (Rope*)br;
      return 1;
    }
    else if (br->pd_refcount == 0 &&
	     RopeLink::is_empty(br->pd_strands) &&
	     !br->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      br->RopeLink::remove();
      delete br;
    }
    else {
      p = p->next;
    }
  }

  // Reach here because we cannot find a match.
  return 0;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
BiDirServerRope::match(const char* sendfrom,
		       const giopAddressList& addrlist) const {

  if (!omni::strMatch(pd_sendfrom,sendfrom)) return 0;

  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    giopAddressList::const_iterator j, k;
    j = pd_redirect_addresses.begin();
    k = pd_redirect_addresses.end();
    for (; j != k; j++) {
      if (omni::ptrStrMatch((*i)->address(),(*j)->address())) return 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
IOP_C*
BiDirServerRope::acquireClient(const omniIOR* ior,
			       const CORBA::Octet* key,
			       CORBA::ULong keysize,
			       omniCallDescriptor* calldesc) {

  GIOP::Version v = ior->getIORInfo()->version();
  giopStreamImpl* impl = giopStreamImpl::matchVersion(v);
  if (!impl) {
    impl = giopStreamImpl::maxVersion();
    v = impl->version();
  }

  // Only use this connection to do calls with GIOP 1.2 or above.
  OMNIORB_ASSERT(v.major > 1 || v.minor >= 2);

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  OMNIORB_ASSERT(!pd_oneCallPerConnection && pd_maxStrands == 1);

  giopStrand* s = 0;
  if (pd_strands.next != &pd_strands) {
    s = (giopStrand*)(pd_strands.next);
  }

  if (!s ||  s->state() == giopStrand::DYING) {
    // We no longer have a working bidirectional connection.
    OMNIORB_THROW(TRANSIENT,TRANSIENT_BiDirConnIsGone,CORBA::COMPLETED_NO);
  }

  OMNIORB_ASSERT(s->state() == giopStrand::ACTIVE);

  // We do not check what GIOP version(s) the strand has been used for
  // previously.  If ever we have 2 calls using 2 different versions
  // (and both are 1.2 or above), we allow this to happen. Contrast
  // this with the algorithm in giopRope::acquireClient.

  GIOP_C* g;
  if (!giopStreamList::is_empty(s->clients)) {
    giopStreamList* gp = s->clients.next;
    for (; gp != &s->clients; gp = gp->next) {
      g = (GIOP_C*)gp;
      if (g->state() == IOP_C::UnUsed) {
	g->impl(impl);
	g->initialise(ior,key,keysize,calldesc);
	return g;
      }
    }
  }
  g = new GIOP_C(this,s);
  g->impl(impl);
  g->initialise(ior,key,keysize,calldesc);
  g->giopStreamList::insert(s->clients);
  return g;
}

////////////////////////////////////////////////////////////////////////
void
BiDirServerRope::decrRefCount() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);
  pd_refcount--;
  OMNIORB_ASSERT(pd_refcount >=0);

  if (pd_refcount) return;

  // Normally pd_strands always contain 1 strand unless the bidirectional
  // strand has been shutdown.
  if (RopeLink::is_empty(pd_strands) && !pd_nwaiting) {
    RopeLink::remove();
    delete this;
  }
  else {
    RopeLink* p = pd_strands.next;
    for (; p != &pd_strands; p = p->next) {
      giopStrand* g = (giopStrand*)p;
      if (g->state() != giopStrand::DYING) {
	if (omniORB::trace(30)) {
	  omniORB::logger l;
	  l << "Bi-directional rope is no longer referenced; strand "
	    << (void*)g << " is a candidate for scavenging.\n";
	}
	g->startIdleCounter();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////
void
BiDirServerRope::realIncrRefCount() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(pd_refcount >= 0);

  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
BiDirClientRope::BiDirClientRope(const giopAddressList& addrlist,
				 const omnivector<CORBA::ULong>& preferred) :
  giopRope(addrlist,preferred)
{
  pd_maxStrands = 1;
  pd_oneCallPerConnection = 0;
}

////////////////////////////////////////////////////////////////////////
IOP_C*
BiDirClientRope::acquireClient(const omniIOR* ior,
			       const CORBA::Octet* key,
			       CORBA::ULong keysize,
			       omniCallDescriptor* calldesc) {

  GIOP_C* giop_c = (GIOP_C*) giopRope::acquireClient(ior,key,keysize,calldesc);

  // Bidirectional is only supported in GIOP 1.2 and above
  GIOP::Version v = ior->getIORInfo()->version();
  if (!(v.major > 1 || v.minor >= 2)) {
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "Bidirectional client using normal connection because "
	  << "it is only GIOP " << (int)v.major << "." << (int)v.minor << "\n";
    }
    return giop_c;
  }

  omni_tracedmutex_lock sync(pd_lock);
  giopStrand& s = (giopStrand&)((giopStream&)(*giop_c));
  if (s.connection == 0 && s.state() != giopStrand::DYING) {
    if (omniORB::trace(20)) {
      omniORB::logger log;
      log << "Bidirectional client attempt to connect to "
	  << s.address->address() << "\n";
    }
    unsigned long s_deadline_secs, s_deadline_nanosecs;
    giop_c->getDeadline(s_deadline_secs,s_deadline_nanosecs);

    unsigned long deadline_secs, deadline_nanosecs;
    if (orbParameters::clientConnectTimeOutPeriod.secs ||
	orbParameters::clientConnectTimeOutPeriod.nanosecs) {

      omni_thread::
	get_time(&deadline_secs,
		 &deadline_nanosecs,
		 orbParameters::clientConnectTimeOutPeriod.secs,
		 orbParameters::clientConnectTimeOutPeriod.nanosecs);

      if ((s_deadline_secs && deadline_secs > s_deadline_secs) ||
	  (deadline_secs == s_deadline_secs &&
	   deadline_nanosecs > s_deadline_nanosecs)) {

	giop_c->setDeadline(deadline_secs, deadline_nanosecs);
	calldesc->setDeadline(deadline_secs, deadline_nanosecs);
      }
    }
    else {
      deadline_secs     = s_deadline_secs;
      deadline_nanosecs = s_deadline_nanosecs;
    }

    giopActiveConnection* c = s.address->Connect(deadline_secs,
						 deadline_nanosecs,
						 s.flags);
    if (c) s.connection = &(c->getConnection());
    if (!s.connection) {
      s.state(giopStrand::DYING);
    }
    else {
      // now make the connection managed by the giopServer.
      s.biDir = 1;
      s.gatekeeper_checked = 1;
      giopActiveCollection* watcher = c->registerMonitor();
      if (omniORB::trace(20)) {
	omniORB::logger log;
	log << "Client opened bidirectional connection to " 
	    << s.connection->peeraddress() << "\n";
      }
      if (!giopServer::singleton()->addBiDirStrand(&s,watcher)) {
	{
	  omni_tracedmutex_lock sync(*omniTransportLock);
	  s.connection->decrRefCount();
	}
	s.connection = 0;
	s.biDir = 0;
	giopRope::releaseClient(giop_c);
	OMNIORB_THROW(TRANSIENT,
		      TRANSIENT_BiDirConnUsedWithNoPOA,
		      CORBA::COMPLETED_NO);
      }
    }
  }
  return giop_c;
}

/////////////////////////////////////////////////////////////////////////////
//            Server side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
getBiDirServiceContext(omniInterceptors::serverReceiveRequest_T::info_T& info)
{

  if (!orbParameters::acceptBiDirectionalGIOP) {
    // XXX If the ORB policy is "don't support bidirectional", don't bother 
    // doing any of the stuff below.
    return 1;
  }

  GIOP::Version ver = info.giop_s.version();

  giopStrand& strand = (giopStrand&)((giopStream&)info.giop_s);

  if (ver.minor != 2 || strand.isClient()) {
    // Only parse service context if the GIOP version is 1.2, on the
    // server side
    return 1;
  }

  IOP::ServiceContextList& svclist = info.giop_s.service_contexts();
  CORBA::ULong total = svclist.length();
  for (CORBA::ULong index = 0; index < total; index++) {
    if (svclist[index].context_id == IOP::BI_DIR_IIOP) {
      cdrEncapsulationStream e(svclist[index].context_data.get_buffer(),
			       svclist[index].context_data.length(),1);

      ListenPointList l;
      l <<= e;

      if (l.length() == 0) continue;

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Receive bidir IIOP service context: ( ";

	for (CORBA::ULong i = 0; i < l.length(); i++) {
	  log << (const char*) l[i].host << ":" << l[i].port << " ";
	}

	log << ")\n";
      }

      giopAddressList addrList;

      for (CORBA::ULong i=0; i<l.length(); i++) {
	giopAddress* p = giopAddress::fromTcpAddress(l[i]);
	if (p) addrList.push_back(p);

	// XXX If this connection is SSL, we also add to the redirection
	// list a ssl version of the address. This will ensure that any 
	// callback objects from the other end will score a match.
	// This is necessary because the BI_DIR_IIOP service context does
	// not allow the other side to say an endpoint is SSL.
	// If omniORB 4 is the client, it sends both the TCP and the SSL
	// endpoint in the BI_DIR_IIOP listen point list. It may not 
	// have a TCP endpoint at all. In that case, the IORs from the
	// client will only contain a ssl endpoint. Our hack here will ensure
	// that the unmarshal object reference code will score a match.
	if (strncmp(strand.connection->myaddress(),"giop:ssl",8) == 0) {
	  p = giopAddress::fromSslAddress(l[i]);
	  if (p) addrList.push_back(p);
	}
      }

      if (addrList.empty()) continue;


      // Check serverTransportRule to see if we should allow bidir from
      // this client.
      {
	transportRules::sequenceString actions;
	CORBA::ULong matchedRule;
	CORBA::Boolean acceptbidir;
	CORBA::Boolean dumprule = 0;
	const char* why;

	if ( (acceptbidir = transportRules::serverRules().
	                    match(strand.connection->peeraddress(),
				  actions,matchedRule)) ) {
	  CORBA::ULong i;
	  for (i = 0; i < actions.length(); i++ ) {
	    if (strcmp(actions[i],"bidir") == 0) {
	      break;
	    }
	  }
	  if ( i == actions.length() ) {
	    acceptbidir = 0;
	    why = (const char*) "\"bidir\" is not found in the matched rule: ";
	    dumprule = 1;
	  }
	}
	else {
	  why = (const char*) "no matching rule is bound";
	}

	if ( !acceptbidir ) {
	  if ( omniORB::trace(1) ) {
	    omniORB::logger log;
	    log << "Request from " << strand.connection->peeraddress()
		<< " to switch to bidirectional is rejected because " 	
		<< why;
	    if (dumprule) {
	      CORBA::String_var rule;
	      rule = transportRules::serverRules().dumpRule(matchedRule);
	      log << "\"" << (const char*) rule << "\"";
	    }
	    log << "\n";
	  }
	  continue;
	}

	if ( omniORB::trace(5) ) {

	  CORBA::String_var rule;
	  rule = transportRules::serverRules().dumpRule(matchedRule);

	  omniORB::logger log;
	  log << "Accepted request from " 
	      << strand.connection->peeraddress()
	      << " to switch to bidirectional because of this rule: \""
	      << (const char*) rule << "\"\n";
	}
      }

      {
	ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

	omni_tracedmutex_lock sync(*omniTransportLock);

	if (!strand.biDir) {
	  strand.biDir = 1;
	  strand.stopIdleCounter();
	  if (!strand.server->notifySwitchToBiDirectional(strand.connection))
	    return 1;
	}

	BiDirServerRope* r = BiDirServerRope::addRope(&strand,addrList);
      }
      
      giopAddressList::const_iterator addr, last;
      addr = addrList.begin();
      last = addrList.end();
      for (; addr != last; addr++) {
	delete (*addr);
      }
    }
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Client side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
setBiDirServiceContext(omniInterceptors::clientSendRequest_T::info_T& info) {

  if (!orbParameters::offerBiDirectionalGIOP) {
    // XXX If the ORB policy is "don't support bidirectional", don't bother 
    // doing any of the stuff below.
    return 1;
  }

  giopStrand& g = (giopStrand&)info.giop_c;
  GIOP::Version ver = info.giop_c.version();

  if (ver.minor != 2 || !g.biDir || !g.isClient() || g.biDir_initiated) {
    // Only send service context if the GIOP version is 1.2, this is
    // a bidirectional connection, on the client side and it has not
    // been used yet.
    return 1;
  }

  const orbServer::EndpointList& epts = omniObjAdapter::listMyEndpoints();
  CORBA::ULong el = epts.length();

  ListenPointList l(el);
  CORBA::ULong j = 0;

  for (CORBA::ULong i=0 ; i != el; i++) {
    const char* ep = epts[i];
    if (strncmp(ep,"giop:tcp:",9) == 0 || strncmp(ep,"giop:ssl:",9) == 0) {
      l.length(j+1);
      if (!tcpTransportImpl::parseAddress(ep+9, l[j++])) OMNIORB_ASSERT(0);
    }
  }
  if (l.length()) {
    cdrEncapsulationStream e(CORBA::ULong(0),CORBA::Boolean(1));
    l >>= e;

    CORBA::Octet* data;
    CORBA::ULong max,datalen;
    e.getOctetStream(data,max,datalen);

    CORBA::ULong len = info.service_contexts.length() + 1;
    info.service_contexts.length(len);
    info.service_contexts[len-1].context_id = IOP::BI_DIR_IIOP;
    info.service_contexts[len-1].context_data.replace(max,datalen,data,1);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Send bidir IIOP service context: ( ";

      for (CORBA::ULong i = 0; i < l.length(); i++) {
	log << (const char*) l[i].host << ":" << l[i].port << " ";
      }

      log << ")\n";
    }
  }
  g.biDir_initiated = 1;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class acceptBiDirectionalGIOPHandler : public orbOptions::Handler {
public:

  acceptBiDirectionalGIOPHandler() : 
    orbOptions::Handler("acceptBiDirectionalGIOP",
			"acceptBiDirectionalGIOP = 0 or 1",
			1,
			"-ORBacceptBiDirectionalGIOP < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::acceptBiDirectionalGIOP = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::acceptBiDirectionalGIOP,
			     result);
  }

};

static acceptBiDirectionalGIOPHandler acceptBiDirectionalGIOPHandler_;

/////////////////////////////////////////////////////////////////////////////
class offerBiDirectionalGIOPHandler : public orbOptions::Handler {
public:

  offerBiDirectionalGIOPHandler() : 
    orbOptions::Handler("offerBiDirectionalGIOP",
			"offerBiDirectionalGIOP = 0 or 1",
			1,
			"-ORBofferBiDirectionalGIOP < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::offerBiDirectionalGIOP = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::offerBiDirectionalGIOP,
			     result);
  }

};

static offerBiDirectionalGIOPHandler offerBiDirectionalGIOPHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopbidir_initialiser : public omniInitialiser {
public:

  omni_giopbidir_initialiser() {
    orbOptions::singleton().registerHandler(offerBiDirectionalGIOPHandler_);
    orbOptions::singleton().registerHandler(acceptBiDirectionalGIOPHandler_);
  }

  void attach() {

    // install interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->clientSendRequest.add(setBiDirServiceContext);
    interceptors->serverReceiveRequest.add(getBiDirServiceContext);
  }
  void detach() {
    // Get rid of any remaining ropes. By now they should all be strand-less.
    omni_tracedmutex_lock sync(*omniTransportLock);

    RopeLink* p = BiDirServerRope::ropes.next;
    giopRope* gr;
    int i=0;

    while (p != &BiDirServerRope::ropes) {
      gr = (giopRope*)p;
      OMNIORB_ASSERT(gr->pd_refcount == 0 &&
		     RopeLink::is_empty(gr->pd_strands) &&
		     !gr->pd_nwaiting);
      p = p->next;
      gr->RopeLink::remove();
      delete gr;
      ++i;
    }
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << i << " remaining bidir rope" << (i == 1 ? "" : "s")
	<< " deleted.\n";
    }
  }
};


static omni_giopbidir_initialiser initialiser;

omniInitialiser& omni_giopbidir_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

