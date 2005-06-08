// -*- Mode: C++; -*-
//                          Package   : nameclt
// nameclt.cc               Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of nameclt.
//
//  Nameclt is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

#include <string.h>
#include <stdlib.h>
#include <omniORB4/Naming.hh>
#include <omniORB4/omniURI.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

OMNI_USING_NAMESPACE(omni)

static int advanced = 0;
static const char* command;

static void
usage(const char* progname)
{
  cerr << "\nusage: " << progname;

  cerr << " [-advanced]";

  cerr << " [-ior <NameService-object-reference>] <operation>"
       << "\n\nwhere <operation> is one of:\n"
       << "\n list <context-name>"
       << "\n    (lists contexts and objects bound to the context with the"
       << " specified name)"
       << "\n bind_new_context <context-name>"
       << "\n    (binds name to a new context, and returns the stringified"
       << " context IOR)"
       << "\n remove_context <context-name>"
       << "\n    (unbinds and destroys the named context)"
       << "\n bind <object-name> <stringified-IOR>"
       << "\n    (binds name to object)"
       << "\n unbind <object-name>"
       << "\n    (unbinds name and object)"
       << "\n resolve <object-name>"
       << "\n    (returns stringified IOR bound to specified name)";

  cerr << "\n\nAdvanced operations:\n"
       << "\n bind_context <context-name> <stringified-context-IOR>"
       << "\n    (binds name to context)"
       << "\n rebind <object-name> <stringified-IOR>"
       << "\n    (binds name to object even if binding already exists)"
       << "\n rebind_context <context-name> <stringified-context-IOR>"
       << "\n    (binds name to context even if binding already exists)"
       << "\n new_context"
       << "\n    (returns stringified IOR for a new context)"
       << "\n destroy"
       << "\n    (destroys the naming context given with -ior flag)";

  cerr << "\n\n<object-name>,<context-name> :  name1_id.kind/.../nameN_id.kind"
       << "\n" << endl;
  exit(1);
}


int
main(int argc, char **argv)
{
  // Initialize the ORB, the object adaptor and the echo object:

  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB4");

  {
    const char* ior = (const char*)0;
    int n;

    for (n = 1; n < argc; n++) {

      if (argv[n][0] != '-')
	break;

      if (strcmp(argv[n], "-ior") == 0) {
	n++;
	if (n == argc) usage(argv[0]);
	ior = argv[n];

      } else if (strcmp(argv[n], "-advanced") == 0) {
	advanced = 1;

      } else {
	usage(argv[0]);
      }
    }

    if (n == argc) usage(argv[0]);


    //
    // Obtain a reference to the root context of the Name service.  Get it
    // as an Object_ptr then narrow it to a NamingContext_ptr.
    //

    CosNaming::NamingContext_var rootContext;
    CORBA::Object_var initServ;

    if (ior != (const char*)0) {
      try {
	initServ = orb->string_to_object(ior);
      } catch (...) {
	cerr << "Error: '" << ior << "' is not a valid object reference" << endl;
	exit(1);
      }
    } else {
      try {
	initServ = orb->resolve_initial_references("NameService");
      }
      catch(CORBA::ORB::InvalidName& ex) {
	cerr << "Service required is invalid [does not exist]." << endl;
	exit(1);
      }
      catch(CORBA::NO_RESOURCES& ex) {
	cerr << "Cannot resolve the root context." << endl;
	cerr << "Have you set up the configuration file properly?" << endl;
	exit(1);
      }
      catch(...) {
	cerr << "Unexpected error encountered while resolving the root context."
	     << endl;
	exit(1);
      }
    }

    try {
      rootContext = CosNaming::NamingContext::_narrow(initServ);
    }
    catch (CORBA::TRANSIENT& ex) {
      cerr << "Caught a TRANSIENT exception when trying to validate the "
	   << "type of the " << endl
	   << "NamingContext. Is the naming service running?" << endl;
      exit(1);
    }
    catch (CORBA::Exception& ex) {
      cerr << "Unexpected CORBA " << ex._name()
	   << " exception when trying to narrow the NamingContext." << endl;
      exit(1);
    }
    catch(...) {
      cerr << "Unexpected error when trying to narrow the NamingContext."
	   << endl;
      exit(1);
    }
    if (CORBA::is_nil(rootContext)) {
      cerr << "NameService object reference was not a NamingContext." << endl;
      exit(1);
    }

    command = argv[n];
    n++;


    try {


      //
      // list
      // ----
      //

      if (strcmp(command, "list") == 0) {

	CosNaming::NamingContext_var context;

	switch (argc-n) {

	case 0:
	  context = rootContext;
	  break;

	case 1:
	  {
	    CosNaming::Name_var name = omniURI::stringToName(argv[n]);

	    if (name->length() == 0) {
	      context = rootContext;
	    } else {
	      CORBA::Object_var obj = rootContext->resolve(name);

	      context = CosNaming::NamingContext::_narrow(obj);

	      if (CORBA::is_nil(context)) {
		cerr << "list: not a naming context" << endl;
		exit(1);
	      }
	    }
	    break;
	  }

	default:
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind" << endl;
	  exit(1);
	}

	CosNaming::BindingIterator_var bi;
	CosNaming::BindingList_var bl;
	CosNaming::Binding_var b;

	context->list(0, bl, bi);

	if (CORBA::is_nil(bi))
	  goto done;

	while (bi->next_one(b)) {

	  CORBA::String_var sname = omniURI::nameToString(b->binding_name);

	  cout << (const char*)sname;

	  if (b->binding_type == CosNaming::ncontext)
	    cout << "/";

	  cout << endl;
	}

	bi->destroy();
	goto done;
      }


      //
      // bind_new_context
      // ----------------
      //

      if (strcmp(command, "bind_new_context") == 0) {

	if (argc != n+1) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n]);

	CosNaming::NamingContext_var context
	  = rootContext->bind_new_context(name);

	char* p = orb->object_to_string(context);
	cout << p << endl;
	goto done;
      }


      //
      // remove_context
      // --------------
      //
      // This actually does a destroy and unbind.
      //

      if (strcmp(command, "remove_context") == 0) {

	if (argc != n+1) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n]);

	CORBA::Object_var obj = rootContext->resolve(name);

	CosNaming::NamingContext_var context
	  = CosNaming::NamingContext::_narrow(obj);

	if (CORBA::is_nil(context)) {
	  cerr << "remove_context: not a naming context" << endl;
	  exit(1);
	}

	context->destroy();

	rootContext->unbind(name);
	goto done;
      }


      //
      // bind
      // ----
      //

      if (strcmp(command, "bind") == 0) {

	if (argc != n+2) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind objref" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n++]);

	CORBA::Object_var obj;

	try {
	  obj = orb->string_to_object(argv[n]);
	} catch (...) {
	  cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	       << endl;
	  exit(1);
	}

	rootContext->bind(name, obj);
	goto done;
      }


      //
      // unbind
      // -------
      //

      if (strcmp(command, "unbind") == 0) {

	if (argc != n+1) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n]);

	if (advanced) {

	  // Don't perform any safety checks for -advanced

	  rootContext->unbind(name);
	  goto done;
	}

	//
	// Without -advanced, list parent context and check that binding type is
	// nobject before allowing unbind.
	//

	CosNaming::NamingContext_var context;
	int len = name->length();
	CORBA::String_var id = name[len-1].id;
	CORBA::String_var kind = name[len-1].kind;

	if (len > 1) {
	  name->length(len-1);

	  CORBA::Object_var obj = rootContext->resolve(name);

	  context = CosNaming::NamingContext::_narrow(obj);

	  if (CORBA::is_nil(context)) {
	    cerr << "unbind: parent is not a naming context" << endl;
	    exit(1);
	  }
	} else {
	  context = rootContext;
	}

	CosNaming::BindingIterator_var bi;
	CosNaming::BindingList_var bl;
	CosNaming::Binding_var b;

	context->list(0, bl, bi);

	while (bi->next_one(b)) {

	  if (strcmp(b->binding_name[0].id, id) == 0 &&
	      strcmp(b->binding_name[0].kind, kind) == 0) {

	    bi->destroy();

	    if (b->binding_type == CosNaming::ncontext) {
	      cerr << "Error: unbind: can't unbind a naming context.\n"
		   << "Use remove_context to remove an empty naming context."
		   << endl;
	      exit(1);
	    }

	    context->unbind(b->binding_name);
	    goto done;
	  }

	}
	bi->destroy();

	cerr << "Error: unbind: couldn't find binding" << endl;
	return 1;
      }


      //
      // resolve
      // -------
      //

      if (strcmp(command, "resolve") == 0) {

	if (argc != n+1) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n]);

	CORBA::Object_var obj = rootContext->resolve(name);

	char* p = orb->object_to_string(obj);
	cout << p << endl;
	goto done;
      }



      /////////////////////////
      //
      // Advanced operations
      //
      /////////////////////////

      if (!advanced) usage(argv[0]);


      //
      // bind_context
      // ------------
      //

      if (strcmp(command, "bind_context") == 0) {

	if (argc != n+2) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind objref" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n++]);

	CORBA::Object_var obj;

	try {
	  obj = orb->string_to_object(argv[n]);
	} catch (...) {
	  cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	       << endl;
	  exit(1);
	}

	CosNaming::NamingContext_var context
	  = CosNaming::NamingContext::_narrow(obj);

	if (CORBA::is_nil(context)) {
	  cerr << "Error: bind_context: not a naming context" << endl;
	  exit(1);
	}

	rootContext->bind_context(name, context);
	goto done;
      }


      //
      // rebind
      // ------
      //

      if (strcmp(command, "rebind") == 0) {

	if (argc != n+2) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind objref" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n++]);

	CORBA::Object_var obj;

	try {
	  obj = orb->string_to_object(argv[n]);
	} catch (...) {
	  cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	       << endl;
	  exit(1);
	}

	rootContext->rebind(name, obj);
	goto done;
      }


      //
      // rebind_context
      // --------------
      //

      if (strcmp(command, "rebind_context") == 0) {

	if (argc != n+2) {
	  cerr << "usage: " << argv[0] << " " << command
	       << " name1_id.kind/.../nameN_id.kind objref" << endl;
	  exit(1);
	}

	CosNaming::Name_var name = omniURI::stringToName(argv[n++]);

	CORBA::Object_var obj;

	try {
	  obj = orb->string_to_object(argv[n]);
	} catch (...) {
	  cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	       << endl;
	  exit(1);
	}

	CosNaming::NamingContext_var context
	  = CosNaming::NamingContext::_narrow(obj);

	if (CORBA::is_nil(context)) {
	  cerr << "Error: rebind_context: not a naming context" << endl;
	  exit(1);
	}

	rootContext->rebind_context(name, context);
	goto done;
      }


      //
      // new_context
      // -----------
      //

      if (strcmp(command, "new_context") == 0) {

	if (argc > n) {
	  cerr << "Error: new_context takes no args" << endl;
	  return 1;
	}

	CosNaming::NamingContext_var context = rootContext->new_context();

	char* p = orb->object_to_string(context);
	cout << p << endl;
	goto done;
      }


      //
      // destroy
      // -------
      //

      if (strcmp(command, "destroy") == 0) {

	if (argc > n) {
	  cerr << "Error: destroy takes no args" << endl;
	  return 1;
	}

	rootContext->destroy();
	goto done;
      }

      usage(argv[0]);
    }

    catch (CosNaming::NamingContext::NotFound& ex) {
      cerr << command << ": NotFound exception: ";
      if (ex.why == CosNaming::NamingContext::missing_node) {
	cerr << "missing node";
      } else if (ex.why == CosNaming::NamingContext::not_context) {
	cerr << "not context";
      } else if (ex.why == CosNaming::NamingContext::not_object) {
	cerr << "not object";
      }
      cerr << endl;
      goto error_return;
    }

    catch (CosNaming::NamingContext::InvalidName) {
      cerr << command << ": InvalidName exception" << endl;
      goto error_return;
    }

    catch (CosNaming::NamingContext::AlreadyBound) {
      cerr << command << ": AlreadyBound exception" << endl;
      goto error_return;
    }

    catch (CosNaming::NamingContext::NotEmpty) {
      cerr << command << ": NotEmpty exception" << endl;
      goto error_return;
    }

    catch (CosNaming::NamingContext::CannotProceed) {
      cerr << command << ": CannotProceed exception" << endl;
      goto error_return;
    }

    catch (CORBA::TRANSIENT) {
      cerr << command 
	   << ": Cannot contact the Naming Service because of "
	   << "TRANSIENT exception." << endl
	   << "Is the naming service running?" << endl;
      goto error_return;
    }

    catch (CORBA::Exception& ex) {
      cerr << command 
	   << ": Cannot contact the Naming Service because of "
	   << ex._name() << " exception." << endl;
      goto error_return;
    }

    catch (...) {
      // Sigh! Cannot emit an error message here or gcc-2.7.2 gives
      // internal compiler error.
    }
    cerr << command
	 << ": Unexpected error encountered."
	 << endl;
  }
 error_return:
  orb->destroy();
  return 1;

 done:
  orb->destroy();
  return 0;
}
