// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniInterceptors.h         Created on: 22/09/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories, Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.4.2  2006/06/05 11:28:04  dgrisby
  Change clientSendRequest interceptor members to a single GIOP_C.

  Revision 1.1.4.1  2003/03/23 21:04:14  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.9  2002/11/26 16:54:13  dgrisby
  Fix exception interception.

  Revision 1.1.2.8  2002/11/26 14:50:43  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.7  2002/09/10 23:17:10  dgrisby
  Thread interceptors.

  Revision 1.1.2.6  2002/08/16 16:03:30  dgrisby
  Interceptor tweaks.

  Revision 1.1.2.5  2002/03/27 11:44:51  dpg1
  Check in interceptors things left over from last week.

  Revision 1.1.2.4  2001/08/15 10:26:08  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.2.3  2001/04/18 17:50:44  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.1.2.2  2000/11/15 17:05:39  sll
  Added interceptors along the giop request processing path.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __OMNIINTERCEPTORS_H__
#define __OMNIINTERCEPTORS_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB4/CORBA.h>
#endif

class omniLocalIdentity;
class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)

class omniInterceptorP;
class giopStream;
class GIOP_S;
class GIOP_C;
class orbServer;

class omniInterceptors {
public:

  //////////////////////////////////////////////////////////////////
  class encodeIOR_T {
  public:

    class info_T {
    public:
      omniIOR&                 ior;
      IIOP::ProfileBody&       iiop;
      CORBA::Boolean           default_only;

      info_T(omniIOR& i, IIOP::ProfileBody& body, CORBA::Boolean b) :
         ior(i), iiop(body), default_only(b) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class decodeIOR_T {
  public:

    class info_T {
    public:
      const IIOP::ProfileBody& iiop;
      omniIOR&                 ior;
      CORBA::Boolean           has_iiop_body;

      info_T(const IIOP::ProfileBody& body, omniIOR& i, CORBA::Boolean b) :
         iiop(body), ior(i), has_iiop_body(b) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class clientSendRequest_T {
  public:

    class info_T {
    public:
      GIOP_C&                  giop_c;
      IOP::ServiceContextList  service_contexts;

      info_T(GIOP_C& c) :
        giop_c(c), service_contexts(5) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class clientReceiveReply_T {
  public:

    class info_T {
    public:
      GIOP_C&                  giop_c;
      IOP::ServiceContextList& service_contexts;
      
      info_T(GIOP_C& c, IOP::ServiceContextList& sc) :
	giop_c(c), service_contexts(sc) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class serverReceiveRequest_T {
  public:

    class info_T {
    public:
      GIOP_S& giop_s;

      info_T(GIOP_S& s) : 
	giop_s(s) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  class serverSendReply_T {
  public:

    class info_T {
    public:
      GIOP_S&                  giop_s;
      
      info_T(GIOP_S& s) :
	giop_s(s) {}
    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class serverSendException_T {
  public:

    class info_T {
    public:
      GIOP_S&                  giop_s;
      const CORBA::Exception*  exception;

      info_T(GIOP_S& s, const CORBA::Exception* e) :
	giop_s(s), exception(e) {}
    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class createIdentity_T {
  public:
    
    class info_T {
    public:
      omniIOR*            ior;
      const char*         targetRepoId;
      omniIdentity*&      invoke_handle;
      CORBA::Boolean      held_internalLock;

      info_T(omniIOR* i, const char* t, omniIdentity*& id, CORBA::Boolean b) :
	ior(i), targetRepoId(t), invoke_handle(id), held_internalLock(b) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class createORBServer_T {
  public:
    
    class info_T {
    public:
      omnivector<orbServer*>& servers;

      info_T(omnivector<orbServer*>& s) : servers(s) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class createThread_T {
  public:
    
    class info_T {
    public:
      virtual void run() = 0;
    };

    typedef void (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };

  //////////////////////////////////////////////////////////////////
  class assignUpcallThread_T {
  public:
    
    class info_T {
    public:
      virtual void run() = 0;
    };

    typedef void (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);
  };


  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  encodeIOR_T                encodeIOR;
  decodeIOR_T                decodeIOR;
  clientSendRequest_T        clientSendRequest;
  clientReceiveReply_T       clientReceiveReply;
  serverReceiveRequest_T     serverReceiveRequest;
  serverSendReply_T          serverSendReply;
  serverSendException_T      serverSendException;
  createIdentity_T           createIdentity;
  createORBServer_T          createORBServer;
  createThread_T             createThread;
  assignUpcallThread_T       assignUpcallThread;

  //////////////////////////////////////////////////////////////////
  friend class omni_interceptor_initialiser;

 private:
  omniInterceptors();
  ~omniInterceptors();
};

OMNI_NAMESPACE_END(omni)


#endif // __OMNIINTERCEPTORS_H__
