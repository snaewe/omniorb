// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_attribute.cc        Created on: 13/09/1996
//			    Author    : Sai-Lai Lo (sll)
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

/*
  $Log$
  Revision 1.8  1997/05/06 13:50:29  sll
  Public release.

  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

o2be_attribute::o2be_attribute(idl_bool ro,
			       AST_Type *ft, 
			       UTL_ScopedName *n, 
			       UTL_StrList *p)
              : AST_Attribute(ro,ft,n,p),
		AST_Field(AST_Decl::NT_attr,ft,n,p),
		AST_Decl(AST_Decl::NT_attr,n,p),
		o2be_name(this)
{
}

void
o2be_attribute::produce_decl_rd(fstream &s,const char *prefix,
				idl_bool out_var_default)
{
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =  o2be_operation::ast2ArgMapping(field_type(),
					      o2be_operation::wResult,mapping);
  if (ntype == o2be_operation::tObjref) {
    AST_Decl *decl = field_type();
    while (decl->node_type() == AST_Decl::NT_typedef) {
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    }
    s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
  }
  else if (ntype == o2be_operation::tString) {
    s << "char *";
  }
  else {
    s << o2be_name::narrow_and_produce_fqname(field_type());
  }
  s << ((mapping.is_arrayslice) ? "_slice":"")
    << " "
    << ((mapping.is_pointer)    ? "*":"")
    << ((mapping.is_reference)  ? "&":"");
  s << " ";
  if (prefix)
    s << prefix << "::";
  s << uqname() << " () ";
  return;
}

void
o2be_attribute::produce_decl_wr(fstream &s,const char *prefix,
				idl_bool out_var_default)
{
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =  o2be_operation::ast2ArgMapping(field_type(),
					o2be_operation::wIN,mapping);
  s << "void ";
  if (prefix)
    s << prefix << "::";
  s << uqname() << " (";
  s << ((mapping.is_const) ? "const ":"");
  if (ntype == o2be_operation::tObjref) {
    AST_Decl *decl = field_type();
    while (decl->node_type() == AST_Decl::NT_typedef) {
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    }
    s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
  }
  else if (ntype == o2be_operation::tString) {
    s << "char *";
  }
  else {
    s << o2be_name::narrow_and_produce_fqname(field_type());
  }
  s  << ((mapping.is_arrayslice) ? "_slice":"")
    << " "
    << ((mapping.is_pointer)    ? "*":"")
    << ((mapping.is_reference)  ? "&":"");
  s << " _value)";
  return;
}

void
o2be_attribute::produce_proxy_rd_skel(fstream &s,o2be_interface &defined_in)
{
  idl_bool hasVariableLenOutArgs = I_FALSE;

  IND(s); produce_decl_rd(s,defined_in.proxy_fqname(),I_FALSE);
  s << " {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "assertObjectExistent();\n";
  IND(s); s << "omniRopeAndKey _r;\n";
  IND(s); s << "CORBA::Boolean _fwd = getRopeAndKey(_r);\n";

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer))
      {
	hasVariableLenOutArgs = I_TRUE;
	IND(s);
	o2be_operation::declareVarType(s,field_type(),0,mapping.is_arrayslice);
	s << ((ntype != o2be_operation::tObjref && 
	       ntype != o2be_operation::tString)?" *":"") 
	  << " _result" << "= 0;\n";
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type());
	s << " _result;\n";
      }
  }

  IND(s); s << "try {\n";
  INC_INDENT_LEVEL();

  IND(s); s << "GIOP_C _c(_r.rope());\n";

  // calculate request message size
  IND(s); s << "CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(_r.keysize(),"
	    << strlen("_get_") + strlen(local_name()->get_string()) + 1 
	    << ");\n";

  IND(s); s << "_c.InitialiseRequest(_r.key(),_r.keysize(),(char *)\""
	    << "_get_" << local_name()->get_string() << "\"," << strlen("_get_") + strlen(local_name()->get_string()) + 1 << ",_msgsize,0);\n";

  IND(s); s << "switch (_c.ReceiveReply())\n";  // invoke method
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();

  IND(s); s << "case GIOP::NO_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  // unmarshall results
  if (hasVariableLenOutArgs)
    {
      o2be_operation::argMapping mapping;
      o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
      if (mapping.is_arrayslice) {
	IND(s); s << "_result = ";
	AST_Decl *truetype = field_type();
	while (truetype->node_type() == AST_Decl::NT_typedef) {
	  truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
	}
	s << o2be_array::narrow_from_decl(truetype)->fqname();
	s  << "_alloc();\n";
      }
      else if (mapping.is_pointer) {
	IND(s); s << "_result = new ";
	o2be_operation::declareVarType(s,field_type());
	s << ";\n";
      }
    }

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    o2be_operation::produceUnMarshalCode(s,field_type(),"_c","_result",ntype,mapping);
  }

  IND(s); s << "_c.RequestCompleted();\n";

  if (hasVariableLenOutArgs)
    {
      IND(s); s << "return _result;\n";

    }
  else
    {
      IND(s); s << "return _result;\n";
    }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::USER_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_c.RequestCompleted(1);\n";
  IND(s); s << "throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);\n";
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
  IND(s); s << "return " << uqname() << "();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "catch (const CORBA::COMM_FAILURE& ex) {\n";
  INC_INDENT_LEVEL();
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
	
    if (mapping.is_arrayslice)
      {
	IND(s); s << "if (_result) delete [] _result;\n";
      }
    else if (mapping.is_reference && mapping.is_pointer)
      {
	IND(s); s << "if (_result) delete _result;\n";
      }
    else if (ntype == o2be_operation::tObjref)
      {
	IND(s); s << "if (_result) CORBA::release(_result);\n";
      }
    else if (ntype == o2be_operation::tString)
      {
	IND(s); s << "if (_result) CORBA::string_free(_result);\n";
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

  if (hasVariableLenOutArgs) {
    IND(s); s << "catch (...) {\n";
    INC_INDENT_LEVEL();
    {
      o2be_operation::argMapping mapping;
      o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
	
      if (mapping.is_arrayslice)
	{
	  IND(s); s << "if (_result) delete [] _result;\n";
	}
      else if (mapping.is_reference && mapping.is_pointer)
	{
	  IND(s); s << "if (_result) delete _result;\n";
	}
      else if (ntype == o2be_operation::tObjref)
	{
	  IND(s); s << "if (_result) CORBA::release(_result);\n";
	}
      else if (ntype == o2be_operation::tString)
	{
	  IND(s); s << "if (_result) CORBA::string_free(_result);\n";
	}
    }
    IND(s); s << "throw;\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
  }

  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer))
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),0,mapping.is_arrayslice);
	s << ((ntype != o2be_operation::tObjref && 
	       ntype != o2be_operation::tString)?" *":"") 
	  << " _result" << "= 0;\n";
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type());
	s << " _result";
	switch (ntype)
	  {
	  case o2be_operation::tShort:
	  case o2be_operation::tLong:
	  case o2be_operation::tUShort:
	  case o2be_operation::tULong:
	  case o2be_operation::tFloat:
	  case o2be_operation::tDouble:
	  case o2be_operation::tBoolean:
	  case o2be_operation::tChar:
	  case o2be_operation::tOctet:
	    s << " = 0;\n";
	    break;
	  case o2be_operation::tEnum:
	    {
	      s << " = ";
	      AST_Decl *decl = field_type();
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
	  case o2be_operation::tStructFixed:
	    s << ";\n";
	    s << "memset((void *)&_result,0,sizeof(_result));\n";
	    break;
	  default:
	    s << ";\n";
	    break;
	  }
      }
  }
  IND(s); s << "return _result;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s);s << "}\n";
  return;
}

void 
o2be_attribute::produce_proxy_wr_skel(fstream &s,o2be_interface &defined_in)
{
  idl_bool hasVariableLenOutArgs = I_FALSE;

  IND(s); produce_decl_wr(s,defined_in.proxy_fqname(),I_FALSE);
  s << " {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "assertObjectExistent();\n";
  IND(s); s << "omniRopeAndKey _r;\n";
  IND(s); s << "CORBA::Boolean _fwd = getRopeAndKey(_r);\n";
  IND(s); s << "try {\n";
  INC_INDENT_LEVEL();

  IND(s); s << "GIOP_C _c(_r.rope());\n";
  
  // calculate request message size
  IND(s); s << "CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(_r.keysize(),"
	    << strlen("_set_") + strlen(local_name()->get_string()) + 1 
	    << ");\n";

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wIN,mapping);
    o2be_operation::produceSizeCalculation(s,field_type(),"_c","_msgsize",
					   "_value",ntype,mapping);
  }

  IND(s); s << "_c.InitialiseRequest(_r.key(),_r.keysize(),(char *)\""
	    << "_set_" << local_name()->get_string() << "\"," << strlen("_set_") + strlen(local_name()->get_string()) + 1 << ",_msgsize,0);\n";

  // marshall arguments;
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wIN,mapping);
    o2be_operation::produceMarshalCode(s,field_type(),"_c",
				       "_value",ntype,mapping);
  }

  IND(s); s << "switch (_c.ReceiveReply())\n";  // invoke method
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();

  IND(s); s << "case GIOP::NO_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_c.RequestCompleted();\n";
  IND(s); s << "return;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "case GIOP::USER_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_c.RequestCompleted(1);\n";
  IND(s); s << "throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);\n";
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
  IND(s); s << uqname() << "(_value);\n";
  IND(s); s << "return;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "catch (const CORBA::COMM_FAILURE& ex) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (_fwd) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "resetRopeAndKey();\n";
  IND(s); s << "throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "throw;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  DEC_INDENT_LEVEL();
  IND(s);s << "}\n";
  return;
}

void
o2be_attribute::produce_server_rd_skel(fstream &s,o2be_interface &defined_in)
{
  IND(s); s << "if (!_response_expected) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  IND(s); s << "_s.RequestReceived();\n";

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer)) 
      {
	// declare a <type>_var variable to manage the pointer type
	o2be_operation::declareVarType(s,field_type(),1,mapping.is_arrayslice);
      }
    else 
      {
	o2be_operation::declareVarType(s,field_type());
      }
    s << " _result = " << uqname() << "();\n";
  }

  // calculate reply message size
  IND(s); s << "size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if ((ntype == o2be_operation::tObjref ||
	 ntype == o2be_operation::tString ||
	 mapping.is_pointer) && !mapping.is_arrayslice)
      {
	// These are declared as <type>_var variable 
	if (ntype == o2be_operation::tString) {
	  o2be_operation::produceSizeCalculation(s,field_type(),
						 "_s","_msgsize",
						 "_result",ntype,mapping);
	}
	else {
	  // use operator->() to get to the pointer
	  o2be_operation::produceSizeCalculation(s,field_type(),"_s","_msgsize",
						 "(_result.operator->())",
						 ntype,mapping);
	}
      }
    else
      {
	o2be_operation::produceSizeCalculation(s,field_type(),"_s","_msgsize",
					       "_result",ntype,mapping);
      }
  }

  IND(s); s << "_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);\n";

  // marshall results
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if ((ntype == o2be_operation::tObjref || 
	 ntype == o2be_operation::tString ||
	 mapping.is_pointer)
	&& !mapping.is_arrayslice)
      {
	// These are declared as <type>_var variable 
	if (ntype == o2be_operation::tString) {
	  o2be_operation::produceMarshalCode(s,field_type(),"_s",
					     "_result",ntype,mapping);
	}
	else {
	  // use operator->() to get to the pointer
	  o2be_operation::produceMarshalCode(s,field_type(),"_s",
					     "(_result.operator->())",
					     ntype,mapping);
	}
      }
    else
      {
	o2be_operation::produceMarshalCode(s,field_type(),"_s",
					   "_result",ntype,mapping);
      }
  }

  IND(s); s << "_s.ReplyCompleted();\n";
  IND(s); s << "return 1;\n";
  return;
}

void
o2be_attribute::produce_server_wr_skel(fstream &s,o2be_interface &defined_in)
{
  IND(s); s << "if (!_response_expected) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  {
    // unmarshall arguments
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wIN,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString) 
      // declare a <type>_var variable to manage the pointer type
      o2be_operation::declareVarType(s,field_type(),1);
    else
      o2be_operation::declareVarType(s,field_type());
    s << " " << "_value;\n";
    o2be_operation::produceUnMarshalCode(s,field_type(),"_s","_value",
					 ntype,mapping);
  }

  IND(s); s << "_s.RequestReceived();\n";

  IND(s); s << uqname() << "(_value);\n";

  IND(s); s << "size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";

  IND(s); s << "_s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);\n";

  IND(s); s << "_s.ReplyCompleted();\n";
  IND(s); s << "return 1;\n";
  return;
}

void
o2be_attribute::produce_nil_rd_skel(fstream &s)
{
  IND(s); produce_decl_rd(s);
  s << " {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer))
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),0,mapping.is_arrayslice);
	s << ((ntype != o2be_operation::tObjref && 
	       ntype != o2be_operation::tString)?" *":"") 
	  << " _result" << "= 0;\n";
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type());
	s << " _result";
	switch (ntype)
	  {
	  case o2be_operation::tShort:
	  case o2be_operation::tLong:
	  case o2be_operation::tUShort:
	  case o2be_operation::tULong:
	  case o2be_operation::tFloat:
	  case o2be_operation::tDouble:
	  case o2be_operation::tBoolean:
	  case o2be_operation::tChar:
	  case o2be_operation::tOctet:
	    s << " = 0;\n";
	    break;
	  case o2be_operation::tEnum:
	    {
	      s << " = ";
	      AST_Decl *decl = field_type();
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
	  case o2be_operation::tStructFixed:
	    s << ";\n";
	    s << "memset((void *)&_result,0,sizeof(_result));\n";
	    break;
	  default:
	    s << ";\n";
	    break;
	  }
      }
  }
  IND(s); s << "return _result;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}

void
o2be_attribute::produce_nil_wr_skel(fstream &s)
{
  IND(s); produce_decl_wr(s);
  s << " {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}

IMPL_NARROW_METHODS1(o2be_attribute, AST_Attribute)
IMPL_NARROW_FROM_DECL(o2be_attribute)
IMPL_NARROW_FROM_SCOPE(o2be_attribute)

