// -*- Mode: C++; -*-
//                            Package   : omniORB2
// dynamicImplementation.h    Created on: 11/1998
//                            Author    : David Riddoch (djr)
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
//   Dynamic Skeleton Interface (DSI).
//

#ifndef __DYNAMICIMPLEMENTATION_H__
#define __DYNAMICIMPLEMENTATION_H__

#include <omniORB2/CORBA.h>


//////////////////////////////////////////////////////////////////////
/////////////////////////// ServerRequest ////////////////////////////
//////////////////////////////////////////////////////////////////////

class ServerRequestImpl : public CORBA::ServerRequest {
public:
  ServerRequestImpl(const char* op, GIOP_S* giopS,
		    CORBA::Boolean responseExpected)
    : pd_state(SR_READY), pd_opName(op), pd_giopS(giopS),
      pd_responseExpected(responseExpected) {}

  virtual const char* op_name();
  virtual CORBA::OperationDef_ptr op_def();
  virtual CORBA::Context_ptr ctx();
  virtual void params(CORBA::NVList_ptr parameters);
  virtual void result(CORBA::Any* value);
  virtual void exception(CORBA::Any* value);

  enum State {
    SR_READY,        // initial state.
    SR_GOT_PARAMS,   // user has retrieved params.
    SR_GOT_CTX,      // user has retrieved params and context.
    SR_GOT_RESULT,   // user has given a result value.
    SR_EXCEPTION,    // user has set an exception.
    SR_ERROR         // usage error or MARSHAL occurred.
  };

  //////////////////////
  // omniORB internal //
  //////////////////////
  State             state()     { return pd_state;                  }
  CORBA::NVList_ptr params()    { return pd_params;                 }
  CORBA::Any*       result()    { return pd_result.operator->();    }
  CORBA::Any*       exception() { return pd_exception.operator->(); }

private:
  State              pd_state;             // to check proper invocation order
  const char*        pd_opName;            // we don't own
  GIOP_S*            pd_giopS;             // we don't own
  CORBA::Boolean     pd_responseExpected;
  CORBA::Context_var pd_context;           // we own (if non-0)
  CORBA::NVList_var  pd_params;            // _nil() until set by DIR,
                                           //  then we own.
  CORBA::Any_var     pd_result;            // 0 until set by DIR, then we own
  CORBA::Any_var     pd_exception;         // 0 until set by DIR, then we own
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// DsiObject /////////////////////////////
//////////////////////////////////////////////////////////////////////

//
// This is a CORBA object which implements the dynamic skeleton
// interface. It has an associated DynamicImplementation which
// is provided by the user to supply the actual implementation.
//  This object is registered with the BOA, and so is assigned
// an object key, and placed in the object table as per usual.
//

class DsiObject;
typedef DsiObject* DsiObject_ptr;

class DsiObject : public virtual omniObject, public virtual CORBA::Object {
public:
  DsiObject(CORBA::BOA::DynamicImplementation_ptr dynImpl,
	    const char* intfRepoId);

  DsiObject() : pd_dynImpl(0) {}
  // This constructor is used for DsiObject::_nil()

  virtual ~DsiObject();

  virtual CORBA::Boolean dispatch(GIOP_S& s,const char *op,
				  CORBA::Boolean response_expected);

  virtual void* _widenFromTheMostDerivedIntf(const char* type_id,
                                             _CORBA_Boolean is_cxx_type_id=0);
  // Overrides omniObject.

  CORBA::Object_ptr _this() {
    CORBA::Object::_duplicate(this);
    return this;
  }

private:
  DsiObject(const DsiObject&);
  const DsiObject& operator= (const DsiObject&);

  CORBA::BOA::DynamicImplementation_ptr pd_dynImpl;

  class resultsMarshaller : public giopMarshaller {
  public:
    resultsMarshaller(giopStream&,ServerRequestImpl&);
    void marshalData();
    size_t dataSize(size_t);
  private:
    giopStream&        pd_s;
    ServerRequestImpl& pd_i;
  };

  class exceptionMarshaller : public giopMarshaller {
  public:
    exceptionMarshaller(giopStream&,ServerRequestImpl&);
    void marshalData();
    size_t dataSize(size_t);
  private:
    giopStream&        pd_s;
    ServerRequestImpl& pd_i;
  };

};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//
// The internal state of DynamicImplementation.
//

struct DynamicImplementation_pd {
  // These are both 0 until the DynamicImplementation
  // has been registered with the BOA.
  DsiObject_ptr  object;
  CORBA::BOA_ptr boa;
};


#endif  // __DYNAMICIMPLEMENTATION_H__
