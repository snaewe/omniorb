// -*- Mode: C++; -*-

#ifndef __CORBA_WRAPPERS_H__
#define __CORBA_WRAPPERS_H__

#include <iostream.h>

#ifndef __WRAPPED_NULL_STMT
#define __WRAPPED_NULL_STMT 	if (0) { }
#endif

//////////////////////////////////////////////////////////////////////
//
//                WRAPPER NOTES
//
// General
//   1. always use the generic init forms at top of main:
//          CORBA::ORB_ptr orb = WRAPPED_ORB_INIT(argc, argv);
//          CORBA::BOA_ptr boa = WRAPPED_BOA_INIT(orb, argc, argv);
//
// Main Servant Implementation
//   1. somewhere in main, specify servant name:
//          WRAPPED_ORB_REGISTER_SERVANT_NAME(orb, "my_servant_name");
//   2. tell boa when an object impl o_impl is ready:
//          WRAPPED_BOA_OBJ_IS_READY(boa, o_impl);
//   3. when this impl obj is no longer needed, dispose/destroy it:
//          WRAPPED_DISPOSE(o_impl);
//   4. blocking form of impl-is-ready:
//          WRAPPED_IMPL_IS_READY_BLOCK(boa, "my_servant_name");
//   5. can use fork-join combo for non-blocking impl-is-ready:
//          WRAPPED_IMPL_IS_READY_FORK(boa, "my_servant_name");
//          ...
//          <other work>
//          ...
//          WRAPPED_IMPL_IS_READY_JOIN; // now we are blocking
//   ** the optional _JOIN call must appear in same scope
//        (or subscope) as _FORK call
//
// Consider IDL interface I in module M
// with foo that takes no args and bar that takes a CORBA::ULong
//
// The skeleton class name can be written as:
//          WRAPPED_SKELETON(M::, I)
//
// To declare an impl class that is subclass of skeleton class:
//
// class I_impl : WRAPPED_SKELETON_SUPER(M::, I) {
// public:
//    <constructor decl>
//    void foo ( WRAPPED_DECLARG_VOID );
//    void bar ( CORBA::ULong u    WRAPPED_DECLARG );
// protected:
//    <rest of class decl>
// }
//
// Corresponding method implementations look like:
// 
// void I_impl::foo ( WRAPPED_IMPLARG_VOID ) { <impl> };
// void I_impl::bar ( CORBA::ULong u WRAPPED_IMPLARG ) { <impl> };
//
// For constructors that require the invocation of the default 
// parent constructor,  or the parent constructor that accepts
// an object key, use one of the following:
//
//	WRAPPED_SUPER(module, interface)
//	WRAPPED_SUPER_KEY(module, interface, key)
//
//  I_impl::I_impl() :
//	WRAPPED_SUPER(M::, I)	{;}
//  I_impl::I_impl(key) :
//	WRAPPED_SUPER_KEY(M::, I, key) {;}
// 
// ** note no comma between last normal param (u) and
//      WRAPPED_DECLARG or WRAPPED_IMPLARG
//
// Duplicating References
//   1. When a CORBA reference needs to be duplicated, use the 
//      WRAPPED_DUPLICATE() macro
//   2. When an implementation reference needs to be duplicated,
//      use the WRAPPED_IMPL2OREF() macro; WRAPPED_RELEASE_IMP()
//      should be used for releasing an implementation reference
//
// Disposing Objects
//   1. When a CORBA object needs to be disposed, use the
//      WRAPPED_DISPOSE() macro
//   2. When an implementation object needs to be disposed,
//      use the WRAPPED_DISPOSE_IMPL() macro
//
//////////////////////////////////////////////////////////////////////

#ifdef __ORBIX3__ 

#include <CORBA.h>
#include <NamingService.hh>
#include "BootStrapAgent.h"

#define WRAPPED_ORB_INIT(argc,argv)      CORBA::ORB_init(argc, argv, "Orbix")
#define WRAPPED_BOA_INIT(orb,argc,argv)  orb->BOA_init(argc, argv, "Orbix_BOA")
#define WRAPPED_ORB_BOA_CLEANUP(orb,boa) __WRAPPED_NULL_STMT
#define WRAPPED_ORB_SETMAXTCP(x)         __WRAPPED_NULL_STMT
#define WRAPPED_SET_TCALIASEXPAND(x)     __WRAPPED_NULL_STMT
#define WRAPPED_IMPL_IS_READY_BLOCK(boa,servant_name) \
  		boa->impl_is_ready(servant_name, CORBA::Orbix.INFINITE_TIMEOUT)
#define WRAPPED_ORB_REGISTER_SERVANT_NAME(orb,servant_name) \
  		orb->setServerName(servant_name)
#define WRAPPED_BOA_OBJ_IS_READY(boa,objref)    __WRAPPED_NULL_STMT
#define WRAPPED_SKELETON(prefix, interfacenm) prefix ## interfacenm ## BOAImpl
#define WRAPPED_SKELETON_SUPER(prefix, interfacenm) public WRAPPED_SKELETON(prefix, interfacenm)
#define WRAPPED_SUPER(prefix, interfacenm) \
	prefix ## interfacenm ## BOAImpl ()
#define WRAPPED_SUPER_KEY(prefix, interfacenm, key) 	__WRAPPED_NULL_STMT
#define WRAPPED_OUTARG_TYPE(T)   T*&
// COBRA 2.x or 3.0 ??? requires last arg to be CORBA::Environment &
#define WRAPPED_DECLARG       , CORBA::Environment &IT_env = CORBA::IT_chooseDefaultEnv()
#define WRAPPED_DECLARG_VOID  CORBA::Environment &IT_env = CORBA::IT_chooseDefaultEnv()
#define WRAPPED_IMPLARG                , CORBA::Environment &IT_env
#define WRAPPED_IMPLARG_VOID           CORBA::Environment &IT_env
#define WRAPPED_DUPLICATE(T,objref)    T::_duplicate(objref)
#define WRAPPED_IMPL2OREF(T,objref)    T::_duplicate(objref)
#define WRAPPED_RELEASE(T,objref)      CORBA::release(objref)
#define WRAPPED_RELEASE_IMPL(T,objref) CORBA::release(objref)
#define WRAPPED_DISPOSE(objref)        delete objref
#define WRAPPED_DISPOSE_IMPL(objref)   delete objref
#define WRAPPED_RESOLVE_INITIAL_REFERENCES(orb,ident) BootStrapAgent::get(orb, ident)

#else
// omniORB2 or omniORB3

#ifdef __OMNIORB3__
#include <omniORB3/CORBA.h>
#include <omniORB3/Naming.hh>
#else
#include <omniORB2/CORBA.h>
#include <omniORB2/Naming.hh>
#endif

#define WRAPPED_ORB_SETMAXTCP(x)         omniORB::maxTcpConnectionPerServer = x
#define WRAPPED_SET_TCALIASEXPAND(x)     omniORB::tcAliasExpand = 1;
#define WRAPPED_IMPL_IS_READY_BLOCK(boa,servant_name)  boa->impl_is_ready(0,0)
// omniORB does not support this op:
#define WRAPPED_ORB_REGISTER_SERVANT_NAME(orb,servant_name) __WRAPPED_NULL_STMT
#define WRAPPED_BOA_OBJ_IS_READY(boa,objref)    objref->_obj_is_ready(boa)
#define WRAPPED_SKELETON(prefix, interfacenm) prefix ## _sk_ ## interfacenm
#define WRAPPED_SKELETON_SUPER(prefix, interfacenm) public virtual WRAPPED_SKELETON(prefix, interfacenm)
// omniorb is still CORBA 2.2
#define WRAPPED_DECLARG
#define WRAPPED_DECLARG_VOID void
#define WRAPPED_IMPLARG
#define WRAPPED_IMPLARG_VOID void

#define WRAPPED_DUPLICATE(T,objref)   T::_duplicate(objref)
#define WRAPPED_RELEASE(T,objref)     CORBA::release(objref)

// Things that differ between omniORB2 and omniORB3:

#ifdef __OMNIORB3__
#define WRAPPED_SUPER(prefix, interfacenm) \
	prefix ## _sk_ ## interfacenm ()
#define WRAPPED_SUPER_KEY(prefix, interfacenm, key) \
	omniOrbBoaServant(key)
#define WRAPPED_ORB_INIT(argc,argv)      CORBA::ORB_init(argc, argv, "omniORB3")
#define WRAPPED_BOA_INIT(orb,argc,argv)  orb->BOA_init(argc,argv,"omniORB3_BOA")
// Note: For omniORB2/3 you MUST NOT call delete on a CORBA object; use dispose / _dispose
#define WRAPPED_DISPOSE(objref)  CORBA::BOA::getBOA()->dispose(objref)
#define WRAPPED_DISPOSE_IMPL(objref) \
	CORBA::BOA::getBOA()->dispose(objref->_this())
#define WRAPPED_OUTARG_TYPE(T)   T ## _out
#define WRAPPED_ORB_BOA_CLEANUP(orb,boa) \
	if (1) { boa->destroy(); orb->destroy(); }
#define WRAPPED_IMPL2OREF(T,objref)    T::_duplicate(objref->_this())
#define WRAPPED_RELEASE_IMPL(T,objref) CORBA::release(objref->_this())

#else

#define WRAPPED_SUPER(prefix, interfacenm) \
	prefix ## _sk_ ## interfacenm ()
#define WRAPPED_SUPER_KEY(prefix, interfacenm, key) \
	prefix ## _sk_ ## interfacenm (key)
#define WRAPPED_ORB_INIT(argc,argv)      CORBA::ORB_init(argc, argv, "omniORB2")
#define WRAPPED_BOA_INIT(orb,argc,argv)  orb->BOA_init(argc,argv,"omniORB2_BOA")
// Note: For omniORB2/3 you MUST NOT call delete on a CORBA object; 
#define WRAPPED_DISPOSE(objref)      CORBA::BOA::getBOA()->dispose(objref)
#define WRAPPED_DISPOSE_IMPL(objref) CORBA::BOA::getBOA()->dispose(objref)
#define WRAPPED_OUTARG_TYPE(T)       T*&
#define WRAPPED_ORB_BOA_CLEANUP(orb,boa) \
	if (1) { boa->destroy(); orb->NP_destroy(); }
#define WRAPPED_IMPL2OREF(T,objref)    T::_duplicate(objref)
#define WRAPPED_RELEASE_IMPL(T,objref) CORBA::release(objref)
#endif

#define WRAPPED_RESOLVE_INITIAL_REFERENCES(orb,ident) \
     	orb->resolve_initial_references(ident)
#endif

//////////////////////////////////////////////////////////////////////////////
// Support for reconstructing an object with its former key 
// for Foo_impl which implements M::Foo,
//   (1) In .h  file for Foo_impl use:
//         WRAPPED_RECONSKEL_DECL(M,foo);
//
//         class Foo_impl: WRAPPED_RECONSKEL_SUPER(M,foo) {
//             Foo_impl(WRAPPED_OBJKEY_TYPE& key);
//             ...
//         };
//   (2) In .cc file for Foo_impl, include the .h file above and use:
//         Foo_impl::Foo_impl(WRAPPED_OBJKEY_TYPE& key)
//             : WRAPPED_RECONSKEL(M,foo) (key) { ..your constructor impl.. };

#if defined(__OMNIORB2__)
// the normal skeleton class is also a recon class -- just use typedef
#define WRAPPED_OBJKEY_TYPE omniORB::objectKey
#define WRAPPED_GENERATE_NEW_OBJKEY(key) omniORB::generateNewKey(key)
#define WRAPPED_OCTETSEQ_TYPE omniORB::seqOctets
#define WRAPPED_NEW_OCTETSEQ_FROM_KEY(key) omniORB::keyToOctetSequence(key)
#define WRAPPED_OCTETSEQ_TO_KEY(os,key) key = omniORB::octetSequenceToKey(os)
#define WRAPPED_RECONSKEL(prefix, interfacenm) \
  prefix ## _RECON_sk_ ## interfacenm
#define WRAPPED_RECONSKEL_DECL(prefix, interfacenm) \
  typedef WRAPPED_SKELETON(prefix, interfacenm)  WRAPPED_RECONSKEL(prefix, interfacenm)
#define WRAPPED_RECONSKEL_SUPER(prefix, interfacenm) \
  public virtual WRAPPED_RECONSKEL(prefix, interfacenm)
 
#elif defined(__OMNIORB3__)
// must define a recon class that inherits from the normal skeleton class
#define WRAPPED_OBJKEY_TYPE omniOrbBoaKey
#define WRAPPED_GENERATE_NEW_OBJKEY(key) omniORB::generateNewKey(key)
#define WRAPPED_OCTETSEQ_TYPE omniORB::seqOctets
#define WRAPPED_NEW_OCTETSEQ_FROM_KEY(key) omniORB::keyToOctetSequence(key)
#define WRAPPED_OCTETSEQ_TO_KEY(os,key) key = omniORB::octetSequenceToKey(os)
#define WRAPPED_RECONSKEL(prefix, interfacenm) \
  prefix ## _RECON_sk_ ## interfacenm
#define WRAPPED_RECONSKEL_DECL(prefix, interfacenm) \
  class WRAPPED_RECONSKEL(prefix, interfacenm) : public virtual WRAPPED_SKELETON(prefix, interfacenm) { \
    WRAPPED_RECONSKEL(prefix, interfacenm) (omniORB::objectKey& key) : omniOrbBoaServant(key) {}; }
#define WRAPPED_RECONSKEL_SUPER(prefix, interfacenm) \
  public virtual WRAPPED_RECONSKEL(prefix, interfacenm)

#endif

//////////////////////////////////////////////////////////////////////////////
// Support for non-blocking impl-is-ready

#include <omnithread.h>

class WRAPPED_impl_is_ready_thread_friend {
};

class WRAPPED_impl_is_ready_thread : public omni_thread 
{
  friend class WRAPPED_impl_is_ready_thread_friend;
  CORBA::BOA_ptr _boa;

  void* run_undetached(void* servant_name) 
  {
    	WRAPPED_IMPL_IS_READY_BLOCK(_boa, (const char*)servant_name);
    	int* rv = new int(0);
    	return (void*)rv;
  }

  // the destructor of a class that inherits from omni_thread should
  // never be public -- otherwise the thread object can be destroyed 
  // while the underlying thread is still running.

  ~WRAPPED_impl_is_ready_thread(void) {}

public:
  WRAPPED_impl_is_ready_thread(CORBA::BOA_ptr b, const char* s) : 
			omni_thread((void*)s), _boa(b) 
	{ cerr << "WRAPPED_impl_is_ready_thread : starting" << endl;
    	  start_undetached();
  	}
  void join_discard_result() {
    cerr << "WRAPPED_impl_is_ready_thread : about to call join" << endl;
    int* rv; join((void**)&rv); delete rv;
    cerr << "WRAPPED_impl_is_ready_thread : join returned" << endl;
  }
};

#ifdef __ORBIX3__ 

// a variable named v is introduced;  the matching _JOIN call must use 
// use same variable name and decl due to this _FORK must be in scope.
// NOTE: sleep is used to allow the new thread to execute its blocking
//       impl_is_ready call

#define WRAPPED_IMPL_IS_READY_FORK(boa,servant_name) \
  WRAPPED_impl_is_ready_thread* _WRAPPED_tmp_var = new WRAPPED_impl_is_ready_thread(boa, servant_name); \
  sleep(1)

#define WRAPPED_IMPL_IS_READY_JOIN \
  _WRAPPED_tmp_var->join_discard_result()

#else
// omniORB : use non-blocking impl_is_ready call to 'fork' and blocking
//           call to 'join'

#define WRAPPED_IMPL_IS_READY_FORK(boa,servant_name)  boa->impl_is_ready(0,1)
#define WRAPPED_IMPL_IS_READY_JOIN                    boa->impl_is_ready(0,0)

#endif

#endif


#if 0
XXX WE DO NOT NEED A TIMED BLOCK ?

(orbix)
#define WRAPPED_IMPL_IS_READY_TIMED_BLOCK(boa,servant_name,timeout) \
  		boa->impl_is_ready(servant_name, timeout)

(omniORB)
  // omniORB does not support timed impl_is_ready call --
  // this should do a fork join with a timer or something
#define WRAPPED_IMPL_IS_READY_TIMED_BLOCK(boa,servant_name,timeout) \
  if (1) { cerr << "omniORB does not support timed impl_is_ready call --" << endl << \
	     "this should do a fork join with a timer or something" << endl; exit(1); }

#endif
