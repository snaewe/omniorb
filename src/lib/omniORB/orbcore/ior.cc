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
  Revision 1.12.2.6  2007/11/28 12:24:26  dgrisby
  Implement a tiny subset of CSIv2 to permit multiple SSL endpoints in IORs.

  Revision 1.12.2.5  2006/09/17 23:24:18  dgrisby
  Remove hard-coded hostname length.

  Revision 1.12.2.4  2006/05/21 17:44:04  dgrisby
  Remove unnecessary uses of cdrCountingStream.

  Revision 1.12.2.3  2006/04/09 19:52:31  dgrisby
  More IPv6, endPointPublish parameter.

  Revision 1.12.2.2  2005/01/06 23:10:30  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.12.2.1  2003/03/23 21:02:13  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.10.2.23  2003/02/03 16:53:14  dgrisby
  Force type in constructor argument to help confused compilers.

  Revision 1.10.2.22  2003/01/14 14:18:12  dgrisby
  Don't overwrite component data when decoding multi component profile.

  Revision 1.10.2.21  2002/08/16 16:00:50  dgrisby
  Bugs accessing uninitialised String_vars with [].

  Revision 1.10.2.20  2002/03/18 12:38:26  dpg1
  Lower trace(0) to trace(1), propagate fatalException.

  Revision 1.10.2.19  2002/03/14 14:39:44  dpg1
  Obscure bug in objref creation with unaligned buffers.

  Revision 1.10.2.18  2002/01/02 18:17:00  dpg1
  Relax IOR strictness when strictIIOP not set.

  Revision 1.10.2.17  2001/08/21 10:50:46  dpg1
  Incorrect length calculation if no components.

  Revision 1.10.2.16  2001/08/17 17:12:39  sll
  Modularise ORB configuration parameters.

  Revision 1.10.2.15  2001/08/06 15:49:17  sll
  Added IOP component TAG_OMNIORB_UNIX_TRANS for omniORB specific local
  transport using the unix domain socket.

  Revision 1.10.2.14  2001/08/03 17:41:22  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.10.2.13  2001/07/31 16:27:59  sll
  Added GIOP BiDir support.

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
#include <omniORB4/omniURI.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <giopBiDir.h>
#include <SocketCollection.h>
#include <orbParameters.h>
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
    OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)s.completion());

  switch (idlen) {

  case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)s.completion());
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
      OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		    (CORBA::CompletionStatus)s.completion());
    idlen = 0;
    break;

  default:
    id = CORBA::string_alloc(idlen);
    s.get_octet_array((CORBA::Octet*)((const char*)id), idlen);
    if( ((char*)id)[idlen - 1] != '\0' )
      OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		    (CORBA::CompletionStatus)s.completion());
    break;
  }

  return id._retn();
}

void
IIOP::Address::operator>>= (cdrStream& s) const {
  s.marshalRawString(host);
  port >>= s;
}

void
IIOP::Address::operator<<= (cdrStream& s) {
  host = s.unmarshalRawString();
  port <<= s;
}

void
IIOP::encodeProfile(const IIOP::ProfileBody& body,IOP::TaggedProfile& profile)
{
  profile.tag = IOP::TAG_INTERNET_IOP;

  {
    cdrEncapsulationStream s((CORBA::ULong)0,1);
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

  {
    cdrEncapsulationStream s((CORBA::ULong)0,1);
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
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

  body.address.host = s.unmarshalRawString();
  body.address.port <<= s;
  body.object_key <<= s;

  if (body.version.minor > 0) {
    CORBA::ULong total;
    total <<= s;
    if (total) {
      if (!s.checkInputOverrun(1,total))
	OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
      body.components.length(total);
      for (CORBA::ULong index=0; index<total; index++) {
	body.components[index] <<= s;
      }
    }
  }

  // Check that the profile body ends here.
  if (s.checkInputOverrun(1,1)) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(10, "IIOP Profile has garbage at end");
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    }
    else
      omniORB::logs(1, "Warning: IIOP Profile has garbage at end. Ignoring.");
  }
}

void
IIOP::unmarshalMultiComponentProfile(const IOP::TaggedProfile& profile,
				     IOP::MultipleComponentProfile& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_MULTIPLE_COMPONENTS);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1);

  CORBA::ULong newitems;
  newitems <<= s;
  if (newitems) {
    if (!s.checkInputOverrun(1,newitems))
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);

    CORBA::ULong oldlen = body.length();
    body.length(oldlen + newitems);
    for (CORBA::ULong index = oldlen; index < oldlen+newitems; index++) {
      body[index] <<= s;
    }
  }
  // Check that the profile body ends here.
  if (s.checkInputOverrun(1,1)) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(10, "Multi-component profile has garbage at end");
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
    }
    else
      omniORB::logs(1, "Warning: Multi-component profile has "
		    "garbage at end. Ignoring.");
  }
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

  if (s.readOnly()) {
    CORBA::Octet* p = (CORBA::Octet*)((omni::ptr_arith_t)s.bufPtr() +
				      s.currentInputPtr());
    key.replace(len,len,p,0);
  }
  else {
    // If the cdrEncapsulationStream had to copy the profile data, we
    // have to copy it _again_ here, otherwise it will be out of scope
    // before the key is used.
    key.length(len);
    s.get_octet_array(key.NP_data(), len);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent& c, omniIOR& ior)
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
    sprintf(outstr,"%s 0x%08lx","TAG_ORB_TYPE",(unsigned long)orb_type);
  }
  return outstr._retn();
}


void
omniIOR::unmarshal_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent& c, omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ALTERNATE_IIOP_ADDRESS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  IIOP::Address v;
  v.host = e.unmarshalRawString();
  v.port <<= e;
  giopAddress* address = giopAddress::fromTcpAddress(v);
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
omniIOR::unmarshal_TAG_GROUP(const IOP::TaggedComponent& c, omniIOR& ior)
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
omniIOR::unmarshal_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent& c,
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

  CORBA::UShort     tcp_port;
  CORBA::String_var tcp_host = omniURI::extractHostPort(tcpaddr+9, tcp_port);

  IIOP::Address ssladdr;
  ssladdr.host = tcp_host._retn();
  ssladdr.port = port;
  giopAddress* address = giopAddress::fromSslAddress(ssladdr);
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


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void
omniIOR::unmarshal_TAG_CSI_SEC_MECH_LIST(const IOP::TaggedComponent& c,
					 omniIOR& ior) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_CSI_SEC_MECH_LIST);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::Boolean stateful = e.unmarshalBoolean();

  CORBA::ULong mech_count;
  mech_count <<= e;

  for (CORBA::ULong mech_idx = 0; mech_idx != mech_count; ++mech_idx) {
    CORBA::UShort target_requires;

    CORBA::UShort as_target_supports, as_target_requires;
    _CORBA_Unbounded_Sequence_Octet as_client_authentication_mech;
    _CORBA_Unbounded_Sequence_Octet as_target_name;

    CORBA::UShort sas_target_supports, sas_target_requires;
    CORBA::ULong sas_privilege_authorities_len;
    _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> sas_supported_naming_mechanisms;
    CORBA::ULong sas_supported_identity_types;

    // CompoundSecMech structure
    target_requires <<= e;

    IOP::TaggedComponent transport_mech;
    transport_mech <<= e;

    // as_context_mech member
    as_target_supports <<= e;
    as_target_requires <<= e;
    as_client_authentication_mech <<= e;
    as_target_name <<= e;

    // sas_context_mech member
    sas_target_supports <<= e;
    sas_target_requires <<= e;
    sas_privilege_authorities_len <<= e;
    for (CORBA::ULong pi = 0; pi != sas_privilege_authorities_len; ++pi) {
      CORBA::ULong syntax;
      _CORBA_Unbounded_Sequence_Octet name;
      
      syntax <<= e;
      name   <<= e;
    }
    sas_supported_naming_mechanisms <<= e;
    sas_supported_identity_types <<= e;

    if (as_target_requires  == 0 &&
	sas_target_requires == 0 &&
	transport_mech.tag  == IOP::TAG_TLS_SEC_TRANS) {

      // No higher-level requirements and a TLS transport tag -- we
      // can support this component.
      CORBA::UShort tls_target_supports, tls_target_requires;
      CORBA::ULong addresses_len;

      cdrEncapsulationStream tls_e(transport_mech.component_data.get_buffer(),
				   transport_mech.component_data.length(),1);
      
      tls_target_supports <<= tls_e;
      tls_target_requires <<= tls_e;
      addresses_len <<= tls_e;

      for (CORBA::ULong ai = 0; ai != addresses_len; ++ai) {
	IIOP::Address ssladdr;

	ssladdr.host = tls_e.unmarshalRawString();
	ssladdr.port <<= tls_e;

	giopAddress* address = giopAddress::fromSslAddress(ssladdr);
	// If we do not have ssl transport linked the return value will be 0

	if (address == 0) return;
	ior.getIORInfo()->addresses().push_back(address);
      }
    }
  }
}

char*
omniIOR::dump_TAG_CSI_SEC_MECH_LIST(const IOP::TaggedComponent& c) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_CSI_SEC_MECH_LIST);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  _CORBA_Unbounded_Sequence_String addrs;
  CORBA::ULong addrs_size = 0;

  CORBA::Boolean stateful = e.unmarshalBoolean();

  CORBA::ULong mech_count;
  mech_count <<= e;

  for (CORBA::ULong mech_idx = 0; mech_idx != mech_count; ++mech_idx) {
    CORBA::UShort target_requires;

    CORBA::UShort as_target_supports, as_target_requires;
    _CORBA_Unbounded_Sequence_Octet as_client_authentication_mech;
    _CORBA_Unbounded_Sequence_Octet as_target_name;

    CORBA::UShort sas_target_supports, sas_target_requires;
    CORBA::ULong sas_privilege_authorities_len;
    _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> sas_supported_naming_mechanisms;
    CORBA::ULong sas_supported_identity_types;

    // CompoundSecMech structure
    target_requires <<= e;

    IOP::TaggedComponent transport_mech;
    transport_mech <<= e;

    // as_context_mech member
    as_target_supports <<= e;
    as_target_requires <<= e;
    as_client_authentication_mech <<= e;
    as_target_name <<= e;

    // sas_context_mech member
    sas_target_supports <<= e;
    sas_target_requires <<= e;
    sas_privilege_authorities_len <<= e;
    for (CORBA::ULong pi = 0; pi != sas_privilege_authorities_len; ++pi) {
      CORBA::ULong syntax;
      _CORBA_Unbounded_Sequence_Octet name;
      
      syntax <<= e;
      name   <<= e;
    }
    sas_supported_naming_mechanisms <<= e;
    sas_supported_identity_types <<= e;

    if (as_target_requires  == 0 &&
	sas_target_requires == 0 &&
	transport_mech.tag  == IOP::TAG_TLS_SEC_TRANS) {

      // No higher-level requirements and a TLS transport tag -- we
      // can support this component.
      CORBA::UShort tls_target_supports, tls_target_requires;
      CORBA::ULong addresses_len;

      cdrEncapsulationStream tls_e(transport_mech.component_data.get_buffer(),
				   transport_mech.component_data.length(),1);
      
      tls_target_supports <<= tls_e;
      tls_target_requires <<= tls_e;
      addresses_len <<= tls_e;

      for (CORBA::ULong ai = 0; ai != addresses_len; ++ai) {
	IIOP::Address ssladdr;

	ssladdr.host = tls_e.unmarshalRawString();
	ssladdr.port <<= tls_e;

	char* addr = omniURI::buildURI("", ssladdr.host, ssladdr.port);

	addrs_size += strlen(addr);

	CORBA::ULong addrslen = addrs.length();
	addrs.length(addrslen+1);
	addrs[addrslen] = addr;
      }
    }
  }

  if (addrs.length()) {
    const char* prefix = "TAG_CSI_SEC_MECH_LIST endpoints ";

    CORBA::String_var outstr = CORBA::string_alloc(strlen(prefix) +
						   addrs_size +
						   addrs.length() * 2);
    strcpy(outstr, prefix);
    for (CORBA::ULong i=0; i != addrs.length(); ++i) {
      strcat(outstr, addrs[i]);
      if (i + 1 != addrs.length())
	strcat(outstr, ", ");
    }
    return outstr._retn();
  }
  else {
    return CORBA::string_dup("TAG_CSI_SEC_MECH_LIST (no usable endpoints)");
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_BIDIR(const IOP::TaggedComponent& c,
				     omniIOR& ior) {
  
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_BIDIR);
  OMNIORB_ASSERT(ior.pd_iorInfo);

  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  char* sendfrom = e.unmarshalRawString();

  BiDirInfo* info = new BiDirInfo(sendfrom);
  
  omniIOR::IORExtraInfoList& infolist = ior.pd_iorInfo->extraInfo();
  CORBA::ULong index = infolist.length();
  infolist.length(index+1);
  infolist[index] = (omniIOR::IORExtraInfo*)info;
}

char*
omniIOR::dump_TAG_OMNIORB_BIDIR(const IOP::TaggedComponent& c) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_BIDIR);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::String_var sendfrom;
  sendfrom = e.unmarshalRawString();
  CORBA::String_var outstr;
  CORBA::ULong len = sizeof("TAG_OMNIORB_BIDIR ")+strlen(sendfrom);
  outstr = CORBA::string_alloc(len);
  sprintf(outstr,"%s %s","TAG_OMNIORB_BIDIR",(const char*)sendfrom);
  return outstr._retn();
}

void
omniIOR::add_TAG_OMNIORB_BIDIR(const char* sendfrom,omniIOR& ior) {

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  s.marshalRawString(sendfrom);

  IOP::MultipleComponentProfile body;
  body.length(1);
  body[0].tag = IOP::TAG_OMNIORB_BIDIR;
  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  body[0].component_data.replace(max,len,p,1);

  CORBA::ULong index = ior.pd_iopProfiles->length();
  ior.pd_iopProfiles->length(index+1);
  IIOP::encodeMultiComponentProfile(body,ior.pd_iopProfiles[index]);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_UNIX_TRANS(const IOP::TaggedComponent& c,
					  omniIOR& ior) {
  
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_UNIX_TRANS);
  OMNIORB_ASSERT(ior.pd_iorInfo);

  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::String_var host;
  host = e.unmarshalRawString();
  CORBA::String_var filename;
  filename = e.unmarshalRawString();

  // Check if we are on the same host and hence can use unix socket.
  char self[OMNIORB_HOSTNAME_MAX];
  if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
    self[0] = '\0';
    omniORB::logs(1, "Cannot get the name of this host.");
  }
  if (strcmp(self,host) != 0) return;

  const char* format = "giop:unix:%s";

  CORBA::ULong len = strlen(filename);
  if (len == 0) return;
  len += strlen(format);
  CORBA::String_var addrstr(CORBA::string_alloc(len));
  sprintf(addrstr,format,(const char*)filename);
  
  giopAddress* address = giopAddress::str2Address(addrstr);
  // If we do not have unix transport linked the return value will be 0
  if (address == 0) return;
  ior.getIORInfo()->addresses().push_back(address);
}

char*
omniIOR::dump_TAG_OMNIORB_UNIX_TRANS(const IOP::TaggedComponent& c) {

  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_UNIX_TRANS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CORBA::String_var host;
  host = e.unmarshalRawString();
  CORBA::String_var filename;
  filename = e.unmarshalRawString();

  const char* format = "TAG_OMNIORB_UNIX_TRANS %s %s";
  CORBA::String_var outstr;
  CORBA::ULong len = strlen(format) + strlen(host) + strlen(filename);
  outstr = CORBA::string_alloc(len);
  sprintf(outstr,format,(const char*)host,(const char*)filename);
  return outstr._retn();

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_OMNIORB_PERSISTENT_ID(const IOP::TaggedComponent& c,
					     omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_PERSISTENT_ID);

  CORBA::ULong len = orbParameters::persistentId.length();

  if (len && len == c.component_data.length()) {

    const CORBA::Octet* a = c.component_data.get_buffer();
    const CORBA::Octet* b = orbParameters::persistentId.get_buffer();
    for (CORBA::ULong i=0; i < len; i++) {
      if (*a++ != *b++)
	return;
    }
    
    omniIOR::IORExtraInfoList& extra = ior.pd_iorInfo->extraInfo();
    CORBA::ULong index = extra.length();
    extra.length(index+1);
    extra[index] = new IORExtraInfo(IOP::TAG_OMNIORB_PERSISTENT_ID);
  }
}


char*
omniIOR::dump_TAG_OMNIORB_PERSISTENT_ID(const IOP::TaggedComponent& c)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_OMNIORB_PERSISTENT_ID);

  const char* prefix = "TAG_OMNIORB_PERSISTENT_ID ";

  CORBA::String_var outstr;

  CORBA::ULong prefix_len = strlen(prefix);
  outstr = CORBA::string_alloc(prefix_len + c.component_data.length() * 2);

  strcpy((char*)outstr, prefix);

  char* o = (char*)outstr + prefix_len;

  int d, n;
  for (CORBA::ULong i=0; i < c.component_data.length(); i++) {
    d = c.component_data[i];
    n = (d & 0xf0) >> 4;
    if (n >= 10)
      *o++ = 'a' + n - 10;
    else
      *o++ = '0' + n;

    n = d & 0xf;
    if (n >= 10)
      *o++ = 'a' + n - 10;
    else
      *o++ = '0' + n;
  }
  *o = '\0';

  return outstr._retn();
}

static void
logPersistentIdentifier()
{
  omniORB::logger l;
  l << "Persistent server identifier: ";

  int c, n;
  for (CORBA::ULong i=0; i < orbParameters::persistentId.length(); i++) {
    c = orbParameters::persistentId[i];
    n = (c & 0xf0) >> 4;
    if (n >= 10)
      l << (char)('a' + n - 10);
    else
      l << (char)('0' + n);

    n = c & 0xf;
    if (n >= 10)
      l << (char)('a' + n - 10);
    else
      l << (char)('0' + n);
  }
  l << "\n";
}  


void
omniORB::
setPersistentServerIdentifier(const _CORBA_Unbounded_Sequence_Octet& id)
{
  if (orbParameters::persistentId.length()) {
    // Once set, it must not be changed
    OMNIORB_THROW(INITIALIZE, INITIALIZE_FailedLoadLibrary,
		  CORBA::COMPLETED_NO);
  }

  orbParameters::persistentId = id;

  if (omniORB::trace(10)) {
    logPersistentIdentifier();
  }
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

  { IOP::TAG_CSI_SEC_MECH_LIST,
    omniIOR::unmarshal_TAG_CSI_SEC_MECH_LIST,
    omniIOR::dump_TAG_CSI_SEC_MECH_LIST },

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

  { IOP::TAG_OMNIORB_BIDIR,
    omniIOR::unmarshal_TAG_OMNIORB_BIDIR,
    omniIOR::dump_TAG_OMNIORB_BIDIR },

  { IOP::TAG_OMNIORB_UNIX_TRANS,
    omniIOR::unmarshal_TAG_OMNIORB_UNIX_TRANS,
    omniIOR::dump_TAG_OMNIORB_UNIX_TRANS },

  { IOP::TAG_OMNIORB_PERSISTENT_ID,
    omniIOR::unmarshal_TAG_OMNIORB_PERSISTENT_ID,
    omniIOR::dump_TAG_OMNIORB_PERSISTENT_ID },

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
    sprintf(p,"unknown tag(0x%08lx)",(unsigned long)c.tag);
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
static _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> my_unix_addr;

static _CORBA_Unbounded_Sequence_Octet my_csi_component;
static _CORBA_Unbounded_Sequence<IIOP::Address> my_tls_addr_list;
static CORBA::UShort  my_tls_supports;
static CORBA::UShort  my_tls_requires;
static CORBA::Boolean my_csi_enabled;

_CORBA_Unbounded_Sequence_Octet orbParameters::persistentId;

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

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  info >>= s;

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  my_code_set.replace(max,len,p,1);
}

/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_ALTERNATE_IIOP_ADDRESS(const IIOP::Address& address) {

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
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

  {
    // Add to list of TLS addresses
    CORBA::ULong length = my_tls_addr_list.length();
    my_tls_addr_list.length(length+1);
    my_tls_addr_list[length] = address;
    my_tls_supports = supports;
    my_tls_requires = requires;
  }

  if (strlen(my_address.host) == 0) {
    my_address.host = address.host;
  }
  else if (strcmp(my_address.host,address.host) != 0) {
    // The address does not match the IIOP address. Cannot add as an
    // SSL address. Enable the minimal CSI support.
    my_csi_enabled = 1;
    return;
  }

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
  supports >>= s;
  requires >>= s;
  address.port >>= s;

  CORBA::ULong index = my_ssl_addr.length();
  my_ssl_addr.length(index+1);

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);

  my_ssl_addr[index].replace(max,len,p,1);
}

/////////////////////////////////////////////////////////////////////////////
static
void add_TAG_CSI_SEC_MECH_LIST(const _CORBA_Unbounded_Sequence<IIOP::Address>& addrs,
			       CORBA::UShort supports, CORBA::UShort requires)
{
  // Anyone would think this structure was designed by committee...

  if (omniORB::trace(10)) {
    omniORB::logger log;
    log << "Create CSIv2 security mechanism list for " << addrs.length()
	<< " addresses.\n";
  }

  cdrEncapsulationStream stream(CORBA::ULong(0),CORBA::Boolean(1));

  CORBA::UShort zeroUShort = 0;
  CORBA::ULong  zeroULong  = 0;

  // struct CompoundSecMechList {
  //   boolean stateful;
  //   CompoundSecMechanisms mechanism_list;
  // };
  stream.marshalBoolean(0);
  CORBA::ULong mechanism_count = 1;
  mechanism_count >>= stream;

  // struct CompoundSecMech {
  //   AssociationOptions taget_requires;
  //   IOP::TaggedComponent transport_mech;
  //   AS_ContextSec as_context_mech;
  //   SAS_ContextSec sas_context_mech;
  // };

  requires >>= stream;

  IOP::TaggedComponent transport_mech;
  transport_mech.tag = IOP::TAG_TLS_SEC_TRANS;

  cdrEncapsulationStream mech_stream(CORBA::ULong(0),CORBA::Boolean(1));

  supports >>= mech_stream;
  requires >>= mech_stream;
  addrs    >>= mech_stream;

  {
    CORBA::Octet* p;
    CORBA::ULong max, len;
    mech_stream.getOctetStream(p,max,len);
    transport_mech.component_data.replace(max, len, p, 1);
  }
  transport_mech >>= stream;

  // struct AS_ContextSec {
  //   AssociationOptions target_supports;
  //   AssociationOptions target_requires;
  //   CSI::OID client_authentication_mech;
  //   CSI::GSS_NT_ExportedName target_name;
  // };
  zeroUShort >>= stream;
  zeroUShort >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;

  // struct SAS_ContextSec {
  //   AssociationOptions target_supports;
  //   AssociationOptions target_requires;
  //   ServiceConfigurationList privilege_authorities;
  //   CSI::OIDList supported_naming_mechanisms;
  //   CSI::IdentityTokenType supported_identity_types;
  // };
  zeroUShort >>= stream;
  zeroUShort >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;
  zeroULong  >>= stream;

  {
    CORBA::Octet* p;
    CORBA::ULong max, len;
    stream.getOctetStream(p,max,len);

    _OMNI_NS(my_csi_component).replace(max, len, p, 1);
  }
}


/////////////////////////////////////////////////////////////////////////////
void
omniIOR::add_TAG_OMNIORB_UNIX_TRANS(const char* filename) {

  OMNIORB_ASSERT(filename && strlen(filename) != 0);

  char self[OMNIORB_HOSTNAME_MAX];
  if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
    omniORB::logs(1, "Cannot get the name of this host.");
    self[0] = '\0';
  }

  if (strlen(my_address.host) == 0) {
    my_address.host = (const char*) self;
  }

  cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));

  s.marshalRawString(self);
  s.marshalRawString(filename);

  CORBA::ULong index = my_unix_addr.length();
  my_unix_addr.length(index+1);

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);

  my_unix_addr[index].replace(max,len,p,1);
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
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
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

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, Insert CSI_SEC_MECH_LIST
    if (my_csi_enabled) {
      if (!my_csi_component.length()) {
	add_TAG_CSI_SEC_MECH_LIST(my_tls_addr_list,
				  my_tls_supports, my_tls_requires);
      }
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_CSI_SEC_MECH_LIST;

      CORBA::ULong max, len;
      max = my_csi_component.maximum();
      len = my_csi_component.length();
      c.component_data.replace(max,len,
			       my_csi_component.get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 2) {
    // 1.2 or later, Insert omniORB unix transport
    for (CORBA::ULong index = 0;
	 index < my_unix_addr.length(); index++) {

      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_OMNIORB_UNIX_TRANS;
      CORBA::ULong max, len;
      max = my_unix_addr[index].maximum();
      len = my_unix_addr[index].length();
      c.component_data.replace(max,len,
			       my_unix_addr[index].get_buffer(),0);
    }
  }

  if (v.major > 1 || v.minor >= 1) {
    // 1.1 or later, insert omniORB persistent id
    if (orbParameters::persistentId.length()) {
      IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(cs);
      c.tag = IOP::TAG_OMNIORB_PERSISTENT_ID;
      c.component_data.replace(orbParameters::persistentId.maximum(),
			       orbParameters::persistentId.length(),
			       orbParameters::persistentId.get_buffer(), 0);
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

  giopAddress* address = giopAddress::fromTcpAddress(info.iiop.address);
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

    if (omniORB::trace(10) && orbParameters::persistentId.length()) {
      logPersistentIdentifier();
    }

  }

  void detach() {
    omniORB::getInterceptors()->encodeIOR.remove(insertSupportedComponents);
    omniORB::getInterceptors()->decodeIOR.remove(extractSupportedComponents);
    _CORBA_Unbounded_Sequence_Octet::freebuf(my_orb_type.get_buffer(1));

    my_alternative_addr.length(0);
    my_ssl_addr.length(0);
    my_unix_addr.length(0);
    my_csi_component.length(0);
    my_tls_addr_list.length(0);
    my_csi_enabled = 0;
  }

};

static omni_ior_initialiser initialiser;

omniInitialiser& omni_ior_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
