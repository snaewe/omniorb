// -*- Mode: C++; -*-
//                            Package   : omniORB2
// libcWrapper.cc             Created on: 19/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	Wrapper for libc functions which are non-reentrant
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>
#include <errno.h>
#include "libcWrapper.h"

static omni_mutex non_reentrant;

int
LibcWrapper::gethostbyname(const char *name,
			   LibcWrapper::hostent_var &h,
			   int &rc)
{
#if defined(__sunos__) && defined(SVR4)

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

#else

  // Use non-reentrant gethostbyname()
  non_reentrant.lock();
  struct hostent *hp;
  if ((hp = ::gethostbyname(name)) == 0) {
    rc = h_errno;
    non_reentrant.unlock();
    return -1;
  }

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

  p = hp->h_aliases;
  while (*p) {
    total += strlen(*p) + 1;
    naliases++;
    p++;
  }
  total += naliases * sizeof(char *);

  p = hp->h_addr_list;
  while (*p) {
    total += hp->h_length;
    naddrs++;
    p++;
  }
  total += naddrs * sizeof(char *);

  if (!h.pd_buffer) {
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

  int idx;

  p = hp->h_aliases;
  idx = 0;
  while (*p) {
    h.pd_ent.h_aliases[idx] = q;
    q += strlen(*p) + 1;
    strcpy(h.pd_ent.h_aliases[idx],*p);
    idx++;
    p++;
  }
  h.pd_ent.h_aliases[idx] = 0;

  p = hp->h_addr_list;
  idx = 0;
  while (*p) {
    h.pd_ent.h_addr_list[idx] = q;
    q += hp->h_length;
    memcpy((void *) h.pd_ent.h_addr_list[idx],(void *)*p,hp->h_length);
    idx++;
    p++;
  }
  h.pd_ent.h_addr_list[idx] = 0;

  non_reentrant.unlock();

#endif
  return 0;
}
