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
 Revision 1.2.2.6  2001/05/29 17:03:47  dpg1
 In process identity.

 Revision 1.2.2.5  2001/04/18 17:50:44  sll
 Big checkin with the brand new internal APIs.
 Scoped where appropriate with the omni namespace.

 Revision 1.2.2.4  2000/11/03 19:00:26  sll
 Removed Suppress_Spurious_gcc_Warnings cpp macro.

 Revision 1.2.2.3  2000/10/06 16:47:26  sll
 Removed inline marshal function from the client and server marshaller.

 Revision 1.2.2.2  2000/09/27 17:18:35  sll
 Updated to use the new cdrStream abstraction.
 Added new member unmarshalArguments(), marshalReturnedValues() and modified
 the ctor arguments to make the omniCallDescriptor class suitable for use in
 the upcalls on the server side.

 Revision 1.2.2.1  2000/07/17 10:35:34  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:05  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.1  1999/09/24 09:51:43  djr
 Moved from omniORB2 + some new files.

*/

#ifndef __OMNIORB_CALLDESCRIPTOR_H__
#define __OMNIORB_CALLDESCRIPTOR_H__

class omniObjRef;
class omniServant;

OMNI_NAMESPACE_BEGIN(omni)
class giopAddress;
OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniCallDescriptor {
public:
  typedef void (*LocalCallFn)(omniCallDescriptor*, omniServant*);

  inline omniCallDescriptor(LocalCallFn lcfn, const char* op,
			    int op_len, _CORBA_Boolean oneway,
			    const char*const* user_excns,
			    int n_user_excns,
                            _CORBA_Boolean is_upcall)
    : pd_localCall(lcfn),
      pd_is_oneway(oneway),
      pd_op(op), pd_oplen(op_len),
      pd_ctxt(0),
      pd_assert_object_existent(omniORB::verifyObjectExistsAndType),
      pd_user_excns(user_excns),
      pd_n_user_excns(n_user_excns),
      pd_is_upcall(is_upcall),
      pd_objref(0),
      pd_first_address_used(0) {}

#if defined(__GNUG__)
  // gcc is rather anal and insists on a class with virtual functions must
  // have a virtual dtor.
  virtual ~omniCallDescriptor() {}
#endif

  //////////////////////////////////////////////////
  // Methods to implement call on the client side //
  //////////////////////////////////////////////////

  virtual void initialiseCall(cdrStream&);

  virtual void marshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void unmarshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  virtual void userException(_OMNI_NS(IOP_C)& iop_client, const char*);
  // Defaults to no user exceptions, and thus throws
  // CORBA::MARSHAL.  Any version of this should in all
  // cases either throw a user exception or CORBA::MARSHAL.
  // Must call iop_client.RequestCompleted().

  //////////////////////////////////////////////////
  // Methods to implement call on the server side //
  //////////////////////////////////////////////////
  virtual void unmarshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void marshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  ///////////////
  // Accessors //
  ///////////////

  inline const char* op() const           { return pd_op;        }
  inline size_t op_len() const            { return pd_oplen;     }
  inline _CORBA_Boolean is_oneway() const { return pd_is_oneway; }
  inline void doLocalCall(omniServant* servant) {
    pd_localCall(this, servant);
  }
  inline const char*const* user_excns() { return pd_user_excns; }
  inline int n_user_excns() { return pd_n_user_excns; }
  inline _CORBA_Boolean is_upcall() const { return pd_is_upcall; }
  inline _CORBA_Boolean haslocalCallFn() const { return (pd_localCall)?1:0; }
  inline const _OMNI_NS(giopAddress)* firstAddressUsed() { 
    return pd_first_address_used;
  }
  inline void objref(omniObjRef* o) { pd_objref = o; }
  inline omniObjRef* objref() { OMNIORB_ASSERT(pd_objref); return pd_objref; }

  inline void firstAddressUsed(const _OMNI_NS(giopAddress)* a) { 
    pd_first_address_used = a;
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

    inline ContextInfo() : context(0), expected(0), num_expected(0) {}
  };

  inline void set_context_info(const ContextInfo* ci) { pd_ctxt = ci; }
  inline const ContextInfo* context_info() { return pd_ctxt; }

  inline void skipAssertObjectExistence() { pd_assert_object_existent = 0; }
  inline _CORBA_Boolean doAssertObjectExistence() { return pd_assert_object_existent; }

private:
  omniCallDescriptor(const omniCallDescriptor&);
  omniCallDescriptor& operator = (const omniCallDescriptor&);
  // Not implemented.

  LocalCallFn                  pd_localCall;
  _CORBA_Boolean               pd_is_oneway;
  const char*                  pd_op;
  size_t                       pd_oplen;
  const ContextInfo*           pd_ctxt;
  _CORBA_Boolean               pd_assert_object_existent;
  const char*const*            pd_user_excns;
  int                          pd_n_user_excns;
  _CORBA_Boolean               pd_is_upcall;

  omniObjRef*                  pd_objref;
  // On the client side, this is needed to check if a servant found by
  // the in process identity can be used in a direct local call.

  const _OMNI_NS(giopAddress)* pd_first_address_used;
  // state holder for the giop transport in relation to this call. Not
  // manipulated by this class other than the access functions.
  // Initialised to 0 in ctor.
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
		     size_t oplen, const char* a_0,
		     _CORBA_Boolean upcall=0) :
      omniCallDescriptor(lcfn, op, oplen, 0, 0, 0, upcall),
      arg_0((char*)a_0) {}
 
    inline ~_cCORBA_mObject_i_cstring() {
      if (is_upcall()) { 
	_CORBA_String_helper::free(arg_0);
      }
    }

    void marshalArguments(cdrStream&);
    void unmarshalReturnedValues(cdrStream&);
    void unmarshalArguments(cdrStream&);
    void marshalReturnedValues(cdrStream&);
 
    inline CORBA::Object_ptr result() { return pd_result._retn(); }
 
    char* arg_0;
    CORBA::Object_var pd_result;
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
    : omniCallDescriptor(lcfn, op, op_len, is_oneway, 0, 0, 0) {}

  // Only useful as client side descriptor. No set up for server side upcall.

  // We only need to override this one -- as it will throw an
  // exception, so the other members won't get called.
  void marshalArguments(cdrStream&);
};

#endif  // __OMNIORB_CALLDESCRIPTOR_H__
