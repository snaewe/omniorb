# -*- python -*-
#                           Package   : omniidl
# showast.py                Created on: 1999/11/3
#			    Author    : David Scott (djs)
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
#   Simple AST dumping module for testing utility functions etc
#   (derived from dpg1's dump.py)

# $Id$
# $Log$
# Revision 1.4.2.1  2000/07/17 10:35:43  sll
# Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
#
# Revision 1.5  2000/07/13 15:26:02  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.2.2.1  2000/02/14 18:34:57  dpg1
# New omniidl merged in.
#
# Revision 1.2  1999/11/03 12:06:19  djs
# Type utility functions now work with the new AST representation
#
# Revision 1.1  1999/11/03 11:09:33  djs
# General module renaming
#


"""Dumps the AST tree"""

from omniidl import idlast, idltype, idlutil, output
from omniidl_be.cxx import tyutil

import sys, string

import showast
self = showast

def run(tree, args):
    print "Showing the AST tree: "

    self.st = output.Stream(sys.stdout, 2)

    tree.accept(self)


def idNode(node):
    if isinstance(node, ast.Module):
        return "Module " + node.identifier()
    elif isinstance(node, ast.Forward):
        return "Forward " + node.identifier()
    elif isinstance(node, ast.Interface):
        return "Interface " + node.identifier()
    elif isinstance(node, ast.Const):
        return "Const "
    elif isinstance(node, ast.Typedef):
        return "Typedef "
    elif isinstance(node, ast.Struct):
        return "Struct " + node.identifier()
    elif isinstance(node, ast.Exception):
        return "Exception " + node.identifier()
    elif isinstance(node, ast.Union):
        return "Union " + node.identifier()
    elif isinstance(node, ast.Enum):
        return "Enum " + node.identifier()
    elif isinstance(node, ast.Attribute):
        return "Attributes " + string.join(node.identifiers(), " ")
    elif isinstance(node, ast.Operation):
        return "Operation " + node.identifier()

def showID(node):
    identifier = node.identifier()
    cxx_identifier = tyutil.mapID(identifier)
    st.out("""\
IDL identifier  = @idl@
CXX identifier  = @cxx@""",
           idl = identifier,
           cxx = cxx_identifier)

def showVariable(node):
    variable = tyutil.isVariableDecl(node)
    if variable:
        st.out("""\
Variable length type""")
    else:
        st.out("""\
Fixed length type""")

def visitAST(node):
    st.out("""\
AST {""")
    st.inc_indent()
    st.out("""\
declarations:""")
    for n in node.declarations():
        n.accept(self)
    st.dec_indent()
    st.out("""\
} /* AST */""")
    
def visitModule(node):
    st.out("""\
module {""")
    st.inc_indent()
    showID(node)
    st.out("""\
guard name = @guard@""", guard = tyutil.guardName(node.scopedName()))
    st.out("""\
definitions:""")
    for n in node.definitions():
        n.accept(self)
    st.dec_indent()
    st.out("""\
} /* module @identifier@ */""", identifier = node.identifier())

def visitInterface(node):
    st.out("""\
interface {""")
    st.inc_indent()
    showID(node)
    showVariable(node)
    st.out("""\
inherits = @inherits@""", inherits = repr(node.inherits()))
    st.out("""\
contents {""")
    st.inc_indent()
    
    for n in node.contents():
        n.accept(self)
        
    st.dec_indent()
    st.out("""\
} /* contents of interface @identifier@ */""", identifier = node.identifier())

    st.dec_indent()
    st.out("""\
} /* interface @identifier@ */""", identifier = node.identifier())
    

def visitForward(node):
    st.out("""\
forward {""")
    st.inc_indent()
    showID(node)
    st.dec_indent()
    st.out("""\
}""")


def visitConst(node):
    st.out("""\
const {""")
    st.inc_indent()
    showID(nodes)
    node.constType().accept(self)
    st.out("""\
type  = @type@
value = @value@""", type = __result_type, value = repr(node.value()))
    st.dec_indent()
    st.out("""\
} /* const */""")
    

def visitTypedef(node):
#    tyutil.deref(1)



    st.out("""\
typedef {""")
    st.inc_indent()
    st.out("""\
constrType = @constrType@""", constrType = repr(node.constrType()))
    deref = tyutil.deref(node.aliasType())
    deref.accept(self)
    st.out("""\
deref Type = @deref@""", deref = __result_type)
    if node.constrType():
        st.out("""\
decl {""")
        st.inc_indent()
        node.aliasType().decl().accept(self)
        st.dec_indent()
        st.out("""\
} /* decl */""")

    node.aliasType().accept(self)
    type = __result_type
    st.out("""\
declarators {""")
    st.inc_indent()
    for d in node.declarators():
        d.accept(self)
        dims = tyutil.typeDims(node.aliasType())
        st.out("""\
@type@ @decl@  dims = @dims@""",
               type = type, decl = __result_declarator, dims = repr(dims))
    st.dec_indent()
    st.out("""\
} /* declarators */""")
    st.dec_indent()
    st.out("""\
} /* typedef */""")

def visitStruct(node):
    st.out("""\
struct {""")
    st.inc_indent()
    showID(node)
    showVariable(node)
    for m in node.members():
        if m.constrType():
            st.out("""\
constrType = 1
decl {""")
            st.inc_indent()
            m.memberType().decl().accept(self)
            st.dec_indent()
            st.out("""\
} /* decl */""")
        m.memberType().accept(self)
        type = self.__result_type
        principalID = tyutil.principalID(m.memberType())
        opINtype = tyutil.operationArgumentType(m.memberType())[1]
        objrefTemplate = tyutil.objRefTemplate(m.memberType(), "Member")
        st.out("""\
declarators { """)
        st.inc_indent()
        st.out("""\
principalID       = @id@
objref template   = @objref@
operation In type = @operin@""",
               id = principalID,
               objref = objrefTemplate,
               operin = opINtype)
        for d in m.declarators():
            d.accept(self)
            st.out("""\
@type@ @decl@""", type = type, decl = __result_declarator)
        st.dec_indent()
        st.out("""\
} /* declarators */""")
            
    st.dec_indent()
    st.out("""\
} /* struct @identifier@ */""", identifier = node.identifier())
    

def visitException(node):
    st.out("""\
exception {""")
    st.inc_indent()
    showID(node)
    showVariable(node)
    for m in node.members():
        if m.constrType():
            st.out("""\
constrType = 1
decl {""")
            st.inc_indent()
            m.memberType().decl().accept(self)
            st.dec_indent()
            st.out("""\
} /* decl */""")
        m.memberType().accept(self)
        type = self.__result_type
        st.out("""\
declarators {""")
        st.inc_indent()
        for d in m.declarators():
            d.accept(self)
            st.out("""\
@type@ @decl@""", type = type, decl = __result_declarator)

        st.dec_indent()
        st.out("""\
} /* declarators */""")
        

    st.dec_indent()
    st.out("""\
} /* exception @identifier@ */""", identifier = node.identifier())

#
# -----------------------------------
#

def visitUnion(node):
    if node.constrType():
        
        self.st.out("""\
union @id@ switch (""",
                    id = node.identifier())
        self.st.inc_indent()
        node.switchType().decl().accept(self)
        self.st.out(""") {""")
        self.st.dec_indent

    else:
        node.switchType().accept(self)
        stype = self.__result_type
        
        self.st.out("""\
union @id@ switch (@stype@) {""",

                    id=node.identifier(), stype=stype)

    showVariable(node)

    for c in node.cases():
        if c.constrType():
            self.st.inc_indent()
            c.caseType().decl().accept(self)
            self.st.dec_indent()
            
        for l in c.labels():
            if l.default():
                self.st.out("""\
default:""")
            else:
                if l.labelKind() == idltype.tk_enum:
                    lv = "::" + idlutil.ccolonName(l.value().scopedName())
                else:
                    lv = str(l.value())
                    
                self.st.out("""\
  case @lv@:""",
                           lv=lv)
                    
        c.caseType().accept(self)
        type = self.__result_type
        c.declarator().accept(self)
        decl = self.__result_declarator

        self.st.out("""\
    @type@ @decl@;""",
                   
                    type=type, decl=decl)

    self.st.out("};")


def visitEnum(node):
    enuml = []
    for e in node.enumerators(): enuml.append(e.identifier())

    enums = string.join(enuml, ", ")

    self.st.out("""\
enum @id@ {@enums@};""",

                id=node.identifier(), enums=enums)

def visitAttribute(node):
    if node.readonly():
        readonly = "readonly "
    else:
        readonly = ""

    node.attrType().accept(self)
    type = self.__result_type
    
    ids  = string.join(node.identifiers(), ", ")

    self.st.out("""\
@readonly@attribute @type@ @ids@;""",

                readonly=readonly, type=type, ids=ids)


def visitOperation(node):
    if node.oneway():
        oneway = "oneway "
    else:
        oneway = ""
        
    node.returnType().accept(self)
    rtype = self.__result_type

    paraml = []
    for p in node.parameters():
        if   p.is_in() and p.is_out(): inout = "inout"
        elif p.is_in():                inout = "in"
        else:                          inout = "out"
        p.paramType().accept(self)
        type = self.__result_type
        paraml.append(inout + " " + type + " " + p.identifier())

    params = string.join(paraml, ", ")

    if len(node.raises()) > 0:
        raisel = []
        for r in node.raises():
            ename  = idlutil.ccolonName(r.scopedName())
            raisel.append(ename)

        raises = " raises (" + string.join(raisel, ", ") + ")"
    else:
        raises = ""
        
    self.st.out("""\
@oneway@@rtype@ @id@(@params@)@raises@;""",
               
                oneway=oneway, rtype=rtype, id=node.identifier(),
                params=params, raises=raises)


def visitDeclarator(node):
    l = [node.identifier()]
    for s in node.sizes():
        l.append("[" + str(s) + "]")

    self.__result_declarator = string.join(l, "")

    




def declaratorToString(decl):
    if (isinstance(decl, ast.SimpleTypedefDeclarator)):
        return ("(SimpleTypedefDeclarator "+decl.identifier()+")")
    elif (isinstance(decl, ast.SimpleDeclarator)):
        return ("(SimpleDeclarator "+decl.identifier()+")")
    elif (isinstance(decl, ast.ArrayTypedefDeclarator)):
        text = "(ArrayTypedefDeclarator "+decl.identifier()

    sizes = []
    for s in decl.sizes(): sizes.append("[" + str(s) + "]")
    return text + string.join(sizes, "")


ttsMap = {
    idltype.tk_void:       "void",
    idltype.tk_short:      "short",
    idltype.tk_long:       "long",
    idltype.tk_ushort:     "unsigned short",
    idltype.tk_ulong:      "unsigned long",
    idltype.tk_float:      "float",
    idltype.tk_double:     "double",
    idltype.tk_boolean:    "boolean",
    idltype.tk_char:       "char",
    idltype.tk_octet:      "octet",
    idltype.tk_any:        "any",
    idltype.tk_TypeCode:   "CORBA::TypeCode",
    idltype.tk_Principal:  "CORBA::Principal",
    idltype.tk_longlong:   "long long",
    idltype.tk_ulonglong:  "unsigned long long",
    idltype.tk_longdouble: "long double",
    idltype.tk_wchar:      "wchar"
}

def visitBaseType(type):
    self.__result_type = self.ttsMap[type.kind()]

def visitStringType(type):
    if type.bound() == 0:
        self.__result_type = "string"
    else:
        self.__result_type = "string<" + str(type.bound()) + ">"

def visitWStringType(type):
    if type.bound() == 0:
        self.__result_type = "wstring"
    else:
        self.__result_type = "wstring<" + str(type.bound()) + ">"

def visitSequenceType(type):
    type.seqType().accept(self)
    if type.bound() == 0:
        self.__result_type = "sequence<" + self.__result_type + ">"
    else:
        self.__result_type = "sequence<" + self.__result_type + ", " +\
                             str(type.bound()) + ">"
    self.__result_type = self.__result_type + \
                         "   sequence template = " + \
                         tyutil.sequenceTemplate(type)

def visitFixedType(type):
    self.__result_type = "fixed <" + str(type.digits()) + ", " +\
                         str(type.scale()) + ">"


def visitDeclaredType(type):
    self.__result_type = repr(type.decl().scopedName())


