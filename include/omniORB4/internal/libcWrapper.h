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
  Revision 1.1.4.7  2002/11/06 11:31:21  dgrisby
  Old ETS patches that got lost; updates patches README.

  Revision 1.1.4.6  2002/09/08 22:12:22  dgrisby
  Last checkin broke it.

  Revision 1.1.4.5  2002/02/25 11:17:12  dpg1
  Use tracedmutexes everywhere.

  Revision 1.1.4.4  2002/02/11 17:09:48  dpg1
  Fix Windows brokenness caused by autoconf stuff.

  Revision 1.1.4.3  2001/08/24 16:43:25  sll
  Switch to use Winsock 2. Removed reference to winsock.h. Let the pre-processor
  define _WIN32_WINNT=0x0400 to select the right header.

  Revision 1.1.4.2  2001/06/13 20:11:37  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 17:18:16  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.7.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

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


#if defined(__WIN32__)
#  include <winsock2.h>
#else
#  include <netdb.h>
#endif

OMNI_NAMESPACE_BEGIN(omni)

class LibcWrapper {
public:
#ifdef _HAS_NETDB_
  class hostent_var;

  static omni_tracedmutex non_reentrant;
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


OMNI_NAMESPACE_END(omni)

#if defined(_MSC_VER)
#  if defined(_OMNIORB_LIBRARY)
#    define _NT_DLL_ATTR __declspec(dllexport)
#  else
#    define _NT_DLL_ATTR __declspec(dllimport)
#  endif
#else
#  define _NT_DLL_ATTR
#endif

#ifndef HAVE_STRCASECMP
int _NT_DLL_ATTR strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRNCASECMP
int _NT_DLL_ATTR strncasecmp(const char *s1, const char *s2,size_t n);
#endif

#endif // __LIBCWRAPPER_H__
