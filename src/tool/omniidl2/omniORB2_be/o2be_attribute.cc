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
  Revision 1.23.6.1  1999/09/24 10:05:22  djr
  Updated for omniORB3.

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

#include <o2be_util.h>


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
o2be_attribute::produce_decl_rd(std::fstream& s, AST_Decl* used_in)
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
      ->unambiguous_objref_name(used_in);
    break;
  case o2be_operation::tString:
    s << "char*";
    break;
  case o2be_operation::tTypeCode:
    s << "CORBA::TypeCode_ptr";
    break;
  default:
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, used_in);
    break;
  }

  if( mapping.is_arrayslice )  s << "_slice";
  if( mapping.is_pointer    )  s << "*";
  if( mapping.is_reference  )  s << "&";
}


void
o2be_attribute::produce_decl_wr(std::fstream& s, AST_Decl* used_in,
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
      ->unambiguous_objref_name(used_in);
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
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, used_in);
    break;
  default:
    s << o2be_name::narrow_and_produce_unambiguous_name(decl, used_in);
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
  IND(s); s << "  : public " << "omniCallDescriptor" << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();

  // Constructor.
  IND(s); s << "inline " << class_name
	    << "(LocalCallFn lcfn, const char* op, size_t oplen) :\n";
  IND(s); s << "  omniCallDescriptor(lcfn, op, oplen)  {}\n\n";

  // Declaration of methods to implement the call.
  IND(s); s << "virtual void unmarshalReturnedValues(GIOP_C&);\n";

  // Result accessor.
  IND(s); s << "inline ";
  produce_decl_rd(s, o2be_global::root());
  s << " result() { return pd_result; }\n\n";

  // Data members - return value.
  IND(s); produce_decl_rd(s, o2be_global::root());
  s << " pd_result;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n\n";


  IND(s); s << "void " << class_name
	      << "::unmarshalReturnedValues(GIOP_C& giop_client)\n";
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
  IND(s); s << "}\n\n\n";
}


void
o2be_attribute::produce_write_proxy_call_desc(std::fstream& s,
					      const char* class_name)
{
  IND(s); s << "// Proxy call descriptor class. Mangled signature:\n";
  IND(s); s << "//  " << mangled_write_signature() << '\n';
  IND(s); s << "class " << class_name << '\n';
  IND(s); s << "  : public " << "omniCallDescriptor" << '\n';
  IND(s); s << "{\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();

  // Constructor.
  IND(s); s << "inline " << class_name
	    << "(LocalCallFn lcfn, const char* op, size_t oplen, ";
  produce_decl_wr(s, o2be_global::root(), I_TRUE);
  s << " arg) :\n";
  IND(s); s << "  omniCallDescriptor(lcfn, op, oplen),\n";
  IND(s); s << "  _value(arg)  {}\n\n";

  // Declaration of methods to implement the call.
  IND(s); s << "virtual CORBA::ULong alignedSize(CORBA::ULong);\n";
  IND(s); s << "virtual void marshalArguments(GIOP_C&);\n\n";

  // Private data members - argument.
  IND(s); produce_decl_wr(s, o2be_global::root(), I_TRUE);
  s << " _value;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n\n";


  IND(s); s << "CORBA::ULong " << class_name
	    << "::alignedSize(CORBA::ULong msgsize)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =
    o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wIN, mapping);

  o2be_operation::produceSizeCalculation(s, field_type(),
					 o2be_global::root(),
					 "giop_client", "msgsize",
					 "_value", ntype, mapping);
  IND(s); s << "return msgsize;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";


  IND(s); s << "void " << class_name
	    << "::marshalArguments(GIOP_C& giop_client)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  o2be_operation::produceMarshalCode(s, field_type(),
				     o2be_global::root(), "giop_client",
				     "_value", ntype, mapping);
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";
}


void
o2be_attribute::produce_proxy_rd_skel(std::fstream& s,
				      o2be_interface& def_in)
{
  // Generate call descriptor.
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::read_descriptor_name(*this);

  // Generate the local call call-back.
  char* lcfn = o2be_call_desc::generate_unique_name("_0RL_lcfn_");
  IND(s); s << "// Local call call-back function.\n";
  IND(s); s << "static void\n";
  IND(s); s << lcfn << "(omniCallDescriptor* cd, omniServant* svnt)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << "* tcd = (" << call_desc_class << "*) cd;\n";
  IND(s); s << def_in.server_fqname() << "* impl = ("
	    << def_in.server_fqname() << "*) svnt->_ptrToInterface("
	    << def_in.fqname() << "::_PD_repoId);\n";
  IND(s); s << "tcd->pd_result = impl->" << uqname() << "();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";

  // Generate the actual proxy method.
  IND(s); produce_decl_rd(s, o2be_global::root());
  s << ' ' << def_in.proxy_fqname() << "::" << uqname() << "()\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(" << lcfn
	    << ", \"_get_" << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_get_") + 1)
	    << ");\n\n";
  IND(s); s << "_invoke(_call_desc);\n";
  IND(s); s << "return _call_desc.result();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";
}


void 
o2be_attribute::produce_proxy_wr_skel(std::fstream& s,
				      o2be_interface& def_in)
{
  // Generate call descriptor.
  o2be_call_desc::produce_descriptor(s, *this);
  const char* call_desc_class = o2be_call_desc::write_descriptor_name(*this);

  // Generate the local call call-back.
  char* lcfn = o2be_call_desc::generate_unique_name("_0RL_lcfn_");
  IND(s); s << "// Local call call-back function.\n";
  IND(s); s << "static void\n";
  IND(s); s << lcfn << "(omniCallDescriptor* cd, omniServant* svnt)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << "* tcd = (" << call_desc_class << "*) cd;\n";
  IND(s); s << def_in.server_fqname() << "* impl = ("
	    << def_in.server_fqname() << "*) svnt->_ptrToInterface("
	    << def_in.fqname() << "::_PD_repoId);\n";
  IND(s); s << "impl->" << uqname() << "(tcd->_value);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";

  // Generate the actual proxy method.
  IND(s); s << "void " << def_in.proxy_fqname() << "::"
	    << uqname() << '(';
  produce_decl_wr(s, o2be_interface::narrow_from_scope(defined_in()), I_TRUE);
  s << " _value)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << call_desc_class << " _call_desc(" << lcfn
	    << ", \"_set_" << local_name()->get_string() << "\", "
	    << (strlen(local_name()->get_string()) + strlen("_set_") + 1)
	    << ", _value);\n\n";
  IND(s); s << "_invoke(_call_desc);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";
}


void
o2be_attribute::produce_server_rd_skel(std::fstream& s, AST_Decl* used_in)
{
  IND(s); s << "giop_s.RequestReceived();\n";

  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	ntype == o2be_operation::tTypeCode ||
	(mapping.is_arrayslice) ||
	(mapping.is_pointer)) 
      {
	// declare a <type>_var variable to manage the pointer type
	IND(s);
	o2be_operation::declareVarType(s, field_type(), used_in, 1,
				       mapping.is_arrayslice);
      }
    else 
      {
	IND(s);
	o2be_operation::declareVarType(s, field_type(), used_in);
      }
    s << " result = this->" << uqname() << "();\n";
  }

  IND(s); s << "if( giop_s.response_expected() ) {\n";
  INC_INDENT_LEVEL();

  // calculate reply message size
  IND(s); s << "size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wResult,mapping);
    if ((ntype == o2be_operation::tObjref ||
	 ntype == o2be_operation::tString ||
	 ntype == o2be_operation::tTypeCode ||
	 mapping.is_pointer) && !mapping.is_arrayslice)
      {
	// These are declared as <type>_var variable 
	if (ntype == o2be_operation::tString) {
	  o2be_operation::produceSizeCalculation(s, field_type(),
						 used_in,
						 "giop_s", "msgsize",
						 "result", ntype,mapping);
	}
	else {
	  // use operator->() to get to the pointer
	  o2be_operation::produceSizeCalculation(s, field_type(),
						 used_in,
						 "giop_s", "msgsize",
						 "(result.operator->())",
						 ntype, mapping);
	}
      }
    else
      {
	o2be_operation::produceSizeCalculation(s, field_type(),
					       used_in,
					       "giop_s", "msgsize",
					       "result", ntype, mapping);
      }
  }

  IND(s); s << "giop_s.InitialiseReply(GIOP::NO_EXCEPTION, (CORBA::ULong) "
	    "msgsize);\n";

  // marshall results
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype =
      o2be_operation::ast2ArgMapping(field_type(), o2be_operation::wResult,
				     mapping);
    if ((ntype == o2be_operation::tObjref || 
	 ntype == o2be_operation::tString ||
	 ntype == o2be_operation::tTypeCode ||
	 mapping.is_pointer)
	&& !mapping.is_arrayslice)
      {
	// These are declared as <type>_var variable 
	if (ntype == o2be_operation::tString) {
	  o2be_operation::produceMarshalCode(s, field_type(),
					     used_in,
					     "giop_s", "result",
					     ntype, mapping);
	}
	else {
	  // use operator->() to get to the pointer
	  o2be_operation::produceMarshalCode(s, field_type(),
					     used_in,
					     "giop_s",
					     "(result.operator->())",
					     ntype, mapping);
	}
      }
    else
      {
	o2be_operation::produceMarshalCode(s, field_type(),
					   used_in,
					   "giop_s", "result",
					   ntype, mapping);
      }
  }

  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "giop_s.ReplyCompleted();\n";
  IND(s); s << "return 1;\n";
}


void
o2be_attribute::produce_server_wr_skel(std::fstream& s, AST_Decl* used_in)
{
  {
    // unmarshall arguments
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(field_type(),o2be_operation::wIN,mapping);
    if (ntype == o2be_operation::tObjref || 
	ntype == o2be_operation::tString ||
	ntype == o2be_operation::tTypeCode) 
      {
      // declare a <type>_var variable to manage the pointer type
	IND(s);
	o2be_operation::declareVarType(s, field_type(), used_in, 1);
      }
    else
      {
	IND(s);
	o2be_operation::declareVarType(s, field_type(), used_in);
      }
    s << " " << "value;\n";
    o2be_operation::produceUnMarshalCode(s, field_type(),
					 used_in,
					 "giop_s", "value",
					 ntype, mapping);
  }

  IND(s); s << "giop_s.RequestReceived();\n";
  IND(s); s << "this->" << uqname() << "(value);\n";

  s << o2be_verbatim(
   "if( giop_s.response_expected() ) {\n"
   "  size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();\n"
   "  giop_s.InitialiseReply(GIOP::NO_EXCEPTION, (CORBA::ULong) msgsize);\n"
   "}\n"
   "giop_s.ReplyCompleted();\n"
   "return 1;\n"
  );
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
