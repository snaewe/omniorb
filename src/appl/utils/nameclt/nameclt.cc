// -*- Mode: C++; -*-
//                          Package   : nameclt
// nameclt.cc               Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
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

#include <iostream.h>
#include <stdlib.h>
#include "omniORB2/Naming.hh"

static int advanced = 0;
static const char* command;

static void GetName(int argc, char** argv, int& n, int argsAfter,
		    const char* usageAfter, CosNaming::Name& name);

static void
usage(const char* progname)
{
  cerr << "\nusage: " << progname;

  if (advanced) {
    cerr << " [-advanced]";
  }

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

  if (advanced) {
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
  }

  cerr << "\n\n<object-name>,<context-name> :  name1_id name1_kind ..."
       << " nameN_id nameN_kind\n"
       << endl;
  exit(1);
}


int
main(int argc, char **argv)
{
  // Initialize the ORB, the object adaptor and the echo object:

  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

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

  rootContext = CosNaming::NamingContext::_narrow(initServ);
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

      if (argc == n) {

	context = rootContext;

      } else {

	CosNaming::Name name;
	GetName(argc, argv, n, 0, "", name);

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
	  cerr << "(" << (char*)(*bl)[i].binding_name[0].id << ","
	       << (char*)(*bl)[i].binding_name[0].kind << ") binding type "
	       << (((*bl)[i].binding_type == CosNaming::nobject) ?
		   "nobject" : "ncontext")
	       << endl;
	}

	delete bl;
	bl = 0;

      } while (bi->next_n(repeat_n, bl));

      bi->destroy();
      return 0;
    }


    //
    // bind_new_context
    // ----------------
    //

    if (strcmp(command, "bind_new_context") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 0, "", name);

      CosNaming::NamingContext_var context = rootContext->bind_new_context(name);

      char* p = orb->object_to_string(context);
      cerr << p << endl;
      return 0;
    }


    //
    // remove_context
    // --------------
    //
    // This actually does a destroy and unbind.
    //

    if (strcmp(command, "remove_context") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 0, "", name);

      CORBA::Object_var obj = rootContext->resolve(name);

      CosNaming::NamingContext_var context
	= CosNaming::NamingContext::_narrow(obj);

      if (CORBA::is_nil(context)) {
	cerr << "remove_context: not a naming context" << endl;
	exit(1);
      }

      context->destroy();

      rootContext->unbind(name);
      return 0;
    }


    //
    // bind
    // ----
    //

    if (strcmp(command, "bind") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 1, "objref", name);

      CORBA::Object_var obj;

      try {
	obj = orb->string_to_object(argv[n]);
      } catch (...) {
	cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	     << endl;
	exit(1);
      }

      rootContext->bind(name, obj);
      return 0;
    }


    //
    // unbind
    // -------
    //

    if (strcmp(command, "unbind") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 0, "", name);

      if (!advanced) {

	CORBA::Object_var obj = rootContext->resolve(name);

	CosNaming::NamingContext_var context
	  = CosNaming::NamingContext::_narrow(obj);

	if (!CORBA::is_nil(context)) {
	  cerr << "Error: unbind: can't unbind a naming context.\n"
	       << "Use remove_context to remove an empty naming context."
	       << endl;
	  exit(1);
	}
      }

      rootContext->unbind(name);
      return 0;
    }


    //
    // resolve
    // -------
    //

    if (strcmp(command, "resolve") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 0, "", name);

      CORBA::Object_var obj = rootContext->resolve(name);

      char* p = orb->object_to_string(obj);
      cout << p << endl;
      return 0;
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

      CosNaming::Name name;
      GetName(argc, argv, n, 1, "objref", name);

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
      return 0;
    }


    //
    // rebind
    // ------
    //

    if (strcmp(command, "rebind") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 1, "objref", name);

      CORBA::Object_var obj;

      try {
	obj = orb->string_to_object(argv[n]);
      } catch (...) {
	cerr << "Error: '" << argv[n] << "' is not a valid object reference"
	     << endl;
	exit(1);
      }

      rootContext->rebind(name, obj);
      return 0;
    }


    //
    // rebind_context
    // --------------
    //

    if (strcmp(command, "rebind_context") == 0) {

      CosNaming::Name name;
      GetName(argc, argv, n, 1, "objref", name);

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
      return 0;
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
      return 0;
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
      return 0;
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
    return 1;
  }

  catch (CosNaming::NamingContext::InvalidName) {
    cerr << command << ": InvalidName exception" << endl;
    return 1;
  }

  catch (CosNaming::NamingContext::AlreadyBound) {
    cerr << command << ": AlreadyBound exception" << endl;
    return 1;
  }

  catch (CosNaming::NamingContext::NotEmpty) {
    cerr << command << ": NotEmpty exception" << endl;
    return 1;
  }

  catch (CosNaming::NamingContext::CannotProceed) {
    cerr << command << ": CannotProceed exception" << endl;
    return 1;
  }
  catch (CORBA::COMM_FAILURE) {
    cerr << command 
	 << ": Cannot contact the Naming Service because of COMM_FAILURE."
	 << endl;
    return 1;
  }

  catch (...) {
    // Sigh! Cannot emit an error message here or gcc-2.7.2 gives
    // internal compiler error.
  }
  cerr << command
       << ": Unexpected error encountered."
       << endl;
  return 1;
}



static void
GetName(int argc, char** argv, int& n, int argsAfter, const char* usageAfter,
	CosNaming::Name& name)
{
  int n_args = (argc - n - argsAfter);

  if (((n_args % 2) != 0) || (n_args < 2)) {
    cerr << "usage: " << argv[0] << " " << command
	 << " name1_id name1_kind ... nameN_id nameN_kind " << usageAfter
	 << endl;
    exit(1);
  }

  int n_components = n_args / 2;

  name.length(n_components);

  for (int i = 0; i < n_components; i++) {
    name[i].id   = (const char*) argv[n++];
    name[i].kind = (const char*) argv[n++];
  }
}
