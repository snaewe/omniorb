// -*- Mode: C++; -*-
//                            Package   : omniORB2
// LibcWrapper.h              Created on: 19/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	Wrapper for libc functions which are non-reentrant
//	

/*
  $Log$
  Revision 1.3  1997/01/23 16:41:10  sll
  New static member non_reentrant in the LibcWrapper class.

 * Revision 1.2  1997/01/08  18:12:39  ewc
 * Added support for ATMos
 *
 * Revision 1.1  1996/10/10  14:37:53  sll
 * Initial revision
 *
  */

#ifndef __LIBCWRAPPER_H__
#define __LIBCWRAPPER_H__


#define _HAS_NETDB_


#ifdef __NT__
#include <winsock.h>
#else
#include <netdb.h>
#endif


class LibcWrapper {
public:
#ifdef _HAS_NETDB_
  class hostent_var;

  static omni_mutex non_reentrant;
  static int gethostbyname(const char *,hostent_var &,int &);
  static int isipaddr(const char* hname);

  class hostent_var {
  public:
    hostent_var () { pd_buffer = 0; pd_buflen = 0; }
    ~hostent_var() { if (pd_buffer) delete [] pd_buffer; }

    const struct hostent * hostent() { 
      if (pd_buffer) {
	return &pd_ent;
      }
      else {
	return 0;
      }
    }

  private:
    friend class LibcWrapper;
    struct hostent pd_ent;
    char          *pd_buffer;
    int            pd_buflen;
  };

#endif // _HAS_NETDB_
			   
};

#endif // __LIBCWRAPPER_H__
