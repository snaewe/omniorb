// -*- Mode: C++; -*-
//                            Package   : omniORB
// LibcWrapper.h              Created on: 19/3/96
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
  Revision 1.1.6.3  2006/02/22 14:56:37  dgrisby
  New endPointPublishHostname and endPointResolveNames parameters.

  Revision 1.1.6.2  2005/01/06 23:08:25  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:03:45  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.8  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

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

#if defined(__WIN32__)
#  define FD_SETSIZE 2048
#  include <winsock2.h>
#else
#  include <netdb.h>
#endif

OMNI_NAMESPACE_BEGIN(omni)

class LibcWrapper {
public:
  class AddrInfo;

  static int isipaddr(const char* node);
  // True if node is an IPv4 address.

  static AddrInfo* getAddrInfo(const char* node, CORBA::UShort port);
  // Return an AddrInfo object for the specified node and port. If
  // node is zero, address is INADDR_ANY. If node is invalid, returns
  // zero.

  static void freeAddrInfo(AddrInfo* ai);
  // Release the AddrInfo object returned by getAddrInfo(), and any in
  // its linked list.

  class AddrInfo {
  public:
    AddrInfo() {}

    virtual ~AddrInfo();

    virtual struct sockaddr* addr() = 0;
    // sockaddr struct suitable for passing to bind(), connect()

    virtual int addrSize() = 0;
    // size of sockaddr struct returned.

    virtual char* asString() = 0;
    // String form of address. Free with CORBA::string_free().

    virtual char* name() = 0;
    // Name relating to address. Returns zero if name cannot be found.

    virtual AddrInfo* next() = 0;
    // Linked list of AddrInfos for multi-homed hosts.

  private:
    // Not implemented:
    AddrInfo(const AddrInfo&);
    AddrInfo& operator=(const AddrInfo&);
  };

  class AddrInfo_var {
    // Partial _var type.
  public:
    inline AddrInfo_var() : pd_ai(0) {}
    inline AddrInfo_var(AddrInfo* ai) : pd_ai(ai) {}
    inline ~AddrInfo_var() {
      if (pd_ai) LibcWrapper::freeAddrInfo(pd_ai);
    }
    inline AddrInfo_var& operator=(AddrInfo* ai) {
      if (pd_ai) LibcWrapper::freeAddrInfo(pd_ai);
      pd_ai = ai;
      return *this;
    }
    inline AddrInfo* operator->() const { return pd_ai; }
    inline operator AddrInfo*() const   { return pd_ai; }
  private:
    AddrInfo* pd_ai;
  };
};


OMNI_NAMESPACE_END(omni)

#if defined(_MSC_VER) && !defined(_WINSTATIC)
#  if defined(_OMNIORB_LIBRARY)
#    define _NT_DLL_ATTR __declspec(dllexport)
#  else
#    define _NT_DLL_ATTR __declspec(dllimport)
#  endif
#endif

#ifndef _NT_DLL_ATTR
#  define _NT_DLL_ATTR
#endif

#ifndef HAVE_STRCASECMP
int _NT_DLL_ATTR strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRNCASECMP
int _NT_DLL_ATTR strncasecmp(const char *s1, const char *s2,size_t n);
#endif

#endif // __LIBCWRAPPER_H__
