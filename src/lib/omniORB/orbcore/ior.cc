// -*- Mode: C++; -*-
//                            Package   : omniORB2
// ior.cc                     Created on: 5/7/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

#ifndef Swap16
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#else
#error "Swap16 has already been defined"
#endif

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

void
IIOP::profileToEncapStream(IIOP::ProfileBody &p,
			   _CORBA_Unbounded_Sequence_Octet &s)
{
  CORBA::ULong hlen = strlen((const char *)p.host) + 1;
  CORBA::ULong klen = p.object_key.length();
  {
    // calculate the total size of the encapsulated stream
    CORBA::ULong total = 8 + hlen;        // first 4 bytes + aligned host
    total = ((total + 1) & ~(1)) + 2;     // aligned port value
    total = ((total + 3) & ~(3)) + 4 +	// aligned object key
      klen;
    
    s.length(total);
  }


  s[0] = omniORB::myByteOrder;
  s[1] = IIOP::current_major;
  s[2] = IIOP::current_minor;
  s[3] = 0;
  {
    CORBA::ULong &l = (CORBA::ULong &) s[4];
    l = hlen;
  }
  memcpy((void *)&(s[8]),(void *)p.host,hlen);
  CORBA::ULong idx = ((8 + hlen) + 1) & ~(1);
  {
    CORBA::UShort &l = (CORBA::UShort &) s[idx];
    l = p.port;
  }
  idx = ((idx + 2) + 3) & ~(3);
  {
    CORBA::ULong &l = (CORBA::ULong &) s [idx];
    l = klen;
  }
  idx += 4;
  memcpy((void *)&s[idx],(void *)&p.object_key[0],klen);
  return;
}

void
IIOP::EncapStreamToProfile(const _CORBA_Unbounded_Sequence_Octet &s,
			   IIOP::ProfileBody &p)
{
  CORBA::ULong begin = 0;
  CORBA::ULong end = 0;

  // s[0] - byteorder
  end += 1;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  CORBA::Boolean byteswap = ((s[begin] == omniORB::myByteOrder) ? 0 : 1);

  // s[1] - iiop_version.major
  // s[2] - iiop_version.minor
  begin = end;
  end = begin + 2;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  p.iiop_version.major = s[begin];
  p.iiop_version.minor = s[begin+1];
  if (p.iiop_version.major != IIOP::current_major ||
      p.iiop_version.minor != IIOP::current_minor)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  // s[3] - padding
  // s[4] - s[7] host string length
  begin = end + 1;
  end = begin + 4;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) s[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) s[begin]);
      len = Swap32(t);
    }

    // s[8] - s[8+len-1] host string
    begin = end;
    end = begin + len;
    if (s.length() <= end)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // Is this string null terminated?
    if (((char)s[end-1]) != '\0')
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    p.host = new CORBA::Char[len];
    if (!p.host)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    memcpy((void *)p.host,(void *)&(s[begin]),len);
  }
    
  // align to CORBA::UShort
  begin = (end + 1) & ~(1);
  // s[begin] port number
  end = begin + 2;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  if (!byteswap) {
    p.port = ((CORBA::UShort &) s[begin]);
  }
  else {
    CORBA::UShort t = ((CORBA::UShort &) s[begin]);
    p.port = Swap16(t);
  }

  // align to CORBA::ULong
  begin = (end + 3) & ~(3);
  // s[begin]  object key length
  end = begin + 4;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) s[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) s[begin]);
      len = Swap32(t);
    }

    begin = end;
    end = begin + len;
    if (s.length() < end)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // extract object key
    p.object_key.length(len);
    memcpy((void *)&p.object_key[0],(void *)&(s[begin]),len);
  }
  return;
}



CORBA::Char * 
IOP::iorToEncapStr(const CORBA::Char *type_id,
		   const IOP::TaggedProfileList *profiles)
{
  MemBufferedStream buf;

  // create an encapsulation
  omniORB::myByteOrder >>= buf;
  CORBA::ULong l = strlen((const char *)type_id) + 1;
  l >>= buf;
  buf.put_char_array(type_id,l);
  *profiles >>= buf;

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewind_in_mkr();
  size_t s = buf.unRead();
  CORBA::Char * data = (CORBA::Char *)buf.data();

  char *result = new char[4+s*2+1];
  if (!result)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  result[4+s*2] = '\0';
  result[0] = 'I';
  result[1] = 'O';
  result[2] = 'R';
  result[3] = ':';
  for (int i=0; i < (int)s; i++) {
    int j = 4 + i*2;
    int v = ((data[i] & 0xf0) >> 4);
    if (v < 10)
      result[j] = '0' + v;
    else
      result[j] = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      result[j+1] = '0' + v;
    else
      result[j+1] = 'a' + (v - 10);
  }
  return (CORBA::Char *)result;
}

void
IOP::EncapStrToIor(const CORBA::Char *str,
		   CORBA::Char *&type_id,
		   IOP::TaggedProfileList *&profiles)
{
  size_t s = strlen((const char *)str);
  if (s<4)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  const char *p = (const char *) str;
  if (p[0] != 'I' ||
      p[1] != 'O' ||
      p[2] != 'R' ||
      p[3] != ':')
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  MemBufferedStream buf((int)s);
  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v;
    
    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    v >>= buf;
  }

  buf.rewind_in_mkr();
  CORBA::Boolean b;
  b <<= buf;
  buf.byteOrder(b);

  type_id = 0;
  profiles = 0;
  try {
    CORBA::ULong l;
    l <<= buf;
    if (l > buf.unRead())
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    type_id = new CORBA::Char[l];
    if (!type_id)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    buf.get_char_array(type_id,l);

    profiles = new IOP::TaggedProfileList;
    if (!profiles)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    *profiles <<= buf;
  }
  catch (...) {
    if (type_id) delete [] type_id;
    if (profiles) delete profiles;
  }
  return;
}

void
IOP::TaggedProfile::operator>>= (NetBufferedStream &s) {
    tag >>= s;
    profile_data >>= s;
}

void
IOP::TaggedProfile::operator<<= (NetBufferedStream &s) {
  tag <<= s;
  profile_data <<= s;
}

void
IOP::TaggedProfile::operator>>= (MemBufferedStream &s) {
    tag >>= s;
    profile_data >>= s;
}

void
IOP::TaggedProfile::operator<<= (MemBufferedStream &s) {
  tag <<= s;
  profile_data <<= s;
}

#undef Swap16
#undef Swap32

#if defined(__GNUG__)

// The following template classes are defined before the template functions
// inline void _CORBA_Sequence<T>::operator<<= (NetBufferedStream &s) etc
// are defined.
// G++ (2.7.2 or may be later versions as well) does not compile in the
// template functions as a result.
// The following is a workaround which explicitly instantiate the classes
// again.

template class _CORBA_Sequence<IOP::TaggedProfile>;

#endif
