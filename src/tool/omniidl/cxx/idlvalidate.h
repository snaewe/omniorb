// -*- c++ -*-
//                          Package   : omniidl
// idlvalidate.h            Created on: 1999/10/26
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
// Revision 1.3.2.1  2000/07/17 10:36:05  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.4  2000/07/13 15:25:52  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.1  1999/10/27 14:05:53  dpg1
// *** empty log message ***
//

#ifndef _idlvalidate_h_
#define _idlvalidate_h_

#include <idlvisitor.h>

// AstValidateVisitor currently just issues warnings if interfaces or
// values are forward declared but never fully declared

class AstValidateVisitor : public AstVisitor {
public:
  AstValidateVisitor() {}
  virtual ~AstValidateVisitor() {}

  void visitAST              (AST*          a);
  void visitModule           (Module*       m);
  void visitForward          (Forward*      f);
  void visitValueForward     (ValueForward* f);
};




#endif // _idlvalidate_h_
