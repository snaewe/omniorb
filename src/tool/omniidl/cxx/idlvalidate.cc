// -*- c++ -*-
//                          Package   : omniidl
// idlvalidate.cc           Created on: 1999/10/26
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
//   Visitor object to validate the tree

// $Id$
// $Log$
// Revision 1.6  2001/02/21 14:12:07  dpg1
// Merge from omni3_develop for 3.0.3 release.
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
visitForward(Forward* f)
{
  if (Config::forwardWarning) {
    if (f->isFirst() && !f->definition()) {
      char* ssn = f->scopedName()->toString();
      IdlWarning(f->file(), f->line(),
		 "Forward declared interface `%s' was never fully defined",
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
		 "Forward declared valuetype `%s' was never fully defined",
		 ssn);
      delete [] ssn;
    }
  }
}
