// -*- c++ -*-
//                          Package   : omniidl
// idlscope.cc              Created on: 1999/10/11
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
//   Scope manipulation

// $Id$
// $Log$
// Revision 1.11.2.8  2000/11/30 11:40:48  dpg1
// Add -nc option to omniidl to accept invalid IDL with identifiers
// differing only by case.
//
// Revision 1.11.2.7  2000/10/24 09:53:30  dpg1
// Clean up omniidl system dependencies. Replace use of _CORBA_ types
// with IDL_ types.
//
// Revision 1.11.2.6  2000/09/19 09:14:26  dpg1
// Scope::Entry::Kind renamed to Scope::Entry::EntryKind to avoid
// problems with over-keen compilers
//
// Revision 1.11.2.5  2000/08/29 15:20:28  dpg1
// New relativeScope() function. New -i flag to enter interactive loop
// after parsing
//
// Revision 1.11.2.4  2000/08/24 11:33:00  dpg1
// Typo in error message % format string
//
// Revision 1.11.2.3  2000/08/04 09:10:27  dpg1
// Fix look-up of escaped identifiers broken on 19 July. (Bug 14.)
//
// Revision 1.11.2.2  2000/08/01 09:46:47  dpg1
// No longer complain about inheriting an operation into an interface
// with the same name.
//
// Revision 1.11.2.1  2000/07/19 17:24:54  dpg1
// omniidl complains if a name which differs only in case is found during
// name look-up
//
// Revision 1.11  1999/11/26 11:33:44  dpg1
// Bug in findWithInheritance() when inherited interface was not found.
//
// Revision 1.10  1999/11/17 17:23:54  dpg1
// Minor bug when scoped name look-up encounters an identifier which does
// not form a scope.
//
// Revision 1.9  1999/11/17 17:17:00  dpg1
// Changes to remove static initialisation of objects.
//
// Revision 1.8  1999/11/11 10:20:30  dpg1
// Bug in marking use of fully-scoped names.
//
// Revision 1.7  1999/11/04 17:16:55  dpg1
// Changes for NT.
//
// Revision 1.6  1999/11/02 17:07:25  dpg1
// Changes to compile on Solaris.
//
// Revision 1.5  1999/11/02 12:10:51  dpg1
// Small bug in addUse()
//
// Revision 1.4  1999/11/02 10:35:03  dpg1
// add...() functions now carry on regardless after a keyword clash, to
// prevent later errors.
//
// Revision 1.3  1999/11/02 10:01:46  dpg1
// Minor fixes.
//
// Revision 1.2  1999/10/29 10:01:50  dpg1
// Global scope initialisation changed.
//
// Revision 1.1  1999/10/27 14:05:55  dpg1
// *** empty log message ***
//

#include <idlscope.h>
#include <idlast.h>
#include <idlerr.h>
#include <idlutil.h>
#include <idlconfig.h>

#include <string.h>

// Global Scope pointers
Scope* Scope::global_  = 0;
Scope* Scope::current_ = 0;

// ScopedName implementation
ScopedName::
ScopedName(const char* identifier, IDL_Boolean absolute) :
  absolute_(absolute)
{
  Fragment* f = new Fragment(identifier);
  scopeList_  = f;
  last_       = f;
}

ScopedName::
ScopedName(const ScopedName* sn) :
  scopeList_(0), last_(0), absolute_(sn->absolute())
{
  const Fragment *f;

  for (f = sn->scopeList(); f; f = f->next())
    append(f->identifier());
}

ScopedName::
ScopedName(const ScopedName::Fragment* frags, IDL_Boolean absolute) :
  scopeList_(0), last_(0), absolute_(absolute)
{
  const Fragment *f;

  for (f = frags; f; f = f->next())
    append(f->identifier());
}


ScopedName::
~ScopedName()
{
  Fragment* f;
  Fragment* g;

  for (f = scopeList_; f; f = g) {
    g = f->next_;
    delete f;
  }
}

char*
ScopedName::
toString() const
{
  int       i;
  Fragment* f;

  i = absolute_ ? 2 : 0;

  for (f = scopeList_; f; f = f->next())
    i += strlen(f->identifier()) + 2;

  char* str = new char [i-1];

  if (absolute_) {
    str[0] = ':'; str[1] = ':';
    i = 2;
  }
  else
    i = 0;

  const char* fi;
  for (f = scopeList_; f; f = f->next()) {

    for (fi = f->identifier(); *fi; fi++, i++)
      str[i] = *fi;

    if (f->next()) {
      str[i++] = ':'; str[i++] = ':';
    }
  }
  str[i] = '\0';
  return str;
}

IDL_Boolean
ScopedName::
equal(const ScopedName* sn) const
{
  if (sn->absolute() != absolute()) return 0;

  Fragment *ourf, *theirf;

  // Compare fragments
  for (ourf = scopeList(), theirf = sn->scopeList();
       ourf && theirf;
       ourf = ourf->next(), theirf = theirf->next()) {

    if (strcmp(ourf->identifier(), theirf->identifier()) != 0)
      return 0;
  }

  // If either name has fragments left, names aren't equal
  if (ourf || theirf) return 0;

  return 1;
}

void
ScopedName::
append(const char* identifier)
{
  Fragment* f = new Fragment(identifier);

  if (last_)
    last_->next_ = f;
  else
    scopeList_ = f;

  last_ = f;
}


// Scope implementation

Scope::
Entry::
Entry(const Scope* container, EntryKind k, const char* identifier,
      Scope* scope, Decl* decl, IdlType* idltype,
      Scope::Entry* inh_from, const char* file, int line)

  : container_(container), kind_(k), identifier_(idl_strdup(identifier)),
    scope_(scope), decl_(decl), idltype_(idltype), inh_from_(inh_from),
    file_(idl_strdup(file)), line_(line), next_(0)
{
  const ScopedName* sn = container->scopedName();

  if (identifier) {
    if (sn) {
      scopedName_ = new ScopedName(sn);
      scopedName_->append(identifier);
    }
    else
      scopedName_ = new ScopedName(identifier, 1);
  }
  else
    scopedName_ = 0;
}

Scope::
Entry::
~Entry()
{
  if (scopedName_) delete scopedName_;
  if (identifier_) delete [] identifier_;
  if (file_)       delete [] file_;
}

void
Scope::
EntryList::
merge(Scope::EntryList* ml)
{
  EntryList*     l;
  IDL_Boolean add;

  for (; ml; ml = ml->tail()) {
    add = 1;
    for (l=this; l; l = l->tail()) {

      if (ml->head() == l->head()) {
	add = 0;
	break;
      }
    }
    if (add) append(new EntryList(ml->head()));
  }
  delete ml;
}


Scope::
Scope(Scope* parent, Scope::Kind k, IDL_Boolean nestedUse,
      const char* file, int line)

  : parent_(parent), kind_(k), identifier_(0), scopedName_(0),
    nestedUse_(nestedUse), entries_(0), last_(0),
    inherited_(0), valueInherited_(0)
{
  if (parent)
    nestedUse_ |= parent->nestedUse();
}

Scope::
Scope(Scope* parent, const char* identifier, Scope::Kind k,
      IDL_Boolean nestedUse,
      const char* file, int line)

  : parent_(parent), kind_(k), nestedUse_(nestedUse),
    inherited_(0), valueInherited_(0)
{
  const ScopedName* psn = 0;

  if (identifier && identifier[0] == '_') ++identifier;
  identifier_ = idl_strdup(identifier);

  if (parent) {
    psn         = parent->scopedName();
    nestedUse_ |= parent->nestedUse();
  }

  if (psn) {
    scopedName_ = new ScopedName(psn);
    scopedName_->append(identifier);
  }
  else
    scopedName_ = new ScopedName(identifier, 1);

  // Add PARENT entry
  entries_ = new Entry(this, Entry::E_PARENT, identifier, 0, 0, 0, 0,
		       file, line);
  last_    = entries_;
}

Scope::
~Scope()
{
  Entry *e, *f;
  for (e=entries_; e; e=f) {
    f = e->next();
    delete e;
  }
  if (identifier_) delete [] identifier_;
  if (scopedName_) delete    scopedName_;
}

void
Scope::
init()
{
  const char* file = "<built in>";

  assert(!global_);

  global_  = new Scope(0, Scope::S_GLOBAL, 0, file, 0);
  Scope* s = global_->newModuleScope("CORBA", file, 1);

  s->addDecl("TypeCode",  0, 0, BaseType::TypeCodeType,  file, 2);
  s->addDecl("Principal", 0, 0, BaseType::PrincipalType, file, 3);

  global_->addModule("CORBA", s, 0, file, 1);
  current_ = global_;
}

void
Scope::
clear()
{
  assert(global_);
  delete global_;
  global_ = 0;
}

void
Scope::
setInherited(InheritSpec* inherited, const char* file, int line)
{
  inherited_ = inherited;

  InheritSpec* is;
  Entry*       e;

  for (is = inherited; is; is = is->next()) {
    if (!is->scope()) continue; // Skip bad entries from earlier errors

    for (e = is->scope()->entries(); e; e = e->next()) {

      switch (e->kind()) {
      case Entry::E_CALLABLE:
	addInherited(e->identifier(), e->scope(), e->decl(), e, file, line);
	break;
      case Entry::E_INHERITED:
	addInherited(e->identifier(), e->scope(), e->decl(), e->inh_from(),
		     file, line);
	break;
      default:
	break;
      }
    }
  }
}

void
Scope::
setInherited(ValueInheritSpec* inherited, const char* file, int line)
{
  valueInherited_ = inherited;

  ValueInheritSpec* is;
  Entry*            e;

  for (is = inherited; is; is = is->next()) {
    if (!is->scope()) continue; // Skip bad entries from earlier errors

    for (e = is->scope()->entries(); e; e = e->next()) {

      switch (e->kind()) {
      case Entry::E_CALLABLE:
      case Entry::E_INHERITED:
	addInherited(e->identifier(), e->scope(), e->decl(), e, file, line);
	break;
      default:
	break;
      }
    }
  }
}

void
Scope::
appendEntry(Entry* e)
{
  if (entries_) last_->next_ = e;
  else entries_ = e;
  last_ = e;
}

void
Scope::
startScope(Scope* s)
{
  assert(s->parent() == current_);
  current_ = s;
}

void
Scope::
endScope()
{
  assert(current_);
  current_ = current_->parent();
  assert(current_);
}

// Scope creation functions

Scope*
Scope::
newModuleScope(const char* identifier, const char* file, int line)
{
  // Only the global scope and modules can contain modules. The
  // grammar enforces this
  assert(kind() == S_GLOBAL || kind() == S_MODULE);

  // If there is already a module entry for this name, return it
  Entry* e = find(identifier);
  if (e && e->kind() == Entry::E_MODULE)
    return e->scope();

  // If there was an entry but it wasn't a module, carry on anyway,
  // since addModule() will report the error

  return new Scope(this, identifier, S_MODULE, 0, file, line);
}

Scope*
Scope::
newInterfaceScope(const char* identifier, const char* file, int line)
{
  assert(kind() == S_GLOBAL || kind() == S_MODULE);
  return new Scope(this, identifier, S_INTERFACE, 1, file, line);
}

Scope*
Scope::
newStructScope(const char* identifier, const char* file, int line)
{
  assert(kind() != S_OPERATION);
  return new Scope(this, identifier, S_STRUCT, 0, file, line);
}

Scope*
Scope::
newExceptionScope(const char* identifier, const char* file, int line)
{
  assert(kind() != S_OPERATION);
  return new Scope(this, identifier, S_EXCEPTION, 0, file, line);
}

Scope*
Scope::
newUnionScope(const char* identifier, const char* file, int line)
{
  assert(kind() != S_OPERATION);
  return new Scope(this, identifier, S_UNION, 0, file, line);
}

Scope*
Scope::
newOperationScope(const char* file, int line)
{
  assert(kind() == S_INTERFACE || kind() == S_VALUE);
  return new Scope(this, S_OPERATION, 0, file, line);
}

Scope*
Scope::
newValueScope(const char* identifier, const char* file, int line)
{
  assert(kind() == S_GLOBAL || kind() == S_MODULE);
  return new Scope(this, identifier, S_VALUE, 1, file, line);
}


// Searching functions

Scope::Entry*
Scope::
find(const char* identifier) const
{
  Entry* e;
  if (identifier[0] == '_') ++identifier;
  for (e = entries_; e; e = e->next()) {
    if (!(strcmp(identifier, e->identifier())))
      return e;
  }
  return 0;
}

Scope::Entry*
Scope::
iFind(const char* identifier) const
{
  Entry* e;
  if (identifier[0] == '_') ++identifier;
  for (e = entries_; e; e = e->next()) {
    if (Config::caseSensitive) {
      if (!(strcmp(identifier, e->identifier())))
	return e;
    }
    else {
      if (!(strcasecmp(identifier, e->identifier())))
	return e;
    }
  }
  return 0;
}


Scope::EntryList*
Scope::
findWithInheritance(const char* identifier) const
{
  const Entry* e;
  EntryList*   el = 0;
  EntryList*   in_el;

  if (identifier[0] == '_') ++identifier;
  if ((e = find(identifier))) {
    switch (e->kind()) {
    case Entry::E_MODULE:
    case Entry::E_DECL:
    case Entry::E_CALLABLE:
    case Entry::E_INHERITED:
    case Entry::E_INSTANCE:
      el = new EntryList(e);
      return el;
    case Entry::E_USE:
    case Entry::E_PARENT:
      break;
    }
  }
  // Not found locally -- try inherited scopes
  for (InheritSpec* is = inherited_; is; is = is->next()) {
    if (!is->scope()) continue; // Skip broken entries from earlier errors

    in_el = is->scope()->findWithInheritance(identifier);

    if (el)
      el->merge(in_el);
    else
      el = in_el;
  }
  for (ValueInheritSpec* vis = valueInherited_; vis; vis = vis->next()) {
    if (!vis->scope()) continue; // Skip broken entries from earlier errors

    in_el = vis->scope()->findWithInheritance(identifier);

    if (el)
      el->merge(in_el);
    else
      el = in_el;
  }
  return el;
}

Scope::EntryList*
Scope::
iFindWithInheritance(const char* identifier) const
{
  const Entry* e;
  EntryList*   el = 0;
  EntryList*   in_el;

  if (identifier[0] == '_') ++identifier;
  if ((e = iFind(identifier))) {
    switch (e->kind()) {
    case Entry::E_MODULE:
    case Entry::E_DECL:
    case Entry::E_CALLABLE:
    case Entry::E_INHERITED:
    case Entry::E_INSTANCE:
      el = new EntryList(e);
      return el;
    case Entry::E_USE:
    case Entry::E_PARENT:
      break;
    }
  }
  // Not found locally -- try inherited scopes
  for (InheritSpec* is = inherited_; is; is = is->next()) {
    if (!is->scope()) continue; // Skip broken entries from earlier errors

    in_el = is->scope()->iFindWithInheritance(identifier);

    if (el)
      el->merge(in_el);
    else
      el = in_el;
  }
  for (ValueInheritSpec* vis = valueInherited_; vis; vis = vis->next()) {
    if (!vis->scope()) continue; // Skip broken entries from earlier errors

    in_el = vis->scope()->iFindWithInheritance(identifier);

    if (el)
      el->merge(in_el);
    else
      el = in_el;
  }
  return el;
}

const Scope::Entry*
Scope::
findScopedName(const ScopedName* sn, const char* file, int line) const
{
  const Scope* s;

  // Start at relevant Scope
  if (sn->absolute())
    s = global();
  else
    s = this;

  // Find entry for each name component
  const Entry*          e = 0;
  EntryList*            el;
  ScopedName::Fragment* f = sn->scopeList();
  const char*           fid;

  IDL_Boolean top_component = 1;

  while (f) {
    fid = f->identifier();
    if (fid[0] == '_') fid++;

    do {
      el = s->iFindWithInheritance(fid);

      e = 0;
      if (el) {
	e = el->head();

	if (el->tail()) {
	  // Error -- ambiguous
	  if (file) {
	    char* ssn = sn->toString();
	    IdlError(file, line, "Ambiguous name `%s':", ssn);
	    delete [] ssn;

	    for (; el; el = el->tail()) {
	      char* ssn = el->head()->container()->scopedName()->toString();
	      IdlErrorCont(el->head()->file(), el->head()->line(),
			   "(`%s' defined in `%s')",
			   el->head()->identifier(), ssn);
	      delete [] ssn;
	    }
	  }
	  delete el;
	  return 0;
	}
	delete el;
	break;
      }
    } while (top_component && (s = s->parent()));
    top_component = 0;

    if (!e) {
      if (file) {
	char* ssn = sn->toString();
	IdlError(file, line, "Error in look-up of `%s': `%s' not found",
		 ssn, fid);
	delete [] ssn;
      }
      return 0;
    }

    if (strcmp(fid, e->identifier())) {
      // Case clash
      if (file) {
	char* ssn = sn->toString();
	IdlError(file, line, "Error in look-up of `%s': `%s' differs in case",
		 ssn, fid);
	delete [] ssn;
	ssn = e->scopedName()->toString();
	IdlErrorCont(e->file(), e->line(), "from `%s' declared here", ssn);
	delete [] ssn;
      }
      return 0;
    }

    f = f->next();
    if (f) { // More name fragments: check that current entry forms a scope
      s = e->scope();

      if (!s) {
	if (file) {
	  char* ssn = sn->toString();
	  IdlError(file, line,
		   "Error in look-up of `%s': `%s' does not form a scope",
		   ssn, e->identifier());
	  IdlErrorCont(e->file(), e->line(), "(`%s' defined here)",
		       e->identifier());
	  delete [] ssn;
	}
	return 0;
      }
    }
  }
  return e;
}


const Scope::Entry*
Scope::
findForUse(const ScopedName* sn, const char* file, int line)
{
  const Entry* e = findScopedName(sn, file, line);
  addUse(sn, file, line);
  return e;
}


static ScopedName*
findRelativeScope(const ScopedName::Fragment* from,
		  const ScopedName::Fragment* to,
		  const Scope* fromScope,
		  const Scope::Entry* target)
{
  ScopedName* result = 0;

  if (!to)
    return 0;

  if (from && !strcmp(from->identifier(), to->identifier())) {
    // Top name components match -- recursively try next components
    result = findRelativeScope(from->next(), to->next(), fromScope, target);
  }

  if (!result) {
    ScopedName*         test = new ScopedName(to, 0);
    const Scope::Entry* find = fromScope->findScopedName(test);

    if (find == target)
      result = test;
    else
      delete test;
  }
  return result;
}

ScopedName*
Scope::relativeScopedName(const ScopedName* from, const ScopedName* to)
{
  if (!global_) {
    // Haven't parsed any IDL yet!
    return 0;
  }

  if ((from && !from->absolute()) || !to->absolute())
    return 0;

  const Scope* fromScope;

  if (from) {
    const Entry* fromEntry = global_->findScopedName(from);
    if (!fromEntry) return 0;
    fromScope = fromEntry->scope();
  }
  else
    fromScope = global_;

  const Entry* toEntry = global_->findScopedName(to);
  if (!toEntry) return 0;

  ScopedName* result = findRelativeScope(from ? from->scopeList() : 0,
					 to->scopeList(), fromScope, toEntry);

  if (!result)
    result = new ScopedName(to);

  return result;
}


// Entry adding functions

void
Scope::
addUse(const ScopedName* sn, const char* file, int line)
{
  if (!sn->absolute()) {
    const char* id = sn->scopeList()->identifier();
    if (id[0] == '_') ++id;

    const Entry* clash = iFind(id);

    if (clash) {
      if (strcmp(id, clash->identifier())) {
	char* ssn = sn->toString();
	IdlError(file, line, "Use of `%s' clashes with identifier `%s'",
		 ssn, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(`%s' declared here)",
		     clash->identifier());
	delete [] ssn;
      }
      // Else the identifier is being used in the same scope that it was
      // declared, so don't mark it as used.
    }
    else {
      Entry* ue = new Entry(this, Entry::E_USE, id, 0, 0, 0, 0, file, line);
      appendEntry(ue);
      if (parent_ && parent_->nestedUse()) parent_->addUse(sn, file, line);
    }
  }
}


void
Scope::
addModule(const char* identifier, Scope* scope, Decl* decl,
	  const char* file, int line)
{
  if (*identifier == '_')
    ++identifier;
  else
    keywordClash(identifier, file, line);

  Entry* clash = iFind(identifier);

  if (clash) {
    switch (clash->kind()) {
    case Entry::E_MODULE:
      {
	if (!strcmp(identifier, clash->identifier())) {
	  return; // Reopening the module
	}
	IdlError(file, line,
		 "Declaration of module `%s' clashes with declaration "
		 "of module `%s'", identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(module `%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_DECL:
      {
	IdlError(file, line,
		 "Declaration of module `%s' clashes with declaration "
		 "of %s `%s'", identifier, clash->decl()->kindAsString(),
		 clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_CALLABLE:
    case Entry::E_INHERITED:
    case Entry::E_INSTANCE:
    case Entry::E_USE:
      {
	assert(0); // Should never reach here
      }
    case Entry::E_PARENT:
      {
	IdlError(file, line,
		 "Declaration of module `%s' clashes with name of "
		 "enclosing module `%s'", identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(module `%s' declared here)", clash->identifier());
	break;
      }
    }
  }
  Entry* e = new Entry(this, Entry::E_MODULE, identifier, scope,
		       decl, 0, 0, file, line);
  appendEntry(e);
}


void
Scope::
addDecl(const char* identifier, Scope* scope, Decl* decl, IdlType* idltype,
	const char* file, int line)
{
  if (*identifier == '_')
    ++identifier;
  else
    keywordClash(identifier, file, line);

  Entry* clash = iFind(identifier);

  if (clash) {
    switch (clash->kind()) {
    case Entry::E_MODULE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with declaration "
		 "of module `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(module `%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_DECL:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with earlier declaration "
		 "of %s `%s'", decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_CALLABLE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with %s `%s'",
		 decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_INHERITED:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with inherited %s `%s'",
		 decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	char* inhfrom =
	  clash->inh_from()->container()->scopedName()->toString();
	IdlErrorCont(clash->inh_from()->file(), clash->inh_from()->line(),
		     "(`%s' declared in %s here)",
		     clash->identifier(), inhfrom);
	delete [] inhfrom;
	break;
      }
    case Entry::E_INSTANCE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with instance `%s'",
		 decl->kindAsString(), identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_USE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with use of "
		 "identifier `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' used here)", clash->identifier());
	break;
      }
    case Entry::E_PARENT:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with name of "
		 "enclosing scope `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(`%s' declared here)",
		     clash->identifier());
	break;
      }
    }
  }
  Entry* e = new Entry(this, Entry::E_DECL, identifier, scope,
		       decl, idltype, 0, file, line);
  appendEntry(e);
}


void
Scope::
addCallable(const char* identifier, Scope* scope, Decl* decl,
	    const char* file, int line)
{
  if (*identifier == '_')
    ++identifier;
  else
    keywordClash(identifier, file, line);

  Entry* clash = iFind(identifier);

  if (clash) {
    switch (clash->kind()) {
    case Entry::E_MODULE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with declaration "
		 "of module `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(module `%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_DECL:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with earlier declaration "
		 "of %s `%s'", decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_CALLABLE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with %s `%s'",
		 decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_INHERITED:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with inherited %s `%s'",
		 decl->kindAsString(), identifier,
		 clash->decl()->kindAsString(), clash->identifier());
	char* inhfrom =
	  clash->inh_from()->container()->scopedName()->toString();
	IdlErrorCont(clash->inh_from()->file(), clash->inh_from()->line(),
		     "(`%s' declared in %s here)",
		     clash->identifier(), inhfrom);
	delete [] inhfrom;
	break;
      }
    case Entry::E_INSTANCE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with instance `%s'",
		 decl->kindAsString(), identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_USE:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with use of "
		 "identifier `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' used here)", clash->identifier());
	break;
      }
    case Entry::E_PARENT:
      {
	IdlError(file, line,
		 "Declaration of %s `%s' clashes with name of "
		 "enclosing scope `%s'", decl->kindAsString(),
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(`%s' declared here)",
		     clash->identifier());
	break;
      }
    }
  }
  Entry* e = new Entry(this, Entry::E_CALLABLE, identifier, scope,
		       decl, 0, 0, file, line);
  appendEntry(e);
}


void
Scope::
addInherited(const char* id, Scope* scope, Decl* decl,
	     Scope::Entry* inh_from, const char* file, int line)
{
  if (*id == '_') ++id;

  Entry* clash = iFind(id);

  if (clash) {
    switch (clash->kind()) {
    case Entry::E_MODULE:
    case Entry::E_DECL:
    case Entry::E_CALLABLE:
    case Entry::E_INSTANCE:
    case Entry::E_USE:
      {
	assert(0); // Should never reach here
      }
    case Entry::E_INHERITED:
      {
	if (inh_from != clash->inh_from()) {
	  IdlError(file, line, "In definition of `%s': clash between "
		   "inherited identifiers `%s' and `%s'",
		   identifier(), id, clash->identifier());
	  char* inhfrom =
	    inh_from->container()->scopedName()->toString();
	  IdlErrorCont(inh_from->file(), inh_from->line(),
		       "(%s `%s' declared in %s here)",
		       decl->kindAsString(), id, inhfrom);
	  delete [] inhfrom;
	  inhfrom = clash->inh_from()->container()->scopedName()->toString();
	  IdlErrorCont(clash->inh_from()->file(), clash->inh_from()->line(),
		       "(%s `%s' declared in %s here)",
		       clash->decl()->kindAsString(),
		       clash->identifier(), inhfrom);
	  delete [] inhfrom;
	}
	break;
      }
    case Entry::E_PARENT:
      {
	// It's not clear whether this is OK, but the spec doesn't say
	// it's definitely illegal

//  	IdlWarning(file, line,
//  		   "Inherited %s `%s' clashes with interface name `%s'",
//  		   decl->kindAsString(), id, clash->identifier());
//  	IdlWarningCont(decl->file(), decl->line(),
//  		       "(%s `%s' declared here)", decl->kindAsString(), id);
	break;
      }
    }
  }
  Entry* e = new Entry(this, Entry::E_INHERITED, id, scope,
		       decl, 0, inh_from, file, line);
  appendEntry(e);
}


void
Scope::
addInstance(const char* identifier, Decl* decl, IdlType* idltype,
	    const char* file, int line)
{
  if (*identifier == '_')
    ++identifier;
  else
    keywordClash(identifier, file, line);

  Entry* clash = iFind(identifier);

  if (clash) {
    switch (clash->kind()) {
    case Entry::E_MODULE:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with declaration "
		 "of module `%s'", identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(module `%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_DECL:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with declaration "
		 "of %s `%s'", identifier, clash->decl()->kindAsString(),
		 clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_CALLABLE:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with %s `%s'",
		 identifier, clash->decl()->kindAsString(),
		 clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(%s `%s' declared here)",
		     clash->decl()->kindAsString(), clash->identifier());
	break;
      }
    case Entry::E_INHERITED:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with inherited %s `%s'",
		 identifier, clash->decl()->kindAsString(),
		 clash->identifier());
	char* inhfrom =
	  clash->inh_from()->container()->scopedName()->toString();
	IdlErrorCont(clash->inh_from()->file(), clash->inh_from()->line(),
		     "(`%s' declared in %s here)",
		     clash->identifier(), inhfrom);
	delete [] inhfrom;
	break;
      }
    case Entry::E_INSTANCE:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with instance `%s'",
		 identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' declared here)", clash->identifier());
	break;
      }
    case Entry::E_USE:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with use of "
		 "identifier `%s'", identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(),
		     "(`%s' used here)", clash->identifier());
	break;
      }
    case Entry::E_PARENT:
      {
	IdlError(file, line,
		 "Instance identifier `%s' clashes with name of "
		 "enclosing scope `%s'", identifier, clash->identifier());
	IdlErrorCont(clash->file(), clash->line(), "(`%s' declared here)",
		     clash->identifier());
	break;
      }
    }
  }
  Entry* e = new Entry(this, Entry::E_INSTANCE, identifier, 0,
		       decl, idltype, 0, file, line);
  appendEntry(e);
}


void
Scope::
remEntry(Scope::Entry* re)
{
  if (entries_ == re) {
    entries_ = re->next();
    if (!entries_) last_ = 0;
  }
  else {
    Entry *e;
    for (e = entries_; e && (e->next() != re); e = e->next());
    assert(e);
    e->next_ = re->next();
    if (!e->next_) last_ = e;
  }
  re->next_ = 0;
  delete re;
}


IDL_Boolean
Scope::
keywordClash(const char* identifier, const char* file, int line)
{
  static const char* keywords[] = {
    "abstract", "any", "attribute", "boolean", "case", "char", "const",
    "context", "custom", "default", "double", "enum", "exception",
    "factory", "FALSE", "fixed", "float", "in", "inout", "interface",
    "long", "module", "native", "Object", "octet", "oneway", "out",
    "private", "public", "raises", "readonly", "sequence", "short",
    "string", "struct", "supports", "switch", "TRUE", "truncatable",
    "typedef", "union", "unsigned", "ValueBase", "valuetype", "void",
    "wchar", "wstring", 0
  };

  for (const char** k = keywords; *k; k++) {
    if (Config::caseSensitive) {
      if (!strcmp(*k, identifier)) {
	IdlError(file, line, "Identifier `%s' is identical to keyword `%s'",
		 identifier, *k);
	return 1;
      }
    }
    else {
      if (!strcasecmp(*k, identifier)) {
	IdlError(file, line, "Identifier `%s' clashes with keyword `%s'",
		 identifier, *k);
	return 1;
      }
    }
  }
  return 0;
}
