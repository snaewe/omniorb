// -*- c++ -*-
//                          Package   : omniidl
// idldump.h                Created on: 1999/10/26
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
//   Visitor object to dump the tree

// $Id$
// $Log$
// Revision 1.3.2.3  2000/11/01 12:45:56  dpg1
// Update to CORBA 2.4 specification.
//
// Revision 1.3.2.2  2000/10/10 10:18:50  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.1.2.1  2000/08/14 14:35:14  dpg1
// IDL dumping now properly escapes string and char constants
//
// Revision 1.1  1999/10/27 14:05:58  dpg1
// *** empty log message ***
//

#ifndef _idldump_h_
#define _idldump_h_

#include <idlvisitor.h>
#include <idlscope.h>

class DumpVisitor : public AstVisitor, public TypeVisitor {
public:
  DumpVisitor();
  virtual ~DumpVisitor();

  void visitAST              (AST*);
  void visitModule           (Module*);
  void visitInterface        (Interface*);
  void visitForward          (Forward*);
  void visitConst            (Const*);
  void visitDeclarator       (Declarator*);
  void visitTypedef          (Typedef*);
  void visitMember           (Member*);
  void visitStruct           (Struct*);
  void visitStructForward    (StructForward*);
  void visitException        (Exception*);
  void visitCaseLabel        (CaseLabel*);
  void visitUnionCase        (UnionCase*);
  void visitUnion            (Union*);
  void visitUnionForward     (UnionForward*);
  void visitEnumerator       (Enumerator*);
  void visitEnum             (Enum*);
  void visitAttribute        (Attribute*);
  void visitParameter        (Parameter*);
  void visitOperation        (Operation*);
  void visitNative           (Native*);
  void visitStateMember      (StateMember*);
  void visitFactory          (Factory*);
  void visitValueForward     (ValueForward*);
  void visitValueBox         (ValueBox*);
  void visitValueAbs         (ValueAbs*);
  void visitValue            (Value*);

  void visitBaseType    (BaseType*);
  void visitStringType  (StringType*);
  void visitWStringType (WStringType*);
  void visitSequenceType(SequenceType*);
  void visitFixedType   (FixedType*);
  void visitDeclaredType(DeclaredType*);

private:
  int indent_;
  void printIndent();
  void printScopedName(const ScopedName* sn);
  void printString(const char* str);
  void printChar(const char c);
};




#endif // _idldump_h_
