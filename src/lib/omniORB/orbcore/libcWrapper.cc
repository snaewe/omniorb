// -*- Mode: C++; -*-
//                            Package   : omniORB
// libcWrapper.cc             Created on: 19/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003 Apasphere Ltd
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
//	Wrapper for libc functions which are non-reentrant
//

/*
  $Log$
  Revision 1.21.2.2  2005/01/06 23:10:31  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.21.2.1  2003/03/23 21:02:12  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.19.2.10  2003/02/17 02:03:08  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.19.2.9  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

  Revision 1.19.2.8  2002/11/04 17:41:42  dgrisby
  Don't use gethostbyname for IP addresses on Win32.

  Revision 1.19.2.7  2002/02/25 11:17:13  dpg1
  Use tracedmutexes everywhere.

  Revision 1.19.2.6  2002/01/15 16:38:13  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.19.2.5  2001/06/13 20:13:15  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.19.2.4  2001/06/08 17:12:21  dpg1
  Merge all the bug fixes from omni3_develop.

  Revision 1.19.2.3  2001/04/18 18:18:07  sll
  Big checkin with the brand new internal APIs.

  Revision 1.19.2.2  2000/09/27 17:57:04  sll
  Changed include/omniORB3 to include/omniORB4

  Revision 1.19.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.20  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.18.6.1  1999/09/22 14:26:51  djr
  Major rewrite of orbcore to support POA.

  Revision 1.18  1999/06/26 18:08:17  sll
  HPUX update to separate difference between HPUX 10.20 and HPUX 11.00.

  Revision 1.17  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.16  1999/01/07 15:57:39  djr
  Implemented strcasecmp() and strncasecmp() for those platforms that do
  not have it.

  Revision 1.15  1998/10/20 17:54:29  sll
  On HPUX, allocate memory hostent_data the right way.

  Revision 1.14  1998/08/14 13:48:27  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.13  1998/08/11 19:08:33  sll
  Added CPP macro to recognise SCO OpenServer 5.

  Revision 1.12  1998/01/21 12:30:32  sll
  Corrected typo that only affects hpux.

// Revision 1.11  1998/01/20  17:32:15  sll
// Added support for OpenVMS.
//
  Revision 1.10  1997/12/23 19:24:00  sll
  gethostbyname now works properly on HPUX.

  Revision 1.9  1997/12/09 18:24:29  sll
  Added support for HPUX.

  Revision 1.8  1997/08/21 22:04:49  sll
  Changed to use the new platform identification proprocessor macro.

// Revision 1.7  1997/05/06  15:22:38  sll
// Public release.
//
  */

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(HAVE_STRCASECMP) || !defined(HAVE_STRNCASECMP)
#  include <ctype.h>  //  for toupper and tolower.
#endif

#ifdef __vxWorks__
#  include <hostLib.h>
#  include <resolvLib.h>
#endif

#include "libcWrapper.h"
#include "SocketCollection.h"

OMNI_NAMESPACE_BEGIN(omni)


static omni_tracedmutex non_reentrant;

LibcWrapper::AddrInfo::~AddrInfo() {}


// *** USE ::getaddrinfo() if it's available.


class IP4AddrInfo : public LibcWrapper::AddrInfo
{
public:
  IP4AddrInfo(CORBA::ULong ip4addr, CORBA::UShort port);
  virtual ~IP4AddrInfo();

  virtual struct sockaddr* addr();
  virtual int addrSize();
  virtual char* asString();
  virtual LibcWrapper::AddrInfo* next();

private:
  struct sockaddr_in pd_addr;
};

IP4AddrInfo::IP4AddrInfo(CORBA::ULong ip4addr, CORBA::UShort port)
{
  pd_addr.sin_family      = INETSOCKET;
  pd_addr.sin_addr.s_addr = ip4addr;
  pd_addr.sin_port        = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  pd_addr.sin_len = sizeof(struct sockaddr_in);
#endif
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_ZERO
  memset((void *)&pd_addr.sin_zero, 0, sizeof(pd_addr.sin_zero));
#endif
}

IP4AddrInfo::~IP4AddrInfo() {}

struct sockaddr*
IP4AddrInfo::addr()
{
  return (struct sockaddr*)&pd_addr;
}

int
IP4AddrInfo::addrSize()
{
  return sizeof(struct sockaddr_in);
}

char*
IP4AddrInfo::asString()
{
  CORBA::ULong hipv4 = ntohl(pd_addr.sin_addr.s_addr);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));

  char* result = CORBA::string_alloc(16);
  sprintf(result,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
  return result;
}


LibcWrapper::AddrInfo*
IP4AddrInfo::next()
{
  return 0; // Not implemented
}


static inline CORBA::ULong hostent_to_ip4(struct hostent* entp)
{
  CORBA::ULong ip4;
  memcpy((void*)&ip4, (void*)entp->h_addr_list[0], sizeof(CORBA::ULong));
  return ip4;
}


LibcWrapper::AddrInfo* LibcWrapper::getAddrInfo(const char* node,
						CORBA::UShort port)
{
  if (!node) {
    // No node address -- used to bind to INADDR_ANY
    return new IP4AddrInfo(INADDR_ANY, port);
  }
  if (isipaddr(node)) {
    // Already an IPv4 address.
    CORBA::ULong ip4 = inet_addr(node);

    if (ip4 == RC_INADDR_NONE)
      return 0;
    else
      return new IP4AddrInfo(ip4, port);
  }

#if defined(__sunos__) && __OSVERSION__ >= 5

  // Use gethostbyname_r() on Solaris 2

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc;
  IP4AddrInfo* ret;

again:
  if (gethostbyname_r(node,&ent,buffer,buflen,&rc) == 0) {
    if (errno == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;

#elif defined(__irix__) && __OSVERSION__ >= 6

  // Use gethostbyname_r() on Irix 6.5

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc;
  IP4AddrInfo* ret;

again:
  if (gethostbyname_r(node,&ent,buffer,buflen,&rc) == 0) {
    if (errno == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;

#elif defined(__linux__) && __OSVERSION__ >= 2 && !defined(__cygwin__)

  // Use gethostbyname_r() on Linux 

  struct hostent ent;
  char* buffer = new char[256];
  int buflen = 256;
  int rc, retValue;
  struct hostent *hp;
  IP4AddrInfo* ret;

again:
  retValue = gethostbyname_r(node,&ent,buffer,buflen,&hp,&rc);
  if (hp == 0) {
    if (retValue == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] buffer;
      buflen = buflen * 2;
      buffer = new char [buflen];
      goto again;
    }
    else {
      ret = 0;
    }
  }
  else {
    ret = new IP4AddrInfo(hostent_to_ip4(&ent), port);
  }
  delete [] buffer;
  return ret;



#elif defined(__osf1__)

  // Use gethostbyname_r() on Digital UNIX

  struct hostent ent;
  char* buffer = new char[sizeof(struct hostent_data)];
  memset((void*)buffer,0,sizeof(struct hostent_data));
  // XXX Is it possible that the pointer buffer is at a wrong alignment
  //     for a struct hostent_data?

  IP4AddrInfo* ret;

  if (gethostbyname_r(node,&ent,(struct hostent_data *)buffer) < 0)
    ret = 0;
  else
    ret = new IP4AddrInfo(hostent_to_ip4(&ent), port);

  delete [] buffer;
  return ret;

#elif defined(__hpux__)

# if __OSVERSION__ >= 11
  // gethostbyname is thread safe and reentrant under HPUX 11.00
  struct hostent *hp;
  if ((hp = ::gethostbyname(node)) == NULL) {
    return 0;
  }
  return new IP4AddrInfo(hostent_to_ip4(hp), port);

# else
  // Use gethostbyname_r() on HPUX 10.20
  //  int gethostbyname_r(const char *node, struct hostent *result,
  //                       struct hostent_data *buffer);
  // -1 = Error, 0 is OK

  struct hostent ent;
  char* buffer = new char[sizeof(hostent_data)];
  memset((void*)buffer,0,sizeof(hostent_data));
  IP4AddrInfo* ret;

  if (gethostbyname_r(node,&ent,(hostent_data*)buffer) == -1)
    ret = 0;
  else
    ret = new IP4AddrInfo(hostent_to_ip4(&ent), port);

  delete [] buffer;
  return ret;

# endif

#elif defined(__vxWorks__)
  int ip4 = hostGetByName(const_cast<char*>(node)); // grep /etc/hosts
  if (ip4 == ERROR) return 0;
  return new IP4AddrInfo(ip4, port);

#else
  // Use non-reentrant gethostbyname()
  omni_tracedmutex_lock sync(non_reentrant);

  struct hostent *hp = ::gethostbyname(node);
  
# ifdef __atmos__
  if (hp <= 0)
    return 0
# else
  if (hp == NULL)
    return 0;
# endif

  return new IP4AddrInfo(hostent_to_ip4(hp), port);
#endif
}

void LibcWrapper::freeAddrInfo(LibcWrapper::AddrInfo* ai)
{
  delete ai;
}


int LibcWrapper::isipaddr(const char* hname)
{
  // Test if string hname is an ip address
  // Return: 0: not ip address,  1: is ip address

  const char* c;
  int len, dots;

  for (c=hname, len=0, dots=0; *c; c++,len++) {
    if (*c == '.')
      dots++;
    else if (*c < '0' || *c > '9')
      return 0;
  }
  if (dots != 3 || len < 7 || len > 15) return 0;

  char comp[4];
  int i, j, val;
  c = hname;

  for (i=0; i < 4; i++) {
    for (j=0; *c && *c != '.'; j++, c++)
      comp[j] = *c;

    comp[j] = '\0';

    val = atoi(comp);
    if (val < 0 || val > 255)
      return 0;
  }
  return 1;
}

OMNI_NAMESPACE_END(omni)


#ifndef HAVE_STRCASECMP
int
strcasecmp(const char *s1, const char *s2)
{
  if( s1 == s2 )  return 0;

  while( *s1 && tolower(*s1) == tolower(*s2) )
    s1++, s2++;

  return (int)*s1 - *s2;
}
#endif


#ifndef HAVE_STRNCASECMP
int
strncasecmp(const char *s1, const char *s2, size_t n)
{
  if( s1 == s2 || !n )  return 0;

  while( --n && *s1 && tolower(*s1) == tolower(*s2) )
    s1++, s2++;

  return (int)*s1 - *s2;
}
#endif

