// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_operation.cc        Created on: 23/08/1996
//                          Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
//
//  This file is part of omniidl2.
//
//  Omniidl2 is free software; you can redistribute it and/or modify
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
// Description:
//
//

/*
  $Log$
  Revision 1.19  1998/01/27 16:47:25  ewc
  Added support for type Any and TypeCode

// Revision 1.18  1998/01/21  12:14:16  sll
// Now accepts null pointer as marshalling argument. Substituted with a
// proper nil string.  Print a warning if traceLevel > 1.
// Now unmarshal zero size string. Substituted with a proper nil string.
// Print a warning if traceLevel > 1.
//
  Revision 1.17  1997/12/23 19:30:37  sll
  Now generate correct stub for multi-dimensional array of object references.

  Revision 1.16  1997/12/10 11:35:30  sll
  Updated life cycle service stub.

  Revision 1.15  1997/12/09 19:54:13  sll
  *** empty log message ***

// Revision 1.14  1997/09/20  16:46:48  dpg1
// Added LifeCycle code generation.
//
  Revision 1.13  1997/08/22 12:42:20  sll
  Oh well, gcc does not like variable names starting with __, changed
  the prefix to _0RL_.

  Revision 1.12  1997/08/21 21:13:09  sll
  Names of internal variables inside the stub code now all start with the
  prefix __ to avoid potential clash with identifiers defined in IDL.

// Revision 1.11  1997/05/06  14:03:08  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

o2be_operation::o2be_operation(AST_Type *rt, AST_Operation::Flags fl,
			       UTL_ScopedName *n, UTL_StrList *p)
                : AST_Operation(rt, fl, n, p),
		  AST_Decl(AST_Decl::NT_op, n, p),
		  UTL_Scope(AST_Decl::NT_op),
		  o2be_name(AST_Decl::NT_op,n,p)
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
			     idl_bool/* ignored */,
			     idl_bool use_fully_qualified_names)
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
	if (use_fully_qualified_names) {
	  s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this,I_TRUE);
	}
	else {
	  s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	}
      }
      else if (ntype == tString) {
	s << "char *";
      }
      else if (ntype == tTypeCode) {
	AST_Decl *decl = return_type();
	while (decl->node_type() == AST_Decl::NT_typedef) {
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	}
	if (use_fully_qualified_names) {
	  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this,I_TRUE);
	}
	else {
	  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	}
      }	
      else {
	if (use_fully_qualified_names) {
	  s << o2be_name::narrow_and_produce_unambiguous_name(return_type(),this,I_TRUE);
	}
	else {
	  s << o2be_name::narrow_and_produce_unambiguous_name(return_type(),this);
	}
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
	  if (use_fully_qualified_names) {
	    s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this,I_TRUE);
	  }
	  else {
	    s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	  }
	}
	else if (ntype == tString) {
	  s << "char *";
	}
	else if (ntype == tTypeCode) {
	  AST_Decl *decl = a->field_type();
	  while (decl->node_type() == AST_Decl::NT_typedef) {
	    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	  }
	  if (use_fully_qualified_names) {
	    s << o2be_name::narrow_and_produce_unambiguous_name(decl,this,I_TRUE);
	  }
	  else {
	  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	  }
	}		
	else {
	  if (use_fully_qualified_names) {
	    s << o2be_name::narrow_and_produce_unambiguous_name(a->field_type(),this,I_TRUE);
	  }
	  else {
	    s << o2be_name::narrow_and_produce_unambiguous_name(a->field_type(),this);
	  }
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
o2be_operation::produce_proxy_skel(fstream &s,o2be_interface &def_in,
				   const char* alias_prefix)
{
  idl_bool hasVariableLenOutArgs = I_FALSE;

  IND(s); produce_decl(s,def_in.proxy_fqname(),alias_prefix,I_FALSE,I_TRUE);
  s << "\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong _0RL_retries = 0;\n";
  s << "_0RL_again:\n";
  IND(s); s << "assertObjectExistent();\n";
  IND(s); s << "omniRopeAndKey _0RL_r;\n";
  IND(s); s << "CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);\n";
  IND(s); s << "CORBA::Boolean _0RL_reuse = 0;\n";

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
	    if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
		(mapping.is_arrayslice) ||
		(mapping.is_reference && mapping.is_pointer)) 
	      {
		hasVariableLenOutArgs = I_TRUE;
		// Declare a local pointer variable
		IND(s);
		declareVarType(s,a->field_type(),this,0,mapping.is_arrayslice);
		s << ((ntype != tObjref && ntype != tString && 
		       ntype != tTypeCode)?" *":"") << " _" << a->uqname() 
		  << " = " 
		  << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
		  << ";\n";
	      }
	  }
	i.next();
      }
  }
  if (!return_is_void()) 
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  hasVariableLenOutArgs = I_TRUE;
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	((ntype != tObjref && ntype != tString && ntype != tTypeCode)?" *":"") 
	    << " _0RL_result = "
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result;\n";
	}
    }

  IND(s); s << "try {\n";
  INC_INDENT_LEVEL();

  IND(s); s << "GIOP_C _0RL_c(_0RL_r.rope());\n";
  IND(s); s << "_0RL_reuse = _0RL_c.isReUsingExistingConnection();\n";

  // calculate request message size
  IND(s); s << "CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),"
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
	      produceSizeCalculation(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_c","_0RL_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceSizeCalculation(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_c","_0RL_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    break;
	  }
	i.next();
      }
  }


  IND(s); s << "_0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)\""
	    << local_name()->get_string() << "\"," << strlen(local_name()->get_string()) + 1 << ",_0RL_msgsize,"
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
	      produceMarshalCode(s,a->field_type(),
				 (AST_Decl*)&def_in,"_0RL_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceMarshalCode(s,a->field_type(),
				 (AST_Decl*)&def_in,
				 "_0RL_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "switch (_0RL_c.ReceiveReply())\n";  // invoke method
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
		  declareVarType(s,a->field_type(),this);
		  s << ";\n";
		}
		else if (ntype == tTypeCode) {
		    IND(s); s << "_" << a->uqname() 
			      << " = new CORBA::TypeCode(CORBA::tk_null);\n";
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
	    IND(s); s << "_0RL_result = ";
	    AST_Decl *truetype = return_type();
	    while (truetype->node_type() == AST_Decl::NT_typedef) {
	      truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
	    }
	    s << o2be_array::narrow_from_decl(truetype)->fqname()
	      << "_alloc();\n";

	  }
	  else if (mapping.is_pointer) {
	    IND(s); s << "_0RL_result = new ";
	    declareVarType(s,return_type(),this);
	    s << ";\n";
	  }
	  else if (ntype == tTypeCode) {
	      IND(s); 
	      s << "_0RL_result = new CORBA::TypeCode(CORBA::tk_null);\n";
	    }
	}
    }

  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      produceUnMarshalCode(s,return_type(),(AST_Decl*)&def_in,
			   "_0RL_c","_0RL_result",ntype,mapping);
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
		    declareVarType(s,a->field_type(),this,0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_c",_argname,
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
		    declareVarType(s,a->field_type(),this,0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_c",_argname,
					 ntype,mapping);
		    IND(s); s << "CORBA::string_free(" << a->uqname() << ");\n";
		    IND(s); s << a->uqname() << " = " << _argname << ";\n";
		    delete [] _argname;
		  }
		  break;
		default:
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",a->uqname(),
				       ntype,mapping);
		  break;
		}
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
		  (mapping.is_arrayslice) ||
		  (mapping.is_reference && mapping.is_pointer)) 
		{
		  char *_argname = new char[strlen(a->uqname())+2];
		  strcpy(_argname,"_");
		  strcat(_argname,a->uqname());
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",_argname,
				       ntype,mapping);
		  delete [] _argname;
		}
	      else
		{
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",a->uqname(),
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

  IND(s); s << "_0RL_c.RequestCompleted();\n";

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
		    ntype == tTypeCode  || (mapping.is_arrayslice) ||
		    (mapping.is_reference && mapping.is_pointer)) 
		  {
		    IND(s); s << a->uqname() << " = _" << a->uqname() << ";\n";
		  }
	      }
	    i.next();
	  }
      }
      if (!return_is_void()) {
	IND(s); s << "return _0RL_result;\n";
      }
      else {
	IND(s); s << "return;\n";
      }
    }
  else
    {
      if (!return_is_void()) {
	IND(s); s << "return _0RL_result;\n";
      }
      else {
	IND(s); s << "return;\n";
      }
    }
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  
  IND(s); s << "case GIOP::USER_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  if (no_user_exception()) {
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
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
    IND(s); s << "CORBA::Char _0RL_excId[" << maxIdsize << "];\n";
    IND(s); s << "CORBA::ULong _0RL_len;\n";
    IND(s); s << "_0RL_len <<= _0RL_c;\n";
    IND(s); s << "if (_0RL_len > " << maxIdsize << ") {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.get_char_array(_0RL_excId,_0RL_len);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";

    UTL_ExceptlistActiveIterator i(exceptions());
    idl_bool firstexc = I_TRUE;
    while (!i.is_done())
      {
	o2be_exception *excpt = o2be_exception::narrow_from_decl(i.item());
	IND(s); s << ((firstexc)? "if" : "else if") 
		  << " (strcmp((const char *)_0RL_excId,"
		  << excpt->repoIdConstName()
		  << ") == 0) {\n";
	INC_INDENT_LEVEL();
	IND(s); s << excpt->fqname() << " _0RL_ex;\n";
	argType ntype = tStructVariable;
	argMapping mapping = {I_FALSE,I_TRUE,I_FALSE,I_FALSE};
	produceUnMarshalCode(s,i.item(),
			     (AST_Decl*)&def_in,
			     "_0RL_c","_0RL_ex",ntype,mapping);
	IND(s); s << "_0RL_c.RequestCompleted();\n";
	IND(s); s << "throw _0RL_ex;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	i.next();
	firstexc = I_FALSE;
      }
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
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
  IND(s); s << "_0RL_c.RequestCompleted(1);\n";
  IND(s); s << "throw omniORB::fatalException(__FILE__,__LINE__,\"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()\");\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::LOCATION_FORWARD:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);\n";
  IND(s); s << "_0RL_c.RequestCompleted();\n";
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
  IND(s); s << "omniRopeAndKey _0RL__r;\n";
  IND(s); s << "obj->PR_getobj()->getRopeAndKey(_0RL__r);\n";
  IND(s); s << "setRopeAndKey(_0RL__r);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "if (omniORB::traceLevel > 10) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "cerr << \"GIOP::LOCATION_FORWARD: retry request.\" << endl;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "break;\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << "if (_0RL_reuse || _0RL_fwd) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (_0RL_fwd)\n";
  INC_INDENT_LEVEL();
  IND(s); s << "resetRopeAndKey();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "CORBA::TRANSIENT _0RL_ex2(ex.minor(),ex.completed());\n";
  IND(s); s << "if (!_omni_callTransientExceptionHandler(this,_0RL_retries++,_0RL_ex2))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw _0RL_ex2;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (!_omni_callCommFailureExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (const CORBA::TRANSIENT& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << "if (!_omni_callTransientExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (const CORBA::OBJECT_NOT_EXIST& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << "if (_0RL_fwd) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "resetRopeAndKey();\n";
  IND(s); s << "CORBA::TRANSIENT _0RL_ex2(ex.minor(),ex.completed());\n";
  IND(s); s << "if (!_omni_callTransientExceptionHandler(this,_0RL_retries++,_0RL_ex2))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw _0RL_ex2;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (!_omni_callSystemExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (const CORBA::SystemException& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }
  IND(s); s << "if (!_omni_callSystemExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "goto _0RL_again;\n";

  if (!return_is_void())
    {
      s << "#ifdef NEED_DUMMY_RETURN\n";
      IND(s); s << "{\n";
      INC_INDENT_LEVEL();
      IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	((ntype != tObjref && ntype != tString && ntype != tTypeCode)?" *":"") 
	    << " _0RL_result" << " = "
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";

	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result";
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
		s << o2be_name::narrow_and_produce_unambiguous_name(eval,this) << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _0RL_result;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      s << "#endif\n";
    }
    
  DEC_INDENT_LEVEL();
  IND(s);s << "}\n";
  return;
}

void
o2be_operation::produce_server_skel(fstream &s,o2be_interface &def_in)
{
  if (flags() == AST_Operation::OP_oneway) {
    IND(s); s << "if (_0RL_response_expected) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
  }
  else {
    IND(s); s << "if (!_0RL_response_expected) {\n";
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
	      if (ntype == tObjref || ntype == tString || ntype == tTypeCode) 
		// declare a <type>_var variable to manage the pointer type
		declareVarType(s,a->field_type(),this,1);
	      else
		declareVarType(s,a->field_type(),this);
	      s << " " << a->uqname();
	      
	      if (ntype == tTypeCode) 
		s << "(new CORBA::TypeCode(CORBA::tk_null));\n";
	      else s << ";\n";

	      produceUnMarshalCode(s,a->field_type(),
				   (AST_Decl*)&def_in,
				   "_0RL_s",a->uqname(),
				   ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
		  (mapping.is_arrayslice) ||
		  (mapping.is_reference && mapping.is_pointer)) 
		{
		  // declare a <type>_var variable to manage the pointer type
		  declareVarType(s,a->field_type(),this,1,mapping.is_arrayslice);
		}
	      else 
		declareVarType(s,a->field_type(),this);
	      s << " " << a->uqname();
	      if (ntype == tTypeCode) 
		s << "(new CORBA::TypeCode(CORBA::tk_null));\n";
	      else s << ";\n";
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      if (ntype == tObjref || ntype == tString || ntype == tTypeCode) 
		{
		  // declare a <type>_var variable to manage the pointer type
		  declareVarType(s,a->field_type(),this,1);
		}
	      else
		  declareVarType(s,a->field_type(),this);
	      s << " " << a->uqname();
	      if (ntype == tTypeCode) 
		s << "(new CORBA::TypeCode(CORBA::tk_null));\n";
	      else s << ";\n";
	      produceUnMarshalCode(s,a->field_type(),
				   (AST_Decl*)&def_in,
				   "_0RL_s",a->uqname(),
				   ntype,mapping);
	      break;
	    }
	  }
	i.next();
      }
  }

  IND(s); s << "_0RL_s.RequestReceived();\n";

  IND(s);
  if (!return_is_void()) {
    argMapping mapping;
    argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
    if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer)) 
      {
	// declare a <type>_var variable to manage the pointer type
	declareVarType(s,return_type(),this,1,mapping.is_arrayslice);
      }
    else 
      {
	declareVarType(s,return_type(),this);
      }
    s << " _0RL_result";
    if (ntype == tTypeCode) 
      s << "(new CORBA::TypeCode(CORBA::tk_null));\n";
    else s << ";\n";
  }

  if (!no_user_exception()) {
    IND(s); s << "try {\n";
    INC_INDENT_LEVEL();
  }
  IND(s);
  if (!return_is_void()) {
    s << "_0RL_result = ";
    if (has_variable_out_arg() || has_pointer_inout_arg()) {
      // Use the indirection function in the base class
      s << def_in.uqname() << "::";
    }
    produce_invoke(s);
    s << ";\n";
  }
  else {
    if (has_variable_out_arg() || has_pointer_inout_arg()) {
      // Use the indirection function in the base class
      s << def_in.uqname() << "::";
    }
    produce_invoke(s);
    s << ";\n";
  }

  if (flags() == AST_Operation::OP_oneway) {
    IND(s); s << "_0RL_s.ReplyCompleted();\n";
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
	IND(s); s << "catch ( " << excpt->fqname() << " &_0RL_ex) {\n";
	INC_INDENT_LEVEL();

	argType ntype = tStructVariable;
	argMapping mapping = {I_FALSE,I_TRUE,I_FALSE,I_FALSE};

	IND(s); s << "size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";

	produceConstStringSizeCalculation(s,"_0RL_msgsize",excpt->repoIdConstLen());
	produceSizeCalculation(s,i.item(),
			       (AST_Decl*)&def_in,
			       "_0RL_s","_0RL_msgsize","_0RL_ex",ntype,mapping);

	IND(s); s << "_0RL_s.InitialiseReply(GIOP::USER_EXCEPTION,(CORBA::ULong)_0RL_msgsize);\n"; 
	produceConstStringMarshalCode(s,"_0RL_s",excpt->repoIdConstName(),
				      excpt->repoIdConstLen());
	produceMarshalCode(s,i.item(),
			   (AST_Decl*)&def_in,
			   "_0RL_s","_0RL_ex",ntype,mapping);
	IND(s); s << "_0RL_s.ReplyCompleted();\n";
	IND(s); s << "return 1;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	i.next();
      }
  }

  // calculate reply message size
  IND(s); s << "size_t _0RL_msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if ((ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	   mapping.is_pointer) && !mapping.is_arrayslice) 
	{
	  // These are declared as <type>_var variable 
	  if (ntype == tString) {
	    produceSizeCalculation(s,return_type(),
				   (AST_Decl*)&def_in,
				   "_0RL_s","_0RL_msgsize",
				   "_0RL_result",ntype,mapping);
	  }
	  else {
	    // use operator->() to get to the pointer
	    produceSizeCalculation(s,return_type(),
				   (AST_Decl*)&def_in,
				   "_0RL_s","_0RL_msgsize",
				   "(_0RL_result.operator->())",ntype,mapping);
	  }
	}
      else
	{
	  produceSizeCalculation(s,return_type(),
				 (AST_Decl*)&def_in,
				 "_0RL_s","_0RL_msgsize",
				 "_0RL_result",ntype,mapping);
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
		   ntype == tTypeCode ||
		   (mapping.is_reference && mapping.is_pointer))
		  && !mapping.is_arrayslice)
		{
		  // These are declared as <type>_var variable 
		  if (ntype == tString) {
		    produceSizeCalculation(s,a->field_type(),
					   (AST_Decl*)&def_in,
					   "_0RL_s","_0RL_msgsize",
					   a->uqname(),ntype,mapping);
		  }
		  else {
		    // use operator->() to get to the pointer
		    char *_argname = new char[strlen(a->uqname())+
					     strlen("(.operator->())")+1];
		    strcpy(_argname,"(");
		    strcat(_argname,a->uqname());
		    strcat(_argname,".operator->())");
		    produceSizeCalculation(s,a->field_type(),
					   (AST_Decl*)&def_in,
					   "_0RL_s","_0RL_msgsize",
					   _argname,ntype,mapping);
		    delete [] _argname;
		  }
		}
	      else
		{
		  produceSizeCalculation(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_s","_0RL_msgsize",
					 a->uqname(),ntype,mapping);
		}
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceSizeCalculation(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_s","_0RL_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_IN:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "_0RL_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_0RL_msgsize);\n";

  // marshall results
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if ((ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	   mapping.is_pointer)
	  && !mapping.is_arrayslice) 
	{
	  // These are declared as <type>_var variable 
	  if (ntype == tString) 
	    {
	      produceMarshalCode(s,return_type(),
				 (AST_Decl*)&def_in,
				 "_0RL_s",
				 "_0RL_result",ntype,mapping);
	    }
	  else 
	    {
	      // use operator->() to get to the pointer
	      produceMarshalCode(s,return_type(),(AST_Decl*)&def_in,
				 "_0RL_s",
				 "(_0RL_result.operator->())",ntype,mapping);
	    }
	}
      else
	{
	  produceMarshalCode(s,return_type(),(AST_Decl*)&def_in,
			     "_0RL_s","_0RL_result",ntype,mapping);
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
	      if ((ntype == tObjref || ntype == tString || ntype == tTypeCode
		   || (mapping.is_reference && mapping.is_pointer))
		  && !mapping.is_arrayslice)
		{
		  // These are declared as <type>_var variable 
		  if (ntype == tString) 
		    {
		      produceMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_s",
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
		      produceMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_s",
					 _argname,ntype,mapping);
		      delete [] _argname;
		    }
		}
	      else
		{
		  produceMarshalCode(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_s",a->uqname(),ntype,mapping);
		}
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceMarshalCode(s,a->field_type(),
				 (AST_Decl*)&def_in,
				 "_0RL_s",a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_IN:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "_0RL_s.ReplyCompleted();\n";
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
  s << "#ifdef NEED_DUMMY_RETURN\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	((ntype != tObjref && ntype != tString && ntype != tTypeCode)?" *":"") 
	    << " _0RL_result" << " = "
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result";
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
		s << o2be_name::narrow_and_produce_unambiguous_name(eval,this)
		  << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _0RL_result;\n";
    }
  else
    {
      IND(s); s << "return;\n";
    }
  DEC_INDENT_LEVEL();
  s << "#endif\n";
  IND(s); s << "}\n";
}

void
o2be_operation::produce_lcproxy_skel(fstream &s,o2be_interface &def_in,
				     const char* alias_prefix)
{
  idl_bool hasVariableLenOutArgs = I_FALSE;

  IND(s); produce_decl(s,def_in.lcproxy_fqname(),alias_prefix,I_FALSE,I_TRUE);
  s << "\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong _0RL_retries = 0;\n";
  s << "_0RL_again:\n";
  IND(s); s << "assertObjectExistent();\n";
  IND(s); s << "omniRopeAndKey _0RL_r;\n";
  IND(s); s << "CORBA::Boolean _0RL_fwd = getRopeAndKey(_0RL_r);\n";
  IND(s); s << "CORBA::Boolean _0RL_reuse = 0;\n";

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
	    if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
		(mapping.is_arrayslice) ||
		(mapping.is_reference && mapping.is_pointer)) 
	      {
		hasVariableLenOutArgs = I_TRUE;
		// Declare a local pointer variable
		IND(s);
		declareVarType(s,a->field_type(),this,0,mapping.is_arrayslice);
		s << ((ntype != tObjref && ntype != tString && 
		       ntype != tTypeCode)?" *":"") 
		  << " _" << a->uqname() << " = "
		  << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
		  << ";\n";
	      }
	  }
	i.next();
      }
  }
  if (!return_is_void()) 
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  hasVariableLenOutArgs = I_TRUE;
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	((ntype != tObjref && ntype != tString && ntype != tTypeCode)?" *":"") 
	    << " _0RL_result = " 
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";

	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result;\n";
	}
    }

  IND(s); s << "try {\n";
  INC_INDENT_LEVEL();

  IND(s); s << "GIOP_C _0RL_c(_0RL_r.rope());\n";
  IND(s); s << "_0RL_reuse = _0RL_c.isReUsingExistingConnection();\n";
  
  // calculate request message size
  IND(s); s << "CORBA::ULong _0RL_msgsize = GIOP_C::RequestHeaderSize(_0RL_r.keysize(),"
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
	      produceSizeCalculation(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_c","_0RL_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceSizeCalculation(s,a->field_type(),
				     (AST_Decl*)&def_in,
				     "_0RL_c","_0RL_msgsize",
				     a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    break;
	  }
	i.next();
      }
  }


  IND(s); s << "_0RL_c.InitialiseRequest(_0RL_r.key(),_0RL_r.keysize(),(char *)\""
	    << local_name()->get_string() << "\"," << strlen(local_name()->get_string()) + 1 << ",_0RL_msgsize,"
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
	      produceMarshalCode(s,a->field_type(),
				 (AST_Decl*)&def_in,"_0RL_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_INOUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wINOUT,mapping);
	      produceMarshalCode(s,a->field_type(),
				 (AST_Decl*)&def_in,
				 "_0RL_c",
				 a->uqname(),ntype,mapping);
	      break;
	    }
	  case AST_Argument::dir_OUT:
	      break;
	  }
	i.next();
      }
  }

  IND(s); s << "switch (_0RL_c.ReceiveReply())\n";  // invoke method
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
		  declareVarType(s,a->field_type(),this);
		  s << ";\n";
		}
		else if (ntype == tTypeCode) {
		  IND(s); s << "_" << a->uqname() 
			    << " = new CORBA::TypeCode(CORBA::tk_null);\n";
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
	    IND(s); s << "_0RL_result = ";
	    AST_Decl *truetype = return_type();
	    while (truetype->node_type() == AST_Decl::NT_typedef) {
	      truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
	    }
	    s << o2be_array::narrow_from_decl(truetype)->fqname()
	      << "_alloc();\n";

	  }
	  else if (mapping.is_pointer) {
	    IND(s); s << "_0RL_result = new ";
	    declareVarType(s,return_type(),this);
	    s << ";\n";
	  }
	  else if (ntype == tTypeCode) {
	      IND(s); 
	      s << "_0RL_result = new CORBA::TypeCode(CORBA::tk_null);\n";
	    }
	}
    }

  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      produceUnMarshalCode(s,return_type(),(AST_Decl*)&def_in,
			   "_0RL_c","_0RL_result",ntype,mapping);
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
		    declareVarType(s,a->field_type(),this,0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_c",_argname,
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
		    declareVarType(s,a->field_type(),this,0,0);
		    s << " " << _argname << ";\n";
		    produceUnMarshalCode(s,a->field_type(),
					 (AST_Decl*)&def_in,
					 "_0RL_c",_argname,
					 ntype,mapping);
		    IND(s); s << "CORBA::string_free(" << a->uqname() << ");\n";
		    IND(s); s << a->uqname() << " = " << _argname << ";\n";
		    delete [] _argname;
		  }
		  break;
		default:
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",a->uqname(),
				       ntype,mapping);
		  break;
		}
	      break;
	    }
	  case AST_Argument::dir_OUT:
	    {
	      ntype = ast2ArgMapping(a->field_type(),wOUT,mapping);
	      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
		  (mapping.is_arrayslice) ||
		  (mapping.is_reference && mapping.is_pointer)) 
		{
		  char *_argname = new char[strlen(a->uqname())+2];
		  strcpy(_argname,"_");
		  strcat(_argname,a->uqname());
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",_argname,
				       ntype,mapping);
		  delete [] _argname;
		}
	      else
		{
		  produceUnMarshalCode(s,a->field_type(),
				       (AST_Decl*)&def_in,
				       "_0RL_c",a->uqname(),
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

  IND(s); s << "_0RL_c.RequestCompleted();\n";

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
		if (ntype == tObjref || ntype == tString || ntype == tTypeCode
		    || (mapping.is_arrayslice) ||
		    (mapping.is_reference && mapping.is_pointer)) 
		  {
		    IND(s); s << a->uqname() << " = _" << a->uqname() << ";\n";
		  }
	      }
	    i.next();
	  }
      }
      if (!return_is_void()) {
	IND(s); s << "return _0RL_result;\n";
      }
      else {
	IND(s); s << "return;\n";
      }
    }
  else
    {
      if (!return_is_void()) {
	IND(s); s << "return _0RL_result;\n";
      }
      else {
	IND(s); s << "return;\n";
      }
    }
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  
  IND(s); s << "case GIOP::USER_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  if (no_user_exception()) {
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
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
    IND(s); s << "CORBA::Char _0RL_excId[" << maxIdsize << "];\n";
    IND(s); s << "CORBA::ULong _0RL_len;\n";
    IND(s); s << "_0RL_len <<= _0RL_c;\n";
    IND(s); s << "if (_0RL_len > " << maxIdsize << ") {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
    IND(s); s << "throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.get_char_array(_0RL_excId,_0RL_len);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";

    UTL_ExceptlistActiveIterator i(exceptions());
    idl_bool firstexc = I_TRUE;
    while (!i.is_done())
      {
	o2be_exception *excpt = o2be_exception::narrow_from_decl(i.item());
	IND(s); s << ((firstexc)? "if" : "else if") 
		  << " (strcmp((const char *)_0RL_excId,"
		  << excpt->repoIdConstName()
		  << ") == 0) {\n";
	INC_INDENT_LEVEL();
	IND(s); s << excpt->fqname() << " _0RL_ex;\n";
	argType ntype = tStructVariable;
	argMapping mapping = {I_FALSE,I_TRUE,I_FALSE,I_FALSE};
	produceUnMarshalCode(s,i.item(),
			     (AST_Decl*)&def_in,
			     "_0RL_c","_0RL_ex",ntype,mapping);
	IND(s); s << "_0RL_c.RequestCompleted();\n";
	IND(s); s << "throw _0RL_ex;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	i.next();
	firstexc = I_FALSE;
      }
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_0RL_c.RequestCompleted(1);\n";
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
  IND(s); s << "_0RL_c.RequestCompleted(1);\n";
  IND(s); s << "throw omniORB::fatalException(__FILE__,__LINE__,\"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()\");\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::LOCATION_FORWARD:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_0RL_c);\n";
  IND(s); s << "_0RL_c.RequestCompleted();\n";
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
  IND(s); s << "_0RL_c.~GIOP_C();\n";
  IND(s); s << def_in.wrapproxy_fqname() << " *_0RL_w = _get_wrap_"
	    << def_in._fqname() << "();\n";
  IND(s); s << "_0RL_w->_forward_to(obj);\n";
  IND(s); s << "if (omniORB::traceLevel > 10) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "cerr << \"GIOP::LOCATION_FORWARD: retry request.\" << endl;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s);
  if (!return_is_void()) {
    s << "return _0RL_w->";
    produce_invoke(s);
    s << ";\n";
  }
  else {
    s << "_0RL_w->";
    produce_invoke(s);
    s << ";\n";
    IND(s); s << "return;\n";
  }
  IND(s); s << "break;\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << "if (_0RL_reuse) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::TRANSIENT _0RL_ex2(ex.minor(),ex.completed());\n";
  IND(s); s << "if (!_omni_callTransientExceptionHandler(this,_0RL_retries++,_0RL_ex2))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw _0RL_ex2;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();

  IND(s); s << def_in.wrapproxy_uqname()
	    << " *_0RL_w = _get_wrap_" << def_in._fqname() << "();\n";
  IND(s); s << "if (_0RL_w->_forwarded()) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_0RL_w->_reset_proxy();\n";
  IND(s); s << "CORBA::TRANSIENT _0RL_ex2(ex.minor(),ex.completed());\n";

  IND(s); s << "if (_omni_callTransientExceptionHandler(this,_0RL_retries++,_0RL_ex2)) {\n";
  INC_INDENT_LEVEL();
  IND(s);
  if (!return_is_void()) {
    s << "return _0RL_w->";
    produce_invoke(s);
    s << ";\n";
  }
  else {
    s << "_0RL_w->";
    produce_invoke(s);
    s << ";\n";
    IND(s); s << "return;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw _0RL_ex2;\n";
  DEC_INDENT_LEVEL();

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (!_omni_callCommFailureExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (const CORBA::TRANSIENT& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << "if (!_omni_callTransientExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "catch (const CORBA::OBJECT_NOT_EXIST& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }

  IND(s); s << def_in.wrapproxy_uqname()
	    << " *_0RL_w = _get_wrap_" << def_in._fqname() << "();\n";
  IND(s); s << "if (_0RL_w->_forwarded()) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_0RL_w->_reset_proxy();\n";
  
  IND(s); s << "CORBA::TRANSIENT _0RL_ex2(ex.minor(),ex.completed());\n";

  IND(s); s << "if (_omni_callTransientExceptionHandler(this,_0RL_retries++,_0RL_ex2)) {\n";
  INC_INDENT_LEVEL();
  IND(s);
  if (!return_is_void()) {
    s << "return _0RL_w->";
    produce_invoke(s);
    s << ";\n";
  }
  else {
    s << "_0RL_w->";
    produce_invoke(s);
    s << ";\n";
    IND(s); s << "return;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw _0RL_ex2;\n";
  DEC_INDENT_LEVEL();

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (!_omni_callSystemExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";


  IND(s); s << "catch (const CORBA::SystemException& ex) {\n";
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }
  IND(s); s << "if (!_omni_callSystemExceptionHandler(this,_0RL_retries++,ex))\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
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
	    else if (ntype == tTypeCode)
	      {
		IND(s); s << "if (!CORBA::is_nil(_" << a->uqname() << ")) "
			  << "CORBA::release(_" << a->uqname() << ");\n";
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
	  IND(s); s << "if (_0RL_result) delete [] _0RL_result;\n";
	}
      else if (ntype == tObjref)
	{
	  IND(s); s << "if (_0RL_result) CORBA::release(_0RL_result);\n";
	}
      else if (ntype == tString)
	{
	  IND(s); s << "if (_0RL_result) CORBA::string_free(_0RL_result);\n";
	}
      else if (mapping.is_pointer)
	{
	  IND(s); s << "if (_0RL_result) delete _0RL_result;\n";
	}
      else if (ntype == tTypeCode)
	{
	  IND(s); s << "if (!CORBA::is_nil(_0RL_result)) CORBA::release(_0RL_result);\n";
	}
    }
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "goto _0RL_again;\n";

  if (!return_is_void())
    {
      s << "#ifdef NEED_DUMMY_RETURN\n";
      IND(s); s << "{\n";
      INC_INDENT_LEVEL();
      IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	((ntype != tObjref && ntype != tString && ntype != tTypeCode)?" *":"") 
	    << " _0RL_result = "
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result";
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
		s << o2be_name::narrow_and_produce_unambiguous_name(eval,this) << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _0RL_result;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      s << "#endif\n";
    }
    
  DEC_INDENT_LEVEL();
  IND(s);s << "}\n";
  return;
}

void
o2be_operation::produce_dead_skel(fstream& s, const char* alias_prefix)
{
  IND(s); produce_decl(s,0,alias_prefix,I_FALSE,I_TRUE);
  s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);\n";
  s << "#ifdef NEED_DUMMY_RETURN\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  if (!return_is_void())
    {
      argMapping mapping;
      argType ntype = ast2ArgMapping(return_type(),wResult,mapping);
      if (ntype == tObjref || ntype == tString || ntype == tTypeCode ||
	  (mapping.is_arrayslice) ||
	  (mapping.is_pointer))
	{
	  IND(s);
	  declareVarType(s,return_type(),this,0,mapping.is_arrayslice);
	  s << 
	 ((ntype != tObjref && ntype != tString && ntype !=tTypeCode)?" *":"") 
	    << " _0RL_result = "
	    << ((ntype == tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	    << ";\n";
	}
      else
	{
	  IND(s);
	  declareVarType(s,return_type(),this);
	  s << " _0RL_result";
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
		s << o2be_name::narrow_and_produce_unambiguous_name(eval,this)
		  << ";\n";
	      }
	      break;
	    case tStructFixed:
	      s << ";\n";
	      s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	      break;
	    default:
	      s << ";\n";
	      break;
	    }
	}
      IND(s); s << "return _0RL_result;\n";
    }
  else
    {
      IND(s); s << "return;\n";
    }
  DEC_INDENT_LEVEL();
  s << "#endif\n";
  IND(s); s << "}\n";
}

void
o2be_operation::produce_home_skel(fstream& s, o2be_interface &def_in,
				  const char* alias_prefix)
{
  IND(s); produce_decl(s,0,alias_prefix,I_FALSE,I_TRUE);
  s << "{\n";
  INC_INDENT_LEVEL();
  IND(s);
  if (!return_is_void())
    s << "return ";
  s << "_actual_" << def_in._fqname() << "->";
  produce_invoke(s);
  s << ";\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}

void
o2be_operation::produce_wrapproxy_skel(fstream& s,
				       o2be_interface &def_in,
				       const char* alias_prefix)
{
  IND(s); produce_decl(s,0,alias_prefix,I_FALSE,I_TRUE);
  s << "{\n";
  INC_INDENT_LEVEL();
  IND(s);
  if (!return_is_void())
    s << "return ";
  s << "_actual_" << def_in._fqname() << "->";
  produce_invoke(s);
  s << ";\n";
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
	str = o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	s << str;
	indent_pos += strlen(str);
      }
      else if (ntype == tString) {
	s << "char *";
	indent_pos += 6;
      }
      else if (ntype == tTypeCode) {
	s << "CORBA::TypeCode_ptr";
	indent_pos += 19;
      }
      else {
	str = o2be_name::narrow_and_produce_unambiguous_name(return_type(),this);
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
	    case tTypeCode:
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
	    case tTypeCode:
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
	      s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	    }
	    else if (ntype == tString) {
	      s << "char *";
	    }
	    else if (ntype == tTypeCode) {
	      AST_Decl *decl = a->field_type();
	      while (decl->node_type() == AST_Decl::NT_typedef) {
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	      }
	      s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	    }
	    else {
	      s << o2be_name::narrow_and_produce_unambiguous_name(a->field_type(),this);
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
		  s << intf->inout_adptarg_name(this) << " ";
		}
		else {
		  s << intf->out_adptarg_name(this) << " ";
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
	    case tTypeCode:
	      {
		if (a->direction() == AST_Argument::dir_INOUT) {
		  s << "CORBA::TypeCode_INOUT_arg ";
		}
		else {
		  s << "CORBA::TypeCode_OUT_arg ";
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
		  s << p->out_adptarg_name(this) << " ";
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
		  s << p->out_adptarg_name(this) << " ";
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
		  s << p->out_adptarg_name(tp,this) << " ";
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
		  s << p->out_adptarg_name(tp,this) << " ";
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
	  if (ntype == tObjref || ntype == tString || ntype == tTypeCode) {
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
	  case tTypeCode:
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
	  case tTypeCode:
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
	  case tTypeCode:
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
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tTypeCode
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
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tTypeCode
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
  { I_FALSE, I_TRUE,  I_FALSE, I_FALSE }, // tTypeCode
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
  { I_FALSE, I_FALSE, I_FALSE, I_FALSE }, // tTypeCode
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
	  case AST_PredefinedType::PT_TypeCode:
	    ntype = tTypeCode;
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
o2be_operation::declareVarType(fstream &s,AST_Decl *decl,AST_Decl* used_in,
			       idl_bool is_var,idl_bool is_arrayslice)
{
  AST_Decl *truetype = decl;
  while (truetype->node_type() == AST_Decl::NT_typedef) {
    truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
  }

  if (truetype->node_type() == AST_Decl::NT_interface)
    {
      if (!is_var)
	s << o2be_interface::narrow_from_decl(truetype)->unambiguous_objref_name(used_in);
      else
	s << o2be_name::narrow_and_produce_unambiguous_name(truetype,used_in) << "_var";
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
    if (truetype->node_type() == AST_Decl::NT_pre_defined)
      {
	    if (AST_PredefinedType::narrow_from_decl(truetype)->pt()
		== AST_PredefinedType::PT_TypeCode)
	      {
		if (is_var)  s << "CORBA::TypeCode_var";
		else s << "CORBA::TypeCode_ptr";
	      }
	    else
	      {
		s << 
		  o2be_name::narrow_and_produce_unambiguous_name(decl,used_in);
		if (is_var)
		  s << "_var";
		else if (is_arrayslice)
		  s << "_slice";
	      }
      }
  else
    {
      s << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in);
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
				     AST_Decl* used_in,
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
    case tTypeCodeMember:
      IND(s); s << ((mapping.is_pointer)?"*":"") << argname << " <<= "
		<< netstream << ";\n";
      break;

    case tString:
      {      
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	IND(s); s << "CORBA::ULong _len;\n";
	IND(s); s << "_len <<= " << netstream << ";\n";
	IND(s); s << "if (!_len) {\n";
	INC_INDENT_LEVEL();
	IND(s); s << "if (omniORB::traceLevel > 1)\n";
	INC_INDENT_LEVEL();
	IND(s); s << "_CORBA_null_string_ptr(1);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "_len = 1;\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	if (!no_size_check) 
	  {
	    IND(s); s << "else if ( " << netstream << ".RdMessageUnRead() < _len)\n";
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
	IND(s); s << "if (_len > 1)\n";
	INC_INDENT_LEVEL();
	IND(s); s << netstream << ".get_char_array((CORBA::Char *)" 
		  << "((char *)" << argname << "),_len);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "else\n";
	INC_INDENT_LEVEL();
	IND(s); s << "*((CORBA::Char*)((char*)" << argname << ")) <<= " << netstream << ";\n";
	DEC_INDENT_LEVEL();
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

    case tTypeCode:
      IND(s); s << "*" << argname << " <<= " << netstream << ";\n";
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
		  IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      if (!mapping.is_arrayslice) {
		IND(s); s << argname;
	      }
	      else {
		IND(s); s << "(("
			  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
			  << "_slice*) " << argname << ")";
	      }
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
	    IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
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
	  case tTypeCode:
	    {
	      if (!mapping.is_arrayslice) {
		IND(s); s << argname;
	      }
	      else {
		IND(s); s << "(("
			  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
			  << "_slice*) " << argname << ")";
	      }
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
	      IND(s); s << "if (!_len) {\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "if (omniORB::traceLevel > 1)\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "_CORBA_null_string_ptr(1);\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << "_len = 1;\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	      if (!no_size_check)
		{
		  IND(s); s << "else if ( " << netstream << ".RdMessageUnRead() < _len)\n";
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


	      IND(s); s << "if (!(char*)(";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
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

	      IND(s); s << "if (_len > 1)\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << netstream << ".get_char_array((CORBA::Char *)((char *)";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "),_len);\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << "else\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "*((CORBA::Char*)((char*) ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ")) <<= " << netstream << " ;\n";
	      DEC_INDENT_LEVEL();	      
	      break;
	    }
	  case tObjref:
	    {
	      if (!mapping.is_arrayslice) {
		IND(s); s << argname;
	      }
	      else {
		IND(s); s << "(("
			  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
			  << "_slice*) " << argname << ")";
	      }
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
				   AST_Decl* used_in,
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
    case tTypeCodeMember:
      IND(s); s << ((mapping.is_pointer)?"*":"") << argname 
		<< " >>= " << netstream << ";\n";
      break;
    case tTypeCode:
      IND(s); s << "*" << argname << " >>= " << netstream << ";\n";
      break;
    case tString:
      {      
	IND(s); s << "{\n";
	INC_INDENT_LEVEL();
	IND(s); s << "CORBA::ULong _len = (((const char*) " << argname
		  << ")? strlen((const char*) " << argname
		  << ") + 1 : 1);\n"; 
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
        IND(s); s << "if (_len > 1)\n";
	INC_INDENT_LEVEL();
	IND(s); s << netstream << ".put_char_array((const CORBA::Char *)((const char*) " 
		  << argname << "),_len);\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "else {\n";
	INC_INDENT_LEVEL();
	IND(s); s << "if ((const char*) " << argname << " == 0 && omniORB::traceLevel > 1)\n";
	INC_INDENT_LEVEL();
	IND(s); s << "_CORBA_null_string_ptr(0);\n";	
	DEC_INDENT_LEVEL();
	IND(s); s << "CORBA::Char _tempchar = '\\0';\n";
	IND(s); s << "_tempchar >>= " << netstream << ";\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      break;

    case tObjref:
      {
	IND(s); s << o2be_interface::narrow_from_decl(decl)->fqname()
		  << "::marshalObjRef(" << argname << ","
		  << netstream << ");\n";
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
		  IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      if (!mapping.is_arrayslice) {
		IND(s); s << argname;
	      }
	      else {
		IND(s); s << "(("
			  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
			  << "_slice*) " << argname << ")";
	      }
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
	    IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
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
	      if (!mapping.is_arrayslice) {
		IND(s); s << argname;
	      }
	      else {
		IND(s); s << "(("
			  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
			  << "_slice*) " << argname << ")";
	      }
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
	      IND(s); s << "CORBA::ULong _len = (((const char*) ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ")? strlen((const char*) ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ") + 1 : 1);\n"; 

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

	      IND(s); s << "if (_len > 1)\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << netstream << ".put_char_array((const CORBA::Char *)((const char*)";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "),_len);\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << "else {\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "if ((const char*) ";

	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " == 0 && omniORB::traceLevel > 1)\n";
	      INC_INDENT_LEVEL();
	      IND(s); s << "_CORBA_null_string_ptr(0);\n";	
	      DEC_INDENT_LEVEL();
	      IND(s); s << "CORBA::Char _tempchar = '\\0';\n";
	      IND(s); s << "_tempchar >>= " << netstream << ";\n";
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	      break;
	    }
	  case tObjref:
	    {
	      AST_Decl *tdecl = o2be_array::narrow_from_decl(decl)->getElementType();
	      while (tdecl->node_type() == AST_Decl::NT_typedef) {
		tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();
	      }
	      IND(s); s << o2be_interface::narrow_from_decl(tdecl)->fqname()
			<< "::marshalObjRef(";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "._ptr," << netstream << ");\n";
 	    }
	    break;

	  case tTypeCode:
	    {
	      IND(s); s << "*(" << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "._ptr) >>= " << netstream << ";\n";
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
				       AST_Decl* used_in,
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
    case tTypeCodeMember:
      IND(s); s << sizevar << " = "
		<< argname << ((mapping.is_pointer)?"->":".")
		<< "NP_alignedSize(" << sizevar << ");\n";
      break;

    case tTypeCode:
      IND(s); s << sizevar << " = "
		<< argname << "->NP_alignedSize(" << sizevar << ");\n";
      break;

    case tString:
      IND(s); s << sizevar << " = omni::align_to(" << sizevar 
		<< ",omni::ALIGN_4);\n";
      IND(s); s << sizevar << " += 4 + (((const char*) " << argname
		  << ")? strlen((const char*) " << argname
		  << ") + 1 : 1);\n";      
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
		  IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << sizevar << " = ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
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
	    IND(s); s << "for (CORBA::ULong _i" << ndim << " =0;"
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
	      IND(s); s << sizevar << " = ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
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
	      IND(s); s << sizevar << " += 4 + (((const char*) ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ")? strlen((const char*) ";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ") + 1 : 1);\n";
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
			<< "::NP_alignedSize(";
	      if (!mapping.is_arrayslice) {
		s << argname;
	      }
	      else {
		s << "(("
		  << o2be_name::narrow_and_produce_unambiguous_name(decl,used_in)
		  << "_slice*) " << argname << ")";
	      }
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "._ptr," << sizevar << ");\n";
 	    }
	    break;

	  case tTypeCode:
	    {
	      IND(s); s << sizevar << " = ((" << argname;
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << "._ptr)->NP_alignedSize(" << sizevar << "));\n";
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





