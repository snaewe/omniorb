// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be.h                   Created on: 5/8/1996
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
//    Defines all classes for omniORB2 backend

/*
 $Log$
 Revision 1.15  1998/05/20 18:23:37  sll
 New option (-t) enable the generation of tie implementation template.

 Revision 1.14  1998/04/09 19:15:01  sll
 VarToken now emits _CORBA_GLOBAL_VAR instead of extern for globals.

 Revision 1.13  1998/04/08 19:36:41  sll
 *** empty log message ***

 Revision 1.12  1998/04/07 18:54:52  sll
 Use std::fstream instead of fstream.
 New helper functions VarToken and FriendToken to support the
 use of namespace to map module.

 * Revision 1.11  1998/01/27  16:30:42  ewc
 * Added support for type any and TypeCode
 *
 Revision 1.10  1997/12/23 19:29:58  sll
 New member o2be_array::produce_typedef_skel.

 Revision 1.9  1997/12/09 19:54:23  sll
 *** empty log message ***

 * Revision 1.8  1997/09/20  16:31:29  dpg1
 * Added new name and produce functions for LifeCycle support.
 *
 * Revision 1.7  1997/05/06  13:47:39  sll
 * Public release.
 *

 */

#ifndef _O2BE_CLASSES_H_
#define _O2BE_CLASSES_H_

#include <string.h>

#define USE_SEQUENCE_TEMPLATE_IN_PLACE 1

class o2be_name : public virtual AST_Decl
{
public:
  o2be_name(AST_Decl::NodeType t,UTL_ScopedName* n, UTL_StrList* p);

  // fully qualified name of the scope
  virtual char *scopename() const { return pd_scopename; }

  // un-qualified name
  virtual char *uqname() const   { return pd_uqname; }

  // fully qualified name, scope name + un-qualified name
  virtual char *fqname() const   { return pd_fqname; }

  // same as scopename but with '_' as the separator
  virtual char *_scopename() const { return pd__scopename; }

  // same as fqname but with '_' as the separator
  virtual char *_fqname() const   { return pd__fqname; }

  // un-qualified name, prefixed by the TypeCode prefix
  virtual char *tcname() const { return pd_tcname; }

  // fully qualified name with TypeCode prefix, 
  // scope name + TypeCode prefix + un-qualified name
  virtual char *fqtcname() const { return pd_fqtcname; }
  
  // same as above, but with '_' as the seperator
  virtual char *_fqtcname() const { return pd__fqtcname; }

  // true if contains a recursive sequence, otherwise false
  virtual idl_bool recursive_seq ()const { return pd_recursive_seq; }

  virtual char *unambiguous_name(AST_Decl* used_in,
				 idl_bool use_fqname = I_FALSE) const;
  // Looks at the scope-name relation between this class and the one it
  // is used in. 
  // If use_fqname == I_FALSE, see if unscoped or partially scoped names
  // can be used.
  // If use_fqname == I_TRUE, use fully scoped name, also check to see
  // if the prefix :: has to be added to avoid clashes with names defined
  // in the inner scopes.
  

  virtual char *unambiguous_scopename(AST_Decl* used_in,
				      idl_bool use_fqname = I_FALSE) const;
  // Looks at the scope-name relation between this class and the one it
  // is used in. 
  // If use_fqname == I_FALSE, see if unscoped or partially scoped names
  // can be used.
  // If use_fqname == I_TRUE, use fully scoped name, also check to see
  // if the prefix :: has to be added to avoid clashes with names defined
  // in the inner scopes.

  virtual char *repositoryID() const;

  void set_scopename(char *n) { pd_scopename = n; }
  void set_uqname(char *n)    { pd_uqname = n; }
  void set_fqname(char *n)    { pd_fqname = n; }
  void set__scopename(char *n){ pd__scopename = n; }
  void set__fqname(char *n)   { pd__fqname = n; }

  void set_tcname(char *n)    { pd_tcname = n; }
  void set_fqtcname(char* n)  { pd_fqtcname = n; }
  void set__fqtcname(char* n) { pd__fqtcname = n; }
  void set_recursive_seq(idl_bool v) { pd_recursive_seq = v; }

  static char *narrow_and_produce_fqname(AST_Decl *type);
  static char *narrow_and_produce__fqname(AST_Decl *type);
  static char *narrow_and_produce_scopename(AST_Decl *type);
  static char *narrow_and_produce__scopename(AST_Decl *type);
  static char *narrow_and_produce_uqname(AST_Decl *type);
  static char *narrow_and_produce_unambiguous_name(AST_Decl *type,
						   AST_Decl *used_in,
						   idl_bool use_fqname=I_FALSE);
  static char *narrow_and_produce_unambiguous_scopename(AST_Decl *type,
						   AST_Decl *used_in,
						   idl_bool use_fqname=I_FALSE);
  static void narrow_and_produce_typecode_skel(AST_Decl *type, 
					       std::fstream& s);
  static void produce_typecode_member(AST_Decl *type, 
				      std::fstream& s,
				      idl_bool new_ptr=I_TRUE);
  static idl_bool narrow_and_check_recursive_seq(AST_Decl *decl);

private:
  o2be_name();
  char *pd_scopename;
  char *pd_uqname;
  char *pd_fqname;
  char *pd__scopename;
  char *pd__fqname;
  char *pd_repositoryID;
  char *pd_tcname;
  char *pd_fqtcname;
  char *pd__fqtcname;
  idl_bool pd_recursive_seq;
};

class o2be_typedef;
class o2be_sequence;
class o2be_sequence_chain : public virtual AST_Decl {
public:
  o2be_sequence_chain(AST_Decl::NodeType t,UTL_ScopedName* n, UTL_StrList* p) :
    AST_Decl(t,n,p) { pd_seq_decl = 0; }
  virtual ~o2be_sequence_chain() {}
  void set_seq_decl(o2be_sequence *d);
  o2be_sequence *get_seq_decl()  { return pd_seq_decl; }
  void produce_seq_hdr_if_defined(std::fstream &s);
private:
  o2be_sequence_chain();
  o2be_sequence *pd_seq_decl;
};

class o2be_predefined_type : public virtual AST_PredefinedType,
			     public virtual o2be_name,
			     public virtual o2be_sequence_chain
{
public:

  o2be_predefined_type(AST_PredefinedType::PredefinedType t,
		       UTL_ScopedName *n,
		       UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_predefined_type, AST_PredefinedType);
  DEF_NARROW_FROM_DECL(o2be_predefined_type);

  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  const char* fieldMemberTypeName();
  static const char* TypeCodeMemberName();

  virtual char *tckname() const { return pd_tckname; }
  void set_tckname(char *n) { pd_tckname = n; }

private:
  o2be_predefined_type();
  char *pd_tckname;
};

class o2be_constant : public virtual AST_Constant,
		      public virtual o2be_name
{
public:

  o2be_constant(AST_Expression::ExprType et,
		AST_Expression *v,
		UTL_ScopedName *n,
		UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_constant, AST_Constant);
  DEF_NARROW_FROM_DECL(o2be_constant);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);

private:
  o2be_constant();

};

class o2be_enum : public virtual AST_Enum,
		  public virtual o2be_name,
		  public virtual o2be_sequence_chain
{
public:

  o2be_enum(UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_enum, AST_Enum);
  DEF_NARROW_FROM_DECL(o2be_enum);
  DEF_NARROW_FROM_SCOPE(o2be_enum);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  void produce_typecode_skel(std::fstream &s);

  void set_hdr_produced_in_field() { pd_hdr_produced_in_field = I_TRUE; }
  idl_bool get_hdr_produced_in_field() { return pd_hdr_produced_in_field; }
  void set_skel_produced_in_field() { pd_skel_produced_in_field = I_TRUE; }
  idl_bool get_skel_produced_in_field() { return pd_skel_produced_in_field; }

private:
  idl_bool pd_hdr_produced_in_field;
  idl_bool pd_skel_produced_in_field;
  o2be_enum();
};

class o2be_enum_val : public virtual AST_EnumVal,
		      public virtual o2be_name
{
public:

  o2be_enum_val(unsigned long v, UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_enum_val, AST_EnumVal);
  DEF_NARROW_FROM_DECL(o2be_enum_val);

private:
  o2be_enum_val();
};

class o2be_string : public virtual AST_String,
		    public virtual o2be_name,
		    public virtual o2be_sequence_chain
{
public:

  o2be_string(AST_Expression *v);
  o2be_string(AST_Expression *v, long wide);

  DEF_NARROW_METHODS1(o2be_string, AST_String);
  DEF_NARROW_FROM_DECL(o2be_string);

  static const char *fieldMemberTypeName();
  static void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  size_t max_length();

private:
  o2be_string();

};

class o2be_field : public virtual AST_Field,
		   public virtual o2be_name
{
public:

  o2be_field(AST_Type *ft, UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_field, AST_Field);
  DEF_NARROW_FROM_DECL(o2be_field);

private:
  o2be_field();

};

class o2be_union : public virtual AST_Union,
		   public virtual o2be_name,
		   public virtual o2be_sequence_chain

{
public:

  o2be_union(AST_ConcreteType *dt, UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_union, AST_Union);
  DEF_NARROW_FROM_DECL(o2be_union);
  DEF_NARROW_FROM_SCOPE(o2be_union);

  virtual AST_UnionBranch *add_union_branch(AST_UnionBranch *un);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  void produce_typecode_skel(std::fstream &s);
  idl_bool check_recursive_seq();

  idl_bool isVariable() { return pd_isvar; }
  idl_bool nodefault() { return pd_nodefault; }
  idl_bool no_missing_disc_value();

  void set_hdr_produced_in_field() { pd_hdr_produced_in_field = I_TRUE; }
  idl_bool get_hdr_produced_in_field() { return pd_hdr_produced_in_field; }
  void set_skel_produced_in_field() { pd_skel_produced_in_field = I_TRUE; }
  idl_bool get_skel_produced_in_field() { return pd_skel_produced_in_field; }

  const char *out_adptarg_name(AST_Decl* used_in) const;

private:
  idl_bool pd_hdr_produced_in_field;
  idl_bool pd_skel_produced_in_field;
  idl_bool pd_isvar;
  idl_bool pd_nodefault;
  char *pd_out_adptarg_name;

  o2be_union();

};

class o2be_union_branch : public virtual AST_UnionBranch,
			  public virtual o2be_field
{
public:

  o2be_union_branch(AST_UnionLabel *lab, AST_Type *ft, UTL_ScopedName *n,
		  UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_union_branch, AST_UnionBranch);
  DEF_NARROW_FROM_DECL(o2be_union_branch);

private:
  o2be_union_branch();

};

class o2be_structure : public virtual AST_Structure,
		       public virtual o2be_name,
		       public virtual o2be_sequence_chain
{
public:

  o2be_structure(UTL_ScopedName *n, UTL_StrList *p);

  virtual AST_Field *add_field(AST_Field *f);

  DEF_NARROW_METHODS1(o2be_structure, AST_Structure);
  DEF_NARROW_FROM_DECL(o2be_structure);
  DEF_NARROW_FROM_SCOPE(o2be_structure);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  void produce_typecode_skel(std::fstream &s);
  idl_bool check_recursive_seq();

  idl_bool isVariable() { return pd_isvar; }

  void set_hdr_produced_in_field() { pd_hdr_produced_in_field = I_TRUE; }
  idl_bool get_hdr_produced_in_field() { return pd_hdr_produced_in_field; }
  void set_skel_produced_in_field() { pd_skel_produced_in_field = I_TRUE; }
  idl_bool get_skel_produced_in_field() { return pd_skel_produced_in_field; }

  const char *out_adptarg_name(AST_Decl* used_in) const;

private:
  idl_bool pd_hdr_produced_in_field;
  idl_bool pd_skel_produced_in_field;
  idl_bool pd_isvar;
  char *pd_out_adptarg_name;

  o2be_structure();

};

class o2be_exception : public virtual AST_Exception,
		       public virtual o2be_name
{
public:

  o2be_exception(UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_exception, AST_Exception);
  DEF_NARROW_FROM_DECL(o2be_exception);
  DEF_NARROW_FROM_SCOPE(o2be_exception);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);

  void produce_typecode_skel(std::fstream &s);
  idl_bool check_recursive_seq();

  size_t repoIdConstLen() const { return pd_repoidsize; }
  const char *repoIdConstName() const { return pd_repoid; }

private:
  char *pd_repoid;
  size_t pd_repoidsize;
  o2be_exception();

};

class o2be_typedef;
class o2be_array : public virtual AST_Array,
		   public virtual o2be_name
{
public:

  o2be_array(UTL_ScopedName *n, unsigned long ndims, UTL_ExprList *dims);

  DEF_NARROW_METHODS1(o2be_array, AST_Array);
  DEF_NARROW_FROM_DECL(o2be_array);

  idl_bool isVariable();

  size_t getSliceDim();
  // Get the array slice dimension

  AST_Decl *getElementType();
  // returns the element type. If the element is an array, returns the
  // value returned by the recursive call to getElementType().

  size_t   getNumOfElements();
  // return the total number of elements. This is the sum of all the
  // dimensions. If the element is an array, multiply by the
  // value returned by the recursive call to getNumOfElements().

  size_t   getNumOfDims();
  // return the number of dimensions. If the element is an array, add
  // the value returned by the recursive call to getNumOfDims();

  class dim_iterator {
  public:
    dim_iterator(o2be_array *);
    ~dim_iterator() {};
    size_t operator() ();
    // iterates and returns the value of each dimension. If the element is
    // an array, also returns the dimensions of the array element.
  private:
    size_t pd_ndim;
    size_t pd_next;
    AST_Expression **pd_dims;
  };

  void produce_hdr (std::fstream &s, o2be_typedef *tdef);
  void produce_skel(std::fstream &s, o2be_typedef *tdef);

  void produce_typecode_skel(std::fstream &s);
  void produce_typecode_member(std::fstream &s, idl_bool new_ptr);
  idl_bool check_recursive_seq();

  static void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef1,
				   o2be_typedef *tdef2);
  static void produce_typedef_skel (std::fstream &s, o2be_typedef *tdef1,
				    o2be_typedef *tdef2);
  void produce_typedef_in_union(std::fstream &s, const char *tname,
				AST_Decl* used_in);
  // Looks at the scope-name relation between this node and the one it is
  // used in. Generate the fieldmember type that used the unambiguous name
  // of this node in the scope where it is used.

  void produce_struct_member_decl (std::fstream &s, AST_Decl *fieldtype,
				   AST_Decl* used_in);
  // Looks at the scope-name relation between <fieldtype> and the one it is
  // used in. Generate the fieldmember type that used the unambiguous name
  // of <fieldtype> in the scope where it is used.

  void produce_union_member_decl (std::fstream &s, AST_Decl *fieldtype,
				  AST_Decl* used_in);
  // Looks at the scope-name relation between <fieldtype> and the one it is
  // used in. Generate the fieldmember type that used the unambiguous name
  // of <fieldtype> in the scope where it is used.

  const char *out_adptarg_name(o2be_typedef* tdef,AST_Decl* used_in) const;

private:
  o2be_array();
  void _produce_member_decl(std::fstream &s, char *varname,AST_Decl* used_in);
};

class o2be_sequence : public virtual AST_Sequence,
		      public virtual o2be_name,
		      public virtual o2be_sequence_chain
{
public:

  o2be_sequence(AST_Expression *v, AST_Type *bt);
  ~o2be_sequence() {}

  char* seq_template_name(AST_Decl* used_in);
  // Looks at the scope-name relation between the element class and the one it
  // is used in. Generate the template that used the unambiguous name of
  // the element in the scope where this template name is used.

  const char* seq_member_name(AST_Decl* used_in);
  // Looks at the scope-name relation between the element class and the one it
  // is used in. Generate the template that used the unambiguous name of
  // the element in the scope where this template name is used.

  DEF_NARROW_METHODS1(o2be_sequence, AST_Sequence);
  DEF_NARROW_FROM_DECL(o2be_sequence);
  DEF_NARROW_FROM_SCOPE(o2be_sequence);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);

  void produce_typecode_skel(std::fstream &s);
  void produce_typecode_member(std::fstream &s, idl_bool new_ptr);
  idl_bool check_recursive_seq();

  static void produce_hdr_for_predefined_types(std::fstream &s);
  static AST_Sequence *attach_seq_to_base_type(AST_Sequence *se);

  const char *out_adptarg_name(o2be_typedef* tdef,AST_Decl* used_in) const;

private:
  o2be_sequence();
  enum seqnametype { EFFECTIVE_TYPE, IMMEDIATE_TYPE, BASE_TYPE };
  char *pd_effname;
  static char *internal_produce_seqname(AST_Decl *basetype,
					enum seqnametype stype);
  char *internal_produce_localname(enum seqnametype stype);
};


class o2be_argument : public virtual AST_Argument,
		      public virtual o2be_name
{
public:
  o2be_argument(AST_Argument::Direction d, AST_Type *ft, UTL_ScopedName *n,
		UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_argument, AST_Argument);
  DEF_NARROW_FROM_DECL(o2be_argument);
private:
  o2be_argument();
};

class o2be_interface;
class o2be_attribute : public virtual AST_Attribute,
		       public virtual o2be_name
{
public:

  o2be_attribute(idl_bool ro, AST_Type *ft, UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_attribute, AST_Attribute);
  DEF_NARROW_FROM_DECL(o2be_attribute);
  DEF_NARROW_FROM_SCOPE(o2be_attribute);

  void produce_decl_rd(std::fstream &s,
		       const char *prefix = 0,
		       idl_bool out_var_default = I_TRUE,
		       idl_bool use_fully_qualified_names = I_FALSE);

  void produce_decl_wr(std::fstream &s,
		       const char *prefix = 0,
		       idl_bool out_var_default = I_TRUE,
		       idl_bool use_fully_qualified_names = I_FALSE);

  void produce_proxy_rd_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the definition of the proxy's method to get this attribute

  void produce_proxy_wr_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the definition of the proxy's method to set this attribute

  void produce_server_rd_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the code fragment within the server's dispatch routine
  // to handle getting this attribute

  void produce_server_wr_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the code fragment within the server's dispatch routine
  // to handle setting this attirbute

  void produce_nil_rd_skel(std::fstream &s);
  // produce the definition of the nil object's method to get this attribute

  void produce_nil_wr_skel(std::fstream &s);
  // produce the definition of the nil object's method to set this attribute

  void produce_dead_rd_skel(std::fstream &s);
  // produce the definition of the dead object's get method

  void produce_dead_wr_skel(std::fstream &s);
  // produce the definition of the dead object's set method

  void produce_home_rd_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the _wrap_home get method

  void produce_home_wr_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the _wrap_home set method

  void produce_lcproxy_rd_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the LifeCycle proxy's method to get this attribute

  void produce_lcproxy_wr_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the LifeCycle proxy's method to set this attribute

  void produce_wrapproxy_rd_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the _wrap_proxy get method

  void produce_wrapproxy_wr_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the _wrap_proxy set method

private:
  o2be_attribute();
};

class o2be_operation : public virtual AST_Operation,
		       public virtual o2be_name
{
public:

  o2be_operation(AST_Type *rt, AST_Operation::Flags fl,
		 UTL_ScopedName *n,UTL_StrList *p);
  ~o2be_operation() {}

  DEF_NARROW_METHODS1(o2be_operation, AST_Operation);
  DEF_NARROW_FROM_DECL(o2be_operation);
  DEF_NARROW_FROM_SCOPE(o2be_operation);

  void produce_decl(std::fstream &s,
		    const char *prefix = 0,
		    const char *alias_prefix = 0,
		    idl_bool out_var_default = I_TRUE,
		    idl_bool use_fully_qualified_names = I_FALSE);
  // produce the declaration of the mapping of this operation

  void produce_proxy_skel(std::fstream &s,o2be_interface &defined_in,
			  const char* alias_prefix=0);
  // produce the definition of the proxy's method to invoke this
  // operation

  void produce_server_skel(std::fstream &s,o2be_interface &defined_in);
  // produce the code fragment within the server's dispatch routine
  // to handle this operation

  void produce_nil_skel(std::fstream &s,const char* alias_prefix=0);
  // produce the definition of the nil object's method

  void produce_dead_skel(std::fstream &s,const char* alias_prefix=0);
  // produce the definition of the dead object's method

  void produce_home_skel(std::fstream &s,o2be_interface &defined_in,
			 const char* alias_prefix=0);
  // produce the _wrap_home method

  void produce_lcproxy_skel(std::fstream &s,o2be_interface &defined_in,
			    const char* alias_prefix=0);
  // produce the LifeCycle proxy's method to invoke this operation

  void produce_wrapproxy_skel(std::fstream &s,o2be_interface &defined_in,
			      const char* alias_prefix=0);
  // produce the _wrap_proxy method

  void produce_mapping_with_indirection(std::fstream& s,
					const char* alias_prefix);
  // produce the mapping for this operation using the adaptation classes
  // <T>_INOUT_arg and <T>_OUT_arg.

  idl_bool has_variable_out_arg();
  // Return true if there is any variable length OUT argument

  idl_bool has_pointer_inout_arg();
  // Return true if there is any string or objref INOUT argument

  idl_bool return_is_void();
  // returns I_TRUE if the return value of this operation is void

  void produce_invoke(std::fstream &s);
  // produce an invocation. Use the argument names as defined in the
  // IDL.

  friend class o2be_attribute;
  friend class o2be_structure;
  friend class o2be_sequence;
  friend class o2be_union;
  friend class o2be_exception;
  friend class o2be_array;

  enum argType {
    tShort = 0, tLong = 1, tUShort = 2, tULong = 3, tFloat = 4, tDouble = 5,
    tBoolean = 6, tChar = 7, tOctet = 8, tEnum = 9, tObjref = 10,
    tStructFixed = 11, tStructVariable = 12, tUnionFixed = 13,
    tUnionVariable = 14, tString = 15, tSequence = 16, tArrayFixed = 17,
    tArrayVariable = 18, tAny = 19, tTypeCode = 20, tObjrefMember = 21, 
    tStringMember = 22, tTypeCodeMember = 23,
    _tMaxValue
  };

  struct argMapping {
    idl_bool is_const;
    idl_bool is_reference;
    idl_bool is_pointer;
    idl_bool is_arrayslice;
  };

  enum argWhich {
    wResult, wIN, wOUT, wINOUT
  };

private:
  o2be_operation();

  idl_bool no_user_exception();
  // returns I_TRUE if this operation does not raise a user exception

  static
  argType ast2ArgMapping(AST_Decl *decl,argWhich dir,argMapping &mapping);

  static
  void declareVarType(std::fstream &s, AST_Decl *decl, AST_Decl* used_in,
		      idl_bool is_var=0,
		      idl_bool is_arrayslice=0);

  static
  void produceUnMarshalCode(std::fstream &s, AST_Decl *decl, AST_Decl* used_in,
			    const char *netstream,
			    const char *argname,
			    argType type, argMapping mapping,
			    idl_bool no_size_check=0);

  static
  void produceMarshalCode(std::fstream &s, AST_Decl *decl, AST_Decl* used_in,
			  const char *netstream,
			  const char *argname,
			  argType type, argMapping mapping);

  static
  void produceSizeCalculation(std::fstream &s, AST_Decl *decl, AST_Decl* used_in,
			      const char *netstream,
			      const char *sizevar,
			      const char *argname,
			      argType type, argMapping mapping);

  static
  void produceConstStringMarshalCode(std::fstream &s,
				     const char *netstream,
				     const char *str,size_t len);

  static
  void produceConstStringSizeCalculation(std::fstream &s,
					 const char *sizevar,
					 size_t len);
};

class o2be_typedef : public virtual AST_Typedef,
		     public virtual o2be_name,
		     public virtual o2be_sequence_chain
{
public:

  o2be_typedef(AST_Type *bt, UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_typedef, AST_Typedef);
  DEF_NARROW_FROM_DECL(o2be_typedef);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);

  void produce_typecode_skel(std::fstream &s);
  idl_bool check_recursive_seq();

  const char *fieldMemberType_uqname() const { return pd_fm_uqname; }
  const char *fieldMemberType_fqname(AST_Decl* used_in);
  // Looks at the scope-name relation between this node and the one it is
  // used in. Generate the fieldmember type that used the unambiguous name
  // of this node in the scope where this template name is used.

private:
  char* pd_fm_uqname;
  o2be_typedef();

};

class o2be_interface : public virtual AST_Interface,
		       public virtual o2be_name,
		       public virtual o2be_sequence_chain
{
public:

  o2be_interface(UTL_ScopedName *n, AST_Interface **ih, long nih,
	       UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_interface, AST_Interface);
  DEF_NARROW_FROM_DECL(o2be_interface);
  DEF_NARROW_FROM_SCOPE(o2be_interface);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_typedef_hdr (std::fstream &s, o2be_typedef *tdef);
  void produce_tie_templates(std::fstream &s);

  const char *objref_uqname() const { return pd_objref_uqname; }
  const char *objref_fqname() const { return pd_objref_fqname; }
  const char *proxy_uqname() const { return pd_proxy_uqname; }
  const char *proxy_fqname() const { return pd_proxy_fqname; }
  const char *server_uqname() const { return pd_server_uqname; }
  const char *server_fqname() const { return pd_server_fqname; }
  const char *fieldMemberType_uqname() const { return pd_fieldmem_uqname; }
  const char *fieldMemberType_fqname(AST_Decl* used_in) const;
  // Looks at the scope-name relation between this node and the one it is
  // used in. Generate the fieldmember type that used the unambiguous name
  // of this node in the scope where this template name is used.

  const char *nil_uqname() const { return pd_nil_uqname; }
  const char *nil_fqname() const { return pd_nil_fqname; }

  const char *lcserver_uqname() const { return pd_lcserver_uqname; }
  const char *lcserver_fqname() const { return pd_lcserver_fqname; }
  const char *dead_uqname() const { return pd_dead_uqname; }
  const char *dead_fqname() const { return pd_dead_fqname; }
  const char *home_uqname() const { return pd_home_uqname; }
  const char *home_fqname() const { return pd_home_fqname; }
  const char *lcproxy_uqname() const { return pd_lcproxy_uqname; }
  const char *lcproxy_fqname() const { return pd_lcproxy_fqname; }
  const char *wrapproxy_uqname() const { return pd_wrapproxy_uqname; }
  const char *wrapproxy_fqname() const { return pd_wrapproxy_fqname; }

  const char *IRrepoId() const { return pd_IRrepoId; }
  size_t IRrepoIdSize() const { return pd_IRrepoIdSize; }
  const char *inout_adptarg_name(AST_Decl* used_in) const;
  const char *out_adptarg_name(AST_Decl* used_in) const;

  const char *unambiguous_objref_name(AST_Decl* used_in,
				      idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_proxy_name(AST_Decl* used_in,
				     idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_server_name(AST_Decl* used_in,
				      idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_nil_name(AST_Decl* used_in,
				   idl_bool use_fqname=I_FALSE) const;

  const char *unambiguous_home_name(AST_Decl* used_in,
				    idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_dead_name(AST_Decl* used_in,
				    idl_bool use_fqname=I_FALSE) const;

  const char *unambiguous_wrapproxy_name(AST_Decl* used_in,
					 idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_lcserver_name(AST_Decl* used_in,
					idl_bool use_fqname=I_FALSE) const;
  const char *unambiguous_lcproxy_name(AST_Decl* used_in,
				      idl_bool use_fqname=I_FALSE) const;

  static idl_bool check_opname_clash(o2be_interface *p,char *opname);

private:
  char *pd_objref_uqname;
  char *pd_objref_fqname;
  char *pd_proxy_uqname;
  char *pd_proxy_fqname;
  char *pd_server_uqname;
  char *pd_server_fqname;
  char *pd_fieldmem_uqname;
  char *pd_fieldmem_fqname;
  char *pd_nil_uqname;
  char *pd_nil_fqname;

  char *pd_lcserver_uqname;
  char *pd_lcserver_fqname;
  char *pd_dead_uqname;
  char *pd_dead_fqname;
  char *pd_home_uqname;
  char *pd_home_fqname;
  char *pd_lcproxy_uqname;
  char *pd_lcproxy_fqname;
  char *pd_wrapproxy_uqname;
  char *pd_wrapproxy_fqname;

  char *pd_IRrepoId;
  size_t pd_IRrepoIdSize;
  char *pd_inout_adptarg_name;
  char *pd_out_adptarg_name;

  o2be_interface();
};

class o2be_interface_fwd : public virtual AST_InterfaceFwd,
			   public virtual o2be_name
{
public:

  o2be_interface_fwd(UTL_ScopedName *n, UTL_StrList *p);

  DEF_NARROW_METHODS1(o2be_interface_fwd, AST_InterfaceFwd);
  DEF_NARROW_FROM_DECL(o2be_interface_fwd);
  DEF_NARROW_FROM_SCOPE(o2be_interface_fwd);

  void produce_hdr(std::fstream &s);

private:
  o2be_interface_fwd();

};

class o2be_module : public virtual AST_Module,
		    public virtual o2be_name
{
public:
  o2be_module(UTL_ScopedName *n, UTL_StrList *p);
  ~o2be_module() {}

  DEF_NARROW_METHODS1(o2be_module, AST_Module);
  DEF_NARROW_FROM_DECL(o2be_module);
  DEF_NARROW_FROM_SCOPE(o2be_module);

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
  void produce_tie_templates(std::fstream &s);

private:
  o2be_module();

};

class o2be_root : public virtual AST_Root,
		  public virtual o2be_module
{
public:

  o2be_root(UTL_ScopedName *n, UTL_StrList *p);
  ~o2be_root() {}

  virtual AST_Sequence *add_sequence(AST_Sequence *s);

  DEF_NARROW_METHODS1(o2be_root, AST_Root);
  DEF_NARROW_FROM_DECL(o2be_root);
  DEF_NARROW_FROM_SCOPE(o2be_root);

  void produce();

private:
  o2be_root();

  char *basename;
  int   baselen;
  std::fstream pd_hdr;
  std::fstream pd_skel;

  void produce_hdr(std::fstream &s);
  void produce_skel(std::fstream &s);
};



#define DEFAULT_IDL_HDR_SUFFIX ".hh"
#define DEFAULT_IDL_SKEL_SUFFIX "SK.cc"
#define DEFAULT_IDL_SUFFIXLEN   5    // Max. of HDR_ and SKEL_SUFFIX

class o2be_global {
private:
  static o2be_root *myself;
  static char     *pd_hdrsuffix;
  static char     *pd_skelsuffix;
  static size_t    pd_suffixlen;
  static int       pd_aflag;      // generate stub for 'any' type
  static int       pd_fflag;      // generate stub for float and double
  static int       pd_qflag;      // always use fully qualified name
  static int       pd_mflag;      // generate stub to work around MSVC bugs

public:
  static void set_aflag(int f) { pd_aflag = f; }
  static int aflag() { return pd_aflag; }

  static void set_fflag(int f) { pd_fflag = f; }
  static int fflag() { return pd_fflag; }

  static void set_qflag(int f) { pd_qflag = f; }
  static int qflag() { return pd_qflag; }

  static void set_mflag(int f) { pd_mflag = f; }
  static int mflag() { return pd_mflag; }

  static int suffixlen() { return pd_suffixlen; }

  static void set_hdrsuffix(char *h) {
    pd_hdrsuffix = new char[strlen(h)+1];
    if (strlen(h) > pd_suffixlen)
      pd_suffixlen = strlen(h);
    strcpy(pd_hdrsuffix,h);
    return;
  }

  static char *hdrsuffix() { return pd_hdrsuffix; }

  static void set_skelsuffix(char *c) {
    pd_skelsuffix = new char[strlen(c)+1];
    if (strlen(c) > pd_suffixlen)
      pd_suffixlen = strlen(c);
    strcpy(pd_skelsuffix,c);
    return;
  }

  static char *skelsuffix() { return pd_skelsuffix; }

  static void set_root(o2be_root *v) { myself = v; }
  static o2be_root *root() { return myself; }
};

#define INC_INDENT_LEVEL()  idl_global->indent()->increase();
#define DEC_INDENT_LEVEL()  idl_global->indent()->decrease();
#define IND(s) idl_global->indent()->skip_to(s);


class o2be_generator : public AST_Generator {
public:
  virtual AST_Root *create_root(UTL_ScopedName *n,
				UTL_StrList *p);

  virtual AST_PredefinedType *
          create_predefined_type(AST_PredefinedType::PredefinedType t,
				 UTL_ScopedName *n,
				 UTL_StrList *p);

  virtual AST_Module *
          create_module(UTL_ScopedName *n, UTL_StrList *p);

  virtual AST_Interface	*
          create_interface(UTL_ScopedName *n,
			   AST_Interface **ih,
			   long nih,
			   UTL_StrList *p);

  virtual AST_InterfaceFwd *
          create_interface_fwd(UTL_ScopedName *n,
			       UTL_StrList *p);

  virtual AST_Exception	*
          create_exception(UTL_ScopedName *n, UTL_StrList *p);

  virtual AST_Structure	*
          create_structure(UTL_ScopedName *n, UTL_StrList *p);

  virtual AST_Enum *
          create_enum(UTL_ScopedName *n, UTL_StrList *p);

  virtual AST_Operation	*
          create_operation(AST_Type *rt,
			   AST_Operation::Flags fl,
			   UTL_ScopedName *n,
			   UTL_StrList *p);

  virtual AST_Field *
          create_field(AST_Type *ft,
		       UTL_ScopedName *n,
		       UTL_StrList *p);

  virtual AST_Argument *
          create_argument(AST_Argument::Direction d,
			  AST_Type *ft,
			  UTL_ScopedName *n,
			  UTL_StrList *p);

  virtual AST_Attribute	*
          create_attribute(idl_bool ro,
			   AST_Type *ft,
			   UTL_ScopedName *n,
			   UTL_StrList *p);

  virtual AST_Union *
          create_union(AST_ConcreteType *dt,
		       UTL_ScopedName *n,
		       UTL_StrList *p);

  virtual AST_UnionBranch *
          create_union_branch(AST_UnionLabel *lab,
			      AST_Type *ft,
			      UTL_ScopedName *n,
			      UTL_StrList *p);

  virtual AST_Constant *
          create_constant(AST_Expression::ExprType et,
			  AST_Expression *ev,
			  UTL_ScopedName *n,
			  UTL_StrList *p);

  virtual AST_EnumVal *
          create_enum_val(unsigned long v,
			  UTL_ScopedName *n,
			  UTL_StrList *p);

  virtual AST_Array *
          create_array(UTL_ScopedName *n,
		       unsigned long ndims,
		       UTL_ExprList *dims);

  virtual AST_Sequence *
          create_sequence(AST_Expression *v, AST_Type *bt);

  virtual AST_String *
          create_string(AST_Expression *v);

  virtual AST_String *
          create_wstring(AST_Expression *v);

  virtual AST_Typedef *
          create_typedef(AST_Type *bt,
			 UTL_ScopedName *n,
			 UTL_StrList *p);
};

class o2be_unsupported {
public:
  o2be_unsupported(const char *idlfile,int line,const char *msg) {
    pd_file = idlfile;
    pd_line = line;
    pd_msg = msg;
  }
  ~o2be_unsupported() {}
  const char *file() const { return pd_file; }
  int line() const { return pd_line; }
  const char *msg() const { return pd_msg; }
private:
  const char *pd_file;
  int	pd_line;
  const char *pd_msg;
  o2be_unsupported();
};

class o2be_internal_error {
public:
  o2be_internal_error(const char *file,int line,const char *errmsg) {
    pd_file = file;
    pd_line = line;
    pd_errmsg = errmsg;
  }
  ~o2be_internal_error(){}
  const char *file() const { return pd_file; }
  int line() const { return pd_line; }
  const char *errmsg() const { return pd_errmsg; }
private:
  const char *pd_file;
  int	      pd_line;
  const char *pd_errmsg;
  o2be_internal_error();
};

class o2be_fileio_error {
public:
  o2be_fileio_error(const char *errmsg) {
    pd_errmsg = errmsg;
  }
  ~o2be_fileio_error() {}
  const char *errmsg() const { return pd_errmsg; }
private:
  const char *pd_errmsg;
  o2be_fileio_error();
};

static inline char const* FriendToken(AST_Decl& decl) {
  if (decl.defined_in()==idl_global->root())
    return "";
  else if (decl.defined_in()->scope_node_type()==AST_Decl::NT_module)
    return "_CORBA_MODULE_OP";
  else
    return "friend";
}

static inline char const* VarToken(AST_Decl& decl) {
  if (decl.defined_in()==idl_global->root())
    return "_CORBA_GLOBAL_VAR";
  else if (decl.defined_in()->scope_node_type()==AST_Decl::NT_module)
    return "_CORBA_MODULE_VAR";
  else
    return "static _LC_attr";
}

#endif
