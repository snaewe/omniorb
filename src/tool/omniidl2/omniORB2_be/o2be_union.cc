// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_union.cc            Created on: 12/08/1996
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
//   OMNI BE for the class AST_Union
//

/*
  $Log$
  Revision 1.8  1997/12/18 17:28:54  sll
  *** empty log message ***

  Revision 1.7  1997/12/09 19:55:11  sll
  *** empty log message ***

  Revision 1.6  1997/08/21 21:13:57  sll
  Names of internal variables inside the stub code now all start with the
  prefix __ to avoid potential clash with identifiers defined in IDL.

// Revision 1.5  1997/05/06  14:10:02  sll
// Public release.
//
  */

/*
  Example:

  // IDL
  typedef octet Bytes[64];
  Struct S { long len; };
  interface A;
  union U switch (long) {
      case 1: long x;
      case 2: Bytes y;
      case 3: string z;
      case 4:
      case 5: S w;
      default: A obj;
  };
  
  // C++
  typedef CORBA::Octet Bytes[64];
  typedef CORBA::Octet Bytes_slice;
  class Bytes_forany { ... };
  struct S { CORBA::Long len; };
  typedef ... A_ptr;

  class U {
  public:
    U();
    U(const U&);
    ~U();
    U &operator= (const U&);
    
    void _d(CORBA::Long);
    Long _d() const;

    void x(CORBA::Long);
    CORBA::Long x() const;

    void y(Bytes);
    Bytes_slice *y() const;
    
    void z(char *);
    void z(const char *);
    void z(const CORBA::String_var &);
    const char *z() const;

    void w(const S &);
    const S &w() const;
    S &w();

    void obj(A_ptr); 	 // release old objref, duplicate
    A_ptr obj() const;   // no duplicate
  };


  class U_var {
  public:
    U_var();
    U_var(U *);
    U_var(const U_var &);
    ~U_var();
    U_var & operator= (U *);
    U_var & operator= (const U_var &);
    U *operator->();
    operator U *&();
    operator U &();
    operator const U *() const;
  };
*/

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#define ADPT_CLASS_TEMPLATE  "_CORBA_ConstrType_Variable_OUT_arg"

static void
produce_disc_value(fstream &s,AST_ConcreteType *t,AST_Expression *exp,
		   AST_Decl* used_in);

static void
produce_default_value(o2be_union &u,fstream& s);

typedef union {
  int i_val;
  unsigned int ui_val;
  short s_val;
  unsigned short us_val;
  char c_val;
  idl_bool b_val;
  AST_Decl* e_val;
} disc_value_t;

static idl_bool
match_disc_value(o2be_union_branch& b,AST_Decl *disc_type,disc_value_t v);

static o2be_union_branch*
lookup_by_disc_value(o2be_union& u,disc_value_t v);

o2be_union::o2be_union(AST_ConcreteType *dt,
		       UTL_ScopedName *n, UTL_StrList *p)
	: AST_Union(dt, n, p),
	  AST_Decl(AST_Decl::NT_union, n, p),
          AST_Structure(AST_Decl::NT_union, n, p),
	  UTL_Scope(AST_Decl::NT_union),
	  o2be_name(AST_Decl::NT_union, n, p),
	  o2be_sequence_chain(AST_Decl::NT_union, n, p)
{
  pd_isvar = I_FALSE;
  pd_nodefault = I_TRUE;
  pd_hdr_produced_in_field = I_FALSE;
  pd_skel_produced_in_field = I_FALSE;

  pd_out_adptarg_name = new char[strlen(ADPT_CLASS_TEMPLATE)+strlen("<,>")+
				 strlen(fqname())+
				 strlen(fqname())+strlen("_var")+1];
  strcpy(pd_out_adptarg_name,ADPT_CLASS_TEMPLATE);
  strcat(pd_out_adptarg_name,"<");
  strcat(pd_out_adptarg_name,fqname());
  strcat(pd_out_adptarg_name,",");
  strcat(pd_out_adptarg_name,fqname());
  strcat(pd_out_adptarg_name,"_var>");  
}

AST_UnionBranch *
o2be_union::add_union_branch(AST_UnionBranch *un)
{
  // Check that the CFE operation succeeds. If it returns 0,
  // stop any further work.
  if (AST_Union::add_union_branch(un) == 0)
    return 0;

  if ((un->label())->label_kind() == AST_UnionLabel::UL_default)
    pd_nodefault = I_FALSE;

  // Now check if the field is of variable size.
  if (isVariable())
    return un;

  AST_Decl *decl = un->field_type();
  while (decl->node_type() == AST_Decl::NT_typedef)
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

  switch (decl->node_type())
    {
    case AST_Decl::NT_struct:
      pd_isvar = o2be_structure::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_union:
      pd_isvar = o2be_union::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_string:
    case AST_Decl::NT_sequence:
    case AST_Decl::NT_interface:
      pd_isvar = I_TRUE;
      break;
    case AST_Decl::NT_pre_defined:
      if (o2be_predefined_type::narrow_from_decl(decl)->pt()
	  == AST_PredefinedType::PT_any) 
	{
	  pd_isvar = I_TRUE;
	}
      break;
    case AST_Decl::NT_array:
      pd_isvar = o2be_array::narrow_from_decl(decl)->isVariable();
      break;
    default:
      break;
    }
  return un;
}

void
o2be_union::produce_hdr(fstream &s)
{
  if (!nodefault())
    {
      if (no_missing_disc_value())
	{
	  // Wrong, all legal discriminant value has been specified
	  // there should not be a default case.
	  UTL_String msg("default case defined where none is needed.");
	  idl_global->err()->back_end(line(),&msg);
	  return;
	}
    }


  idl_bool has_fix_member = I_FALSE;

  IND(s); s << "class " << uqname() << " {\n";
  IND(s); s << "public:\n\n";
  INC_INDENT_LEVEL();
  {
    // declare any constructor types defined in this scope
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_Decl *decl=AST_UnionBranch::narrow_from_decl(d)->field_type();
	    if (decl->has_ancestor(this))
	      {
		switch (decl->node_type())
		  {
		  case AST_Decl::NT_enum:
		    if (!o2be_enum::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_enum::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_enum::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  case AST_Decl::NT_struct:
		    if (!o2be_structure::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_structure::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_structure::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  case AST_Decl::NT_union:
		    if (!o2be_union::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_union::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_union::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  default:
		    break;
		  }
	      }
	  }
	i.next();
      }
  }

  IND(s); s << uqname() << "() {\n";
  INC_INDENT_LEVEL();
  if (nodefault())
    {
      if (!no_missing_disc_value()) {
	IND(s); s << "_default();\n";
      }
    }
  else
    {
      IND(s); s << "pd__default = 1;\n";
      IND(s); s << "pd__d = ";
      produce_default_value(*this,s);
      s << ";\n";
    }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << uqname() << "(const " << uqname() << "& _value) {\n";
  INC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "if ((pd__default = _value.pd__default)) {\n";
      INC_INDENT_LEVEL();
      {
	IND(s); s << "pd__d = _value.pd__d;\n";
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_union_branch)
	      {
		AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
		if (l->label_kind() == AST_UnionLabel::UL_default)
		  {
		    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		    IND(s); s << f->uqname() << "(_value.pd_"
			      << f->uqname() << ");\n";
		    break;
		  }
	      }
	    i.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "else {\n";
    }
  INC_INDENT_LEVEL();
  IND(s); s << "switch(_value.pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ": "
		  << f->uqname() << "(_value.pd_"
		  << f->uqname() << "); break;\n";
	      }
	  }
	i.next();
      }
  }
  IND(s); s << "default: break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value())) {
    IND(s); s << "}\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "~" << uqname() << "() {}\n\n";

  IND(s); s << uqname() << "& operator=(const " << uqname() << "& _value) {\n";
  INC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "if ((pd__default = _value.pd__default)) {\n";
      INC_INDENT_LEVEL();
      {
	IND(s); s << "pd__d = _value.pd__d;\n";
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_union_branch)
	      {
		AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
		if (l->label_kind() == AST_UnionLabel::UL_default)
		  {
		    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		    IND(s); s << f->uqname() << "(_value.pd_"
			      << f->uqname() << ");\n";
		    break;
		  }
	      }
	    i.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "else {\n";
    }
  INC_INDENT_LEVEL();
  IND(s); s << "switch(_value.pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ": "
		  << f->uqname() << "(_value.pd_"
		  << f->uqname() << "); break;\n";
	      }
	  }
	i.next();
      }
  }
  IND(s); s << "default: break;\n";
  DEC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value())) {
    IND(s); s << "}\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "return *this;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
  
  IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(disc_type(),this) 
	    << " _d () const { return pd__d;}\n";
  IND(s); s << "void _d("
	    << o2be_name::narrow_and_produce_unambiguous_name(disc_type(),this)
	    << " _value) {}\n\n";

  if (nodefault() && !no_missing_disc_value())
    {
      // No default case declared.
      IND(s); s << "void _default()\n";
      IND(s); s << "{\n";
      INC_INDENT_LEVEL();
      IND(s); s << "pd__d = ";
      produce_default_value(*this,s);
      s << ";\n";
      IND(s); s << "pd__default = 1;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
    }

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype;

	    ntype =  o2be_operation::ast2ArgMapping(f->field_type(),
					   o2be_operation::wResult,mapping);

	    if (ntype != o2be_operation::tString &&
		ntype != o2be_operation::tObjref &&
		ntype != o2be_operation::tStructVariable &&
		ntype != o2be_operation::tUnionVariable &&
		ntype != o2be_operation::tUnionFixed &&
		ntype != o2be_operation::tSequence &&
		ntype != o2be_operation::tArrayVariable &&
		ntype != o2be_operation::tAny)
	      {
		has_fix_member = I_TRUE;
	      }

	    switch (ntype)
	      {
	      case o2be_operation::tString:
		IND(s); s << "const char * " << f->uqname() << " () const { return (const char*) pd_" << f->uqname() << "; }\n";
		break;
	      case o2be_operation::tObjref:
		{
		  AST_Decl *decl = f->field_type();
		  while (decl->node_type() == AST_Decl::NT_typedef) {
		    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		  }
		  IND(s); s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this)
			    << " " << f->uqname() << " () const { return "
			    << "pd_" << f->uqname() << "._ptr; }\n";
		  break;
		}
	      case o2be_operation::tSequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
		{
		  if (f->field_type()->node_type() == AST_Decl::NT_sequence)
		    {
		      IND(s); s << "const "
				<< o2be_sequence::narrow_from_decl(f->field_type())->seq_template_name(this)
				<< " &"
				<< f->uqname() << " () const { return pd_"
				<< f->uqname() << "; }\n";
		      IND(s); s << o2be_sequence::narrow_from_decl(f->field_type())->seq_template_name(this)
				<< " &"
				<< f->uqname() << " () { return pd_"
				<< f->uqname() << "; }\n";
		    }
		  else
		    {
		      IND(s); s << "const "
				<< o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
				<< " &"
				<< f->uqname() << " () const { return pd_"
				<< f->uqname() << "; }\n";
		      IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
				<< " &"
				<< f->uqname() << " () { return pd_"
				<< f->uqname() << "; }\n";
		    }
		  break;
		}
#endif
	      case o2be_operation::tStructFixed:
	      case o2be_operation::tStructVariable:
	      case o2be_operation::tUnionFixed:
	      case o2be_operation::tUnionVariable:
	      case o2be_operation::tAny:
		IND(s); s << "const "
			  << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " &"
			  << f->uqname() << " () const { return pd_"
			  << f->uqname() << "; }\n";
		IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " &"
			  << f->uqname() << " () { return pd_"
			  << f->uqname() << "; }\n";
		break;
	      case o2be_operation::tArrayFixed:
	      case o2be_operation::tArrayVariable:
		{
		  // Check if this is an anonymous array type, if so
		  // generate the supporting typedef for the array slice
		  AST_Decl *decl = f->field_type();
		  if (decl->node_type() == AST_Decl::NT_array &&
		      decl->has_ancestor(this)) 
		    {
		      char * tmpname = new char [strlen(f->uqname()) + 2];
		      strcpy(tmpname,"_");
		      strcat(tmpname,f->uqname());
		      o2be_array::narrow_from_decl(decl)->produce_typedef_in_union(s,tmpname,this);
		      IND(s); s << "const " << tmpname << "_slice* " 
				<< f->uqname() << "() const { return pd_"
				<< f->uqname() << "; }\n";
		      delete [] tmpname;
		    }
		  else
		    {
		      IND(s); s << "const "
				<< o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
				<< ((mapping.is_arrayslice) ? "_slice":"")
				<< " "
				<< ((mapping.is_pointer)    ? "*":"")
				<< ((mapping.is_reference)  ? "&":"")
				<< f->uqname() << " () const { return pd_"
				<< f->uqname() << "; }\n";
		    }
		  break;
		}
	      default: 
		IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " "
			  << ((mapping.is_pointer)    ? "*":"")
			  << ((mapping.is_reference)  ? "&":"")
			  << f->uqname() << " () const { return pd_"
			  << f->uqname() << "; }\n";
		break;
	      }

	    ntype =  o2be_operation::ast2ArgMapping(f->field_type(),
						  o2be_operation::wIN,mapping);


	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();

	    switch (ntype)
	      {
	      case o2be_operation::tString:
		IND(s); s << "void "
			  << f->uqname() << "(char* _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		IND(s); s << "void "
			  << f->uqname() << "(const char*  _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		IND(s); s << "void "
			  << f->uqname() << "(const CORBA::String_var& _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		IND(s); s << "void "
			  << f->uqname() << "(const "
			  << o2be_string::fieldMemberTypeName() 
			  << "& _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		break;
	      case o2be_operation::tObjref:
		{
		  AST_Decl *decl = f->field_type();
		  while (decl->node_type() == AST_Decl::NT_typedef) {
		    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
		  }
		  IND(s); s << "void " << f->uqname() << "(" 
			    << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this)
			    << " _value) {\n";
		  INC_INDENT_LEVEL();
		  if (l->label_kind() == AST_UnionLabel::UL_label)
		    {
		      IND(s); s << "pd__d = ";
		      produce_disc_value(s,disc_type(),l->label_val(),this);
		      s << ";\n";
		      IND(s); s << "pd__default = 0;\n";
		    }
		  else
		    {
		      IND(s); s << "pd__d = ";
		      produce_default_value(*this,s);
		      s << ";\n";
		      IND(s); s << "pd__default = 1;\n";
		    }
		  IND(s); s << "pd_" << f->uqname() << " = "
			    << o2be_interface::narrow_from_decl(decl)->unambiguous_name(this)
			    << "::_duplicate(_value);\n";
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  IND(s); s << "void " << f->uqname() << "(const " 
			    << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(this)
			    << "& _value) {\n";
		  INC_INDENT_LEVEL();
		  if (l->label_kind() == AST_UnionLabel::UL_label)
		    {
		      IND(s); s << "pd__d = ";
		      produce_disc_value(s,disc_type(),l->label_val(),this);
		      s << ";\n";
		      IND(s); s << "pd__default = 0;\n";
		    }
		  else
		    {
		      IND(s); s << "pd__d = ";
		      produce_default_value(*this,s);
		      s << ";\n";
		      IND(s); s << "pd__default = 1;\n";
		    }
		  IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  IND(s); s << "void " << f->uqname() << "(const " 
			    << o2be_interface::narrow_from_decl(decl)->unambiguous_name(this)
			    << "_var&  _value) {\n";
		  INC_INDENT_LEVEL();
		  if (l->label_kind() == AST_UnionLabel::UL_label)
		    {
		      IND(s); s << "pd__d = ";
		      produce_disc_value(s,disc_type(),l->label_val(),this);
		      s << ";\n";
		      IND(s); s << "pd__default = 0;\n";
		    }
		  else
		    {
		      IND(s); s << "pd__d = ";
		      produce_default_value(*this,s);
		      s << ";\n";
		      IND(s); s << "pd__default = 1;\n";
		    }
		  IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  break;
		}
	      case o2be_operation::tSequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
		if (f->field_type()->node_type() == AST_Decl::NT_sequence)
		  {
		    IND(s); s << "void "
			      << f->uqname() << " (const "
			      << o2be_sequence::narrow_from_decl(f->field_type())->seq_template_name(this)
			      << "& _value) {\n";
		    INC_INDENT_LEVEL();
		    if (l->label_kind() == AST_UnionLabel::UL_label)
		      {
			IND(s); s << "pd__d = ";
			produce_disc_value(s,disc_type(),l->label_val(),this);
			s << ";\n";
			IND(s); s << "pd__default = 0;\n";
		      }
		    else
		      {
			IND(s); s << "pd__d = ";
			produce_default_value(*this,s);
			s << ";\n";
			IND(s); s << "pd__default = 1;\n";
		      }
		    IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		    DEC_INDENT_LEVEL();
		    IND(s); s << "}\n";
		  }
		else
		  {
		    IND(s); s << "void "
			      << f->uqname() << " (const "
			      << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			      << "& _value) {\n";
		    INC_INDENT_LEVEL();
		    if (l->label_kind() == AST_UnionLabel::UL_label)
		      {
			IND(s); s << "pd__d = ";
			produce_disc_value(s,disc_type(),l->label_val(),this);
			s << ";\n";
			IND(s); s << "pd__default = 0;\n";
		      }
		    else
		      {
			IND(s); s << "pd__d = ";
			produce_default_value(*this,s);
			s << ";\n";
			IND(s); s << "pd__default = 1;\n";
		      }
		    IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		    DEC_INDENT_LEVEL();
		    IND(s); s << "}\n";
		  }
		break;
#endif
	      case o2be_operation::tStructFixed:
	      case o2be_operation::tStructVariable:
	      case o2be_operation::tUnionFixed:
	      case o2be_operation::tUnionVariable:
	      case o2be_operation::tAny:
		IND(s); s << "void "
			  << f->uqname() << " (const "
			  << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << "& _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		break;
	      case o2be_operation::tArrayFixed:
	      case o2be_operation::tArrayVariable:
		{
		  // Check if this is an anonymous array type, if so
		  // generate the supporting typedef for the array slice
		  AST_Decl *decl = f->field_type();
		  if (decl->node_type() == AST_Decl::NT_array &&
		      decl->has_ancestor(this)) 
		    {
		      IND(s); s << "void "
				<< f->uqname() << " (const "
				<< "_" << f->uqname() << " _value) {\n";
		    }
		  else
		    {
		      IND(s); s << "void "
				<< f->uqname() << " (const "
				<< o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
				<< " _value) {\n";
		    }
		  INC_INDENT_LEVEL();
		  if (l->label_kind() == AST_UnionLabel::UL_label)
		    {
		      IND(s); s << "pd__d = ";
		      produce_disc_value(s,disc_type(),l->label_val(),this);
		      s << ";\n";
		      IND(s); s << "pd__default = 0;\n";
		    }
		  else
		    {
		      IND(s); s << "pd__d = ";
		      produce_default_value(*this,s);
		      s << ";\n";
		      IND(s); s << "pd__default = 1;\n";
		    }
		  {
		    unsigned int ndim = 0;
		    unsigned int dimval;

		    while (decl->node_type() == AST_Decl::NT_typedef)
		      decl = o2be_typedef::narrow_from_decl(decl)->base_type();

		    o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
		    while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		      {
			dimval = next();
			IND(s); s << "for (unsigned int _i" << ndim << " =0;"
				  << "_i" << ndim << " < " << dimval << ";"
				  << "_i" << ndim << "++) {\n";
			INC_INDENT_LEVEL();
			ndim++;
		      }
		    IND(s); s << "pd_" << f->uqname();
		    ndim = 0;
		    while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		      {
			s << "[_i" << ndim << "]";
			ndim++;
		      }
		    s << " = _value";
		    ndim = 0;
		    while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		      {
			s << "[_i" << ndim << "]";
			ndim++;
		      }
		    s << ";\n";
		    ndim = 0;
		    while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		      {
			DEC_INDENT_LEVEL();
			IND(s); s << "}\n";
			ndim++;
		      }
		  }
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  break;
		}
	      default:
		IND(s); s << "void " 
			  << f->uqname() << " ("
			  << ((mapping.is_const) ? "const ":"")
			  << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " "
			  << ((mapping.is_pointer)    ? "*":"")
			  << ((mapping.is_reference)  ? "&":"")
			  << " _value) {\n";
		INC_INDENT_LEVEL();
		if (l->label_kind() == AST_UnionLabel::UL_label)
		  {
		    IND(s); s << "pd__d = ";
		    produce_disc_value(s,disc_type(),l->label_val(),this);
		    s << ";\n";
		    IND(s); s << "pd__default = 0;\n";
		  }
		else
		  {
		    IND(s); s << "pd__d = ";
		    produce_default_value(*this,s);
		    s << ";\n";
		    IND(s); s << "pd__default = 1;\n";
		  }
		IND(s); s << "pd_" << f->uqname() << " = _value;\n";
		DEC_INDENT_LEVEL();
		IND(s); s << "}\n";
		break;
	      }

	  }
	i.next();
      }
  }


  IND(s); s << "\n";
  IND(s); s << "size_t NP_alignedSize(size_t initialoffset) const;\n";
  IND(s); s << "void operator>>= (NetBufferedStream &) const;\n";
  IND(s); s << "void operator<<= (NetBufferedStream &);\n";
  IND(s); s << "void operator>>= (MemBufferedStream &) const;\n";
  IND(s); s << "void operator<<= (MemBufferedStream &);\n\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "private:\n\n";
  INC_INDENT_LEVEL();

  IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(disc_type(),this)
	    << " pd__d;\n";
  IND(s); s << "CORBA::Boolean pd__default;\n";

  if (has_fix_member) {
    IND(s); s << "union {\n";
    INC_INDENT_LEVEL();
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype;

	    ntype =  o2be_operation::ast2ArgMapping(f->field_type(),
				       o2be_operation::wResult,mapping);
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
	      case o2be_operation::tEnum:
	      case o2be_operation::tStructFixed:
		IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " pd_" << f->uqname() << ";\n";
		break;
	      case o2be_operation::tArrayFixed:
		if (f->field_type()->node_type() == AST_Decl::NT_array)
		  {
		    IND(s);
		    o2be_array::narrow_from_decl(f->field_type())->produce_union_member_decl(s,f,this);
		  }
		else
		  {
		    IND(s); s << o2be_typedef::narrow_from_decl(f->field_type())->unambiguous_name(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
		break;
	      default:
		break;
	      }
	  }
	i.next();
      }
    DEC_INDENT_LEVEL();
    IND(s); s << "};\n";
  }
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype;

	    ntype =  o2be_operation::ast2ArgMapping(f->field_type(),
				       o2be_operation::wResult,mapping);
	    switch (ntype)
	      {
	      case o2be_operation::tString:
		IND(s); s << o2be_string::fieldMemberTypeName()
			  << " pd_" << f->uqname() << ";\n";
		break;
	      case o2be_operation::tObjref:
		if (f->field_type()->node_type() == AST_Decl::NT_interface)
		  {
		    IND(s); s << o2be_interface::narrow_from_decl(f->field_type())->fieldMemberType_fqname(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
		else
		  {
		    IND(s); s << o2be_typedef::narrow_from_decl(f->field_type())->fieldMemberType_fqname(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
		break;
	      case o2be_operation::tStructVariable:
	      case o2be_operation::tUnionFixed:
	      case o2be_operation::tUnionVariable:
	      case o2be_operation::tAny:
		IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " pd_" << f->uqname() << ";\n";
		break;
	      case o2be_operation::tSequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
		if (f->field_type()->node_type() == AST_Decl::NT_sequence) 
		  {
		    IND(s); s << o2be_sequence::narrow_from_decl(f->field_type())->seq_template_name(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
		else
		  {
		    IND(s); s << o2be_typedef::narrow_from_decl(f->field_type())->unambiguous_name(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
#else
		IND(s); s << o2be_name::narrow_and_produce_unambiguous_name(f->field_type(),this)
			  << " pd_" << f->uqname() << ";\n";
#endif
		break;
	      case o2be_operation::tArrayVariable:
		if (f->field_type()->node_type() == AST_Decl::NT_array)
		  {
		    IND(s);
		    o2be_array::narrow_from_decl(f->field_type())->produce_union_member_decl(s,f,this);
		  }
		else
		  {
		    IND(s); s << o2be_typedef::narrow_from_decl(f->field_type())->unambiguous_name(this)
			      << " pd_" << f->uqname() << ";\n";
		  }
		break;
	      default:
		break;
	      }
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  IND(s); s << "typedef _CORBA_ConstrType_"
	    << ((isVariable())?"Variable":"Fix")
	    << "_Var<" << uqname() << "> " 
	      << uqname() << "_var;\n\n";

  produce_seq_hdr_if_defined(s);
}

void
o2be_union::produce_skel(fstream &s)
{
  s << "\n";
  {
    // declare any constructor types defined in this scope
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_Decl *decl=AST_UnionBranch::narrow_from_decl(d)->field_type();
	    if (decl->has_ancestor(this))
	      {
		switch (decl->node_type())
		  {
		  case AST_Decl::NT_enum:
		    if (!o2be_enum::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_enum::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_enum::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  case AST_Decl::NT_struct:
		    if (!o2be_structure::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_structure::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_structure::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  case AST_Decl::NT_union:
		    if (!o2be_union::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_union::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_union::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  default:
		    break;
		  }
	      }
	  }
	i.next();
      }
  }
  
  s << "\n";

  IND(s); s << "size_t\n";
  IND(s); s << fqname() << "::NP_alignedSize(size_t initialoffset) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong _msgsize = initialoffset;\n";
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     disc_type(),
		     o2be_operation::wIN,mapping);
    o2be_operation::produceSizeCalculation(
		     s,
		     disc_type(),
		     ScopeAsDecl(defined_in()),
		     "",
		     "_msgsize",
		     "pd__d",
		     ntype,
		     mapping);
  }

  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "if (pd__default) {\n";
      INC_INDENT_LEVEL();
      {
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_union_branch)
	      {
		AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
		if (l->label_kind() == AST_UnionLabel::UL_default)
		  {
		    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		    o2be_operation::argMapping mapping;
		    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
										   f->field_type(),
										   o2be_operation::wIN,mapping);
		    if (ntype == o2be_operation::tString) {
		      ntype = o2be_operation::tStringMember;
		      mapping.is_pointer = I_FALSE;
		    }
		    else if (ntype == o2be_operation::tObjref) {
		      ntype = o2be_operation::tObjrefMember;
		      mapping.is_pointer = I_FALSE;
		    }

		    char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		    strcpy(tmpname,"pd_");
		    strcat(tmpname,f->uqname());
		    o2be_operation::produceSizeCalculation(
							   s,
							   f->field_type(),
							   ScopeAsDecl(defined_in()),
							   "",
							   "_msgsize",
							   tmpname,
							   ntype,
							   mapping);
		    delete [] tmpname;
		    break;
		  }
	      }
	    i.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "else {\n";
    }
  INC_INDENT_LEVEL();
  IND(s); s << "switch(pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ":\n";
		INC_INDENT_LEVEL();
		o2be_operation::argMapping mapping;
		o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     f->field_type(),
		     o2be_operation::wIN,mapping);
		if (ntype == o2be_operation::tString) {
		  ntype = o2be_operation::tStringMember;
		  mapping.is_pointer = I_FALSE;
		}
		else if (ntype == o2be_operation::tObjref) {
		  ntype = o2be_operation::tObjrefMember;
		  mapping.is_pointer = I_FALSE;
		}

		char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		strcpy(tmpname,"pd_");
		strcat(tmpname,f->uqname());
		o2be_operation::produceSizeCalculation(
		     s,
		     f->field_type(),
		     ScopeAsDecl(defined_in()),
		     "",
		     "_msgsize",
		     tmpname,
		     ntype,
		     mapping);
		delete [] tmpname;
		IND(s); s << "break;\n";
		DEC_INDENT_LEVEL();
	      }
	  }
	i.next();
      }
  }
  IND(s); s << "default: break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "}\n";
    }
  IND(s); s << "return _msgsize;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (NetBufferedStream &_n) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     disc_type(),
		     o2be_operation::wIN,mapping);
    o2be_operation::produceMarshalCode(
		     s,
		     disc_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     "pd__d",
		     ntype,
		     mapping);
  }
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "if (pd__default) {\n";
      INC_INDENT_LEVEL();
      {
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_union_branch)
	      {
		AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
		if (l->label_kind() == AST_UnionLabel::UL_default)
		  {
		    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		    o2be_operation::argMapping mapping;
		    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
										   f->field_type(),
										   o2be_operation::wIN,mapping);
		    if (ntype == o2be_operation::tString) {
		      ntype = o2be_operation::tStringMember;
		      mapping.is_pointer = I_FALSE;
		    }
		    else if (ntype == o2be_operation::tObjref) {
		      ntype = o2be_operation::tObjrefMember;
		      mapping.is_pointer = I_FALSE;
		    }

		    char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		    strcpy(tmpname,"pd_");
		    strcat(tmpname,f->uqname());
		    o2be_operation::produceMarshalCode(
						       s,
						       f->field_type(),
						       ScopeAsDecl(defined_in()),
						       "_n",
						       tmpname,
						       ntype,
						       mapping);
		    delete [] tmpname;
		    break;
		  }
	      }
	    i.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "else {\n";
    }
  INC_INDENT_LEVEL();
  IND(s); s << "switch(pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ":\n";
		INC_INDENT_LEVEL();
		o2be_operation::argMapping mapping;
		o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     f->field_type(),
		     o2be_operation::wIN,mapping);
		if (ntype == o2be_operation::tString) {
		  ntype = o2be_operation::tStringMember;
		  mapping.is_pointer = I_FALSE;
		}
		else if (ntype == o2be_operation::tObjref) {
		  ntype = o2be_operation::tObjrefMember;
		  mapping.is_pointer = I_FALSE;
		}

		char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		strcpy(tmpname,"pd_");
		strcat(tmpname,f->uqname());
		o2be_operation::produceMarshalCode(
		     s,
		     f->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     tmpname,
		     ntype,
		     mapping);
		delete [] tmpname;
		IND(s); s << "break;\n";
		DEC_INDENT_LEVEL();
	      }
	  }
	i.next();
      }
  }
  IND(s); s << "default: break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "}\n";
    }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator<<= (NetBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     disc_type(),
		     o2be_operation::wIN,mapping);
    o2be_operation::produceUnMarshalCode(
		     s,
		     disc_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     "pd__d",
		     ntype,
		     mapping);
  }

  IND(s); s << "switch(pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ":\n";
		INC_INDENT_LEVEL();
		IND(s); s << "pd__default = 0;\n";
	      }
	    else
	      {
		IND(s); s << "default:\n";
		INC_INDENT_LEVEL();
		IND(s); s << "pd__default = 1;\n";
	      }

	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     f->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }

	    char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
	    strcpy(tmpname,"pd_");
	    strcat(tmpname,f->uqname());
	    o2be_operation::produceUnMarshalCode(
		     s,
		     f->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     tmpname,
		     ntype,
		     mapping);
	    delete [] tmpname;
	    IND(s); s << "break;\n";
	    DEC_INDENT_LEVEL();
	  }
	i.next();
      }
  }
  if (nodefault() && !no_missing_disc_value()) {
    IND(s); s << "default: pd__default = 1; break;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
  
  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (MemBufferedStream &_n) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     disc_type(),
		     o2be_operation::wIN,mapping);
    o2be_operation::produceMarshalCode(
		     s,
		     disc_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     "pd__d",
		     ntype,
		     mapping);
  }
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "if (pd__default) {\n";
      INC_INDENT_LEVEL();
      {
	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_union_branch)
	      {
		AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
		if (l->label_kind() == AST_UnionLabel::UL_default)
		  {
		    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		    o2be_operation::argMapping mapping;
		    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
										   f->field_type(),
										   o2be_operation::wIN,mapping);
		    if (ntype == o2be_operation::tString) {
		      ntype = o2be_operation::tStringMember;
		      mapping.is_pointer = I_FALSE;
		    }
		    else if (ntype == o2be_operation::tObjref) {
		      ntype = o2be_operation::tObjrefMember;
		      mapping.is_pointer = I_FALSE;
		    }

		    char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		    strcpy(tmpname,"pd_");
		    strcat(tmpname,f->uqname());
		    o2be_operation::produceMarshalCode(
						       s,
						       f->field_type(),
						       ScopeAsDecl(defined_in()),
						       "_n",
						       tmpname,
						       ntype,
						       mapping);
		    delete [] tmpname;
		    break;
		  }
	      }
	    i.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "else {\n";
    }
  INC_INDENT_LEVEL();
  IND(s); s << "switch(pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		o2be_field *f = o2be_union_branch::narrow_from_decl(d);
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ":\n";
		INC_INDENT_LEVEL();
		o2be_operation::argMapping mapping;
		o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     f->field_type(),
		     o2be_operation::wIN,mapping);
		if (ntype == o2be_operation::tString) {
		  ntype = o2be_operation::tStringMember;
		  mapping.is_pointer = I_FALSE;
		}
		else if (ntype == o2be_operation::tObjref) {
		  ntype = o2be_operation::tObjrefMember;
		  mapping.is_pointer = I_FALSE;
		}

		char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
		strcpy(tmpname,"pd_");
		strcat(tmpname,f->uqname());
		o2be_operation::produceMarshalCode(
		     s,
		     f->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     tmpname,
		     ntype,
		     mapping);
		delete [] tmpname;
		IND(s); s << "break;\n";
		DEC_INDENT_LEVEL();
	      }
	  }
	i.next();
      }
  }
  IND(s); s << "default: break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  if (!(nodefault() && no_missing_disc_value()))
    {
      IND(s); s << "}\n";
    }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator<<= (MemBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    o2be_operation::argMapping mapping;
    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     disc_type(),
		     o2be_operation::wIN,mapping);
    o2be_operation::produceUnMarshalCode(
		     s,
		     disc_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     "pd__d",
		     ntype,
		     mapping);
  }

  IND(s); s << "switch(pd__d) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_union_branch)
	  {
	    AST_UnionLabel* l =o2be_union_branch::narrow_from_decl(d)->label();
	    o2be_field *f = o2be_union_branch::narrow_from_decl(d);
	    if (l->label_kind() == AST_UnionLabel::UL_label)
	      {
		IND(s); s << "case ";
		produce_disc_value(s,disc_type(),l->label_val(),this);
		s << ":\n";
		INC_INDENT_LEVEL();
		IND(s); s << "pd__default = 0;\n";
	      }
	    else
	      {
		IND(s); s << "default:\n";
		INC_INDENT_LEVEL();
		IND(s); s << "pd__default = 1;\n";
	      }
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     f->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }

	    char *tmpname = new char [strlen("pd_")+strlen(f->uqname())+1];
	    strcpy(tmpname,"pd_");
	    strcat(tmpname,f->uqname());
	    o2be_operation::produceUnMarshalCode(
		     s,
		     f->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     tmpname,
		     ntype,
		     mapping);
	    delete [] tmpname;
	    IND(s); s << "break;\n";
	    DEC_INDENT_LEVEL();
	  }
	i.next();
      }
  }
  if (nodefault() && !no_missing_disc_value()) {
    IND(s); s << "default: pd__default = 1; break;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

}

void
o2be_union::produce_typedef_hdr(fstream &s, o2be_typedef *tdef)
{
  IND(s); s << "typedef " << unambiguous_name(tdef)
	    << " " << tdef->uqname() << ";\n";
  IND(s); s << "typedef " << unambiguous_name(tdef)
	    << "_var " << tdef->uqname() << "_var;\n";
}

idl_bool
o2be_union::no_missing_disc_value()
{
  AST_Decl *decl = disc_type();
  while (decl->node_type() == AST_Decl::NT_typedef)
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

  switch (decl->node_type())
    {
    case AST_Decl::NT_enum:
      {
	disc_value_t v;
	UTL_ScopeActiveIterator i(o2be_enum::narrow_from_decl(decl),
				  UTL_Scope::IK_decls);
	while (!(i.is_done())) {
	  v.e_val = i.item();
	  if (!lookup_by_disc_value(*this,v)) {
	    return I_FALSE;
	  }
	  i.next();
	}
	break;
      }
    case AST_Decl::NT_pre_defined:
      switch (AST_PredefinedType::narrow_from_decl(decl)->pt())
	{
	case AST_PredefinedType::PT_long:
	  {
	    disc_value_t v;
	    v.i_val = -((int)0x7fffffff);
	    while (v.i_val < ((int)0x7fffffff)) {
	      if (!lookup_by_disc_value(*this,v)) {
		return I_FALSE;
	      }
	      v.i_val++;
	    }
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    break;
	  }
	case AST_PredefinedType::PT_ulong:
	  {
	    disc_value_t v;
	    v.ui_val = 0;
	    while (v.ui_val < ((unsigned int)0xffffffff)) {
	      if (!lookup_by_disc_value(*this,v)) {
		return I_FALSE;
	      }
	      v.ui_val++;
	    }
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    break;
	  }
	case AST_PredefinedType::PT_short:
	  {
	    disc_value_t v;
	    v.s_val = -((short)0x7fff);
	    while (v.s_val < ((short)0x7ffff)) {
	      if (!lookup_by_disc_value(*this,v)) {
		return I_FALSE;
	      }
	      v.s_val++;
	    }
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    break;
	  }
	case AST_PredefinedType::PT_ushort:
	  {
	    disc_value_t v;
	    v.us_val = 0;
	    while (v.us_val < ((unsigned short)0xffff)) {
	      if (!lookup_by_disc_value(*this,v)) {
		return I_FALSE;
	      }
	      v.us_val++;
	    }
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    break;
	  }
	case AST_PredefinedType::PT_char:
	  {
	    disc_value_t v;
	    v.c_val = '\0';
	    while (v.c_val < '\177') {
	      if (!lookup_by_disc_value(*this,v)) {
		return I_FALSE;
	      }
	      v.c_val++;
	    }
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    break;
	  }
	case AST_PredefinedType::PT_boolean:
	  {
	    disc_value_t v;
	    v.b_val = I_TRUE;
	    if (!lookup_by_disc_value(*this,v))
	      return I_FALSE;
	    else
	      {
		v.b_val = I_FALSE;
		if (!lookup_by_disc_value(*this,v))
		  return I_FALSE;
	      }
	    break;
	  }
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,
				    "Unexpected union discriminant value");
	  break;
	}
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,
				"Unexpected union discriminant value");
      break;
    }
  return I_TRUE;
}


static
void
produce_default_value(o2be_union &u,fstream& s)
{
  AST_Decl *decl = u.disc_type();
  while (decl->node_type() == AST_Decl::NT_typedef)
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

  switch (decl->node_type())
    {
    case AST_Decl::NT_enum:
      {
	disc_value_t v;
	UTL_ScopeActiveIterator i(o2be_enum::narrow_from_decl(decl),
				  UTL_Scope::IK_decls);
	while (!(i.is_done())) {
	  v.e_val = i.item();
	  if (!lookup_by_disc_value(u,v)) {
	    s << o2be_name::narrow_and_produce_unambiguous_name(v.e_val,&u);
	    break;
	  }
	  i.next();
	}
	break;
      }
    case AST_Decl::NT_pre_defined:
      switch (AST_PredefinedType::narrow_from_decl(decl)->pt())
	{
	case AST_PredefinedType::PT_long:
	  {
	    disc_value_t v;
	    v.i_val = -((int)0x7fffffff);
	    while (lookup_by_disc_value(u,v)) {
	      v.i_val++;
	    }
	    s << v.i_val;
	    break;
	  }
	case AST_PredefinedType::PT_ulong:
	  {
	    disc_value_t v;
	    v.ui_val = 0;
	    while (lookup_by_disc_value(u,v)) {
	      v.ui_val++;
	    }
	    s << v.ui_val;
	    break;
	  }
	case AST_PredefinedType::PT_short:
	  {
	    disc_value_t v;
	    v.s_val = -((short)0x7fff);
	    while (lookup_by_disc_value(u,v)) {
	      v.s_val++;
	    }
	    s << v.s_val;
	    break;
	  }
	case AST_PredefinedType::PT_ushort:
	  {
	    disc_value_t v;
	    v.us_val = 0;
	    while (lookup_by_disc_value(u,v)) {
	      v.us_val++;
	    }
	    s << v.us_val;
	    break;
	  }
	case AST_PredefinedType::PT_char:
	  {
	    disc_value_t v;
	    v.c_val = '\0';
	    while (lookup_by_disc_value(u,v)) {
	      v.c_val++;
	    }
	    if (v.c_val >= ' ' && v.c_val <= '~')
	      s << "'" << v.c_val << "'";
	    else {
	      s << "'\\"
		<< (int) ((v.c_val & 0100) >> 6)
		<< (int) ((v.c_val & 070) >> 3)
		<< (int) (v.c_val & 007)
		<< "'";
	    }
	    break;
	  }
	case AST_PredefinedType::PT_boolean:
	  {
	    disc_value_t v;
	    v.b_val = I_TRUE;
	    if (lookup_by_disc_value(u,v))
	      s << "0";
	    else
	      s << "1";
	    break;
	  }
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,
				    "Unexpected union discriminant value");
	  break;
	}
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,
				"Unexpected union discriminant value");
      break;
    }
  return;
}

static
void
produce_disc_value(fstream &s,AST_ConcreteType *t,AST_Expression *exp,
		   AST_Decl* used_in)
{

  if (t->node_type() != AST_Decl::NT_enum)
    {
      AST_Expression::AST_ExprValue *v = exp->ev();
      switch (v->et) 
	{
	case AST_Expression::EV_short:
	  s << v->u.sval;
	  break;
	case AST_Expression::EV_ushort:
	  s << v->u.usval;
	  break;
	case AST_Expression::EV_long:
	  s << v->u.lval;
	  break;
	case AST_Expression::EV_ulong:
	  s << v->u.ulval;
	  break;
	case AST_Expression::EV_bool:
	  s << ((v->u.bval == 0) ? "0" : "1");
	  break;
	case AST_Expression::EV_char:
	  {
	    char c = v->u.cval;
	    if (c >= ' ' && c <= '~')
	      s << "'" << c << "'";
	    else {
	      s << "'\\"
		<< (int) ((c & 0100) >> 6)
		<< (int) ((c & 070) >> 3)
		<< (int) (c & 007)
		<< "'";
	    }
	  }
	  break;
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,
				    "Unexpected union discriminant value");

	}
    }
  else
    {
      AST_Decl *v = AST_Enum::narrow_from_decl(t)->lookup_by_value(exp);
      s << o2be_name::narrow_and_produce_unambiguous_name(v,used_in);
    }
}

static idl_bool
match_disc_value(o2be_union_branch& b,AST_Decl *decl,disc_value_t v)
{
  AST_UnionLabel* l = b.label();
  
  if (l->label_kind() == AST_UnionLabel::UL_default)
    return I_FALSE;

  while (decl->node_type() == AST_Decl::NT_typedef)
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

  switch (decl->node_type())
    {
    case AST_Decl::NT_enum:
      {
	AST_Decl *bv = AST_Enum::narrow_from_decl(decl)->lookup_by_value(l->label_val());
	if (bv == v.e_val)
	  return I_TRUE;
	break;
      }
    case AST_Decl::NT_pre_defined:
      {
	AST_Expression::AST_ExprValue *bv = l->label_val()->ev();
	switch (AST_PredefinedType::narrow_from_decl(decl)->pt())
	  {
	  case AST_PredefinedType::PT_long:
	    if (bv->u.lval == v.i_val)
	      return I_TRUE;
	    break;
	  case AST_PredefinedType::PT_ulong:
	    if (bv->u.ulval == v.ui_val)
	      return I_TRUE;
	    break;
	  case AST_PredefinedType::PT_short:
	    if (bv->u.sval == v.s_val)
	      return I_TRUE;
	    break;
	  case AST_PredefinedType::PT_ushort:
	    if (bv->u.usval == v.us_val)
	      return I_TRUE;
	    break;
	  case AST_PredefinedType::PT_char:
	    if (bv->u.cval == v.c_val)
	      return I_TRUE;
	    break;
	  case AST_PredefinedType::PT_boolean:
	    if (bv->u.bval == (unsigned long)v.b_val)
	      return I_TRUE;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,
				      "Unexpected union discriminant value");
	    break;
	  }
	break;
      }
    default:
      throw o2be_internal_error(__FILE__,__LINE__,
				"Unexpected union discriminant value");
      break;
    }
  return I_FALSE;
}


static o2be_union_branch*
lookup_by_disc_value(o2be_union& u,disc_value_t v)
{
  UTL_ScopeActiveIterator i(&u,UTL_Scope::IK_decls);
  while (!i.is_done())
    {
      AST_Decl *d = i.item();
      if (d->node_type() == AST_Decl::NT_union_branch)
	{
	  o2be_union_branch *b = o2be_union_branch::narrow_from_decl(d);
	  if (match_disc_value(*(b),u.disc_type(),v))
	    return b;
	}
      i.next();
    }
  return 0;
}

const char *
o2be_union::out_adptarg_name(AST_Decl* used_in) const
{
  if (o2be_global::qflag()) {
    return pd_out_adptarg_name;
  }
  else {
    const char* ubname = unambiguous_name(used_in);
    if (strcmp(fqname(),ubname) == 0) {
      return pd_out_adptarg_name;
    }
    else {
      char* result = new char[strlen(ADPT_CLASS_TEMPLATE)+strlen("<,>")+
		       strlen(ubname)+
		       strlen(ubname)+strlen("_var")+1];
      strcpy(result,ADPT_CLASS_TEMPLATE);
      strcat(result,"<");
      strcat(result,ubname);
      strcat(result,",");
      strcat(result,ubname);
      strcat(result,"_var>");  
      return result;
    }
  }
}

IMPL_NARROW_METHODS1(o2be_union, AST_Union)
IMPL_NARROW_FROM_DECL(o2be_union)
IMPL_NARROW_FROM_SCOPE(o2be_union)

