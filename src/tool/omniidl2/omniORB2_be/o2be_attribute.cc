// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_attribute.cc        Created on: 13/09/1996
//			    Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.23.4.2  1999/11/04 20:16:04  sll
  Server side stub now use a descriptor mechanism similar to the client size
  stub.

  Revision 1.23.4.1  1999/09/15 20:18:45  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.22  1999/06/28 13:24:51  dpg1
  LifeCycle code updated for proxyCallWrapper support.

  Revision 1.21  1999/05/26 10:44:37  sll
  Added connection code to generate typecode constant for anonymous bounded
  string defined used in attributed.

  Revision 1.20  1999/03/11 16:26:10  djr
  Updated copyright notice

  Revision 1.19  1999/01/07 09:51:11  djr
  New implementation of proxy calls which reduces the size of
  the code generated.

  Revision 1.18  1998/08/13 22:35:48  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available

  Revision 1.17  1998/04/07 18:40:29  sll
  Use std::fstream instead of fstream.
  Stub now use omniORB::log to log error messages.

  Revision 1.16  1998/03/25 14:39:12  sll
  *** empty log message ***

  Revision 1.15  1998/03/25 14:19:50  sll
  Temporary work-around for egcs compiler.

  Revision 1.14  1998/01/27 16:34:29  ewc
   Added support for type any and TypeCode

  Revision 1.13  1997/12/10 11:35:10  sll
  Updated life cycle service stub.

  Revision 1.12  1997/12/09 19:54:19  sll
  *** empty log message ***

// Revision 1.11  1997/09/20  16:35:25  dpg1
// New code generation for LifeCycle support.
//
  Revision 1.10  1997/08/22 12:43:23  sll
  Oh well, gcc does not like variable names starting with __, changed
  the prefix to _0RL_.

  Revision 1.9  1997/08/21 21:20:58  sll
  Names of internal variables inside the stub code now all start with the
  prefix __ to avoid potential clash with identifiers defined in IDL.

// Revision 1.8  1997/05/06  13:50:29  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

o2be_attribute::o2be_attribute(idl_bool ro, AST_Type *ft,
			       UTL_ScopedName *n, UTL_StrList *p)
  : AST_Attribute(ro,ft,n,p),
    AST_Field(AST_Decl::NT_attr,ft,n,p),
    AST_Decl(AST_Decl::NT_attr,n,p),
    o2be_name(AST_Decl::NT_attr,n,p)
{
  pd_mangled_read_signature = 0;
  pd_mangled_write_signature = 0;
}


void
o2be_attribute::produce_decl_rd(std::fstream& s,
				idl_bool use_fully_qualified_names)
{
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =
    o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wResult,
				   mapping);
  AST_Decl* decl = field_type();

  switch( ntype ) {
  case o2be_operation::tObjref:
    while( decl->node_type() == AST_Decl::NT_typedef )
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    s << o2be_interface::narrow_from_decl(decl)
      ->unambiguous_objref_name(this, use_fully_qualified_names);
    break;
  case o2be_operation::tString:
    s << "char*";
    break;
  case o2be_operation::tTypeCode:
    s << "CORBA::TypeCode_ptr";
    break;
  default:
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, this,
						use_fully_qualified_names);
    break;
  }

  if( mapping.is_arrayslice )  s << "_slice";
  if( mapping.is_pointer    )  s << "*";
  if( mapping.is_reference  )  s << "&";
}


void
o2be_attribute::produce_decl_wr(std::fstream& s,
				idl_bool use_fully_qualified_names,
				idl_bool for_call_desc)
{
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =
    o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wIN,mapping);
  AST_Decl* decl = field_type();

  if( mapping.is_const )  s << "const ";

  switch( ntype ) {
  case o2be_operation::tObjref:
    while( decl->node_type() == AST_Decl::NT_typedef )
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    s << o2be_interface::narrow_from_decl(decl)
      ->unambiguous_objref_name(this, use_fully_qualified_names);
    break;
  case o2be_operation::tString:
    s << "char*";
    break;
  case o2be_operation::tTypeCode:
    s << "CORBA::TypeCode_ptr";
    break;
  case o2be_operation::tArrayFixed:
  case o2be_operation::tArrayVariable:
    // We use pointer to slice for passing arrays to call descriptors ...
    if( for_call_desc ) {
      mapping.is_arrayslice = I_TRUE;
      mapping.is_pointer = I_TRUE;
    }
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, this,
						use_fully_qualified_names);
    break;
  default:
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, this,
						use_fully_qualified_names);
    break;
  }
  if( mapping.is_arrayslice )  s << "_slice";
  if( mapping.is_pointer    )  s << "*";
  if( mapping.is_reference  )  s << "&";
}


void
o2be_attribute::produce_read_proxy_call_desc(std::fstream& s,
					     const char* class_name)
{
  IND(s); s << "// Proxy call descriptor class. Mangled signature:\n";
  IND(s); s << "//  " << mangled_read_signature() << '\n';
  IND(s); s << "class " << class_name << '\n';
  IND(s); s << "  : public " << "OmniProxyCallDesc" << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();

  // Constructor.
  IND(s); s << "inline " << class_name
	    << "(const char* _op, size_t _op_len) :\n";
  IND(s); s << "  OmniProxyCallDesc(_op, _op_len)  {}\n\n";

  // Declaration of methods to implement the call.
  IND(s); s << "virtual void unmarshalReturnedValues(cdrStream&);\n";

  // Result accessor.
  IND(s); s << "inline ";
  produce_decl_rd(s, I_TRUE);
  s << " result() { return pd_result; }\n";

  s << "\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "private:\n";
  INC_INDENT_LEVEL();

  // Private data members - return value.
  IND(s); produce_decl_rd(s, I_TRUE);
  s << " pd_result;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";


  IND(s); s << "void " << class_name
	      << "::unmarshalReturnedValues(cdrStream& giop_client)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype =
      o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wResult,
				     mapping);

    // Allocate memory for the return value.
    if (mapping.is_arrayslice) {
      IND(s); s << "pd_result = ";
      AST_Decl* truetype = field_type();
      while( truetype->node_type() == AST_Decl::NT_typedef )
	truetype = o2be_typedef::narrow_from_decl(truetype)->base_type();
      s << o2be_array::narrow_from_decl(truetype)->fqname()
	<< "_alloc();\n";
    }
    else if (mapping.is_pointer) {
      IND(s); s << "pd_result = new ";
      o2be_operation::declareVarType(s, field_type(), o2be_global::root());
      s << ";\n";
    }

    // Unmarshal it.
    o2be_operation::produceUnMarshalCode(s, field_type(), o2be_global::root(),
					 "giop_client", "pd_result", ntype,
					 mapping);
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


void
o2be_attribute::produce_write_proxy_call_desc(std::fstream& s,
					      const char* class_name)
{
  IND(s); s << "// Proxy call descriptor class. Mangled signature:\n";
  IND(s); s << "//  " << mangled_write_signature() << '\n';
  IND(s); s << "class " << class_name << '\n';
  IND(s); s << "  : public " << "OmniProxyCallDesc" << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();

  // Constructor.
  IND(s); s << "inline " << class_name
	    << "(const char* _op, size_t _op_len, ";
  produce_decl_wr(s, I_TRUE, I_TRUE);
  s << " arg) :\n";
  IND(s); s << "  OmniProxyCallDesc(_op, _op_len),\n";
  IND(s); s << "  _value(arg)  {}\n\n";

  // Declaration of methods to implement the call.
  IND(s); s << "virtual void marshalArguments(cdrStream&);\n";

  s << "\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "private:\n";
  INC_INDENT_LEVEL();

  // Private data members - argument.
  IND(s); produce_decl_wr(s, I_TRUE, I_TRUE);
  s << " _value;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =
    o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wIN, mapping);

  IND(s); s << "void " << class_name
	    << "::marshalArguments(cdrStream& giop_client)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  o2be_operation::produceMarshalCode(s, field_type(),
				     o2be_global::root(), "giop_client",
				     "_value", ntype, mapping);
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


void
o2be_attribute::produce_proxy_rd_skel(std::fstream& s,
				      o2be_interface& defined_in)
{
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::read_descriptor_name(*this);

  IND(s); produce_decl_rd(s, I_TRUE);
  s << ' ' << defined_in.proxy_fqname() << "::" << uqname() << "()\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(\"_get_"
	    << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_get_") + 1)
	    << ");\n\n";
  IND(s); s << "OmniProxyCallWrapper::invoke(this, _call_desc);\n";
  IND(s); s << "return _call_desc.result();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


void 
o2be_attribute::produce_proxy_wr_skel(std::fstream& s,
				      o2be_interface& defined_in)
{
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::write_descriptor_name(*this);

  IND(s); s << "void " << defined_in.proxy_fqname() << "::"
	    << uqname() << '(';
  produce_decl_wr(s, I_TRUE, I_TRUE);
  s << " _value)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(\"_set_"
	    << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_set_") + 1)
	    << ", _value);\n\n";
  IND(s); s << "OmniProxyCallWrapper::invoke(this, _call_desc);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}

void
o2be_attribute::produce_server_skel_aux(std::fstream& s)
{
  o2be_upcall_desc::produce_descriptor(s, *this);

  {
    const char* call_class = o2be_upcall_desc::read_descriptor_name(*this);

    IND(s); s << "static void _0RL_" << _fqname() << "_get_UpCall "
	      << "(OmniUpCallDesc& desc, void* h) {\n";
    INC_INDENT_LEVEL();
    o2be_interface* intf = o2be_interface::narrow_from_scope(defined_in());
    IND(s); s << intf->server_fqname() << "* obj = ("
	      << intf->server_fqname() << "*) h;\n";
    IND(s); s << call_class << "& d = (" << call_class << "&)desc;\n";
    IND(s);
    s << "d.pd_result = ";
    s << "obj->" << uqname() << "();\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
  }

  if (readonly()) return;

  {
    const char* call_class = o2be_upcall_desc::write_descriptor_name(*this);
    IND(s); s << "static void _0RL_" << _fqname() << "_set_UpCall "
	      << "(OmniUpCallDesc& desc, void* h) {\n";
    INC_INDENT_LEVEL();
    o2be_interface* intf = o2be_interface::narrow_from_scope(defined_in());
    IND(s); s << intf->server_fqname() << "* obj = ("
	      << intf->server_fqname() << "*) h;\n";
    IND(s); s << call_class << "& d = (" << call_class << "&)desc;\n";
    IND(s);
    s << "obj->" << uqname() << "(d.arg_0);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
  }
}

void
o2be_attribute::produce_server_rd_call_desc(std::fstream& s, const char* class_name)
{
  const char* call_desc_base_class;
  call_desc_base_class = "OmniUpCallDesc";

  IND(s); s << "// Up-call descriptor class. Mangled signature:\n";
  IND(s); s << "//  " << mangled_read_signature() << '\n';
  IND(s); s << "class " << class_name << '\n';
  IND(s); s << "  : public " << call_desc_base_class << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();


  /////////// Constructor.
  IND(s); s << "inline " << class_name << "(OmniUpCallDesc::UpCallFn _upcallFn, void* _handle) : \n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_base_class 
	    << "(_upcallFn,_handle,0,0) {}\n\n";
  DEC_INDENT_LEVEL();

  // Declaration of methods to implement the call.
  IND(s); s << "void marshalReturnedValues(cdrStream&);\n";

  DEC_INDENT_LEVEL();

  // Data member
  INC_INDENT_LEVEL();
  IND(s);
  o2be_operation::declareUpcallVarType(s,field_type(),o2be_global::root(),
				       o2be_operation::wResult);
  s << " pd_result;\n";
  DEC_INDENT_LEVEL();

  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";



  // Method to marshal returned values to the stream.
  IND(s); s << "void " << class_name	
	    << "::marshalReturnedValues(cdrStream& giop_s) {\n";
  {
    INC_INDENT_LEVEL();
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype;
    ntype = o2be_operation::ast2ArgMapping(field_type(), 
					   o2be_operation::wResult, mapping);

    if ((ntype == o2be_operation::tObjref || 
	 ntype == o2be_operation::tString || 
	 ntype == o2be_operation::tTypeCode ||
	 mapping.is_pointer) && 
	!mapping.is_arrayslice) {
	  
      // These are declared as <type>_var variable
      if (ntype == o2be_operation::tString) {
	o2be_operation::produceMarshalCode(s,field_type(),
					   o2be_global::root(),"giop_s",
					   "pd_result",ntype,mapping);
      }
      else {
	// use operator->() to get to the pointer
	o2be_operation::produceMarshalCode(s,field_type(),o2be_global::root(),"giop_s",
			   "(pd_result.operator->())",ntype,mapping);
      }
    }
    else {
      o2be_operation::produceMarshalCode(s,field_type(),o2be_global::root(),
					 "giop_s","pd_result",ntype,mapping);
    }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}

void
o2be_attribute::produce_server_rd_skel(std::fstream& s,o2be_interface &defined_in)
{
  const char* call_desc_class = o2be_upcall_desc::read_descriptor_name(*this);
  IND(s); s << call_desc_class << " _call_desc(_0RL_"
	    << _fqname() << "_get_UpCall,(void*)this"
	    << ");\n\n";
  IND(s); s << "OmniUpCallWrapper::upcall(_giop_s,_call_desc);\n";
  IND(s); s << "return 1;\n";
}


void
o2be_attribute::produce_server_wr_call_desc(std::fstream& s, const char* class_name)
{
  const char* call_desc_base_class = "OmniUpCallDesc";

  IND(s); s << "// Up-call descriptor class. Mangled signature:\n";
  IND(s); s << "//  " << mangled_write_signature() << '\n';
  IND(s); s << "class " << class_name << '\n';
  IND(s); s << "  : public " << call_desc_base_class << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();


  /////////// Constructor.
  IND(s); s << "inline " << class_name << "(OmniUpCallDesc::UpCallFn _upcallFn, void* _handle) : \n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_base_class 
	    << "(_upcallFn,_handle,0,0) {}\n\n";
  DEC_INDENT_LEVEL();

  // Declaration of methods to implement the call.
  IND(s); s << "void unmarshalArguments(cdrStream&);\n";

  DEC_INDENT_LEVEL();

  // Data member
  INC_INDENT_LEVEL();
  IND(s);
  o2be_operation::declareUpcallVarType(s,field_type(),o2be_global::root(),
				       o2be_operation::wIN);
  s << " arg_0;\n";
  DEC_INDENT_LEVEL();

  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  // Method to unmarshal the arguments from the stream.
  IND(s); s << "void " << class_name	
	    << "::unmarshalArguments(cdrStream& giop_s) {\n";
  INC_INDENT_LEVEL();
  {
    // unmarshall arguments
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype;
    ntype = o2be_operation::ast2ArgMapping(field_type(),
					   o2be_operation::wIN,mapping);
    o2be_operation::produceUnMarshalCode(s,field_type(),
					 o2be_global::root(),
					 "giop_s","arg_0",
					 ntype,mapping);
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}

void
o2be_attribute::produce_server_wr_skel(std::fstream& s,o2be_interface &defined_in)
{
  const char* call_desc_class = o2be_upcall_desc::write_descriptor_name(*this);
  IND(s); s << call_desc_class << " _call_desc(_0RL_"
	    << _fqname() << "_set_UpCall,(void*)this";
  s << ");\n\n";
  IND(s); s << "OmniUpCallWrapper::upcall(_giop_s,_call_desc);\n";
  IND(s); s << "return 1;\n";
}

void
o2be_attribute::produce_nil_rd_skel(std::fstream& s)
{
  IND(s); produce_decl_rd(s);
  s << ' ' << uqname() << "() {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  s << "#ifdef NEED_DUMMY_RETURN\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	ntype == o2be_operation::tTypeCode ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer))
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),this,
				       0,mapping.is_arrayslice);
	s << 
       ((ntype != o2be_operation::tObjref && ntype != o2be_operation::tString
	 && ntype != o2be_operation::tTypeCode) ? " *" : "") 
	  << " _0RL_result" << " = "
	  << ((ntype == o2be_operation::tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	  << ";\n";
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),this);
	s << " _0RL_result";
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
	      s << o2be_name::narrow_and_produce_unambiguous_name(eval,this) 
		<< ";\n";
	    }
	    break;
	  case o2be_operation::tStructFixed:
	    s << ";\n";
	    s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	    break;
	  default:
	    s << ";\n";
	    break;
	  }
      }
  }
  IND(s); s << "return _0RL_result;\n";
  s << "#endif\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_nil_wr_skel(std::fstream& s)
{
  IND(s); s << "void " << uqname() << '(';
  produce_decl_wr(s);
  s << " _value) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_lcproxy_rd_skel(std::fstream& s,
					o2be_interface& defined_in)
{
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::read_descriptor_name(*this);

  IND(s); produce_decl_rd(s, I_TRUE);
  s << ' ' << defined_in.lcproxy_fqname() << "::" << uqname() << "()\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(\"_get_"
	    << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_get_") + 1)
	    << ");\n\n";

  IND(s); s << "if (!OmniLCProxyCallWrapper::invoke(this, _call_desc, "
	    << "_get_wrap_" << defined_in._fqname() << "())) {\n";
  INC_INDENT_LEVEL();
  IND(s);
  s << "return _get_wrap_" << defined_in._fqname() << "()->"
    << uqname() << "();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "return _call_desc.result();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


void 
o2be_attribute::produce_lcproxy_wr_skel(std::fstream& s,
					o2be_interface& defined_in)
{
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::write_descriptor_name(*this);

  IND(s); s << "void " << defined_in.lcproxy_fqname() << "::"
	    << uqname() << '(';
  produce_decl_wr(s, I_TRUE, I_TRUE);
  s << " _value)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(\"_set_"
	    << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_set_") + 1)
	    << ", _value);\n\n";
  IND(s); s << "if (!OmniLCProxyCallWrapper::invoke(this, _call_desc, "
	    << "_get_wrap_" << defined_in._fqname() << "())) {\n";
  INC_INDENT_LEVEL();
  IND(s);
  s << "_get_wrap_" << defined_in._fqname() << "()->"
    << uqname() << "(_value);\n";
  IND(s); s << "return;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


void
o2be_attribute::produce_dead_rd_skel(std::fstream& s)
{
  IND(s); produce_decl_rd(s, I_TRUE);
  s << ' ' << uqname() << "()\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);\n";
  s << "#ifdef NEED_DUMMY_RETURN\n";
  IND(s); s << "// never reach here! Dummy return to keep some compilers happy.\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	ntype == o2be_operation::tTypeCode ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer))
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),this,
				       0,mapping.is_arrayslice);

	s << 
	 ((ntype != o2be_operation::tObjref && ntype != o2be_operation::tString
	   && ntype != o2be_operation::tTypeCode)?" *":"") 
	  << " _0RL_result" << " = "
	  << ((ntype == o2be_operation::tTypeCode) ? "CORBA::TypeCode::_nil()" : "0")
	  << ";\n";
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s,field_type(),this);
	s << " _0RL_result";
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
	      s << o2be_name::narrow_and_produce_unambiguous_name(eval,this) 
		<< ";\n";
	    }
	    break;
	  case o2be_operation::tStructFixed:
	    s << ";\n";
	    s << "memset((void *)&_0RL_result,0,sizeof(_0RL_result));\n";
	    break;
	  default:
	    s << ";\n";
	    break;
	  }
      }
  }
  IND(s); s << "return _0RL_result;\n";
  s << "#endif\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_dead_wr_skel(std::fstream& s)
{
  IND(s); s << "void " << uqname() << '(';
  produce_decl_wr(s, I_TRUE);
  s << " _value) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_home_rd_skel(std::fstream& s,
				     o2be_interface &defined_in)
{
  IND(s); produce_decl_rd(s, I_TRUE);
  s << ' ' << uqname() << "() {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return _actual_" << defined_in._fqname() << "->"
	    << uqname() << "();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_home_wr_skel(std::fstream& s, o2be_interface &defined_in)
{
  IND(s); s << "void " << uqname() << '(';
  produce_decl_wr(s, I_TRUE);
  s << " _value) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_actual_" << defined_in._fqname() << "->"
	    << uqname() << "(_value);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_wrapproxy_rd_skel(std::fstream& s,
					  o2be_interface &defined_in)
{
  IND(s); produce_decl_rd(s, I_TRUE);
  s << ' ' << uqname() << "() {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return _actual_" << defined_in._fqname() << "->"
	    << uqname() << "();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_attribute::produce_wrapproxy_wr_skel(std::fstream& s,
					  o2be_interface &defined_in)
{
  IND(s); s << "void " << uqname() << '(';
  produce_decl_wr(s, I_TRUE);
  s << " _value) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_actual_" << defined_in._fqname() << "->"
	    << uqname() << "(_value);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}

void
o2be_attribute::produce_decls_at_global_scope_in_hdr(std::fstream& s)
{
  o2be_operation::check_and_produce_unnamed_argument_tc_decl(s,field_type());
}

void
o2be_attribute::produce_dynskel(std::fstream& s)
{
  o2be_operation::check_and_produce_unnamed_argument_tc_value(s,field_type());
}

const char*
o2be_attribute::mangled_read_signature()
{
  if( !pd_mangled_read_signature )
    pd_mangled_read_signature =
      o2be_name_mangler::produce_attribute_read_signature(*this);

  return pd_mangled_read_signature;
}


const char*
o2be_attribute::mangled_write_signature()
{
  if( !pd_mangled_write_signature )
    pd_mangled_write_signature =
      o2be_name_mangler::produce_attribute_write_signature(*this);

  return pd_mangled_write_signature;
}


IMPL_NARROW_METHODS1(o2be_attribute, AST_Attribute)
IMPL_NARROW_FROM_DECL(o2be_attribute)
IMPL_NARROW_FROM_SCOPE(o2be_attribute)
