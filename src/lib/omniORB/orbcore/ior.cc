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
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);

  switch (idlen) {

  case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
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
    id[0] = s.unmarshalOctet();
    if (((char*)id)[0] != '\0')
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    idlen = 0;
    break;
    
  default:
    id = CORBA::string_alloc(idlen);
    s.get_octet_array((CORBA::Octet*)((const char*)id), idlen);
    if( ((char*)id)[idlen - 1] != '\0' )
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    break;
  }

  return id._retn();
}


void
IIOP::addAlternativeIIOPAddress(IOP::MultipleComponentProfile& components,
				const IIOP::Address& addr)
{

  CORBA::ULong hlen = strlen(addr.host) + 1;
  cdrEncapsulationStream s(hlen+8,1);
  hlen >>= s;
  s.put_octet_array((const CORBA::Octet*)(const char*)addr.host,hlen);
  addr.port >>= s;

  _CORBA_Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);

  CORBA::ULong index = components.length();
  components.length(index+1);
  components[index].tag = IOP::TAG_ALTERNATE_IIOP_ADDRESS;
  components[index].component_data.replace(max,len,p,1);
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
    {
      CORBA::ULong hlen = strlen(body.address.host) + 1;
      hlen >>= s;
      s.put_octet_array((const CORBA::Octet*)
			(const char*)body.address.host,hlen);
    }
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
    {
      CORBA::ULong hlen = strlen(body.address.host) + 1;
      hlen >>= s;
      s.put_octet_array((const CORBA::Octet*)
			(const char*)body.address.host,hlen);
    }
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
IIOP::decodeProfile(const IOP::TaggedProfile& profile,
		    IIOP::ProfileBody& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_INTERNET_IOP);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1); 

  body.version.major = s.unmarshalOctet();
  body.version.minor = s.unmarshalOctet();

  if (body.version.major != 1) 
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  {
    // Don't use unmarshalString() to unmarshal the host address because
    // the profile is always encoded in ISO-8859-1 irrespective of the
    // TCS or NCS setting.
    CORBA::ULong idlen; 
    idlen <<= s;
    if (!s.checkInputOverrun(1,idlen))
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    body.address.host = CORBA::string_alloc(idlen);
    s.get_octet_array((CORBA::Octet*)((const char*)body.address.host), idlen);
    if( ((char*)body.address.host)[idlen - 1] != '\0' )
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }

  body.address.port <<= s;
  body.object_key <<= s;
  
  if (body.version.minor > 0) {
    CORBA::ULong total;
    total <<= s;
    if (total) {
      if (!s.checkInputOverrun(1,total))
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      body.components.length(total);
      for (CORBA::ULong index=0; index<total; index++) {
	body.components[index] <<= s;
      }
    }
  }
  // Check if the profile body ends here.
  if (s.checkInputOverrun(1,1))
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
}

void
IIOP::decodeMultiComponentProfile(const IOP::TaggedProfile& profile,
				  IIOP::ProfileBody& body)
{
  OMNIORB_ASSERT(profile.tag == IOP::TAG_MULTIPLE_COMPONENTS);

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1); 

  CORBA::ULong total;
  total <<= s;
  if (total) {
    if (!s.checkInputOverrun(1,total))
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    CORBA::ULong index = body.components.length();
    total += index;
    body.components.length(total);
    for ( ; index<total; index++) {
      body.components[index] <<= s;
    }
  }
  // Check if the profile body ends here.
  if (s.checkInputOverrun(1,1))
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
}


//////////////////////////////////////////////////////////////////////////
void
omniIOR::unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent& c , omniIOR* ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_ORB_TYPE);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);
  ior->orb_type <<= e;
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
// For the TAGs that the ORB will look at, add a handler to the following
// table. 
//
static struct {
  IOP::ComponentId id;
  void (*fn)(const IOP::TaggedComponent&,omniIOR*);
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
  { IOP::TAG_ALTERNATE_IIOP_ADDRESS, 0, 0 },
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
  { IOP::TAG_SSL_SEC_TRANS, 0, 0 },
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
  { 0xffffffff, 0, 0 }
};

static int tablesize = 0;

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
static _CORBA_Unbounded_Sequence_Octet my_orb_type;

void
omniIOR::add_TAG_ORB_TYPE(IOP::TaggedComponent& component, const omniIOR*)
{
  component.tag = IOP::TAG_ORB_TYPE;
  CORBA::ULong max, len;
  max = my_orb_type.maximum();
  len = my_orb_type.length();
  component.component_data.replace(max,len,my_orb_type.get_buffer(),0);
}

static
void insertSupportedComponents(omniIOR* ior)
{
  CORBA::ULong index = ior->iiop.components.length();
  ior->iiop.components.length(index+2);

  // Insert ORB TYPE
  omniIOR::add_TAG_ORB_TYPE(ior->iiop.components[index],ior);

  if (ior->iiop.version.major > 1 || ior->iiop.version.minor != 0)
    // Insert CODE SET
    omniIOR::add_TAG_CODE_SETS(ior->iiop.components[index+1],ior);
}

static
void extractSupportedComponents(omniIOR* ior)
{
  if (!tablesize) {
    while (componentUnmarshalHandlers[tablesize].id != 0xffffffff) tablesize++;
  }

  CORBA::ULong total = ior->iiop.components.length();
  for (CORBA::ULong index = 0; index < total; index++) {

    int top = tablesize;
    int bottom = 0;

    do {
      int i = (top + bottom) >> 1;
      IOP::ComponentId id = componentUnmarshalHandlers[i].id;
      if (id == ior->iiop.components[index].tag) {
	if (componentUnmarshalHandlers[i].fn) {
	  componentUnmarshalHandlers[i].fn(ior->iiop.components[index],ior);
	}
	break;
      }
      else if (id > ior->iiop.components[index].tag) {
	top = i;
      }
      else {
	bottom = i + 1;
      }
    } while (top != bottom);
  }
}




/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_ior_initialiser : public omniInitialiser {
public:
  omni_ior_initialiser() {}

  void attach() {
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
