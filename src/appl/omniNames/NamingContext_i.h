#ifndef _NamingContext_i_h_
#define _NamingContext_i_h_

#include "ReadersWritersLock.h"
#include "omniORB2/Naming.hh"
#include "log.h"

class NamingContext_i : public virtual CosNaming::_sk_NamingContext {

  friend class ObjectBinding;
  friend class log;

public:

  NamingContext_i(CORBA::BOA_ptr boa, const omniORB::objectKey& k,
		  log* l);

  //
  // IDL operations:
  //

  void bind(const CosNaming::Name& n, CORBA::Object_ptr obj) {
    bind_helper(n, obj, CosNaming::nobject, 0);
  }

  void rebind(const CosNaming::Name& n, CORBA::Object_ptr obj) {
    bind_helper(n, obj, CosNaming::nobject, 1);
  }

  void bind_context(const CosNaming::Name& n,
		    CosNaming::NamingContext_ptr nc) {
    bind_helper(n, nc, CosNaming::ncontext, 0);
  }

  void rebind_context(const CosNaming::Name& n,
		      CosNaming::NamingContext_ptr nc) {
    bind_helper(n, nc, CosNaming::ncontext, 1);
  }

  CORBA::Object_ptr resolve(const CosNaming::Name& n);

  void unbind(const CosNaming::Name& n);

  CosNaming::NamingContext_ptr new_context();

  CosNaming::NamingContext_ptr bind_new_context(const CosNaming::Name& n);

  void destroy();
  void list(CORBA::ULong how_many, CosNaming::BindingList*& bl,
	    CosNaming::BindingIterator_ptr& bi);


private:

  log* redolog;

  //
  // This multiple-readers, single-writer lock is used to control access to
  // all NamingContexts served by this process.
  //

  static ReadersWritersLock lock;

  //
  // We keep a list of all NamingContexts served by this process.  The
  // order in the list is simply that of creation.  This has nothing to do
  // with any naming hierarchy, except that the headContext is always the
  // "root" context since it is created first.
  //

  static NamingContext_i* headContext;
  static NamingContext_i* tailContext;
  NamingContext_i* next;
  NamingContext_i* prev;

  //
  // The following represent the list of bindings within this context.
  //

  ObjectBinding* headBinding;
  ObjectBinding* tailBinding;
  unsigned long size;

  //
  // These are private routines which do most of the job of the various
  // IDL operations.
  //

  void bind_helper(const CosNaming::Name& n, CORBA::Object_ptr obj,
		   CosNaming::BindingType t, CORBA::Boolean rebind);

  ObjectBinding* resolve_simple(const CosNaming::Name& name);

  CosNaming::NamingContext_ptr resolve_compound(const CosNaming::Name& name,
						CosNaming::Name& restOfName);

  // remember the destructor for an object should never be called explicitly.
  ~NamingContext_i();
};

#endif
