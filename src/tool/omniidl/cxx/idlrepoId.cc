// -*- c++ -*-
//                          Package   : omniidl
// idlrepoId.cc             Created on: 1999/10/11
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
//   Implementation of repoId management

// $Id$
// $Log$
// Revision 1.5.2.9  2005/07/21 15:25:08  dgrisby
// Silence some gcc 4 warnings / errors.
//
// Revision 1.5.2.8  2005/05/03 10:12:40  dgrisby
// Trying to redefine built in CORBA module types led to a segfault.
//
// Revision 1.5.2.7  2003/04/25 15:54:28  dgrisby
// Remove unnecessary iostream.h include.
//
// Revision 1.5.2.6  2001/11/13 14:11:46  dpg1
// Tweaks for CORBA 2.5 compliance.
//
// Revision 1.5.2.5  2001/10/17 16:48:33  dpg1
// Minor error message tweaks
//
// Revision 1.5.2.4  2000/11/03 12:20:58  dpg1
// #pragma ID can now be declared more than once for a type, as long as
// the id is the same.
//
// Revision 1.5.2.3  2000/10/27 16:31:09  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.5.2.2  2000/10/10 10:18:51  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.3.2.1  2000/08/29 10:20:27  dpg1
// Operations and attributes now have repository ids.
//
// Revision 1.3  1999/11/08 10:50:47  dpg1
// Change to behaviour when files end inside a scope.
//
// Revision 1.2  1999/11/02 17:07:25  dpg1
// Changes to compile on Solaris.
//
// Revision 1.1  1999/10/27 14:05:56  dpg1
// *** empty log message ***
//

#include <idlrepoId.h>
#include <idlast.h>
#include <idlutil.h>
#include <idlerr.h>
#include <idlvisitor.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Globals from lexer/parser
extern int   yylineno;
extern char* currentFile;

Prefix* Prefix::current_ = 0;

Prefix::
Prefix(char* str, IDL_Boolean isfile) :
  str_(str), parent_(current_), isfile_(isfile)
{
  current_ = this;
}

Prefix::
~Prefix()
{
  current_ = parent_;
  delete [] str_;
}

const char*
Prefix::
current()
{
  return current_->get();
}

void
Prefix::
newScope(const char* name)
{
  if (name[0] == '_') ++name;
  int len   = strlen(current()) + strlen(name) + 2;
  char* str = new char[len];

  strcpy(str, current());
  if (str[0] != '\0') strcat(str, "/");
  strcat(str, name);

  new Prefix(str, 0);
}

void
Prefix::
newFile()
{
  char* str = new char[1];
  str[0]    = '\0';
  new Prefix(str, 1);
}

void
Prefix::
setPrefix(const char* prefix)
{
  current_->set(prefix);
}

void
Prefix::
endScope()
{
  if (current_->parent_)
    delete current_;
  else
    IdlWarning(currentFile, yylineno,
	       "Confused by pre-processor line directives");
}

void
Prefix::
endFile()
{
  if (!current_->isfile()) {
    IdlWarning(currentFile, yylineno,
	       "File ended inside a declaration. "
	       "Repository identifiers may be incorrect");
  }
  if (current_->parent_)
    delete current_;
  else
    IdlWarning(currentFile, yylineno,
	       "Confused by pre-processor line directives");
}

void
Prefix::
endOuterFile()
{
  if (current_->parent_)
    IdlWarning(currentFile, yylineno,
	       "Confused by pre-processor line directives");
  else
    delete current_;
}

const char*
Prefix::
get()
{
  return str_;
}

void
Prefix::
set(const char* setTo)
{
  char* str;
  delete [] str_;
  if (setTo[0] == '\0') {
    str    = new char[1];
    str[0] = '\0';
  }
  else
    str = idl_strdup(setTo);

  str_ = str;
}

IDL_Boolean
Prefix::
isfile()
{
  return isfile_;
}


// Implementation of DeclRepoId

DeclRepoId::
DeclRepoId(const char* identifier)

  : eidentifier_(idl_strdup(identifier)),
    prefix_(idl_strdup(Prefix::current())),
    set_(0), maj_(1), min_(0)
{
  if (identifier[0] == '_')
    identifier_ = idl_strdup(++identifier);
  else
    identifier_ = eidentifier_;

  const ScopedName* psn = Scope::current()->scopedName();
  if (psn) {
    scopedName_ = new ScopedName(psn);
    scopedName_->append(identifier);
  }
  else
    scopedName_ = new ScopedName(identifier, 1);

  genRepoId();
}

DeclRepoId::
~DeclRepoId()
{
  if (identifier_ != eidentifier_) delete [] identifier_;
  delete [] eidentifier_;
  delete [] repoId_;
  delete [] prefix_;
  if (set_) delete [] rifile_;
}

void
DeclRepoId::
setRepoId(const char* repoId, const char* file, int line)
{
  if (set_) {
    if (strcmp(repoId, repoId_)) {
      IdlError(file, line, "Cannot set repository id of '%s' to '%s'",
	       identifier_, repoId);
      IdlErrorCont(rifile_, riline_,
		   "Repository id previously set to '%s' here", repoId_);
    }
  }
  else {
    delete [] repoId_;
    repoId_ = idl_strdup(repoId);
    set_    = 1;
    rifile_ = idl_strdup(file);
    riline_ = line;

    for (; *repoId && *repoId != ':'; ++repoId);
    if (*repoId == '\0') goto invalid;

    // If the repoId set is in OMG IDL format, we must figure out the
    // version number, so a future #pragma version can succeed if the
    // same version is given. Evil spec.

    if (!strncmp(repoId_, "IDL:", 4)) {
      const char* c;
      for (c=repoId_ + 4; *c && *c != ':'; ++c);
      if (*c++ == '\0') goto invalid;

      // c should now point to a string of the form maj.min
      if (sscanf(c, "%hd.%hd", &maj_, &min_) != 2)
	goto invalid;

      // Check there's no trailing garbage
      for (; *c && isdigit(*c); ++c);
      if (*c++ != '.') goto invalid;
      for (; *c && isdigit(*c); ++c);
      if (*c != '\0') goto invalid;
    }
    else {
      maj_ = -1; // Make sure a future #pragma version complains
    }
  }
  return;

 invalid:
  IdlWarning(file, line,
	     "Repository id of '%s' set to invalid string '%s'",
	     identifier_, repoId_);
  maj_ = -1;
}

void
DeclRepoId::
setVersion(IDL_Short maj, IDL_Short min, const char* file, int line)
{
  if (set_) {
    if (maj_ != maj || min_ != min) {
      IdlError(file, line, "Cannot set version of '%s' to '%d.%d'",
	       identifier_, maj, min);
      IdlErrorCont(rifile_, riline_,
		   "Repository id previously set to '%s' here", repoId_);
    }
  }
  else {
    delete [] repoId_;
    maj_    = maj;
    min_    = min;
    set_    = 1;
    rifile_ = idl_strdup(file);
    riline_ = line;
    genRepoId();
  }
}

void
DeclRepoId::
genRepoId()
{
  char* id;
  int   len;

  // RepoId length = IDL: + prefix + "/" + identifier + : + maj + . + min + \0
  len = 4 + strlen(prefix_) + 1 + strlen(identifier_) + 1 + 5 + 1 + 5 + 1;

  char* repoId = new char[len];

  sprintf(repoId, "IDL:%s%s%s:%hd.%hd", prefix_,
	  prefix_[0] == '\0' ? "" : "/", identifier_, maj_, min_);
  repoId_ = repoId;
}


// Static set functions

class SetRepoIdVisitor : public AstVisitor {
public:

  SetRepoIdVisitor(const char* repoId, const char* file, int line)
    : r_(repoId), f_(file), l_(line) {}
  virtual ~SetRepoIdVisitor() {}

  void visitModule       (Module* d)       { d->setRepoId(r_, f_, l_); }
  void visitInterface    (Interface* d)    { d->setRepoId(r_, f_, l_); }
  void visitForward      (Forward* d)      { d->setRepoId(r_, f_, l_); }
  void visitConst        (Const* d)        { d->setRepoId(r_, f_, l_); }
  void visitDeclarator   (Declarator* d)   { d->setRepoId(r_, f_, l_); }
  void visitStruct       (Struct* d)       { d->setRepoId(r_, f_, l_); }
  void visitException    (Exception* d)    { d->setRepoId(r_, f_, l_); }
  void visitUnion        (Union* d)        { d->setRepoId(r_, f_, l_); }
  void visitEnum         (Enum* d)         { d->setRepoId(r_, f_, l_); }
  void visitNative       (Native* d)       { d->setRepoId(r_, f_, l_); }
  void visitValueForward (ValueForward* d) { d->setRepoId(r_, f_, l_); }
  void visitValueBox     (ValueBox* d)     { d->setRepoId(r_, f_, l_); }
  void visitValueAbs     (ValueAbs* d)     { d->setRepoId(r_, f_, l_); }
  void visitValue        (Value* d)        { d->setRepoId(r_, f_, l_); }
  void visitMember       (Member* d)       { error(d); }
  void visitUnionCase    (UnionCase* d)    { error(d); }
  void visitEnumerator   (Enumerator* d)   { error(d); }
  void visitAttribute    (Attribute* d)    { error(d); }
  void visitOperation    (Operation* d)    { d->setRepoId(r_, f_, l_); }
  void visitStateMember  (StateMember* d)  { error(d); }
  void visitFactory      (Factory* d)      { error(d); }

private:
  const char* r_;
  const char* f_;
  int         l_;
  void error(Decl* d) {
    IdlError(f_, l_, "Cannot set repository id of %s", d->kindAsString());
  }
};

void
DeclRepoId::
setRepoId(Decl* d, const char* repoId, const char* file, int line)
{
  SetRepoIdVisitor v(repoId, file, line);
  d->accept(v);
}


class SetVersionVisitor : public AstVisitor {
public:

  SetVersionVisitor(IDL_Short maj, IDL_Short min,
		    const char* file, int line)
    : a_(maj), i_(min), f_(file), l_(line) {}
  virtual ~SetVersionVisitor() {}

  void visitModule       (Module* d)       { d->setVersion(a_, i_, f_, l_); }
  void visitInterface    (Interface* d)    { d->setVersion(a_, i_, f_, l_); }
  void visitForward      (Forward* d)      { d->setVersion(a_, i_, f_, l_); }
  void visitConst        (Const* d)        { d->setVersion(a_, i_, f_, l_); }
  void visitDeclarator   (Declarator* d)   { d->setVersion(a_, i_, f_, l_); }
  void visitStruct       (Struct* d)       { d->setVersion(a_, i_, f_, l_); }
  void visitException    (Exception* d)    { d->setVersion(a_, i_, f_, l_); }
  void visitUnion        (Union* d)        { d->setVersion(a_, i_, f_, l_); }
  void visitEnum         (Enum* d)         { d->setVersion(a_, i_, f_, l_); }
  void visitNative       (Native* d)       { d->setVersion(a_, i_, f_, l_); }
  void visitValueForward (ValueForward* d) { d->setVersion(a_, i_, f_, l_); }
  void visitValueBox     (ValueBox* d)     { d->setVersion(a_, i_, f_, l_); }
  void visitValueAbs     (ValueAbs* d)     { d->setVersion(a_, i_, f_, l_); }
  void visitValue        (Value* d)        { d->setVersion(a_, i_, f_, l_); }
  void visitMember       (Member* d)       { error(d); }
  void visitUnionCase    (UnionCase* d)    { error(d); }
  void visitEnumerator   (Enumerator* d)   { error(d); }
  void visitAttribute    (Attribute* d)    { error(d); }
  void visitOperation    (Operation* d)    { d->setVersion(a_, i_, f_, l_); }
  void visitStateMember  (StateMember* d)  { error(d); }
  void visitFactory      (Factory* d)      { error(d); }

private:
  IDL_Short a_;
  IDL_Short i_;
  const char*  f_;
  int          l_;
  void error(Decl* d) {
    IdlError(f_, l_, "Cannot set version of %s", d->kindAsString());
  }
};

void
DeclRepoId::
setVersion(Decl* d, IDL_Short maj, IDL_Short min,
	   const char* file, int line)
{
  SetVersionVisitor v(maj, min, file, line);
  d->accept(v);
}
