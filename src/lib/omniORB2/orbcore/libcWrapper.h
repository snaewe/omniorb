// -*- Mode: C++; -*-
//                            Package   : omniORB
// LibcWrapper.h              Created on: 19/3/96
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
  Revision 1.8  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.6.8.1  1999/09/22 14:26:52  djr
  Major rewrite of orbcore to support POA.

  Revision 1.6  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.5  1997/12/09 18:25:28  sll
  Use __WIN32__ instead of __NT__

 * Revision 1.4  1997/05/06  15:23:10  sll
 * Public release.
 *
  */

#ifndef __LIBCWRAPPER_H__
#define __LIBCWRAPPER_H__


#define _HAS_NETDB_


#ifdef __WIN32__
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
