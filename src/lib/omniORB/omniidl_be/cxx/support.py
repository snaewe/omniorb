# -*- python -*-
#                           Package   : omniidl
# support.py                Created on: 2000/08/14
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2003-2005 Apasphere Ltd
#    Copyright (C) 2000 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   Perform an initial pass of the AST and report anything accepted
#   by the front end but not yet supported.

from omniidl import idlast, idltype, idlvisitor
from omniidl_be.cxx import util

def checkIDL(tree):
    assert isinstance(tree, idlast.AST)
    
    TypeVisitor = SupportedTypeVisitor()
    ASTVisitor = SupportedASTVisitor(TypeVisitor)
    TypeVisitor.setASTVisitor(ASTVisitor)

    tree.accept(ASTVisitor)
    return

error_body = ""

def unsupportedIDL(found):
    error = "Unsupported IDL construct found in input (" + found + ")\n\n"
    error = error + error_body
    util.fatalError(error)
    
def unsupportedType(found):
    error = "Unsupported IDL type found in input (" + found + ")\n\n"
    error = error + error_body
    util.fatalError(error)

# Define an AST visitor to check for unsupported AST declarations.

# This should cover all the IDL AST node types defined in idlvisitor.py

seen = {}

class SupportedASTVisitor(idlvisitor.AstVisitor):
    def __init__(self, type_visitor):
        self.type_visitor = type_visitor

    def visitAST(self, node):
        for declaration in node.declarations():
            declaration.accept(self)

    def visitModule(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        for definition in node.definitions():
            definition.accept(self)

    def visitInterface(self, node):
        if seen.has_key(node): return
        seen[node] = 1

        for decl in node.contents():
            decl.accept(self)

    def visitForward(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
    def visitConst(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        node.constType().accept(self.type_visitor)

    def visitDeclarator(self, node):
        if seen.has_key(node): return
        seen[node] = 1

        if node.alias():
            node.alias().accept(self)

    def visitTypedef(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        node.aliasType().accept(self.type_visitor)

        for declarator in node.declarators():
            declarator.accept(self)

    def visitMember(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        node.memberType().accept(self.type_visitor)

        for declarator in node.declarators():
            declarator.accept(self)

    def visitStruct(self, node):
        if seen.has_key(node): return
        seen[node] = 1

        for member in node.members():
            member.accept(self)

    def visitException(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        for member in node.members():
            member.accept(self)

    # Don't bother recursing down to a case label

    def visitUnionCase(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        node.caseType().accept(self.type_visitor)
        node.declarator().accept(self)

    def visitUnion(self, node):
        if seen.has_key(node): return
        seen[node] = 1
        
        node.switchType().accept(self.type_visitor)
        for case in node.cases():
            case.accept(self)

    # Skip Enumerators

    def visitEnum(self, node): return

    def visitAttribute(self, node):        
        node.attrType().accept(self.type_visitor)

    def visitParameter(self, node):
        node.paramType().accept(self.type_visitor)

    def visitOperation(self, node):
        node.returnType().accept(self.type_visitor)
        for parameter in node.parameters():
            parameter.accept(self)
        for exception in node.raises():
            exception.accept(self)

    def visitNative(self, node):
        unsupportedIDL("native")

    def visitStateMember(self, node):
        pass

    def visitFactory(self, node):
        pass

    def visitValueForward(self, node):
        pass

    def visitValueBox(self, node):
        pass

    def visitValueAbs(self, node):
        pass

    def visitValue(self, node):
        if node.custom():
            unsupportedIDL("custom valuetypes")

        


class SupportedTypeVisitor(idlvisitor.TypeVisitor):
    def __init__(self):
        pass

    def setASTVisitor(self, visitor):
        self.ast_visitor = visitor

    def visitBaseType(self, type):

        not_supported = { idltype.tk_null:       "null",
                          idltype.tk_Principal:  "CORBA::Principal" }

        if type.kind() in not_supported.keys():
            unsupportedType(not_supported[type.kind()])
                            
    def visitStringType(self, type):
        return

    def visitWStringType(self, type):
        return

    def visitSequenceType(self, type):
        type.seqType().accept(self)

    def visitFixedType(self, type):
        return

    def visitDeclaredType(self, type):
        type.decl().accept(self.ast_visitor)
    
