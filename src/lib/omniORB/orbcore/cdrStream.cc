// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrStream.cc               Created on: 09/11/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambrige
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
  Revision 1.1.2.3.2.1  2001/02/23 16:50:40  sll
  SLL work in progress.

  Revision 1.1.2.3  2000/11/20 11:59:43  dpg1
  API to configure code sets.

  Revision 1.1.2.2  2000/11/15 19:16:06  sll
  Changed default native wchar from UCS-4 to UTF-16.

  Revision 1.1.2.1  2000/11/15 17:17:20  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <GIOP_S.h>
#include <initialiser.h>
#include <giopStreamImpl.h>
#include <exceptiondefs.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
cdrStream::completion() {
  return CORBA::COMPLETED_NO;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMNI_USING_NAMESPACE(omni)

/////////////////////////////////////////////////////////////////////////////
omniCodeSet::NCS_C* cdrStream::ncs_c = 0;
omniCodeSet::NCS_W* cdrStream::ncs_w = 0;

omniCodeSet::TCS_C* cdrMemoryStream::default_tcs_c = 0;
omniCodeSet::TCS_W* cdrMemoryStream::default_tcs_w = 0;

/////////////////////////////////////////////////////////////////////////////
//            Deal with insert and extract codeset component in IOR        //
/////////////////////////////////////////////////////////////////////////////
static _CORBA_Unbounded_Sequence_Octet my_code_set;

static
void initialise_my_code_set()
{
  CONV_FRAME::CodeSetComponentInfo info;

  // Could put more conversion_code_sets but our default is sufficent for
  // most cases.
  info.ForCharData.native_code_set = cdrStream::ncs_c->id();
  info.ForCharData.conversion_code_sets.length(1);
  info.ForCharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_8;

  info.ForWcharData.native_code_set = cdrStream::ncs_w->id();
  info.ForWcharData.conversion_code_sets.length(1);
  info.ForWcharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_16;

  cdrEncapsulationStream s(CORBA::ULong(0),1);
  info >>= s;

  CORBA::Octet* p; CORBA::ULong max,len; s.getOctetStream(p,max,len);
  my_code_set.replace(max,len,p,1);
}


void
omniIOR::add_TAG_CODE_SETS(IOP::TaggedComponent& component, const omniIOR*)
{
  component.tag = IOP::TAG_CODE_SETS;
  CORBA::ULong max, len;
  max = my_code_set.maximum();
  len = my_code_set.length();
  component.component_data.replace(max,len,my_code_set.get_buffer(),0);
}

void
omniIOR::unmarshal_TAG_CODE_SETS(const IOP::TaggedComponent& c, omniIOR* ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_CODE_SETS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CONV_FRAME::CodeSetComponentInfo info;
  info <<= e;

  // Pick a char code set convertor
  ior->pd_tcs_c = 0;
  if (info.ForCharData.native_code_set) {
    ior->pd_tcs_c = omniCodeSet::getTCS_C(info.ForCharData.native_code_set,
					  ior->pd_iiop.version);
  }
  if (!ior->pd_tcs_c) {
    CORBA::ULong total = info.ForCharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      ior->pd_tcs_c = omniCodeSet::getTCS_C(
		      info.ForCharData.conversion_code_sets[index],
		      ior->pd_iiop.version);
      if (ior->pd_tcs_c) break;
    }
  }
  if (!ior->pd_tcs_c && (info.ForCharData.native_code_set ||
		      info.ForCharData.conversion_code_sets.length())) {
    // The server has specified its native code set or at least one
    // conversion code set. But we cannot a TCS_C for any of these
    // code set. In this case, we use the fallback code set.
    ior->pd_tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_UTF_8,
					  ior->pd_iiop.version);
  }

  // Pick a wchar code set convertor
  ior->pd_tcs_w = 0;
  if (info.ForWcharData.native_code_set) {
    ior->pd_tcs_w = omniCodeSet::getTCS_W(info.ForWcharData.native_code_set,
					  ior->pd_iiop.version);
  }
  if (!ior->pd_tcs_w) {
    CORBA::ULong total = info.ForWcharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      ior->pd_tcs_w = omniCodeSet::getTCS_W(
		       info.ForWcharData.conversion_code_sets[index],
		       ior->pd_iiop.version);
      if (ior->pd_tcs_w) break;
    }
  }
  if (!ior->pd_tcs_w && (info.ForWcharData.native_code_set ||
			 info.ForWcharData.conversion_code_sets.length())) {
    // The server has specified its native code set or at least one
    // conversion code set. But we cannot a TCS_W for any of these
    // code set. In this case, we use the fallback code set.
    ior->pd_tcs_w = omniCodeSet::getTCS_W(omniCodeSet::ID_UTF_16,
					  ior->pd_iiop.version);
  }
}

static  const char* not_specified = "not specified";
static  const char* not_supported = "0xXXXXXXXX";

static void write_codeset_name(char* buf, const char* cname, 
			       CONV_FRAME::CodeSetId id)
{
  if (strcmp(cname,not_supported))
    strcat(buf,cname);
  else {
    char* p = buf + strlen(buf);
    sprintf(p,"0x%08lx",id);
  }
}

char*
omniIOR::dump_TAG_CODE_SETS(const IOP::TaggedComponent& c)
{

  OMNIORB_ASSERT(c.tag == IOP::TAG_CODE_SETS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CONV_FRAME::CodeSetComponentInfo info;
  info <<= e;

  CORBA::ULong bufsize = 0;
  const char* ncs_c;
  const char* ncs_w;

  {
    omniCodeSet::NCS_C* n;
    n = omniCodeSet::getNCS_C(info.ForCharData.native_code_set);
    if (n)
      ncs_c = n->name();
    else if (info.ForCharData.native_code_set == 0)
      ncs_c = not_specified;
    else
      ncs_c = not_supported;
    bufsize += strlen(ncs_c) + 1;
  }

  {
    omniCodeSet::NCS_W* n;
    n = omniCodeSet::getNCS_W(info.ForWcharData.native_code_set);
    if (n)
      ncs_w = n->name();
    else if (info.ForWcharData.native_code_set == 0)
      ncs_w = not_specified;
    else
      ncs_w = not_supported;
    bufsize += strlen(ncs_w) + 1;
  }

  CORBA::ULong total,index;
  const char** tcs_c;
  const char** tcs_w;

  total = info.ForCharData.conversion_code_sets.length();
  tcs_c = new const char*[total+1];
  for (index = 0; index < total; index++) {
    omniCodeSet::TCS_C* t;
    t = omniCodeSet::getTCS_C(info.ForCharData.conversion_code_sets[index],
			      giopStreamImpl::maxVersion()->version());
    if (t)
      tcs_c[index] = t->name();
    else if (info.ForCharData.conversion_code_sets[index] == 0)
      tcs_c[index] = not_specified;
    else
      tcs_c[index] = not_supported;
    bufsize += strlen(tcs_c[index]) + 3;
  }
  tcs_c[index] = 0;

  total = info.ForWcharData.conversion_code_sets.length();
  tcs_w = new const char*[total+1];
  for (index = 0; index < total; index++) {
    omniCodeSet::TCS_W* t;
    t = omniCodeSet::getTCS_W(info.ForWcharData.conversion_code_sets[index],
			      giopStreamImpl::maxVersion()->version());
    if (t)
      tcs_w[index] = t->name();
    else if (info.ForWcharData.conversion_code_sets[index] == 0)
      tcs_w[index] = not_specified;
    else
      tcs_w[index] = not_supported;
    bufsize += strlen(tcs_w[index]) + 3;
  }
  tcs_w[index] = 0;

  CORBA::String_var strbuf = CORBA::string_alloc(bufsize+256);
  const char** p;
  strcpy(strbuf,"TAG_CODE_SETS char native code set: ");
  write_codeset_name(strbuf,ncs_c,info.ForCharData.native_code_set);
  strcat(strbuf,"\n");
  strcat(strbuf,"              char conversion code set: ");
  p = tcs_c;
  index = 0;
  while (*p) {
    if (index) 
      strcat(strbuf,", ");
    write_codeset_name(strbuf,*p,info.ForCharData.conversion_code_sets[index]);
    p++; index++;
  }
  strcat(strbuf,"\n");

  strcat(strbuf,"              wchar native code set: ");
  write_codeset_name(strbuf,ncs_w,info.ForWcharData.native_code_set);
  strcat(strbuf,"\n");
  strcat(strbuf,"              wchar conversion code set: ");
  p = tcs_w;
  index = 0;
  while (*p) {
    if (index) 
      strcat(strbuf,", ");
    write_codeset_name(strbuf,*p,
		       info.ForWcharData.conversion_code_sets[index]);
    p++; index++;
  }
  strcat(strbuf,"\n");

  delete [] tcs_c;
  delete [] tcs_w;

  return strbuf._retn();
}

/////////////////////////////////////////////////////////////////////////////
//            Client side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
void
setCodeSetServiceContext(omniInterceptors::clientSendRequest_T::info_T& info) {
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;
  CORBA::Boolean sendcontext = 0;
  giopStrand& d = (giopStrand&)(info.giopstream);
  GIOP::Version ver = info.giopstream.version();

  if (ver.minor < 1) {
    // Code set service context is only defined from GIOP 1.1 onwards,
    // so here we do not attempt to set a codeset service context.
    info.giopstream.TCS_C(omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver));
    info.giopstream.TCS_W(0);
    return;
  }

  if (d.tcs_selected) {
    // giopStream::acquireClient never gives out the same strand
    // to run 2 different GIOP versions.
    OMNIORB_ASSERT(d.version.major == ver.major && 
		   d.version.minor == ver.minor);

    tcs_c = d.tcs_c;
    tcs_w = d.tcs_w;
    
    // Notice that we do not check the chosen convertors against the IOR
    // of the object. In fact, the IOR of the object may specify a set
    // of codesets that does not contain the ones we have just selected!
    // This is possible because our selection might be based on the IOR
    // of another object in the same server and a perverse server may put
    // into the 2 IORs a different set of codesets.
  }
  else {
    tcs_c = info.ior.TCS_C();
    tcs_w = info.ior.TCS_W();
    if (tcs_c || tcs_w) {
      sendcontext = 1;
      d.tcs_c = tcs_c;
      d.tcs_w = tcs_w;
      d.version = ver;
      d.tcs_selected = 1;
    }
    else {
      // The server has not supplied any code set information.
      // Use the default code set.
      tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    }
  }
  info.giopstream.TCS_C(tcs_c);
  info.giopstream.TCS_W(tcs_w);
  
  if (sendcontext) {
    cdrEncapsulationStream s(CORBA::ULong(0),1);
    tcs_c->id() >>= s;
    tcs_w->id() >>= s;

    CORBA::Octet* data;
    CORBA::ULong max,datalen;
    s.getOctetStream(data,max,datalen);

    CORBA::ULong len = info.service_contexts.length() + 1;
    info.service_contexts.length(len);
    info.service_contexts[len-1].context_id = IOP::CodeSets;
    info.service_contexts[len-1].context_data.replace(max,datalen,data,1);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << " send codeset service context: ( "
	  << tcs_c->name() << "," << tcs_w->name()
	  << ")\n";
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Server side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
void
getCodeSetServiceContext(omniInterceptors::serverReceiveRequest_T::info_T& info) {
  GIOP::Version ver = info.giop_s.version();

  if (ver.minor < 1) {
    // Code set service context is only defined from  GIOP 1.1 onwards
    info.giop_s.TCS_C(omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver));
    info.giop_s.TCS_W(0);
    return;
  }

  giopStrand& d = (giopStrand&)(info.giop_s);

  omniCodeSet::TCS_C* tcs_c = d.tcs_c;
  omniCodeSet::TCS_W* tcs_w = d.tcs_w;

  IOP::ServiceContextList& svclist = info.giop_s.receive_service_contexts();
  CORBA::ULong total = svclist.length();
  for (CORBA::ULong index = 0; index < total; index++) {
    if (svclist[index].context_id == IOP::CodeSets) {
      cdrEncapsulationStream e(svclist[index].context_data.get_buffer(),
			       svclist[index].context_data.length(),1);

      CONV_FRAME::CodeSetId tcs_cid, tcs_wid;
      tcs_cid <<= e;
      tcs_wid <<= e;
      if (tcs_cid) {
	tcs_c = omniCodeSet::getTCS_C(tcs_cid,ver);
      }
      else {
	// Client do not specify char TCS. Use default.
	tcs_c =  omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
      }
      tcs_w = omniCodeSet::getTCS_W(tcs_wid,ver);
      d.version = ver;
      d.tcs_c = tcs_c;
      d.tcs_w = tcs_w;
      d.tcs_selected = 1;

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << " recieve codeset service context and set TCS to ( "
	    << tcs_c->name() << "," 
	    << tcs_w->name() << ")\n";
      }
      break;
    }
  }

  if (!d.tcs_selected) {
    // In the absence of any codeset negotiation, we choose 
    // ISO-8859-1 as the transmission code set for char
    d.version.major = ver.major; d.version.minor = ver.minor;
    tcs_c = d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    tcs_w = d.tcs_w = 0;
    d.tcs_selected = 1;
  }

  if (d.version.major != ver.major || d.version.minor != ver.minor) {

    // We are in murky water here. The strand has previously been used
    // for one GIOP version and now a request of another GIOP version
    // is coming in. Does the codeset negotiation established for
    // the previous request also apply to this one?
    //
    // Our interpretation is that the previous codeset negotiation only
    // applies to the GIOP version it negotiates for. For the current
    // request, we treat the situation as if no negotiation has taken place
    // at all.
    tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
    tcs_w = 0;
  }

  info.giop_s.TCS_C(tcs_c);
  info.giop_s.TCS_W(tcs_w);
}


/////////////////////////////////////////////////////////////////////////////
//            omniORB API functions                                        //
/////////////////////////////////////////////////////////////////////////////

void
omniORB::nativeCharCodeSet(const char* name)
{
  if (cdrStream::ncs_c) OMNIORB_THROW(BAD_INV_ORDER, 0, CORBA::COMPLETED_NO);

  omniCodeSet::NCS_C* ncs = omniCodeSet::getNCS_C(name);
  if (!ncs) OMNIORB_THROW(NO_RESOURCES, 0, CORBA::COMPLETED_NO);

  cdrStream::ncs_c = ncs;
}

void
omniORB::nativeWCharCodeSet(const char* name)
{
  if (cdrStream::ncs_w) OMNIORB_THROW(BAD_INV_ORDER, 0, CORBA::COMPLETED_NO);

  omniCodeSet::NCS_W* ncs = omniCodeSet::getNCS_W(name);
  if (!ncs) OMNIORB_THROW(NO_RESOURCES, 0, CORBA::COMPLETED_NO);

  cdrStream::ncs_w = ncs;
}

void
omniORB::anyCharCodeSet(const char* name)
{
  if (cdrMemoryStream::default_tcs_c)
    OMNIORB_THROW(BAD_INV_ORDER, 0, CORBA::COMPLETED_NO);

  GIOP::Version ver = giopStreamImpl::maxVersion()->version();
  omniCodeSet::TCS_C* tcs = omniCodeSet::getTCS_C(name, ver);
  if (!tcs) OMNIORB_THROW(NO_RESOURCES, 0, CORBA::COMPLETED_NO);

  cdrMemoryStream::default_tcs_c = tcs;
}

void
omniORB::anyWCharCodeSet(const char* name)
{
  if (cdrMemoryStream::default_tcs_w)
    OMNIORB_THROW(BAD_INV_ORDER, 0, CORBA::COMPLETED_NO);

  GIOP::Version ver = giopStreamImpl::maxVersion()->version();
  omniCodeSet::TCS_W* tcs = omniCodeSet::getTCS_W(name, ver);
  if (!tcs) OMNIORB_THROW(NO_RESOURCES, 0, CORBA::COMPLETED_NO);

  cdrMemoryStream::default_tcs_w = tcs;
}

const char*
omniORB::nativeCharCodeSet()
{
  if (cdrStream::ncs_c) return cdrStream::ncs_c->name();
  return 0;
}

const char*
omniORB::nativeWCharCodeSet()
{
  if (cdrStream::ncs_w) return cdrStream::ncs_w->name();
  return 0;
}

const char*
omniORB::anyCharCodeSet()
{
  if (cdrMemoryStream::default_tcs_c)
    return cdrMemoryStream::default_tcs_c->name();
  return 0;
}

const char*
omniORB::anyWCharCodeSet()
{
  if (cdrMemoryStream::default_tcs_w)
    return cdrMemoryStream::default_tcs_w->name();
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omni_cdrStream_initialiser : public omniInitialiser {
public:
  void attach() {

    if (!cdrStream::ncs_c)
      cdrStream::ncs_c = omniCodeSet::getNCS_C(omniCodeSet::ID_8859_1);

    if (!cdrStream::ncs_w)
      cdrStream::ncs_w = omniCodeSet::getNCS_W(omniCodeSet::ID_UTF_16);

    GIOP::Version ver = giopStreamImpl::maxVersion()->version();

    if (!cdrMemoryStream::default_tcs_c)
      cdrMemoryStream::default_tcs_c =
	omniCodeSet::getTCS_C(cdrStream::ncs_c->id(), ver);

    if (!cdrMemoryStream::default_tcs_w)
      cdrMemoryStream::default_tcs_w =
	omniCodeSet::getTCS_W(cdrStream::ncs_w->id(), ver);

    // Create the tagged component for all IORs created by this ORB.
    initialise_my_code_set();

    // install interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->clientSendRequest.add(setCodeSetServiceContext);
    interceptors->serverReceiveRequest.add(getCodeSetServiceContext);
  }
  void detach() {
  }
};


static omni_cdrStream_initialiser initialiser;

omniInitialiser& omni_cdrStream_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
