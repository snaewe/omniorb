// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_operation.cc        Created on: 23/08/1996
//                          Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//
//

/*
  $Log$
  Revision 1.10  1997/05/02 16:27:29  sll
  The string that sent over the wire to identify an operation name that
  clashes with a C++ reserved word should not be prepended with _.

// Revision 1.9  1997/04/29  12:30:39  sll
// Removed name ambiguity.
//
// Revision 1.8  1997/04/23  14:31:16  sll
// - added support for LOCATION_FORWARD message
// - added code to assert object existent when first do a remote call.
//
// Revision 1.7  1997/03/10  16:43:05  sll
// - New member function produce_mapping_with_indirection. This function produce
//   the mapping for an operation that uses the adaptation classes for passing
//   variable length INOUT and OUT arguments.
// - New boolean member functions has_variable_out_arg, has_pointer_inout_arg
//   to tell whether the operation contains these argument types.
// - Minor changes to accommodate the creation of a public API for omniORB2.
//
// Revision 1.6  1997/02/17  18:11:33  ewc
// IDL Compiler now adds a dummy return after some exceptions - this
//  stops some C++ compilers from complaining (e.g. MSVC++ 4.2) about
// some control paths not returning values.
//
  Revision 1.5  1997/01/28 18:36:24  sll
  Fixed the bugs in the proxy and the server skeleton code which only
  affects oneway operation.

  Revision 1.4  1997/01/24 19:40:46  sll
  The operations for nil object now return properly initialised result values.
  Fixed a bug in the marshalling of object reference and string as INOUT arg.

  Revision 1.3  1997/01/23 17:06:56  sll
  Now do the right thing to initialise typedefed arrays in result and out
  arguments.

// Revision 1.2  1997/01/13  15:30:24  sll
// In server skeleton code, don't use operator->() for results and INOUT
// arguments if the type is array_slice.
// Bug fixed at various places to get to the true type of a typedef before
// calling the narrow_from_decl function of the true type.
//
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

o2be_operation::o2be_operation(AST_Type *rt, AST_Operation::Flags fl,
			       UTL_ScopedName *n, UTL_StrList *p)
                : AST_Operation(rt, fl, n, p),
		  AST_Decl(AST_Decl::NT_op, n, p),
		  UTL_Scope(AST_Decl::NT_op),
		  o2be_name(this)
{
  o2be_interface *intf = o2be_interface::narrow_from_scope(defined_in());
  if (o2be_interface::check_opname_clash(intf,uqname())) {
    idl_global->err()->operation_name_clash(this);
  }
  if (flags() == AST_Operation::OP_oneway) {
    if (!no_user_exception()) {
      idl_global->err()->syntax_error(idl_global->parse_state());
    }
  }
}

void
o2be_operation::produce_decl(fstream &s,
			     const char *prefix,
			     const char *alias_prefix,
			     idl_bool out_var_default /* ignored */)
{
  if (context())
    throw o2be_unsupported(idl_global->stripped_filename()->get_string(),
			   line(),
			   "context argument");

  // return type
  if (!return_is_void())
    {
      argMapping mapping;
      argType    ntype;

      ntype = ast2ArgMapping(return_type(),wResult,mapping);	

      if (ntype == tObjref) {
	AST_Decl *decl = return_type();
	while (decl->node_type() == AST_Decl::NT_typedef) {
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	}
	s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
      }
      else if (ntype == tString) {
	s << "char *";
      }
      else {
	s << o2be_name::narrow_and_produce_fqname(return_type());
      }
      s << ((mapping.is_arrayslice) ? "_slice":"")
	<< " "
	<< ((mapping.is_pointer)    ? "*":"")
	<< ((mapping.is_reference)  ? "&":"");
    }
  else
    {
      s << "void";
    }
  // function name
  s << " ";
  if (prefix)
    s << prefix << "::";
  s << ((alias_prefix)? alias_prefix : "");
  s << uqname() << " ( ";

  // argument list
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType    ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_IN:
	    ntype = ast2ArgMapping(a->field_type(),wIN,mapping);
	    break;
	  case AST_Argument::dir_OUT:
	    ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	    break;
	  case AST_Argument::dir_INOUT:
	    ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	    break;
	  }
	s << ((mapping.is_const) ? "const ":"");
	if (ntype == tObjref) {
	  AST_Decl *decl = a->field_type();
	  while (decl->node_type() == AST_Decl::NT_typedef) {
	    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	  }
	  s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
	}
	else if (ntype == tString) {
	  s << "char *";
	}
	else {
	  s << o2be_name::narrow_and_produce_fqname(a->field_type());
	}
	s << ((mapping.is_arrayslice) ? "_slice":"")
	  << " "
	  << ((mapping.is_pointer)    ? "*":"")
	  << ((mapping.is_reference)  ? "&":"");
	s << " " << a->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " :"");
      }
  }
  s << " )";
  return;
}

void
o2be_operation::produce_invoke(fstream &s)
{
  s << uqname() << " ( ";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	s << a->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " :"");
      }
  }
  s << " )";
  return;
}

void
o2be_operation::produce_proxy_skel(fstream &s,o2be_interface &defined_in,
				   const char* alias_prefix)
{
  idl_bool hasVariableLenOutArgs = I_FALSE;

  IND(s); produce_decl(s,defined_in.proxy_fqname(),alias_prefix,I_FALSE);
  s << "\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "assertObjectExistent();\n";
  IND(s); s << "omniRopeAndKey _r;\n";
  IND(s); s << "CORBA::Boolean _fwd = getRopeAndKey(_r);\n";

  // Declare a local variable for result and variable length OUT arguments.
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	if (a->direction() == AST_Argument::dir_OUT)
	  {
	    ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	    if (ntype == tObjref || ntype == tString ||
		(mapping.is_arrayslice) ||
		(mapping.is_reference && mapping.is_pointer)) 
	      {
		hasVariableLenOutArgs = I_TRUE;
		// Declare a local pointer variable
		IND(s);
		declareVarType(s,a->field_type(),0,mapping.is_arrayslice);
		s << ((ntype != tObjref && ntype != tString)?" *":"") 
		  << " _" << a->uqname() << "= 0;\n";
	      }
	  }
	i.next();
      }
  }
  if (!return_is_void()) 
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  hasVariableLenOutArgs = I_TRUE;
	  IND(s);
	  declareVarType(s,return_type(),0,mapping.is_arrayslice);
	  s << ((ntype != tObjref && ntype != tString)?" *":"") 
	    << " _result" << "= 0;\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type());
	  s << " _result;\n";
	}
    }

  IND(s); s << "try {\n";
  INC_INDENT_LEVEL();

  IND(s); s << "GIOP_C _c(_r.rope());\n";
  
  // calculate request message size
  IND(s); s << "CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(_r.keysize(),"
	    << strlen(local_name()->get_string()) + 1 
	    << ");\n";

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_IN:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wIN,mapping);
	      produceSizeCalculation(s,a->field_type(),"_c","_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceSizeCalculation(s,a->field_type(),"_c","_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    break;
	  }
	i.next();
      }
  }


  IND(s); s << "_c.InitialiseRequest(_r.key(),_r.keysize(),(char *)\""
	    << local_name()->get_string() << "\"," << strlen(local_name()->get_string()) + 1 << ",_msgsize,"
	    << ((flags() == AST_Operation::OP_oneway)?"1":"0")
	    << ");\n";

  // marshall arguments;
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_IN:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wIN,mapping);
	      produceMarshalCode(s,a->field_type(),"_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceMarshalCode(s,a->field_type(),"_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "switch (_c.ReceiveReply())\n";  // invoke method
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();

  IND(s); s << "case GIOP::NO_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  // unmarshall results
  if (hasVariableLenOutArgs)
    {
      {
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    argMapping mapping;
	    argType ntype;

	    o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	    if (a->direction() == AST_Argument::dir_OUT)
	      {
		ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
		if (mapping.is_arrayslice) {
		  IND(s); s << "_" << a->uqname() << " = ";
		  AST_Decl *truetype = a->field_type();
		  while (truetype->node_type() == AST_Decl::NT_typedef) {
		    truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
		  }
		  s << o2be_array::narrow_from_decl(truetype)->fqname()
		    << "_alloc();\n";
		}
		else if (mapping.is_reference && mapping.is_pointer) {
		  IND(s); s << "_" << a->uqname() << " = new ";
		  declareVarType(s,a->field_type());
		  s << ";\n";
		}
	      }
	    i.next();
	  }
      }
      if (!return_is_void())
	{
	  argMapping mapping;
	  argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
	  if (mapping.is_arrayslice) {
	    IND(s); s << "_result = ";
	    AST_Decl *truetype = return_type();
	    while (truetype->node_type() == AST_Decl::NT_typedef) {
	      truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
	    }
	    s << o2be_array::narrow_from_decl(truetype)->fqname()
	      << "_alloc();\n";

	  }
	  else if (mapping.is_pointer) {
	    IND(s); s << "_result = new ";
	    declareVarType(s,return_type());
	    s << ";\n";
	  }
	}
    }

  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      produceUnMarshalCode(s,return_type(),"_c","_result",ntype,mapping);
    }


  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      switch (ntype) 
		{
		case tObjref:
		  {
		    char *_argname = new char[strlen(a->uqname())+2];
		    strcpy(_argname,"_");
		    strcat(_argname,a->uqname());
		    IND(s);
		    declareVarType(s,a->field_type(),0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),"_c",_argname,
					 ntype,mapping);
		    IND(s); s << "CORBA::release(" << a->uqname() << ");\n";
		    IND(s); s << a->uqname() << " = " << _argname << ";\n";
		    delete [] _argname;
		  }
		  break;
		case tString:
		  {
		    char *_argname = new char[strlen(a->uqname())+2];
		    strcpy(_argname,"_");
		    strcat(_argname,a->uqname());
		    IND(s);
		    declareVarType(s,a->field_type(),0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),"_c",_argname,
					 ntype,mapping);
		    IND(s); s << "CORBA::string_free(" << a->uqname() << ");\n";
		    IND(s); s << a->uqname() << " = " << _argname << ";\n";
		    delete [] _argname;
		  }
		  break;
		default:
		  produceUnMarshalCode(s,a->field_type(),"_c",a->uqname(),
				       ntype,mapping);
		  break;
		}
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if (ntype == tObjref || ntype == tString ||
		  (mapping.is_arrayslice) ||
		  (mapping.is_reference && mapping.is_pointer)) 
		{
		  char *_argname = new char[strlen(a->uqname())+2];
		  strcpy(_argname,"_");
		  strcat(_argname,a->uqname());
		  produceUnMarshalCode(s,a->field_type(),"_c",_argname,
				       ntype,mapping);
		  delete [] _argname;
		}
	      else
		{
		  produceUnMarshalCode(s,a->field_type(),"_c",a->uqname(),
				       ntype,mapping);
		}
	      break;
	    }
	  case AST_Argument::dir_IN:
	    break;
	  }
	i.next();
      }
  }

  IND(s); s << "_c.RequestCompleted();\n";

  if (hasVariableLenOutArgs)
    {
      {
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    argMapping mapping;
	    argType ntype;

	    o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	    if (a->direction() == AST_Argument::dir_OUT)
	      {
		ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
		if (ntype == tObjref || ntype == tString ||
		    (mapping.is_arrayslice) ||
		    (mapping.is_reference && mapping.is_pointer)) 
		  {
		    IND(s); s << a->uqname() << " = _" << a->uqname() << ";\n";
		  }
	      }
	    i.next();
	  }
      }
      if (!return_is_void()) {
	IND(s); s << "return _result;\n";
      }
    }
  else
    {
      if (!return_is_void()) {
	IND(s); s << "return _result;\n";
      }
    }
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  
  IND(s); s << "case GIOP::USER_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  if (no_user_exception()) {
    IND(s); s << "_c.RequestCompleted(1);\n";
    IND(s); s << "throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);\n";
  }
  else {
    // unmarshall user exceptions
    size_t maxIdsize = 0;
    {
      UTL_ExceptlistActiveIterator i(exceptions());
      while (!i.is_done())
	{
	  o2be_exception *excpt = o2be_exception::narrow_from_decl(i.item());
	  if (excpt->repoIdConstLen() > maxIdsize)
	    maxIdsize = excpt->repoIdConstLen();
	  i.next();
	}
    }
    IND(s); s << "CORBA::Char _excId[" << maxIdsize << "];\n";
    IND(s); s << "CORBA::ULong _len;\n";
    IND(s); s << "_len <<= _c;\n";
    IND(s); s << "if (_len > " << maxIdsize << ") {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_c.RequestCompleted(1);\n";
    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_c.get_char_array(_excId,_len);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";

    UTL_ExceptlistActiveIterator i(exceptions());
    idl_bool firstexc = I_TRUE;
    while (!i.is_done())
      {
	o2be_exception *excpt = o2be_exception::narrow_from_decl(i.item());
	IND(s); s << ((firstexc)? "if" : "else if") 
		  << " (strcmp((const char *)_excId,"
		  << excpt->repoIdConstName()
		  << ") == 0) {\n";
	INC_INDENT_LEVEL();
	IND(s); s << excpt->fqname() << " _ex;\n";
	argType ntype = tStructVariable;
	argMapping mapping = {I_FALSE,I_TRUE,I_FALSE,I_FALSE};
	produceUnMarshalCode(s,i.item(),"_c","_ex",ntype,mapping);
	IND(s); s << "_c.RequestCompleted();\n";
	IND(s); s << "throw _ex;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	i.next();
	firstexc = I_FALSE;
      }
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_c.RequestCompleted(1);\n";
    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
  }
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::SYSTEM_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_c.RequestCompleted(1);\n";
  IND(s); s << "throw omniORB::fatalException(__FILE__,__LINE__,\"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()\");\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::LOCATION_FORWARD:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_c);\n";
  IND(s); s << "_c.RequestCompleted();\n";
  IND(s); s << "if (CORBA::is_nil(obj)) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (omniORB::traceLevel > 10) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "cerr << \"Received GIOP::LOCATION_FORWARD message that contains a nil object reference.\" << endl;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "omniRopeAndKey __r;\n";
  IND(s); s << "obj->PR_getobj()->getRopeAndKey(__r);\n";
  IND(s); s << "setRopeAndKey(__r);\n";
  IND(s); s << "_c.~GIOP_C();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "if (omniORB::traceLevel > 10) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "cerr << \"GIOP::LOCATION_FORWARD: retry request.\" << endl;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s);
  if (!return_is_void()) {
    s << "return ";
    produce_invoke(s);
    s << ";\n";
  }
  else {
    produce_invoke(s);
    s << ";\n";
    IND(s); s << "return;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "catch (const CORBA::COMM_FAILURE& ex) {\n";
  INC_INDENT_LEVEL();

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;
	
	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	if (a->direction() == AST_Argument::dir_OUT)
	  {
	    ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	    if (mapping.is_arrayslice)
	      {
		IND(s); s << "if (_" << a->uqname() << ") delete [] _" << a->uqname() << ";\n";
	      }
	    else if (mapping.is_reference && mapping.is_pointer)
	      {
		IND(s); s << "if (_" << a->uqname() << ") delete _" << a->uqname() << ";\n";
	      }
	    else if (ntype == tObjref)
	      {
		IND(s); s << "if (_" << a->uqname() 
			  << ") CORBA::release(_" << a->uqname() <<");\n";
		  }	
	    else if (ntype == tString)
	      {
		IND(s); s << "if (_" << a->uqname() 
			  << ") CORBA::string_free(_"
			  << a->uqname() << ");\n";
	      }
	  }
	i.next();
      }
  }
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      
      if (mapping.is_arrayslice)
	{
	  IND(s); s << "if (_result) delete [] _result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_result) CORBA::release(_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_result) CORBA::string_free(_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_result) delete _result;\n";
	}
    }
  IND(s); s << "if (_fwd) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "resetRopeAndKey();\n";
  IND(s); s << "throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (...) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;
	
	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	if (a->direction() == AST_Argument::dir_OUT)
	  {
	    ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	    if (mapping.is_arrayslice)
	      {
		IND(s); s << "if (_" << a->uqname() << ") delete [] _" << a->uqname() << ";\n";
	      }
	    else if (mapping.is_reference && mapping.is_pointer)
	      {
		IND(s); s << "if (_" << a->uqname() << ") delete _" << a->uqname() << ";\n";
	      }
	    else if (ntype == tObjref)
	      {
		IND(s); s << "if (_" << a->uqname() 
			  << ") CORBA::release(_" << a->uqname() <<");\n";
		  }	
	    else if (ntype == tString)
	      {
		IND(s); s << "if (_" << a->uqname() 
			  << ") CORBA::string_free(_"
			  << a->uqname() << ");\n";
	      }
	  }
	i.next();
      }
  }
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      
      if (mapping.is_arrayslice)
	{
	  IND(s); s << "if (_result) delete [] _result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_result) CORBA::release(_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_result) CORBA::string_free(_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_result) delete _result;\n";
	}
    }
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  if (!return_is_void())
    {
      IND(s); s << "{\n";
      INC_INDENT_LEVEL();
      IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),0,mapping.is_arrayslice);
	  s << ((ntype != tObjref && ntype != tString)?" *":"") 
	    << " _result" << "= 0;\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type());
	  s << " _result";
	  switch (ntype)
	    {
	    case tShort:
	    case tLong:
	    case tUShort:
	    case tULong:
	    case tFloat:
	    case tDouble:
	    case tBoolean:
	    case tChar:
	    case tOctet:
	      s << " = 0;\n";
	      break;
	    case tEnum:
	      {
		s << " = ";
		AST_Decl *decl = return_type();
		while (decl->node_type() == AST_Decl::NT_typedef)
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		UTL_ScopeActiveIterator i(o2be_enum::narrow_from_decl(decl),
					  UTL_Scope::IK_decls);
		AST_Decl *eval = i.item();
		if (strlen(o2be_name::narrow_and_produce_scopename(decl))) {
		  s << o2be_name::narrow_and_produce_scopename(decl);
		}
		else {
		  s << "::";
		}
		s << o2be_name::narrow_and_produce_uqname(eval) << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_result,0,sizeof(_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _result;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
    }
    
  DEC_INDENT_LEVEL();
  IND(s);s << "}\n";
  return;
}

void
o2be_operation::produce_server_skel(fstream &s,o2be_interface &defined_in)
{
  if (flags() == AST_Operation::OP_oneway) {
    IND(s); s << "if (_response_expected) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
  }
  else {
    IND(s); s << "if (!_response_expected) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
  }

  {
    // unmarshall arguments
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	IND(s);
	switch(a->direction())
	  {
	  case AST_Argument::dir_IN:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wIN,mapping);
	      if (ntype == tObjref || ntype == tString) 
		// declare a <type>_var variable to manage the pointer type
		declareVarType(s,a->field_type(),1);
	      else
		declareVarType(s,a->field_type());
	      s << " " << a->uqname() << ";\n";
	      produceUnMarshalCode(s,a->field_type(),"_s",a->uqname(),
				   ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if (ntype == tObjref || ntype == tString ||
		  (mapping.is_arrayslice) ||
		  (mapping.is_reference && mapping.is_pointer)) 
		{
		  // declare a <type>_var variable to manage the pointer type
		  declareVarType(s,a->field_type(),1,mapping.is_arrayslice);
		}
	      else 
		declareVarType(s,a->field_type());
	      s << " " << a->uqname() << ";\n";
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      if (ntype == tObjref || ntype == tString) 
		{
		  // declare a <type>_var variable to manage the pointer type
		  declareVarType(s,a->field_type(),1);
		}
	      else
		  declareVarType(s,a->field_type());
	      s << " " << a->uqname() << ";\n";
	      produceUnMarshalCode(s,a->field_type(),"_s",a->uqname(),
				   ntype,mapping);
	      break;
	    }
	  }
	i.next();
      }
  }

  IND(s); s << "_s.RequestReceived();\n";

  IND(s);
  if (!return_is_void()) {
    argMapping mapping;
    argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
    if (ntype == tObjref || ntype == tString ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer)) 
      {
	// declare a <type>_var variable to manage the pointer type
	declareVarType(s,return_type(),1,mapping.is_arrayslice);
      }
    else 
      {
	declareVarType(s,return_type());
      }
    s << " _result;\n";
  }

  if (!no_user_exception()) {
    IND(s); s << "try {\n";
    INC_INDENT_LEVEL();
  }
  IND(s);
  if (!return_is_void()) {
    s << "_result = ";
    if (has_variable_out_arg() || has_pointer_inout_arg()) {
      // Use the indirection function in the base class
      s << defined_in.fqname() << "::";
    }
    produce_invoke(s);
    s << ";\n";
  }
  else {
    if (has_variable_out_arg() || has_pointer_inout_arg()) {
      // Use the indirection function in the base class
      s << defined_in.fqname() << "::";
    }
    produce_invoke(s);
    s << ";\n";
  }

  if (flags() == AST_Operation::OP_oneway) {
    IND(s); s << "_s.ReplyCompleted();\n";
    IND(s); s << "return 1;\n";
    return;
  }

  if (!no_user_exception()) {
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    // catch and marshal each user exception
    UTL_ExceptlistActiveIterator i(exceptions());
    while (!i.is_done())
      {
	o2be_exception *excpt = o2be_exception::narrow_from_decl(i.item());
	IND(s); s << "catch ( " << excpt->fqname() << " &ex) {\n";
	INC_INDENT_LEVEL();

	argType ntype = tStructVariable;
	argMapping mapping = {I_FALSE,I_TRUE,I_FALSE,I_FALSE};

	IND(s); s << "size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";

	produceConstStringSizeCalculation(s,"_msgsize",excpt->repoIdConstLen());
	produceSizeCalculation(s,i.item(),"_s","_msgsize","ex",ntype,mapping);

	IND(s); s << "_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_msgsize);\n"; 
	produceConstStringMarshalCode(s,"_s",excpt->repoIdConstName(),
				      excpt->repoIdConstLen());
	produceMarshalCode(s,i.item(),"_s","ex",ntype,mapping);
	IND(s); s << "_s.ReplyCompleted();\n";
	IND(s); s << "return 1;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	i.next();
      }
  }

  // calculate reply message size
  IND(s); s << "size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if ((ntype == tObjref || ntype == tString || mapping.is_pointer)
	  && !mapping.is_arrayslice) 
	{
	  // These are declared as <type>_var variable 
	  if (ntype == tString) {
	    produceSizeCalculation(s,return_type(),"_s","_msgsize",
				   "_result",ntype,mapping);
	  }
	  else {
	    // use operator->() to get to the pointer
	    produceSizeCalculation(s,return_type(),"_s","_msgsize",
				   "(_result.operator->())",ntype,mapping);
	  }
	}
      else
	{
	  produceSizeCalculation(s,return_type(),"_s","_msgsize",
				 "_result",ntype,mapping);
	}

    }
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if ((ntype == tObjref || ntype == tString ||
		   (mapping.is_reference && mapping.is_pointer))
		  && !mapping.is_arrayslice)
		{
		  // These are declared as <type>_var variable 
		  if (ntype == tString) {
		    produceSizeCalculation(s,a->field_type(),"_s","_msgsize",
					   a->uqname(),ntype,mapping);
		  }
		  else {
		    // use operator->() to get to the pointer
		    char *_argname = new char[strlen(a->uqname())+
					     strlen("(.operator->())")+1];
		    strcpy(_argname,"(");
		    strcat(_argname,a->uqname());
		    strcat(_argname,".operator->())");
		    produceSizeCalculation(s,a->field_type(),"_s","_msgsize",
					   _argname,ntype,mapping);
		    delete [] _argname;
		  }
		}
	      else
		{
		  produceSizeCalculation(s,a->field_type(),"_s","_msgsize",
					 a->uqname(),ntype,mapping);
		}
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceSizeCalculation(s,a->field_type(),"_s","_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_IN:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);\n";

  // marshall results
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if ((ntype == tObjref || ntype == tString || mapping.is_pointer)
	  && !mapping.is_arrayslice) 
	{
	  // These are declared as <type>_var variable 
	  if (ntype == tString) 
	    {
	      produceMarshalCode(s,return_type(),"_s",
				 "_result",ntype,mapping);
	    }
	  else 
	    {
	      // use operator->() to get to the pointer
	      produceMarshalCode(s,return_type(),"_s",
				 "(_result.operator->())",ntype,mapping);
	    }
	}
      else
	{
	  produceMarshalCode(s,return_type(),"_s","_result",ntype,mapping);
	}
    }
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType ntype;

	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if ((ntype == tObjref || ntype == tString ||
		   (mapping.is_reference && mapping.is_pointer))
		  && !mapping.is_arrayslice)
		{
		  // These are declared as <type>_var variable 
		  if (ntype == tString) 
		    {
		      produceMarshalCode(s,a->field_type(),"_s",
					 a->uqname(),ntype,mapping);
		    }
		  else
		    {
		      // use operator->() to get to the pointer
		      char *_argname = new char[strlen(a->uqname())+
					       strlen("(.operator->())")+1];
		      strcpy(_argname,"(");
		      strcat(_argname,a->uqname());
		      strcat(_argname,".operator->())");
		      produceMarshalCode(s,a->field_type(),"_s",
					 _argname,ntype,mapping);
		      delete [] _argname;
		    }
		}
	      else
		{
		  produceMarshalCode(s,a->field_type(),"_s",a->uqname(),ntype,mapping);
		}
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceMarshalCode(s,a->field_type(),"_s",a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_IN:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "_s.ReplyCompleted();\n";
  IND(s); s << "return 1;\n";
  return;
}

void
o2be_operation::produce_nil_skel(fstream &s,const char* alias_prefix)
{
  IND(s); produce_decl(s,0,alias_prefix);
  s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),0,mapping.is_arrayslice);
	  s << ((ntype != tObjref && ntype != tString)?" *":"") 
	    << " _result" << "= 0;\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type());
	  s << " _result";
	  switch (ntype)
	    {
	    case tShort:
	    case tLong:
	    case tUShort:
	    case tULong:
	    case tFloat:
	    case tDouble:
	    case tBoolean:
	    case tChar:
	    case tOctet:
	      s << " = 0;\n";
	      break;
	    case tEnum:
	      {
		s << " = ";
		AST_Decl *decl = return_type();
		while (decl->node_type() == AST_Decl::NT_typedef)
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		UTL_ScopeActiveIterator i(o2be_enum::narrow_from_decl(decl),
					  UTL_Scope::IK_decls);
		AST_Decl *eval = i.item();
		if (strlen(o2be_name::narrow_and_produce_scopename(decl))) {
		  s << o2be_name::narrow_and_produce_scopename(decl);
		}
		else {
		  s << "::";
		}
		s << o2be_name::narrow_and_produce_uqname(eval) << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_result,0,sizeof(_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _result;\n";
    }
  else
    {
      IND(s); s << "return;\n";
    }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_operation::produce_mapping_with_indirection(fstream& s,
						 const char* alias_prefix)
{
  if (!has_variable_out_arg() && !has_pointer_inout_arg())
    return;

  unsigned int indent_pos = 0;

  IND(s); 
  // return type
  if (!return_is_void())
    {
      argMapping mapping;
      argType    ntype;
      const char* str;

      ntype = ast2ArgMapping(return_type(),wResult,mapping);	

      if (ntype == tObjref) {
	AST_Decl *decl = return_type();
	while (decl->node_type() == AST_Decl::NT_typedef) {
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	}
	str = o2be_interface::narrow_from_decl(decl)->objref_fqname();
	s << str;
	indent_pos += strlen(str);
      }
      else if (ntype == tString) {
	s << "char *";
	indent_pos += 6;
      }
      else {
	str = o2be_name::narrow_and_produce_fqname(return_type());
	s << str;
	indent_pos += strlen(str);
      }
      if (mapping.is_arrayslice) {
	s << "_slice";
	indent_pos += 6;
	
      }
      s << " ";
      indent_pos += 1;
      if (mapping.is_pointer) {
	s << "*";
	indent_pos += 1;
      }
      if (mapping.is_reference) {
	s << "&";
	indent_pos += 1;
      }
    }
  else
    {
      s << "void";
      indent_pos += 4;
    }
  // function namees
  s << " " << uqname() << " ( ";
  indent_pos += strlen(alias_prefix) + strlen(uqname()) + 4;

  // argument list
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argMapping mapping;
	argType    ntype;
	idl_bool   outvar;
	idl_bool   inoutptr;

	outvar = I_FALSE;
	inoutptr = I_FALSE;
	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	switch(a->direction())
	  {
	  case AST_Argument::dir_IN:
	    ntype = ast2ArgMapping(a->field_type(),wIN,mapping);
	    break;
	  case AST_Argument::dir_INOUT:
	    ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	    switch(ntype) {
	    case tObjref:
	    case tString:
	      inoutptr = I_TRUE;
	      break;
	    default:
	      break;
	    }
	    break;
	  case AST_Argument::dir_OUT:
	    ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	    switch(ntype) {
	    case tObjref:
	    case tStructVariable:
	    case tUnionVariable:
	    case tString:
	    case tSequence:
	    case tArrayVariable:
	    case tAny:
	      outvar = I_TRUE;
	      break;
	    default:
	      break;
	    }
	    break;
	  }

	if (!outvar && !inoutptr)
	  {
	    s << ((mapping.is_const) ? "const ":"");
	    if (ntype == tObjref) {
	      AST_Decl *decl = a->field_type();
	      while (decl->node_type() == AST_Decl::NT_typedef) {
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	      }
	      s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
	    }
	    else if (ntype == tString) {
	      s << "char *";
	    }
	    else {
	      s << o2be_name::narrow_and_produce_fqname(a->field_type());
	    }
	    s << ((mapping.is_arrayslice) ? "_slice":"")
	      << " "
	      << ((mapping.is_pointer)    ? "*":"")
	      << ((mapping.is_reference)  ? "&":"");
	    s << " " << a->uqname();
	  }
	else
	  {
	    switch (ntype) {
	    case tObjref:
	      {
		AST_Decl *decl = a->field_type();
		while (decl->node_type() == AST_Decl::NT_typedef) {
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		}
		o2be_interface* intf = o2be_interface::narrow_from_decl(decl);
		if (a->direction() == AST_Argument::dir_INOUT) {
		  s << intf->inout_adptarg_name() << " ";
		}
		else {
		  s << intf->out_adptarg_name() << " ";
		}
		break;
	      }
	    case tString:
	      {
		if (a->direction() == AST_Argument::dir_INOUT) {
		  s << "CORBA::String_INOUT_arg ";
		}
		else {
		  s << "CORBA::String_OUT_arg ";
		}
		break;
	      }
	    case tAny:
	      {
		if (a->direction() == AST_Argument::dir_OUT) {
		  s << "CORBA::Any_OUT_arg ";
		}
		break;
	      }
	    case tStructVariable:
	      {
		AST_Decl *decl = a->field_type();
		while (decl->node_type() == AST_Decl::NT_typedef) {
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		}
		o2be_structure* p = o2be_structure::narrow_from_decl(decl);
		if (a->direction() == AST_Argument::dir_OUT) {
		  s << p->out_adptarg_name() << " ";
		}
		break;
	      }
	    case tUnionVariable:
	      {
		AST_Decl *decl = a->field_type();
		while (decl->node_type() == AST_Decl::NT_typedef) {
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		}
		o2be_union* p = o2be_union::narrow_from_decl(decl);
		if (a->direction() == AST_Argument::dir_OUT) {
		  s << p->out_adptarg_name() << " ";
		}
		break;
	      }
	    case tSequence:
	      {
		AST_Decl *decl = a->field_type();
		if (decl->node_type() != AST_Decl::NT_typedef) {
		  throw o2be_internal_error(__FILE__,__LINE__,
					    "Typedef expected");
		}
		o2be_typedef* tp = o2be_typedef::narrow_from_decl(decl);
		while (decl->node_type() == AST_Decl::NT_typedef) {
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		}
		o2be_sequence* p = o2be_sequence::narrow_from_decl(decl);
		if (a->direction() == AST_Argument::dir_OUT) {
		  s << p->out_adptarg_name(tp) << " ";
		}
		break;
	      }
	    case tArrayVariable:
	      {
		AST_Decl *decl = a->field_type();
		if (decl->node_type() != AST_Decl::NT_typedef) {
		  throw o2be_internal_error(__FILE__,__LINE__,
					    "Typedef expected");
		}
		o2be_typedef* tp = o2be_typedef::narrow_from_decl(decl);
		while (decl->node_type() == AST_Decl::NT_typedef) {
		  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		}
		o2be_array* p = o2be_array::narrow_from_decl(decl);
		if (a->direction() == AST_Argument::dir_OUT) {
		  s << p->out_adptarg_name(tp) << " ";
		}
		break;
	      }
	    default:
	      break;
	    }
	    s << " " << a->uqname();
	  }
	i.next();
	if (!i.is_done()) {
	  s << ",\n";
	  IND(s);
	  for (unsigned int j=0; j < indent_pos; j++) 
	    s << " ";
	}
      }
  }

  s << " )\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s);
  if (!return_is_void()) {
    s << "return ";
  }
  s << alias_prefix;
  s << uqname() << " ( ";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	argType ntype;
	argMapping mapping;
	o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
	s << a->uqname();
	if (a->direction() == AST_Argument::dir_INOUT) {
	  ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	  if (ntype == tObjref || ntype == tString) {
	    s << "._data";
	  }
	}
	else if (a->direction() == AST_Argument::dir_OUT) {
	  ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	  switch(ntype) {
	  case tObjref:
	  case tStructVariable:
	  case tUnionVariable:
	  case tString:
	  case tSequence:
	  case tArrayVariable:
	  case tAny:
	    s << "._data";
	  default:
	    break;
	  }
	}
	i.next();
	s << ((!i.is_done()) ? ", " :"");
      }
  }
  s << " )";

  s << ";\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}

idl_bool
o2be_operation::return_is_void()
{
  AST_Decl *rtype = return_type();
  if (rtype->node_type() == AST_Decl::NT_pre_defined &&
      AST_PredefinedType::narrow_from_decl(rtype)->pt()
          == AST_PredefinedType::PT_void)
    return I_TRUE;
  else
    return I_FALSE;
}

idl_bool
o2be_operation::no_user_exception()
{
  if (exceptions() == NULL)
    return I_TRUE;
  else
    return I_FALSE;
}

idl_bool
o2be_operation::has_variable_out_arg()
{
  idl_bool hasvar = I_FALSE;
  UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
  while (!i.is_done())
    {
      argMapping mapping;
      argType    ntype;
      o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
      switch(a->direction())
	{
	case AST_Argument::dir_OUT:
	  ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	  switch (ntype) {
	  case tObjref:
	  case tStructVariable:
	  case tUnionVariable:
	  case tString:
	  case tSequence:
	  case tArrayVariable:
	  case tAny:
	    hasvar = I_TRUE;
	    break;
	  default:
	    break;
	  }
	  break;
	default:
	  break;
	}
      i.next();
    }
  return hasvar;
}


idl_bool
o2be_operation::has_pointer_inout_arg()
{
  idl_bool hasptr = I_FALSE;
  UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
  while (!i.is_done())
    {
      argMapping mapping;
      argType    ntype;
      o2be_argument *a = o2be_argument::narrow_from_decl(i.item());
      switch(a->direction())
	{
	case AST_Argument::dir_INOUT:
	  ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	  switch (ntype) {
	  case tObjref:
	  case tString:
	    hasptr = I_TRUE;
	    break;
	  default:
	    break;
	  }
	  break;
	default:
	  break;
	}
      i.next();
    }
  return hasptr;
}

IMPL_NARROW_METHODS1(o2be_operation, AST_Operation)
IMPL_NARROW_FROM_DECL(o2be_operation)
IMPL_NARROW_FROM_SCOPE(o2be_operation)

static
const o2be_operation::argMapping 
      inArgMappingTable[o2be_operation::_tMaxValue] = {
  // IN argument passing rule.
  // Reference: CORBA V2.0 Table 24
  //
  // const, reference, pointer, arrayslice
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tShort
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tLong
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tUShort
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tULong
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tFloat
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tDouble
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tBoolean
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tChar
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tOctet
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tEnum
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tObjref
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tStructFixed
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tStructVariable
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tUnionFixed
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tUnionVariable
  { I_TRUE,  I_FALSE, I_FALSE, I_FALSE }, // tString
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tSequence
  { I_TRUE,  I_FALSE, I_FALSE, I_FALSE }, // tArrayFixed
  { I_TRUE,  I_FALSE, I_FALSE, I_FALSE }, // tArrayVariable
  { I_TRUE,  I_TRUE,  I_FALSE, I_FALSE }, // tAny
};

static
const o2be_operation::argMapping 
      outArgMappingTable[o2be_operation::_tMaxValue] = {
  // OUT argument passing rule.
  // Reference: CORBA V2.0 Table 24
  //
  // const, reference, pointer, arrayslice
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tShort
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tLong
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tUShort
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tULong
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tFloat
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tDouble
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tBoolean
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tChar
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tOctet
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tEnum
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tObjref
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tStructFixed
  { I_FALSE, I_TRUE,  I_TRUE,  I_FALSE }, // tStructVariable
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tUnionFixed
  { I_FALSE, I_TRUE,  I_TRUE,  I_FALSE }, // tUnionVariable
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tString
  { I_FALSE, I_TRUE,  I_TRUE,  I_FALSE }, // tSequence
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tArrayFixed
  { I_FALSE, I_TRUE,  I_TRUE,  I_TRUE  }, // tArrayVariable
  { I_FALSE, I_TRUE,  I_TRUE,  I_FALSE }, // tAny
};

static
const o2be_operation::argMapping
      inOutArgMappingTable[o2be_operation::_tMaxValue] = {
  // INOUT argument passing rule.
  // Reference: CORBA V2.0 Table 24
  //
  // const, reference, pointer, arrayslice
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tShort
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tLong
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tUShort
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tULong
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tFloat
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tDouble
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tBoolean
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tChar
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tOctet
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tEnum
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tObjref
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tStructFixed
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tStructVariable
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tUnionFixed
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tUnionVariable
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tString
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tSequence
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tArrayFixed
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tArrayVariable
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tAny
};

static
const o2be_operation::argMapping 
      retValMappingTable[o2be_operation::_tMaxValue] = {
  // Return Value  passing rule.
  // Reference: CORBA V2.0 Table 24
  //
  // const, reference, pointer, arrayslice
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tShort
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tLong
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tUShort
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tULong
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tFloat
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tDouble
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tBoolean
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tChar
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tOctet
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tEnum
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tObjref
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tStructFixed
  { I_FALSE, I_FALSE, I_TRUE,  I_FALSE }, // tStructVariable
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tUnionFixed
  { I_FALSE, I_FALSE, I_TRUE,  I_FALSE }, // tUnionVariable
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tString
  { I_FALSE, I_FALSE, I_TRUE,  I_FALSE }, // tSequence
  { I_FALSE, I_FALSE, I_TRUE,  I_TRUE  }, // tArrayFixed
  { I_FALSE, I_FALSE, I_TRUE,  I_TRUE  }, // tArrayVariable
  { I_FALSE, I_FALSE, I_TRUE,  I_FALSE }, // tAny
};

o2be_operation::argType
o2be_operation::ast2ArgMapping(AST_Decl *decl,
			       o2be_operation::argWhich dir,
			       o2be_operation::argMapping &mapping)
{
  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }

  argType ntype = tShort;

  switch(decl->node_type())
    {
    case AST_Decl::NT_pre_defined:
      {
	switch(o2be_predefined_type::narrow_from_decl(decl)->pt())
	  {
	  case AST_PredefinedType::PT_short:
	    ntype = tShort;
	    break;
	  case AST_PredefinedType::PT_long:
	    ntype = tLong;
	    break;
	  case AST_PredefinedType::PT_ushort:
	    ntype = tUShort;
	    break;
	  case AST_PredefinedType::PT_ulong:
	    ntype = tULong;
	    break;
	  case AST_PredefinedType::PT_float:
	    ntype = tFloat;
	    break;
	  case AST_PredefinedType::PT_double:
	    ntype = tDouble;
	    break;
	  case AST_PredefinedType::PT_boolean:
	    ntype = tBoolean;
	    break;
	  case AST_PredefinedType::PT_char:
	    ntype = tChar;
	    break;
	  case AST_PredefinedType::PT_octet:
	    ntype = tOctet;
	    break;
	  case AST_PredefinedType::PT_any:
	    ntype = tAny;
	    break;
	  case AST_PredefinedType::PT_longlong:
	  case AST_PredefinedType::PT_ulonglong:
	  case AST_PredefinedType::PT_longdouble:
	  case AST_PredefinedType::PT_wchar:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unsupported argument type");
	  case AST_PredefinedType::PT_void:
	  case AST_PredefinedType::PT_pseudo:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Invalid argument type");
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unknown arguement type");
	  }
      }
      break;
    case AST_Decl::NT_enum:
      ntype = tEnum;
      break;
    case AST_Decl::NT_interface:
      ntype = tObjref;
      break;
    case AST_Decl::NT_union:
      if (o2be_union::narrow_from_decl(decl)->isVariable())
	ntype = tUnionVariable;
      else
	ntype = tUnionFixed;
      break;
    case AST_Decl::NT_struct:
      if (o2be_structure::narrow_from_decl(decl)->isVariable())
	ntype = tStructVariable;
      else
	ntype = tStructFixed;
      break;
    case AST_Decl::NT_string:
      ntype = tString;
      break;
    case AST_Decl::NT_sequence:
      ntype = tSequence;
      break;
    case AST_Decl::NT_array:
      if (o2be_array::narrow_from_decl(decl)->isVariable())
	ntype = tArrayVariable;
      else
	ntype = tArrayFixed;
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
    }

  switch (dir) 
    {
    case wIN:
      mapping = inArgMappingTable[ntype];
      break;
    case wOUT:
      mapping = outArgMappingTable[ntype];
      break;
    case wINOUT:
      mapping = inOutArgMappingTable[ntype];
      break;
    case wResult:
      mapping = retValMappingTable[ntype];
      break;
    }
  return ntype;
}

void
o2be_operation::declareVarType(fstream &s,AST_Decl *decl,idl_bool is_var,
			       idl_bool is_arrayslice)
{
  AST_Decl *truetype = decl;
  while (truetype->node_type() == AST_Decl::NT_typedef) {
    truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
  }

  if (truetype->node_type() == AST_Decl::NT_interface)
    {
      if (!is_var)
	s << o2be_interface::narrow_from_decl(truetype)->objref_fqname();
      else
	s << o2be_name::narrow_and_produce_fqname(truetype) << "_var";
    }
  else 
    if (truetype->node_type() == AST_Decl::NT_string)
      {
	if (!is_var)
	  s << "char *";
	else
	  s << "CORBA::String_var";
      }
  else
    {
      s << o2be_name::narrow_and_produce_fqname(decl);
      if (is_var)
	s << "_var";
      else
	if (is_arrayslice)
	  s << "_slice";
    }
  return;
}

void
o2be_operation::produceUnMarshalCode(fstream &s, AST_Decl *decl,
		     const char *netstream,
		     const char *argname,
		     argType type, argMapping mapping,
		     idl_bool no_size_check)
{
  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }
  switch(type)
    {
    case tBoolean:
    case tChar:
    case tOctet:
    case tShort:
    case tUShort:
    case tLong:
    case tULong:
    case tEnum:
    case tFloat:
    case tDouble:
      IND(s); s << argname << " <<= " << netstream << ";\n";
      break;

    case tStructFixed:
    case tUnionFixed:
    case tStructVariable:
    case tUnionVariable:
    case tSequence:
    case tAny:
    case tObjrefMember:
    case tStringMember:
      IND(s); s << ((mapping.is_pointer)?"*":"") << argname << " <<= "
		<< netstream << ";\n";
      break;

    case tString:
      {      
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	IND(s); s << "CORBA::ULong _len;\n";
	IND(s); s << "_len <<= " << netstream << ";\n";
	if (!no_size_check) 
	  {
	    IND(s); s << "if (!_len || " << netstream << ".RdMessageUnRead() < _len)\n";
	    INC_INDENT_LEVEL();
	    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);\n";
	    DEC_INDENT_LEVEL();
	  }
	else
	  {
	    IND(s); s << "if (!_len)\n";
	    INC_INDENT_LEVEL();
	    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);\n";
	    DEC_INDENT_LEVEL();
	  }
	if (o2be_string::narrow_from_decl(decl)->max_length()) {
	  IND(s); s << "if (_len > " 
		    << o2be_string::narrow_from_decl(decl)->max_length()
		    << "+1)\n";
	  INC_INDENT_LEVEL();
	  IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);\n";
	  DEC_INDENT_LEVEL();
	}
	IND(s); s << argname << " = CORBA::string_alloc(_len-1);\n";
	IND(s); s << "if (!((char *)" << argname << "))\n";
	INC_INDENT_LEVEL();
	IND(s); s << "throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << netstream << ".get_char_array((CORBA::Char *)" 
		  << "((char *)" << argname << "),_len);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    case tObjref:
      {
	IND(s); s << argname << " = "
		  << o2be_interface::narrow_from_decl(decl)->fqname()
		  << "::unmarshalObjRef(" << netstream << ");\n";
#if 0
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	IND(s); s << "CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef("
		  << o2be_interface::narrow_from_decl(decl)->IRrepoId() << "," 
		  << netstream << ");\n";
	IND(s); s << argname << " = " 
		  << o2be_interface::narrow_from_decl(decl)->fqname()
		  << "::_narrow(_obj);\n";
	IND(s); s << "CORBA::release(_obj);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
#endif
      }
      break;

    case tArrayFixed:
      {
	argMapping dummymapping;
	argType atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	switch (atype)
	  {
	  case tBoolean:
	  case tChar:
	  case tOctet:
	  case tShort:
	  case tUShort:
	  case tLong:
	  case tULong:
	  case tEnum:
	  case tFloat:
	  case tDouble:
	  case tStructFixed:
	  case tUnionFixed:
	    {
	      unsigned int ndim = 0;
	      unsigned int dimval;
	      o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	      IND(s); s << "{\n";
	      INC_INDENT_LEVEL();
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  dimval = next();
		  IND(s); s << "for (unsigned int _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " <<= " << netstream << ";\n";
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  ndim++;
		}
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	    }
	  break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for fixed array");
	  }
      }
      
      break;

    case tArrayVariable:
      {
	argType atype;
	argMapping dummymapping;
	atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	unsigned int ndim = 0;
	unsigned int dimval;
	o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    dimval = next();
	    IND(s); s << "for (unsigned int _i" << ndim << " =0;"
		      << "_i" << ndim << " < " << dimval << ";"
		      << "_i" << ndim << "++) {\n";
	    INC_INDENT_LEVEL();
	    ndim++;
	  }
	switch (atype)
	  {
	  case tStructVariable:
	  case tUnionVariable:
	  case tSequence:
	  case tAny:
	    {
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " <<= " << netstream << ";\n";
	      break;
	    }
	  case tString:
	    {
	      IND(s); s << "CORBA::ULong _len;\n";
	      IND(s); s << "_len <<= " << netstream << ";\n";
	      if (!no_size_check)
		{
		  IND(s); s << "if (!_len || " << netstream << ".RdMessageUnRead() < _len)\n";
		  INC_INDENT_LEVEL();
		  IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);\n";
		  DEC_INDENT_LEVEL();
		}
	      else
		{
		  IND(s); s << "if (!_len)\n";
		  INC_INDENT_LEVEL();
		  IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);\n";
		  DEC_INDENT_LEVEL();
		}

	      AST_Decl *sdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (sdecl->node_type() == AST_Decl::NT_typedef) {
		sdecl = o2be_typedef::narrow_from_decl(sdecl)->base_type();
	      }
	      if (o2be_string::narrow_from_decl(sdecl)->max_length()) {
		IND(s); s << "if (_len > " 
			  << o2be_string::narrow_from_decl(sdecl)->max_length()
			  << "+1) {\n";
		INC_INDENT_LEVEL();
		IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
	      }


	      IND(s); s << "if (!(char*)(" 
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s	<< " = CORBA::string_alloc(_len-1)))\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << netstream << ".get_char_array((CORBA::Char *)((char *)" 
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "),_len);\n";
	      break;
	    }
	  case tObjref:
	    {
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}

	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      s << " = "
		<< o2be_interface::narrow_from_decl(tdecl)->fqname()
		<< "::unmarshalObjRef(" << netstream << ");\n";
#if 0
	      IND(s); s << "CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef("
			<< o2be_interface::narrow_from_decl(decl)->IRrepoId() << "," 
			<< netstream << ");\n";
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      IND(s); s << " = " 
			<< o2be_interface::narrow_from_decl(decl)->fqname()
			<< "::_narrow(_obj);\n";
	      IND(s); s << "CORBA::release(_obj);\n";
#endif
 	    }
	    break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for variable array");
	  }
	ndim = 0;
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    DEC_INDENT_LEVEL();
	    IND(s); s << "}\n";
	    ndim++;
	  }
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      break;
    }
}

void
o2be_operation::produceMarshalCode(fstream &s, AST_Decl *decl,
		   const char *netstream,
		   const char *argname,
		   argType type, argMapping mapping)
{
  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }
  switch(type)
    {
    case tBoolean:
    case tChar:
    case tOctet:
    case tShort:
    case tUShort:
    case tLong:
    case tULong:
    case tEnum:
    case tFloat:
    case tDouble:
    case tStructFixed:
    case tUnionFixed:
      IND(s); s << argname << " >>= " << netstream << ";\n";
      break;


    case tStructVariable:
    case tUnionVariable:
    case tSequence:
    case tAny:
    case tStringMember:
    case tObjrefMember:
      IND(s); s << ((mapping.is_pointer)?"*":"") << argname 
		<< " >>= " << netstream << ";\n";
      break;

    case tString:
      {      
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	IND(s); s << "CORBA::ULong _len = strlen((const char *)"<< argname <<")+1;\n";
	if (o2be_string::narrow_from_decl(decl)->max_length()) {
	  IND(s); s << "if (_len > " 
		    << o2be_string::narrow_from_decl(decl)->max_length()
		    << "+1) {\n";
	  INC_INDENT_LEVEL();
	  IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
	  DEC_INDENT_LEVEL();
	  IND(s); s << "}\n";
	}
	IND(s); s << "_len >>= " << netstream << ";\n";
	IND(s); s << netstream << ".put_char_array((const CORBA::Char *)((const char*) " 
		  << argname << "),_len);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    case tObjref:
      {
	IND(s); s << o2be_interface::narrow_from_decl(decl)->fqname()
		  << "::marshalObjRef(" << argname << ","
		  << netstream << ");\n";
#if 0
	IND(s); s << "CORBA::MarshalObjRef(" << argname << ","
		  << o2be_interface::narrow_from_decl(decl)->IRrepoId() << "," 
		  << (strlen(o2be_interface::narrow_from_decl(decl)->repositoryID()) + 1) << "," 
		  << netstream << ");\n";
#endif
      }
      break;

    case tArrayFixed:
      {
	argMapping dummymapping;
	argType atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	switch (atype)
	  {
	  case tBoolean:
	  case tChar:
	  case tOctet:
	  case tShort:
	  case tUShort:
	  case tLong:
	  case tULong:
	  case tEnum:
	  case tFloat:
	  case tDouble:
	  case tStructFixed:
	  case tUnionFixed:
	    {
	      unsigned int ndim = 0;
	      unsigned int dimval;
	      o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	      IND(s); s << "{\n";
	      INC_INDENT_LEVEL();
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  dimval = next();
		  IND(s); s << "for (unsigned int _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " >>= " << netstream << ";\n";
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  ndim++;
		}
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	    }
	  break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for fixed array");
	  }
      }
      
      break;

    case tArrayVariable:
      {
	argType atype;
	argMapping dummymapping;
	atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	unsigned int ndim = 0;
	unsigned int dimval;
	o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    dimval = next();
	    IND(s); s << "for (unsigned int _i" << ndim << " =0;"
		      << "_i" << ndim << " < " << dimval << ";"
		      << "_i" << ndim << "++) {\n";
	    INC_INDENT_LEVEL();
	    ndim++;
	  }
	switch (atype)
	  {
	  case tStructVariable:
	  case tUnionVariable:
	  case tSequence:
	  case tAny:
	    {
	      IND(s); s << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " >>= " << netstream << ";\n";
	      break;
	    }
	  case tString:
	    {
	      IND(s); s << "CORBA::ULong _len = strlen((const char *)" << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      IND(s); s << ")+1;\n";

	      AST_Decl *sdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (sdecl->node_type() == AST_Decl::NT_typedef) {
		sdecl = o2be_typedef::narrow_from_decl(sdecl)->base_type();
	      }
	      if (o2be_string::narrow_from_decl(sdecl)->max_length()) {
		IND(s); s << "if (_len > " 
			  << o2be_string::narrow_from_decl(sdecl)->max_length()
			  << "+1) {\n";
		INC_INDENT_LEVEL();
		IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
	      }

	      IND(s); s << "_len >>= " << netstream << ";\n";
	      IND(s); s << netstream << ".put_char_array((const CORBA::Char *)((const char*)"
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "),_len);\n";
	      break;
	    }
	  case tObjref:
	    {
	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      IND(s); s << o2be_interface::narrow_from_decl(tdecl)->fqname()
			<< "::marshalObjRef(" << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]._ptr";
		  ndim++;
		}
	      s << "," << netstream << ");\n";
#if 0
	      IND(s); s << "CORBA::MarshalObjRef(" << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ",";
	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      s << o2be_interface::narrow_from_decl(tdecl)->IRrepoId() << "," 
		<< (strlen(o2be_interface::narrow_from_decl(tdecl)->repositoryID()) + 1) << "," 
		<< netstream << ");\n";
#endif
 	    }
	    break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for variable array");
	  }
	ndim = 0;
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    DEC_INDENT_LEVEL();
	    IND(s); s << "}\n";
	    ndim++;
	  }
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      break;
    }
}

void
o2be_operation::produceSizeCalculation(fstream &s, AST_Decl *decl,
		       const char *netstream,
		       const char *sizevar,
		       const char *argname,
		       argType type, argMapping mapping)
{
  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }
  switch(type)
    {
    case tBoolean:
    case tChar:
    case tOctet:
      IND(s); s << sizevar << " += 1;\n";
      break;

    case tShort:
    case tUShort:
      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		<< ",omni::ALIGN_2);\n";
      IND(s); s << sizevar << " += 2;\n";
      break;

    case tLong:
    case tULong:
    case tEnum:
    case tFloat:
      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		<< ",omni::ALIGN_4);\n";
      IND(s); s << sizevar << " += 4;\n";
      break;

    case tDouble:
      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		<< ",omni::ALIGN_8);\n";
      IND(s); s << sizevar << " += 8;\n";
      break;

    case tStructFixed:
    case tUnionFixed:
    case tStructVariable:
    case tUnionVariable:
    case tSequence:
    case tAny:
    case tObjrefMember:
    case tStringMember:
      IND(s); s << sizevar << " = "
		<< argname << ((mapping.is_pointer)?"->":".")
		<< "NP_alignedSize(" << sizevar << ");\n";
      break;

    case tString:
      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		<< ",omni::ALIGN_4);\n";
      IND(s); s << sizevar << " += 4 + strlen((const char *)" 
		<< argname << ") + 1;\n";  
      break;

    case tObjref:
      {
	IND(s); s << sizevar << " = " 
		  << o2be_interface::narrow_from_decl(decl)->fqname()
		  << "::NP_alignedSize("
		  << argname << "," << sizevar << ");\n";
#if 0
	IND(s); s << sizevar << " = CORBA::AlignedObjRef("
		  << argname << "," 
		  << o2be_interface::narrow_from_decl(decl)->IRrepoId() << "," 
		  << (strlen(o2be_interface::narrow_from_decl(decl)->repositoryID())+1) << "," 
		  << sizevar << ");\n";
#endif
      }
      break;

    case tArrayFixed:
      {
	argType atype;
	argMapping dummymapping;
	atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	switch (atype)
	  {
	  case tBoolean:
	  case tChar:
	  case tOctet:
	    IND(s); s << sizevar << " += "
		      << o2be_array::narrow_from_decl(decl)->getNumOfElements()
		      << ";\n";
	    break;

	  case tShort:
	  case tUShort:
	    IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		      << ",omni::ALIGN_2);\n";
	    IND(s); s << sizevar << " += "
		      << o2be_array::narrow_from_decl(decl)->getNumOfElements()
		      << "*2;\n";
	    break;

	  case tLong:
	  case tULong:
	  case tEnum:
	  case tFloat:
	    IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		      << ",omni::ALIGN_4);\n";
	    IND(s); s << sizevar << " += "
		      << o2be_array::narrow_from_decl(decl)->getNumOfElements()
		      << "*4;\n";
	    break;

	  case tDouble:
	    IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		      << ",omni::ALIGN_8);\n";
	    IND(s); s << sizevar << " += "
		      << o2be_array::narrow_from_decl(decl)->getNumOfElements()
		      << "*8;\n";
	    break;

	  case tStructFixed:
	  case tUnionFixed:
	    {
	      unsigned int ndim = 0;
	      unsigned int dimval;
	      o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	      IND(s); s << "{\n";
	      INC_INDENT_LEVEL();
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  dimval = next();
		  IND(s); s << "for (unsigned int _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << sizevar << " = "
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ".NP_alignedSize(" << sizevar << ");\n";
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  ndim++;
		}
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	    }
	  break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for fixed array");
	  }
      }
      
      break;

    case tArrayVariable:
      {
	argType atype;
	argMapping dummymapping;
	atype = ast2ArgMapping(o2be_array::narrow_from_decl(decl)->getElementType(),
			       wIN,         // dummy argument
			       dummymapping // ignored
			       );
	unsigned int ndim = 0;
	unsigned int dimval;
	o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    dimval = next();
	    IND(s); s << "for (unsigned int _i" << ndim << " =0;"
		      << "_i" << ndim << " < " << dimval << ";"
		      << "_i" << ndim << "++) {\n";
	    INC_INDENT_LEVEL();
	    ndim++;
	  }
	switch (atype)
	  {
	  case tStructVariable:
	  case tUnionVariable:
	  case tSequence:
	  case tAny:
	    {
	      IND(s); s << sizevar << " = "
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ".NP_alignedSize(" << sizevar << ");\n";
	      break;
	    }
	  case tString:
	    {
	      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
			<< ",omni::ALIGN_4);\n";
	      IND(s); s << sizevar << " += 4 + strlen((const char *)"
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ") + 1;\n";
	      break;
	    }
	  case tObjref:
	    {
	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      IND(s); s << sizevar << " = "
			<< o2be_interface::narrow_from_decl(tdecl)->fqname()
			<< "::NP_alignedSize("
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]._ptr";
		  ndim++;
		}
	      s << "," << sizevar << ");\n";
#if 0
	      IND(s); s << sizevar << " = CORBA::AlignedObjRef("
			<< argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      s << "," 
		<< o2be_interface::narrow_from_decl(tdecl)->IRrepoId() << "," 
		<< (strlen(o2be_interface::narrow_from_decl(tdecl)->repositoryID())+1) << "," 
		<< sizevar << ");\n";
#endif
 	    }
	    break;

	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected element type for variable array");
	  }
	ndim = 0;
	while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
	  {
	    DEC_INDENT_LEVEL();
	    IND(s); s << "}\n";
	    ndim++;
	  }
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      break;
    }
}


void
o2be_operation::produceConstStringMarshalCode(fstream &s,
			      const char *netstream,
			      const char *str,size_t len)
{
  IND(s); s << "CORBA::ULong _len = " << len << ";\n";
  IND(s); s << "_len >>= " << netstream << ";\n";
  IND(s); s << netstream << ".put_char_array((CORBA::Char *)"
	    << str << "," << len << ");\n";
}

void
o2be_operation::produceConstStringSizeCalculation(fstream &s,
				  const char *sizevar,
				  size_t len)
{
  IND(s); s << sizevar << " = omni::align_to("
	    << sizevar << ",omni::ALIGN_4);\n";
  IND(s); s << sizevar << " += " << (4+len) << ";\n";
}





