// -*- Mode: C++; -*-
//                            Package   : omniORB
// ior.cc                     Created on: 5/7/96
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.10.2.12  2001/06/13 20:13:15  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.10.2.11  2001/06/11 17:53:22  sll
   The omniIOR ctor used by genior and corbaloc now has the option to
   select whether to call interceptors and what set of interceptors to call.

  Revision 1.10.2.10  2001/05/31 16:18:13  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.10.2.9  2001/05/09 16:59:08  sll
  Added unmarshalObjectKey() to allow quick extraction of the object key.

  Revision 1.10.2.8  2001/04/18 18:18:07  sll
  Big checkin with the brand new internal APIs.

  Revision 1.10.2.7  2000/12/05 17:39:31  dpg1
  New cdrStream functions to marshal and unmarshal raw strings.

  Revision 1.10.2.6  2000/11/20 14:42:23  sll
  Do not insert codeset component if the IOR is GIOP 1.0.

  Revision 1.10.2.5  2000/11/15 17:24:45  sll
  Added service context marshalling operators.
  Added hooks to add TAG_CODE_SETS componment to an IOR.

  Revision 1.10.2.4  2000/11/03 19:12:07  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.10.2.3  2000/10/04 16:53:16  sll
  Added default interceptor to encode and decode supported tag components.

  Revision 1.10.2.2  2000/09/27 18:20:32  sll
  Removed obsoluted IOP::iorToEncapStr and IOP::EncapStrToIor.
  Added new function IOP::IOR::unmarshaltype_id(), IIOP::encodeProfile(),
  IIOP::decodeProfile(), IIOP::addAlternativeIIOPAddress().
  Use the new cdrStream abstraction.

  Revision 1.10.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.11  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.9.6.5  2000/06/22 10:40:15  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.9.6.4  2000/04/27 10:50:30  dpg1
  Interoperable Naming Service

  IOR: prefix is not case sensitive.

  Revision 1.9.6.3  1999/10/14 16:22:11  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.9.6.2  1999/09/27 08:48:32  djr
  Minor corrections to get rid of warnings.

  Revision 1.9.6.1  1999/09/22 14:26:51  djr
  Major rewrite of orbcore to support POA.

  Revision 1.9  1999/05/25 17:06:14  sll
  Make sure all padding bytes are converted to 0s in the stringified IOR.

  Revision 1.8  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.7  1998/08/14 13:48:04  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.6  1997/12/09 17:32:39  sll
  Removed obsoluted functions IIOP::profileToEncapStream and
  IIOP::EncapStreamToProfile.
  IOP::EncapStrToIor now accepts dodgy nil object reference.

  Revision 1.5  1997/08/21 22:04:23  sll
  minor cleanup to get rid of purify's warnings.

// Revision 1.4  1997/05/06  15:21:58  sll
// Public release.
//
  */

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <stdio.h>

OMNI_USING_NAMESPACE(omni)

void
IOP::TaggedProfile::operator>>= (cdrStream &s) const {
    tag >>= s;
    profile_data >>= s;
}


void
IOP::TaggedProfile::operator<<= (cdrStream &s) {
  tag <<= s;
  profile_data <<= s;
}


void
IOP::TaggedComponent::operator>>= (cdrStream& s) const {
  tag >>= s;
  component_data >>= s;
}

void
IOP::TaggedComponent::operator<<= (cdrStream& s) {
  tag <<= s;
  component_data <<= s;
}


void
IOP::ServiceContext::operator>>= (cdrStream& s) const {
  context_id >>= s;
  context_data >>= s;
}

void
IOP::ServiceContext::operator<<= (cdrStream& s) {
  context_id <<= s;
  context_data <<= s;
}

void
IOP::IOR::operator<<= (cdrStream& s) {
  type_id = unmarshaltype_id(s);
  profiles <<= s;
}

void
IOP::IOR::operator>>= (cdrStream& s) {
  type_id >>= s;
  profiles >>= s;
}


char*
IOP::IOR::unmarshaltype_id(cdrStream& s) {
  CORBA::ULong idlen;
  CORBA::String_var id;

  idlen <<= s;

  if (!s.checkInputOverrun(1,idlen))
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  switch (idlen) {

  case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
#else
    // According to the CORBA specification 2.0 section 10.6.2:
    //   Null object references are indicated by an empty set of
    //   profiles, and by a NULL type ID (a string which contain
    //   only *** a single terminating character ***).
    //
    // Therefore the idlen should be 1.
    // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
    // and sends out a 0 len string.
    // We quietly accept it here. Turn this off by defining
    //   NO_SLOPPY_NIL_REFERENCE
    id = CORBA::string_alloc(1);
    ((char*)id)[0] = '\0';
#endif
    break;

  case 1:
    id = CORBA::string_alloc(1);
    ((char*)id)[0] = s.unmarshalOctet();
    if (((char*)id)[0] != '\0')
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
    idlen = 0;
    break;

  default:
    id = CORBA::string_alloc(idlen);
    s.get_octet_array((CORBA::Octet*)((const char*)id), idlen);
    if( ((char*)id)[idlen - 1] != '\0' )
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
    break;
  }

  return id._retn();
}


void
IIOP::encodeProfile(const IIOP::ProfileBody& body,IOP::TaggedProfile& profile)
{
  profile.tag = IOP::TAG_INTERNET_IOP;

  CORBA::ULong bufsize;
  {
    cdrCountingStream s(cdrMemoryStream::default_tcs_c,
			cdrMemoryStream::default_tcs_w);
    s.marshalOctet(omni::myByteOrder);
    s.marshalOctet(body.version.major);
    s.marshalOctet(body.version.minor);
    s.marshalRawString(body.address.host);
    body.address.port >>= s;
    body.object_key >>= s;

    if (body.version.minor > 0) {
      CORBA::ULong total = body.components.length();
      if (total) {
	total >>= s;
	for (CORBA::ULong index=0; index < total; index++) {
	  body.components[index] >>= s;
	}
      }
    }
    bufsize = s.total();
  }

  {
    cdrEncapsulationStream s(bufsize,1);
    s.marshalOctet(body.version.major);
    s.marshalOctet(body.version.minor);
    s.marshalRawString(body.address.host);
    body.address.port >>= s;
    body.object_key >>= s;

    if (body.version.minor > 0) {
      CORBA::ULong total = body.components.length();
      total >>= s;
      for (CORBA::ULong index=0; index < total; index++) {
	body.components[index] >>= s;
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
IIOP::encodeMultiComponentProfile(const IOP::MultipleComponentProfile& body,
				  IOP::TaggedProfile& profile)
{
  profile.tag = IOP::TAG_MULTIPLE_COMPONENTS;

  CORBA::ULong bufsize;
  {
    cdrCountingStream s(cdrMemoryStream::default_tcs_c,
			cdrMemoryStream::default_tcs_w);
    s.marshalOctet(omni::myByteOrder);
    CORBA::ULong total = body.length();
    if (total) {
      total >>= s;
      for (CORBA::ULong index=0; index < total; index++) {
	body[index] >>= s;
      }
    }
    bufsize = s.total();
  }

  {
    cdrEncapsulationStream s(bufsize,1);
    CORBA::ULong total = body.length();
    if (total) {
      total >>= s;
      for (CORBA::ULong index=0; index < total; index++) {
	body[index] >>= s;
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
IIOP::unmarshalProfile(const IOP::TaggedProfile& profile,
		       IIOP::ProfileBody& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_INTERNET_IOP);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  body.version.major = s.unmarshalOctet();
  body.version.minor = s.unmarshalOctet();

  if (body.version.major != 1)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  body.address.host = s.unmarshalRawString();
  body.address.port <<= s;
  body.object_key <<= s;

  if (body.version.minor > 0) {
    CORBA::ULong total;
    total <<= s;
    if (total) {
      if (!s.checkInputOverrun(1,total))
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
      body.components.length(total);
      for (CORBA::ULong index=0; index<total; index++) {
	body.components[index] <<= s;
      }
    }
  }
  // Check if the profile body ends here.
  if (s.checkInputOverrun(1,1))
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
}

void
IIOP::unmarshalMultiComponentProfile(const IOP::TaggedProfile& profile,
				     IOP::MultipleComponentProfile& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_MULTIPLE_COMPONENTS);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  CORBA::ULong total;
  total <<= s;
  if (total) {
    if (!s.checkInputOverrun(1,total))
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    body.length(total);
    for (CORBA::ULong index = 0; index<total; index++) {
      body[index] <<= s;
    }
  }
  // Check if the profile body ends here.
  if (s.checkInputOverrun(1,1))
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
}


void
IIOP::unmarshalObjectKey(const IOP::TaggedProfile& profile,
			 _CORBA_Unbounded_Sequence_Octet& key)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_INTERNET_IOP);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  CORBA::ULong len;
  CORBA::UShort port;

  // skip version
  s.skipInput(2);

  // skip address & port
  len <<= s;
  s.skipInput(len);
  port <<= s;

  len <<= s; // Get object key length
  CORBA::Octet* p = (CORBA::Octet*)((omni::ptr_arith_t)s.bufPtr() +
				    s.currentInputPtr());
  key.replace(len,len,p,0);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent& c , omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ORB_TYPE);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  CORBA::ULong v;
  v <<= e;
  ior.getIORInfo()->orbType(v);
}


char*
omniIOR::dump_TAG_ORB_TYPE(const IOP::TaggedComponent& c)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ORB_TYPE);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  CORBA::ULong orb_type;
  orb_type <<= e;

  CORBA::String_var outstr;
  CORBA::ULong len = sizeof("TAG_ORB_TYPE") + 1;
  if (orb_type == omniORB_TAG_ORB_TYPE) {
    len += sizeof("omniORB");
    outstr = CORBA::string_alloc(len);
    strcpy(outstr,"TAG_ORB_TYPE omniORB");
  }
  else {
    len += 16;
    outstr = CORBA::string_alloc(len);
    sprintf(outstr,"%s 0x%08lx","TAG_ORB_TYPE",orb_type);
  }
  return outstr._retn();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static
giopAddress* unmarshal_IIOP_Address(const IIOP::Address& addr) {

  const char* format = "giop:tcp:%s:%d";

  CORBA::ULong len = strlen(addr.host);
  if (len == 0) return 0;
  len += strlen(format) + 6;
  CORBA::String_var addrstr;
  addrstr = CORBA::string_alloc(len);
  sprintf(addrstr,format,(const char*)addr.host,addr.port);
  return giopAddress::str2Address(addrstr);
}


void
omniIOR::unmarshal_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent& c , omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  IIOP::Address v;
  v.host = e.unmarshalRawString();
  v.port <<= e;
  giopAddress* address = unmarshal_IIOP_Address(v);
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}


char*
omniIOR::dump_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent& c)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  IIOP::Address v;
  v.host = e.unmarshalRawString();
  v.port <<= e;
  CORBA::String_var outstr;
  CORBA::ULong len = sizeof("TAG_ALTERNATE_IIOP_ADDRESS ")+strlen(v.host)+8;
  outstr = CORBA::string_alloc(len);
  sprintf(outstr,"%s %s %d","TAG_ALTERNATE_IIOP_ADDRESS",(const char*)v.host,
	  v.port);
  return outstr._retn();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_GROUP(const IOP::TaggedComponent& c , omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_GROUP);

#if 0
  // XXX Temporarily disabled.
  ior.pd_is_IOGR = 1;
#endif

#if 0
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  GIOP::Version ftsvc_ver;
  CORBA::FT::FTDomainId ftdom_id;
  CORBA::FT::ObjectGroupId objgrp_id;
  CORBA::FT::ObjectGroupRefVersion objgrp_ver;
  ftsvc_ver <<= e;
  ftdom_id <<= e;
  objgrp_id <<= e;
  objgrp_ver <<= e;
  // XXX STORE IN EXTRA INFO PART OF IOR
#endif
}


char*
omniIOR::dump_TAG_GROUP(const IOP::TaggedComponent& c)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_GROUP);

#if 0
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  GIOP::Version ftsvc_ver;
  CORBA::FT::FTDomainId ftdom_id;
  CORBA::FT::ObjectGroupId objgrp_id;
  CORBA::FT::ObjectGroupRefVersion objgrp_ver;
  ftsvc_ver <<= e;
  ftdom_id <<= e;
  objgrp_id <<= e;
  objgrp_ver <<= e;
  // XXX Format everything into a string
#endif
  CORBA::String_var outstr((const char*)"TAG_GROUP");
  return outstr._retn();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent& c ,
				     omniIOR& ior) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_SSL_SEC_TRANS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::UShort target_supports,target_requires, port;

  try {
    switch (c.component_data.length()) {
      // Remember this is an encapsulation, so the length includes the
      // first endian octet plus the necessary paddings after it
    case 8:
      {
	// This is the standard format
	target_supports <<= e;
	target_requires <<= e;
	port <<= e;
	break;
      }
    default:
      {

	omniORB::logs(1," decode TAG_SSL_SEC_TRANS "
		      "WARNING: Wrong component size, attempt to decode it as the Visibroker non-compilant format");
	CORBA::ULong v;
	v <<= e; target_supports = v;
	v <<= e; target_requires = v;
	port <<= e;
	break;
      }
    }
  }
  catch (...) {
    omniORB::logs(1," decode TAG_SSL_SEC_TRANS "
		  "WARNING: fail to decode the component. The format neither conforms to the standard or is visibroker proprietory.");
    return;
  }

  giopAddressList& addresses = ior.getIORInfo()->addresses();
  // The first address in the list is the host port combo stored in the
  // IOR's address field. We have to copy the host name from there.
  const char* tcpaddr = 0;
  giopAddressList::iterator i, last;
  i    = addresses.begin();
  last = addresses.end();
  for (; i != last; i++) {
    if (omni::strMatch((*i)->type(),"giop:tcp")) {
      tcpaddr = (*i)->address();
      break;
    }
  }
  if (tcpaddr == 0) return;

  const char* host = strchr(tcpaddr,':') + 1; host = strchr(host,':') + 1;
  CORBA::ULong hostlen = strchr(host,':') - host;
  CORBA::String_var copyhost(CORBA::string_alloc(hostlen));
  strncpy(copyhost,host,hostlen);
  copyhost[hostlen] = '\0';

  const char* format = "giop:ssl:%s:%d";
  CORBA::String_var addrstr(CORBA::string_alloc(strlen(format)+hostlen+6));
  sprintf(addrstr,format,(const char*)copyhost,port);

  giopAddress* address = giopAddress::str2Address(addrstr);
  // If we do not have ssl transport linked the return value will be 0
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}

char*
omniIOR::dump_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent& c) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_SSL_SEC_TRANS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::UShort target_supports,target_requires, port;
  CORBA::Boolean is_visi = 0;

  CORBA::String_var outstr;

  try {
    switch (c.component_data.length()) {
      // Remember this is an encapsulation, so the length includes the
      // first endian octet plus the necessary paddings after it
    case 8:
      {
	// This is the standard format
	target_supports <<= e;
	target_requires <<= e;
	port <<= e;
	break;
      }
    default:
      {
	// Try visibroker propriety format
	CORBA::ULong v;
	v <<= e; target_supports = v;
	v <<= e; target_requires = v;
	port <<= e;
	is_visi = 1;
	break;
      }
    }
    const char* format = "TAG_SSL_SEC_TRANS port = %d supports = %d requires = %d";
    const char* visiformat = " (visibroker format)";
    outstr = CORBA::string_alloc(strlen(format)+strlen(visiformat)+36);
    sprintf(outstr,format,port,target_supports,target_requires);
    if (is_visi) strcat(outstr,visiformat);
  }
  catch (...) {
    outstr = (const char*)"TAG_SSL_SEC_TRANS (non-standard and unknown format)";
  }
  return outstr._retn();
}


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// For the TAGs that the ORB will look at, add a handler to the following
// table.
//
static struct {
  IOP::ComponentId id;
  void (*fn)(const IOP::TaggedComponent&,omniIOR&);
  char* (*dump)(const IOP::TaggedComponent&);
} componentUnmarshalHandlers[] = {
  // This table must be arranged in ascending order of IOP::ComponentId

  { IOP::TAG_ORB_TYPE,
    omniIOR::unmarshal_TAG_ORB_TYPE,
    omniIOR::dump_TAG_ORB_TYPE },

  { IOP::TAG_CODE_SETS,
    omniIOR::unmarshal_TAG_CODE_SETS,
    omniIOR::dump_TAG_CODE_SETS },

  { IOP::TAG_POLICIES, 0, 0 },

  { IOP::TAG_ALTERNATE_IIOP_ADDRESS,
    omniIOR::unmarshal_TAG_ALTERNATE_IIOP_ADDRESS,
    omniIOR::dump_TAG_ALTERNATE_IIOP_ADDRESS },

  { IOP::TAG_COMPLETE_OBJECT_KEY, 0, 0 },
  { IOP::TAG_ENDPOINT_ID_POSITION, 0, 0 },
  { IOP::TAG_LOCATION_POLICY, 0, 0 },
  { IOP::TAG_ASSOCIATION_OPTIONS, 0, 0 },
  { IOP::TAG_SEC_NAME, 0, 0 },
  { IOP::TAG_SPKM_1_SEC_MECH, 0, 0 },
  { IOP::TAG_SPKM_2_SEC_MECH, 0, 0 },
  { IOP::TAG_KERBEROSV5_SEC_MECH, 0, 0 },
  { IOP::TAG_CSI_ECMA_SECRET_SEC_MECH, 0, 0 },
  { IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH, 0, 0 },

  { IOP::TAG_SSL_SEC_TRANS,
    omniIOR::unmarshal_TAG_SSL_SEC_TRANS,
    omniIOR::dump_TAG_SSL_SEC_TRANS },

  { IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH, 0, 0 },
  { IOP::TAG_GENERIC_SEC_MECH, 0, 0 },
  { IOP::TAG_FIREWALL_TRANS, 0, 0 },
  { IOP::TAG_SCCP_CONTACT_INFO, 0, 0 },
  { IOP::TAG_JAVA_CODEBASE, 0, 0 },
  { IOP::TAG_DCE_STRING_BINDING, 0, 0 },
  { IOP::TAG_DCE_BINDING_NAME, 0, 0 },
  { IOP::TAG_DCE_NO_PIPES, 0, 0 },
  { IOP::TAG_DCE_SEC_MECH, 0, 0 },
  { IOP::TAG_INET_SEC_TRANS, 0, 0 },

  { IOP::TAG_GROUP,
    omniIOR::unmarshal_TAG_GROUP,
    omniIOR::dump_TAG_GROUP  },

  { IOP::TAG_PRIMARY, 0, 0 },
  { IOP::TAG_HEARTBEAT_ENABLED, 0, 0 },
  { 0xffffffff, 0, 0 }
};

static int tablesize = 0;

OMNI_NAMESPACE_END(omni)

/////////////////////////////////////////////////////////////////////////////
char*
IOP::dumpComponent(const IOP::TaggedComponent& c) {

  if (!tablesize) {
    while (componentUnmarshalHandlers[tablesize].id != 0xffffffff) tablesize++;
  }

  int top = tablesize;
  int bottom = 0;

  do {
    int i = (top + bottom) >> 1;
    IOP::ComponentId id = componentUnmarshalHandlers[i].id;
    if (id == c.tag) {
      if (componentUnmarshalHandlers[i].dump) {
	return componentUnmarshalHandlers[i].dump(c);
      }
      break;
    }
    else if (id > c.tag) {
      top = i;
    }
    else {
      bottom = i + 1;
    }
  } while (top != bottom);

  // Reach here if we don't know how to dump the content.
  CORBA::ULong len = c.component_data.length() * 2 + 4;
  const char* tagname = IOP::ComponentIDtoName(c.tag);
  if (!tagname) {
    len += sizeof("unknown tag()") + 10;
  }
  else {
    len += strlen(tagname);
  }
  CORBA::String_var outstr;
  char* p;
  outstr = p = CORBA::string_alloc(len);
  memset(p,0,len+1);

  if (tagname) {
    strcpy(p,tagname);
  }
  else {
    sprintf(p,"unknown tag(0x%08lx)",c.tag);
  }
  p += strlen(p);
  *p++ = ' ';
  *p++ = '0';
  *p++ = 'x';

  CORBA::Char* data = (CORBA::Char *) c.component_data.get_buffer();

  for (CORBA::ULong i=0; i < c.component_data.length(); i++) {
    int v = (data[i] & 0xf0);
    v = v >> 4;
    if (v < 10)
      *p++ = '0' + v;
    else
      *p++ = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      *p++ = '0' + v;
    else
      *p++ = 'a' + (v - 10);
  }

  return outstr._retn();
}


/////////////////////////////////////////////////////////////////////////////
//            Default interceptors                                         //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

static IIOP::Address                   my_address;
static _CORBA_Unbounded_Sequence_Octet my_code_set;
static _CORBA_Unbounded_Sequence_Octet my_orb_type;
static _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> my_alternative_addr;
static _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> my_ssl_addr;


OMNI_NAMESPACE_END(omni)

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_IIOP_ADDRESS(const IIOP::Address& address) {
  if (my_address.port == 0) {
    my_address = address;
  }
  else {
    add_TAG_ALTERNATE_IIOP_ADDRESS(address);
  }
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_CODE_SETS(const CONV_FRAME::CodeSetComponentInfo& info) {

  cdrEncapsulationStream s(CORBA::ULong(0),1);
  info >>= s;

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  my_code_set.replace(max,len,p,1);
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_ALTERNATE_IIOP_ADDRESS(const IIOP::Address& address) {

  cdrEncapsulationStream s(CORBA::ULong(0),1);
  s.marshalRawString(address.host);
  address.port >>= s;

  CORBA::ULong index = my_alternative_addr.length();
  my_alternative_addr.length(index+1);

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);

  my_alternative_addr[index].replace(max,len,p,1);
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_SSL_SEC_TRANS(const IIOP::Address& address,
			       CORBA::UShort supports,CORBA::UShort requires) {

  if (strlen(my_address.host) == 0) {
    my_address.host = address.host;
  }
  else if (strcmp(my_address.host,address.host) != 0) {
    // The address does not match. We cannot add this address to our IOR.
    // Shouldn't have happened!
    omniORB::logger log;
    log << "Warning: cannot add this SSL address ("
	<< address.host << "," << address.port
	<< ") to the IOR because the host name does not match my host name ("
	<< my_address.host << ")\n";
    return;
  }

  cdrEncapsulationStream s(CORBA::ULong(0),1);
  supports >>= s;
  requires >>= s;
  address.port >>= s;

  CORBA::ULong index = my_ssl_addr.length();
  my_ssl_addr.length(index+1);

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);

  my_ssl_addr[index].replace(max,len,p,1);
}

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean insertSupportedComponents(omniInterceptors::encodeIOR_T::info_T& info)
{
  const GIOP::Version& v = info.iiop.version;
  IOP::MultipleComponentProfile& cs = info.iiop.components;

  if (strlen(info.iiop.address.host) == 0) {
    if (strlen(my_address.host) == 0) {
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
    }
    info.iiop.address = my_address;
  }

  if ((v.major > 1 || v.minor >= 1) && my_orb_type.length()) {
    // 1.1 or later, Insert ORB TYPE
    IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
    c.tag = IOP::TAG_ORB_TYPE;
    CORBA::ULong max, len;
    max = my_orb_type.maximum();
    len = my_orb_type.length();
    c.component_data.replace(max,len,my_orb_type.get_buffer(),0);
  }

  if ((v.major > 1 || v.minor >= 2) && my_code_set.length()) {
    // 1.2 or later, Insert CODE SET
    IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
    c.tag = IOP::TAG_CODE_SETS;
    CORBA::ULong max, len;
    max = my_code_set.maximum();
    len = my_code_set.length();
    c.component_data.replace(max,len,my_code_set.get_buffer(),0);
  }

  if (v.major > 1 || v.minor >= 2) {
    // 1.2 or later, Insert ALTERNATIVE IIOP ADDRESS
    for (CORBA::ULong index = 0;
	 index < my_alternative_addr.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;
      CORBA::ULong max, len;
      max = my_alternative_addr[index].maximum();
      len = my_alternative_addr[index].length();
      c.component_data.replace(max,len,
			       my_alternative_addr[index].get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, Insert SSL_SEC_TRANS
    for (CORBA::ULong index = 0;
	 index < my_ssl_addr.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_SSL_SEC_TRANS;
      CORBA::ULong max, len;
      max = my_ssl_addr[index].maximum();
      len = my_ssl_addr[index].length();
      c.component_data.replace(max,len,
			       my_ssl_addr[index].get_buffer(),0);
    }
  }

  return (info.default_only ? 0 : 1);
}

/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean extractSupportedComponents(omniInterceptors::decodeIOR_T::info_T& info)
{
  if (!info.has_iiop_body) return 1;

  omniIOR::IORInfo& iorInfo = *(info.ior.getIORInfo());

  iorInfo.version(info.iiop.version);

  giopAddress* address = unmarshal_IIOP_Address(info.iiop.address);
  if (address)
    iorInfo.addresses().push_back(address);

  if (!tablesize) {
    while (componentUnmarshalHandlers[tablesize].id != 0xffffffff) tablesize++;
  }

  const IOP::MultipleComponentProfile& components = info.iiop.components;

  CORBA::ULong total = components.length();
  for (CORBA::ULong index = 0; index < total; index++) {

    int top = tablesize;
    int bottom = 0;

    do {
      int i = (top + bottom) >> 1;
      IOP::ComponentId id = componentUnmarshalHandlers[i].id;
      if (id == components[index].tag) {
	if (componentUnmarshalHandlers[i].fn) {
	  componentUnmarshalHandlers[i].fn(components[index],info.ior);
	}
	break;
      }
      else if (id > components[index].tag) {
	top = i;
      }
      else {
	bottom = i + 1;
      }
    } while (top != bottom);
  }
  return 1;
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_ior_initialiser : public omniInitialiser {
public:
  omni_ior_initialiser() {}

  void attach() {
    my_address.port = 0;
    omniORB::getInterceptors()->encodeIOR.add(insertSupportedComponents);
    omniORB::getInterceptors()->decodeIOR.add(extractSupportedComponents);

    cdrEncapsulationStream s(8,1);
    omniORB_TAG_ORB_TYPE >>= s;
    _CORBA_Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
    my_orb_type.replace(max,len,p,1);

  }

  void detach() {
    omniORB::getInterceptors()->encodeIOR.remove(insertSupportedComponents);
    omniORB::getInterceptors()->decodeIOR.remove(extractSupportedComponents);
    _CORBA_Unbounded_Sequence_Octet::freebuf(my_orb_type.get_buffer(1));
  }

};

static omni_ior_initialiser initialiser;

omniInitialiser& omni_ior_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
