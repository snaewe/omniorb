// -*- Mode: C++; -*-
//                            Package   : omniORB
// callDescriptor.h           Created on: 12/98
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

/*
 $Log$
 Revision 1.4  2001/02/21 14:12:22  dpg1
 Merge from omni3_develop for 3.0.3 release.

 Revision 1.1.2.1  1999/09/24 09:51:43  djr
 Moved from omniORB2 + some new files.

*/

#ifndef __OMNIORB_CALLDESCRIPTOR_H__
#define __OMNIORB_CALLDESCRIPTOR_H__


class omniObjRef;
class omniServant;
class GIOP_C;


//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniCallDescriptor {
public:
  typedef void (*LocalCallFn)(omniCallDescriptor*, omniServant*);

  inline omniCallDescriptor(LocalCallFn lcfn, const char* op,
			    int op_len, _CORBA_Boolean oneway = 0)
    : pd_localCall(lcfn),
      pd_is_oneway(oneway),
      pd_op(op), pd_oplen(op_len),
      pd_ctxt(0) {}

  ///////////////////////////////
  // Methods to implement call //
  ///////////////////////////////

  virtual _CORBA_ULong alignedSize(_CORBA_ULong);
  // Defaults to no arguments.

  virtual void marshalArguments(GIOP_C&);
  // Defaults to no arguments.

  virtual void unmarshalReturnedValues(GIOP_C&);
  // Defaults to no arguments and returns void.

  virtual void userException(GIOP_C&, const char*);
  // Defaults to no user exceptions, and thus throws
  // CORBA::MARSHAL.  Any version of this should in all
  // cases either throw a user exception or CORBA::MARSHAL.
  // Must call giop_client.RequestCompleted().

  ///////////////
  // Accessors //
  ///////////////

  inline const char* op() const           { return pd_op;        }
  inline size_t op_len() const            { return pd_oplen;     }
  inline _CORBA_Boolean is_oneway() const { return pd_is_oneway; }
  inline void doLocalCall(omniServant* servant) {
    pd_localCall(this, servant);
  }

  /////////////////////
  // Context support //
  /////////////////////

  struct ContextInfo {
    inline ContextInfo(CORBA::Context_ptr c, const char*const* cl, int n)
      : context(c), expected(cl), num_expected(n) {}

    CORBA::Context_ptr context;
    const char*const*  expected;
    int                num_expected;
  };

  inline void set_context_info(const ContextInfo* ci) { pd_ctxt = ci; }
  inline const ContextInfo* context_info() { return pd_ctxt; }

private:
  omniCallDescriptor(const omniCallDescriptor&);
  omniCallDescriptor& operator = (const omniCallDescriptor&);
  // Not implemented.

  LocalCallFn        pd_localCall;
  _CORBA_Boolean     pd_is_oneway;
  const char*        pd_op;
  size_t             pd_oplen;
  const ContextInfo* pd_ctxt;
};


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniStdCallDesc //////////////////////////
//////////////////////////////////////////////////////////////////////

//  This just provides a namespace for pre-defined call descriptors.

class omniStdCallDesc {
public:

  // Mangled signature: void
  typedef omniCallDescriptor void_call;

  // Mangled signature: _cCORBA_mObject_i_cstring
  class _cCORBA_mObject_i_cstring : public omniCallDescriptor {
  public:
    inline _cCORBA_mObject_i_cstring(LocalCallFn lcfn, const char* op,
		     size_t oplen, _CORBA_Boolean oneway, const char* a_0) :
      omniCallDescriptor(lcfn, op, oplen, oneway),
      arg_0(a_0)  {}
 
    virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
    virtual void marshalArguments(GIOP_C&);
    virtual void unmarshalReturnedValues(GIOP_C&);
 
    inline CORBA::Object_ptr result() { return pd_result; }
 
    const char* arg_0;
    CORBA::Object_ptr pd_result;
  };

};

//////////////////////////////////////////////////////////////////////
///////////////////// omniLocalOnlyCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

//  This class is needed to support calls to objects which
// may only reside in the local address space.
// eg. ServantLocator, ServantActivator, AdapterActivator.

class omniLocalOnlyCallDescriptor : public omniCallDescriptor {
public:
  omniLocalOnlyCallDescriptor(LocalCallFn lcfn, const char* op,
			      int op_len, _CORBA_Boolean is_oneway = 0)
    : omniCallDescriptor(lcfn, op, op_len, is_oneway) {}


  // We only need to override this one -- as it will throw an
  // exception, so the other members won't get called.
  virtual _CORBA_ULong alignedSize(_CORBA_ULong);
};


#endif  // __OMNIORB_CALLDESCRIPTOR_H__
