// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_attribute.cc        Created on: 13/09/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//

/*
  $Log$
  Revision 1.2  1997/01/13 15:34:22  sll
  Don't use operator->() if the return value is array_slice.
  Don't use new (type)[i]. The brackets are unnecessary.
  /

  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

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
  IND(s); s << "GIOP_C _c(_rope());\n";
  
  // calculate request message size
  IND(s); s << "CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),"
	    << strlen("_get_") + strlen(uqname()) + 1 
	    << ");\n";

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

  IND(s); s << "_c.InitialiseRequest(objkey(),objkeysize(),(char *)\""
	    << "_get_" << uqname() << "\"," << strlen("_get_") + strlen(uqname()) + 1 << ",_msgsize,0);\n";

  IND(s); s << "switch (_c.ReceiveReply())\n";  // invoke method
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();

  IND(s); s << "case GIOP::NO_EXCEPTION:\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  // unmarshall results
  if (hasVariableLenOutArgs)
    {
      IND(s); s << "try {\n";
      INC_INDENT_LEVEL();
      o2be_operation::argMapping mapping;
      o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
      if (mapping.is_arrayslice) {
	IND(s); s << "_result = new ";
	o2be_operation::declareVarType(s,field_type(),0,1);
	AST_Decl *truetype = field_type();
	while (truetype->node_type() == AST_Decl::NT_typedef) {
	  truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
	}
	s << "[";
	s << o2be_array::narrow_from_decl(truetype)->getSliceDim();
	s  << "];\n";
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
      IND(s); s << "return _result;";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
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
  else
    {
      IND(s); s << "return _result;";
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
#if 1
  IND(s); s << "_c.RequestCompleted(1);\n";
  IND(s); s << "throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);\n";
#else
  IND(s); s << "IOP::IOR *newref = new IOP::IOR();\n";
  IND(s); s << "newref <<= _c;\n";
  IND(s); s << "_c.RequestCompleted();\n";
  IND(s); s << "this->newObjRef(newref);\n";
  IND(s); s << "_c.~GIOP_C();\n";
  IND(s); s << "return " << uqname() << "();\n";
#endif
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

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
  IND(s); s << "GIOP_C _c(_rope());\n";
  
  // calculate request message size
  IND(s); s << "CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(objkeysize(),"
	    << strlen("_set_") + strlen(uqname()) + 1 
	    << ");\n";

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wIN,mapping);
    o2be_operation::produceSizeCalculation(s,field_type(),"_c","_msgsize",
					   "_value",ntype,mapping);
  }

  IND(s); s << "_c.InitialiseRequest(objkey(),objkeysize(),(char *)\""
	    << "_set_" << uqname() << "\"," << strlen("_set_") + strlen(uqname()) + 1 << ",_msgsize,0);\n";

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
#if 1
  IND(s); s << "_c.RequestCompleted(1);\n";
  IND(s); s << "throw CORBA::UNKNOWN(2,CORBA::COMPLETED_NO);\n";
#else
  IND(s); s << "IOP::IOR *newref = new IOP::IOR();\n";
  IND(s); s << "newref <<= _c;\n";
  IND(s); s << "_c.RequestCompleted();\n";
  IND(s); s << "this->newObjRef(newref);\n";
  IND(s); s << "_c.~GIOP_C();\n";
  IND(s); s << uqname() << "(_value);\n";
  IND(s); s << "return;\n";
#endif
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

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
	s << " _result;\n";
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

