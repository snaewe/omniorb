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


//////////////////////////////////////////////////////////////////////
/////////////////////////// ServerRequest ////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniServerRequest : public CORBA::ServerRequest {
public:
  virtual ~omniServerRequest();
  omniServerRequest(GIOP_S& giopS)
    : pd_state(SR_READY), pd_giop_s(giopS) {}

  virtual const char* operation();
  virtual void arguments(CORBA::NVList_ptr&);
  virtual CORBA::Context_ptr ctx();
  virtual void set_result(const CORBA::Any&);
  virtual void set_exception(const CORBA::Any&);

  enum State {
    SR_READY,        // initial state
    SR_GOT_PARAMS,   // user has retrieved params AND there is context info
                     // left to be retrieved.
    SR_GOT_CTX,      // user has retrieved params and context
    SR_GOT_RESULT,   // user has given a result value
    SR_EXCEPTION,    // user has set an exception
    SR_DSI_ERROR,    // usage error
    SR_ERROR         // MARSHAL or other error
  };

  //////////////////////
  // omniORB internal //
  //////////////////////
  State             state()     { return pd_state;     }
  CORBA::NVList_ptr params()    { return pd_params;    }
  CORBA::Any&       result()    { return pd_result;    }
  CORBA::Any&       exception() { return pd_exception; }

private:
  State              pd_state;             // to check proper invocation order
  GIOP_S&            pd_giop_s;            // we don't own
  CORBA::Context_var pd_context;           // we own (if non-0)
  CORBA::NVList_var  pd_params;            // _nil() until set by DIR,
                                           //  then we own.
  CORBA::Any         pd_result;
  CORBA::Any         pd_exception;
};


#endif  // __DYNAMICIMPLEMENTATION_H__
