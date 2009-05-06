// -*- c++ -*-
//                          Package   : omniidl
// idlvalidate.cc           Created on: 1999/10/26
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2006 Apasphere Ltd
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
//   Visitor object to validate the tree

// $Id$
// $Log$
// Revision 1.7.2.3  2009/05/06 16:14:32  dgrisby
// Update lots of copyright notices.
//
// Revision 1.7.2.2  2006/11/02 14:26:14  dgrisby
// Suppress warning about unresolved forward delcared interfaces for
// interfaces in the CORBA module.
//
// Revision 1.7.2.1  2003/03/23 21:01:43  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.4.2.3  2001/10/17 16:48:34  dpg1
// Minor error message tweaks
//
// Revision 1.4.2.2  2000/11/01 15:44:56  dpg1
// Support for forward-declared structs and unions
//
// Revision 1.4.2.1  2000/07/17 10:36:05  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.5  2000/07/13 15:25:52  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.2.2.1  2000/03/06 15:03:47  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//
// Revision 1.2  1999/11/02 17:07:24  dpg1
// Changes to compile on Solaris.
//
// Revision 1.1  1999/10/27 14:05:53  dpg1
// *** empty log message ***
//

#include <idlvalidate.h>
#include <idlerr.h>
#include <idlast.h>
#include <idlconfig.h>

#include <string.h>

void
AstValidateVisitor::
visitAST(AST* a)
{
  for (Decl* d = a->declarations(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitModule(Module* m)
{
  for (Decl* d = m->definitions(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitInterface(Interface* i)
{
  for (Decl* d = i->contents(); d; d = d->next())
    d->accept(*this);
}

void
AstValidateVisitor::
visitForward(Forward* f)
{
  if (Config::forwardWarning) {
    if (f->isFirst() && !f->definition() &&
        strcmp(f->scopedName()->scopeList()->identifier(), "CORBA")) {

      char* ssn = f->scopedName()->toString();
      IdlWarning(f->file(), f->line(),
		 "Forward declared interface '%s' was never fully defined",
		 ssn);
      delete [] ssn;
    }
  }
}

void
AstValidateVisitor::
visitValueForward(ValueForward* f)
{
  if (Config::forwardWarning) {
    if (f->isFirst() && !f->definition()) {
      char* ssn = f->scopedName()->toString();
      IdlWarning(f->file(), f->line(),
		 "Forward declared valuetype '%s' was never fully defined",
		 ssn);
      delete [] ssn;
    }
  }
}

void
AstValidateVisitor::
visitStructForward(StructForward* f)
{
  if (f->isFirst() && !f->definition()) {
    char* ssn = f->scopedName()->toString();
    IdlError(f->file(), f->line(),
	     "Forward declared struct '%s' was never fully defined", ssn);
    delete [] ssn;
  }
}

void
AstValidateVisitor::
visitUnionForward(UnionForward* f)
{
  if (f->isFirst() && !f->definition()) {
    char* ssn = f->scopedName()->toString();
    IdlError(f->file(), f->line(),
	     "Forward declared union '%s' was never fully defined", ssn);
    delete [] ssn;
  }
}
