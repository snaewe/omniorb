// -*- c++ -*-
//                          Package   : omniidl
// idlast.cc                Created on: 1999/10/20
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
//   Abstract Syntax Tree objects and support functions

// $Id$
// $Log$
// Revision 1.6  1999/11/01 20:19:57  dpg1
// Support for union switch types declared inside the switch statement.
//
// Revision 1.5  1999/11/01 16:38:40  dpg1
// Missed an update when adding recursive union detection.
//
// Revision 1.4  1999/11/01 10:05:00  dpg1
// New file attribute to AST.
//
// Revision 1.3  1999/10/29 15:42:43  dpg1
// DeclaredType() now takes extra DeclRepoId* argument.
// Code to detect recursive structs and unions.
//
// Revision 1.2  1999/10/29 10:00:43  dpg1
// Added code to find a value for the default case in a union.
//
// Revision 1.1  1999/10/27 14:05:59  dpg1
// *** empty log message ***
//

#include <idlast.h>
#include <idlrepoId.h>
#include <idlvalidate.h>
#include <idlerr.h>

#include <ctype.h>

// Globals from lexer
extern FILE*       yyin;
extern const char* currentFile;
extern int         yylineno;

AST   AST::tree_;
Decl* Decl::mostRecent_;

// AST
AST::AST() : declarations_(0), file_(0) {}
AST::~AST() { if (declarations_) delete declarations_; }

_CORBA_Boolean
AST::
process(FILE* f, const char* name)
{
  Scope::init();

  yyin        = f;
  currentFile = idl_strdup(name);
  Prefix::newFile();

  tree_.setFile(name);

  int yr = yyparse();
  if (yr) IdlError(currentFile, yylineno, "Syntax error");

  return IdlReportErrors();
}

void
AST::
clear()
{
  if (declarations_) delete declarations_;
  Scope::clear();
  declarations_ = 0;
}

void
AST::
setFile(const char* file)
{
  if (file_) {
    if (!strcmp(file_, file)) return;
    delete [] file_;
  }
  file_ = idl_strdup(file);
}

void
AST::
setDeclarations(Decl* d)
{
  assert(!declarations_);
  declarations_ = d;

  // Validate the declarations
  AstValidateVisitor v;
  accept(v);
}


// Base Decl
Decl::
Decl(Kind kind, const char* file, int line, _CORBA_Boolean mainFile)

  : kind_(kind), file_(idl_strdup(file)), line_(line),
    mainFile_(mainFile), inScope_(Scope::current()),
    pragmas_(0), lastPragma_(0), next_(0)
{
  last_       = this;
  mostRecent_ = this;
}

Decl::
~Decl()
{
  if (file_) delete [] file_;
  if (next_) delete next_;
}

Decl*
Decl::
scopedNameToDecl(const char* file, int line, const ScopedName* sn)
{
  const Scope::Entry* se = Scope::current()->findScopedName(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL) {
      return se->decl();
    }
    const char* ssn = sn->toString();
    IdlError(file, line, "`%s' is not a declaration", ssn);
    IdlErrorCont(se->file(), se->line(), "(`%s' created here)", ssn);
    delete [] ssn;
  }
  return 0;
}

void
Decl::
addPragma(const char* pragmaText)
{
  Pragma* p = new Pragma(pragmaText);
  if (pragmas_)
    lastPragma_->next_ = p;
  else
    pragmas_ = p;
  lastPragma_ = p;
}


// Module
Module::
Module(const char* file, int line, _CORBA_Boolean mainFile,
       const char* identifier)

  : Decl(D_MODULE, file, line, mainFile),
    DeclRepoId(identifier),
    definitions_(0)
{
  Scope* s = Scope::current()->newModuleScope(identifier, file, line);
  Scope::current()->addModule(identifier, s, this, file, line);
  Scope::startScope(s);
  Prefix::newScope(identifier);
  //  cout << "Module " << identifier << " created" << endl;
}

Module::
~Module()
{
  if (definitions_) delete definitions_;
}

void
Module::
finishConstruction(Decl* defs)
{
  definitions_ = defs;
  Prefix::endScope();
  Scope::endScope();
  //  cout << "\nModule " << identifier() << " finished." << endl;
}


// Interface
InheritSpec::
InheritSpec(const ScopedName* sn, const char* file, int line)

  : interface_(0), decl_(0), scope_(0), next_(0)
{
  const Scope::Entry* se = Scope::current()->findScopedName(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL) {

      decl_      = se->decl();
      IdlType* t = se->idltype();

      while (t && t->kind() == IdlType::tk_alias)
	t = ((Declarator*)((DeclaredType*)t)->decl())->alias()->aliasType();

      if (!t) return;

      if (t->kind() == IdlType::tk_objref ||
	  t->kind() == IdlType::tk_abstract_interface) {

	Decl* d = ((DeclaredType*)t)->decl();

	if (d->kind() == Decl::D_INTERFACE) {
	  interface_ = (Interface*)d;
	  scope_     = interface_->scope();
	  return;
	}
	else if (d->kind() == Decl::D_FORWARD) {
	  const char* ssn = ((Forward*)d)->scopedName()->toString();
	  IdlError(file, line,
		   "Inherited interface `%s' must be fully defined", ssn);

	  if (decl_ != d) {
	    const char* tssn = sn->toString();
	    IdlErrorCont(se->file(), se->line(),
			 "(`%s' reached through typedef `%s')",
			 ssn, tssn);
	    delete [] tssn;
	  }
	  IdlErrorCont(d->file(), d->line(),
		       "(`%s' forward declared here)", ssn);
	  delete [] ssn;
	  return;
	}
      }
    }
    const char* ssn = sn->toString();
    IdlError(file, line,
	     "`%s' used in inheritance specification is not an interface",
	     ssn);
    IdlErrorCont(se->file(), se->line(), "(`%s' declared here)", ssn);
    delete [] ssn;
  }
}

void
InheritSpec::
append(InheritSpec* is, const char* file, int line)
{
  InheritSpec *i, *last;

  for (i=this; i; i = i->next_) {
    last = i;
    if (is->interface() == i->interface()) {
      const char* ssn = is->interface()->scopedName()->toString();
      IdlError(file, line,
	       "Cannot specify `%s' as a direct base interface "
	       "more than once", ssn);
      delete [] ssn;
      delete is;
      return;
    }
  }
  last->next_ = is;
}

Interface::
Interface(const char* file, int line, _CORBA_Boolean mainFile,
	  const char* identifier, _CORBA_Boolean abstract,
	  InheritSpec* inherits)

  : Decl(D_INTERFACE, file, line, mainFile),
    DeclRepoId(identifier),
    abstract_(abstract),
    inherits_(inherits),
    contents_(0)
{
  // Look for forward interface
  Scope::Entry* se = Scope::current()->find(identifier);

  if (se &&
      se->kind() == Scope::Entry::E_DECL &&
      se->decl()->kind() == Decl::D_FORWARD) {

    Forward* f = (Forward*)se->decl();

    if (strcmp(f->prefix(), prefix())) {

      IdlError(file, line,
	       "In declaration of interface `%s', repository id "
	       "prefix `%s' differs from that of forward declaration",
	       identifier, prefix());

      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here with prefix `%s')",
		   f->identifier(), f->prefix());
    }
    if (abstract && !f->abstract()) {
      IdlError(file, line,
	       "Declaration of abstract interface `%s' conflicts with "
	       "forward declaration as non-abstract", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as non-abstract here)");
    }
    else if (!abstract && f->abstract()) {
      IdlError(file, line,
	       "Declaration of non-abstract interface `%s' conflicts with "
	       "forward declaration as abstract", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as abstract here)");
    }
    if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());

    f->setDefinition(this);
    Scope::current()->remEntry(se);
  }
  scope_ = Scope::current()->newInterfaceScope(identifier, file, line);

  if (abstract_) {
    thisType_ = new DeclaredType(IdlType::tk_abstract_interface, this, this);

    // Check that all inherited interfaces are abstract
    for (InheritSpec* inh = inherits; inh; inh = inh->next()) {
      if (!inh->interface()->abstract()) {
	const char* ssn = inh->scope()->scopedName()->toString();
	IdlError(file, line,
		 "In declaration of abstract interface `%s', inherited "
		 "interface `%s' is not abstract", identifier, ssn);
	IdlErrorCont(inh->interface()->file(), inh->interface()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
    }
  }
  else
    thisType_ = new DeclaredType(IdlType::tk_objref, this, this);

  scope_->setInherited(inherits, file, line);
  Scope::current()->addDecl(identifier, scope_, this, thisType_, file, line);
  Scope::startScope(scope_);
  Prefix::newScope(identifier);
  //  cout << "Interface created" << endl;
}

Interface::
~Interface()
{
  if (inherits_) delete inherits_;
  if (contents_) delete contents_;
  delete thisType_;
}

void
Interface::
finishConstruction(Decl* decls)
{
  contents_ = decls;
  Prefix::endScope();
  Scope::endScope();
}


// Forward
Forward::
Forward(const char* file, int line, _CORBA_Boolean mainFile,
	const char* identifier, _CORBA_Boolean abstract)

  : Decl(D_FORWARD, file, line, mainFile),
    DeclRepoId(identifier),
    abstract_(abstract),
    definition_(0),
    firstForward_(0),
    thisType_(0)
{
  Scope::Entry*  se  = Scope::current()->find(identifier);
  _CORBA_Boolean reg = 1;

  if (se && se->kind() == Scope::Entry::E_DECL) {

    if (se->decl()->kind() == D_INTERFACE) {
      Interface* i = (Interface*)se->decl();
      definition_  = i;

      if (strcmp(i->prefix(), prefix())) {

	IdlError(file, line,
		 "In forward declaration of interface `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(i->file(), i->line(),
		     "(`%s' fully declared here with prefix `%s')",
		     i->identifier(), i->prefix());
	reg = 0;
      }
      if (abstract && !i->abstract()) {
	IdlError(file, line,
		 "Forward declaration of abstract interface `%s' conflicts "
		 "with earlier full declaration as non-abstract",
		 identifier);
	IdlErrorCont(i->file(), i->line(),
		     "(`%s' declared as non-abstract here)");
      }
      else if (!abstract && i->abstract()) {
	IdlError(file, line,
		 "Forward declaration of non-abstract interface `%s' "
		 "conflicts with earlier full declaration as abstract",
		 identifier);
	IdlErrorCont(i->file(), i->line(),
		     "(`%s' declared as abstract here)");
      }
      reg = 0;
    }
    else if (se->decl()->kind() == D_FORWARD) {
      Forward* f    = (Forward*)se->decl();
      firstForward_ = f;

      if (strcmp(f->prefix(), prefix())) {

	IdlError(file, line,
		 "In forward declaration of interface `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward declared here with prefix `%s')",
		     f->identifier(), f->prefix());
      }
      if (abstract && !f->abstract()) {
	IdlError(file, line,
		 "Forward declaration of abstract interface `%s' conflicts "
		 "with earlier forward declaration as non-abstract",
		 identifier);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward declared as non-abstract here)");
      }
      else if (!abstract && f->abstract()) {
	IdlError(file, line,
		 "Forward declaration of non-abstract interface `%s' "
		 "conflicts  with earlier forward declaration as abstract",
		 identifier);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward declared as abstract here)");
      }
      if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());
      reg = 0;
    }
  }
  if (reg) {
    if (abstract)
      thisType_ = new DeclaredType(IdlType::tk_abstract_interface, this, this);
    else
      thisType_ = new DeclaredType(IdlType::tk_objref, this, this);

    Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
  }
  //  cout << "Forward created" << endl;
}

Forward::
~Forward()
{
  delete thisType_;
}

Interface*
Forward::
definition() const
{
  if (firstForward_)
    return firstForward_->definition();
  else
    return definition_;
}

void
Forward::
setDefinition(Interface* defn)
{
  definition_ = defn;
}


// Const

Const::
Const(const char* file, int line, _CORBA_Boolean mainFile,
      IdlType* constType, const char* identifier, IdlExpr* expr)

  : Decl(D_CONST, file, line, mainFile),
    DeclRepoId(identifier),
    constType_(constType)
{
  if (constType) delType_ = constType->shouldDelete();
  else           delType_ = 0;

  if (!constType || !expr) return; // Ignore nulls due to earlier errors

  IdlType* t = constType;

  while (t && t->kind() == IdlType::tk_alias)
    t = ((Declarator*)((DeclaredType*)t)->decl())->alias()->aliasType();

  if (!t) { // Broken alias due to earlier error
    constKind_ = IdlType::tk_null;
    delete expr;
    return;
  }
  constKind_ = t->kind();

  switch (constKind_) {
  case IdlType::tk_short:      v_.short_      = expr->evalAsShort();     break;
  case IdlType::tk_long:       v_.long_       = expr->evalAsLong();      break;
  case IdlType::tk_ushort:     v_.ushort_     = expr->evalAsUShort();    break;
  case IdlType::tk_ulong:      v_.ulong_      = expr->evalAsULong();     break;
  case IdlType::tk_float:      v_.float_      = expr->evalAsFloat();     break;
  case IdlType::tk_double:     v_.double_     = expr->evalAsDouble();    break;
  case IdlType::tk_boolean:    v_.boolean_    = expr->evalAsBoolean();   break;
  case IdlType::tk_char:       v_.char_       = expr->evalAsChar();      break;
  case IdlType::tk_octet:      v_.octet_      = expr->evalAsOctet();     break;
  case IdlType::tk_string:     v_.string_     =
				 idl_strdup(expr->evalAsString());       break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong:   v_.longlong_   = expr->evalAsLongLong();  break;
  case IdlType::tk_ulonglong:  v_.ulonglong_  = expr->evalAsULongLong(); break;
#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble: v_.longdouble_ = expr->evalAsLongDouble();break;
#endif
  case IdlType::tk_wchar:      v_.wchar_      = expr->evalAsWChar();     break;
  case IdlType::tk_wstring:    v_.wstring_    =
				 idl_wstrdup(expr->evalAsWString());     break;
  case IdlType::tk_fixed:      v_.fixed_      = expr->evalAsFixed(); break;
  case IdlType::tk_enum:
    v_.enumerator_ = expr->evalAsEnumerator((Enum*)((DeclaredType*)t)->decl());
    break;

  default:
    assert(0);
  }
  delete expr;

  Scope::current()->addDecl(identifier, 0, this, constType, file, line);
}

Const::
~Const()
{
  if (constKind_ == IdlType::tk_string)  delete [] v_.string_;
  if (constKind_ == IdlType::tk_wstring) delete [] v_.wstring_;
  if (delType_) delete constType_;
}

#define CONST_AS(rt, op, tk, un) \
rt Const::op() const { \
  assert(constKind_ == IdlType::tk); \
  return v_.un; \
}

CONST_AS(_CORBA_Short,        constAsShort,      tk_short,      short_)
CONST_AS(_CORBA_Long,         constAsLong,       tk_long,       long_)
CONST_AS(_CORBA_UShort,       constAsUShort,     tk_ushort,     ushort_)
CONST_AS(_CORBA_ULong,        constAsULong,      tk_ulong,      ulong_)
CONST_AS(_CORBA_Float,        constAsFloat,      tk_float,      float_)
CONST_AS(_CORBA_Double,       constAsDouble,     tk_double,     double_)
CONST_AS(_CORBA_Boolean,      constAsBoolean,    tk_boolean,    boolean_)
CONST_AS(_CORBA_Char,         constAsChar,       tk_char,       char_)
CONST_AS(_CORBA_Octet,        constAsOctet,      tk_octet,      octet_)
CONST_AS(const char*,         constAsString,     tk_string,     string_)
#ifdef HAS_LongLong
CONST_AS(_CORBA_LongLong,     constAsLongLong,   tk_longlong,   longlong_)
CONST_AS(_CORBA_ULongLong,    constAsULongLong,  tk_ulonglong,  ulonglong_)
#endif
#ifdef HAS_LongDouble
CONST_AS(_CORBA_LongDouble,   constAsLongDouble, tk_longdouble, longdouble_)
#endif
CONST_AS(_CORBA_WChar,        constAsWChar,      tk_wchar,      wchar_)
CONST_AS(const _CORBA_WChar*, constAsWString,    tk_wstring,    wstring_)
CONST_AS(_CORBA_Fixed,        constAsFixed,      tk_fixed,      fixed_)
CONST_AS(Enumerator*,         constAsEnumerator, tk_enum,       enumerator_)


// Declarator
Declarator::
Declarator(const char* file, int line, _CORBA_Boolean mainFile,
	   const char* identifier, ArraySize* sizes)

  : Decl(D_DECLARATOR, file, line, mainFile),
    DeclRepoId(identifier),
    sizes_(sizes),
    thisType_(0),
    alias_(0)
{
}

Declarator::
~Declarator()
{
  if (sizes_)    delete sizes_;
  if (thisType_) delete thisType_;
}

const char*
Declarator::
kindAsString() const
{
  if (alias_) return "typedef declarator";
  else return "declarator";
}

void
Declarator::
setAlias(Typedef* td)
{
  alias_    = td;
  thisType_ = new DeclaredType(IdlType::tk_alias, this, this);
}

// Typedef
Typedef::
Typedef(const char* file, int line, _CORBA_Boolean mainFile,
	IdlType* aliasType, _CORBA_Boolean constrType,
	Declarator* declarators)

  : Decl(D_TYPEDEF, file, line, mainFile),
    aliasType_(aliasType),
    constrType_(constrType),
    declarators_(declarators)
{
  if (aliasType) delType_ = aliasType->shouldDelete();
  else           delType_ = 0;

  for (Declarator* d = declarators; d; d = (Declarator*)d->next()) {
    d->setAlias(this);
    Scope::current()->addDecl(d->eidentifier(), 0, d, d->thisType(),
			      d->file(), d->line());
  }
}

Typedef::
~Typedef()
{
  if (delType_)     delete aliasType_;
  if (declarators_) delete declarators_;
}


// Member
Member::
Member(const char* file, int line, _CORBA_Boolean mainFile,
       IdlType* memberType, _CORBA_Boolean constrType,
       Declarator* declarators)

  : Decl(D_MEMBER, file, line, mainFile),
    memberType_(memberType),
    constrType_(constrType),
    declarators_(declarators)
{
  if (memberType) delType_ = memberType->shouldDelete();
  else {
    delType_ = 0;
    return;
  }
  if (memberType->kind() == IdlType::tk_struct) {
    Struct* s = (Struct*)((DeclaredType*)memberType)->decl();
    if (!s->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of struct `%s' inside "
	       "its own definition", s->identifier());
    }
  }
  else if (memberType->kind() == IdlType::tk_union) {
    Union* u = (Union*)((DeclaredType*)memberType)->decl();
    if (!u->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of union `%s' inside "
	       "its own definition", u->identifier());
    }
  }
  else if (memberType->kind() == IdlType::tk_sequence) {
    // Look for recursive sequence
    IdlType* t = memberType;
    while (t->kind() == IdlType::tk_sequence)
      t = ((SequenceType*)t)->seqType();

    if (t->kind() == IdlType::tk_struct) {
      Struct* s = (Struct*)((DeclaredType*)t)->decl();
      if (!s->finished())
	s->setRecursive();
    }
    else if (t->kind() == IdlType::tk_union) {
      Union* u = (Union*)((DeclaredType*)t)->decl();
      if (!u->finished())
	u->setRecursive();
    }
  }
  for (Declarator* d = declarators; d; d = (Declarator*)d->next()) {
    Scope::current()->addInstance(d->eidentifier(), d, memberType,
				  d->file(), d->line());
  }
}

Member::
~Member()
{
  if (declarators_) delete declarators_;
  if (delType_)     delete memberType_;
}

// Struct
Struct::
Struct(const char* file, int line, _CORBA_Boolean mainFile,
       const char* identifier)

  : Decl(D_STRUCT, file, line, mainFile),
    DeclRepoId(identifier),
    members_(0),
    recursive_(0),
    finished_(0)
{
  Scope* s  = Scope::current()->newStructScope(identifier, file, line);
  thisType_ = new DeclaredType(IdlType::tk_struct, this, this);
  Scope::current()->addDecl(identifier, s, this, thisType_, file, line);
  Scope::startScope(s);
  Prefix::newScope(identifier);
}

Struct::
~Struct()
{
  if (members_) delete members_;
  delete thisType_;
}

void
Struct::
finishConstruction(Member* members)
{
  members_ = members;
  Prefix::endScope();
  Scope::endScope();
  finished_ = 1;
}


// Exception
Exception::
Exception(const char* file, int line, _CORBA_Boolean mainFile,
	  const char* identifier)

  : Decl(D_EXCEPTION, file, line, mainFile),
    DeclRepoId(identifier),
    members_(0)
{
  Scope* s = Scope::current()->newExceptionScope(identifier, file, line);
  Scope::current()->addDecl(identifier, s, this, 0, file, line);
  Scope::startScope(s);
  Prefix::newScope(identifier);
}

Exception::
~Exception()
{
  if (members_) delete members_;
}

void
Exception::
finishConstruction(Member* members)
{
  members_ = members;
  Prefix::endScope();
  Scope::endScope();
}


// Case label
CaseLabel::
CaseLabel(const char* file, int line, _CORBA_Boolean mainFile,
	  IdlExpr* value)

  : Decl(D_CASELABEL, file, line, mainFile),
    value_(value),
    labelKind_(IdlType::tk_null)
{
  isDefault_ = (value_ == 0);
}

CaseLabel::
~CaseLabel()
{
  if (value_) delete value_;
}

void
CaseLabel::
setType(IdlType* type)
{
  labelKind_ = type->kind();

  if (!value_) return;

  switch (labelKind_) {
  case IdlType::tk_short:     v_.short_     = value_->evalAsShort();     break;
  case IdlType::tk_long:      v_.long_      = value_->evalAsLong();      break;
  case IdlType::tk_ushort:    v_.ushort_    = value_->evalAsUShort();    break;
  case IdlType::tk_ulong:     v_.ulong_     = value_->evalAsULong();     break;
  case IdlType::tk_boolean:   v_.boolean_   = value_->evalAsBoolean();   break;
  case IdlType::tk_char:      v_.char_      = value_->evalAsChar();      break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong:  v_.longlong_  = value_->evalAsLongLong();  break;
  case IdlType::tk_ulonglong: v_.ulonglong_ = value_->evalAsULongLong(); break;
#endif
  case IdlType::tk_wchar:     v_.wchar_     = value_->evalAsWChar();     break;
  case IdlType::tk_enum:
    v_.enumerator_ =
      value_->evalAsEnumerator((Enum*)((DeclaredType*)type)->decl());
    break;
  default:
    assert(0);
  }
  delete value_;
  value_ = 0;
}

#define LABEL_AS(rt, op, tk, un) \
rt CaseLabel::op() const { \
  assert(labelKind_ == IdlType::tk); \
  return v_.un; \
}

LABEL_AS(_CORBA_Short,        labelAsShort,      tk_short,      short_)
LABEL_AS(_CORBA_Long,         labelAsLong,       tk_long,       long_)
LABEL_AS(_CORBA_UShort,       labelAsUShort,     tk_ushort,     ushort_)
LABEL_AS(_CORBA_ULong,        labelAsULong,      tk_ulong,      ulong_)
LABEL_AS(_CORBA_Boolean,      labelAsBoolean,    tk_boolean,    boolean_)
LABEL_AS(_CORBA_Char,         labelAsChar,       tk_char,       char_)
#ifdef HAS_LongLong
LABEL_AS(_CORBA_LongLong,     labelAsLongLong,   tk_longlong,   longlong_)
LABEL_AS(_CORBA_ULongLong,    labelAsULongLong,  tk_ulonglong,  ulonglong_)
#endif
LABEL_AS(_CORBA_WChar,        labelAsWChar,      tk_wchar,      wchar_)
LABEL_AS(Enumerator*,         labelAsEnumerator, tk_enum,       enumerator_)


// UnionCase
UnionCase::
UnionCase(const char* file, int line, _CORBA_Boolean mainFile,
	  IdlType* caseType, _CORBA_Boolean constrType,
	  Declarator* declarator)

  : Decl(D_UNIONCASE, file, line, mainFile),
    labels_(0),
    caseType_(caseType),
    constrType_(constrType),
    declarator_(declarator)
{
  if (caseType) delType_ = caseType->shouldDelete();
  else {
    delType_ = 0;
    return;
  }
  if (caseType->kind() == IdlType::tk_struct) {
    Struct* s = (Struct*)((DeclaredType*)caseType)->decl();
    if (!s->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of struct `%s' inside "
	       "its own definition", s->identifier());
    }
  }
  else if (caseType->kind() == IdlType::tk_union) {
    Union* u = (Union*)((DeclaredType*)caseType)->decl();
    if (!u->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of union `%s' inside "
	       "its own definition", u->identifier());
    }
  }
  else if (caseType->kind() == IdlType::tk_sequence) {
    // Look for recursive sequence
    IdlType* t = caseType;
    while (t->kind() == IdlType::tk_sequence)
      t = ((SequenceType*)t)->seqType();

    if (t->kind() == IdlType::tk_struct) {
      Struct* s = (Struct*)((DeclaredType*)t)->decl();
      if (!s->finished())
	s->setRecursive();
    }
    else if (t->kind() == IdlType::tk_union) {
      Union* u = (Union*)((DeclaredType*)t)->decl();
      if (!u->finished())
	u->setRecursive();
    }
  }
  Scope::current()->addInstance(declarator->identifier(), declarator,
				caseType, declarator->file(),
				declarator->line());
}

UnionCase::
~UnionCase()
{
  if (labels_)     delete labels_;
  if (declarator_) delete declarator_;
  if (delType_ )   delete caseType_;
}

void
UnionCase::
finishConstruction(CaseLabel* labels)
{
  labels_ = labels;
}


// Union
Union::
Union(const char* file, int line, _CORBA_Boolean mainFile,
      const char* identifier)

  : Decl(D_UNION, file, line, mainFile),
    DeclRepoId(identifier),
    switchType_(0),
    constrType_(0),
    cases_(0),
    recursive_(0),
    finished_(0)
{
  Scope* s  = Scope::current()->newUnionScope(identifier, file, line);
  thisType_ = new DeclaredType(IdlType::tk_union, this, this);
  Scope::current()->addDecl(identifier, s, this, thisType_, file, line);
  Scope::startScope(s);
  Prefix::newScope(identifier);
}

Union::
~Union()
{
  if (cases_) delete cases_;
  delete thisType_;
}

// Ugly code to set case label types, check for label clashes, and
// come up with a default label value. Loops are O(n^2), but n will
// usually be quite small. ***
#define UNION_SWITCH(lt, op, defstart, islastdef, nextdef) { \
  lt label; \
  for (c = cases; c; c = (UnionCase*)c->next()) { \
    for (l = c->labels(); l; l = (CaseLabel*)l->next()) { \
      l->setType(t); \
      if (!l->isDefault()) \
        label = l->labelAs ## op(); \
      else \
        defLabel = l; \
      for (d = cases; d != c->next(); d = (UnionCase*)d->next()) { \
        for (m = d->labels(); m && m != l; m = (CaseLabel*)m->next()) { \
          if (l->isDefault() || m->isDefault()) { \
            if (l->isDefault() && m->isDefault()) \
              ++clash; \
          } \
	  else if (m->labelAs ## op() == label) ++clash; \
        } \
      } \
    } \
  } \
  if (defLabel) { \
    lt defVal = defstart; \
  again ## op: \
    for (c = cases; c; c = (UnionCase*)c->next()) { \
      for (l = c->labels(); l; l = (CaseLabel*)l->next()) { \
        if (!l->isDefault() && l->labelAs ## op() == defVal) { \
          if (islastdef) goto nomore ## op; \
          nextdef; \
          goto again ## op; \
        } \
      } \
    } \
    foundDef = 1; \
  nomore ## op: \
    defLabel->setDefault ## op(defVal); \
  } \
  break; \
}

void
Union::
finishConstruction(IdlType* switchType, _CORBA_Boolean constrType,
		   UnionCase* cases)
{
  if (!switchType) return;

  switchType_ = switchType;
  constrType_ = constrType;
  cases_      = cases;
  finished_   = 1;

  IdlType* t  = switchType;

  while (t && t->kind() == IdlType::tk_alias)
    t = ((Declarator*)((DeclaredType*)t)->decl())->alias()->aliasType();

  if (!t) { // Broken alias due to earlier error
    Prefix::endScope();
    Scope::endScope();
    return;
  }
  UnionCase      *c, *d;
  CaseLabel      *l, *m, *defLabel = 0;
  IdlType::Kind  k     = t->kind();
  int            clash = 0;
  _CORBA_Boolean foundDef = 0;

  switch (k) {
  case IdlType::tk_short:
    UNION_SWITCH(_CORBA_Short, Short, -0x8000, defVal==0x7fff, ++defVal)
  case IdlType::tk_long:
    UNION_SWITCH(_CORBA_Long, Long, -0x80000000, defVal==0x7fffffff, ++defVal)
  case IdlType::tk_ushort:
    UNION_SWITCH(_CORBA_UShort, UShort, 0xffff, defVal==0, --defVal)
  case IdlType::tk_ulong:
    UNION_SWITCH(_CORBA_ULong, ULong, 0xffffffff, defVal==0, --defVal)
  case IdlType::tk_boolean:
    UNION_SWITCH(_CORBA_Boolean, Boolean, 0, defVal==1, defVal=1)
  case IdlType::tk_char:
    UNION_SWITCH(_CORBA_Char, Char, 0xff, defVal==0, --defVal)
#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    UNION_SWITCH(_CORBA_LongLong, LongLong,
		 -0x8000000000000000LL, defVal==0x7fffffffffffffffLL, ++defVal)
  case IdlType::tk_ulonglong:
    UNION_SWITCH(_CORBA_ULongLong, ULongLong,
		 0xffffffffffffffffLL, defVal==0LL, --defVal)
#endif
  case IdlType::tk_wchar:
    UNION_SWITCH(_CORBA_WChar, WChar, 0xffff, defVal==0, --defVal)

  case IdlType::tk_enum:
    {
      Enum *e = (Enum*)((DeclaredType*)t)->decl();
      UNION_SWITCH(Enumerator*, Enumerator, e->enumerators(),
		   !defVal->next(), defVal=(Enumerator*)defVal->next())
    }
  default:
    IdlError(file(), line(), "Invalid type for union switch: %s",
	     t->kindAsString());
  }

  if (clash)
    IdlError(file(), line(), "Error in union `%s': %d repeated union label%s",
	     identifier(), clash, clash == 1 ? "" : "s");

  if (defLabel && !foundDef)
    IdlError(defLabel->file(), defLabel->line(),
	     "Error in union `%s': cannot declare default case since "
	     "all cases are explicitly listed", identifier());

  Prefix::endScope();
  Scope::endScope();
}


// Enum

Enumerator::
Enumerator(const char* file, int line, _CORBA_Boolean mainFile,
	   const char* identifier)

  : Decl(D_ENUMERATOR, file, line, mainFile),
    DeclRepoId(identifier),
    container_(0)
{
  Scope::current()->addDecl(identifier, 0, this, 0, file, line);
}

Enumerator::
~Enumerator()
{
}

void
Enumerator::
finishConstruction(Enum* container)
{
  container_ = container;
}

Enum::
Enum(const char* file, int line, _CORBA_Boolean mainFile,
     const char* identifier)

  : Decl(D_ENUM, file, line, mainFile),
    DeclRepoId(identifier),
    enumerators_(0)
{
  thisType_ = new DeclaredType(IdlType::tk_enum, this, this);
  Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
}

Enum::
~Enum()
{
  if (enumerators_) delete enumerators_;
  delete thisType_;
}

void
Enum::
finishConstruction(Enumerator* enumerators)
{
  enumerators_ = enumerators;

  for (Enumerator* e = enumerators; e; e = (Enumerator*)e->next())
    e->finishConstruction(this);
}


// Attribute
Attribute::
Attribute(const char* file, int line, _CORBA_Boolean mainFile,
	  _CORBA_Boolean readonly, IdlType* attrType,
	  Declarator* declarators)

  : Decl(D_ATTRIBUTE, file, line, mainFile),
    readonly_(readonly),
    attrType_(attrType),
    declarators_(declarators)
{
  if (attrType) delType_ = attrType->shouldDelete();
  else          delType_ = 0;

  for (Declarator* d = declarators; d; d = (Declarator*)d->next()) {
    assert(!d->sizes()); // Enforced by grammar
    Scope::current()->addCallable(d->eidentifier(), 0, this, file, line);
  }
}

Attribute::
~Attribute()
{
  delete declarators_;
  if (delType_) delete attrType_;
}


// Operation

Parameter::
Parameter(const char* file, int line, _CORBA_Boolean mainFile,
	  int direction, IdlType* paramType, const char* identifier)

  : Decl(D_PARAMETER, file, line, mainFile),
    direction_(direction),
    paramType_(paramType)
{
  if (paramType) delType_ = paramType->shouldDelete();
  else           delType_ = 0;

  if (identifier[0] == '_')
    identifier_ = idl_strdup(identifier+1);
  else
    identifier_ = idl_strdup(identifier);

  Scope::current()->addDecl(identifier, 0, this, paramType, file, line);
}

Parameter::
~Parameter()
{
  delete [] identifier_;
  if (delType_) delete paramType_;
}


RaisesSpec::
RaisesSpec(const ScopedName* sn, const char* file, int line)

  : exception_(0), next_(0)
{
  last_ = this;

  const Scope::Entry* se = Scope::current()->findScopedName(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL &&
	se->decl()->kind() == Decl::D_EXCEPTION) {
      exception_ = (Exception*)se->decl();
    }
    else {
      const char* ssn = sn->toString();
      IdlError(file, line,
	       "`%s' used in raises expression is not an exception",
	       ssn);
      IdlErrorCont(se->file(), se->line(), "(`%s' declared here)", ssn);
      delete [] ssn;
    }
  }
}

RaisesSpec::
~RaisesSpec()
{
  if (next_) delete next_;
}


ContextSpec::
ContextSpec(const char* c, const char* file, int line)

  : context_(idl_strdup(c)),
    next_(0)
{
  last_ = this;

  _CORBA_Boolean bad = 0;

  if (!isalpha(*c))
    bad = 1;
  else {
    for (; *c; c++) {
      if (!isalnum(*c) && *c != '.' && *c != '_' && *c != '*') {
	bad = 1;
	break;
      }
    }
  }
  if (bad)
    IdlError(file, line, "Invalid context name \"%s\"", context_);
}

ContextSpec::
~ContextSpec()
{
  delete [] context_;
  if (next_) delete next_;
}


Operation::
Operation(const char* file, int line, _CORBA_Boolean mainFile,
	  _CORBA_Boolean oneway, IdlType* returnType,
	  const char* identifier)

  : Decl(D_OPERATION, file, line, mainFile),
    oneway_(oneway),
    returnType_(returnType),
    parameters_(0),
    raises_(0),
    contexts_(0)
{
  if (returnType) delType_ = returnType->shouldDelete();
  else            delType_ = 0;

  if (identifier[0] == '_')
    identifier_ = idl_strdup(identifier+1);
  else
    identifier_ = idl_strdup(identifier);

  Scope* s = Scope::current()->newOperationScope(file, line);
  Scope::current()->addCallable(identifier, s, this, file, line);
  Scope::startScope(s);
}

Operation::
~Operation()
{
  delete [] identifier_;
  if (parameters_) delete parameters_;
  if (raises_)     delete raises_;
  if (contexts_)   delete contexts_;
  if (delType_)    delete returnType_;
}

void
Operation::
finishConstruction(Parameter* parameters, RaisesSpec* raises,
		   ContextSpec* contexts)
{
  parameters_ = parameters;
  raises_     = raises;
  contexts_   = contexts;

  if (oneway_) {
    if (returnType_ && returnType_->kind() != IdlType::tk_void) {
      IdlError(file(), line(), "Oneway operation `%s' does not return void",
	       identifier());
    }
    for (Parameter* p = parameters; p; p = (Parameter*)p->next()) {
      if (p->direction() == 1) {
	IdlError(p->file(), p->line(),
		 "In oneway operation `%s': out parameter `%s' "
		 "is not permitted",
		 identifier(), p->identifier());
      }
      else if (p->direction() == 2) {
	IdlError(p->file(), p->line(),
		 "In oneway operation `%s': inout parameter `%s' "
		 "is not permitted",
		 identifier(), p->identifier());
      }
    }
    if (raises_) {
      IdlError(file(), line(),
	       "Oneway operation `%s' is not permitted to have "
	       "a raises expression", identifier());
    }
  }
  Scope::endScope();
}


// Native
Native::
Native(const char* file, int line, _CORBA_Boolean mainFile,
       const char* identifier)

  : Decl(D_NATIVE, file, line, mainFile),
    DeclRepoId(identifier)
{
  Scope::current()->addDecl(identifier, 0, this, 0, file, line);
}

Native::
~Native()
{
}


// ValueType

StateMember::
StateMember(const char* file, int line, _CORBA_Boolean mainFile,
	    int memberAccess, IdlType* memberType,
	    _CORBA_Boolean constrType, Declarator* declarators)

  : Decl(D_STATEMEMBER, file, line, mainFile),
    memberAccess_(memberAccess),
    memberType_(memberType),
    constrType_(constrType),
    declarators_(declarators)
{
  if (memberType) delType_ = memberType->shouldDelete();
  else            delType_ = 0;

  // *** Is this right?  Should StateMembers be Callables?
  for (Declarator* d = declarators; d; d = (Declarator*)d->next())
    Scope::current()->addCallable(d->eidentifier(), 0, this, file, line);
}

StateMember::
~StateMember()
{
  if (delType_)     delete memberType_;
  if (declarators_) delete declarators_;
}


Factory::
Factory(const char* file, int line, _CORBA_Boolean mainFile,
	const char* identifier)

  : Decl(D_FACTORY, file, line, mainFile)
{
  if (identifier[0] == '_')
    identifier_ = idl_strdup(identifier+1);
  else
    identifier_ = idl_strdup(identifier);

  Scope* s = Scope::current()->newOperationScope(file, line);
  Scope::current()->addCallable(identifier, s, this, file, line);
  Scope::startScope(s);
}

Factory::
~Factory()
{
  delete [] identifier_;
  if (parameters_) delete parameters_;
}

void
Factory::
finishConstruction(Parameter* parameters)
{
  parameters_ = parameters_;
  Scope::endScope();
}

ValueBase::
ValueBase(Decl::Kind k, const char* file, int line, _CORBA_Boolean mainFile,
	  const char* identifier)

  : Decl(k, file, line, mainFile),
    DeclRepoId(identifier)
{
}

ValueBase::
~ValueBase()
{
}

ValueForward::
ValueForward(const char* file, int line, _CORBA_Boolean mainFile,
	     _CORBA_Boolean abstract, const char* identifier)

  : ValueBase(D_VALUEFORWARD, file, line, mainFile, identifier),
    abstract_(abstract),
    definition_(0),
    firstForward_(0)
{
  Scope::Entry*  se  = Scope::current()->find(identifier);
  _CORBA_Boolean reg = 1;

  if (se && se->kind() == Scope::Entry::E_DECL) {

    if (se->decl()->kind() == D_VALUEABS) {
      ValueAbs* v = (ValueAbs*)se->decl();
      definition_ = v;

      if (strcmp(v->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of valuetype `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(v->file(), v->line(),
		     "(`%s' fully declared here with prefix `%s')",
		     v->identifier(), v->prefix());
	reg = 0;
      }
      if (!abstract) {
	IdlError(file, line,
		 "Forward declaration of non-abstract valuetype `%s' "
		 "conflicts with earlier full declaration as abstract",
		 identifier);
	IdlErrorCont(v->file(), v->line(),
		     "(`%s' declared as abstract here)");
      }
    }
    else if (se->decl()->kind() == D_VALUE) {
      Value* v    = (Value*)se->decl();
      definition_ = v;

      if (strcmp(v->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of valuetype `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(v->file(), v->line(),
		     "(`%s' fully declared here with prefix `%s')",
		     v->identifier(), v->prefix());
	reg = 0;
      }
      if (abstract) {
	IdlError(file, line,
		 "Forward declaration of abstract valuetype `%s' "
		 "conflicts with earlier full declaration as non-abstract",
		 identifier);
	IdlErrorCont(v->file(), v->line(),
		     "(`%s' declared as non-abstract here)");
      }
    }
    else if (se->decl()->kind() == D_VALUEFORWARD) {
      ValueForward* v = (ValueForward*)se->decl();
      firstForward_   = v;

      if (strcmp(v->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of interface `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(v->file(), v->line(),
		     "(`%s' forward declared here with prefix `%s')",
		     v->identifier(), v->prefix());
      }
      if (abstract && !v->abstract()) {
	IdlError(file, line,
		 "Forward declaration of abstract valuetype `%s' conflicts "
		 "with earlier forward declaration as non-abstract",
		 identifier);
	IdlErrorCont(v->file(), v->line(),
		     "(`%s' forward declared as non-abstract here)");
      }
      else if (!abstract && v->abstract()) {
	IdlError(file, line,
		 "Forward declaration of non-abstract valuetype `%s' "
		 "conflicts  with earlier forward declaration as abstract",
		 identifier);
	IdlErrorCont(v->file(), v->line(),
		     "(`%s' forward declared as abstract here)");
      }
      if (v->repoIdSet()) setRepoId(v->repoId(), v->rifile(), v->riline());
    }
    reg = 0;
  }
  if (reg) {
    thisType_ = new DeclaredType(IdlType::tk_value, this, this);
    Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
  }
}

ValueForward::
~ValueForward()
{
  delete thisType_;
}

ValueBase*
ValueForward::
definition() const
{
  if (firstForward_)
    return firstForward_->definition();
  else
    return definition_;
}

void
ValueForward::
setDefinition(ValueBase* defn)
{
  definition_ = defn;
}

ValueBox::
ValueBox(const char* file, int line, _CORBA_Boolean mainFile,
	 const char* identifier, IdlType* boxedType,
	 _CORBA_Boolean constrType)

  : ValueBase(D_VALUEBOX, file, line, mainFile, identifier),
    boxedType_(boxedType),
    constrType_(constrType)
{
  thisType_ = new DeclaredType(IdlType::tk_value_box, this, this);
  Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
}

ValueBox::
~ValueBox()
{
  delete thisType_;
}

ValueInheritSpec::
ValueInheritSpec(ScopedName* sn, const char* file, int line)

  : value_(0), decl_(0), scope_(0), next_(0), truncatable_(0)
{
  const Scope::Entry* se = Scope::current()->findScopedName(sn, file, line);

  if (se) {
    if (se->kind() == Scope::Entry::E_DECL) {

      decl_      = se->decl();
      IdlType* t = se->idltype();

      while (t && t->kind() == IdlType::tk_alias)
	t = ((Declarator*)((DeclaredType*)t)->decl())->alias()->aliasType();

      if (!t) return;

      if (t->kind() == IdlType::tk_value) {

	Decl* d = ((DeclaredType*)t)->decl();

	if (d->kind() == Decl::D_VALUE) {
	  value_ = (Value*)d;
	  scope_ = ((Value*)d)->scope();
	  return;
	}
	else if (d->kind() == Decl::D_VALUEABS) {
	  value_ = (ValueAbs*)d;
	  scope_ = ((ValueAbs*)d)->scope();
	  return;
	}
	else if (d->kind() == Decl::D_VALUEFORWARD) {
	  const char* ssn = ((ValueForward*)d)->scopedName()->toString();
	  IdlError(file, line,
		   "Inherited valuetype `%s' must be fully defined", ssn);

	  if (decl_ != d) {
	    const char* tssn = sn->toString();
	    IdlErrorCont(se->file(), se->line(),
			 "(`%s' reached through typedef `%s')",
			 ssn, tssn);
	    delete [] tssn;
	  }
	  IdlErrorCont(d->file(), d->line(),
		       "(`%s' forward declared here)", ssn);
	  delete [] ssn;
	  return;
	}
      }
    }
    const char* ssn = sn->toString();
    IdlError(file, line,
	     "`%s' used in inheritance specification is not a valuetype",
	     ssn);
    IdlErrorCont(se->file(), se->line(), "(`%s' declared here)", ssn);
    delete [] ssn;
  }
}


void
ValueInheritSpec::
append(ValueInheritSpec* is, const char* file, int line)
{
  ValueInheritSpec *i, *last;

  for (i=this; i; i = i->next_) {
    last = i;
    if (is->value() == i->value()) {
      const char* ssn = is->value()->scopedName()->toString();
      IdlError(file, line,
	       "Cannot specify `%s' as a direct base valuetype "
	       "more than once", ssn);
      delete [] ssn;
      delete is;
      return;
    }
  }
  last->next_ = is;
}

ValueAbs::
ValueAbs(const char* file, int line, _CORBA_Boolean mainFile,
	 const char* identifier, ValueInheritSpec* inherits,
	 InheritSpec* supports)

  : ValueBase(D_VALUEABS, file, line, mainFile, identifier),
    inherits_(inherits),
    supports_(supports),
    contents_(0)
{
  // Look for forward valuetype
  Scope::Entry* se = Scope::current()->find(identifier);

  if (se &&
      se->kind() == Scope::Entry::E_DECL &&
      se->decl()->kind() == Decl::D_VALUEFORWARD) {

    ValueForward* f = (ValueForward*)se->decl();

    if (strcmp(f->prefix(), prefix())) {

      IdlError(file, line,
	       "In declaration of valuetype `%s', repository id "
	       "prefix `%s' differs from that of forward declaration",
	       identifier, prefix());

      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here with prefix `%s')",
		   f->identifier(), f->prefix());
    }
    if (!f->abstract()) {
      IdlError(file, line,
	       "Declaration of abstract valuetype `%s' conflicts with "
	       "forward declaration as non-abstract", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as non-abstract here)");
    }
    if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());

    f->setDefinition(this);
    Scope::current()->remEntry(se);
  }
  scope_    = Scope::current()->newValueScope(identifier, file, line);
  thisType_ = new DeclaredType(IdlType::tk_value, this, this);

  // Check that all inherited valuetypes are abstract
  for (ValueInheritSpec* vinh = inherits; vinh; vinh = vinh->next()) {
    if (!vinh->value()->kind() == D_VALUE) {
      const char* ssn = vinh->scope()->scopedName()->toString();
      IdlError(file, line,
	       "In declaration of abstract valuetype `%s', inherited "
	       "valuetype `%s' is not abstract", identifier, ssn);
      IdlErrorCont(vinh->value()->file(), vinh->value()->line(),
		   "(%s declared here)", ssn);
      delete [] ssn;
    }
  }
  scope_->setInherited(inherits, file, line);
  scope_->setInherited(supports, file, line);
  Scope::current()->addDecl(identifier, scope_, this, thisType_, file, line);
  Scope::startScope(scope_);
  Prefix::newScope(identifier);
}

ValueAbs::
~ValueAbs()
{
  if (inherits_) delete inherits_;
  if (contents_) delete contents_;
  delete thisType_;
}

void
ValueAbs::
finishConstruction(Decl* contents)
{
  contents_ = contents;
  Prefix::endScope();
  Scope::endScope();
}

Value::
Value(const char* file, int line, _CORBA_Boolean mainFile,
      _CORBA_Boolean custom, const char* identifier,
      ValueInheritSpec* inherits, InheritSpec* supports)

  : ValueBase(D_VALUE, file, line, mainFile, identifier),
    custom_(custom),
    inherits_(inherits),
    supports_(supports),
    contents_(0)
{
  // Look for forward valuetype
  Scope::Entry* se = Scope::current()->find(identifier);

  if (se &&
      se->kind() == Scope::Entry::E_DECL &&
      se->decl()->kind() == Decl::D_VALUEFORWARD) {

    ValueForward* f = (ValueForward*)se->decl();

    if (strcmp(f->prefix(), prefix())) {

      IdlError(file, line,
	       "In declaration of valuetype `%s', repository id "
	       "prefix `%s' differs from that of forward declaration",
	       identifier, prefix());

      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here with prefix `%s')",
		   f->identifier(), f->prefix());
    }
    if (f->abstract()) {
      IdlError(file, line,
	       "Declaration of non-abstract valuetype `%s' conflicts with "
	       "forward declaration as abstract", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as abstract here)");
    }
    if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());

    f->setDefinition(this);
    Scope::current()->remEntry(se);
  }
  scope_    = Scope::current()->newValueScope(identifier, file, line);
  thisType_ = new DeclaredType(IdlType::tk_value, this, this);

  // Check all the various inheritance rules
  if (inherits) {
    if (custom) {
      if (inherits->truncatable()) {
	IdlError(file, line,
		 "`truncatable' may not be specified for a custom valuetype");
      }
    }
    else {
      if (inherits->value()->kind() == D_VALUE) {
	if (((Value*)inherits->value())->custom()) {
	  const char* ssn = inherits->scope()->scopedName()->toString();
	  IdlError(file, line,
		   "In declaration of non-custom valuetype `%s', inherited "
		   "valuetype `%s' is custom", identifier, ssn);
	  IdlErrorCont(inherits->value()->file(), inherits->value()->line(),
		       "(%s declared here)", ssn);
	  delete [] ssn;
	}	  
      }
    }
    for (ValueInheritSpec* vinh = inherits->next();
	 vinh;
	 vinh = vinh->next()) {

      if (vinh->value()->kind() == D_VALUE) {
	const char* ssn = vinh->scope()->scopedName()->toString();
	IdlError(file, line,
		 "In declaration of valuetype `%s', inherited valuetype "
		 "`%s' is non-abstract but is not specified first",
		 identifier, ssn);
	IdlErrorCont(vinh->value()->file(), vinh->value()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
    }
    scope_->setInherited(inherits, file, line);
  }
  // Supports rules
  if (supports) {
    for (InheritSpec* inh = supports->next(); inh; inh = inh->next()) {
      if (!inh->interface()->abstract()) {
	const char* ssn = inh->scope()->scopedName()->toString();
	IdlError(file, line,
		 "In declaration of valuetype `%s', supported interface "
		 "`%s' is non-abstract but is not specified first",
		 identifier, ssn);
	IdlErrorCont(inh->interface()->file(), inh->interface()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
    }
    scope_->setInherited(supports, file, line);
  }
  Scope::current()->addDecl(identifier, scope_, this, thisType_, file, line);
  Scope::startScope(scope_);
  Prefix::newScope(identifier);
}


Value::
~Value()
{
  if (inherits_) delete inherits_;
  if (supports_) delete supports_;
  if (contents_) delete contents_;
  delete thisType_;
}

void
Value::
finishConstruction(Decl* contents)
{
  contents_ = contents;
  Prefix::endScope();
  Scope::endScope();
}
