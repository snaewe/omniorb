// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcpSocket.cc               Created on: 18/3/96
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

/*
  $Log$
  Revision 1.8.4.5  2000/03/27 17:26:16  sll
  Change reference to CORBA::Octet to _CORBA_Octet to workaround a bug in
  gcc in CodeFusion 1.0.

  Revision 1.8.4.4  1999/10/05 20:35:36  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.8.4.3  1999/10/02 18:21:30  sll
  Added support to decode optional tagged components in the IIOP profile.
  Added support to negogiate with a firewall proxy- GIOPProxy to invoke
  remote objects inside a firewall.
  Added tagged component TAG_ORB_TYPE to identify omniORB IORs.

  Revision 1.8.4.2  1999/09/25 17:00:22  sll
  Merged changes from omni2_8_develop branch.

  Revision 1.8.2.1  1999/09/21 20:37:18  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.8.4.1  1999/09/15 20:18:28  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.8  1999/08/23 08:35:59  sll
  Do not prepend the tcpSocketFactoryType singleton onto the
  ropeFactoryTypeList. This has been done by the ctor of ropeFactoryType.

  Revision 1.7  1999/03/11 16:25:56  djr
  Updated copyright notice

  Revision 1.6  1998/08/14 13:54:13  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.5  1998/08/13 16:10:07  sll
  Now can accept IOR with IIOP 1.1 profile. IIOP 1.1 specific part of
  the profile is ignored silently.

  Revision 1.4  1998/04/07 19:39:24  sll
  Replace cerr with omniORB::log.

  Revision 1.3  1998/03/13 17:34:16  sll
  Bug fix. No longer reject an IOR with zero length object key.

  Revision 1.2  1997/12/12 18:45:33  sll
  Added call to print out the version of gatekeeper.

  Revision 1.1  1997/12/09 18:43:12  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <giopStreamImpl.h>
#include <giopObjectInfo.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <gatekeeper.h>

const char* tcpSocketEndpoint::protocol_name = "TCPIP";
tcpSocketFactoryType* tcpSocketFactoryType::singleton = 0;

static void unmarshalIOPComponent(cdrStream&,GIOPObjectInfo*);
// XXX Temporary holding place for this function. More appropriate place
//     is the IOP module.


void
tcpSocketFactoryType::init()
{
  if (singleton) return;
  singleton = new tcpSocketFactoryType;
  singleton->pd_optionalcomponents.length(1);

  // Inside TAG_ORB_TYPE to identify omniORB
  singleton->pd_optionalcomponents[0].tag = IOP::TAG_ORB_TYPE;
  cdrEncapsulationStream s(8,1);
  omniORB_TAG_ORB_TYPE >>= s;
  _CORBA_Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  singleton->pd_optionalcomponents[0].component_data.replace(max,len,p,1);
  
  if (omniORB::trace(2)) {
    omniORB::logger log("omniORB: gateKeeper is ");
    log << gateKeeper::version() << "\n";
  }
}

tcpSocketFactoryType::tcpSocketFactoryType()
{
}

CORBA::Boolean
tcpSocketFactoryType::is_IOPprofileId(IOP::ProfileId tag) const
{
  return (tag == IOP::TAG_INTERNET_IOP) ? 1 : 0;
}

CORBA::Boolean
tcpSocketFactoryType::is_protocol(const char* name) const
{
  return (strcmp(name,tcpSocketEndpoint::protocol_name) == 0) ? 1 : 0;
}

CORBA::Boolean
tcpSocketFactoryType::decodeIOPprofile(const IOP::TaggedProfile& profile,
				       // return values:
				       Endpoint*&     addr,
				       GIOPObjectInfo* objectInfo) const

{
  if (profile.tag != IOP::TAG_INTERNET_IOP)
    return 0;

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1); /* No 8 bytes align data member, so
				  4 bytes align is sufficent. */

  IIOP::ProfileBody body;

  body.version.major <<= s;
  body.version.minor <<= s;
  
  if (body.version.major != 1) 
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  body.host <<= s;

  body.port <<= s;

  body.object_key <<= s;

  switch (body.version.minor) {
  case 0:
    // Check if the profile body ends here.
    if (s.checkInputOverrun(1,1))
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    break;
  default:
    {
      CORBA::ULong ncomponents;
      ncomponents <<= s;

      for (CORBA::ULong i=0; i < ncomponents; i++) {
	unmarshalIOPComponent(s,objectInfo);
      }
    }
  }
  objectInfo->version_.major = body.version.major;
  objectInfo->version_.minor = body.version.minor;
  CORBA::ULong max = body.object_key.maximum();
  CORBA::ULong len = body.object_key.length();
  objectInfo->object_key_.replace(max,len,body.object_key.get_buffer(1),1);
  addr = new tcpSocketEndpoint((CORBA::Char*)body.host._ptr,body.port);
  return 1;
}

void
tcpSocketFactoryType::encodeIOPprofile(const ropeFactoryType::EndpointList& addrs,
				       const CORBA::Octet* objkey,
				       const size_t objkeysize,
				       IOP::TaggedProfile& profile) const
{
  tcpSocketEndpoint* tcpaddr;
  CORBA::ULong hlen;

  // Check if we have multiple endpoints, if so create a tagged component
  // for each of the extra endpoints.
  IOP::MultipleComponentProfile alternate_iiop_address;
  if (addrs.length() > 1) {

    alternate_iiop_address.length(addrs.length() - 1);

    for (CORBA::ULong index = 0; index < addrs.length()-1; index++) {

      alternate_iiop_address[index].tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;

      tcpaddr = tcpSocketEndpoint::castup(addrs[index+1]);
      hlen = strlen((const char*)tcpaddr->host()) + 1;

      cdrEncapsulationStream s(hlen+8,1);
      hlen >>= s;
      s.put_char_array((const CORBA::Char*)tcpaddr->host(),hlen);
      tcpaddr->port() >>= s;

      _CORBA_Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
      alternate_iiop_address[index].component_data.replace(max,len,p,1);
    }
  }

  tcpaddr = tcpSocketEndpoint::castup(addrs[0]);
  hlen = strlen((const char*)tcpaddr->host()) + 1;

  profile.tag = IOP::TAG_INTERNET_IOP;

  GIOP::Version ver = giopStreamImpl::maxVersion()->version();

  CORBA::ULong bufsize;
  {
    cdrCountingStream s;
    omni::myByteOrder >>= s;
    ver.major >>= s;
    ver.minor >>= s;
    hlen >>= s;
    s.put_char_array((CORBA::Char*)tcpaddr->host(),hlen);
    tcpaddr->port() >>= s;
    ::operator>>=((CORBA::ULong) objkeysize,s);
    s.put_char_array((CORBA::Char*)objkey,objkeysize);

    if (ver.minor > 0) {

      CORBA::ULong total = alternate_iiop_address.length() + 
	                   pd_optionalcomponents.length();

      ::operator>>=(total,s);

      CORBA::ULong index;

      total = pd_optionalcomponents.length();
      for (index=0; index < total; index++) {
	pd_optionalcomponents[index].tag >>= s;
	pd_optionalcomponents[index].component_data >>= s;
      }
      total = alternate_iiop_address.length();
      for (index=0; index < total; index++) {
	alternate_iiop_address[index].tag >>= s;
	alternate_iiop_address[index].component_data >>= s;
      }
    }
    bufsize = s.total();
  }

  {
    cdrEncapsulationStream s(bufsize,1);
    ver.major >>= s;
    ver.minor >>= s;
    hlen >>= s;
    s.put_char_array((CORBA::Char*)tcpaddr->host(),hlen);
    tcpaddr->port() >>= s;
    ::operator>>=((CORBA::ULong) objkeysize,s);
    s.put_char_array((CORBA::Char*)objkey,objkeysize);

    if (ver.minor > 0) {

      CORBA::ULong total = alternate_iiop_address.length() + 
	                   pd_optionalcomponents.length();

      ::operator>>=(total,s);

      CORBA::ULong index;

      total = pd_optionalcomponents.length();
      for (index=0; index < total; index++) {
	pd_optionalcomponents[index].tag >>= s;
	pd_optionalcomponents[index].component_data >>= s;
      }
      total = alternate_iiop_address.length();
      for (index=0; index < total; index++) {
	alternate_iiop_address[index].tag >>= s;
	alternate_iiop_address[index].component_data >>= s;
      }
    }

    _CORBA_Octet* p;
    CORBA::ULong max;
    CORBA::ULong len;
    s.getOctetStream(p,max,len);
    profile.profile_data.replace(max,len,p,1);
  }
}

void
tcpSocketFactoryType::insertOptionalIOPComponent(IOP::TaggedComponent& c)
{
  CORBA::ULong index = pd_optionalcomponents.length();
  pd_optionalcomponents.length(index+1);
  pd_optionalcomponents[index] = c;
}


const IOP::MultipleComponentProfile&
tcpSocketFactoryType::getOptionalIOPComponents() const
{
  return pd_optionalcomponents;
}

tcpSocketEndpoint::tcpSocketEndpoint(CORBA::Char *h,CORBA::UShort p)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(h);
  port(p);
}

tcpSocketEndpoint::tcpSocketEndpoint(const tcpSocketEndpoint *e)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(e->host());
  port(e->port());
}

tcpSocketEndpoint&
tcpSocketEndpoint::operator=(const tcpSocketEndpoint &e) 
{
  host(e.host());
  port(e.port());
  return *this;
}
  
CORBA::Boolean
tcpSocketEndpoint::operator==(const tcpSocketEndpoint *e)
{
  if ((strcmp((const char *)pd_host,
	      (const char *)e->host())==0) && (pd_port == e->port()))
    return 1;
  else
    return 0;
}
 
tcpSocketEndpoint::~tcpSocketEndpoint()
{
  if (pd_host) delete [] pd_host;
}

CORBA::Char* 
tcpSocketEndpoint::host() const
{ 
  return pd_host;
}

void 
tcpSocketEndpoint::host(const CORBA::Char *p) 
{
  if (pd_host) delete [] pd_host;
  if (p) {
    pd_host = new CORBA::Char [strlen((char *)p) + 1];
    strcpy((char *)pd_host,(char *)p);
  }
  else {
    pd_host = new CORBA::Char [1];
    pd_host[0] = '\0';
  }
  return;
}

CORBA::UShort
tcpSocketEndpoint::port() const
{ 
  return pd_port;
}

void
tcpSocketEndpoint::port(const CORBA::UShort p) 
{ 
  pd_port = p;
}
  
tcpSocketEndpoint *
tcpSocketEndpoint::castup(const Endpoint *e) 
{
  if (e->is_protocol((CORBA::Char *)tcpSocketEndpoint::protocol_name)) {
    return (tcpSocketEndpoint *)e;
  }
  else {
    return 0;
  }
}

CORBA::Boolean
tcpSocketIncomingRope::this_is(Endpoint *&e)
{
  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == me)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(me);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}

CORBA::Boolean
tcpSocketOutgoingRope::remote_is(Endpoint *&e)
{
  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == remote)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(remote);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}

//////////////////////////////////////////////////////////////////////////
static void unmarshal_TAG_ORB_TYPE(cdrStream&,IOP::ComponentId,GIOPObjectInfo*);
static void unmarshal_TAG_generic(cdrStream&,IOP::ComponentId,GIOPObjectInfo*);

//////////////////////////////////////////////////////////////////////////
// For the TAGs that the ORB will look at, add a handler to the following
// table. Use unmarshal_TAG_generic just to store the encapsulated octet
// stream in the GIOPObjectInfo
//
static struct {
  IOP::ComponentId id;
  void (*fn)(cdrStream&, IOP::ComponentId, GIOPObjectInfo*);
} componentUnmarshalHandlers[] = {
  // This table must be arranged in ascending order of IOP::ComponentId
  { IOP::TAG_ORB_TYPE,  unmarshal_TAG_ORB_TYPE },
  { IOP::TAG_CODE_SETS, 0 },
  { IOP::TAG_POLICIES, 0 },
  { IOP::TAG_ALTERNATE_IIOP_ADDRESS, unmarshal_TAG_generic },
  { IOP::TAG_COMPLETE_OBJECT_KEY, 0 },
  { IOP::TAG_ENDPOINT_ID_POSITION, 0 },
  { IOP::TAG_LOCATION_POLICY, 0 },
  { IOP::TAG_ASSOCIATION_OPTIONS, 0 },
  { IOP::TAG_SEC_NAME, 0 },
  { IOP::TAG_SPKM_1_SEC_MECH, 0 },
  { IOP::TAG_SPKM_2_SEC_MECH, 0 },
  { IOP::TAG_KERBEROSV5_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_SECRET_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH, 0 },
  { IOP::TAG_SSL_SEC_TRANS, unmarshal_TAG_generic },
  { IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH, 0 },
  { IOP::TAG_GENERIC_SEC_MECH, 0 },
  { IOP::TAG_FIREWALL_TRANS, unmarshal_TAG_generic },
  { IOP::TAG_SCCP_CONTACT_INFO, 0 },
  { IOP::TAG_JAVA_CODEBASE, 0 },
  { IOP::TAG_DCE_STRING_BINDING, 0 },
  { IOP::TAG_DCE_BINDING_NAME, 0 },
  { IOP::TAG_DCE_NO_PIPES, 0 },
  { IOP::TAG_DCE_SEC_MECH, 0 },
  { IOP::TAG_INET_SEC_TRANS, 0 },
  { 0xffffffff, 0 }
};

//////////////////////////////////////////////////////////////////////////
//  Unmarshal IOP Tag components.
static
void 
unmarshalIOPComponent(cdrStream& s,GIOPObjectInfo* objectInfo)
{
  static int tablesize = 0;

  if (!tablesize) {
    while (componentUnmarshalHandlers[tablesize].id != 0xffffffff) tablesize++;
  }

  int top = tablesize;
  int bottom = 0;

  IOP::ComponentId v;
  v <<= s;

  do {
    int index = (top + bottom) >> 1;
    IOP::ComponentId id = componentUnmarshalHandlers[index].id;
    if (id == v) {
      if (componentUnmarshalHandlers[index].fn) {
	componentUnmarshalHandlers[index].fn(s,v,objectInfo);
	return;
      }
      break;
    }
    else if (id > v) {
      top = index;
    }
    else {
      bottom = index + 1;
    }
  } while (top != bottom);

  // Default is to skip this component quietly
  {
    CORBA::ULong len;
    len <<= s;
    s.skipInput(len);
  }
}

//////////////////////////////////////////////////////////////////////////
static
void
unmarshal_TAG_ORB_TYPE(cdrStream& s, IOP::ComponentId, GIOPObjectInfo* g)
{
  _CORBA_Unbounded_Sequence_Octet v;
  v <<= s;

  cdrEncapsulationStream e(v.get_buffer(),v.length(),1);
  g->orb_type_ <<= e;
}

//////////////////////////////////////////////////////////////////////////
static
void
unmarshal_TAG_generic(cdrStream& s, IOP::ComponentId id, GIOPObjectInfo* g)
{
  if (!g->tag_components_) {
    g->tag_components_ = new IOP::MultipleComponentProfile(3);
  }
  CORBA::ULong index = g->tag_components_->length();
  g->tag_components_->length(index+1);
  (*(g->tag_components_))[index].tag = id;
  (*(g->tag_components_))[index].component_data <<= s;
}

#undef Swap16
#undef Swap32
