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
# Revision 1.1  1999/10/29 15:47:07  dpg1
# First revision.
#

class AstVisitor :
    def visitAST(self):          return
    def visitModule(self):       return
    def visitInterface(self):    return
    def visitForward(self):      return
    def visitConst(self):        return
    def visitDeclarator(self):   return
    def visitTypedef(self):      return
    def visitMember(self):       return
    def visitStruct(self):       return
    def visitException(self):    return
    def visitCaseLabel(self):    return
    def visitUnionCase(self):    return
    def visitUnion(self):        return
    def visitEnumerator(self):   return
    def visitEnum(self):         return
    def visitAttribute(self):    return
    def visitParameter(self):    return
    def visitOperation(self):    return
    def visitNative(self):       return
    def visitStateMember(self):  return
    def visitFactory(self):      return
    def visitValueForward(self): return
    def visitValueBox(self):     return
    def visitValueAbs(self):     return
    def visitValue(self):        return


class TypeVisitor:
    def visitBaseType(self):     return
    def visitStringType(self):   return
    def visitWStringType(self):  return
    def visitSequenceType(self): return
    def visitFixedType(self):    return
    def visitDeclaredType(self): return
