# -*- python -*-
#                           Package   : omniidl
# idlvisitor.py             Created on: 1999/10/27
#			    Author    : Duncan Grisby (dpg1)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
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
#   Base classes for Visitors

# $Id$
# $Log$
# Revision 1.2  1999/10/29 18:20:01  dpg1
# Clean up
#
# Revision 1.1  1999/10/29 15:47:07  dpg1
# First revision.
#

class AstVisitor :
    def visitAST(self, node):          return
    def visitModule(self, node):       return
    def visitInterface(self, node):    return
    def visitForward(self, node):      return
    def visitConst(self, node):        return
    def visitDeclarator(self, node):   return
    def visitTypedef(self, node):      return
    def visitMember(self, node):       return
    def visitStruct(self, node):       return
    def visitException(self, node):    return
    def visitCaseLabel(self, node):    return
    def visitUnionCase(self, node):    return
    def visitUnion(self, node):        return
    def visitEnumerator(self, node):   return
    def visitEnum(self, node):         return
    def visitAttribute(self, node):    return
    def visitParameter(self, node):    return
    def visitOperation(self, node):    return
    def visitNative(self, node):       return
    def visitStateMember(self, node):  return
    def visitFactory(self, node):      return
    def visitValueForward(self, node): return
    def visitValueBox(self, node):     return
    def visitValueAbs(self, node):     return
    def visitValue(self, node):        return


class TypeVisitor:
    def visitBaseType(self, type):     return
    def visitStringType(self, type):   return
    def visitWStringType(self, type):  return
    def visitSequenceType(self, type): return
    def visitFixedType(self, type):    return
    def visitDeclaredType(self, type): return
