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
// Revision 1.16.2.6  2001/06/08 17:12:22  dpg1
// Merge all the bug fixes from omni3_develop.
//
// Revision 1.16.2.5  2001/03/13 10:32:11  dpg1
// Fixed point support.
//
// Revision 1.16.2.4  2000/11/01 12:45:56  dpg1
// Update to CORBA 2.4 specification.
//
// Revision 1.16.2.3  2000/10/27 16:31:08  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.16.2.2  2000/10/10 10:18:50  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.14.2.9  2000/08/29 10:20:26  dpg1
// Operations and attributes now have repository ids.
//
// Revision 1.14.2.8  2000/08/25 13:33:02  dpg1
// Multiple comments preceding a declaration are now properly attached on
// all platforms
//
// Revision 1.14.2.7  2000/08/07 15:34:36  dpg1
// Partial back-port of long long from omni3_1_develop.
//
// Revision 1.14.2.6  2000/07/26 10:59:14  dpg1
// Incorrect error report when inheriting typedef to forward declared
// interface
//
// Revision 1.14.2.5  2000/06/08 14:36:19  dpg1
// Comments and pragmas are now objects rather than plain strings, so
// they can have file,line associated with them.
//
// Revision 1.14.2.4  2000/06/05 18:13:26  dpg1
// Comments can be attached to subsequent declarations (with -K). Better
// idea of most recent decl in operation declarations
//
// Revision 1.14.2.3  2000/03/16 17:35:21  dpg1
// Bug with comments in input when keepComments is false.
//
// Revision 1.14.2.2  2000/03/07 10:31:26  dpg1
// More sensible idea of the "most recent" declaration.
//
// Revision 1.14.2.1  2000/03/06 15:03:49  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//
// Revision 1.14  2000/02/03 14:50:07  dpg1
// Native declarations can now be used as types.
//
// Revision 1.13  2000/01/05 11:21:08  dpg1
// Removed warning about signed/unsigned comparison.
// * can only be the last character of a context key.
//
// Revision 1.12  1999/12/28 18:15:45  dpg1
// Bounds of string constants now checked.
//
// Revision 1.11  1999/11/30 18:06:19  dpg1
// Alias dereferencing bugs.
//
// Revision 1.10  1999/11/22 11:07:46  dpg1
// Correctly report error with interface which tries to inherit from
// CORBA::Object.
//
// Revision 1.9  1999/11/17 17:17:00  dpg1
// Changes to remove static initialisation of objects.
//
// Revision 1.8  1999/11/04 17:15:52  dpg1
// Typo.
//
// Revision 1.7  1999/11/02 17:07:27  dpg1
// Changes to compile on Solaris.
//
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
#include <idlconfig.h>

#include <string.h>
#include <ctype.h>

// Globals from lexer
extern FILE* yyin;
extern char* currentFile;
extern int   yylineno;

AST*     AST::tree_           = 0;
Decl*    Decl::mostRecent_    = 0;
Comment* Comment::mostRecent_ = 0;
Comment* Comment::saved_      = 0;


// Static error message functions
static void
checkNotForward(const char* file, int line, IdlType* t)
{
  if (t->kind() == IdlType::ot_structforward) {
    StructForward* f = (StructForward*)((DeclaredType*)t)->decl();

    if (!f->definition()) {
      char* ssn = f->scopedName()->toString();
      IdlError(file, line,
	       "Cannot use forward-declared struct `%s' before it is "
	       "fully defined", ssn);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward-declared here)", f->identifier());
      delete [] ssn;
    }
  }
  else if (t->kind() == IdlType::ot_unionforward) {
    UnionForward* f = (UnionForward*)((DeclaredType*)t)->decl();

    if (!f->definition()) {
      char* ssn = f->scopedName()->toString();
      IdlError(file, line,
	       "Cannot use forward-declared union `%s' before it is "
	       "fully defined", ssn);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward-declared here)", f->identifier());
      delete [] ssn;
    }
  }
}

static
void
checkValidType(const char* file, int line, IdlType* t)
{
  t = t->unalias();
  checkNotForward(file, line, t);

  if (t->kind() == IdlType::tk_sequence) {

    while (t->kind() == IdlType::tk_sequence)
      t = ((SequenceType*)t)->seqType()->unalias();

    checkNotForward(file, line, t);
  }
}


// Pragma
void
Pragma::
add(const char* pragmaText, const char* file, int line)
{
  if (Decl::mostRecent())
    Decl::mostRecent()->addPragma(pragmaText, file, line);
  else
    AST::tree()->addPragma(pragmaText, file, line);
}

// Comment

void
Comment::
add(const char* commentText, const char* file, int line)
{
  if (Config::keepComments) {
    if (Config::commentsFirst) {
      if (saved_) {
	// C++ says that the order of value evaluation is undefined.
	// Comment's constructor sets mostRecent_, so the innocent-
	// looking mostRecent_->next_ = new Comment... does the wrong
	// thing with some compilers :-(
	Comment* mr = mostRecent_;
	mr->next_ = new Comment(commentText, file, line);
      }
      else
	saved_ = new Comment(commentText, file, line);
    }
    else {
      if (Decl::mostRecent())
	Decl::mostRecent()->addComment(commentText, file, line);
      else
	AST::tree()->addComment(commentText, file, line);
    }
  }
}

void
Comment::
append(const char* commentText)
{
  if (Config::keepComments) {
    assert(mostRecent_ != 0);
    char* newText = new char[(strlen(mostRecent_->commentText_) +
			      strlen(commentText) + 1)];
    strcpy(newText, mostRecent_->commentText_);
    strcat(newText, commentText);
    delete [] mostRecent_->commentText_;
    mostRecent_->commentText_ = newText;
  }
}

Comment*
Comment::
grabSaved()
{
  Comment* ret = saved_;
  saved_ = 0;
  return ret;
}


// AST
AST::AST() : declarations_(0), file_(0),
	     pragmas_(0), lastPragma_(0),
	     comments_(0), lastComment_(0) {}

AST::~AST() {
  if (declarations_) delete declarations_;
  if (file_)         delete [] file_;
  if (pragmas_)      delete pragmas_;
  if (comments_)     delete comments_;
}

void
AST::
addPragma(const char* pragmaText, const char* file, int line)
{
  Pragma* p = new Pragma(pragmaText, file, line);
  if (pragmas_)
    lastPragma_->next_ = p;
  else
    pragmas_ = p;
  lastPragma_ = p;
}

void
AST::
addComment(const char* commentText, const char* file, int line)
{
  Comment* p = new Comment(commentText, file, line);
  if (comments_)
    lastComment_->next_ = p;
  else
    comments_ = p;
  lastComment_ = p;
}


AST*
AST::
tree()
{
  if (!tree_) tree_ = new AST();
  assert(tree_ != 0);
  return tree_;
}

IDL_Boolean
AST::
process(FILE* f, const char* name)
{
  IdlType::init();
  Scope::init();

  yyin        = f;
  currentFile = idl_strdup(name);
  Prefix::newFile();

  tree()->setFile(name);

  int yr = yyparse();
  if (yr) IdlError(currentFile, yylineno, "Syntax error");

  if (Config::keepComments && Config::commentsFirst)
    tree()->comments_ = Comment::grabSaved();

  return IdlReportErrors();
}

void
AST::
clear()
{
  if (tree_) {
    delete tree_;
    tree_ = 0;
  }
  Scope::clear();
  Decl::clear();
  Comment::clear();
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
  assert(declarations_ == 0);
  declarations_ = d;

  // Validate the declarations
  AstValidateVisitor v;
  accept(v);
}


// Base Decl
Decl::
Decl(Kind kind, const char* file, int line, IDL_Boolean mainFile)

  : kind_(kind), file_(idl_strdup(file)), line_(line),
    mainFile_(mainFile), inScope_(Scope::current()),
    pragmas_(0), lastPragma_(0),
    comments_(0), lastComment_(0),
    next_(0)
{
  last_       = this;
  mostRecent_ = this;

  if (Config::keepComments && Config::commentsFirst)
    comments_ = Comment::grabSaved();
}

Decl::
~Decl()
{
  if (file_)     delete [] file_;
  if (pragmas_)  delete pragmas_;
  if (comments_) delete comments_;
  if (next_)     delete next_;
}

Decl*
Decl::
scopedNameToDecl(const char* file, int line, const ScopedName* sn)
{
  const Scope::Entry* se = Scope::current()->findScopedName(sn, file, line);

  if (se) {
    switch (se->kind()) {

    case Scope::Entry::E_MODULE:
    case Scope::Entry::E_DECL:
    case Scope::Entry::E_CALLABLE:
    case Scope::Entry::E_INHERITED:
      return se->decl();

    default:
      {
	char* ssn = sn->toString();
	IdlError(file, line, "`%s' is not a declaration", ssn);
	IdlErrorCont(se->file(), se->line(), "(`%s' created here)", ssn);
	delete [] ssn;
      }
    }
  }
  return 0;
}

void
Decl::
addPragma(const char* pragmaText, const char* file, int line)
{
  Pragma* p = new Pragma(pragmaText, file, line);
  if (pragmas_)
    lastPragma_->next_ = p;
  else
    pragmas_ = p;
  lastPragma_ = p;
}

void
Decl::
addComment(const char* commentText, const char* file, int line)
{
  Comment* p = new Comment(commentText, file, line);
  if (comments_)
    lastComment_->next_ = p;
  else
    comments_ = p;
  lastComment_ = p;
}



// Module
Module::
Module(const char* file, int line, IDL_Boolean mainFile,
       const char* identifier)

  : Decl(D_MODULE, file, line, mainFile),
    DeclRepoId(identifier),
    definitions_(0)
{
  Scope* s = Scope::current()->newModuleScope(identifier, file, line);
  Scope::current()->addModule(identifier, s, this, file, line);
  Scope::startScope(s);
  Prefix::newScope(identifier);
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
  mostRecent_ = this;
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
      IdlType* t = se->idltype()->unalias();

      if (!t) return;

      if (t->kind() == IdlType::tk_objref ||
	  t->kind() == IdlType::tk_abstract_interface ||
	  t->kind() == IdlType::tk_local_interface) {

	Decl* d = ((DeclaredType*)t)->decl();

	if (!d) {
	  char* ssn = sn->toString();
	  IdlError(file, line, "Cannot inherit from CORBA::Object");
	  IdlErrorCont(se->file(), se->line(),
		       "(accessed through typedef `%s')", ssn);
	  delete [] ssn;
	  return;
	}
	else if (d->kind() == Decl::D_INTERFACE) {
	  interface_ = (Interface*)d;
	  scope_     = interface_->scope();
	  return;
	}
	else if (d->kind() == Decl::D_FORWARD) {
	  Interface* def = ((Forward*)d)->definition();
	  if (def) {
	    interface_ = def;
	    scope_     = interface_->scope();
	    return;
	  }
	  else {
	    char* ssn = ((Forward*)d)->scopedName()->toString();
	    IdlError(file, line,
		     "Inherited interface `%s' must be fully defined", ssn);

	    if (decl_ != d) {
	      char* tssn = sn->toString();
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
    }
    char* ssn = sn->toString();
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

  if (is->interface()) {
    for (i=this; i; i = i->next_) {
      last = i;
      if (is->interface() == i->interface()) {
	char* ssn = is->interface()->scopedName()->toString();
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
}

Interface::
Interface(const char* file, int line, IDL_Boolean mainFile,
	  const char* identifier, IDL_Boolean abstract, IDL_Boolean local,
	  InheritSpec* inherits)

  : Decl(D_INTERFACE, file, line, mainFile),
    DeclRepoId(identifier),
    abstract_(abstract),
    local_(local),
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
    if (local && !f->local()) {
      IdlError(file, line,
	       "Declaration of local interface `%s' conflicts with "
	       "forward declaration as unconstrained", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as unconstrained here)");
    }
    else if (!local && f->local()) {
      IdlError(file, line,
	       "Declaration of unconstrained interface `%s' conflicts with "
	       "forward declaration as local", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared as local here)");
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
	char* ssn = inh->scope()->scopedName()->toString();
	IdlError(file, line,
		 "In declaration of abstract interface `%s', inherited "
		 "interface `%s' is not abstract", identifier, ssn);
	IdlErrorCont(inh->interface()->file(), inh->interface()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
    }
  }
  else if (local_) {
    thisType_ = new DeclaredType(IdlType::tk_local_interface, this, this);
    thisType_->setLocal();
  }
  else {
    thisType_ = new DeclaredType(IdlType::tk_objref, this, this);

    // Check that all inherited interfaces are unconstrained
    for (InheritSpec* inh = inherits; inh; inh = inh->next()) {
      if (inh->interface() && inh->interface()->local()) {
	char* ssn = inh->scope()->scopedName()->toString();
	IdlError(file, line,
		 "In declaration of unconstrained interface `%s', inherited "
		 "interface `%s' is local", identifier, ssn);
	IdlErrorCont(inh->interface()->file(), inh->interface()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
    }
  }
  scope_->setInherited(inherits, file, line);
  Scope::current()->addDecl(identifier, scope_, this, thisType_, file, line);
  Scope::startScope(scope_);
  Prefix::newScope(identifier);
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
  mostRecent_ = this;

  if (!local_) {
    for (Decl* d = decls; d; d = d->next()) {

      if (d->kind() == D_ATTRIBUTE) {
	Attribute* a = (Attribute*)d;

	if (a->attrType() && a->attrType()->local()) {
	  DeclaredType* dt = (DeclaredType*)a->attrType();
	  assert(dt->declRepoId());
	  char* ssn = dt->declRepoId()->scopedName()->toString();

	  IdlError(a->file(), a->line(),
		   "In unconstrained interface `%s', attribute `%s' has "
		   "local type `%s'",
		   identifier(), a->declarators()->identifier(), ssn);
	  IdlErrorCont(dt->decl()->file(), dt->decl()->line(),
		       "(%s declared here)", ssn);
	  delete [] ssn;
	}
      }
      else if (d->kind() == D_OPERATION) {
	Operation* o = (Operation*)d;

	if (o->returnType() && o->returnType()->local()) {
	  DeclaredType* dt = (DeclaredType*)o->returnType();
	  assert(dt->declRepoId());
	  char* ssn = dt->declRepoId()->scopedName()->toString();

	  IdlError(o->file(), o->line(),
		   "In unconstrained interface `%s', operation `%s' has "
		   "local return type `%s'",
		   identifier(), o->identifier(), ssn);
	  IdlErrorCont(dt->decl()->file(), dt->decl()->line(),
		       "(%s declared here)", ssn);
	  delete [] ssn;
	}
	for (Parameter* p = o->parameters(); p; p = (Parameter*)p->next()) {
	  if (p->paramType() && p->paramType()->local()) {
	    DeclaredType* dt = (DeclaredType*)p->paramType();
	    assert(dt->declRepoId());
	    char* ssn = dt->declRepoId()->scopedName()->toString();

	    IdlError(p->file(), p->line(),
		     "In unconstrained interface `%s', operation `%s' has "
		     "parameter `%s' with local type `%s'",
		     identifier(), o->identifier(), p->identifier(), ssn);
	    IdlErrorCont(dt->decl()->file(), dt->decl()->line(),
			 "(%s declared here)", ssn);
	    delete [] ssn;
	  }
	}
	for (RaisesSpec* r = o->raises(); r; r = r->next()) {
	  if (r->exception() && r->exception()->local()) {
	    char* ssn = r->exception()->scopedName()->toString();

	    IdlError(o->file(), o->line(),
		     "In unconstrained interface `%s', operation `%s' raises "
		     "local exception `%s'", identifier(),
		     o->identifier(), ssn);
	    IdlErrorCont(r->exception()->file(), r->exception()->line(),
			 "(%s declared here)", ssn);
	    delete [] ssn;
	  }
	}
      }
    }
  }
}


// Forward
Forward::
Forward(const char* file, int line, IDL_Boolean mainFile,
	const char* identifier, IDL_Boolean abstract, IDL_Boolean local)

  : Decl(D_FORWARD, file, line, mainFile),
    DeclRepoId(identifier),
    abstract_(abstract),
    local_(local),
    definition_(0),
    firstForward_(0),
    thisType_(0)
{
  Scope::Entry* se  = Scope::current()->find(identifier);
  IDL_Boolean   reg = 1;

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
      if (local && !i->local()) {
	IdlError(file, line,
		 "Forward declaration of local interface `%s' conflicts "
		 "with earlier full declaration as unconstrained",
		 identifier);
	IdlErrorCont(i->file(), i->line(),
		     "(`%s' declared as unconstrained here)");
      }
      else if (!local && i->local()) {
	IdlError(file, line,
		 "Forward declaration of unconstrained interface `%s' "
		 "conflicts with earlier full declaration as local",
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
      if (local && !f->local()) {
	IdlError(file, line,
		 "Forward declaration of local interface `%s' conflicts "
		 "with earlier forward declaration as unconstrained",
		 identifier);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward declared as unconstrained here)");
      }
      else if (!local && f->local()) {
	IdlError(file, line,
		 "Forward declaration of unconstrained interface `%s' "
		 "conflicts  with earlier forward declaration as local",
		 identifier);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward declared as local here)");
      }
      //***?      if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());
      reg = 0;
    }
  }
  if (reg) {
    if (abstract) {
      thisType_ = new DeclaredType(IdlType::tk_abstract_interface, this, this);
    }
    else if (local) {
      thisType_ = new DeclaredType(IdlType::tk_local_interface, this, this);
      thisType_->setLocal();
    }
    else {
      thisType_ = new DeclaredType(IdlType::tk_objref, this, this);
    }
    Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
  }
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
Const(const char* file, int line, IDL_Boolean mainFile,
      IdlType* constType, const char* identifier, IdlExpr* expr)

  : Decl(D_CONST, file, line, mainFile),
    DeclRepoId(identifier),
    constType_(constType)
{
  if (constType) delType_ = constType->shouldDelete();
  else           delType_ = 0;

  if (!constType || !expr) return; // Ignore nulls due to earlier errors

  IdlType* t = constType->unalias();

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
  case IdlType::tk_string:
    {
      v_.string_      = idl_strdup(expr->evalAsString());
      IDL_ULong bound = ((StringType*)t)->bound();

      if (bound && strlen(v_.string_) > bound) {
	IdlError(file, line,
		 "Length of bounded string constant exceeds bound");
      }
      break;
    }
#ifdef HAS_LongLong
  case IdlType::tk_longlong:   v_.longlong_   = expr->evalAsLongLong();  break;
  case IdlType::tk_ulonglong:  v_.ulonglong_  = expr->evalAsULongLong(); break;
#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble: v_.longdouble_ = expr->evalAsLongDouble();break;
#endif
  case IdlType::tk_wchar:      v_.wchar_      = expr->evalAsWChar();     break;
  case IdlType::tk_wstring:
    {
      v_.wstring_     = idl_wstrdup(expr->evalAsWString());
      IDL_ULong bound = ((WStringType*)t)->bound();

      if (bound && (unsigned)idl_wstrlen(v_.wstring_) > bound) {
	IdlError(file, line,
		 "Length of bounded wide string constant exceeds bound");
      }
      break;
    }
  case IdlType::tk_fixed:
    {
      IDL_Fixed* f = expr->evalAsFixed();

      FixedType* ft = (FixedType*)t;
      if (ft->digits()) {
	// Check constant fits in the target type
	IDL_Fixed* g = new IDL_Fixed(f->truncate(ft->scale()));

	if (g->fixed_digits() > ft->digits()) {
	  IdlError(file, line,
		   "Fixed point constant has too many digits to "
		   "fit fixed<%u,%u>",
		   ft->digits(), ft->scale());
	}
	else if (f->fixed_scale() > g->fixed_scale()) {
	  IdlWarning(file, line,
		     "Fixed point constant truncated to fit fixed<%u,%u>",
		     ft->digits(), ft->scale());
	}
	delete f;
	f = g;
      }
      v_.fixed_ = f;
      break;
    }
  case IdlType::tk_enum:
    v_.enumerator_ = expr->evalAsEnumerator((Enum*)((DeclaredType*)t)->decl());
    break;

  default:
    IdlError(file, line, "Invalid type for constant: %s", t->kindAsString());
    break;
  }
  delete expr;

  Scope::current()->addDecl(identifier, 0, this, constType, file, line);
}

Const::
~Const()
{
  if (constKind_ == IdlType::tk_string)  delete [] v_.string_;
  if (constKind_ == IdlType::tk_wstring) delete [] v_.wstring_;
  if (constKind_ == IdlType::tk_fixed)   delete    v_.fixed_;
  if (delType_) delete constType_;
}

#define CONST_AS(rt, op, tk, un) \
rt Const::op() const { \
  assert(constKind_ == IdlType::tk); \
  return v_.un; \
}

CONST_AS(IDL_Short,        constAsShort,      tk_short,      short_)
CONST_AS(IDL_Long,         constAsLong,       tk_long,       long_)
CONST_AS(IDL_UShort,       constAsUShort,     tk_ushort,     ushort_)
CONST_AS(IDL_ULong,        constAsULong,      tk_ulong,      ulong_)
CONST_AS(IDL_Float,        constAsFloat,      tk_float,      float_)
CONST_AS(IDL_Double,       constAsDouble,     tk_double,     double_)
CONST_AS(IDL_Boolean,      constAsBoolean,    tk_boolean,    boolean_)
CONST_AS(IDL_Char,         constAsChar,       tk_char,       char_)
CONST_AS(IDL_Octet,        constAsOctet,      tk_octet,      octet_)
CONST_AS(const char*,      constAsString,     tk_string,     string_)
#ifdef HAS_LongLong
CONST_AS(IDL_LongLong,     constAsLongLong,   tk_longlong,   longlong_)
CONST_AS(IDL_ULongLong,    constAsULongLong,  tk_ulonglong,  ulonglong_)
#endif
#ifdef HAS_LongDouble
CONST_AS(IDL_LongDouble,   constAsLongDouble, tk_longdouble, longdouble_)
#endif
CONST_AS(IDL_WChar,        constAsWChar,      tk_wchar,      wchar_)
CONST_AS(const IDL_WChar*, constAsWString,    tk_wstring,    wstring_)
CONST_AS(Enumerator*,      constAsEnumerator, tk_enum,       enumerator_)

IDL_Fixed*
Const::constAsFixed() const {
  assert(constKind_ == IdlType::tk_fixed);

  // Have to copy the fixed object to get the memory management right
  return new IDL_Fixed(*v_.fixed_);
}


// Declarator
Declarator::
Declarator(const char* file, int line, IDL_Boolean mainFile,
	   const char* identifier, ArraySize* sizes)

  : Decl(D_DECLARATOR, file, line, mainFile),
    DeclRepoId(identifier),
    sizes_(sizes),
    thisType_(0),
    alias_(0),
    attribute_(0)
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
  if (alias_)     return "typedef declarator";
  if (attribute_) return "attribute declarator";
                  return "declarator";
}

void
Declarator::
setAlias(Typedef* td)
{
  alias_    = td;
  thisType_ = new DeclaredType(IdlType::tk_alias, this, this);
  if (td->aliasType() && td->aliasType()->local()) thisType_->setLocal();
  if (sizes_) checkValidType(file(), line(), td->aliasType());
}

void
Declarator::
setAttribute(Attribute* at)
{
  attribute_ = at;
}

// Typedef
Typedef::
Typedef(const char* file, int line, IDL_Boolean mainFile,
	IdlType* aliasType, IDL_Boolean constrType,
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
Member(const char* file, int line, IDL_Boolean mainFile,
       IdlType* memberType, IDL_Boolean constrType,
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
  checkNotForward(file, line, memberType);

  IdlType* bareType = memberType->unalias();

  if (bareType->kind() == IdlType::tk_struct) {
    Struct* s = (Struct*)((DeclaredType*)bareType)->decl();
    if (!s->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of struct `%s' inside "
	       "its own definition", s->identifier());
    }
  }
  else if (bareType->kind() == IdlType::tk_union) {
    Union* u = (Union*)((DeclaredType*)bareType)->decl();
    if (!u->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of union `%s' inside "
	       "its own definition", u->identifier());
    }
  }
  else if (bareType->kind() == IdlType::tk_sequence) {
    // Look for recursive sequence
    IdlType* t = bareType;
    while (t && t->kind() == IdlType::tk_sequence)
      t = ((SequenceType*)t)->seqType()->unalias();

    if (!t) return; // Sequence of undeclared type

    if (t->kind() == IdlType::tk_struct) {
      Struct* s = (Struct*)((DeclaredType*)t)->decl();
      if (!s->finished()) {
	s->setRecursive();
	IdlWarning(file, line,
		   "Anonymous sequences for recursive structures "
		   "are deprecated in CORBA 2.4. Use a forward "
		   "declaration instead.");
      }
    }
    else if (t->kind() == IdlType::tk_union) {
      Union* u = (Union*)((DeclaredType*)t)->decl();
      if (!u->finished()) {
	u->setRecursive();
	IdlWarning(file, line,
		   "Anonymous sequences for recursive unions "
		   "are deprecated in CORBA 2.4. Use a forward "
		   "declaration instead.");
      }
    }
    else if (t->kind() == IdlType::ot_structforward) {
      StructForward* f = (StructForward*)((DeclaredType*)t)->decl();
      Struct* s = f->definition();
      if (s) {
	if (!s->finished())
	  s->setRecursive();
      }
      else {
	char* ssn = f->scopedName()->toString();
	IdlError(file, line,
		 "Cannot use sequence of forward-declared struct `%s' "
		 "before it is fully defined", ssn);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward-declared here)", f->identifier());
	delete [] ssn;
      }
    }
    else if (t->kind() == IdlType::ot_unionforward) {
      UnionForward* f = (UnionForward*)((DeclaredType*)t)->decl();
      Union* u = f->definition();
      if (u) {
	if (!u->finished())
	  u->setRecursive();
      }
      else {
	char* ssn = f->scopedName()->toString();
	IdlError(file, line,
		 "Cannot use sequence of forward-declared union `%s' "
		 "before it is fully defined", ssn);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward-declared here)", f->identifier());
	delete [] ssn;
      }
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
Struct(const char* file, int line, IDL_Boolean mainFile,
       const char* identifier)

  : Decl(D_STRUCT, file, line, mainFile),
    DeclRepoId(identifier),
    members_(0),
    recursive_(0),
    finished_(0)
{
  // Look for forward struct
  Scope::Entry* se = Scope::current()->find(identifier);

  if (se &&
      se->kind() == Scope::Entry::E_DECL &&
      se->decl()->kind() == Decl::D_STRUCTFORWARD) {

    StructForward* f = (StructForward*)se->decl();

    if (strcmp(f->file(), file)) {
      IdlError(file, line,
	       "Struct `%s' defined in different source file to "
	       "its forward declaration", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here)", identifier);
    }
    if (strcmp(f->prefix(), prefix())) {
      IdlError(file, line,
	       "In declaration of struct `%s', repository id "
	       "prefix `%s' differs from that of forward declaration",
	       identifier, prefix());

      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here with prefix `%s')",
		   f->identifier(), f->prefix());
    }
    if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());
    f->setDefinition(this);
    Scope::current()->remEntry(se);
  }
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
  // Is this a local type?
  for (Member* m = members; m; m = (Member*)m->next()) {
    if (m->memberType() && m->memberType()->local()) {
      thisType()->setLocal();
      break;
    }
  }
  members_ = members;
  Prefix::endScope();
  Scope::endScope();
  finished_ = 1;
  mostRecent_ = this;
}

// StructForward
StructForward::
StructForward(const char* file, int line, IDL_Boolean mainFile,
	      const char* identifier)
  : Decl(D_STRUCTFORWARD, file, line, mainFile),
    DeclRepoId(identifier),
    definition_(0),
    firstForward_(0),
    thisType_(0)
{
  Scope::Entry* se  = Scope::current()->find(identifier);
  IDL_Boolean   reg = 1;

  if (se && se->kind() == Scope::Entry::E_DECL) {

    if (se->decl()->kind() == D_STRUCT) {
      Struct* s = (Struct*)se->decl();
      definition_ = s;

      if (strcmp(s->file(), file)) {
	IdlError(file, line,
		 "Struct `%s' forward declared in different source file to "
		 "its definition", identifier);
	IdlErrorCont(s->file(), s->line(),
		     "(`%s' defined here)", identifier);
      }
      if (strcmp(s->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of struct `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(s->file(), s->line(),
		     "(`%s' fully declared here with prefix `%s')",
		     s->identifier(), s->prefix());
      }
      reg = 0;
    }
    else if (se->decl()->kind() == D_STRUCTFORWARD) {
      StructForward* s = (StructForward*)se->decl();
      firstForward_ = s;

      if (strcmp(s->file(), file)) {
	IdlError(file, line,
		 "Struct `%s' forward declared in more than one "
		 "source file", identifier);
	IdlErrorCont(s->file(), s->line(),
		     "(`%s' also forward declared here)", identifier);
      }
      if (strcmp(s->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of struct `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(s->file(), s->line(),
		     "(`%s' forward declared here with prefix `%s')",
		     s->identifier(), s->prefix());
      }
      reg = 0;
    }
  }
  if (reg) {
    thisType_ = new DeclaredType(IdlType::ot_structforward, this, this);
    Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
  }
}

StructForward::
~StructForward()
{
  delete thisType_;
}

Struct*
StructForward::
definition() const
{
  if (firstForward_)
    return firstForward_->definition();
  else
    return definition_;
}

void
StructForward::
setDefinition(Struct* defn)
{
  definition_ = defn;
}



// Exception
Exception::
Exception(const char* file, int line, IDL_Boolean mainFile,
	  const char* identifier)

  : Decl(D_EXCEPTION, file, line, mainFile),
    DeclRepoId(identifier),
    members_(0),
    local_(0)
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
  // Is this a local exception?
  for (Member* m = members; m; m = (Member*)m->next()) {
    if (m->memberType() && m->memberType()->local()) {
      local_ = 1;
      break;
    }
  }
  members_ = members;
  Prefix::endScope();
  Scope::endScope();
  mostRecent_ = this;
}


// Case label
CaseLabel::
CaseLabel(const char* file, int line, IDL_Boolean mainFile,
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

LABEL_AS(IDL_Short,        labelAsShort,      tk_short,      short_)
LABEL_AS(IDL_Long,         labelAsLong,       tk_long,       long_)
LABEL_AS(IDL_UShort,       labelAsUShort,     tk_ushort,     ushort_)
LABEL_AS(IDL_ULong,        labelAsULong,      tk_ulong,      ulong_)
LABEL_AS(IDL_Boolean,      labelAsBoolean,    tk_boolean,    boolean_)
LABEL_AS(IDL_Char,         labelAsChar,       tk_char,       char_)
#ifdef HAS_LongLong
LABEL_AS(IDL_LongLong,     labelAsLongLong,   tk_longlong,   longlong_)
LABEL_AS(IDL_ULongLong,    labelAsULongLong,  tk_ulonglong,  ulonglong_)
#endif
LABEL_AS(IDL_WChar,        labelAsWChar,      tk_wchar,      wchar_)
LABEL_AS(Enumerator*,      labelAsEnumerator, tk_enum,       enumerator_)


// UnionCase
UnionCase::
UnionCase(const char* file, int line, IDL_Boolean mainFile,
	  IdlType* caseType, IDL_Boolean constrType,
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
  checkNotForward(file, line, caseType);

  IdlType* bareType = caseType->unalias();

  if (bareType->kind() == IdlType::tk_struct) {
    Struct* s = (Struct*)((DeclaredType*)bareType)->decl();
    if (!s->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of struct `%s' inside "
	       "its own definition", s->identifier());
    }
  }
  else if (bareType->kind() == IdlType::tk_union) {
    Union* u = (Union*)((DeclaredType*)bareType)->decl();
    if (!u->finished()) {
      IdlError(file, line,
	       "Cannot create an instance of union `%s' inside "
	       "its own definition", u->identifier());
    }
  }
  else if (bareType->kind() == IdlType::tk_sequence) {
    // Look for recursive sequence
    IdlType* t = bareType;
    while (t && t->kind() == IdlType::tk_sequence)
      t = ((SequenceType*)t)->seqType()->unalias();

    if (!t) return; // Sequence of undeclared type

    if (t->kind() == IdlType::tk_struct) {
      Struct* s = (Struct*)((DeclaredType*)t)->decl();
      if (!s->finished()) {
	s->setRecursive();
	IdlWarning(file, line,
		   "Anonymous sequences for recursive structures "
		   "are deprecated in CORBA 2.4. Use a forward "
		   "declaration instead.");
      }
    }
    else if (t->kind() == IdlType::tk_union) {
      Union* u = (Union*)((DeclaredType*)t)->decl();
      if (!u->finished()) {
	u->setRecursive();
	IdlWarning(file, line,
		   "Anonymous sequences for recursive unions "
		   "are deprecated in CORBA 2.4. Use a forward "
		   "declaration instead.");
      }
    }
    else if (t->kind() == IdlType::ot_structforward) {
      StructForward* f = (StructForward*)((DeclaredType*)t)->decl();
      Struct* s = f->definition();
      if (s) {
	if (!s->finished())
	  s->setRecursive();
      }
      else {
	char* ssn = f->scopedName()->toString();
	IdlError(file, line,
		 "Cannot use sequence of forward-declared struct `%s' "
		 "before it is fully defined", ssn);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward-declared here)", f->identifier());
	delete [] ssn;
      }
    }
    else if (t->kind() == IdlType::ot_unionforward) {
      UnionForward* f = (UnionForward*)((DeclaredType*)t)->decl();
      Union* u = f->definition();
      if (u) {
	if (!u->finished())
	  u->setRecursive();
      }
      else {
	char* ssn = f->scopedName()->toString();
	IdlError(file, line,
		 "Cannot use sequence of forward-declared union `%s' "
		 "before it is fully defined", ssn);
	IdlErrorCont(f->file(), f->line(),
		     "(`%s' forward-declared here)", f->identifier());
	delete [] ssn;
      }
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
  mostRecent_ = this;
}


// Union
Union::
Union(const char* file, int line, IDL_Boolean mainFile,
      const char* identifier)

  : Decl(D_UNION, file, line, mainFile),
    DeclRepoId(identifier),
    switchType_(0),
    constrType_(0),
    cases_(0),
    recursive_(0),
    finished_(0)
{
  // Look for forward union
  Scope::Entry* se = Scope::current()->find(identifier);

  if (se &&
      se->kind() == Scope::Entry::E_DECL &&
      se->decl()->kind() == Decl::D_UNIONFORWARD) {

    UnionForward* f = (UnionForward*)se->decl();

    if (strcmp(f->file(), file)) {
      IdlError(file, line,
	       "Union `%s' defined in different source file to "
	       "its forward declaration", identifier);
      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here)", identifier);
    }
    if (strcmp(f->prefix(), prefix())) {
      IdlError(file, line,
	       "In declaration of union `%s', repository id "
	       "prefix `%s' differs from that of forward declaration",
	       identifier, prefix());

      IdlErrorCont(f->file(), f->line(),
		   "(`%s' forward declared here with prefix `%s')",
		   f->identifier(), f->prefix());
    }
    if (f->repoIdSet()) setRepoId(f->repoId(), f->rifile(), f->riline());
    f->setDefinition(this);
    Scope::current()->remEntry(se);
  }
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
finishConstruction(IdlType* switchType, IDL_Boolean constrType,
		   UnionCase* cases)
{
  if (!switchType) return;

  switchType_ = switchType;
  constrType_ = constrType;
  cases_      = cases;
  finished_   = 1;

  // Local type?
  UnionCase* c;
  for (c = cases; c; c = (UnionCase*)c->next()) {
    if (c->caseType() && c->caseType()->local()) {
      thisType_->setLocal();
      break;
    }
  }
  IdlType* t  = switchType->unalias();

  if (!t) { // Broken alias due to earlier error
    Prefix::endScope();
    Scope::endScope();
    return;
  }
  UnionCase     *d;
  CaseLabel     *l, *m, *defLabel = 0;
  IdlType::Kind k     = t->kind();
  int           clash = 0;
  IDL_Boolean   foundDef = 0;

  switch (k) {
  case IdlType::tk_short:
    UNION_SWITCH(IDL_Short, Short, -0x8000, defVal==0x7fff, ++defVal)
  case IdlType::tk_long:
    UNION_SWITCH(IDL_Long, Long, -0x80000000, defVal==0x7fffffff, ++defVal)
  case IdlType::tk_ushort:
    UNION_SWITCH(IDL_UShort, UShort, 0xffff, defVal==0, --defVal)
  case IdlType::tk_ulong:
    UNION_SWITCH(IDL_ULong, ULong, 0xffffffff, defVal==0, --defVal)
  case IdlType::tk_boolean:
    UNION_SWITCH(IDL_Boolean, Boolean, 0, defVal==1, defVal=1)
  case IdlType::tk_char:
    UNION_SWITCH(IDL_Char, Char, 0xff, defVal==0, --defVal)
#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    UNION_SWITCH(IDL_LongLong, LongLong,
		 _CORBA_LONGLONG_CONST(-0x8000000000000000),
		 defVal==_CORBA_LONGLONG_CONST(0x7fffffffffffffff), ++defVal)
  case IdlType::tk_ulonglong:
    UNION_SWITCH(IDL_ULongLong, ULongLong,
		 _CORBA_LONGLONG_CONST(0xffffffffffffffff),
		 defVal==_CORBA_LONGLONG_CONST(0), --defVal)
#endif
  case IdlType::tk_wchar:
    UNION_SWITCH(IDL_WChar, WChar, 0xffff, defVal==0, --defVal)

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
  mostRecent_ = this;
}

// UnionForward

UnionForward::
UnionForward(const char* file, int line, IDL_Boolean mainFile,
	     const char* identifier)
  : Decl(D_UNIONFORWARD, file, line, mainFile),
    DeclRepoId(identifier),
    definition_(0),
    firstForward_(0),
    thisType_(0)
{
  Scope::Entry* se  = Scope::current()->find(identifier);
  IDL_Boolean   reg = 1;

  if (se && se->kind() == Scope::Entry::E_DECL) {

    if (se->decl()->kind() == D_UNION) {
      Union* u = (Union*)se->decl();
      definition_ = u;

      if (strcmp(u->file(), file)) {
	IdlError(file, line,
		 "Union `%s' forward declared in different source file to "
		 "its definition", identifier);
	IdlErrorCont(u->file(), u->line(),
		     "(`%s' defined here)", identifier);
      }
      if (strcmp(u->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of union `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(u->file(), u->line(),
		     "(`%s' fully declared here with prefix `%s')",
		     u->identifier(), u->prefix());
      }
      reg = 0;
    }
    else if (se->decl()->kind() == D_UNIONFORWARD) {
      UnionForward* u = (UnionForward*)se->decl();
      firstForward_ = u;

      if (strcmp(u->file(), file)) {
	IdlError(file, line,
		 "Union `%s' forward declared in more than one "
		 "source file", identifier);
	IdlErrorCont(u->file(), u->line(),
		     "(`%s' also forward declared here)", identifier);
      }
      if (strcmp(u->prefix(), prefix())) {
	IdlError(file, line,
		 "In forward declaration of union `%s', repository "
		 "id prefix `%s' differs from that of earlier declaration",
		 identifier, prefix());

	IdlErrorCont(u->file(), u->line(),
		     "(`%s' forward declared here with prefix `%s')",
		     u->identifier(), u->prefix());
      }
      reg = 0;
    }
  }
  if (reg) {
    thisType_ = new DeclaredType(IdlType::ot_unionforward, this, this);
    Scope::current()->addDecl(identifier, 0, this, thisType_, file, line);
  }
}

UnionForward::
~UnionForward()
{
  delete thisType_;
}

Union*
UnionForward::
definition() const
{
  if (firstForward_)
    return firstForward_->definition();
  else
    return definition_;
}

void
UnionForward::
setDefinition(Union* defn)
{
  definition_ = defn;
}


// Enum

Enumerator::
Enumerator(const char* file, int line, IDL_Boolean mainFile,
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
Enum(const char* file, int line, IDL_Boolean mainFile,
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
  mostRecent_ = this;
}


// Attribute
Attribute::
Attribute(const char* file, int line, IDL_Boolean mainFile,
	  IDL_Boolean readonly, IdlType* attrType,
	  Declarator* declarators)

  : Decl(D_ATTRIBUTE, file, line, mainFile),
    readonly_(readonly),
    attrType_(attrType),
    declarators_(declarators)
{
  if (attrType) {
    delType_ = attrType->shouldDelete();
    checkValidType(file, line, attrType);
  }
  else
    delType_ = 0;

  for (Declarator* d = declarators; d; d = (Declarator*)d->next()) {
    assert(d->sizes() == 0); // Enforced by grammar
    d->setAttribute(this);
    Scope::current()->addCallable(d->eidentifier(), 0, d, file, line);
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
Parameter(const char* file, int line, IDL_Boolean mainFile,
	  int direction, IdlType* paramType, const char* identifier)

  : Decl(D_PARAMETER, file, line, mainFile),
    direction_(direction),
    paramType_(paramType)
{
  if (paramType) {
    delType_ = paramType->shouldDelete();
    checkValidType(file, line, paramType);
  }
  else
    delType_ = 0;

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
      char* ssn = sn->toString();
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

  IDL_Boolean bad = 0;

  if (!isalpha(*c++))
    bad = 1;
  else {
    for (; *c; c++) {
      if (!isalnum(*c) && *c != '.' && *c != '_') {
	if (!(*c == '*' && *(c+1) == '\0'))
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
Operation(const char* file, int line, IDL_Boolean mainFile,
	  IDL_Boolean oneway, IdlType* returnType,
	  const char* identifier)

  : Decl(D_OPERATION, file, line, mainFile),
    DeclRepoId(identifier),
    oneway_(oneway),
    returnType_(returnType),
    parameters_(0),
    raises_(0),
    contexts_(0)
{
  if (returnType) {
    delType_ = returnType->shouldDelete();
    checkValidType(file, line, returnType);
  }
  else
    delType_ = 0;

  Scope* s = Scope::current()->newOperationScope(file, line);
  Scope::current()->addCallable(identifier, s, this, file, line);
  Scope::startScope(s);
}

Operation::
~Operation()
{
  if (parameters_) delete parameters_;
  if (raises_)     delete raises_;
  if (contexts_)   delete contexts_;
  if (delType_)    delete returnType_;
}


void
Operation::
closeParens()
{
  mostRecent_ = this;
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
Native(const char* file, int line, IDL_Boolean mainFile,
       const char* identifier)

  : Decl(D_NATIVE, file, line, mainFile),
    DeclRepoId(identifier)
{
  DeclaredType* type = new DeclaredType(IdlType::tk_native, this, this);
  Scope::current()->addDecl(identifier, 0, this, type, file, line);
}

Native::
~Native()
{
}


// ValueType

StateMember::
StateMember(const char* file, int line, IDL_Boolean mainFile,
	    int memberAccess, IdlType* memberType,
	    IDL_Boolean constrType, Declarator* declarators)

  : Decl(D_STATEMEMBER, file, line, mainFile),
    memberAccess_(memberAccess),
    memberType_(memberType),
    constrType_(constrType),
    declarators_(declarators)
{
  if (memberType) {
    delType_ = memberType->shouldDelete();
    checkValidType(file, line, memberType);

    if (memberType->local()) {
      if (memberType->kind() != IdlType::tk_sequence) {
	DeclaredType* dt = (DeclaredType*)memberType;
	assert(dt->declRepoId());
	char* ssn = dt->declRepoId()->scopedName()->toString();

	IdlError(file, line, "State member `%s' has local type `%s'",
		 declarators->identifier(), ssn);
	IdlErrorCont(dt->decl()->file(), dt->decl()->line(),
		     "(%s declared here)", ssn);
	delete [] ssn;
      }
      else {
	IdlError(file, line, "State member `%s' has local type",
		 declarators->identifier());
      }
    }
  }
  else {
    delType_ = 0;
  }

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
Factory(const char* file, int line, IDL_Boolean mainFile,
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
closeParens()
{
  mostRecent_ = this;
}


void
Factory::
finishConstruction(Parameter* parameters)
{
  parameters_ = parameters_;
  Scope::endScope();
}

ValueBase::
ValueBase(Decl::Kind k, const char* file, int line, IDL_Boolean mainFile,
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
ValueForward(const char* file, int line, IDL_Boolean mainFile,
	     IDL_Boolean abstract, const char* identifier)

  : ValueBase(D_VALUEFORWARD, file, line, mainFile, identifier),
    abstract_(abstract),
    definition_(0),
    firstForward_(0)
{
  Scope::Entry*  se  = Scope::current()->find(identifier);
  IDL_Boolean reg = 1;

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
ValueBox(const char* file, int line, IDL_Boolean mainFile,
	 const char* identifier, IdlType* boxedType,
	 IDL_Boolean constrType)

  : ValueBase(D_VALUEBOX, file, line, mainFile, identifier),
    boxedType_(boxedType),
    constrType_(constrType)
{
  checkValidType(file, line, boxedType);
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
      IdlType* t = se->idltype()->unalias();

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
	  ValueBase* def = ((ValueForward*)d)->definition();

	  if (def) {
	    if (def->kind() == Decl::D_VALUE) {
	      value_ = (Value*)def;
	      scope_ = ((Value*)def)->scope();
	      return;
	    }
	    else if (def->kind() == Decl::D_VALUEABS) {
	      value_ = (ValueAbs*)def;
	      scope_ = ((ValueAbs*)def)->scope();
	      return;
	    }
	  }
	  else {
	    char* ssn = ((ValueForward*)d)->scopedName()->toString();
	    IdlError(file, line,
		     "Inherited valuetype `%s' must be fully defined", ssn);

	    if (decl_ != d) {
	      char* tssn = sn->toString();
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
    }
    char* ssn = sn->toString();
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
      char* ssn = is->value()->scopedName()->toString();
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
ValueAbs(const char* file, int line, IDL_Boolean mainFile,
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
    if (vinh->value()->kind() == D_VALUE) {
      char* ssn = vinh->scope()->scopedName()->toString();
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
  mostRecent_ = this;
}

Value::
Value(const char* file, int line, IDL_Boolean mainFile,
      IDL_Boolean custom, const char* identifier,
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
	  char* ssn = inherits->scope()->scopedName()->toString();
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
	char* ssn = vinh->scope()->scopedName()->toString();
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
	char* ssn = inh->scope()->scopedName()->toString();
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
  mostRecent_ = this;
}
