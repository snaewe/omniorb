// -*- c++ -*-
//                          Package   : omniidl
// idl.yy                   Created on: 1999/10/05
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Bison parser

// $Id$
// $Log$
// Revision 1.11.2.6  2001/03/13 10:32:10  dpg1
// Fixed point support.
//
// Revision 1.11.2.5  2000/12/05 17:45:18  dpg1
// omniidl case sensitivity updates from omni3_develop.
//
// Revision 1.11.2.4  2000/11/01 12:45:55  dpg1
// Update to CORBA 2.4 specification.
//
// Revision 1.11.2.3  2000/10/27 16:31:07  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.11.2.2  2000/10/10 10:18:50  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.9.2.5  2000/08/01 11:27:45  dpg1
// Comments were incorrectly attached to struct members.
//
// Revision 1.9.2.4  2000/06/09 11:20:47  dpg1
// Last fix put __omni_pragma line numbers off by one...
//
// Revision 1.9.2.3  2000/06/08 14:58:19  dpg1
// Line numbers for #pragmas and // comments were off by one
//
// Revision 1.9.2.2  2000/06/08 14:36:19  dpg1
// Comments and pragmas are now objects rather than plain strings, so
// they can have file,line associated with them.
//
// Revision 1.9.2.1  2000/06/05 18:13:26  dpg1
// Comments can be attached to subsequent declarations (with -K). Better
// idea of most recent decl in operation declarations
//
// Revision 1.9  2000/02/04 12:17:09  dpg1
// Support for VMS.
//
// Revision 1.8  1999/12/28 18:16:07  dpg1
// positive_int_const isn't allowed to be zero.
//
// Revision 1.7  1999/11/17 17:17:00  dpg1
// Changes to remove static initialisation of objects.
//
// Revision 1.6  1999/11/04 17:16:55  dpg1
// Changes for NT.
//
// Revision 1.5  1999/11/03 17:24:05  dpg1
// Added optional pragmas all over the place.
//
// Revision 1.4  1999/11/02 17:07:28  dpg1
// Changes to compile on Solaris.
//
// Revision 1.3  1999/11/01 20:19:57  dpg1
// Support for union switch types declared inside the switch statement.
//
// Revision 1.2  1999/10/29 15:41:31  dpg1
// DeclaredType() now takes extra DeclRepoId* argument.
//
// Revision 1.1  1999/10/27 14:06:00  dpg1
// *** empty log message ***
//

%{

#include <stdlib.h>
#include <string.h>

#include <idlutil.h>
#include <idlerr.h>
#include <idlrepoId.h>
#include <idlscope.h>
#include <idltype.h>
#include <idlexpr.h>
#include <idlast.h>

#define YYDEBUG 1

// Globals from lexer
extern int         yylineno;
extern char*       currentFile;
extern IDL_Boolean mainFile;

void yyerror(char *s) {
}
extern int yylex();

// Nasty hack for abstract valuetypes
ValueAbs* valueabs_hack = 0;

#ifdef __VMS
/*  Apparently, __ALLOCA is defined for some versions of the C (but not C++)
    compiler on VAX. */
#if defined(__ALPHA) || defined(__DECC) && __DECC_VER >= 60000000
#include <builtins.h>
#define alloca __ALLOCA
#else
#define alloca malloc
#endif
#endif

%}

%union {
  char*                    id_val;
  int                      int_val;
  IDL_ULong                ulong_val;
  IdlIntLiteral            int_literal_val;
#ifndef __VMS
  IdlFloatLiteral          float_literal_val;
#else
  double                   float_literal_val;
#endif
  char                     char_val;
  char*                    string_val;
  IDL_WChar                wchar_val;
  IDL_WChar*               wstring_val;
  IDL_Boolean              boolean_val;
  IDL_Fixed*               fixed_val;
  IdlType*                 type_val;
  TypeSpec*                type_spec_val;
  IdlExpr*                 expr_val;
  ScopedName*              scopedname_val;
  Decl*                    decl_val;
  Module*                  module_val;
  Interface*               interface_val;
  InheritSpec*             inheritspec_val;
  Forward*                 forward_val;
  Const*                   const_val;
  Typedef*                 typedef_val;
  Struct*                  struct_val;
  Exception*               exception_val;
  Member*                  member_val;
  Declarator*              declarator_val;
  Union*                   union_val;
  UnionCase*               union_case_val;
  CaseLabel*               case_label_val;
  ValueBase*               value_base_val;
  Value*                   value_val;
  ValueForward*            value_forward_val;
  ValueBox*                value_box_val;
  ValueAbs*                value_abs_val;
  ValueInheritSpec*        valueinheritspec_val;
  ValueInheritSupportSpec* valueinheritsupportspec_val;
  StateMember*             statemember_val;
  Factory*                 factory_val;
  Enumerator*              enumerator_val;
  Enum*                    enum_val;
  ArraySize*               array_size_val;
  Attribute*               attribute_val;
  Operation*               operation_val;
  Parameter*               parameter_val;
  RaisesSpec*              raisesspec_val;
  ContextSpec*             contextspec_val;
}

%token <id_val> IDENTIFIER

// Keywords
%token ABSTRACT
%token ANY
%token ATTRIBUTE
%token BOOLEAN
%token CASE
%token CHAR
%token CONST
%token CONTEXT
%token CUSTOM
%token DEFAULT
%token DOUBLE
%token ENUM
%token EXCEPTION
%token FACTORY
%token FALSE_
%token FIXED
%token FLOAT
%token IN
%token INOUT
%token INTERFACE
%token LOCAL
%token LONG
%token MODULE
%token NATIVE
%token OBJECT
%token OCTET
%token ONEWAY
%token OUT
%token PRIVATE
%token PUBLIC
%token RAISES
%token READONLY
%token SEQUENCE
%token SHORT
%token STRING
%token STRUCT
%token SUPPORTS
%token SWITCH
%token TRUE_
%token TRUNCATABLE
%token TYPEDEF
%token UNION
%token UNSIGNED
%token VALUEBASE
%token VALUETYPE
%token VOID
%token WCHAR
%token WSTRING

// Pragmas
%token PRAGMA
%token PRAGMA_PREFIX
%token PRAGMA_ID
%token PRAGMA_VERSION
%token OMNI_PRAGMA
%token END_PRAGMA
%token <string_val> UNKNOWN_PRAGMA_BODY

// Literals
%token <int_literal_val>   INTEGER_LITERAL
%token <char_val>          CHARACTER_LITERAL
%token <wchar_val>         WIDE_CHARACTER_LITERAL
%token <float_literal_val> FLOATING_PT_LITERAL
%token <string_val>        STRING_LITERAL
%token <wstring_val>       WIDE_STRING_LITERAL
%token <fixed_val>         FIXED_PT_LITERAL

// Scope delimiter
%token SCOPE_DELIM

// Operators
%token LEFT_SHIFT
%token RIGHT_SHIFT

// Types of nonterminals
%type <decl_val>                    start
%type <decl_val>                    definition_plus
%type <decl_val>                    definition
%type <module_val>                  module
%type <module_val>                  module_header
%type <decl_val>                    interface
%type <interface_val>               interface_dcl
%type <forward_val>                 forward_dcl
%type <interface_val>               interface_header
%type <decl_val>                    interface_body
%type <decl_val>                    export_star
%type <decl_val>                    export
%type <inheritspec_val>             interface_inheritance_spec_opt
%type <inheritspec_val>             interface_inheritance_spec
%type <inheritspec_val>             interface_inheritance_list
%type <scopedname_val>              interface_name
%type <scopedname_val>              scoped_name
%type <value_base_val>              value
%type <value_forward_val>           value_forward_dcl
%type <int_val>                     abstract_local_opt
%type <value_box_val>               value_box_dcl
%type <value_abs_val>               value_abs_dcl
%type <value_val>                   value_dcl
%type <value_val>                   value_header
%type <valueinheritsupportspec_val> value_inheritance_spec
%type <valueinheritspec_val>        value_value_inheritance_spec
%type <boolean_val>                 truncatable_opt
%type <valueinheritspec_val>        value_inheritance_list
%type <scopedname_val>              value_name
%type <decl_val>                    value_element_star
%type <decl_val>                    value_element
%type <statemember_val>             state_member
%type <ulong_val>                   member_access
%type <factory_val>                 init_dcl
%type <factory_val>                 init_dcl_header
%type <parameter_val>               init_param_decls_opt
%type <parameter_val>               init_param_decls
%type <parameter_val>               init_param_decl
%type <const_val>                   const_dcl
%type <type_val>                    const_type
%type <expr_val>                    const_exp
%type <expr_val>                    or_expr
%type <expr_val>                    xor_expr
%type <expr_val>                    and_expr
%type <expr_val>                    shift_expr
%type <expr_val>                    add_expr
%type <expr_val>                    mult_expr
%type <expr_val>                    unary_expr
%type <char_val>                    unary_operator
%type <expr_val>                    primary_expr
%type <expr_val>                    literal
%type <boolean_val>                 boolean_literal
%type <string_val>                  string_literal_plus
%type <wstring_val>                 wide_string_literal_plus
%type <ulong_val>                   positive_int_const
%type <decl_val>                    type_dcl
%type <typedef_val>                 type_declarator
%type <type_spec_val>               type_spec
%type <type_val>                    simple_type_spec
%type <type_val>                    base_type_spec
%type <type_val>                    template_type_spec
%type <type_val>                    constr_type_spec
%type <declarator_val>              declarators
%type <declarator_val>              declarator
%type <declarator_val>              simple_declarator
%type <declarator_val>              complex_declarator
%type <type_val>                    floating_pt_type
%type <type_val>                    integer_type
%type <type_val>                    signed_int
%type <type_val>                    signed_short_int
%type <type_val> 	      	    signed_long_int
%type <type_val> 		    signed_long_long_int
%type <type_val> 		    unsigned_int
%type <type_val> 		    unsigned_short_int
%type <type_val> 		    unsigned_long_int
%type <type_val> 		    unsigned_long_long_int
%type <type_val> 		    char_type
%type <type_val> 		    wide_char_type
%type <type_val> 		    boolean_type
%type <type_val> 		    octet_type
%type <type_val> 		    any_type
%type <type_val> 		    object_type
%type <struct_val> 		    struct_type
%type <struct_val> 		    struct_header
%type <member_val> 		    member_list
%type <member_val> 		    member
%type <union_val> 		    union_type
%type <union_val> 		    union_header
%type <type_spec_val> 		    switch_type_spec
%type <union_case_val> 		    switch_body
%type <union_case_val> 		    case_plus
%type <union_case_val> 		    case
%type <case_label_val> 		    case_label_plus
%type <case_label_val> 		    case_label
%type <union_case_val> 		    element_spec
%type <enum_val> 		    enum_type
%type <enum_val> 		    enum_header
%type <enumerator_val> 		    enumerator_list
%type <enumerator_val> 		    enumerator
%type <type_val> 		    sequence_type
%type <type_val> 		    string_type
%type <type_val> 		    wide_string_type
%type <declarator_val> 		    array_declarator
%type <array_size_val> 		    fixed_array_size_plus
%type <array_size_val> 		    fixed_array_size
%type <attribute_val> 		    attr_dcl
%type <boolean_val> 		    readonly_opt
%type <declarator_val> 		    simple_declarator_list
%type <exception_val> 		    except_dcl
%type <exception_val> 		    except_header
%type <member_val> 		    member_star
%type <operation_val> 		    op_dcl
%type <operation_val> 		    op_header
%type <boolean_val> 		    op_attribute_opt
%type <boolean_val> 		    op_attribute
%type <type_val> 		    op_type_spec
%type <parameter_val> 		    parameter_dcls
%type <parameter_val> 		    param_dcl_list
%type <parameter_val> 		    param_dcl
%type <int_val> 		    param_attribute
%type <raisesspec_val>		    raises_expr_opt
%type <raisesspec_val> 		    raises_expr
%type <raisesspec_val> 		    scoped_name_list
%type <contextspec_val> 	    context_expr_opt
%type <contextspec_val> 	    context_expr
%type <contextspec_val> 	    string_literal_list
%type <type_val> 		    param_type_spec
%type <type_val> 		    fixed_pt_type
%type <type_val> 		    fixed_pt_const_type
%type <type_val> 		    value_base_type
%type <decl_val>                    constr_forward_decl
%type <string_val> 		    unknown_pragma_body_plus

%%

start:
    /* empty */     { $$ = 0; }
  | definition_plus {
      $$ = $1;
      AST::tree()->setDeclarations($1);
    }
    ;

definition_plus:
    definition { $$ = $1; }
  | definition_plus definition {
      if ($1) { $1->append($2); $$ = $1; }
      else $$ = $2;
    }
    ;

definition:
    type_dcl   ';' { $$ = $1; }
  | const_dcl  ';' { $$ = $1; }
  | except_dcl ';' { $$ = $1; }
  | interface  ';' { $$ = $1; }
  | module     ';' { $$ = $1; }
  | value      ';' { $$ = $1; }
  | pragma         { $$ = 0; }
  | pragma_prefix  { $$ = 0; }
  | error {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in definition");
      $$ = 0;
    }
    ;

module:
    module_header pragmas_opt '{' definition_plus '}' {
      $1->finishConstruction($4);
      $$ = $1;
    }
  | module_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in module definition");
    } '{' definition_plus '}' {
      $1->finishConstruction($5);
      $$ = $1;
    }
  | module_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in module definition (no body found)");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

module_header:
    MODULE IDENTIFIER { $$ = new Module(currentFile, yylineno, mainFile, $2); }
    ;

interface:
    interface_dcl { $$ = $1; }
  | forward_dcl   { $$ = $1; }
    ;

interface_dcl:
    interface_header '{' interface_body '}' {
      $1->finishConstruction($3);
      $$ = $1;
    }
  | interface_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in interface definition");
    } '{' interface_body '}' {
      $1->finishConstruction($5);
      $$ = $1;
    }
  | interface_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in interface definition (no body found)");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

forward_dcl:
    abstract_local_opt INTERFACE IDENTIFIER {
      $$ = new Forward(currentFile, yylineno, mainFile, $3, $1==1, $1==2);
    }
    ;

interface_header:
    abstract_local_opt INTERFACE IDENTIFIER pragmas_opt
        interface_inheritance_spec_opt {
      $$ = new Interface(currentFile, yylineno, mainFile,
			 $3, $1==1, $1==2, $5);
    }
    ;

abstract_local_opt:
    /* empty */ { $$ = 0; }
  | ABSTRACT    { $$ = 1; }
  | LOCAL       { $$ = 2; }
    ;

interface_body:
    export_star { $$ = $1; }
    ;

export_star:
    /* empty */ { $$ = 0; }
  | export_star export {
      if ($1) { $1->append($2); $$ = $1; }
      else $$ = $2;
    }
    ;

export:
    type_dcl   ';' { $$ = $1; }
  | const_dcl  ';' { $$ = $1; }
  | except_dcl ';' { $$ = $1; }
  | attr_dcl   ';' { $$ = $1; }
  | op_dcl     ';' { $$ = $1; }
  | pragma         { $$ = 0; }
  | error {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in interface body");
      $$ = 0;
    }
    ;

interface_inheritance_spec_opt:
    /* empty */                { $$ = 0; }
  | interface_inheritance_spec { $$ = $1; }
    ;

interface_inheritance_spec:
    ':' interface_inheritance_list { $$ = $2; }
    ;

interface_inheritance_list:
    interface_name pragmas_opt {
      $$ = new InheritSpec($1, currentFile, yylineno);
      if (!$$->interface()) {
	delete $$;
	$$ = 0;
      }
    }
  | interface_inheritance_list ',' pragmas_opt interface_name pragmas_opt {
      if ($1) {
	$1->append(new InheritSpec($4, currentFile, yylineno),
		   currentFile, yylineno);
	$$ = $1;
      }
      else $$ = new InheritSpec($4, currentFile, yylineno);
    }
  | error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in inheritance list");
      $$ = 0;
    }
    ;

interface_name:
    scoped_name { $$ = $1; }
    ;

scoped_name:
    IDENTIFIER {
      $$ = new ScopedName($1, 0);
    }
  | SCOPE_DELIM IDENTIFIER {
      $$ = new ScopedName($2, 1);
    }
  | scoped_name SCOPE_DELIM IDENTIFIER {
      $1->append($3);
      $$=$1;
    }
    ;

/* The obvious way to specify valuetypes isn't LALR(1), but the
   following is:
*/

value:
    value_dcl         { $$ = $1; }
  | value_abs_dcl     { $$ = $1; }
  | value_box_dcl     { $$ = $1; }
  | value_forward_dcl { $$ = $1; }
    ;

value_forward_dcl:
    VALUETYPE IDENTIFIER {
      $$ = new ValueForward(currentFile, yylineno, mainFile, 0, $2);
    }
  | ABSTRACT VALUETYPE IDENTIFIER {
      $$ = new ValueForward(currentFile, yylineno, mainFile, 1, $3);
    }
    ;

value_box_dcl:
    VALUETYPE IDENTIFIER type_spec {
      $$ = new ValueBox(currentFile, yylineno, mainFile,
			$2, $3->type(), $3->constr());
      delete $3;
    }
    ;

value_abs_dcl:
    ABSTRACT VALUETYPE IDENTIFIER {
      valueabs_hack = new ValueAbs(currentFile, yylineno, mainFile, $3, 0, 0);
    } '{' export_star '}' {
      valueabs_hack->finishConstruction($6);
      $$ = valueabs_hack;
      valueabs_hack = 0;
    }
  | ABSTRACT VALUETYPE IDENTIFIER value_inheritance_spec {
      valueabs_hack = new ValueAbs(currentFile, yylineno, mainFile, $3,
				   $4->inherits(), $4->supports());
      delete $4;
    } '{' export_star '}' {
      valueabs_hack->finishConstruction($7);
      $$ = valueabs_hack;
      valueabs_hack = 0;
    }
  | error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in abstract valuetype");
      if (valueabs_hack) {
	valueabs_hack->finishConstruction(0);
	$$ = valueabs_hack;
	valueabs_hack = 0;
      }
      else $$ = 0;
    }
    ;

value_dcl:
    value_header '{' value_element_star '}' {
      $1->finishConstruction($3);
      $$ = $1;
    }
    ;

value_header:
    VALUETYPE IDENTIFIER value_inheritance_spec {
      $$ = new Value(currentFile, yylineno, mainFile, 0, $2,
		     $3->inherits(), $3->supports());
      delete $3;
    }
  | CUSTOM VALUETYPE IDENTIFIER value_inheritance_spec {
      $$ = new Value(currentFile, yylineno, mainFile, 1, $3,
		     $4->inherits(), $4->supports());
      delete $4;
    }
  | VALUETYPE IDENTIFIER {
      $$ = new Value(currentFile, yylineno, mainFile, 0, $2, 0, 0);
    }
  | CUSTOM VALUETYPE IDENTIFIER {
      $$ = new Value(currentFile, yylineno, mainFile, 1, $3, 0, 0);
    }
    ;

value_inheritance_spec:
    ':' value_value_inheritance_spec SUPPORTS interface_inheritance_list {
      $$ = new ValueInheritSupportSpec($2, $4);
    }
  | ':' value_value_inheritance_spec {
      $$ = new ValueInheritSupportSpec($2, 0);
    }
  | SUPPORTS interface_inheritance_list {
      $$ = new ValueInheritSupportSpec(0, $2);
    }
    ;

value_value_inheritance_spec:
    truncatable_opt value_inheritance_list {
      if ($1) $2->setTruncatable();
      $$ = $2;
    }
    ;

truncatable_opt:
    /* empty */ { $$ = 0; }
  | TRUNCATABLE { $$ = 1; }
    ;

value_inheritance_list:
    value_name {
      $$ = new ValueInheritSpec($1, currentFile, yylineno);
      if (!$$->value()) {
	delete $$;
	$$ = 0;
      }
    }
  | value_inheritance_list ',' value_name {
      if ($1) {
	$1->append(new ValueInheritSpec($3, currentFile, yylineno),
		   currentFile, yylineno);
	$$ = $1;
      }
      else $$ = new ValueInheritSpec($3, currentFile, yylineno);
    }
    ;

value_name:
    scoped_name { $$ = $1; }
    ;

value_element_star:
    /* empty */ { $$ = 0; }
  | value_element_star value_element {
      if ($1) { $1->append($2); $$ = $1; }
      else $$ = $2;
    }
    ;

value_element:
    export       { $$ = $1; }
  | state_member { $$ = $1; }
  | init_dcl     { $$ = $1; }
    ;

state_member:
    member_access type_spec declarators ';' {
      $$ = new StateMember(currentFile, yylineno, mainFile,
			   $1, $2->type(), $2->constr(), $3);
      delete $2;
    }
    ;

member_access:
    PUBLIC  { $$ = 0; }
  | PRIVATE { $$ = 1; }
    ;

init_dcl:
    init_dcl_header '(' init_param_decls_opt ')' {
      $1->closeParens();
    } ';' {
      $1->finishConstruction($3);
      $$ = $1;
    }
  | init_dcl_header '(' error ')' {
      $1->closeParens();
    } ';' {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in factory parameters");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

init_dcl_header:
    FACTORY IDENTIFIER {
      $$ = new Factory(currentFile, yylineno, mainFile, $2);
    }
    ;

init_param_decls_opt:
    /* empty */      { $$ = 0; }
  | init_param_decls { $$ = $1; }
    ;

init_param_decls:
    init_param_decl { $$ = $1; }
  | init_param_decls ',' init_param_decl {
      if ($1) { $1->append($3); $$ = $1; }
      else $$ = $3;
    }
    ;

init_param_decl:
    IN param_type_spec IDENTIFIER {
      $$ = new Parameter(currentFile, yylineno, mainFile, 0, $2, $3);
    }
    ;

// End of ValueType nastiness

const_dcl:
    CONST const_type IDENTIFIER '=' const_exp {
      $$ = new Const(currentFile, yylineno, mainFile, $2, $3, $5);
    }
    ;

const_type:
    integer_type        { $$ = $1; }
  | char_type           { $$ = $1; }
  | wide_char_type      { $$ = $1; }
  | boolean_type        { $$ = $1; }
  | floating_pt_type    { $$ = $1; }
  | string_type         { $$ = $1; }
  | wide_string_type    { $$ = $1; }
  | fixed_pt_const_type { $$ = $1; }
  | scoped_name {
      $$ = IdlType::scopedNameToType(currentFile, yylineno, $1);
    }
  | octet_type          { $$ = $1; }
    ;

const_exp:
    or_expr { $$ = $1; }
    ;

or_expr:
    xor_expr             { $$ = $1; }
  | or_expr '|' xor_expr { $$ = new OrExpr(currentFile, yylineno, $1, $3); }
    ;

xor_expr:
    and_expr              { $$ = $1; }
  | xor_expr '^' and_expr { $$ = new XorExpr(currentFile, yylineno, $1, $3); }
    ;

and_expr:
    shift_expr              { $$ = $1; }
  | and_expr '&' shift_expr {
      $$ = new AndExpr(currentFile, yylineno, $1, $3);
    }
    ;

shift_expr:
    add_expr                        { $$ = $1; }
  | shift_expr RIGHT_SHIFT add_expr {
    $$ = new RShiftExpr(currentFile, yylineno, $1, $3);
  }
  | shift_expr LEFT_SHIFT  add_expr {
    $$ = new LShiftExpr(currentFile, yylineno, $1, $3);
  }
    ;

add_expr:
    mult_expr              { $$ = $1; }
  | add_expr '+' mult_expr { $$ = new AddExpr(currentFile, yylineno, $1, $3); }
  | add_expr '-' mult_expr { $$ = new SubExpr(currentFile, yylineno, $1, $3); }
    ;

mult_expr:
    unary_expr               { $$ = $1; }
  | mult_expr '*' unary_expr {
      $$ = new MultExpr(currentFile, yylineno, $1, $3);
    }
  | mult_expr '/' unary_expr {
      $$ = new DivExpr(currentFile, yylineno, $1, $3);
    }
  | mult_expr '%' unary_expr {
      $$ = new ModExpr(currentFile, yylineno, $1, $3);
    }
    ;

unary_expr:
    unary_operator primary_expr {
      if ($1 == '-') $$ = new MinusExpr(currentFile, yylineno, $2);
      if ($1 == '+') $$ = new PlusExpr(currentFile, yylineno, $2);
      if ($1 == '~') $$ = new InvertExpr(currentFile, yylineno, $2);
    }
  | primary_expr { $$ = $1; }
    ;

unary_operator:
    '-' { $$ = '-'; }
  | '+' { $$ = '+'; }
  | '~' { $$ = '~'; }
    ;

primary_expr:
    scoped_name {
      $$ = IdlExpr::scopedNameToExpr(currentFile, yylineno, $1);
    }
  | literal           { $$ = $1; }
  | '(' const_exp ')' { $$ = $2; }
    ;

literal:
    INTEGER_LITERAL {
      $$ = new IntegerExpr(currentFile, yylineno, $1);
    }
  | string_literal_plus {
      $$ = new StringExpr(currentFile, yylineno, $1);
    }
  | wide_string_literal_plus {
      $$ = new WStringExpr(currentFile, yylineno, $1);
    }
  | CHARACTER_LITERAL {
      $$ = new CharExpr(currentFile, yylineno, $1);
    }
  | WIDE_CHARACTER_LITERAL {
      $$ = new WCharExpr(currentFile, yylineno, $1);
    }
  | FIXED_PT_LITERAL {
      $$ = new FixedExpr(currentFile, yylineno, $1);
    }
  | FLOATING_PT_LITERAL {
      $$ = new FloatExpr(currentFile, yylineno, $1);
    }
  | boolean_literal {
      $$ = new BooleanExpr(currentFile, yylineno, $1);
    }
    ;

string_literal_plus:
    STRING_LITERAL                     { $$ = $1; }
  | string_literal_plus STRING_LITERAL {
      $$ = new char [strlen($1) + strlen($2) + 1];
      strcpy($$, $1);
      strcat($$, $2);
      delete [] $1;
      delete [] $2;
    }
    ;

wide_string_literal_plus:
    WIDE_STRING_LITERAL { $$ = $1; }
  | wide_string_literal_plus WIDE_STRING_LITERAL {
      $$ = new IDL_WChar [idl_wstrlen($1) + idl_wstrlen($2) + 1];
      idl_wstrcpy($$, $1);
      idl_wstrcat($$, $2);
      delete [] $1;
      delete [] $2;
    }
    ;

boolean_literal:
    TRUE_  { $$ = 1; }
  | FALSE_ { $$ = 0; }
    ;

positive_int_const:
    const_exp {
      $$ = $1->evalAsULong();
      if ($$ < 1)
	IdlError(currentFile, yylineno, "Size must be at least 1");
    }
    ;

type_dcl:
    TYPEDEF type_declarator  { $$ = $2; }
  | struct_type              { $$ = $1; }
  | union_type               { $$ = $1; }
  | enum_type                { $$ = $1; }
  | NATIVE IDENTIFIER {
      $$ = new Native(currentFile, yylineno, mainFile, $2);
    }
  | constr_forward_decl      { $$ = $1; }
    ;

type_declarator:
    type_spec declarators {
      $$ = new Typedef(currentFile, yylineno, mainFile,
		       $1->type(), $1->constr(), $2);
      delete $1;
    }
    ;

type_spec:
    simple_type_spec { $$ = new TypeSpec($1, 0); }
  | constr_type_spec { $$ = new TypeSpec($1, 1); }
    ;

simple_type_spec:
    base_type_spec     { $$ = $1; }
  | template_type_spec { $$ = $1; }
  | scoped_name {
      $$ = IdlType::scopedNameToType(currentFile, yylineno, $1);
    }
    ;

base_type_spec:
    floating_pt_type { $$ = $1; }
  | integer_type     { $$ = $1; }
  | char_type        { $$ = $1; }
  | wide_char_type   { $$ = $1; }
  | boolean_type     { $$ = $1; }
  | octet_type       { $$ = $1; }
  | any_type         { $$ = $1; }
  | object_type      { $$ = $1; }
  | value_base_type  { $$ = $1; }
    ;

template_type_spec:
    sequence_type    { $$ = $1; }
  | string_type      { $$ = $1; }
  | wide_string_type { $$ = $1; }
  | fixed_pt_type    { $$ = $1; }
    ;

constr_type_spec:
    struct_type { $$ = $1->thisType(); }
  | union_type  { $$ = $1->thisType(); }
  | enum_type   { $$ = $1->thisType(); }
    ;

declarators:
    declarator pragmas_opt { $$ = $1; }
  | declarators ',' pragmas_opt declarator pragmas_opt {
      if ($1) { $1->append($4); $$ = $1; }
      else $$ = $4;
    }
    ;

declarator:
    simple_declarator  { $$ = $1; }
  | complex_declarator { $$ = $1; }
    ;

simple_declarator:
    IDENTIFIER {
      $$ = new Declarator(currentFile, yylineno, mainFile, $1, 0);
    }
    ;

complex_declarator:
    array_declarator { $$ = $1; }
    ;

floating_pt_type:
    FLOAT       { $$ = BaseType::floatType; }
  | DOUBLE      { $$ = BaseType::doubleType; }
  | LONG DOUBLE { $$ = BaseType::longdoubleType; }
    ;

integer_type:
    signed_int   { $$ = $1; }
  | unsigned_int { $$ = $1; }
    ;

signed_int:
    signed_short_int     { $$ = $1; }
  | signed_long_int      { $$ = $1; }
  | signed_long_long_int { $$ = $1; }
    ;

signed_short_int:
    SHORT { $$ = BaseType::shortType; }
    ;

signed_long_int:
    LONG { $$ = BaseType::longType; }
    ;

signed_long_long_int:
    LONG LONG { $$ = BaseType::longlongType; }
    ;

unsigned_int:
    unsigned_short_int     { $$ = $1; }
  | unsigned_long_int      { $$ = $1; }
  | unsigned_long_long_int { $$ = $1; }
    ;

unsigned_short_int:
    UNSIGNED SHORT { $$ = BaseType::ushortType; }
    ;

unsigned_long_int:
    UNSIGNED LONG { $$ = BaseType::ulongType; }
    ;

unsigned_long_long_int:
    UNSIGNED LONG LONG { $$ = BaseType::ulonglongType; }
    ;

char_type:
    CHAR { $$ = BaseType::charType; }
    ;

wide_char_type:
    WCHAR { $$ = BaseType::wcharType; }
    ;

boolean_type:
    BOOLEAN { $$ = BaseType::booleanType; }
    ;

octet_type:
    OCTET { $$ = BaseType::octetType; }
    ;

any_type:
    ANY { $$ = BaseType::anyType; }
    ;

object_type:
    OBJECT { $$ = DeclaredType::corbaObjectType; }
    ;

struct_type:
    struct_header pragmas_opt '{' pragmas_opt member_list '}' {
      $1->finishConstruction($5);
      $$ = $1;
    }
  | struct_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in struct definition");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

struct_header:
    STRUCT IDENTIFIER {
      $$ = new Struct(currentFile, yylineno, mainFile, $2);
    }
    ;

member_list:
    member pragmas_opt { $$ = $1; }
  | member_list member pragmas_opt {
      if ($1) { $1->append($2); $$ = $1; }
      else $$ = $2;
    }
    ;

member:
    type_spec declarators ';' {
      $$ = new Member(currentFile, yylineno, mainFile,
		      $1->type(), $1->constr(), $2);
      delete $1;
    }
  | error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in member declaration");
      $$ = 0;
    }
    ;

union_type:
    union_header pragmas_opt SWITCH
        '(' pragmas_opt switch_type_spec pragmas_opt ')'
        pragmas_opt '{' pragmas_opt switch_body '}' {

      $1->finishConstruction($6->type(), $6->constr(), $12);
      delete $6;
      $$ = $1;
    }
  | union_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in union declaration");
      $1->finishConstruction(0, 0, 0);
      $$ = $1;
    }
    ;

union_header:
    UNION IDENTIFIER {
      $$ = new Union(currentFile, yylineno, mainFile, $2);
    }
    ;

switch_type_spec:
    integer_type { $$ = new TypeSpec($1, 0); }
  | char_type    { $$ = new TypeSpec($1, 0); }
  | boolean_type { $$ = new TypeSpec($1, 0); }
  | enum_type    { $$ = new TypeSpec($1->thisType(), 1); }
  | scoped_name {
      $$ = new TypeSpec(IdlType::scopedNameToType(currentFile, yylineno, $1),
			0);
    }
    ;

switch_body:
    case_plus { $$ = $1; }
    ;

case_plus:
    case pragmas_opt { $$ = $1; }
  | case_plus case pragmas_opt {
      $1->append($2);
      $$ = $1;
    }
    ;

case:
    case_label_plus element_spec ';' {
      $2->finishConstruction($1);
      $$ = $2;
    }
    ;

case_label_plus:
    case_label { $$ = $1; }
  | case_label_plus case_label {
      $1->append($2);
      $$ = $1;
    }
    ;

case_label:
    CASE const_exp ':' pragmas_opt {
      $$ = new CaseLabel(currentFile, yylineno, mainFile, $2);
    }
  | DEFAULT ':' pragmas_opt {
      $$ = new CaseLabel(currentFile, yylineno, mainFile, 0);
    }
    ;

element_spec:
    type_spec declarator {
      $$ = new UnionCase(currentFile, yylineno, mainFile,
			 $1->type(), $1->constr(), $2);
    }
    ;

enum_type:
    enum_header pragmas_opt '{' pragmas_opt enumerator_list '}' {
      $1->finishConstruction($5);
      $$ = $1;
    }
  | enum_header error {
      IdlSyntaxError(currentFile, yylineno, "Syntax error in enum definition");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

enum_header:
    ENUM IDENTIFIER {
      $$ = new Enum(currentFile, yylineno, mainFile, $2);
    }
    ;

enumerator_list:
    enumerator pragmas_opt { $$ = $1; }
  | enumerator_list ',' pragmas_opt enumerator pragmas_opt {
      $1->append($4);
      $$ = $1;
    }
    ;

enumerator:
    IDENTIFIER {
      $$ = new Enumerator(currentFile, yylineno, mainFile, $1);
    }
    ;

sequence_type:
    SEQUENCE '<' simple_type_spec ',' positive_int_const '>' {
      $$ = new SequenceType($3, $5);
    }
  | SEQUENCE '<' simple_type_spec '>' {
      $$ = new SequenceType($3, 0);
    }
    ;

string_type:
    STRING '<' positive_int_const '>' { $$ = new StringType($3); }
  | STRING {
      $$ = StringType::unboundedStringType;
    }
    ;

wide_string_type:
    WSTRING '<' positive_int_const '>' { $$ = new WStringType($3); }
  | WSTRING {
      $$ = WStringType::unboundedWStringType;
    }
    ;

array_declarator:
    IDENTIFIER fixed_array_size_plus {
      $$ = new Declarator(currentFile, yylineno, mainFile, $1, $2);
    }
    ;

fixed_array_size_plus:
    fixed_array_size { $$ = $1; }
  | fixed_array_size_plus fixed_array_size {
      $1->append($2);
      $$ = $1;
    }
    ;

fixed_array_size:
    '[' positive_int_const ']' { $$ = new ArraySize($2); }
    ;

attr_dcl:
    readonly_opt ATTRIBUTE param_type_spec simple_declarator_list {
      $$ = new Attribute(currentFile, yylineno, mainFile, $1, $3, $4);
    }
    ;

readonly_opt:
    /* empty */ { $$ = 0; }
  | READONLY    { $$ = 1; }
    ;

simple_declarator_list:
    simple_declarator pragmas_opt { $$ = $1; }
  | simple_declarator_list ',' pragmas_opt simple_declarator pragmas_opt {
      if ($1) { $1->append($4); $$ = $1; }
      else $$ = $4;
    }
    ;

except_dcl:
    except_header pragmas_opt '{' pragmas_opt member_star '}' {
      $1->finishConstruction($5);
      $$ = $1;
    }
  | except_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in exception definition");
      $1->finishConstruction(0);
      $$ = $1;
    }
    ;

except_header:
    EXCEPTION IDENTIFIER {
      $$ = new Exception(currentFile, yylineno, mainFile, $2);
    }
    ;

member_star:
    /* empty */ { $$ = 0; }
  | member_star member {
      if ($1) { $1->append($2);	$$ = $1; }
      else $$ = $2;
    }
    ;

op_dcl:
    op_header pragmas_opt parameter_dcls {
      $1->closeParens();
    } pragmas_opt raises_expr_opt context_expr_opt {
      $1->finishConstruction($3, $6, $7);
      $$ = $1;
    }
  | op_header error {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in operation declaration");
      $1->closeParens();
      $1->finishConstruction(0, 0, 0);
      $$ = $1;
    }
    ;

op_header:
    op_attribute_opt op_type_spec IDENTIFIER {
      $$ = new Operation(currentFile, yylineno, mainFile, $1, $2, $3);
    }
    ;

op_attribute_opt:
    /* empty */  { $$ = 0; }
  | op_attribute { $$ = $1; }
    ;

op_attribute:
    ONEWAY { $$ = 1; }
    ;

op_type_spec:
    param_type_spec { $$ = $1; }
  | VOID            { $$ = BaseType::voidType; }
    ;

parameter_dcls:
    '(' pragmas_opt param_dcl_list ')' { $$ = $3; }
  | '(' pragmas_opt ')'                { $$ = 0; }
  | '(' error ')' {
      IdlSyntaxError(currentFile, yylineno,
		     "Syntax error in operation parameters");
      $$ = 0;
    }
    ;

param_dcl_list:
    param_dcl pragmas_opt { $$ = $1; }
  | param_dcl_list ',' pragmas_opt param_dcl pragmas_opt {
      if ($1) { $1->append($4); $$ = $1; }
      else $$ = $4;
    }
    ;

param_dcl:
    param_attribute param_type_spec IDENTIFIER {
      $$ = new Parameter(currentFile, yylineno, mainFile, $1, $2, $3);
    }
    ;

param_attribute:
    IN    { $$ = 0; }
  | OUT   { $$ = 1; }
  | INOUT { $$ = 2; }
    ;

raises_expr_opt:
    /* empty */             { $$ = 0; }
  | raises_expr pragmas_opt { $$ = $1; }
    ;

raises_expr:
    RAISES '(' scoped_name_list ')' { $$ = $3; }
    ;

scoped_name_list:
    scoped_name pragmas_opt {
      $$ = new RaisesSpec($1, currentFile, yylineno);
    }
  | scoped_name_list ',' pragmas_opt scoped_name pragmas_opt {
      $1->append(new RaisesSpec($4, currentFile, yylineno));
      $$ = $1;
    }
    ;

context_expr_opt:
    /* empty */              { $$ = 0; }
  | context_expr pragmas_opt { $$ = $1; }
    ;

context_expr:
    CONTEXT '(' string_literal_list ')' { $$ = $3; }
    ;

string_literal_list:
    string_literal_plus pragmas_opt {
      $$ = new ContextSpec($1, currentFile, yylineno);
    }
  | string_literal_list ',' pragmas_opt string_literal_plus pragmas_opt {
      $1->append(new ContextSpec($4, currentFile, yylineno));
      $$ = $1;
    }
    ;

param_type_spec:
    base_type_spec   { $$ = $1; }
  | string_type      { $$ = $1; }
  | wide_string_type { $$ = $1; }
  | scoped_name      {
      $$ = IdlType::scopedNameToType(currentFile, yylineno, $1);
    }
    ;

fixed_pt_type:
    FIXED '<' positive_int_const ',' const_exp '>' {
      IDL_ULong scale = $5->evalAsULong();

      if ($3 > 31) {
	IdlError(currentFile, yylineno,
		 "Fixed point values may not have more than 31 digits");
      }
      if (scale > $3) {
	IdlError(currentFile, yylineno,
		 "Fixed point scale factor is greater than "
		 "the number of digits");
      }
      $$ = new FixedType($3, scale);
    }
    ;

fixed_pt_const_type:
    FIXED {
      $$ = new FixedType(0, 0);
    }
    ;

value_base_type:
    VALUEBASE { $$ = new DeclaredType(IdlType::tk_value, 0, 0); }
    ;

constr_forward_decl:
    STRUCT IDENTIFIER {
      $$ = new StructForward(currentFile, yylineno, mainFile, $2);
    }
  | UNION IDENTIFIER {
      $$ = new UnionForward(currentFile, yylineno, mainFile, $2);
    }
    ;

pragma:
    pragma_id
  | pragma_version
  | unknown_pragma
  | omni_pragma
    ;

pragmas:
    pragma
  | pragmas pragma
    ;

pragmas_opt:
    /* empty */
  | pragmas
    ;

pragma_prefix:
    PRAGMA_PREFIX string_literal_plus END_PRAGMA {
      Prefix::setPrefix(idl_strdup($2));
    }
  | PRAGMA_PREFIX error {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma prefix");
    }
    ;

pragma_id:
    PRAGMA_ID scoped_name string_literal_plus END_PRAGMA {
      Decl* d = Decl::scopedNameToDecl(currentFile, yylineno, $2);
      if (d) DeclRepoId::setRepoId(d, $3, currentFile, yylineno);
    }
  | PRAGMA_ID error END_PRAGMA {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma id");
    }
    ;

pragma_version:
    PRAGMA_VERSION scoped_name INTEGER_LITERAL '.' INTEGER_LITERAL END_PRAGMA {
      Decl* d = Decl::scopedNameToDecl(currentFile, yylineno, $2);
      if (d) DeclRepoId::setVersion(d, $3, $5, currentFile, yylineno);
    }
  | PRAGMA_VERSION error END_PRAGMA {
      IdlSyntaxError(currentFile, yylineno, "Malformed #pragma version");
    }
    ;

unknown_pragma:
    PRAGMA unknown_pragma_body_plus END_PRAGMA {
      Pragma::add($2, currentFile, yylineno-1);
    }
    ;

omni_pragma:
    OMNI_PRAGMA unknown_pragma_body_plus END_PRAGMA {
      Pragma::add($2, currentFile, yylineno);
    }
    ;

unknown_pragma_body_plus:
    UNKNOWN_PRAGMA_BODY { $$ = $1; }
  | unknown_pragma_body_plus UNKNOWN_PRAGMA_BODY {
      $$ = new char [strlen($1) + strlen($2) + 1];
      strcpy($$, $1);
      strcat($$, $2);
      delete [] $1;
      delete [] $2;
    }
    ;

%%
