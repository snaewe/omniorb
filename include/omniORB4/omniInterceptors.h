// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
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

OMNI_NAMESPACE_BEGIN(omni)

class omniInterceptorP;
class giopStream;
class GIOP_S;
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

    encodeIOR_T();
    ~encodeIOR_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&); // ORB internal function


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

    decodeIOR_T();
    ~decodeIOR_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };


  //////////////////////////////////////////////////////////////////
  class clientSendRequest_T {
  public:

    class info_T {
    public:
      giopStream&              giopstream;
      const omniIOR&           ior;
      const char*              opname;
      CORBA::Boolean           oneway;
      CORBA::Boolean           response_expected;
      IOP::ServiceContextList  service_contexts;

      info_T(giopStream& s, const omniIOR& i, const char* op,
	     CORBA::Boolean ow, CORBA::Boolean re) :
	giopstream(s),ior(i),opname(op),oneway(ow),response_expected(re),
	service_contexts(5) {}

    private:
      info_T();
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    clientSendRequest_T();
    ~clientSendRequest_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };


  //////////////////////////////////////////////////////////////////
  class clientReceiveReply_T {
  public:

    class info_T {
    public:
      
      info_T();
    private:
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    clientReceiveReply_T();
    ~clientReceiveReply_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  class clientReceiveException_T {
  public:

    class info_T {
    public:
      
      info_T();
    private:
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    clientReceiveException_T();
    ~clientReceiveException_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  class serverReceiveRequest_T {
  public:

    class info_T {
    public:
      GIOP_S&    giop_s;

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

    serverReceiveRequest_T();
    ~serverReceiveRequest_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };


  //////////////////////////////////////////////////////////////////
  class serverSendReply_T {
  public:

    class info_T {
    public:
      
      info_T();
    private:
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    serverSendReply_T();
    ~serverSendReply_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  class serverSendException_T {
  public:

    class info_T {
    public:
      
      info_T();
    private:
      info_T(const info_T&);
      info_T& operator=(const info_T&);
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    serverSendException_T();
    ~serverSendException_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&);  // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  class createIdentity_T {
  public:
    
    class info_T {
    public:
      omniIOR*            ior;
      omniIdentity*&      invoke_handle;
      CORBA::Boolean      held_internalLock;

      info_T(omniIOR* i, omniIdentity*& id, CORBA::Boolean b) :
	ior(i), invoke_handle(id), held_internalLock(b) {}
    };

    typedef CORBA::Boolean (*interceptFunc)(info_T& info);

    void add(interceptFunc);
    void remove(interceptFunc);

    createIdentity_T();
    ~createIdentity_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&); // ORB internal function
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

    createORBServer_T();
    ~createORBServer_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(info_T&); // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  encodeIOR_T                encodeIOR;
  decodeIOR_T                decodeIOR;
  clientSendRequest_T        clientSendRequest;
  clientReceiveReply_T       clientReceiveReply;
  clientReceiveException_T   clientReceiveException;
  serverReceiveRequest_T     serverReceiveRequest;
  serverSendReply_T          serverSendReply;
  serverSendException_T      serverSendException;
  createIdentity_T           createIdentity;
  createORBServer_T          createORBServer;

  //////////////////////////////////////////////////////////////////
  friend class omni_interceptor_initialiser;

 private:
  omniInterceptors();
  ~omniInterceptors();
};

OMNI_NAMESPACE_END(omni)


#endif // __OMNIINTERCEPTORS_H__
