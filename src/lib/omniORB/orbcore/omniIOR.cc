// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniIOR.cc                 Created on: 19/09/2000
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 1999-2000 AT&T Laboratories, Cambridge
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

/*
  $Log$
  Revision 1.1.2.4.2.2  2001/02/23 19:25:19  sll
  Merged interim FT client stuff.

  Revision 1.1.2.4.2.1  2001/02/23 16:50:36  sll
  SLL work in progress.

  Revision 1.1.2.4  2000/11/15 17:25:11  sll
  Added char, wchar codeset convertor support.

  Revision 1.1.2.3  2000/10/04 16:53:48  sll
  Make sure IIOP version is set in the iiop member.

  Revision 1.1.2.2  2000/10/03 17:37:07  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.1.2.1  2000/09/27 17:30:30  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniIdentity.h>
#include <initialiser.h>
#include <giopStreamImpl.h>
#include <omniORB4/omniInterceptors.h>
#include <giopRope.h>

omni_tracedmutex*                omniIOR::lock = 0;

omniIOR::omniIOR(char* repoId, IOP::TaggedProfileList* iop) : 
  pd_iopProfiles(iop),
  pd_decoded(0), 
  pd_addr_mode(GIOP::KeyAddr),
  pd_addr_selected_profile_index(-1),
  pd_orb_type(0),
  pd_tcs_c(0),
  pd_tcs_w(0),
  pd_is_IOGR(0),
  pd_refCount(1)
{
  {
    omniORB::logger log;
    log << "omniIOR ctor(1)\n";
  }

  pd_repositoryID = repoId;
}

omniIOR::omniIOR(char* repoId, IOP::TaggedProfile* iop, CORBA::ULong niops,
		 CORBA::ULong selected_profile_index) :
  pd_decoded(0), 
  pd_addr_mode(GIOP::KeyAddr),
  pd_addr_selected_profile_index((CORBA::Long)selected_profile_index),
  pd_orb_type(0),
  pd_tcs_c(0),
  pd_tcs_w(0),
  pd_is_IOGR(0),
  pd_refCount(1)
{    
  {
    omniORB::logger log;
    log << "omniIOR ctor(2)\n";
  }

  pd_repositoryID = repoId;
  pd_iopProfiles = new IOP::TaggedProfileList(niops,niops,iop,1);
}

omniIOR::omniIOR(const char* repoId, omniIdentity* id) :
  pd_iopProfiles(0),
  pd_decoded(0), 
  pd_addr_mode(GIOP::KeyAddr),
  pd_addr_selected_profile_index(-1),
  pd_orb_type(0),
  pd_tcs_c(0),
  pd_tcs_w(0),
  pd_is_IOGR(0),
  pd_refCount(1)
{
  {
    omniORB::logger log;
    log << "omniIOR ctor(3)\n";
  }

  pd_repositoryID = repoId;   // copied.

  pd_iiop.object_key.replace((CORBA::ULong)id->keysize(),
			     (CORBA::ULong)id->keysize(),
			     (CORBA::Octet*)id->key(),0);

  encodeIOPprofile();
  // This is a ctor for local objects. The ORB never looks at the
  // decoded members again.
  // To minimize memory footprint, clear out all decoded members.
  clearDecodedMembers();
}

omniIOR::omniIOR(const char* repoId, 
		 const _CORBA_Unbounded_Sequence_Octet& key,
		 const IIOP::Address* addrs, CORBA::ULong naddrs,
		 GIOP::Version ver, CORBA::Boolean callInterceptors) :
  pd_iopProfiles(0),
  pd_decoded(0), 
  pd_addr_mode(GIOP::KeyAddr),
  pd_addr_selected_profile_index(-1),
  pd_orb_type(0),
  pd_tcs_c(0),
  pd_tcs_w(0),
  pd_is_IOGR(0),
  pd_refCount(1)
{
  {
    omniORB::logger log;
    log << "omniIOR ctor(4)\n";
  }

  pd_repositoryID = repoId;

  pd_iiop.object_key.replace(key.length(),key.length(),
			     (CORBA::Octet*)key.get_buffer(),0);

  pd_iiop.address.host = addrs[0].host; 
  pd_iiop.address.port = addrs[0].port;
  pd_iiop.version = ver;

  if (naddrs > 1) {
    for (CORBA::ULong index = 1; index < naddrs; index++) {
      IIOP::addAlternativeIIOPAddress(pd_iiop.components, addrs[index]);
    }
  }
  encodeIOPprofile(callInterceptors);
}

omniIOR::~omniIOR()
{
  {
    omniORB::logger log;
    log << "~omniIOR\n";
  }

  OMNIORB_ASSERT(pd_refCount <= 0);
}


omniIOR*
omniIOR::duplicateNoLock()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 1);
  OMNIORB_ASSERT(pd_refCount > 0);
  pd_refCount++;
  return this;
}

omniIOR*
omniIOR::duplicate()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 0);
  omni_tracedmutex_lock sync(*omniIOR::lock);
  return duplicateNoLock();
}

void
omniIOR::releaseNoLock()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 1);
  if (--pd_refCount <= 0)
    delete this;
}

void
omniIOR::release()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniIOR::lock, 0);
  omni_tracedmutex_lock sync(*omniIOR::lock);
  releaseNoLock();
}


void
omniIOR::marshalIORAddressingInfo(cdrStream& s)
{
  OMNIORB_ASSERT(pd_decoded);

  pd_addr_mode >>= s;
  if (pd_addr_mode == GIOP::KeyAddr) {
    pd_iiop.object_key >>= s;
  }
  else if (pd_addr_mode == GIOP::ProfileAddr) {
    pd_iopProfiles[pd_addr_selected_profile_index] >>= s;
  }
  else {
    pd_addr_selected_profile_index >>= s;
    IOP::IOR ior;
    ior.type_id = pd_repositoryID;
    ior.profiles.replace(pd_iopProfiles->maximum(),
			 pd_iopProfiles->length(),
			 pd_iopProfiles->get_buffer(),0);
    ior >>= s;
  }
}


void
omniIOR::clearDecodedMembers()
{
  pd_iiop.address.host = _CORBA_String_helper::empty_string;

  if (pd_iiop.object_key.release())
    _CORBA_Unbounded_Sequence_Octet::freebuf(pd_iiop.object_key.get_buffer(1));

  if (pd_iiop.components.release())
    IOP::MultipleComponentProfile::freebuf(pd_iiop.components.get_buffer(1));

  pd_decoded = 0;
}

IOP::TaggedComponent&
omniIOR::newIIOPtaggedComponent()
{
  CORBA::ULong len = pd_iiop.components.length() + 1;

  if (pd_iiop.components.maximum() < len) {
    // Instead of letting the sequence to increase its buffer space,
    // we do it ourselves here to avoid frequent reallocation.
    CORBA::ULong newmax = pd_iiop.components.maximum() + 8;
    IOP::TaggedComponent* newbuf = IOP::MultipleComponentProfile::allocbuf(newmax);
    IOP::TaggedComponent* oldbuf = pd_iiop.components.get_buffer();
    for (CORBA::ULong i = 0; i < pd_iiop.components.length(); i++) {
      newbuf[i].tag = oldbuf[i].tag;
      newbuf[i].component_data.replace(oldbuf[i].component_data.maximum(),
				       oldbuf[i].component_data.length(),
				       oldbuf[i].component_data.get_buffer(1),
				       1);
    }
    pd_iiop.components.replace(newmax,len - 1,newbuf,1);
  }
  pd_iiop.components.length(len);

  return pd_iiop.components[len];
}

void
omniIOR::encodeIOPprofile(CORBA::Boolean call_interceptors)
{
  if (!pd_iopProfiles.operator->()) {
    pd_iopProfiles = new IOP::TaggedProfileList();
  }
  else {
    // If ever we are called more than once, we erase the old profile list
    // and starts all over again.
    pd_iopProfiles->length(0);
  }

  // Call interceptors
  if (call_interceptors) {
    omniORB::getInterceptors()->encodeIOR.visit(this);
  }

  if (strlen(pd_iiop.address.host) != 0) {
    // Do not encode the IIOP profile if no IIOP address is set.
    CORBA::ULong last = pd_iopProfiles->length();
    pd_iopProfiles->length(last+1);
    IIOP::encodeProfile(pd_iiop,pd_iopProfiles[last]);
  }
  OMNIORB_ASSERT(pd_iopProfiles->length() != 0);
}

int
omniIOR::selectRope(_OMNI_NS(Rope)*& rope, _CORBA_Boolean& is_local)
{
  if (!pd_decoded) {
    CORBA::ULong total = pd_iopProfiles->length();
    CORBA::ULong index;
    for (index = 0; index < total; index++) {
      if (pd_iopProfiles[index].tag == IOP::TAG_INTERNET_IOP)
	break;
      // XXX We should do a better job to cope with IOR with multiple
      // IIOP profiles. More work needed. 
    }
    if (index != total) {
      pd_addr_selected_profile_index = index;
      IIOP::decodeProfile(pd_iopProfiles[index],pd_iiop);
    }
    // If there is any multiple component profile, decode its content as well
    // because these tagged components apply to all IIOP profiles, this one
    // included.
    for (index = 0; index < total; index++) {
      if (pd_iopProfiles[index].tag == IOP::TAG_MULTIPLE_COMPONENTS)
	IIOP::decodeMultiComponentProfile(pd_iopProfiles[index],pd_iiop);
    }

    // Call interceptors
    omniORB::getInterceptors()->decodeIOR.visit(this);
    pd_decoded = 1;
  }


  if (!pd_addresses.size()) {
    return _OMNI_NS(giopRope)::selectRope(pd_addresses,rope,is_local);
  }
  return 0;

}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omni_omniIOR_initialiser : public omniInitialiser {
public:

  void attach() {

    omniIOR::lock   = new omni_tracedmutex;
  }

  void detach() {
    delete omniIOR::lock;
    omniIOR::lock = 0;
  }
};

static omni_omniIOR_initialiser initialiser;

omniInitialiser& omni_omniIOR_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
