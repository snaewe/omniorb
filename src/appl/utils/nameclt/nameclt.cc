#include <iostream.h>
#include <stdlib.h>
#include "omniORB2/Naming.hh"

static void
usage()
{
  cerr << "usage: nameclt [-ior <NameService-object-reference>] <operation>"
       << endl;
  exit(1);
}


int
main(int argc, char **argv)
{
  // Initialize the ORB, the object adaptor and the echo object:

  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  if (argc < 2) usage();
  int n = 1;


  //
  // Obtain a reference to the root context of the Name service.  Get it
  // as an Object_ptr then narrow it to a NamingContext_ptr.
  //

  CosNaming::NamingContext_var rootContext;

  try {
    CORBA::Object_var initServ;

    if (strcmp(argv[n], "-ior") == 0) {
      if (argc < 4) usage();
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
      cerr << "bind usage: name1.id name1.kind ... nameN.kind objref"
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
      cerr << "rebind usage: name1.id name1.kind ... nameN.kind objref"
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
      cerr << "bind_context usage: name1.id name1.kind ... nameN.kind objref"
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
      cerr << "rebind_context usage: name1.id name1.kind ... nameN.kind objref"
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
      cerr << "resolve usage: name1.id name1.kind ... nameN.kind"
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
      cerr << "unbind usage: name1.id name1.kind ... nameN.kind"
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
      cerr << "bind_new_context usage: name1.id name1.kind ... nameN.kind"
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
  // destroy
  // -------
  //

  else if (strcmp(argv[n], "destroy") == 0) {

    n++;

    if (argc > n) {
      cerr << "destroy takes no args" << endl;
      return 1;
    }

    rootContext->destroy();
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
/*
    if (argc > 3)
      first_n = atoi(argv[3]);
    if (argc > 4)
      repeat_n = atoi(argv[4]);
*/      
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

  return 0;
}
