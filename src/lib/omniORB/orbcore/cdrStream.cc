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
  Revision 1.1.4.5  2005/01/25 11:17:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.4  2005/01/06 23:10:12  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.3  2003/11/06 11:56:57  dgrisby
  Yet more valuetype. Plain valuetype and abstract valuetype are now working.

  Revision 1.1.4.2  2003/05/20 16:53:15  dgrisby
  Valuetype marshalling support.

  Revision 1.1.4.1  2003/03/23 21:02:24  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.15  2003/02/03 16:53:14  dgrisby
  Force type in constructor argument to help confused compilers.

  Revision 1.1.2.14  2002/11/26 14:51:50  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.13  2002/05/07 12:55:47  dgrisby
  Minor tweak to log message format.

  Revision 1.1.2.12  2002/04/25 23:13:30  dgrisby
  Minor tweak to codeset setting interceptors for GIOP 1.0.

  Revision 1.1.2.11  2001/11/14 17:13:43  dpg1
  Long double support.

  Revision 1.1.2.10  2001/10/17 16:33:28  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.2.9  2001/08/21 11:02:12  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.8  2001/08/17 17:12:35  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.7  2001/08/03 17:41:18  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.6  2001/07/31 17:42:11  sll
  Cleanup String_var usage.

  Revision 1.1.2.5  2001/07/31 16:32:02  sll
  Added virtual function is_giopStream to check if a cdrStream is a giopStream.
  That is, a poor man's substitute for dynamic_cast.

  Revision 1.1.2.4  2001/04/18 18:18:11  sll
  Big checkin with the brand new internal APIs.

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
#include <orbOptions.h>
#include <orbParameters.h>
#include <stdio.h>

OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
omniCodeSet::NCS_C* orbParameters::nativeCharCodeSet = 0;
//  set the native code set for char and string
//

omniCodeSet::NCS_W* orbParameters::nativeWCharCodeSet = 0;
//  set the native code set for wchar and wstring
//

omniCodeSet::TCS_C* orbParameters::anyCharCodeSet = 0;
//  set the preferred code set for char data inside anys
//

omniCodeSet::TCS_W* orbParameters::anyWCharCodeSet = 0;
//  set the preferred code set for wchar data inside anys


/////////////////////////////////////////////////////////////////////////////
cdrStream::cdrStream() : pd_unmarshal_byte_swap(0), pd_marshal_byte_swap(0),
			 pd_inb_end(0), pd_inb_mkr(0),
			 pd_outb_end(0), pd_outb_mkr(0),
			 pd_tcs_c(0), pd_tcs_w(0),
			 pd_ncs_c(orbParameters::nativeCharCodeSet),
			 pd_ncs_w(orbParameters::nativeWCharCodeSet),
			 pd_valueTracker(0), pd_chunked(0)
{
}

/////////////////////////////////////////////////////////////////////////////
cdrStream::~cdrStream()
{
  if (pd_valueTracker) {
    delete pd_valueTracker;
  }
}

/////////////////////////////////////////////////////////////////////////////
void*
cdrStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrStream::_classid) return (cdrStream*)this;
  return 0;
}

int cdrStream::_classid;

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
cdrStream::completion() {
  return CORBA::COMPLETED_NO;
}

/////////////////////////////////////////////////////////////////////////////
void
cdrStream::copy_to(cdrStream& s,int size,omni::alignment_t align) {
  try {
    alignInput(align);
    if (s.maybeReserveOutputSpace(align,size)) {
      omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)s.pd_outb_mkr,
					    align);
      get_octet_array((CORBA::Octet*)p1,size,align);
      s.pd_outb_mkr = (void*)(p1 + size);
    }
    else {
      skipInput(size);
    }
  }
  catch (const CORBA::BAD_PARAM&) {
    // Use a bounce buffer if the destination stream cannot satisfy
    // maybeReserveOutputSpace().
    cdrMemoryStream mbuf(size,0);
    mbuf.maybeReserveOutputSpace(align,size);
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)mbuf.pd_outb_mkr,
					  align);
    get_octet_array((CORBA::Octet*)p1,size,align);
    s.put_octet_array((const CORBA::Octet*)p1,size,align);
  }
}

void
cdrStream::chunkStreamDeclareArrayLength(omni::alignment_t, size_t)
{
  int not_a_chunked_stream = 0;
  OMNIORB_ASSERT(not_a_chunked_stream);
}


/////////////////////////////////////////////////////////////////////////////
#ifdef HAS_LongDouble
#  if SIZEOF_LONG_DOUBLE == 12
#    ifndef __x86__
#      error "12-byte long double only supported for x86"
#    endif

// Intel x86 extended double is odd. Firstly, it's 80 bits, not 96, so
// the two most significant bytes are always zero. Secondly, the
// significand _includes_ the most significant bit. IEEE floating
// point always misses out the msb, as do the other floating point
// formats on x86. This means we have to do lots of bit shifting.
//
// This isn't the most efficient code in the world, but it's designed
// to be easy to understand.

void
operator>>=(_CORBA_LongDouble a, cdrStream& s)
{
  _CORBA_Octet  mbuf[16];
  _CORBA_Octet* dbuf = (_CORBA_Octet*)&a;

  memset(mbuf, 0, 16);

  if (s.pd_marshal_byte_swap) { // big endian
    // Sign and exponent
    mbuf[0] = dbuf[9];
    mbuf[1] = dbuf[8];

    // significand
    mbuf[2] = (dbuf[7] << 1) | (dbuf[6] >> 7);
    mbuf[3] = (dbuf[6] << 1) | (dbuf[5] >> 7);
    mbuf[4] = (dbuf[5] << 1) | (dbuf[4] >> 7);
    mbuf[5] = (dbuf[4] << 1) | (dbuf[3] >> 7);
    mbuf[6] = (dbuf[3] << 1) | (dbuf[2] >> 7);
    mbuf[7] = (dbuf[2] << 1) | (dbuf[1] >> 7);
    mbuf[8] = (dbuf[1] << 1) | (dbuf[0] >> 7);
    mbuf[9] = (dbuf[0] << 1);
  }
  else { // little endian
    // Sign and exponent
    mbuf[15] = dbuf[9];
    mbuf[14] = dbuf[8];

    // significand
    mbuf[13] = (dbuf[7] << 1) | (dbuf[6] >> 7);
    mbuf[12] = (dbuf[6] << 1) | (dbuf[5] >> 7);
    mbuf[11] = (dbuf[5] << 1) | (dbuf[4] >> 7);
    mbuf[10] = (dbuf[4] << 1) | (dbuf[3] >> 7);
    mbuf[ 9] = (dbuf[3] << 1) | (dbuf[2] >> 7);
    mbuf[ 8] = (dbuf[2] << 1) | (dbuf[1] >> 7);
    mbuf[ 7] = (dbuf[1] << 1) | (dbuf[0] >> 7);
    mbuf[ 6] = (dbuf[0] << 1);
  }
  s.put_octet_array((_CORBA_Octet*)mbuf, 16, omni::ALIGN_8);
}

void
operator<<=(_CORBA_LongDouble& a, cdrStream& s)
{
  _CORBA_Octet  mbuf[16];
  _CORBA_Octet* dbuf = (_CORBA_Octet*)&a;

  s.get_octet_array((_CORBA_Octet*)mbuf, 16, omni::ALIGN_8);

  dbuf[11] = dbuf[10] = 0;

  if (s.pd_unmarshal_byte_swap) { // big endian
    // Sign and exponent
    dbuf[9] = mbuf[0];
    dbuf[8] = mbuf[1];

    // significand
    dbuf[7] = (mbuf[0] == 0 && mbuf[1] == 0) ? 0 : 0x80 | (mbuf[2] >> 1);
    dbuf[6] = (mbuf[2] << 7) | (mbuf[3] >> 1);
    dbuf[5] = (mbuf[3] << 7) | (mbuf[4] >> 1);
    dbuf[4] = (mbuf[4] << 7) | (mbuf[5] >> 1);
    dbuf[3] = (mbuf[5] << 7) | (mbuf[6] >> 1);
    dbuf[2] = (mbuf[6] << 7) | (mbuf[7] >> 1);
    dbuf[1] = (mbuf[7] << 7) | (mbuf[8] >> 1);
    dbuf[0] = (mbuf[8] << 7) | (mbuf[9] >> 1);
  }
  else { // little endian
    // Sign and exponent
    dbuf[9] = mbuf[15];
    dbuf[8] = mbuf[14];

    // significand
    dbuf[7] = (mbuf[15] == 0 && mbuf[14] == 0) ? 0 : 0x80 | (mbuf[13] >> 1);
    dbuf[6] = (mbuf[13] << 7) | (mbuf[12] >> 1);
    dbuf[5] = (mbuf[12] << 7) | (mbuf[11] >> 1);
    dbuf[4] = (mbuf[11] << 7) | (mbuf[10] >> 1);
    dbuf[3] = (mbuf[10] << 7) | (mbuf[ 9] >> 1);
    dbuf[2] = (mbuf[ 9] << 7) | (mbuf[ 8] >> 1);
    dbuf[1] = (mbuf[ 8] << 7) | (mbuf[ 7] >> 1);
    dbuf[0] = (mbuf[ 7] << 7) | (mbuf[ 6] >> 1);
  }
}

#  endif // SIZEOF_LONG_DOUBLE == 12
#endif // HAS_LongDouble


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMNI_USING_NAMESPACE(omni)

/////////////////////////////////////////////////////////////////////////////
//            Deal with insert and extract codeset component in IOR        //
/////////////////////////////////////////////////////////////////////////////
static
void initialise_my_code_set()
{
  CONV_FRAME::CodeSetComponentInfo info;

  // Could put more conversion_code_sets but our default is sufficent for
  // most cases.
  info.ForCharData.native_code_set = orbParameters::nativeCharCodeSet->id();
  info.ForCharData.conversion_code_sets.length(1);
  info.ForCharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_8;

  info.ForWcharData.native_code_set = orbParameters::nativeWCharCodeSet->id();
  info.ForWcharData.conversion_code_sets.length(1);
  info.ForWcharData.conversion_code_sets[0] = omniCodeSet::ID_UTF_16;

  omniIOR::add_TAG_CODE_SETS(info);
}


void
omniIOR::unmarshal_TAG_CODE_SETS(const IOP::TaggedComponent& c, omniIOR& ior)
{
  OMNIORB_ASSERT(c.tag == IOP::TAG_CODE_SETS);
  cdrEncapsulationStream e(c.component_data.get_buffer(),
			   c.component_data.length(),1);

  CONV_FRAME::CodeSetComponentInfo info;
  info <<= e;

  // Pick a char code set convertor
  omniCodeSet::TCS_C* tcs_c = 0;
  if (info.ForCharData.native_code_set) {
    tcs_c = omniCodeSet::getTCS_C(info.ForCharData.native_code_set,
				  ior.getIORInfo()->version());
  }
  if (!tcs_c) {
    CORBA::ULong total = info.ForCharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      tcs_c = omniCodeSet::getTCS_C(
		      info.ForCharData.conversion_code_sets[index],
		      ior.getIORInfo()->version());
      if (tcs_c) break;
    }
  }
  if (!tcs_c && (info.ForCharData.native_code_set ||
		 info.ForCharData.conversion_code_sets.length())) {
    // The server has specified its native code set or at least one
    // conversion code set. But we cannot a TCS_C for any of these
    // code set. In this case, we use the fallback code set.
    tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_UTF_8,
				  ior.getIORInfo()->version());
  }
  ior.getIORInfo()->TCS_C(tcs_c);

  // Pick a wchar code set convertor
  omniCodeSet::TCS_W* tcs_w = 0;
  if (info.ForWcharData.native_code_set) {
    tcs_w = omniCodeSet::getTCS_W(info.ForWcharData.native_code_set,
				  ior.getIORInfo()->version());
  }
  if (!tcs_w) {
    CORBA::ULong total = info.ForWcharData.conversion_code_sets.length();
    for (CORBA::ULong index=0; index < total; index++) {
      tcs_w = omniCodeSet::getTCS_W(
		       info.ForWcharData.conversion_code_sets[index],
		       ior.getIORInfo()->version());
      if (tcs_w) break;
    }
  }
  if (!tcs_w && (info.ForWcharData.native_code_set ||
		 info.ForWcharData.conversion_code_sets.length())) {
    // The server has specified its native code set or at least one
    // conversion code set. But we cannot a TCS_W for any of these
    // code set. In this case, we use the fallback code set.
    tcs_w = omniCodeSet::getTCS_W(omniCodeSet::ID_UTF_16,
				  ior.getIORInfo()->version());
  }
  ior.getIORInfo()->TCS_W(tcs_w);
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
    sprintf(p,"0x%08lx",(unsigned long)id);
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

  CORBA::String_var strbuf(CORBA::string_alloc(bufsize+256));
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
CORBA::Boolean
setCodeSetServiceContext(omniInterceptors::clientSendRequest_T::info_T& info) {
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;
  CORBA::Boolean sendcontext = 0;
  giopStrand& d = (giopStrand&)(info.giopstream);
  GIOP::Version ver = info.giopstream.version();

  if (ver.minor < 1) {
    // Code set service context is only defined from GIOP 1.1 onwards,
    // so here we do not attempt to set a codeset service context.
    if (!d.tcs_selected) {
      d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
      d.tcs_w = 0;
    }
    info.giopstream.TCS_C(d.tcs_c);
    info.giopstream.TCS_W(d.tcs_w);
    return 1;
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
    tcs_c = info.ior.getIORInfo()->TCS_C();
    tcs_w = info.ior.getIORInfo()->TCS_W();
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
    cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
    tcs_c->id() >>= s;
    if (tcs_w) {
      tcs_w->id() >>= s;
    }
    else {
      operator>>=(CORBA::ULong(0),s);
    }

    CORBA::Octet* data;
    CORBA::ULong max,datalen;
    s.getOctetStream(data,max,datalen);

    CORBA::ULong len = info.service_contexts.length() + 1;
    info.service_contexts.length(len);
    info.service_contexts[len-1].context_id = IOP::CodeSets;
    info.service_contexts[len-1].context_data.replace(max,datalen,data,1);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << " send codeset service context: ("
	  << tcs_c->name() << "," << ((tcs_w) ? tcs_w->name() : "none")
	  << ")\n";
    }
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Server side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
getCodeSetServiceContext(omniInterceptors::serverReceiveRequest_T::info_T& info) {
  giopStrand& d = (giopStrand&)(info.giop_s);
  GIOP::Version ver = info.giop_s.version();

  if (ver.minor < 1) {
    // Code set service context is only defined from  GIOP 1.1 onwards
    if (!d.tcs_selected) {
      d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1,ver);
      d.tcs_w = 0;
    }
    info.giop_s.TCS_C(d.tcs_c);
    info.giop_s.TCS_W(d.tcs_w);
    return 1;
  }

  omniCodeSet::TCS_C* tcs_c = d.tcs_c;
  omniCodeSet::TCS_W* tcs_w = d.tcs_w;

  IOP::ServiceContextList& svclist = info.giop_s.service_contexts();
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
      if (tcs_wid) {
	tcs_w = omniCodeSet::getTCS_W(tcs_wid,ver);
      }
      else {
	// Client do not specify wchar TCS.
	tcs_w = 0;
      }
      d.version = ver;
      d.tcs_c = tcs_c;
      d.tcs_w = tcs_w;
      d.tcs_selected = 1;

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << " receive codeset service context and set TCS to ("
	    << ((tcs_c) ? tcs_c->name() : "none") << "," 
	    << ((tcs_w) ? tcs_w->name() : "none") << ")\n";
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

  return 1;
}


/////////////////////////////////////////////////////////////////////////////
static const char* unknown_code_set_msg = "Unknown code set name";

/////////////////////////////////////////////////////////////////////////////
class nativeCharCodeSetHandler : public orbOptions::Handler {
public:

  nativeCharCodeSetHandler() : 
    orbOptions::Handler("nativeCharCodeSet",
			"nativeCharCodeSet = <code set name, e.g. ISO-8859-1>",
			1,
			"-ORBnativeCharCodeSet <code set name, e.g. ISO-8859-1>") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {
    
    omniCodeSet::NCS_C* v = omniCodeSet::getNCS_C(value);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::nativeCharCodeSet = v;
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::nativeCharCodeSet)
      v = orbParameters::nativeCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static nativeCharCodeSetHandler nativeCharCodeSetHandler_;

/////////////////////////////////////////////////////////////////////////////
class nativeWCharCodeSetHandler : public orbOptions::Handler {
public:

  nativeWCharCodeSetHandler() : 
    orbOptions::Handler("nativeWCharCodeSet",
			"nativeWCharCodeSet = <code set name, e.g. UTF-16>",
			1,
			"-ORBnativeWCharCodeSet <code set name, e.g. UTF-16>") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {
    
    omniCodeSet::NCS_W* v = omniCodeSet::getNCS_W(value);
    if (!v) {
      throw orbOptions::BadParam(key(),value,unknown_code_set_msg);
    }
    orbParameters::nativeWCharCodeSet = v;
  }

  void dump(orbOptions::sequenceString& result) {

    const char* v;
    if (orbParameters::nativeWCharCodeSet)
      v = orbParameters::nativeWCharCodeSet->name();
    else
      v = "nil";
    orbOptions::addKVString(key(),v,result);
  }
};

static nativeWCharCodeSetHandler nativeWCharCodeSetHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omni_cdrStream_initialiser : public omniInitialiser {
public:

  omni_cdrStream_initialiser() {
    orbOptions::singleton().registerHandler(nativeCharCodeSetHandler_);
    orbOptions::singleton().registerHandler(nativeWCharCodeSetHandler_);
  }


  void attach() {

    if (!orbParameters::nativeCharCodeSet)
      orbParameters::nativeCharCodeSet = omniCodeSet::getNCS_C(omniCodeSet::ID_8859_1);

    if (!orbParameters::nativeWCharCodeSet)
      orbParameters::nativeWCharCodeSet = omniCodeSet::getNCS_W(omniCodeSet::ID_UTF_16);

    GIOP::Version ver = giopStreamImpl::maxVersion()->version();

    if (!orbParameters::anyCharCodeSet)
      orbParameters::anyCharCodeSet =
	omniCodeSet::getTCS_C(orbParameters::nativeCharCodeSet->id(), ver);

    if (!orbParameters::anyWCharCodeSet)
      orbParameters::anyWCharCodeSet =
	omniCodeSet::getTCS_W(orbParameters::nativeWCharCodeSet->id(), ver);

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

