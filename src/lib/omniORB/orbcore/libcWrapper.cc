// -*- Mode: C++; -*-
//                            Package   : omniORB
// libcWrapper.cc             Created on: 19/3/96
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
//	Wrapper for libc functions which are non-reentrant
//

/*
  $Log$
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

#if !defined(HAVE_STRCASECMP) || !defined(HAVE_STRNCASECMP)
#  include <ctype.h>  //  for toupper and tolower.
#endif

#include "libcWrapper.h"

OMNI_NAMESPACE_BEGIN(omni)


omni_tracedmutex LibcWrapper::non_reentrant;


int
LibcWrapper::gethostbyname(const char *name,
			   LibcWrapper::hostent_var &h,
			   int &rc)
{
#if defined(__sunos__) && __OSVERSION__ >= 5

  // Use gethostbyname_r() on Solaris 2
  if (!h.pd_buffer) {
    h.pd_buffer = new char[256];
    h.pd_buflen = 256;
  }
again:
  if (gethostbyname_r(name,&h.pd_ent,h.pd_buffer,h.pd_buflen,&rc) == 0) {
    if (errno == ERANGE) {
      // buffer is too small to store the result, try again
      delete [] h.pd_buffer;
      h.pd_buflen = h.pd_buflen * 2;
      h.pd_buffer = new char [h.pd_buflen];
      goto again;
    }
    return -1;
  }

#elif defined(__osf1__)

  // Use gethostbyname_r() on Digital UNIX
  if (!h.pd_buffer) {
    h.pd_buffer = new char[sizeof(struct hostent_data)];
    memset(h.pd_buffer,0,sizeof(struct hostent_data));
    // XXX Is it possible that the pointer h.pd_buffer is at a wrong alignment
    //     for a struct hostent_data?
    h.pd_buflen = sizeof(struct hostent_data);
  }
  if (gethostbyname_r(name,&h.pd_ent,(struct hostent_data *)h.pd_buffer) < 0) {
    if (errno) {
      rc = HOST_NOT_FOUND;
    }
    else {
      rc = h_errno;
    }
    return -1;
  }

#elif defined(__hpux__)

# if __OSVERSION__ >= 11
  // gethostbyname is thread safe and reentrant under HPUX 11.00
  struct hostent *hp;
  if ((hp = ::gethostbyname(name)) == NULL) {
    rc = errno;
    return -1;
  }
  else {
    // the hostent_var object needs a dummy buffer here otherwise
    // the hostent() member returns NULL
    if (h.pd_buffer == NULL)
      h.pd_buffer = new char[0];
    h.pd_ent = *hp;
  }
# else
  // Use gethostbyname_r() on HPUX 10.20
  //  int gethostbyname_r(const char *name, struct hostent *result,
  //                       struct hostent_data *buffer);
  // -1 = Error, 0 is OK
  extern int h_errno;
  if (!h.pd_buffer) {
    h.pd_buffer = new char[sizeof(hostent_data)];
    memset((void*)h.pd_buffer,0,sizeof(hostent_data));
  }
  if (gethostbyname_r(name,&h.pd_ent,(hostent_data*)h.pd_buffer) == -1) {
    rc = h_errno;  // Error
    return -1;
  }
# endif

#else

  // Use non-reentrant gethostbyname()
  non_reentrant.lock();

  struct hostent *hp;

#ifdef __WIN32__
  long ip;

  // avoid using a numeric address with gethostbyname()
  if ((ip = ::inet_addr(name)) != INADDR_NONE)
    hp = ::gethostbyaddr((char*)&ip, sizeof(ip), AF_INET);
  else
    hp = ::gethostbyname(name);
#else
  hp = ::gethostbyname(name);
#endif
  
#ifdef __atmos__
  if (hp <= 0)
    {
      rc = 0;
      non_reentrant.unlock();
      return -1;
    }
#else
  if (hp == NULL)
    {
#if defined(__WIN32__) || defined(__vms) && __VMS_VER < 70000000
    rc = 0;
#else
#ifdef __osr5__
    extern int h_errno;
#endif
    rc = h_errno;
#endif

    non_reentrant.unlock();
    return -1;
    }
#endif

  // Have to copy the data point to by struct hostent *hp into h.pd_buffer
  //
  // The code below assumes that struct hostent looks like this:
  //
  // struct hostent {
  //        char    *h_name;        /* official name of host */
  //        char    **h_aliases;    /* alias list */
  //        int     h_addrtype;     /* host address type */
  //        int     h_length;       /* length of address */
  //        char    **h_addr_list;  /* list of addresses from name server */
  // };
  //
  //

  int total = 0;
  int naliases = 1;
  int naddrs = 1;
  char **p;

  total += strlen(hp->h_name) + 1;
#ifndef __atmos__
  p = hp->h_aliases;
  while (*p) {
    total += strlen(*p) + 1;
    naliases++;
    p++;
  }
#endif
  total += naliases * sizeof(char *);
  p = hp->h_addr_list;
  while (*p) {
    total += hp->h_length;
    naddrs++;
    p++;
  }
  total += naddrs * sizeof(char *);

  if (h.pd_buffer) {
    delete [] h.pd_buffer;
    h.pd_buffer = 0;
  }
  h.pd_buffer = new char[total];
  h.pd_buflen = total;

  h.pd_ent.h_addrtype = hp->h_addrtype;
  h.pd_ent.h_length = hp->h_length;
  char *q = h.pd_buffer;
  h.pd_ent.h_aliases = (char **) q;
  q += naliases * sizeof(char *);

  h.pd_ent.h_addr_list = (char **) q;
  q += naddrs * sizeof(char *);

  h.pd_ent.h_name = q;
  q += strlen(hp->h_name) + 1;
  strcpy((char *)h.pd_ent.h_name,hp->h_name);

  int idx = 0;
#ifndef __atmos__
  p = hp->h_aliases;
  while (*p) {
    h.pd_ent.h_aliases[idx] = q;
    q += strlen(*p) + 1;
    strcpy(h.pd_ent.h_aliases[idx],*p);
    idx++;
    p++;
  }
#endif

  h.pd_ent.h_aliases[idx] = 0;
  p = hp->h_addr_list;
  idx = 0;
  while (*p) {
    h.pd_ent.h_addr_list[idx] = q;
    q += hp->h_length;
    memcpy((void *) h.pd_ent.h_addr_list[idx],(void *)*p,hp->h_length);
    idx++;
    p++;
#ifdef __atmos__
    // ATMos h_addr_list is not terminated by a null - and
    // only has one IP address per hostname.
    break;
#endif
  }

  h.pd_ent.h_addr_list[idx] = 0;
   non_reentrant.unlock();
#endif
  return 0;
}


int LibcWrapper::isipaddr(const char* hname)
{
  // Test if string contained hname is ipaddress
  // Return: 0: not ip address,  1: is ip address

  int hlen = strlen(hname);

  // Quick tests for invalidity:

  if (hlen < 7 || hlen > 15) return 0;
  if ((int) hname[0] < 48 || (int) hname[0] > 57) return 0;


  // Full test:

  char* orig_pos = new char[hlen+1];
  strcpy(orig_pos,hname);
  char* curr_pos = orig_pos;

  for(int count = 0; count <4; count++)
    {
      char* dot_pos;

      if (((dot_pos = strchr(curr_pos,'.')) == 0) && count < 3)
	{
	  delete[] orig_pos;
	  return 0;
	}
      else if (count == 3) dot_pos = orig_pos+hlen;

      int ip_component_len = (dot_pos - curr_pos) / sizeof(char);
      if (ip_component_len <1 || ip_component_len > 3)
	{
	  delete[] orig_pos;
	  return 0;
	}

      char* ip_component = new char[ip_component_len+1];
      strncpy(ip_component,curr_pos,ip_component_len);
      ip_component[ip_component_len] = '\0';

      for(int str_iter=0; str_iter < ip_component_len; str_iter++)
	{
	  if ((int) ip_component[str_iter] < 48 ||
	                       (int) ip_component[str_iter] > 57)
	    {
	      delete[] ip_component;
	      delete[] orig_pos;

	      return 0;
	    }
	}

      int ip_value = atoi(ip_component);
      delete[] ip_component;

      if (ip_value < 0 || ip_value > 255)
	{
	  delete[] orig_pos;
	  return 0;
	}

      curr_pos = dot_pos+1;
    }

  delete[] orig_pos;

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

