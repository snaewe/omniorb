#include <iostream.h>
#include <stdlib.h>
#ifdef __NT__
#include "omniORB2/Naming_NT.hh"
#else
#include "omniORB2/Naming.hh"
#endif

static void
usage(const char* progname)
{
  cerr << "usage: " << progname << " [-ior <NameService-object-reference>] <operation> [object name]"
       << "\nwhere <operation> is one of: "
       << "\n\n bind <object name> <stringified IOR>"
       << "\n    (bind name and object)"
       << "\n rebind <object name> <stringified IOR>"
       << "\n    (bind name and object even if binding already exists)"
       << "\n bind_context <object name> <stringified context IOR>"
       << "\n    (bind name and context)"
       << "\n rebind_context <object name> <stringified context IOR>"
       << "\n    (bind name and context even if binding already exists)"
       << "\n new_context"
       << "\n    (returns stringified IOR for a new context)"
       << "\n bind_new_context <context name>"
       << "\n    (binds name to a new context, and returns the stringified context IOR)"
       << "\n resolve <object name>"
       << "\n    (returns stringified IOR bound to specified name)"
       << "\n unbind <object name>"
       << "\n    (unbinds name and object)"
       << "\n list <context name>"
       << "\n    (lists contexts and objects bound to the context with the specified name)"
       << "\n\n    <object name>, <context name> :  name1_id name1_kind name2_id name2_kind ..."
       << "\n     nameN_id NameN_kind"
       << endl;
exit(1);
}


int
main(int argc, char **argv)
{
  // Initialize the ORB, the object adaptor and the echo object:

  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  if (argc < 2) usage(argv[0]);
  int n = 1;


  //
  // Obtain a reference to the root context of the Name service.  Get it
  // as an Object_ptr then narrow it to a NamingContext_ptr.
  //

  CosNaming::NamingContext_var rootContext;

  try {
    CORBA::Object_var initServ;

    if (strcmp(argv[n], "-ior") == 0) {
      if (argc < 4) usage(argv[0]);
      n++;
      initServ = orb->string_to_object(argv[n]);
      n++;
    } else {
      initServ = orb->resolve_initial_references("NameService");
    }

    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) {
      cerr << "NameService object reference was not a NamingContext." << endl;
      exit(1);
    }
  }
  catch(CORBA::ORB::InvalidName& ex) {
      cerr << "Service required is invalid [does not exist]." << endl;
      exit(1);
  }


  //
  // bind
  // ----
  //

  if (strcmp(argv[n], "bind") == 0) {

    n++;

    int n_components = (argc - n - 1) / 2;

    if (n_components < 1) {
      cerr << "bind usage: " << argv[0] <<" bind name1_id name1_kind ... nameN_kind objref"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CORBA::Object_var obj = orb->string_to_object(argv[n]);

    rootContext->bind(name, obj);
  }


  //
  // rebind
  // ------
  //

  else if (strcmp(argv[n], "rebind") == 0) {

    n++;

    int n_components = (argc - n - 1) / 2;

    if (n_components < 1) {
      cerr << "rebind usage: " << argv[0] << " rebind name1_id name1_kind ... nameN_kind objref"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CORBA::Object_var obj = orb->string_to_object(argv[n]);

    rootContext->rebind(name, obj);
  }


  //
  // bind_context
  // ------------
  //

  else if (strcmp(argv[n], "bind_context") == 0) {

    n++;

    int n_components = (argc - n - 1) / 2;

    if (n_components < 1) {
      cerr << "bind_context usage: " << argv[0] << " bind_context name1_id name1_kind ... nameN_kind objref"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CORBA::Object_var obj = orb->string_to_object(argv[n]);

    CosNaming::NamingContext_var context
      = CosNaming::NamingContext::_narrow(obj);

    if (CORBA::is_nil(context)) {
      cerr << "bind_context: not a naming context" << endl;
      exit(1);
    }

    rootContext->bind_context(name, context);
  }


  //
  // rebind_context
  // --------------
  //

  else if (strcmp(argv[n], "rebind_context") == 0) {

    n++;

    int n_components = (argc - n - 1) / 2;

    if (n_components < 1) {
      cerr << "rebind_context usage: " << argv[0] << " rebind_context name1_id name1_kind ... nameN_kind objref"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CORBA::Object_var obj = orb->string_to_object(argv[n]);

    CosNaming::NamingContext_var context
      = CosNaming::NamingContext::_narrow(obj);

    if (CORBA::is_nil(context)) {
      cerr << "rebind_context: not a naming context" << endl;
      exit(1);
    }

    rootContext->rebind_context(name, context);
  }


  //
  // resolve
  // -------
  //

  else if (strcmp(argv[n], "resolve") == 0) {

    n++;

    int n_components = (argc - n) / 2;

    if (n_components < 1) {
      cerr << "resolve usage: " << argv[0] << " resolve name1_id name1_kind ... nameN_kind"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CORBA::Object_var obj = rootContext->resolve(name);

    char* p = orb->object_to_string(obj);
    cerr << p << endl;
  }


  //
  // unbind
  // -------
  //

  else if (strcmp(argv[n], "unbind") == 0) {

    n++;

    int n_components = (argc - n) / 2;

    if (n_components < 1) {
      cerr << "unbind usage: " << argv[0] << " unbind name1_id name1_kind ... nameN_kind"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    rootContext->unbind(name);
  }


  //
  // new_context
  // -----------
  //

  else if (strcmp(argv[n], "new_context") == 0) {

    n++;

    if (argc > n) {
      cerr << "new_context takes no args" << endl;
      return 1;
    }

    CosNaming::NamingContext_var context = rootContext->new_context();

    char* p = orb->object_to_string(context);
    cerr << p << endl;
  }


  //
  // bind_new_context
  // ----------------
  //

  else if (strcmp(argv[n], "bind_new_context") == 0) {

    n++;

    int n_components = (argc - n) / 2;

    if (n_components < 1) {
      cerr << "bind_new_context usage: " << argv[0] << " bind_new_context name1_id name1_kind ... nameN_kind"
	   << endl;
      return 1;
    }

    CosNaming::Name name(n_components);
    name.length(n_components);

    for (int i = 0; i < n_components; i++) {
      name[i].id   = (const char*) argv[n++];
      name[i].kind = (const char*) argv[n++];
    }

    CosNaming::NamingContext_var context = rootContext->bind_new_context(name);

    char* p = orb->object_to_string(context);
    cerr << p << endl;
  }


  //
  // list
  // ----
  //

  else if (strcmp(argv[n], "list") == 0) {

    n++;

    CosNaming::NamingContext_var context;

    int n_components = (argc - n) / 2;

    if (n_components == 0) {

      context = rootContext;

    } else {
      CosNaming::Name name(n_components);
      name.length(n_components);

      for (int i = 0; i < n_components; i++) {
	name[i].id   = (const char*) argv[n++];
	name[i].kind = (const char*) argv[n++];
      }

      CORBA::Object_var obj = rootContext->resolve(name);

      context = CosNaming::NamingContext::_narrow(obj);

      if (CORBA::is_nil(context)) {
	cerr << "list: not a naming context" << endl;
	exit(1);
      }
    }

    CosNaming::BindingList* bl = 0;
    CosNaming::BindingIterator_ptr bi = 0;
    int first_n = 5, repeat_n = 3;

    context->list(first_n, bl, bi);

    do {
      for (unsigned int i = 0; i < bl->length(); i++) {
	cerr << "(" << (*bl)[i].binding_name[0].id << ","
	     << (*bl)[i].binding_name[0].kind << ") binding type "
	     << (((*bl)[i].binding_type == CosNaming::nobject) ?
		 "nobject" : "ncontext")
	     << endl;
      }

      delete bl;
      bl = 0;

    } while (bi->next_n(repeat_n, bl));

    bi->destroy();

  }
  else usage(argv[0]);
  return 0;
}
